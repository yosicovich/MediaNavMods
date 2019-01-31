//
// AC3DecoderFilter.h

#pragma once

#include "StdAfx.h"
#include "a52.h"
#include <CAudioDecodeFilter.h>

class DECLSPEC_UUID("1F3F5741-A9EE-4bd9-B64E-99C5534B3817") // ac3decfilter
AC3DecoderFilter: public CAudioDecodeFilter
{
public:
    DECLARE_IUNKNOWN;
    virtual ~AC3DecoderFilter();
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
    AC3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr);

private:
    // Codec specific
    a52_state_t *m_a52State;
    int m_a52Flags;
    level_t m_a52Level;
    int m_a52OutputSpeakersConfig;
    long m_a52OneOutBlockSize;
    
    // Output data
    int m_maxChannels;

    OutBufferDesc m_outBufferDesc;
};



