//
// DemuxFilter.cpp
// 
// Implementation of classes for DirectShow Mpeg-4 Demultiplexor filter
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DShowDemuxFilter.h"
#include <math.h>
#include "utils.h"


// --- registration tables ----------------

// filter registration -- these are the types that our
// pins accept and produce
const AMOVIESETUP_MEDIATYPE 
DShowDemultiplexor::m_sudType[] = 
{
    {
        &MEDIATYPE_Stream,
        &MEDIASUBTYPE_NULL,
    },
    {
        &MEDIATYPE_Video,
        &MEDIASUBTYPE_NULL      // wild card
    },
    {
        &MEDIATYPE_Audio,
        &MEDIASUBTYPE_NULL
    },
};

// registration of our pins for auto connect and render operations
const AMOVIESETUP_PIN 
DShowDemultiplexor::m_sudPin[] = 
{
    {
        L"Input",           // pin name
        FALSE,              // is rendered?    
        FALSE,              // is output?
        FALSE,              // zero instances allowed?
        FALSE,              // many instances allowed?
        &CLSID_NULL,        // connects to filter (for bridge pins)
        NULL,               // connects to pin (for bridge pins)
        1,                  // count of registered media types
        &m_sudType[0]       // list of registered media types    
    },
    {
        L"Video",          // pin name
        FALSE,              // is rendered?    
        TRUE,               // is output?
        FALSE,              // zero instances allowed?
        FALSE,              // many instances allowed?
        &CLSID_NULL,        // connects to filter (for bridge pins)
        NULL,               // connects to pin (for bridge pins)
        1,                  // count of registered media types
        &m_sudType[1]       // list of registered media types    
    },
    {
        L"Audio",          // pin name
        FALSE,              // is rendered?    
        TRUE,               // is output?
        FALSE,              // zero instances allowed?
        FALSE,              // many instances allowed?
        &CLSID_NULL,        // connects to filter (for bridge pins)
        NULL,               // connects to pin (for bridge pins)
        1,                  // count of registered media types
        &m_sudType[2]       // list of registered media types    
    },
};

// ---- construction/destruction and COM support -------------

DShowDemultiplexor::DShowDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsID)
: m_pInput(NULL),
  m_pSeekingPin(NULL),
  m_tStart(0),
  m_tStop(0x7ffffffffffffff),       // less than MAX_TIME so we can add one second to it
  m_dRate(1.0),
  m_alwaysSeekToKeyFrame(false),
  CBaseFilter(NAME("DShowDemultiplexor"), pUnk, &m_csFilter, clsID)
{
    debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::DShowDemultiplexor()\r\n");
    m_pInput = new DemuxInputPin(this, &m_csFilter, phr);
}

DShowDemultiplexor::~DShowDemultiplexor()
{
    debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::~DShowDemultiplexor(): m_pMovie->Tracks() = %u\r\n", m_pMovie->Tracks());
}


int 
DShowDemultiplexor::GetPinCount()
{
    return 1 + (int)m_Outputs.size();
}

CBasePin *
DShowDemultiplexor::GetPin(int n)
{
    if (n == 0)
    {
        return m_pInput.get();
    } else if (n <= (int)m_Outputs.size())
    {
        return Output(n-1);
    } else {
        return NULL;
    }
}


bool 
DShowDemultiplexor::SelectSeekingPin(DemuxOutputPin* pPin)
{
    CAutoLock lock(&m_csSeeking);
    if (m_pSeekingPin == NULL)
    {
        m_pSeekingPin = pPin;
    }
    return(m_pSeekingPin == pPin);
}

void 
DShowDemultiplexor::DeselectSeekingPin(DemuxOutputPin* pPin)
{
    CAutoLock lock(&m_csSeeking);
    if (pPin == m_pSeekingPin)
        m_pSeekingPin = NULL;
}

void 
DShowDemultiplexor::GetSeekingParams(REFERENCE_TIME* ptStart, REFERENCE_TIME* ptStop, double* pdRate)
{
    if (ptStart != NULL)
    {
        *ptStart = m_tStart;
    }
    if (ptStop != NULL)
    {
        *ptStop = m_tStop;
    }
    if (pdRate != NULL)
    {
        *pdRate = m_dRate;
    }
}
HRESULT 
DShowDemultiplexor::SetRate(double dRate)
{
    CAutoLock lock(&m_csSeeking);
    m_dRate = dRate;
    return S_OK;
}

HRESULT 
DShowDemultiplexor::SetStopTime(const REFERENCE_TIME& tStop)
{
    CAutoLock lock(&m_csSeeking);
    // this does not guarantee that a stop change only, while running,
    // will stop at the right point -- but most filters only
    // implement stop/rate changes when the current position changes
    m_tStop = tStop;
    return S_OK;
}

HRESULT 
DShowDemultiplexor::Seek(REFERENCE_TIME& tStart, BOOL bSeekToKeyFrame, const REFERENCE_TIME& tStop, double dRate)
{
    debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: tStart=%I64u, bSeekToKeyFrame=%u, tStop=%I64u\r\n", tStart, (bSeekToKeyFrame ? 1 : 0), tStop);
	#pragma region Flush, Stop Thread
    if (IsActive())
    {
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: before DeliverBeginFlush\r\n");
        for(SIZE_T nIndex = 0; nIndex < m_Outputs.size(); nIndex++)
        {
            DemuxOutputPin* pPin = Output((INT) nIndex);
            if(!pPin->IsConnected())
				continue;
            debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: pPin->DeliverBeginFlush(), nIndex=%u\r\n", nIndex);
			pPin->DeliverBeginFlush();
        }
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: before BeginRestartThread\r\n");
        for(SIZE_T nIndex = 0; nIndex < m_Outputs.size(); nIndex++)
        {
            DemuxOutputPin* pPin = Output((INT) nIndex);
            if(!pPin->IsConnected())
				continue;
			#if TRUE
                debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: pPin->BeginRestartThread(), nIndex=%u\r\n", nIndex);
				pPin->BeginRestartThread();
			#else
				pPin->StopThread();
			#endif
        }
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: before DeliverEndFlush\r\n");
        for(SIZE_T nIndex = 0; nIndex < m_Outputs.size(); nIndex++)
        {
            DemuxOutputPin* pPin = Output((INT) nIndex);
            if(!pPin->IsConnected())
				continue;
            debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: pPin->DeliverEndFlush(), nIndex=%u\r\n", nIndex);
            pPin->DeliverEndFlush();
        }
    }
	#pragma endregion
	#pragma region Start Time Adjustment
	if(m_alwaysSeekToKeyFrame || bSeekToKeyFrame )
    {
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: Seeking to key frame\r\n");
		DemuxOutputPin* pSeekingPin = m_pSeekingPin;
        {
			CAutoLock lock(&m_csSeeking);
            GUID MajorType;
			if(!m_pSeekingPin || !m_pSeekingPin->GetMajorMediaType(MajorType) || MajorType != MEDIATYPE_Video)
			{
				if(m_Outputs.size())
				{
					DemuxOutputPin* pConnectedPin = NULL;
					// Take connected video pin
					for(vector<DemuxOutputPinPtr>::iterator it = m_Outputs.begin(); it < m_Outputs.end(); it++)
					{
						DemuxOutputPin* pPin = static_cast<DemuxOutputPin*>((IPin*) *it);
                        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: searching for Video pin: trying %s\r\n", pPin->Name());
						if(!pPin->IsConnected())
							continue;
						if(!pConnectedPin)
							pConnectedPin = pPin;
						if(pPin->GetMajorMediaType(MajorType) && MajorType == MEDIATYPE_Video)
						{
                            debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: got Video pin named as %s\r\n", pPin->Name());
							pSeekingPin = pPin;
							break;
                        }
                    }
					// Or, take just connected pin
					if(!pSeekingPin)
                    {
						pSeekingPin = pConnectedPin;
                        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: hasn't got Video pin, use pin named as %s\r\n", pSeekingPin->Name());
                    }
                }
			} else
            {
				pSeekingPin = m_pSeekingPin;
                debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: use already selected pin, use pin named as %s\r\n", pSeekingPin->Name());
            }
            debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: got seeking pin\r\n");
		}
		if(pSeekingPin)
		{
            debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: pSeekingPin->SeekBackToKeyFrame()\r\n");
            pSeekingPin->SeekBackToKeyFrame(tStart);
		}
	}
	#pragma endregion 
	#pragma region Update
    m_tStart = tStart;
    m_tStop = tStop;
    m_dRate = dRate;
	#pragma endregion
	#pragma region Start Thread
    if (IsActive())
    {
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: before EndRestartThread\r\n");
        for(SIZE_T nIndex = 0; nIndex < m_Outputs.size(); nIndex++)
            {
            DemuxOutputPin* pPin = Output((INT) nIndex);
            if(!pPin->IsConnected())
				continue;
			#if TRUE
                debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::Seek: pPin->EndRestartThread(), nIndex=%u\r\n", nIndex);
				pPin->EndRestartThread();
			#else
	            pPin->StartThread();
			#endif
        }
    }
	#pragma endregion
    return S_OK;
}

HRESULT 
DShowDemultiplexor::BeginFlush()
{
    for (UINT i =0; i < m_Outputs.size(); i++)
    {
        DemuxOutputPin* p = Output(i);
        p->DeliverBeginFlush();
    }
    return S_OK;
}

HRESULT 
DShowDemultiplexor::EndFlush()
{
    for (UINT i =0; i < m_Outputs.size(); i++)
    {
        DemuxOutputPin* p = Output(i);
        p->DeliverEndFlush();
    }
    return S_OK;
}

HRESULT
DShowDemultiplexor::BreakConnect()
{
    for (UINT i =0; i < m_Outputs.size(); i++)
    {
        IPinPtr pPeer;
        m_Outputs[i]->ConnectedTo(&pPeer);
        if (pPeer != NULL)
        {
            m_Outputs[i]->Disconnect();
            pPeer->Disconnect();
        }
    }
    m_Outputs.clear();
    return S_OK;
}

HRESULT 
DShowDemultiplexor::CompleteConnect(IPin* pPeer)
{
    IAsyncReaderPtr pRdr = pPeer;
    if (pRdr == NULL)
    {
        return E_NOINTERFACE;
    }
    LONGLONG llTotal, llAvail;
    pRdr->Length(&llTotal, &llAvail);
    debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::CompleteConnect: Is about to create movie, current m_pMovie = %u\r\n", m_pMovie.get());
    m_pMovie = createMovie(m_pInput.dynamic_pointer_cast<AtomReader>());

    debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::CompleteConnect: m_pMovie->Tracks() = %u\r\n", m_pMovie->Tracks());
    if (m_pMovie->Tracks() <= 0)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }


    // construct output pin for each valid track
    HRESULT hr = S_OK;
    for (long  nTrack = 0; nTrack < m_pMovie->Tracks(); nTrack++)
    {
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::CompleteConnect: getting track nTrack = %u\r\n", nTrack);
        MovieTrackPtr pTrack = m_pMovie->Track(nTrack);
        _bstr_t strName(pTrack->Name());
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::CompleteConnect: strName = %s - %S\r\n", strName.GetBSTR(), pTrack->Name());
        // Make priority higher not to exhaust the queue
        DemuxOutputPinPtr pPin = new DemuxOutputPin(pTrack, this, &m_csFilter, &hr, strName, THREAD_PRIORITY_NORMAL);
        debugPrintf(DEMUX_DBG, L"DShowDemultiplexor::CompleteConnect: DemuxOutputPin created\r\n", strName.GetBSTR(), pTrack->Name());
        m_Outputs.push_back(pPin);
    }
    return hr;
}

// -------- input pin -----------------------------------------

DemuxInputPin::DemuxInputPin(DShowDemultiplexor* pFilter, CCritSec* pLock, HRESULT* phr)
: m_pParser(pFilter), 
  CBasePin(NAME("DemuxInputPin"), pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxInputPin::DemuxInputPin()\r\n");
}

// base pin overrides
HRESULT 
DemuxInputPin::CheckMediaType(const CMediaType* pmt)
{
    // we accept any stream type and validate it ourselves during Connection
	UNREFERENCED_PARAMETER(pmt);
	return S_OK;
}

HRESULT 
DemuxInputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
    if (iPosition != 0)
    {
        return VFW_S_NO_MORE_ITEMS;
    }
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Stream);
    pmt->SetSubtype(&MEDIASUBTYPE_NULL);
    return S_OK;
}

STDMETHODIMP 
DemuxInputPin::BeginFlush()
{
    return m_pParser->BeginFlush();
}

STDMETHODIMP 
DemuxInputPin::EndFlush()
{
    return m_pParser->EndFlush();
}

HRESULT 
DemuxInputPin::CompleteConnect(IPin* pPeer)
{
    HRESULT hr = CBasePin::CompleteConnect(pPeer);

    if (SUCCEEDED(hr))
    {
        // validate input with parser
        hr = m_pParser->CompleteConnect(pPeer);
    }
    return hr;
}

HRESULT 
DemuxInputPin::BreakConnect()
{
    HRESULT hr = CBasePin::BreakConnect();
    if (SUCCEEDED(hr))
    {
        hr = m_pParser->BreakConnect();
    }
    return hr;
}

HRESULT 
DemuxInputPin::Read(LONGLONG llOffset, long cBytes, void* pBuffer)
{
    HRESULT hr = E_FAIL;
    IAsyncReaderPtr pRdr = GetConnected();
    if (pRdr != NULL)
    {
        hr = pRdr->SyncRead(llOffset, cBytes, reinterpret_cast<BYTE *>(pBuffer));
    }
    return hr;
}

LONGLONG 
DemuxInputPin::Length()
{
    LONGLONG llTotal = 0, llAvail;
    IAsyncReaderPtr pRdr = GetConnected();
    if (pRdr != NULL)
    {
        pRdr->Length(&llTotal, &llAvail);
    }
    return llTotal;
}


// -------- output pin ----------------------------------------

DemuxOutputPin::DemuxOutputPin(const MovieTrackPtr& pTrack, DShowDemultiplexor* pDemux, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, DWORD dwPriority/* = THREAD_PRIORITY_NORMAL*/)
: thread(dwPriority),
  m_pParser(pDemux),
  m_pTrack(pTrack),
  m_tLate(0),
  CBaseOutputPin(NAME("DemuxOutputPin"), pDemux, pLock, phr, pName)
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::DemuxOutputPin()\r\n");
}
	
STDMETHODIMP 
DemuxOutputPin::NonDelegatingQueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IMediaSeeking)
    {
        return GetInterface((IMediaSeeking*)this, ppv);
    } else
    {
        return CBaseOutputPin::NonDelegatingQueryInterface(iid, ppv);
    }
}

HRESULT 
DemuxOutputPin::CheckMediaType(const CMediaType *pmt)
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::CheckMediaType: enter\r\n");
    CMediaType mtTrack;
    int idx = 0;
    while(m_pTrack->GetType(&mtTrack, idx++))
    {
        if (*pmt == mtTrack)
        {
            // precise match to the type in the file
            pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::CheckMediaType: Pin=%s MATCH idx = %d\r\n", Name(), --idx);
            return S_OK;
        }
    }
    // does not match any alternative
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::CheckMediaType: Pin=%s NO MATCH\r\n", Name());
    return S_FALSE;
}

HRESULT 
DemuxOutputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::GetMediaType: Pin=%s iPosition = %d\r\n", Name(), iPosition);
    if (m_pTrack->GetType(pmt, iPosition))
    {
        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::GetMediaType: Pin=%s success\r\n", Name(), iPosition);
        return S_OK;
    } 
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::GetMediaType: Pin=%s no more items\r\n", Name(), iPosition);
    return VFW_S_NO_MORE_ITEMS;
}
    
HRESULT 
DemuxOutputPin::SetMediaType(const CMediaType* pmt)
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::SetMediaType: enter\r\n");
    HRESULT hr = CBaseOutputPin::SetMediaType(pmt);
    if (SUCCEEDED(hr))
    {
        if (!m_pTrack->SetType(pmt))
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    return hr;
}

HRESULT 
DemuxOutputPin::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pprop)
{
    long cMax = m_pTrack->SizeIndex()->MaxSize();
    if (m_pTrack->Handler() == NULL)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    pprop->cbBuffer = m_pTrack->Handler()->BufferSize(cMax);

    pprop->cBuffers = 30;
    ALLOCATOR_PROPERTIES propActual;

    return pAlloc->SetProperties(pprop, &propActual);
}

// this group of methods deal with the COutputQueue
HRESULT 
DemuxOutputPin::Active()
{
    HRESULT hr = CBaseOutputPin::Active();
    if (SUCCEEDED(hr) && IsConnected())
    {
        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::Active: StartThread()\r\n");
        StartThread();
        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::Active: Started()\r\n");
    }
    return hr;
}

HRESULT 
DemuxOutputPin::Inactive()
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::Inactive: CBaseOutputPin::Inactive()\r\n");
    HRESULT hr = CBaseOutputPin::Inactive();
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::Inactive: StopThread()\r\n");
    StopThread();
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::Inactive: Stopped()\r\n");
    return hr;
}

DWORD
DemuxOutputPin::ThreadProc()
{
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: priority = %d\r\n", CeGetThreadPriority(GetCurrentThread()));

    if (!IsConnected() || (m_pTrack == NULL))
    {
        return 0;
    }
    FormatHandler* pHandler = m_pTrack->Handler();
    if (pHandler == NULL)
    {
        m_pParser->NotifyEvent(EC_STREAM_ERROR_STOPPED, VFW_E_TYPE_NOT_ACCEPTED, 0);
        return 0;
    }

    for(; ; )
    {
        if(!InternalRestartThread())
            break;

        REFERENCE_TIME tStart, tStop;
        // HOTFIX: Volatile specifier is not really necessary here but it fixes a nasty problem with MainConcept AVC SDK violating x64 calling convention;
        //         MS compiler might choose to keep dRate in XMM6 register and the value would be destroyed by the violating call leading to incorrect 
        //         further streaming (wrong time stamps)
        volatile DOUBLE dRate;
        m_pParser->GetSeekingParams(&tStart, &tStop, (DOUBLE*) &dRate);

        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: DeliverNewSegment()\r\n");
        DeliverNewSegment(tStart, tStop, dRate);

        m_tLate = 0;

        // wind back to key frame before and check against duration
        long nSample;
        size_t segment;
        if (!m_pTrack->CheckInSegment(tStart, true, &segment, &nSample))
        {
            pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: DeliverEndOfStream() case 1\r\n");
            DeliverEndOfStream();
            return 0;
        }
        
        {
            REFERENCE_TIME nSampleTime = m_pTrack->TimesIndex()->SampleToCTS(nSample);
            long nSample2 = m_pTrack->TimesIndex()->CTSToSample(nSampleTime);
            pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: nSample=%u,  nSampleTime=%I64d, tStart=%I64d, nSample2=%u, nSyncBefore=%u\r\n", nSample, nSampleTime, tStart, nSample2, m_pTrack->GetKeyMap()->SyncFor(nSample));
        }

        if (tStop > m_pTrack->Duration())   
        {
            tStop = m_pTrack->Duration();
        }
        // used only for quality management. No segment support yet
        long nStop = m_pTrack->TimesIndex()->DTSToSample(tStop);

        bool bFirst = true;
        pHandler->StartStream();

        bool bHandleQuality = false;
        // for some formats it might make sense to handle quality here, since we
        // can skip the read as well as the decode.
        // For now this is only enabled for the (rare and old) Quicktime RLE codec.
        if (*m_mt.Subtype() == MEDIASUBTYPE_QTRle)
        {
            bHandleQuality = true;
        }

        ////////////////////////////////////////////////
        // HOTFIX: For zero length samples
        const GUID& Subtype = *m_mt.Subtype();
        const BOOL bIsFourCharacterCodeSubtype = memcmp(&Subtype.Data2, &MEDIASUBTYPE_YV12.Data2, sizeof (GUID) - offsetof(GUID, Data2)) == 0;
        const BOOL bIsAvc1Subtype = bIsFourCharacterCodeSubtype && (Subtype.Data1 == MAKEFOURCC('A', 'V', 'C', '1') || Subtype.Data1 == MAKEFOURCC('a', 'v', 'c', '1'));
        ////////////////////////////////////////////////

        const HANDLE phObjects[] = { ExitEvent(), RestartRequestEvent() };
        BOOL bRestart = FALSE;
        for(; ; )
        {
            const DWORD nWaitResult = WaitForMultipleObjects(_countof(phObjects), phObjects, FALSE, 0);
            ASSERT(nWaitResult - WAIT_OBJECT_0 < _countof(phObjects) || nWaitResult == WAIT_TIMEOUT);
            if(nWaitResult != WAIT_TIMEOUT)
            {
                bRestart = nWaitResult == WAIT_OBJECT_0 + 1; // m_evRestartRequest
                break;
            }

            REFERENCE_TIME tNext, tDur;
            m_pTrack->GetTimeBySegment(nSample, segment, &tNext, &tDur);

            if (tNext >= tStop)
            {
                pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: DeliverEndOfStream() case 2\r\n");
                DeliverEndOfStream();
                break;
            }

            if(m_tLate)
                pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: LATE!!! m_tLate = %d\r\n", m_tLate);
#pragma region Quality
            if (bHandleQuality)
            {
                // only for uncompressed YUV format -- no segment support yet
                REFERENCE_TIME late;
                {
                    CAutoLock lock(&m_csLate);
                    late = m_tLate;
                }
                REFERENCE_TIME perFrame = REFERENCE_TIME(tDur / dRate);
                // if we are more than two frames late, aim to be a frame early
                if (late > (perFrame * 2))
                {
                    late += perFrame;
                }

                while (late > (perFrame / 2))
                {
                    // we are more than 3/4 frame late. Should we skip?
                    int next = m_pTrack->GetKeyMap()->Next(nSample);
                    if (next && (next <= nStop))
                    {
                        REFERENCE_TIME tDiff = m_pTrack->TimesIndex()->SampleToCTS(next) - m_pTrack->TimesIndex()->SampleToCTS(nSample);
                        tDiff = REFERENCE_TIME(tDiff / dRate);
                        if ((next == (nSample+1)) || ((tDiff/2) < late))
                        {
                            // yes -- we are late by at least 1/2 of the distance to the next key frame
                            late -= tDiff;
                            nSample = next;
                            bFirst = true;
                        }
                    }

                    if (nSample != next)
                    {
                        break;
                    }
                }
            }
#pragma endregion 
#pragma region Sample
            IMediaSamplePtr pSample;
            HRESULT hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
            if (hr != S_OK)
            {
                pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: error on GetDeliveryBuffer() - no allocator? Stop. hr=0x%08X\r\n", hr);
                break;
            }
#pragma endregion 

            LONGLONG llPos = m_pTrack->SizeIndex()->Offset(nSample);
            long cSample = m_pTrack->SizeIndex()->Size(nSample);
            long lastSample = nSample;

#pragma region Processing
            if ((cSample < 16) && (m_pTrack->IsOldAudioFormat()))
            {
                // this is the older MOV format: uncompressed audio is indexed by individual samples
                // fill the buffer with contiguous samples
                long nThis = lastSample;
                size_t segThis = segment;
                while (m_pTrack->NextBySegment(&nThis, &segThis))
                {
                    REFERENCE_TIME tAdd, tDurAdd;
                    m_pTrack->GetTimeBySegment(nThis, segThis, &tAdd, &tDurAdd);
                    if (tAdd >= tStop)
                    {
                        break;
                    }
                    LONGLONG llPosNext = m_pTrack->SizeIndex()->Offset(nThis);
                    if (llPosNext != (llPos + cSample))
                    {
                        break;
                    }
                    long cNext = m_pTrack->SizeIndex()->Size(nThis);
                    if ((cSample + cNext) > pSample->GetSize())
                    {
                        break;
                    }
                    tDur += tDurAdd;
                    cSample += cNext;
                    lastSample = nThis;
                    segment = segThis;
                }
            }
            else if ((cSample < 4) || (tDur < 1))
            {
                // some formats have empty frames with non-zero entries of less than 4 bytes
                ////////////////////////////////////////////////
                // HOTFIX: H.264 video streams might have vital zero length NALs we cannot skip
                if(!bIsAvc1Subtype)
                    ////////////////////////////////////////////////
                    cSample = 0;
            }
#pragma endregion
#pragma region Delivery
            if (cSample > 0)
            {
                if (cSample > pSample->GetSize())
                {
                    // internal error since we checked the sizes
                    // before setting the allocator
                    m_pParser->NotifyEvent(EC_STREAM_ERROR_STOPPED, VFW_E_BUFFER_OVERFLOW, 0);
                    break;
                }

                // format-specific handler does read from file and any conversion/wrapping needed
                cSample = pHandler->PrepareOutput(pSample, m_pTrack->GetMovie(), llPos, cSample);

                if (cSample > 0)
                {
                    pSample->SetActualDataLength(cSample);
                    if (m_pTrack->GetKeyMap()->SyncFor(nSample) == nSample)
                    {
                        pinDebugPrintf(DEMUX_TRACE, L"DemuxOutputPin::ThreadProc: %S pSample->SetSyncPoint(true), nSample=%u\r\n", m_pTrack->Name(), nSample);
                        pSample->SetSyncPoint(true);
                    }

                    {
                        REFERENCE_TIME& nMediaStartTime = tNext;
                        REFERENCE_TIME nMediaStopTime = nMediaStartTime + tDur;
                        pinDebugPrintf(DEMUX_TRACE, L"DemuxOutputPin::ThreadProc: pSample->SetMediaTime(%I64d, %I64d)\r\n", nMediaStartTime, nMediaStopTime);
                        pSample->SetMediaTime(&nMediaStartTime, &nMediaStopTime);
                    }

                    REFERENCE_TIME tSampleStart = tNext - tStart;
                    if (tSampleStart < 0)
                    {
                        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: pSample->SetPreroll(true) tStart=%I64d\r\n", tStart);
                        pSample->SetPreroll(true);
                    }
                    REFERENCE_TIME tSampleEnd = tSampleStart + tDur;

                    // oops. clearly you need to divide by dRate. At double the rate, each frame lasts half as long.
                    tSampleStart = REFERENCE_TIME(tSampleStart / dRate);
                    tSampleEnd = REFERENCE_TIME(tSampleEnd / dRate);

                    pSample->SetTime(&tSampleStart, &tSampleEnd);
                    pinDebugPrintf(DEMUX_TRACE, L"DemuxOutputPin::ThreadProc: pSample->SetTime(%I64d, %I64d)\r\n", tSampleStart, tSampleEnd);
                    if (bFirst)
                    {
                        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: pSample->SetDiscontinuity(true)\r\n");
                        pSample->SetDiscontinuity(true);
                        bFirst = false;
                    }
                    HRESULT hr = Deliver(pSample);
                    switch(hr)
                    {
                    case S_OK:
                        break;
                    case E_FAIL:// Not ready yet?
                        hr = S_OK;
                        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: error on Deliver() - E_FAIL, codec not ready?\r\n");
                        break;
                    case VFW_E_WRONG_STATE:// Most likely we have been stopped
                        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: error on Deliver() - Stopped?\r\n");
                        break;
                    default:
                        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: error on Deliver() - sample 0x%08X!\r\n", hr);
                        m_pParser->NotifyEvent(EC_STREAM_ERROR_STOPPED, hr, 0);
                        break;
                    }

                    if(hr != S_OK)
                        break;
                }else
                {
                    // Most likely media has been removed
                    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: Media removal detected!\r\n");
                    m_pParser->NotifyEvent(EC_STREAM_ERROR_STOPPED, VFW_S_STREAM_OFF, 0);
                    break;
                }
            }
#pragma endregion

            nSample = lastSample;
            if (!m_pTrack->NextBySegment(&nSample, &segment))
            {
                pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::ThreadProc: DeliverEndOfStream() case 3 nSample = %d, segment = %d\r\n", nSample, segment);
                DeliverEndOfStream();
                break;
            }
        }
        if(bRestart)
            continue;
        break;
    }
    return 0;
}

BOOL DemuxOutputPin::GetMajorMediaType(GUID& MajorType) const
{
	CAutoLock Lock(m_pLock);
	if(!m_mt.IsValid())
		return FALSE;
	MajorType = *m_mt.Type();
	return TRUE;
}

HRESULT DemuxOutputPin::SeekBackToKeyFrame(REFERENCE_TIME& tStart)
{
	// NOTE: This basically duplicates initial seek logic in ThreadProc above
	if(!m_pTrack)
		return E_NOINTERFACE;
	size_t segment;
	long nSample;
	if (!m_pTrack->CheckInSegment(tStart, true, &segment, &nSample))
		return S_FALSE;
	REFERENCE_TIME tNext, tDur;
	m_pTrack->GetTimeBySegment(nSample, segment, &tNext, &tDur);
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::SeekBackToKeyFrame: tStart=%I64d, tNext=%I64d\r\n", tStart, tNext);
	if(tNext == tStart)
		return S_FALSE;
	tStart = tNext;
    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::SeekBackToKeyFrame: EXIT success\r\n");
	return S_OK;
}

// output pin seeking implementation -----------------------

STDMETHODIMP 
DemuxOutputPin::GetCapabilities(DWORD * pCapabilities)
{

    // Some versions of DShow have an aggregation bug that
    // affects playback with Media Player. To work around this,
    // we need to report the capabilities and time format the
    // same on all pins, even though only one
    // can seek at once.
    *pCapabilities =        AM_SEEKING_CanSeekAbsolute |
                            AM_SEEKING_CanSeekForwards |
                            AM_SEEKING_CanSeekBackwards |
                            AM_SEEKING_CanGetDuration |
                            AM_SEEKING_CanGetCurrentPos |
                            AM_SEEKING_CanGetStopPos;
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::CheckCapabilities(DWORD * pCapabilities)
{
    DWORD dwActual;
    GetCapabilities(&dwActual);
    if (*pCapabilities & (~dwActual))
    {
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::IsFormatSupported(const GUID * pFormat)
{
    // Some versions of DShow have an aggregation bug that
    // affects playback with Media Player. To work around this,
    // we need to report the capabilities and time format the
    // same on all pins, even though only one
    // can seek at once.
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
    {
        return S_OK;
    }
    return S_FALSE;

}
STDMETHODIMP 
DemuxOutputPin::QueryPreferredFormat(GUID * pFormat)
{
    // Some versions of DShow have an aggregation bug that
    // affects playback with Media Player. To work around this,
    // we need to report the capabilities and time format the
    // same on all pins, even though only one
    // can seek at once.
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::GetTimeFormat(GUID *pFormat)
{
    return QueryPreferredFormat(pFormat);
}

STDMETHODIMP 
DemuxOutputPin::IsUsingTimeFormat(const GUID * pFormat)
{
    GUID guidActual;
    HRESULT hr = GetTimeFormat(&guidActual);

    if (SUCCEEDED(hr) && (guidActual == *pFormat))
    {
        return S_OK;
    } else
    {
        return S_FALSE;
    }
}

STDMETHODIMP 
DemuxOutputPin::ConvertTimeFormat(
                                      LONGLONG* pTarget, 
                                      const GUID* pTargetFormat,
                                      LONGLONG Source, 
                                      const GUID* pSourceFormat)
{
    // format guids can be null to indicate current format

    // since we only support TIME_FORMAT_MEDIA_TIME, we don't really
    // offer any conversions.
    if (pTargetFormat == 0 || *pTargetFormat == TIME_FORMAT_MEDIA_TIME)
    {
        if (pSourceFormat == 0 || *pSourceFormat == TIME_FORMAT_MEDIA_TIME)
        {
            *pTarget = Source;
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

STDMETHODIMP 
DemuxOutputPin::SetTimeFormat(const GUID * pFormat)
{
    // only one pin can control seeking for the whole filter.
    // This method is used to select the seeker.
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
    {
        // try to select this pin as seeker (if the first to do so)
        if (m_pParser->SelectSeekingPin(this))
        {
            return S_OK;
        } else
        {
            return E_NOTIMPL;
        }
    } else if (*pFormat == TIME_FORMAT_NONE)
    {
        // deselect ourself, if we were the controlling pin
        m_pParser->DeselectSeekingPin(this);
        return S_OK;
    } else
    {
        // no other formats supported
        return E_NOTIMPL;
    }
}

STDMETHODIMP 
DemuxOutputPin::GetDuration(LONGLONG *pDuration)
{
    *pDuration = m_pTrack->Duration();
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::GetStopPosition(LONGLONG *pStop)
{
    REFERENCE_TIME tStart, tStop;
    double dRate;
    m_pParser->GetSeekingParams(&tStart, &tStop, &dRate);
    *pStop = tStop;
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    // this method is not supposed to report the previous start
    // position, but rather where we are now. This is normally
    // implemented by renderers, not parsers
    UNREFERENCED_PARAMETER(pCurrent);
    return E_NOTIMPL;
}

STDMETHODIMP 
DemuxOutputPin::SetPositions(
                                 LONGLONG * pCurrent, 
                                 DWORD dwCurrentFlags, 
                                 LONGLONG * pStop, 
                                 DWORD dwStopFlags)
{
	ASSERT(!(dwCurrentFlags & (AM_SEEKING_Segment | AM_SEEKING_NoFlush)));
	ASSERT(!(dwStopFlags & (AM_SEEKING_SeekToKeyFrame | AM_SEEKING_Segment | AM_SEEKING_NoFlush)));

    // for media player, with the aggregation bug in DShow, it
    // is better to return success and ignore the call if we are
    // not the controlling pin
    if (!m_pParser->SelectSeekingPin(this))
    {
        pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::SetPositions EXIT not control pin\r\n");
        return S_OK;
    }

    pinDebugPrintf(DEMUX_DBG, L"DemuxOutputPin::SetPositions we are the control pin(%s) - seeking\r\n", Name());
    // fetch current properties
    REFERENCE_TIME tStart, tStop;
    double dRate;
    m_pParser->GetSeekingParams(&tStart, &tStop, &dRate);
    if (dwCurrentFlags & AM_SEEKING_AbsolutePositioning)
    {
        tStart = *pCurrent;
    } else if (dwCurrentFlags & AM_SEEKING_RelativePositioning)
    {
        tStart += *pCurrent;
    }

    if (dwStopFlags & AM_SEEKING_AbsolutePositioning)
    {
        tStop = *pStop;
    } else if (dwStopFlags & AM_SEEKING_IncrementalPositioning)
    {
        tStop = *pStop + tStart;
    } else
    {
        if (dwStopFlags & AM_SEEKING_RelativePositioning)
        {
            tStop += *pStop;
        }
    }

	HRESULT nResult;
    if (dwCurrentFlags & AM_SEEKING_PositioningBitsMask)
    {
        nResult = m_pParser->Seek(tStart, dwCurrentFlags & AM_SEEKING_SeekToKeyFrame, tStop, dRate);
    } else 
	if(dwStopFlags & AM_SEEKING_PositioningBitsMask)
    {
        nResult = m_pParser->SetStopTime(tStop); // stop change only
    } else
        return S_FALSE; // no operation required
	if(SUCCEEDED(nResult))
    {
		if(pCurrent && (dwCurrentFlags & AM_SEEKING_ReturnTime))
			*pCurrent = tStart;
		if(pStop && (dwStopFlags & AM_SEEKING_ReturnTime))
			*pStop = tStop;
    }

	return nResult;
}

STDMETHODIMP 
DemuxOutputPin::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop)
{
    REFERENCE_TIME tStart, tStop;
    double dRate;
    m_pParser->GetSeekingParams(&tStart, &tStop, &dRate);
    *pCurrent = tStart;
    *pStop = tStop;
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest)
{
    if (pEarliest != NULL)
    {
        *pEarliest = 0;
    }
    if (pLatest != NULL)
    {
        *pLatest = m_pTrack->Duration();
    }
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::SetRate(double dRate)
{
    HRESULT hr = S_OK;
    if (m_pParser->SelectSeekingPin(this))
    {
        hr = m_pParser->SetRate(dRate);
    }
    return hr;
}



STDMETHODIMP 
DemuxOutputPin::GetRate(double * pdRate)
{
    REFERENCE_TIME tStart, tStop;
    double dRate;
    m_pParser->GetSeekingParams(&tStart, &tStop, &dRate);
    *pdRate = dRate;
    return S_OK;
}

STDMETHODIMP 
DemuxOutputPin::GetPreroll(LONGLONG * pllPreroll)
{
    // don't need to allow any preroll time for us
    *pllPreroll = 0;
    return S_OK;
}

// IDemuxOutputPin

STDMETHODIMP DemuxOutputPin::GetMediaSampleTimes(ULONG* pnCount, LONGLONG** ppnStartTimes, LONGLONG** ppnStopTimes, ULONG** ppnFlags, ULONG** ppnDataSizes)
{
	if(!pnCount || !ppnStartTimes)
		return E_POINTER;
	if(!ppnStopTimes && !ppnFlags && !ppnDataSizes)
		return E_INVALIDARG; // Nothing to Do
	if(ppnStopTimes || ppnDataSizes)
		return E_NOTIMPL;
	//CAutoLock Lock(m_pLock);
	if(!m_pTrack)
		return E_NOINTERFACE;
	*pnCount = (ULONG) m_pTrack->GetTimes(ppnStartTimes, ppnStopTimes, ppnFlags, ppnDataSizes);
	return S_OK;
}

BSTR _com_util::ConvertStringToBSTR(const char* pSrc)
{
    if(!pSrc) return NULL;

    DWORD cwch;

    BSTR wsOut(NULL);

    if(cwch = ::MultiByteToWideChar(CP_ACP, 0, pSrc, 
        -1, NULL, 0))//get size minus NULL terminator
    {
        cwch--;
        wsOut = ::SysAllocStringLen(NULL, cwch);

        if(wsOut)
        {
            if(!::MultiByteToWideChar(CP_ACP, 
                0, pSrc, -1, wsOut, cwch))
            {
                if(ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                    return wsOut;
                ::SysFreeString(wsOut);//must clean up
                wsOut = NULL;
            }
        }

    };

    return wsOut;
};
