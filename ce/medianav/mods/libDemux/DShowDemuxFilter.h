//
// DemuxFilter.h
//
// Declaration of classes for DirectShow MPEG-4 Demultiplexor filter
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////

#pragma once


#include "thread.h"
#include "demuxtypes.h"

//
// Partially derived from GDCL sample MPEG-1 parser filter
// available in source form at http://www.gdcl.co.uk/articles
//

// Declaration of Mpeg-4 Demultiplexor Filter
//
// The filter has an IAsyncReader input pin and creates an output pin
// for each valid stream in the input pin.

class DShowDemultiplexor;
class DemuxInputPin;
class DemuxOutputPin;

// Since we are not using IMemInputPin, it does not make sense to derive from 
// CBaseInputPin. We need random access to the data, so we do not use CPullPin.
// A worker thread on the filter calls a Read method to synchronously fetch data
// from the source.
//
// Since we provide the data source for the file, we implement
// AtomReader to provide access to the data.
class DemuxInputPin 
: public CBasePin,
  public AtomReader
{
public:
    DemuxInputPin(DShowDemultiplexor* pFilter, CCritSec* pLock, HRESULT* phr);

    // base pin overrides
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    HRESULT CompleteConnect(IPin* pPeer);
    HRESULT BreakConnect();

    // AtomReader abstraction:
    // access to the file from container parsing classes
    HRESULT Read(LONGLONG llOffset, long cBytes, void* pBuffer);
    LONGLONG Length();

    // ...but we don't support the caching interface for the whole file
    // -- only individual atoms should be cached in memory
    bool IsBuffered()
    {
        return false;
    }

    // calls to Buffer and BufferRelease are refcounted and should correspond.
    const BYTE* Buffer() 
    {
        return NULL;
    }
    void BufferRelease() 
    {
    }

private:
    DShowDemultiplexor* m_pParser;
};


class DemuxOutputPin
: public CBaseOutputPin,
  public IMediaSeeking,
  public thread
{
public:
    DemuxOutputPin(MovieTrack* pTrack, DShowDemultiplexor* pDemux, CCritSec* pLock, HRESULT* phr, LPCWSTR pName);
	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

    // normally, pins share the filter's lifetime, but we will release the pins
    // when the input is disconnected, so they need to have a normal COM lifetime
    STDMETHODIMP_(ULONG) NonDelegatingAddRef()
    {
        return CUnknown::NonDelegatingAddRef();
    }
    STDMETHODIMP_(ULONG) NonDelegatingRelease()
    {
        return CUnknown::NonDelegatingRelease();
    }

    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType* pmt);
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pprop);
    
	// normally this is handled in the decoder, but there are some
	// rare cases (low complexity, high bitrate with temporal compression) where
	// it makes sense to do it here. We record the info
	// and only use it if it makes sense in ThreadProc
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q)
    {
		UNREFERENCED_PARAMETER(pSender);
		CAutoLock lock(&m_csLate);
		m_tLate = q.Late;
		return S_OK;
    }
    
    HRESULT Active();
    HRESULT Inactive();
    DWORD ThreadProc();

	BOOL GetMajorMediaType(GUID& MajorType) const;
	HRESULT SeekBackToKeyFrame(REFERENCE_TIME& tStart);

// IMediaSeeking
public:
    STDMETHODIMP GetCapabilities(DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities(DWORD * pCapabilities );
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                              LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions(LONGLONG * pCurrent, DWORD dwCurrentFlags
			, LONGLONG * pStop, DWORD dwStopFlags );
    STDMETHODIMP GetPositions(LONGLONG * pCurrent,
                              LONGLONG * pStop );
    STDMETHODIMP GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG * pllPreroll);

//  Debug stuff
    STDMETHODIMP GetMediaSampleTimes(ULONG* pnCount, LONGLONG** ppnStartTimes, LONGLONG** ppnStopTimes, ULONG** ppnFlags, ULONG** ppnDataSizes);

private:
    MovieTrack* m_pTrack;
    DShowDemultiplexor* m_pParser;

	// quality record
	CCritSec m_csLate;
	REFERENCE_TIME m_tLate;
};
typedef IPinPtr DemuxOutputPinPtr;

class DShowDemultiplexor: public CBaseFilter
{
public:
    DShowDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsID);
    virtual ~DShowDemultiplexor();
    // filter registration tables
    static const AMOVIESETUP_MEDIATYPE m_sudType[];
    static const AMOVIESETUP_PIN m_sudPin[];

    // CBaseFilter methods
    int GetPinCount();
    CBasePin *GetPin(int n);

    // called from input pin
    HRESULT BeginFlush();
    HRESULT EndFlush();
    HRESULT CompleteConnect(IPin* pPeer);
    HRESULT BreakConnect();

    // called from output pins for seeking support
    bool SelectSeekingPin(DemuxOutputPin* pPin);
    void DeselectSeekingPin(DemuxOutputPin* pPin);
    void GetSeekingParams(REFERENCE_TIME* ptStart, REFERENCE_TIME* ptStop, double* pdRate);
    HRESULT Seek(REFERENCE_TIME& tStart, BOOL bSeekToKeyFrame, const REFERENCE_TIME& tStop, double dRate);
    HRESULT SetRate(double dRate);
    HRESULT SetStopTime(const REFERENCE_TIME& tStop);

protected:
    virtual Atom* createAtom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader) = 0;
    virtual Movie* createMovie(Atom* pRoot) = 0;
    void setAlwaysSeekToKeyFrame(bool bVal)
    {
        m_alwaysSeekToKeyFrame = bVal;
    }

    DemuxOutputPin* Output(int n)
    {
        return static_cast<DemuxOutputPin*>((IPin*)m_Outputs[n]);
    }
private:
    CCritSec m_csFilter;
    DemuxInputPin* m_pInput;

    // one output pin for each enabled track
    vector<DemuxOutputPinPtr> m_Outputs;

    // for seeking
    CCritSec m_csSeeking;
    REFERENCE_TIME m_tStart;
    REFERENCE_TIME m_tStop;
    double m_dRate;
    DemuxOutputPin* m_pSeekingPin;
    bool m_alwaysSeekToKeyFrame;

    // file headers
    smart_ptr<Movie> m_pMovie;
};

#define pinDebugPrintf(lev, format, ...) debugPrintf(lev, L"PIN(%s) --- " format, Name(), __VA_ARGS__)

