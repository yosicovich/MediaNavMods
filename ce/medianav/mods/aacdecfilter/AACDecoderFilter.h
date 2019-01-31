//
// AACDecoderFilter.h

#pragma once

#include "StdAfx.h"
#include <CAudioDecodeFilter.h>
#include <neaacdec.h>

class DECLSPEC_UUID("313F1007-5458-4275-8143-E760A1D73D0F") // accdecfilter
ACCDecoderFilter: public CAudioDecodeFilter
{
public:
    DECLARE_IUNKNOWN;
    virtual ~ACCDecoderFilter();
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

protected:
    HRESULT SetOutputMediaType(const CMediaType *pmt);
    HRESULT decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity);
    DWORD getFrameBufferSize();
    OutBufferDesc getOutBufferDesc();

private:
    // construct only via class factory
    ACCDecoderFilter(LPUNKNOWN pUnk, HRESULT* phr);

private:
    // Codec specific
    NeAACDecHandle m_hDecoder;
    bool m_decoderReady;
    bool m_streamingMode;  

    OutBufferDesc m_outBufferDesc;
};

// {000000FF-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AAC, 0x000000FF, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
