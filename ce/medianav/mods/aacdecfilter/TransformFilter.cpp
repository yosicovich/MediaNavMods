//
// TransformFilter.cpp
// 

#include "TransformFilter.h"
#include <audshow.h>

// --- registration tables ----------------

// filter registration information. 
const AMOVIESETUP_MEDIATYPE c_sudPinTypes =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN ACCDecoderFilter::m_sudPins[] =
{
    { L"Input",             // Pins string name
    FALSE,                // Is it rendered
    FALSE,                // Is it an output
    FALSE,                // Are we allowed none
    FALSE,                // And allowed many
    &CLSID_NULL,          // Connects to filter
    NULL,                 // Connects to pin
    1,                    // Number of types
    &c_sudPinTypes          // Pin information
    },
    { L"Output",            // Pins string name
    FALSE,                // Is it rendered
    TRUE,                 // Is it an output
    FALSE,                // Are we allowed none
    FALSE,                // And allowed many
    &CLSID_NULL,          // Connects to filter
    NULL,                 // Connects to pin
    1,                    // Number of types
    &c_sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER 
ACCDecoderFilter::m_sudFilter = 
{
    &__uuidof(ACCDecoderFilter),  // filter clsid
    L"AAC decoder",   // filter name
    MERIT_NORMAL,                   // ie default for auto graph building
    3,                              // count of registered pins
    ACCDecoderFilter::m_sudPins    // list of pins to register
};

// Configuration
static const int cOutBufferTimeS = 1; // Buffer length in seconds
static const int cSampleSizePerChannel = 2; //sizeof(short); 
static const int cAACOneChannelOutFrameSize = 1024 * cSampleSizePerChannel;//256(samples per channel) * size of sample
static const int cMaxChannels = 6; // make this higher to support files with more channels

// ---- construction/destruction and COM support -------------

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
ACCDecoderFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new ACCDecoderFilter(pUnk, phr, FAAD_MIN_STREAMSIZE * cMaxChannels);// buffer to hold one frame at maximal possible bitrate.
}

ACCDecoderFilter::ACCDecoderFilter(LPUNKNOWN pUnk, HRESULT* phr, long frameBufferSize)
:CTransformFilter(NAME("ACCDecoderFilter"), pUnk, *m_sudFilter.clsID)
,m_hDecoder(NULL)
,m_decoderReady(false)
,m_streamingMode(true)
,m_maxOutputBufUsed(0)
,m_frameBufferSize(frameBufferSize)
,m_frameBuffer(NULL)
,m_frameBufferUsed(0)
,m_frameBufferPos(0)
#if DBG > 0
,m_dbgMaxFrameProcessTime(0)
,m_dbgAvgFrameProcessTime(0)
#endif
{
    if(!m_frameBufferSize)
    {
        debugPrintf(DBG, L"AACDecoderFilter::AACDecoderFilter() m_frameBufferSize == 0. Wrong configuration?\r\n");
        return;
    }

    m_frameBuffer = new BYTE[m_frameBufferSize]; 
    if(!m_frameBuffer)
    {
        debugPrintf(DBG, L"AACDecoderFilter::AACDecoderFilter() frame buffer allocation failed! Out of memory!\r\n");
        return;
    }

    debugPrintf(DBG, L"AACDecoderFilter::AACDecoderFilter() Init codec\r\n");
    
    m_hDecoder = NeAACDecOpen();
    if(!m_hDecoder)
    {
        debugPrintf(DBG, L"AACDecoderFilter::AACDecoderFilter() NeAACDecOpen() failed\r\n");
        return;
    }

    // Configure
    NeAACDecConfigurationPtr pCfg = NeAACDecGetCurrentConfiguration(m_hDecoder);
    pCfg->defObjectType = LC;
    pCfg->downMatrix = 1; // 5.1 -> 2
    pCfg->outputFormat = FAAD_FMT_16BIT;
    if(!NeAACDecSetConfiguration(m_hDecoder, pCfg))
    {
        debugPrintf(DBG, L"AACDecoderFilter::AACDecoderFilter() NeAACDecSetConfiguration() failed\r\n");
        NeAACDecClose(m_hDecoder);
        m_hDecoder = NULL;
    }
}

ACCDecoderFilter::~ACCDecoderFilter()
{
#if DBG > 0
    LARGE_INTEGER freq;
    if(QueryPerformanceFrequency(&freq))
    {
        m_dbgAvgFrameProcessTime *= 1000000;
        m_dbgAvgFrameProcessTime /= freq.QuadPart;

        m_dbgMaxFrameProcessTime *= 1000000;
        m_dbgMaxFrameProcessTime /= freq.QuadPart;
    }else
    {
        debugPrintf(DBG, L"AACDecoderFilter::~AACDecoderFilter() QueryPerformanceFrequency() failed\r\n");
    }
    debugPrintf(DBG, L"AACDecoderFilter::~AACDecoderFilter() m_maxOutputBufUsed=%d, avarage frame processing time = %I64d us, max frame processing time = %I64d us\r\n", m_maxOutputBufUsed, m_dbgAvgFrameProcessTime, m_dbgMaxFrameProcessTime);
#else
    debugPrintf(DBG, L"AACDecoderFilter::~AACDecoderFilter() m_maxOutputBufUsed=%d\r\n", m_maxOutputBufUsed);
#endif
    if(m_hDecoder)
    {
        NeAACDecClose(m_hDecoder);
    }

    if(m_frameBuffer)
        delete[] m_frameBuffer;
}

HRESULT ACCDecoderFilter::decodeOneFrame(TTransformBuffersState& buffersState, bool isDiscontinuity)
{
    debugDump(DBG_TRACE, buffersState.inData, min(buffersState.inDataSize, 32));

    if(m_streamingMode)
    {
        // Streaming mode. The decoder is not initialized yet and now data alignment can be expected. So work on FAAD_MIN_STREAMSIZE basis.
        // Most likely .aac raw file is about to be played. Do on demand initialization.
        long errCode;
        while(!m_decoderReady && buffersState.inDataSize >= FAAD_MIN_STREAMSIZE)
        {
            unsigned long sampleRate;
            unsigned char channels;
            errCode = NeAACDecInit(m_hDecoder, buffersState.inData, buffersState.inDataSize, &sampleRate, &channels);
            if(errCode >= 0)
            {
                buffersState.inDataSize -= errCode;
                buffersState.inData += errCode;
                debugPrintf(DBG, L"AACDecoderFilter::decodeOneFrame(): NeAACDecInit() on demand initialization complete: SampleRate = %d, channels = %u.\r\n", sampleRate, static_cast<unsigned int>(channels));
                if(sampleRate != m_curOutputWfx.nSamplesPerSec)
                {
                    debugPrintf(DBG, L"AACDecoderFilter::decodeOneFrame(): NeAACDecInit() sampleRate(%u) != m_curOutputWfx.nSamplesPerSec(%u)!!!\r\n", sampleRate, m_curOutputWfx.nSamplesPerSec);
                    // Try to break connection with output pin to re-negotiate about the output format.
                    m_curOutputWfx.nSamplesPerSec = sampleRate;
                    m_pOutput->BreakConnect();
                }
                m_decoderReady = true;
                isDiscontinuity = false;
            }else
            {
                ++buffersState.inData;
                --buffersState.inDataSize;
            }
        }
        if(!m_decoderReady)
        {
            debugPrintf(DBG, L"AACDecoderFilter::decodeOneFrame(): NeAACDecInit() failed: %d!\r\n", errCode);
        }

        if(buffersState.inDataSize < FAAD_MIN_STREAMSIZE)
        {
            return VFW_E_BUFFER_UNDERFLOW;
        }
    }

    if(!m_decoderReady)
    {
        debugPrintf(DBG, L"AACDecoderFilter::decodeOneFrame(): The decoder is not ready!!!\r\n");
        return E_UNEXPECTED;
    }
    
    if(isDiscontinuity)
    {
        NeAACDecPostSeekReset(m_hDecoder, 0);
    }

    NeAACDecFrameInfo hInfo;
    NeAACDecDecode2(m_hDecoder, &hInfo, buffersState.inData, buffersState.inDataSize, reinterpret_cast<void **>(&buffersState.outBuf), buffersState.outBufSize);
    buffersState.inDataSize -= hInfo.bytesconsumed;
    buffersState.inData += hInfo.bytesconsumed;
    if(hInfo.samples)
    {
        DWORD bytesProduced = hInfo.samples * cSampleSizePerChannel;
        buffersState.outBuf += bytesProduced;
        buffersState.outBufSize -= bytesProduced;
    }
    
    if(hInfo.error)
    {
        switch(hInfo.error)
        {
        case 14:
            return VFW_E_BUFFER_UNDERFLOW;
        case 27:
            return VFW_E_BUFFER_OVERFLOW;
        default:
            debugPrintf(DBG, L"AACDecoderFilter::decodeOneFrame(): NeAACDecDecode2() failed: %S!\r\n", NeAACDecGetErrorMessage(hInfo.error));
            m_decoderReady = false;
            return E_FAIL;
        }
    }
    return S_OK;
}

HRESULT ACCDecoderFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    debugPrintf(DBG_TRACE, L"AACDecoderFilter::Transform()\r\n");
    TTransformBuffersState buffersState;
    BYTE* inData;
    long inDataSize;
    pIn->GetPointer(&inData);
    inDataSize = pIn->GetActualDataLength();
    bool discontinuity = false;
    // Reset frame buffer in case of seeking.
    if(pIn->IsDiscontinuity() == S_OK)
    {
        debugPrintf(DBG, L"AACDecoderFilter::Transform() discontinuity detected! Reset frame buffer.\r\n");
        m_frameBufferUsed = 0;
        discontinuity = true;
    }

    pOut->GetPointer(&buffersState.outBuf);
    buffersState.outBufSize = pOut->GetSize();

    while(inDataSize)
    {
        if(buffersState.outBufSize < m_oneFrameOutSize)
        {
            debugPrintf(DBG, L"AACDecoderFilter::Transform() buffersState.outBufSize = %d < m_oneFrameOutSize = %d, possible causes - missed discontinuity or decode failure. Drop whole sample.\r\n", buffersState.outBufSize, m_oneFrameOutSize);
            break;
        }

        if(m_frameBufferUsed < m_frameBufferSize)
        {
            long fillSize = min(m_frameBufferSize - m_frameBufferUsed, inDataSize);
            memcpy(&(m_frameBuffer[m_frameBufferUsed]), inData, fillSize);
            inData += fillSize;
            m_frameBufferUsed += fillSize;
            inDataSize -= fillSize;
        }

        buffersState.inData = m_frameBuffer;
        buffersState.inDataSize = m_frameBufferUsed;

        HRESULT frameProcessResult;
        do 
        {
#if DBG > 0
            LARGE_INTEGER startTime;
            bool measureFailed = false;
            if(!QueryPerformanceCounter(&startTime))
            {
                debugPrintf(DBG, L"AACDecoderFilter::Transform() QueryPerformanceCounter() failed at start\r\n");
                measureFailed = true;
            }
#endif
            frameProcessResult = decodeOneFrame(buffersState, discontinuity);
            discontinuity = false; // Once only.
#if DBG > 0
            if(!measureFailed)
            {
                LARGE_INTEGER endTime;
                if(!QueryPerformanceCounter(&endTime))
                {
                    debugPrintf(DBG, L"AACDecoderFilter::Transform() QueryPerformanceCounter() failed at and\r\n");
                }else
                {
                    LONGLONG processValue = endTime.QuadPart - startTime.QuadPart;
                    if(m_dbgAvgFrameProcessTime == 0)
                        m_dbgAvgFrameProcessTime = processValue;
                    else
                    {
                        m_dbgAvgFrameProcessTime = (m_dbgAvgFrameProcessTime + processValue) / 2;
                    }

                    if(m_dbgMaxFrameProcessTime < processValue)
                        m_dbgMaxFrameProcessTime = processValue;
                }
            }
#endif
        }while(frameProcessResult == S_OK && buffersState.inDataSize);
        
        if(frameProcessResult == E_UNEXPECTED)
        {
            // Something definitely went REALLY wrong! Let the system known.
            debugPrintf(DBG, L"AACDecoderFilter::Transform() decodeOneFrame() == E_UNEXPECTED. Let the system known and expect no further calls.\r\n");
            m_frameBufferUsed = 0;
            return E_UNEXPECTED;
        }

        if(frameProcessResult == E_FAIL)
        {
            debugPrintf(DBG, L"AACDecoderFilter::Transform() decodeOneFrame() == E_FAIL. Possible causes - missed discontinuity or decode failure. Drop whole sample.\r\n");
            // Reset frame buffer 
            m_frameBufferUsed = 0;
            break;
        }

        if(buffersState.inDataSize)
        {
            if(buffersState.inDataSize == m_frameBufferUsed)
            {
                if(buffersState.outBufSize == pOut->GetSize())
                {
                    debugPrintf(DBG, L"AACDecoderFilter::Transform() Bad frame data detected. Discarding it.\r\n");
                    m_frameBufferUsed = 0;
                    continue;
                }
            }else
            {
                memcpy(m_frameBuffer, &(m_frameBuffer[m_frameBufferUsed - buffersState.inDataSize]), buffersState.inDataSize);
            }
        }
        m_frameBufferUsed = buffersState.inDataSize;

        if(frameProcessResult == VFW_E_BUFFER_OVERFLOW)
        {
            debugPrintf(DBG, L"AACDecoderFilter::Transform() decodeOneFrame() == VFW_E_BUFFER_OVERFLOW. No more output space!\r\n");
            break;
        }else if(frameProcessResult == VFW_E_BUFFER_UNDERFLOW)
        {
            // Normal situation. Let frame buffer be populated with the data from current sample if any still available.
        }
    }

    long usedBuffer = pOut->GetSize() - buffersState.outBufSize;
    if(m_maxOutputBufUsed < usedBuffer)
        m_maxOutputBufUsed = usedBuffer;
    pOut->SetActualDataLength(usedBuffer);

    debugPrintf(DBG_TRACE, L"AACDecoderFilter::Transform() OK\r\n");
    return S_OK;
}

HRESULT ACCDecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    if(!m_hDecoder)
    {
        debugPrintf(DBG, L"AACDecoderFilter::CheckInputType(): codec is not initialized!\r\n");
        return E_UNEXPECTED;
    }
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->Subtype() != MEDIASUBTYPE_AAC && *mtIn->Subtype() != MEDIASUBTYPE_AAC_AUDIO)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    WAVEFORMATEX* pInWfx = reinterpret_cast<WAVEFORMATEX*>(mtIn->Format());
    m_curOutputWfx = *pInWfx;

    if(pInWfx->cbSize)
    {
        // Try to initialize with decoder specific data
        DWORD sampleRate;
        BYTE channels;
        int errCode = NeAACDecInit2(m_hDecoder, reinterpret_cast<BYTE*>(pInWfx) + sizeof(WAVEFORMATEX), pInWfx->cbSize, &sampleRate, &channels);
        if(!errCode)
        {
            m_decoderReady = true;
            m_streamingMode = false;
            m_curOutputWfx.nSamplesPerSec = sampleRate;
            m_curOutputWfx.nChannels = channels;
        }else
        {
            debugPrintf(DBG, L"AACDecoderFilter::CheckInputType(): NeAACDecInit2() failed: %d. Stay in streaming mode!\r\n", errCode);
        }
    }
    
    m_curOutputWfx.cbSize = 0;
    m_curOutputWfx.wFormatTag = WAVE_FORMAT_PCM;
    
    // Nothing more than STEREO is supported
    if(m_curOutputWfx.nChannels > 2)
        m_curOutputWfx.nChannels = 2;

    m_mediaType.InitMediaType();
    m_mediaType.SetType(&MEDIATYPE_Audio);
    m_mediaType.SetFormatType(&FORMAT_WaveFormatEx);
    m_mediaType.SetSubtype(&MEDIASUBTYPE_PCM);
    debugPrintf(DBG, L"AACDecoderFilter::CheckInputType(): OK!\r\n");
    return S_OK;
}

HRESULT ACCDecoderFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->Subtype() != MEDIASUBTYPE_AAC && *mtIn->Subtype() != MEDIASUBTYPE_AAC_AUDIO)
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
    return S_OK;
}

HRESULT ACCDecoderFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    debugPrintf(DBG, L"AACDecoderFilter::DecideBufferSize()\r\n");
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    size_t bufferSize = m_curOutputWfx.nAvgBytesPerSec * cOutBufferTimeS;

    // Make buffer frame aligned.
    if( bufferSize % m_oneFrameOutSize > 0)
        bufferSize += m_oneFrameOutSize - (bufferSize % m_oneFrameOutSize);

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 4; //1 working and one in advance
    pProperties->cbBuffer = bufferSize;
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr))
    {
        return hr;
    }

    ASSERT( Actual.cBuffers == 4 );

    if (pProperties->cBuffers > Actual.cBuffers ||
        pProperties->cbBuffer > Actual.cbBuffer) 
    {
            return E_FAIL;
    }
    debugPrintf(DBG, L"AACDecoderFilter::DecideBufferSize() OK size=%d, count=%d \r\n", Actual.cbBuffer, Actual.cBuffers);
    return S_OK;
}

HRESULT ACCDecoderFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    if(iPosition >= 2)
        return VFW_S_NO_MORE_ITEMS;
    m_curOutputWfx.wBitsPerSample = 8 * cSampleSizePerChannel;
    m_curOutputWfx.nBlockAlign = m_curOutputWfx.nChannels * cSampleSizePerChannel;
    m_curOutputWfx.nAvgBytesPerSec = m_curOutputWfx.nBlockAlign * m_curOutputWfx.nSamplesPerSec;
    if(iPosition % 2 == 0)
        m_mediaType.SetSubtype(&MEDIASUBTYPE_PostProcPCM);
    else
        m_mediaType.SetSubtype(&MEDIASUBTYPE_PCM);

    if(!m_mediaType.SetFormat(reinterpret_cast<BYTE *>(&m_curOutputWfx), sizeof(WAVEFORMATEX)))
        return E_FAIL;
    *pMediaType = m_mediaType;
    debugPrintf(DBG, L"AACDecoderFilter::GetMediaType() iPosition=%d, nChannels=%d, SubType=%s\r\n", iPosition, static_cast<unsigned int>(m_curOutputWfx.nChannels), iPosition % 2 == 0 ? L"MEDIASUBTYPE_PostProcPCM": L"MEDIASUBTYPE_PCM");
    return S_OK;
}

HRESULT ACCDecoderFilter::SetOutputMediaType(const CMediaType *pmt)
{
    debugPrintf(DBG, L"AACDecoderFilter::SetOutputMediaType()\r\n");
    if(*pmt->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    m_curOutputWfx = *reinterpret_cast<PWAVEFORMATEX>(pmt->Format());
    m_oneFrameOutSize = cAACOneChannelOutFrameSize * m_curOutputWfx.nChannels;

    debugPrintf(DBG, L"AACDecoderFilter::SetOutputMediaType() nChannels=%d, SubType=%s\r\n", static_cast<unsigned int>(m_curOutputWfx.nChannels), *pmt->Subtype() == MEDIASUBTYPE_PostProcPCM ? L"MEDIASUBTYPE_PostProcPCM" : L"MEDIASUBTYPE_PCM");
    return S_OK;
}

