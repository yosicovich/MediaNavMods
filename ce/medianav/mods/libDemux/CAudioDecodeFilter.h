
#pragma once

#include "StdAfx.h"
#include <utils.h>
#include <smartptr.h>


#if (((ADECODE_DBG > 0) || defined(_DEBUG))&& !defined(ADECODE_PERF))
#define ADECODE_PERF 1
#endif

class CAudioDecodeFilter: public CTransformFilter
{
public:
    DECLARE_IUNKNOWN;
    CAudioDecodeFilter(TCHAR *name, LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsID);
    virtual ~CAudioDecodeFilter();

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
        ALLOCATOR_PROPERTIES *pProperties);
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
    struct TransformBuffersState
    {
        BYTE* inData;
        DWORD  inDataSize;
        BYTE* outBuf;
        DWORD  outBufSize;
    };

    struct OutBufferDesc
    {
        OutBufferDesc()
        :alignment(0)
        ,buffersCount(0)
        ,bufferSize(0)
        {
        }
        OutBufferDesc(DWORD alignment, BYTE buffersCount, DWORD bufferSize)
            :alignment(alignment)
            ,buffersCount(buffersCount)
            ,bufferSize(bufferSize)
        {
        }
        DWORD alignment;
        BYTE  buffersCount;
        DWORD bufferSize;
    };

    // Output data
    WAVEFORMATEX m_curOutputWfx;

protected:
    virtual HRESULT SetOutputMediaType(const CMediaType *pmt);
    virtual HRESULT decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity) = 0;
    // Return 0 to disable frame buffer if you expect frame size aligned samples only.
    virtual DWORD getFrameBufferSize() = 0;
    virtual OutBufferDesc getOutBufferDesc() = 0;

private:
    // Frame buffering
    smart_array<BYTE> m_frameBuffer;
    DWORD m_frameBufferUsed;
    DWORD m_frameBufferSize;

    // Output data
    DWORD m_outBufferAlignment;

#if ADECODE_PERF > 0
    // Misc
    DWORD m_maxOutputBufUsed;
    LONGLONG m_dbgMaxFrameProcessTime;
    LONGLONG m_dbgAvgFrameProcessTime;
    DWORD    m_framesTookLongerThanAvg;
    DWORD    m_totalFramesProcessed;
#endif

};
