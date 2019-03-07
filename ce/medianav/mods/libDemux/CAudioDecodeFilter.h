
#pragma once

#include "StdAfx.h"
#include <utils.h>
#include <smartptr.h>
#include <atlbase.h>


#if (((ADECODE_DBG > 0) || defined(_DEBUG))&& !defined(ADECODE_PERF))
#define ADECODE_PERF 1
#endif

class CAudioDecodeFilter: public CTransformFilter
{
public:
    DECLARE_IUNKNOWN;
    CAudioDecodeFilter(TCHAR *name, LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsID);
    virtual ~CAudioDecodeFilter();

    HRESULT Receive(IMediaSample *pSample);
    HRESULT EndOfStream(void);
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
        const BYTE* inData;
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
        ,deliveryThreshold(0)
        {
        }
        OutBufferDesc(DWORD alignment, BYTE buffersCount, DWORD bufferSize, DWORD deliveryThreshold = 0)
            :alignment(alignment)
            ,buffersCount(buffersCount)
            ,bufferSize(bufferSize)
            ,deliveryThreshold(deliveryThreshold)
        {
        }
        DWORD alignment;
        BYTE  buffersCount;
        DWORD bufferSize;
        DWORD deliveryThreshold;
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
    DWORD m_deliveryThreshold;

    IMediaSamplePtr m_curOutMediaSample;

#if ADECODE_PERF > 0
    // Misc
    DWORD m_maxOutputBufUsed;
    LONGLONG m_dbgMaxFrameProcessTime;
    LONGLONG m_dbgAvgFrameProcessTime;
    DWORD    m_framesTookLongerThanAvg;
    DWORD    m_totalFramesProcessed;
#endif

private:
    __forceinline void attachToMediaSample(const IMediaSamplePtr& pSample, TransformBuffersState& buffersState)
    {
        pSample->GetPointer(&buffersState.outBuf);
        buffersState.outBuf += pSample->GetActualDataLength();
        buffersState.outBufSize = pSample->GetSize() - pSample->GetActualDataLength();
    }

    __forceinline HRESULT deliverAndReleaseCurOutSample()
    {
        HRESULT hr = S_OK;
        if(m_curOutMediaSample)
        {
            if(m_curOutMediaSample->GetActualDataLength() > 0 && m_pOutput)
                hr = m_pOutput->Deliver((IMediaSample *)m_curOutMediaSample);
            m_curOutMediaSample.Release();
        }
        return hr;
    }

    __forceinline HRESULT deliverAndReleaseOutSample(TransformBuffersState& buffersState)
    {
        DWORD usedBuffer = m_curOutMediaSample->GetSize() - buffersState.outBufSize;
        buffersState.outBufSize = 0;
        if(usedBuffer > 0)
        {
            m_curOutMediaSample->SetActualDataLength(usedBuffer);
#if ADECODE_PERF > 0
            if(m_maxOutputBufUsed < usedBuffer)
                m_maxOutputBufUsed = usedBuffer;
#endif
        }else
        {
            m_curOutMediaSample->SetActualDataLength(0);
        }

        return deliverAndReleaseCurOutSample();
    }

    __forceinline HRESULT deliverOutSampleAndContinue(IMediaSample* pSample, TransformBuffersState& buffersState)
    {
        REFERENCE_TIME tStart, tStop;
        bool hasStop = false;
        if(m_curOutMediaSample->GetTime(&tStart, &tStop) == S_OK)
        {
            m_curOutMediaSample->SetTime(&tStart, NULL);
            hasStop = true;
        }

        REFERENCE_TIME tMediaStart, tMediaStop;
        bool hasMediaStop = false;
        if(m_curOutMediaSample->GetMediaTime(&tMediaStart, &tMediaStop) == S_OK)
        {
            m_curOutMediaSample->SetMediaTime(&tMediaStart, NULL);
            hasMediaStop = true;
        }

        HRESULT hr = deliverAndReleaseOutSample(buffersState);
        if(FAILED(hr))
            return hr;

        IMediaSample *pOutSample;
        hr = InitializeOutputSample(pSample, &pOutSample);
        if(FAILED(hr)) 
            return hr;

        m_curOutMediaSample.Attach(pOutSample);
        m_curOutMediaSample->SetActualDataLength(0);

        m_curOutMediaSample->SetTime(NULL, hasStop ? &tStop : NULL);
        m_curOutMediaSample->SetMediaTime(NULL, hasMediaStop ? &tMediaStop : NULL);

        attachToMediaSample(m_curOutMediaSample, buffersState);
        return S_OK;
    }

};
