//
// TransformFilter.h

#pragma once


#include "StdAfx.h"
#include "voAC3.h"

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
private:
    // construct only via class factory
    AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr);
    HRESULT SetOutputMediaType(const CMediaType *pmt);
private:
    // Codec specific
    VO_MEM_OPERATOR m_cMemOps;
    VO_AUDIO_CODECAPI m_audioAPI;
    VO_HANDLE m_hCodec;

    // Output data
    int m_maxChannels;
    WAVEFORMATEX m_curOutputWfx;
    CMediaType m_mediaType;

    // Misc
    long m_maxOutputBufUsed;
};



