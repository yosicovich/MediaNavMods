//
// TransformFilter.cpp
// 

#include "TransformFilter.h"
#include <audshow.h>
#include "cmnMemory.h"

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

// ---- construction/destruction and COM support -------------

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
AC3DecoderFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new AC3DecoderFilter(pUnk, phr);
}

AC3DecoderFilter::AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr)
:CTransformFilter(NAME("AC3DecoderFilter"), pUnk, *m_sudFilter.clsID)
,m_hCodec(NULL)
,m_maxChannels(0)
,m_maxOutputBufUsed(0)
#if DBG > 0
,m_dbgAvgFrameProcessTime(0)
#endif
{
    debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter()\r\n");
    yyGetAC3DecFunc(&m_audioAPI);
    m_cMemOps.Alloc = cmnMemAlloc;
    m_cMemOps.Copy = cmnMemCopy;
    m_cMemOps.Free = cmnMemFree;
    m_cMemOps.Set = cmnMemSet;
    m_cMemOps.Check = cmnMemCheck;

    VO_CODEC_INIT_USERDATA useData;
    useData.memflag = VO_IMF_USERMEMOPERATOR;
    useData.memData = (VO_PTR)(&m_cMemOps);	

    debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() Init codec\r\n");
    int returnCode = m_audioAPI.Init(&m_hCodec, VO_AUDIO_CodingAC3, &useData);
    if(returnCode)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() m_audioAPI.Init() failed: 0x%08X\r\n", returnCode);
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
    }else
    {
        debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() QueryPerformanceFrequency() failed\r\n");
    }
    debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() m_maxOutputBufUsed=%d, avarage frame processing time = %I64d us\r\n", m_maxOutputBufUsed, m_dbgAvgFrameProcessTime);
#else
    debugPrintf(DBG, L"AC3DecoderFilter::~AC3DecoderFilter() m_maxOutputBufUsed=%d\r\n", m_maxOutputBufUsed);
#endif
    if(m_hCodec)
    {
        m_audioAPI.Uninit(m_hCodec);
    }
}

HRESULT AC3DecoderFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::Transform()\r\n");
    VO_CODECBUFFER inData;
    VO_CODECBUFFER outData;
    VO_AUDIO_OUTPUTINFO outFormat;
    pIn->GetPointer(&inData.Buffer);
    inData.Length = pIn->GetActualDataLength();

    pOut->GetPointer(&outData.Buffer);
    size_t bufRemains = pOut->GetSize();
    outData.Length = 0;

    /* decode one amr block */
    int returnCode = m_audioAPI.SetInputData(m_hCodec,&inData);
    if(returnCode)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::Transform(): m_audioAPI.SetInputData() failed: 0x%08X\r\n", returnCode);
        return E_FAIL;
    }
    
    do 
    {
        outData.Buffer = reinterpret_cast<VO_PBYTE>(reinterpret_cast<size_t>(outData.Buffer) + outData.Length);
        bufRemains -=outData.Length;
        outData.Length = bufRemains;
#if DBG > 0
        LARGE_INTEGER startTime;
        bool measureFailed = false;
        if(!QueryPerformanceCounter(&startTime))
        {
            debugPrintf(DBG, L"AC3DecoderFilter::Transform() QueryPerformanceCounter() failed at start\r\n");
            measureFailed = true;
        }
#endif
        returnCode = m_audioAPI.GetOutputData(m_hCodec, &outData, &outFormat);
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
            }
        }
#endif

    } while (!returnCode);
    if(returnCode && returnCode != VO_ERR_INPUT_BUFFER_SMALL)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::Transform(): m_audioAPI.GetOutputData() failed: 0x%08X\r\n", returnCode);
        return E_FAIL;
    }
    long usedBuffer = pOut->GetSize() - bufRemains;
    if(m_maxOutputBufUsed < usedBuffer)
        m_maxOutputBufUsed = usedBuffer;
    pOut->SetActualDataLength(usedBuffer);

    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::Transform() OK\r\n");
    return S_OK;
}

HRESULT AC3DecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    if(!m_hCodec)
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
    if(*mtOut->Subtype() != MEDIASUBTYPE_PCM)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(*mtOut->FormatType() != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    PWAVEFORMATEX pInWfx = reinterpret_cast<PWAVEFORMATEX>(mtIn->Format());
    PWAVEFORMATEX pOutWfx = reinterpret_cast<PWAVEFORMATEX>(mtOut->Format());
    
    if(pInWfx->nSamplesPerSec != pOutWfx->nSamplesPerSec)
        return VFW_E_TYPE_NOT_ACCEPTED;
    if(pInWfx->nChannels < pOutWfx->nChannels)
        return VFW_E_TYPE_NOT_ACCEPTED;
    return S_OK;
}

static const int cBufferTimeS = 1; // Buffer length in seconds
HRESULT AC3DecoderFilter::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    debugPrintf(DBG, L"AC3DecoderFilter::DecideBufferSize()\r\n");
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    size_t bufferSize = m_curOutputWfx.nAvgBytesPerSec * cBufferTimeS;

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
    m_curOutputWfx.wBitsPerSample = 8 * sizeof(short);
    m_curOutputWfx.nBlockAlign = m_curOutputWfx.nChannels * sizeof(short);
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

    int paramValue = m_curOutputWfx.nChannels > 2 ? 1 : 0;
    //Separate sub-woofer channel
    if(m_audioAPI.SetParam(m_hCodec, VO_PID_AC3_OUTLFEON, &paramValue))
        return E_FAIL;

/*
    0 = reserved
    1 = 1/0 (C)
    2 = 2/0 (L, R)
    3 = 3/0 (L, C, R)
    4 = 2/1 (L, R, l)
    5 = 3/1 (L, C, R, l)
    6 = 2/2 (L, R, l, r)
    7 = 3/2 (L, C, R, l, r)
*/
    switch(m_curOutputWfx.nChannels)
    {
    case 0:
        return E_FAIL;
    case 1:
    case 2:
        paramValue = m_curOutputWfx.nChannels;
        break;
    case 3:
    case 4:
        paramValue = 6;
        break;
    default:
        paramValue = 7;
        break;
    }
    if(m_audioAPI.SetParam(m_hCodec, VO_PID_AC3_OUTPUTMODE, &paramValue))
        return E_FAIL;

    paramValue = m_curOutputWfx.nChannels;
    if(m_audioAPI.SetParam(m_hCodec, VO_PID_AC3_NUMCHANS, &paramValue))
        return E_FAIL;

    debugPrintf(DBG, L"AC3DecoderFilter::SetOutputMediaType() nChannels=%d, SubType=%s\r\n", static_cast<unsigned int>(m_curOutputWfx.nChannels), *pmt->Subtype() == MEDIASUBTYPE_PostProcPCM ? L"MEDIASUBTYPE_PostProcPCM" : L"MEDIASUBTYPE_PCM");
    return S_OK;
}

