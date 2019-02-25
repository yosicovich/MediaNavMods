//
// AACDecoderFilter.cpp
// 

#include "AACDecoderFilter.h"
#include <audshow.h>

// Configuration
static const int cOutBufferTimeS = 1; // Buffer length in seconds
static const int cSampleSizePerChannel = 2; //sizeof(short); 
static const int cAACOneChannelOutFrameSize = 1024 * cSampleSizePerChannel;//1024(samples per channel) * size of sample
static const int cMaxChannels = 6; // make this higher to support files with more channels

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
AACDecoderFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new AACDecoderFilter(pUnk, phr);
}

AACDecoderFilter::AACDecoderFilter(LPUNKNOWN pUnk, HRESULT* phr)
:CAudioDecodeFilter(NAME("ACCDecoderFilter"), pUnk, phr, __uuidof(AACDecoderFilter))
,m_hDecoder(NULL)
,m_decoderReady(false)
,m_streamingMode(true)
{
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

AACDecoderFilter::~AACDecoderFilter()
{
    if(m_hDecoder)
    {
        NeAACDecClose(m_hDecoder);
    }
}

HRESULT AACDecoderFilter::decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity)
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
            errCode = NeAACDecInit(m_hDecoder, (BYTE*)buffersState.inData, buffersState.inDataSize, &sampleRate, &channels);
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
    NeAACDecDecode2(m_hDecoder, &hInfo, (BYTE*)buffersState.inData, buffersState.inDataSize, reinterpret_cast<void **>(&buffersState.outBuf), buffersState.outBufSize);
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

DWORD AACDecoderFilter::getFrameBufferSize()
{
    return m_streamingMode ? FAAD_MIN_STREAMSIZE * cMaxChannels : 0;
}

CAudioDecodeFilter::OutBufferDesc AACDecoderFilter::getOutBufferDesc()
{
    return m_outBufferDesc;
}

HRESULT AACDecoderFilter::CheckInputType(const CMediaType *mtIn)
{
    if(!m_hDecoder)
    {
        debugPrintf(DBG, L"AACDecoderFilter::CheckInputType(): codec is not initialized!\r\n");
        return E_UNEXPECTED;
    }
    HRESULT hr = CAudioDecodeFilter::CheckInputType(mtIn);
    if(FAILED(hr))
        return hr;

    if(*mtIn->Subtype() != MEDIASUBTYPE_AAC && *mtIn->Subtype() != MEDIASUBTYPE_AAC_AUDIO)
        return VFW_E_TYPE_NOT_ACCEPTED;

    WAVEFORMATEX* pInWfx = reinterpret_cast<WAVEFORMATEX*>(mtIn->Format());
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
    
    // Nothing more than STEREO is supported
    if(m_curOutputWfx.nChannels > 2)
        m_curOutputWfx.nChannels = 2;
    debugPrintf(DBG, L"AACDecoderFilter::CheckInputType() return S_OK!\r\n");
    return S_OK;
}

HRESULT AACDecoderFilter::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    HRESULT hr = CAudioDecodeFilter::CheckTransform(mtIn, mtOut);
    if(FAILED(hr))
        return hr;
    if(*mtIn->Subtype() != MEDIASUBTYPE_AAC && *mtIn->Subtype() != MEDIASUBTYPE_AAC_AUDIO)
        return VFW_E_TYPE_NOT_ACCEPTED;
    return S_OK;
}

HRESULT AACDecoderFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
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
    debugPrintf(DBG_TRACE, L"AACDecoderFilter::GetMediaType() iPosition=%d, nChannels=%d, SubType=%s\r\n", iPosition, static_cast<unsigned int>(m_curOutputWfx.nChannels), iPosition % 2 == 0 ? L"MEDIASUBTYPE_PostProcPCM": L"MEDIASUBTYPE_PCM");
    return S_OK;
}

HRESULT AACDecoderFilter::SetOutputMediaType(const CMediaType *pmt)
{
    debugPrintf(DBG_TRACE, L"AACDecoderFilter::SetOutputMediaType()\r\n");
    HRESULT hr = CAudioDecodeFilter::SetOutputMediaType(pmt);
    if(FAILED(hr))
        return hr;

    if(m_streamingMode)
    {
        // There is no frame alignment expected in streaming mode, so use frame buffer.
        m_outBufferDesc.alignment = cAACOneChannelOutFrameSize * m_curOutputWfx.nChannels;
        m_outBufferDesc.buffersCount = 2;
        m_outBufferDesc.bufferSize = m_curOutputWfx.nAvgBytesPerSec * cOutBufferTimeS;
    }else
    {
        // Frame aligned samples are expected
        m_outBufferDesc.alignment = cAACOneChannelOutFrameSize * m_curOutputWfx.nChannels;
        m_outBufferDesc.buffersCount = 4;
        m_outBufferDesc.deliveryThreshold = m_outBufferDesc.alignment * 8;
        m_outBufferDesc.bufferSize = m_outBufferDesc.deliveryThreshold;
    }
    return S_OK;
}

