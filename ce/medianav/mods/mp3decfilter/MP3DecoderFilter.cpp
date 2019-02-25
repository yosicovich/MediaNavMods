//
// MP3DecoderFilter.cpp
// 

#include "MP3DecoderFilter.h"
#include <audshow.h>

// Configuration
static const int cSampleSizePerChannel = 2; //sizeof(short);
static const int cMP3SamplePerFrame = 1152;
static const int cMP3OneChannelOutFrameSize = cMP3SamplePerFrame * cSampleSizePerChannel;

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
MP3DecoderFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new MP3DecoderFilter(pUnk, phr);
}

MP3DecoderFilter::MP3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr)
:CAudioDecodeFilter(NAME("MP3DecoderFilter"), pUnk, phr, __uuidof(MP3DecoderFilter))
,m_underFlowCondition(false)
,m_leftDither(16, 28, MAD_F_ONE)
,m_rightDither(16, 28, MAD_F_ONE)
{
    debugPrintf(DBG, L"MP3DecoderFilter::MP3DecoderFilter() Init codec\r\n");
    
    mad_stream_init(&m_madStream);
    mad_frame_init(&m_madFrame);
    mad_synth_init(&m_madSynth);
}

MP3DecoderFilter::~MP3DecoderFilter()
{
    mad_synth_finish(&m_madSynth);
    mad_frame_finish(&m_madFrame);
    mad_stream_finish(&m_madStream);
}

#ifdef SND_TEST
static short convert( mad_fixed_t i)
{
    // input 28bit float + 3bit whole + 1bit sing
    // output 15bit data + 1bit sign
    // MAD gives output in range wider than from -1.0 to 1.0 so We take 1 whole as well.
    // So we have to shrink 29bit(28 + 1) input to 15bit output.-> 29 - 15 = 14
    i = i >> 14;
    if (i > 32767)
        return 32767;
    else if (i < -32767)
        return -32768;
    else
        return i;
}
#endif

bool MP3DecoderFilter::storeSamples(TransformBuffersState& buffersState, const mad_pcm& pcm)
{
    DWORD dataSize = pcm.length * 2 * pcm.channels;
    if(buffersState.outBufSize < dataSize)    
        return false;
    short *pBuffer = reinterpret_cast<short*>(buffersState.outBuf);
    int bufPos = 0;
#ifdef SND_TEST
    if(Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, L"LGE\\SystemInfo", L"MP3_SND_TEST", false))
    {
        for (WORD i = 0; i < pcm.length; ++i)
        {
            pBuffer[bufPos++] = convert(pcm.samples[0][i]);
            if(pcm.channels > 1)
                pBuffer[bufPos++] = convert(pcm.samples[1][i]);
        }
    }else
#endif
    {
        for (WORD i = 0; i < pcm.length; ++i)
        {
            pBuffer[bufPos++] = m_leftDither.ditherSample(pcm.samples[0][i] >> 1);
            if(pcm.channels > 1)
                pBuffer[bufPos++] = m_rightDither.ditherSample(pcm.samples[1][i] >> 1);
        }
    }
    
    buffersState.outBufSize -= dataSize;
    buffersState.outBuf += dataSize;

    return true;
}

HRESULT MP3DecoderFilter::decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity)
{
    mad_stream_buffer(&m_madStream, buffersState.inData, buffersState.inDataSize);
    
    if(isDiscontinuity)
    {
        m_madStream.sync = 0;
        m_underFlowCondition = false;
    }else
    {
        if(m_underFlowCondition)
        {
            if(!storeSamples(buffersState, m_madSynth.pcm))
            {
                return VFW_E_BUFFER_OVERFLOW;
            }
            m_underFlowCondition = false;
        }
    }

    int madStatus = mad_frame_decode(&m_madFrame, &m_madStream);
    updateInBuffer(buffersState, m_madStream);

    if(madStatus == 0)
    {
        mad_synth_frame(&m_madSynth, &m_madFrame);
        if(!storeSamples(buffersState, m_madSynth.pcm))
        {
            m_underFlowCondition = true;
            return VFW_E_BUFFER_OVERFLOW;
        }
        m_underFlowCondition = false;
        return S_OK;
    }else
    {
        if(MAD_RECOVERABLE(m_madStream.error))
        {
            // Let the show go on.
            return S_OK;
        }else if(m_madStream.error == MAD_ERROR_BUFLEN)
        {
            return VFW_E_BUFFER_UNDERFLOW;
        }else
        {
            return E_FAIL;
        }
    }

    // We shouldn't get here really.
    return E_UNEXPECTED;
}

DWORD MP3DecoderFilter::getFrameBufferSize()
{
    // Let's assume the worst case. 320kbit at 32kHz
    // (320000/8) /(32000/1152) = 1152 * 10 /8
    return (cMP3SamplePerFrame * 10 / 8) * 2;// Mad needs at least two consequent frames to decode. We give him 2.
}

CAudioDecodeFilter::OutBufferDesc MP3DecoderFilter::getOutBufferDesc()
{
    return m_outBufferDesc;
}

HRESULT MP3DecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    HRESULT hr = CAudioDecodeFilter::CheckInputType(mtIn);
    if(FAILED(hr))
        return hr;

    if(*mtIn->Subtype() != MEDIASUBTYPE_MP3
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1AudioPayload
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1Payload
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1Packet
        && *mtIn->Subtype() != MEDIASUBTYPE_DIVX_ADRM
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG2_AUDIO
        )
        return VFW_E_TYPE_NOT_ACCEPTED;

    // Nothing more than STEREO is supported
    if(m_curOutputWfx.nChannels > 2)
        m_curOutputWfx.nChannels = 2;
    debugPrintf(DBG, L"MP3DecoderFilter::CheckInputType() return S_OK!\r\n");
    return S_OK;
}

HRESULT MP3DecoderFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    HRESULT hr = CAudioDecodeFilter::CheckTransform(mtIn, mtOut);
    if(FAILED(hr))
        return hr;
    if(*mtIn->Subtype() != MEDIASUBTYPE_MP3
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1AudioPayload
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1Payload
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG1Packet
        && *mtIn->Subtype() != MEDIASUBTYPE_DIVX_ADRM
        && *mtIn->Subtype() != MEDIASUBTYPE_MPEG2_AUDIO
        )
        return VFW_E_TYPE_NOT_ACCEPTED;
    return S_OK;
}

HRESULT MP3DecoderFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if(m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;
    if(iPosition >= 2)
        return VFW_S_NO_MORE_ITEMS;
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
    debugPrintf(DBG_TRACE, L"MP3DecoderFilter::GetMediaType() iPosition=%d, nChannels=%d, SubType=%s\r\n", iPosition, static_cast<unsigned int>(m_curOutputWfx.nChannels), iPosition % 2 == 0 ? L"MEDIASUBTYPE_PostProcPCM": L"MEDIASUBTYPE_PCM");
    return S_OK;
}

HRESULT MP3DecoderFilter::SetOutputMediaType(const CMediaType *pmt)
{
    debugPrintf(DBG_TRACE, L"MP3DecoderFilter::SetOutputMediaType()\r\n");
    HRESULT hr = CAudioDecodeFilter::SetOutputMediaType(pmt);
    if(FAILED(hr))
        return hr;

    m_outBufferDesc.alignment = cMP3OneChannelOutFrameSize * m_curOutputWfx.nChannels;
    m_outBufferDesc.buffersCount = 2;
    m_outBufferDesc.deliveryThreshold = m_outBufferDesc.alignment; //  1 frame per MediaSample
    m_outBufferDesc.bufferSize = m_outBufferDesc.deliveryThreshold * 2; // Take a doubled space to be on safer side.
    return S_OK;
}

