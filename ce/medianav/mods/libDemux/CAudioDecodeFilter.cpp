
#include "CAudioDecodeFilter.h"
#include <audshow.h>

CAudioDecodeFilter::CAudioDecodeFilter(TCHAR *name, LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsID)
:CTransformFilter(name, pUnk, clsID)
,m_frameBuffer(NULL)
,m_frameBufferUsed(0)
,m_frameBufferSize(0)
,m_outBufferAlignment(1)
,m_deliveryThreshold(0)
#if ADECODE_PERF > 0
,m_maxOutputBufUsed(0)
,m_dbgMaxFrameProcessTime(0)
,m_dbgAvgFrameProcessTime(0)
,m_framesTookLongerThanAvg(0)
,m_totalFramesProcessed(0)
#endif
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::CAudioDecodeFilter()\r\n");
}

CAudioDecodeFilter::~CAudioDecodeFilter()
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::~CAudioDecodeFilter()\r\n");
#if ADECODE_PERF > 0
    LARGE_INTEGER freq;
    if(QueryPerformanceFrequency(&freq))
    {
        m_dbgAvgFrameProcessTime *= 1000000;
        m_dbgAvgFrameProcessTime /= freq.QuadPart;

        m_dbgMaxFrameProcessTime *= 1000000;
        m_dbgMaxFrameProcessTime /= freq.QuadPart;
    }else
    {
        filterDebugPrintf(ADECODE_PERF, L"CAudioDecodeFilter::~CAudioDecodeFilter() QueryPerformanceFrequency() failed\r\n");
    }
    filterDebugPrintf(ADECODE_PERF, L"CAudioDecodeFilter::~CAudioDecodeFilter() \r\n\t m_maxOutputBufUsed=%d \r\n\t avarage frame processing time = %I64d us \r\n\t max frame processing time = %I64d us \r\n\t total frames = %u \r\n\t long frames = %u  -  %u%%\r\n", m_maxOutputBufUsed, m_dbgAvgFrameProcessTime, m_dbgMaxFrameProcessTime, m_totalFramesProcessed, m_framesTookLongerThanAvg, m_framesTookLongerThanAvg * 100 / (m_totalFramesProcessed ? m_totalFramesProcessed : 1));
#endif
}

HRESULT CAudioDecodeFilter::Receive(IMediaSample *pSample)
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::Receive\r\n");
    /*  Check for other streams and pass them on */
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA) {
        return m_pOutput->Deliver(pSample);
    }

    // Actual code start here
    TransformBuffersState buffersState;
    BYTE* inData;
    DWORD inDataSize;
    pSample->GetPointer(&inData);
    inDataSize = pSample->GetActualDataLength();

    bool discontinuity = false;
    // Reset frame buffer in case of seeking.
    if(pSample->IsDiscontinuity() == S_OK)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() discontinuity detected! Reset frame buffer.\r\n");
        m_frameBufferUsed = 0;
        // Release any pending output data
        if(m_curOutMediaSample)
            m_curOutMediaSample.Release();
        discontinuity = true;
    }

    HRESULT hr;
    if(!m_curOutMediaSample)
    {
        IMediaSample *pOutSample;
        hr = InitializeOutputSample(pSample, &pOutSample);
        if(FAILED(hr)) 
            return hr;
        m_curOutMediaSample.Attach(pOutSample);
        m_curOutMediaSample->SetActualDataLength(0);
    }else
    {
        // Update Stop times if any
        REFERENCE_TIME tStart, tStop;
        if(pSample->GetTime(&tStart, &tStop) == S_OK)
        {
            REFERENCE_TIME cStart, cStop;
            hr = m_curOutMediaSample->GetTime(&cStart, &cStop);
            if(hr == S_OK || hr == VFW_S_NO_STOP_TIME)
            {
                m_curOutMediaSample->SetTime(&cStart, &tStop);
            }else
            {
                m_curOutMediaSample->SetTime(NULL, &tStop);
            }
        }
    }
    
    // Populate output buffer data
    attachToMediaSample(m_curOutMediaSample, buffersState);

    while(inDataSize)
    {
        if(m_frameBuffer != NULL)
        {
            if(m_frameBufferUsed < m_frameBufferSize)
            {
                DWORD fillSize = min(m_frameBufferSize - m_frameBufferUsed, inDataSize);
                memcpy(&((*m_frameBuffer)[m_frameBufferUsed]), inData, fillSize);
                inData += fillSize;
                m_frameBufferUsed += fillSize;
                inDataSize -= fillSize;
            }

            buffersState.inData = *m_frameBuffer;
            buffersState.inDataSize = m_frameBufferUsed;
        }else
        {
            buffersState.inData = inData;
            buffersState.inDataSize = inDataSize;
            inDataSize = 0;
        }

        HRESULT frameProcessResult;
        do 
        {
            if(m_deliveryThreshold != 0 && (m_curOutMediaSample->GetSize() - buffersState.outBufSize) >= m_deliveryThreshold)
            {
                hr = deliverOutSampleAndContinue(pSample, buffersState);

                if(FAILED(hr))
                    return hr;
            }
#if ADECODE_PERF > 0
            LARGE_INTEGER startTime;
            bool measureFailed = false;
            if(!QueryPerformanceCounter(&startTime))
            {
                filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() QueryPerformanceCounter() failed at start\r\n");
                measureFailed = true;
            }
#endif
            frameProcessResult = decodeOneFrame(buffersState, discontinuity);
            discontinuity = false; // Once only.
#if ADECODE_PERF > 0
            if((frameProcessResult == S_OK || frameProcessResult == VFW_E_BUFFER_OVERFLOW) && !measureFailed) // Take into account good frames only.
            {
                LARGE_INTEGER endTime;
                if(!QueryPerformanceCounter(&endTime))
                {
                    filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() QueryPerformanceCounter() failed at and\r\n");
                }else
                {
                    ++m_totalFramesProcessed;
                    LONGLONG processValue = endTime.QuadPart - startTime.QuadPart;
                    if(m_dbgAvgFrameProcessTime && processValue * 10 > (m_dbgAvgFrameProcessTime * 12)) // 20% longer than average
                        ++m_framesTookLongerThanAvg;

                    if(m_dbgAvgFrameProcessTime && m_dbgMaxFrameProcessTime < processValue)
                        m_dbgMaxFrameProcessTime = processValue;

                    if(m_dbgAvgFrameProcessTime == 0)
                        m_dbgAvgFrameProcessTime = processValue;
                    else
                    {
                        m_dbgAvgFrameProcessTime = (m_dbgAvgFrameProcessTime + processValue) / 2;
                    }
                }
            }
#endif
        }while(frameProcessResult == S_OK && buffersState.inDataSize);

        if(frameProcessResult == E_UNEXPECTED)
        {
            // Something definitely went REALLY wrong! Let the system know.
            filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() decodeOneFrame() == E_UNEXPECTED. Let the system known and expect no further calls.\r\n");
            m_frameBufferUsed = 0;
            m_curOutMediaSample.Release();
            return E_UNEXPECTED;
        }

        if(frameProcessResult == E_FAIL)
        {
            filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() decodeOneFrame() == E_FAIL. Possible causes - missed discontinuity or decode failure. Drop whole sample.\r\n");
            // Reset frame buffer 
            m_frameBufferUsed = 0;
            // Deliver data if any
            return deliverAndReleaseOutSample(buffersState);
        }

        if(m_frameBuffer != NULL)
        {
            if(buffersState.inDataSize)
            {
                if(buffersState.inDataSize == m_frameBufferUsed)
                {
                    if(m_frameBufferUsed == m_frameBufferSize)
                    {
                        // Frame buffer is full and no data has been picked at all. So frame data most likely is bad, Discard it.
                        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() Bad frame data detected. Discarding it.\r\n");
                        m_frameBufferUsed = 0;
                        continue;
                    }
                }else
                {
                    memcpy(*m_frameBuffer, &((*m_frameBuffer)[m_frameBufferUsed - buffersState.inDataSize]), buffersState.inDataSize);
                }
            }
            m_frameBufferUsed = buffersState.inDataSize;
        }
        
        if(frameProcessResult == VFW_E_BUFFER_OVERFLOW)
        {
            filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::Receive() decodeOneFrame() == VFW_E_BUFFER_OVERFLOW. No more output space!\r\n");
            // Deliver data and get new output sample.
            if(m_curOutMediaSample->GetSize() == buffersState.outBufSize)
            {
                filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() decodeOneFrame() == VFW_E_BUFFER_OVERFLOW and m_curOutMediaSample->GetSize() == buffersState.outBufSize. How has this happend!!!! Stop!\r\n");
                m_curOutMediaSample.Release();
                return E_UNEXPECTED;
            }
            
            hr = deliverOutSampleAndContinue(pSample, buffersState);
            if(FAILED(hr))
                return hr;

            continue;
        }else if(frameProcessResult == VFW_E_BUFFER_UNDERFLOW)
        {
            if(m_frameBuffer != NULL)
            {
                // Normal situation. Let frame buffer be populated with the data from current sample if any still available.
            }else
            {
                filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::Receive() decodeOneFrame() == VFW_E_BUFFER_UNDERFLOW and no frame buffering is used. STOP!!!\r\n");
                m_curOutMediaSample.Release();
                return E_UNEXPECTED;
            }
        }        
    }

    if(m_curOutMediaSample)
    {
        DWORD usedBuffer = m_curOutMediaSample->GetSize() - buffersState.outBufSize;
        if(m_deliveryThreshold != 0)
        {
            if(usedBuffer >= m_deliveryThreshold)
            {
                hr = deliverAndReleaseOutSample(buffersState);
                if(FAILED(hr))
                    return hr;

            }else if(usedBuffer == 0)
            {
                // Just created sample. Release it to get correct timestamps at next run.
                m_curOutMediaSample.Release();
            }else
            {
                m_curOutMediaSample->SetActualDataLength(usedBuffer);
            }
        }else
        {
            hr = deliverAndReleaseOutSample(buffersState);
            if(FAILED(hr))
                return hr;
        }
    }
    return S_OK;
}

HRESULT CAudioDecodeFilter::EndOfStream(void)
{
    deliverAndReleaseCurOutSample();
    return CTransformFilter::EndOfStream();
}

HRESULT CAudioDecodeFilter::BeginFlush(void)
{
    if(m_curOutMediaSample)
        m_curOutMediaSample.Release();
    return CTransformFilter::BeginFlush();
}

HRESULT CAudioDecodeFilter::EndFlush(void)
{
    if(m_curOutMediaSample)
        m_curOutMediaSample.Release();
    return CTransformFilter::EndFlush();
}

HRESULT CAudioDecodeFilter::CheckInputType(const CMediaType *mtIn)
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::CheckInputType()\r\n");
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->FormatType() != FORMAT_WaveFormatEx || !checkCopyrightProtection())
        return VFW_E_TYPE_NOT_ACCEPTED;

    WAVEFORMATEX* pInWfx = reinterpret_cast<WAVEFORMATEX*>(mtIn->Format());
    m_curOutputWfx = *pInWfx;
    m_curOutputWfx.cbSize = 0;
    m_curOutputWfx.wFormatTag = WAVE_FORMAT_PCM;

    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::CheckInputType(): return S_OK!\r\n");
    return S_OK;
}

HRESULT CAudioDecodeFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::CheckTransform()\r\n");
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    if(*mtOut->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtOut->Subtype() != MEDIASUBTYPE_PCM && *mtOut->Subtype() != MEDIASUBTYPE_PostProcPCM )
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtOut->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    PWAVEFORMATEX pInWfx = reinterpret_cast<PWAVEFORMATEX>(mtIn->Format());
    PWAVEFORMATEX pOutWfx = reinterpret_cast<PWAVEFORMATEX>(mtOut->Format());

    if(pInWfx->nSamplesPerSec != pOutWfx->nSamplesPerSec)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(m_curOutputWfx.nChannels != pOutWfx->nChannels)
        return VFW_E_TYPE_NOT_ACCEPTED;
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::CheckTransform(): return S_OK!\r\n");
    return S_OK;
}

HRESULT CAudioDecodeFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::DecideBufferSize()\r\n");
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    OutBufferDesc outBufferDesc = getOutBufferDesc();
    if(outBufferDesc.alignment == 0)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() outBufferDesc.alignment == 0 - mis-configuration!!!\r\n");
        return E_UNEXPECTED;
    }

    if(outBufferDesc.buffersCount == 0)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() outBufferDesc.buffersCount == 0 - mis-configuration!!!\r\n");
        return E_UNEXPECTED;
    }

    if(outBufferDesc.bufferSize == 0)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() outBufferDesc.bufferSize == 0 - mis-configuration!!!\r\n");
        return E_UNEXPECTED;
    }

    size_t bufferSize = outBufferDesc.bufferSize;

    // Make buffer frame aligned.
    m_outBufferAlignment = outBufferDesc.alignment;
    if(!m_outBufferAlignment)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() m_outBufferAlignment == 0 - mis-configuration!!!\r\n");
        return E_UNEXPECTED;
    }

    /*if( bufferSize % m_outBufferAlignment > 0)
        bufferSize += m_outBufferAlignment - (bufferSize % m_outBufferAlignment);*/

    m_deliveryThreshold = outBufferDesc.deliveryThreshold;

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = outBufferDesc.buffersCount;
    if(!pProperties->cBuffers)
    {
        filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() getOutBuffersCount() == 0 - mis-configuration!!!\r\n");
        return E_UNEXPECTED;
    }

    pProperties->cbBuffer = bufferSize;
    ASSERT(pProperties->cbBuffer);

    pProperties->cbAlign = m_outBufferAlignment;
    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr))
    {
        return hr;
    }

    ASSERT( Actual.cBuffers == pProperties->cBuffers );

    if (pProperties->cBuffers > Actual.cBuffers
        || pProperties->cbBuffer > Actual.cbBuffer
        || pProperties->cbAlign > Actual.cbAlign
        ) 
    {
        return E_FAIL;
    }
    filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::DecideBufferSize() return S_OK \t\r\n size=%d (requested = %d with delivery treshold = %d) \t\r\n count=%d \r\n", Actual.cbBuffer, pProperties->cbBuffer, m_deliveryThreshold, Actual.cBuffers);
    return S_OK;
}

HRESULT CAudioDecodeFilter::SetOutputMediaType(const CMediaType *pmt)
{
    filterDebugPrintf(ADECODE_TRACE, L"CAudioDecodeFilter::SetOutputMediaType()\r\n");
    if(*pmt->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    // Setup frame buffer here since it is the last stage and all data is available at this point.
    m_frameBufferSize = getFrameBufferSize();
    if(m_frameBufferSize > 0)
    {
        m_frameBuffer = new BYTE[m_frameBufferSize];
        if(m_frameBuffer == NULL)
        {
            filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::CheckInputType(): new BYTE[m_frameBufferSize] failed - out of memory!\r\n");
            return E_OUTOFMEMORY;
        }
    }else
    {
        m_frameBuffer = NULL;
    }

    m_curOutputWfx = *reinterpret_cast<PWAVEFORMATEX>(pmt->Format());
    filterDebugPrintf(ADECODE_DBG, L"CAudioDecodeFilter::SetOutputMediaType() nChannels=%d, SubType=%s\r\n", static_cast<unsigned int>(m_curOutputWfx.nChannels), *pmt->Subtype() == MEDIASUBTYPE_PostProcPCM ? L"MEDIASUBTYPE_PostProcPCM" : L"MEDIASUBTYPE_PCM");
    return S_OK;
}
