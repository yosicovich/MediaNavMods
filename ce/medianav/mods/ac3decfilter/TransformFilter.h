//
// TransformFilter.h

#pragma once


#include "StdAfx.h"
#include "a52.h"

class DECLSPEC_UUID("1F3F5741-A9EE-4bd9-B64E-99C5534B3817") // ac3decfilter
AC3DecoderFilter: public CTransformFilter
{
public:
    DECLARE_IUNKNOWN;
    virtual ~AC3DecoderFilter();
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
    HRESULT decodeOneFrame(TTransformBuffersState& buffersState);
private:
    // construct only via class factory
    AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr, long frameBufferSize);
private:
    // Codec specific
    a52_state_t *m_a52State;
    int m_a52Flags;
    level_t m_a52Level;
    int m_a52OutputSpeakersConfig;
    long m_a52OneOutBlockSize;
    
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



