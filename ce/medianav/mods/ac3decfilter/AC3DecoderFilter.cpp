//
// AC3DecoderFilter.cpp
// 

#include "AC3DecoderFilter.h"
#include <audshow.h>
#include <libao/audio_out_internal.h>

// --- registration tables ----------------

// filter registration information. 
const AMOVIESETUP_FILTER 
AC3DecoderFilter::m_sudFilter = 
{
    &__uuidof(AC3DecoderFilter),  // filter clsid
    L"AC3 decoder",   // filter name
    MERIT_NORMAL,                   // ie default for auto graph building
    3,                              // count of registered pins
    CAudioDecodeFilter::m_sudPins    // list of pins to register
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
    return new AC3DecoderFilter(pUnk, phr);// buffer to hold one frame at maximal possible bitrate.
}

AC3DecoderFilter::AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr)
:CAudioDecodeFilter(NAME("AC3DecoderFilter"), pUnk, phr, *m_sudFilter.clsID)
,m_a52State(NULL)
,m_maxChannels(0)
,m_a52OneOutBlockSize(0)
,m_a52OutputSpeakersConfig(A52_CHANNEL)
{
    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::AC3DecoderFilter() Init codec\r\n");
    m_a52State = a52_init();
    if(!m_a52State)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::AC3DecoderFilter() a52_init() failed\r\n");
        return;
    }
}

AC3DecoderFilter::~AC3DecoderFilter()
{
    if(m_a52State)
    {
        a52_free (m_a52State);
    }
}

static const int cAC3HeaderSize = 7;
HRESULT AC3DecoderFilter::decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity)
{
    DWORD frameDataSize = 0;
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

    if(buffersState.outBufSize < m_outBufferDesc.alignment)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::decodeOneFrame() outBufSize = %d < m_outBufferDesc.alignment = %d\r\n", buffersState.outBufSize, m_outBufferDesc.alignment);
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

DWORD AC3DecoderFilter::getFrameBufferSize()
{
    return 3840;
}

CAudioDecodeFilter::OutBufferDesc AC3DecoderFilter::getOutBufferDesc()
{
    return m_outBufferDesc;
}

HRESULT AC3DecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    if(!m_a52State)
    {
        debugPrintf(DBG, L"AC3DecoderFilter::CheckInputType(): codec is not initialized!\r\n");
        return E_UNEXPECTED;
    }
    HRESULT hr = CAudioDecodeFilter::CheckInputType(mtIn);
    if(FAILED(hr))
        return hr;

    if(*mtIn->Subtype() != MEDIASUBTYPE_DVM && *mtIn->Subtype() != MEDIASUBTYPE_DOLBY_AC3)
        return VFW_E_TYPE_NOT_ACCEPTED;
    
    m_maxChannels = m_curOutputWfx.nChannels;

    // Nothing more than STEREO is supported
    if(m_maxChannels > 2)
        m_maxChannels = 2;

    debugPrintf(DBG, L"AC3DecoderFilter::CheckInputType(): OK!\r\n");
    return S_OK;
}

HRESULT AC3DecoderFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    HRESULT hr = CAudioDecodeFilter::CheckTransform(mtIn, mtOut);
    if(FAILED(hr))
        return hr;
    if(*mtIn->Subtype() != MEDIASUBTYPE_DVM && *mtIn->Subtype() != MEDIASUBTYPE_DOLBY_AC3)
        return VFW_E_TYPE_NOT_ACCEPTED;
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

    CMediaType mediaType;
    mediaType.InitMediaType();
    mediaType.SetType(&MEDIATYPE_Audio);
    mediaType.SetFormatType(&FORMAT_WaveFormatEx);
    mediaType.SetSubtype(&MEDIASUBTYPE_PCM);
    if(iPosition % 2 == 0)
        mediaType.SetSubtype(&MEDIASUBTYPE_PostProcPCM);
    else
        mediaType.SetSubtype(&MEDIASUBTYPE_PCM);

    if(!mediaType.SetFormat(reinterpret_cast<BYTE *>(&m_curOutputWfx), sizeof(WAVEFORMATEX)))
        return E_FAIL;
    *pMediaType = mediaType;
    debugPrintf(DBG, L"AC3DecoderFilter::GetMediaType() iPosition=%d, nChannels=%d, SubType=%s\r\n", iPosition, static_cast<unsigned int>(m_curOutputWfx.nChannels), iPosition % 2 == 0 ? L"MEDIASUBTYPE_PostProcPCM": L"MEDIASUBTYPE_PCM");
    return S_OK;
}

HRESULT AC3DecoderFilter::SetOutputMediaType(const CMediaType *pmt)
{
    debugPrintf(DBG_TRACE, L"AC3DecoderFilter::SetOutputMediaType()\r\n");
    HRESULT hr = CAudioDecodeFilter::SetOutputMediaType(pmt);
    if(FAILED(hr))
        return hr;

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

    // Gain level 1 means no gain. Maximum possible gain is 2 since our fixed point implementation has -2 to 2 range.
    m_a52Level = a52_to_level(1);

    m_a52OneOutBlockSize = cAC3OneChannelOutBlockSize * m_curOutputWfx.nChannels;

    m_outBufferDesc.alignment = m_a52OneOutBlockSize * 6; // AC3 has 6 blocks per frame.
    m_outBufferDesc.buffersCount = 2;
    m_outBufferDesc.bufferSize = m_curOutputWfx.nAvgBytesPerSec * cOutBufferTimeS;
    return S_OK;
}

