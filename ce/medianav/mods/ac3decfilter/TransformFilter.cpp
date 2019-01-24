//
// TransformFilter.cpp
// 

#include "TransformFilter.h"
#include <audshow.h>
#include <libao/audio_out_internal.h>

// --- registration tables ----------------

// filter registration information. 
const AMOVIESETUP_MEDIATYPE c_sudPinTypes =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN AC3DecoderFilter::m_sudPins[] =
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
AC3DecoderFilter::m_sudFilter = 
{
    &__uuidof(AC3DecoderFilter),  // filter clsid
    L"AC3 decoder",   // filter name
    MERIT_NORMAL,                   // ie default for auto graph building
    3,                              // count of registered pins
    AC3DecoderFilter::m_sudPins    // list of pins to register
};

// Configuration
static const int cOutBufferTimeS = 1; // Buffer length in seconds
static const int cSampleSizePerChannel = 2; //sizeof(short); 
static const int cAC3OneChannelOutBlockSize = 256 * cSampleSizePerChannel;//256(samples per channel) * size of sample

// ---- construction/destruction and COM support -------------

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
AC3DecoderFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new AC3DecoderFilter(pUnk, phr, 3840);// buffer to hold one frame at maximal possible bitrate.
}

AC3DecoderFilter::AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr, long frameBufferSize)
:CTransformFilter(NAME("AC3DecoderFilter"), pUnk, *m_sudFilter.clsID)
,m_a52State(NULL)
,m_maxChannels(0)
,m_maxOutputBufUsed(0)
,m_a52OneOutBlockSize(0)
,m_frameBufferSize(frameBufferSize)
,m_frameBuffer(NULL)
,m_frameBufferUsed(0)
,m_frameBufferPos(0)
,m_a52OutputSpeakersConfig(A52_CHANNEL)
#if DBG > 0
,m_dbgMaxFrameProcessTime(0)
,m_dbgAvgFrameProcessTime(0)
#endif
{
    if(!m_frameBufferSize)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() m_frameBufferSize == 0. Wrong configuration?\r\n");
        return;
    }

    m_frameBuffer = new BYTE[m_frameBufferSize]; 
    if(!m_frameBuffer)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() frame buffer allocation failed! Out of memory!\r\n");
        return;
    }

    debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() Init codec\r\n");
    m_a52State = a52_init();
    if(!m_a52State)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() a52_init() failed\r\n");
        return;
    }
}

AC3DecoderFilter::~AC3DecoderFilter()
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
        debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() QueryPerformanceFrequency() failed\r\n");
    }
    debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() m_maxOutputBufUsed=%d, avarage frame processing time = %I64d us, max frame processing time = %I64d us\r\n", m_maxOutputBufUsed, m_dbgAvgFrameProcessTime, m_dbgMaxFrameProcessTime);
#else
    debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() m_maxOutputBufUsed=%d\r\n", m_maxOutputBufUsed);
#endif
    if(m_a52State)
    {
        a52_free (m_a52State);
    }

    if(m_frameBuffer)
        delete[] m_frameBuffer;
}

static const int cAC3HeaderSize = 7;
HRESULT AC3DecoderFilter::decodeOneFrame(TTransformBuffersState& buffersState)
{
    int frameDataSize = 0;
    int flags, sampleRate, bitRate;

    while(!frameDataSize && buffersState.inDataSize >= cAC3HeaderSize)
    {
        frameDataSize = a52_syncinfo(buffersState.inData, &flags, &sampleRate, &bitRate);
        debugPrintf(DBG_TRACE, L"AC3DecoderFilter::decodeOneFrame() frameDataSize = %d\r\n", frameDataSize);
        if(!frameDataSize)
        {
            ++buffersState.inData;
            --buffersState.inDataSize;
        }
    }

    if(!frameDataSize)
        return VFW_E_BUFFER_UNDERFLOW;

    if(buffersState.inDataSize < frameDataSize)
    {
        debugPrintf(DBG_TRACE, L"AC3DecoderFilter::decodeOneFrame() inDataSize = %d < frameDataSize = %d\r\n", buffersState.inDataSize, frameDataSize);
        return VFW_E_BUFFER_UNDERFLOW;
    }

    // Here can be some checks against stream encoding changes from original one but who matters?

    if(buffersState.outBufSize < m_oneFrameOutSize)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::decodeOneFrame() outBufSize = %d < m_oneFrameOutSize = %d\r\n", buffersState.outBufSize, m_oneFrameOutSize);
        return VFW_E_BUFFER_OVERFLOW;
    }

    // Override flags with chosen ones to down/up mix as requested.
    flags = m_a52Flags;
    level_t level = m_a52Level;
    sample_t bias = 0; // No biasing is required for fixed point implementation.
    if(a52_frame(m_a52State, buffersState.inData, &flags, &level, bias))
    {
        debugPrintf(DBG, L"AC3DecoderFilter::decodeOneFrame() a52_frame() failed!!!\r\n");
        return E_FAIL;
    }

    for ( int i = 0; i < 6; i++) 
    {
        if (a52_block(m_a52State))
        {
            debugPrintf(DBG, L"AC3DecoderFilter::decodeOneFrame() a52_block() failed!!!\r\n");
            return E_FAIL;
        }
        sample2s16_multi(a52_samples(m_a52State), (int16_t *)buffersState.outBuf, m_a52OutputSpeakersConfig);
        buffersState.outBufSize -= m_a52OneOutBlockSize;
        buffersState.outBuf += m_a52OneOutBlockSize;
    }
    buffersState.inDataSize -= frameDataSize;
    buffersState.inData += frameDataSize;
    return S_OK;
}

HRESULT AC3DecoderFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::Transform()\r\n");
    TTransformBuffersState buffersState;
    BYTE* inData;
    long inDataSize;
    pIn->GetPointer(&inData);
    inDataSize = pIn->GetActualDataLength();
    /*// Reset frame buffer in case of seeking.
    if(pIn->IsDiscontinuity())
    {
        debugPrintf(DBG_TRACE, L"AC3DecoderFilter::Transform() discontinuity detected! Reset frame buffer.\r\n");
        m_frameBufferUsed = 0;
    }*/

    pOut->GetPointer(&buffersState.outBuf);
    buffersState.outBufSize = pOut->GetSize();

    while(inDataSize)
    {
        if(buffersState.outBufSize < m_oneFrameOutSize)
        {
            debugPrintf(DBG, L"AC3DecoderFilter::Transform() buffersState.outBufSize = %d < m_oneFrameOutSize = %d, possible causes - missed discontinuity or decode failure. Drop whole sample.\r\n", buffersState.outBufSize, m_oneFrameOutSize);
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
                debugPrintf(DBG, L"AC3DecoderFilter::Transform() QueryPerformanceCounter() failed at start\r\n");
                measureFailed = true;
            }
#endif
            frameProcessResult = decodeOneFrame(buffersState);
#if DBG > 0
            if(!measureFailed)
            {
                LARGE_INTEGER endTime;
                if(!QueryPerformanceCounter(&endTime))
                {
                    debugPrintf(DBG, L"AC3DecoderFilter::Transform() QueryPerformanceCounter() failed at and\r\n");
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
        
        if(frameProcessResult == E_FAIL)
        {
            debugPrintf(DBG, L"AC3DecoderFilter::Transform() decodeOneFrame() == E_FAIL. Possible causes - missed discontinuity or decode failure. Drop whole sample.\r\n");
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
                    debugPrintf(DBG, L"AC3DecoderFilter::Transform() Bad frame data detected. Discarding it.\r\n");
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
            debugPrintf(DBG, L"AC3DecoderFilter::Transform() decodeOneFrame() == VFW_E_BUFFER_OVERFLOW. No more output space!\r\n");
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

    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::Transform() OK\r\n");
    return S_OK;
}

HRESULT AC3DecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    if(!m_a52State)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::CheckInputType(): codec is not initialized!\r\n");
        return E_UNEXPECTED;
    }
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->Subtype() != MEDIASUBTYPE_DVM && *mtIn->Subtype() != MEDIASUBTYPE_DOLBY_AC3)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->FormatType() != FORMAT_DolbyAC3 && *mtIn->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;
    m_curOutputWfx = *reinterpret_cast<PWAVEFORMATEX>(mtIn->Format());
    
    m_curOutputWfx.cbSize = 0;
    m_curOutputWfx.wFormatTag = WAVE_FORMAT_PCM;
    m_maxChannels = m_curOutputWfx.nChannels;
    
    // Nothing more than STEREO is supported
    if(m_maxChannels > 2)
        m_maxChannels = 2;

    m_mediaType.InitMediaType();
    m_mediaType.SetType(&MEDIATYPE_Audio);
    m_mediaType.SetFormatType(&FORMAT_WaveFormatEx);
    m_mediaType.SetSubtype(&MEDIASUBTYPE_PCM);
    debugPrintf(DBG, L"AC3DecoderFilter::CheckInputType(): OK!\r\n");
    return S_OK;
}

HRESULT AC3DecoderFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if(*mtIn->Type() != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->Subtype() != MEDIASUBTYPE_DVM && *mtIn->Subtype() != MEDIASUBTYPE_DOLBY_AC3)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtIn->FormatType() != FORMAT_DolbyAC3 && *mtIn->FormatType() != FORMAT_WaveFormatEx)
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
    if(m_maxChannels < pOutWfx->nChannels)
        return VFW_E_TYPE_NOT_ACCEPTED;
    return S_OK;
}

HRESULT AC3DecoderFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    debugPrintf(DBG, L"AC3DecoderFilter::DecideBufferSize()\r\n");
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    size_t bufferSize = m_curOutputWfx.nAvgBytesPerSec * cOutBufferTimeS;

    // Make buffer frame aligned.
    if( bufferSize % m_oneFrameOutSize > 0)
        bufferSize += m_oneFrameOutSize - (bufferSize % m_oneFrameOutSize);

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 2; //1 working and one in advance
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

    ASSERT( Actual.cBuffers == 2 );

    if (pProperties->cBuffers > Actual.cBuffers ||
        pProperties->cbBuffer > Actual.cbBuffer) 
    {
            return E_FAIL;
    }
    debugPrintf(DBG, L"AC3DecoderFilter::DecideBufferSize() OK size=%d, count=%d \r\n", Actual.cbBuffer, Actual.cBuffers);
    return S_OK;
}

HRESULT AC3DecoderFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    if(m_maxChannels <= iPosition / 2)
        return VFW_S_NO_MORE_ITEMS;
    m_curOutputWfx.nChannels = m_maxChannels - iPosition / 2;
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
    debugPrintf(DBG, L"AC3DecoderFilter::GetMediaType() iPosition=%d, nChannels=%d, SubType=%s\r\n", iPosition, static_cast<unsigned int>(m_curOutputWfx.nChannels), iPosition % 2 == 0 ? L"MEDIASUBTYPE_PostProcPCM": L"MEDIASUBTYPE_PCM");
    return S_OK;
}

HRESULT AC3DecoderFilter::SetOutputMediaType(const CMediaType *pmt)
{
    debugPrintf(DBG, L"AC3DecoderFilter::SetOutputMediaType()\r\n");
    if(*pmt->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    m_curOutputWfx = *reinterpret_cast<PWAVEFORMATEX>(pmt->Format());

    m_a52Flags = A52_ADJUST_LEVEL;

    switch(m_curOutputWfx.nChannels)
    {
    case 1:
        m_a52OutputSpeakersConfig = A52_MONO;
        break;
    case 2:
        m_a52OutputSpeakersConfig = A52_STEREO;
        break;
    default:
        return E_FAIL;
        break;
    }
    m_a52Flags |= m_a52OutputSpeakersConfig;

    // Gain level 1 means no gain.
    m_a52Level = a52_to_level(1);

    m_a52OneOutBlockSize = cAC3OneChannelOutBlockSize * m_curOutputWfx.nChannels;
    m_oneFrameOutSize = m_a52OneOutBlockSize * 6; // AC3 has 6 blocks per frame.

    debugPrintf(DBG, L"AC3DecoderFilter::SetOutputMediaType() nChannels=%d, SubType=%s\r\n", static_cast<unsigned int>(m_curOutputWfx.nChannels), *pmt->Subtype() == MEDIASUBTYPE_PostProcPCM ? L"MEDIASUBTYPE_PostProcPCM" : L"MEDIASUBTYPE_PCM");
    return S_OK;
}

