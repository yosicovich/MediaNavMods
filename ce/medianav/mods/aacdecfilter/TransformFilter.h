//
// TransformFilter.h

#pragma once


#include "StdAfx.h"
#include <neaacdec.h>

class DECLSPEC_UUID("313F1007-5458-4275-8143-E760A1D73D0F") // accdecfilter
ACCDecoderFilter: public CTransformFilter
{
public:
    DECLARE_IUNKNOWN;
    virtual ~ACCDecoderFilter();
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    // filter registration tables
    static const AMOVIESETUP_PIN m_sudPins[];
    static const AMOVIESETUP_FILTER m_sudFilter;

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
        ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
    {
        HRESULT hr = CTransformFilter::SetMediaType(direction, pmt);
        if(FAILED(hr))
            return hr;

        if(direction == PINDIR_OUTPUT)
            return SetOutputMediaType(pmt);

        return S_OK;
    }
protected:
    typedef struct  
    {
        BYTE* inData;
        long  inDataSize;
        BYTE* outBuf;
        long  outBufSize;
    }TTransformBuffersState;

    HRESULT SetOutputMediaType(const CMediaType *pmt);
    HRESULT decodeOneFrame(TTransformBuffersState& buffersState, bool isDiscontinuity);
private:
    // construct only via class factory
    ACCDecoderFilter(LPUNKNOWN pUnk, HRESULT* phr, long frameBufferSize);
private:
    // Codec specific
    NeAACDecHandle m_hDecoder;
    bool m_decoderReady;
    bool m_streamingMode;
    
    // Frame buffering
    long m_frameBufferSize;
    BYTE* m_frameBuffer;
    long m_frameBufferUsed;
    long m_frameBufferPos;

    // Output data
    int m_maxChannels;
    WAVEFORMATEX m_curOutputWfx;
    CMediaType m_mediaType;
    long m_oneFrameOutSize;

    // Misc
    long m_maxOutputBufUsed;

#if DBG > 0
    //Debug
    LONGLONG m_dbgMaxFrameProcessTime;
    LONGLONG m_dbgAvgFrameProcessTime;
#endif

};

// {000000FF-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AAC, 0x000000FF, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
