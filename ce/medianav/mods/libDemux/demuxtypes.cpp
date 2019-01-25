//
// demuxtypes.cpp: implementation of common demux classes
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "demuxtypes.h"
#include <sstream>

Atom::Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, DWORD cHeader, bool canHaveChildren/* = true*/)
: m_pSource(pReader),
  m_llOffset(llOffset),
  m_cBufferRefCount(0),
  m_cHeader(cHeader),
  m_llLength(llLength),
  m_type(type),
  m_bChildrenScaned(!canHaveChildren)
{
}

HRESULT 
Atom::Read(LONGLONG llOffset, DWORD cBytes, void* pBuffer)
{
    HRESULT hr = S_OK;
    if (IsBuffered())
    {
        const BYTE* pSrc = Buffer() + DWORD(llOffset);
        CopyMemory(pBuffer,  pSrc,  cBytes);
        BufferRelease();
    } else {
        hr = m_pSource->Read(llOffset + m_llOffset, cBytes, pBuffer);
    }
    return hr;
}
   

DWORD 
Atom::ChildCount()
{
    if (m_Children.size() == 0 && !m_bChildrenScaned)
    {
        ScanChildrenAt(0);
        m_bChildrenScaned = true;
    }
    return (DWORD)m_Children.size();
}

AtomPtr 
Atom::Child(DWORD nChild)
{
    if (nChild >= ChildCount())
    {
        return AtomPtr();
    }
    list<AtomPtr>::iterator it = m_Children.begin();
    while ((nChild-- > 0) && (it != m_Children.end()))
    {
        it++;
    }
    if (it == m_Children.end())
    {
        return AtomPtr();
    }
    return *it;
}

AtomPtr 
Atom::FindChild(DWORD fourcc)
{
    if (ChildCount() == 0) // force enum of children
    {
        return AtomPtr();
    }

    list<AtomPtr>::iterator it = m_Children.begin();
    while (it != m_Children.end())
    {
        if ((*it)->Type() == fourcc)
        {
            return *it;
        }
        it++;
    }
    return AtomPtr();
}

AtomPtr Atom::FindNextChild(const AtomPtr& after, DWORD fourcc)
{
    if (ChildCount() == 0) // force enum of children
    {
        return AtomPtr();
    }

    if(!after)
        return FindChild(fourcc);

    list<AtomPtr>::iterator it = m_Children.begin();
    while (it != m_Children.end())
    {
        if((*it) == after)
        {
            ++it;
            break;
        }
        ++it;
    }

    while (it != m_Children.end())
    {
        if ((*it)->Type() == fourcc)
        {
            return (*it);
        }
        it++;
    }
    return AtomPtr();
}

bool 
Atom::IsBuffered()
{
    if (m_cBufferRefCount <= 0)
    {
        return m_pSource->IsBuffered();
    } 
    return true;
}

const BYTE* 
Atom::Buffer()
{
    if (m_llLength > 0x7fffffff)
    {
        return NULL;
    }

    if (!m_Buffer)
    {
        if (m_pSource->IsBuffered() && (m_llOffset < 0x7fffffff))
        {
            return m_pSource->Buffer() + DWORD(m_llOffset);
        }
        m_Buffer = new BYTE[DWORD(m_llLength)];
        Read(0, DWORD(m_llLength), m_Buffer);
    }
    m_cBufferRefCount++;
    return m_Buffer;
}

void 
Atom::BufferRelease()
{
    if (--m_cBufferRefCount == 0)
    {
        m_Buffer = NULL;
    }
}


// sample count and sizes ------------------------------------------------

TrackIndex::TrackIndex()
: m_nSamples(0),
  m_nMaxSize(0),
  m_scale(1),
  m_rate(1),
  m_total(0)
{}

DWORD TrackIndex::CTSToSample(LONGLONG tStart)
{
    if (!HasCTSTable())
    {
        return DTSToSample(tStart);
    }

    // we have a RLE list of durations and a RLE list of CTS offsets.
    // maybe start from a DTS time a little earlier, and step forward?
    LONGLONG pos = tStart;
    if (tStart > 0)
    {
        if (tStart < (UNITS/2))
        {
            pos = 0;
        }
        else
        {
            pos = tStart - (UNITS/2);
        }
    }
    DWORD n = DTSToSample(pos); 
    for (;;)
    {
        LONGLONG cts = SampleToCTS(n);
        if (cts < 0)
        {
            return -1;
        }
        LONGLONG dur = Duration(n);
        if (cts > tStart)
        {
            return n;
        }
        if ((cts <= tStart) && ((cts + dur) > tStart))
        {
            return n;
        }
        if (dur == 0)
        {
            return -1;
        }
        n++;
    }
}
LONGLONG 
TrackIndex::TrackToReftime(LONGLONG nTrack) const
{
    // convert times in the track timescale to 100ns
    return (REFERENCE_TIME(nTrack) * m_scale * UNITS) / m_rate;
}

LONGLONG TrackIndex::ReftimeToTrack(LONGLONG reftime) const
{
    return ((reftime * m_rate) + (UNITS/2)) / (UNITS * m_scale);
}

ElementaryType::ElementaryType()

{
}

bool ElementaryType::SetType(const CMediaType* pmt)
{
    debugPrintf(DEMUX_DBG, L"ElementaryType::SetType: enter\r\n");
    if (m_mtChosen != *pmt)
    {
        int idx = 0;
        CMediaType mtCompare;
        while (GetType(&mtCompare, idx))
        {
            if (mtCompare == *pmt)
            {
                debugPrintf(DEMUX_DBG, L"ElementaryType::SetType: mtCompare == *pmt, idx = %d\r\n", idx);
                m_mtChosen = *pmt;
                setHandler(pmt, idx);
                return true;
            }

            idx++;
        }
        return false;
    }
    return true;
}
// -- main movie header, contains list of tracks ---------------

Movie::Movie(const AtomReaderPtr& pRoot)
: m_pRoot(pRoot)
{
}
    
HRESULT 
Movie::ReadAbsolute(LONGLONG llPos, BYTE* pBuffer, DWORD cBytes)
{
    return m_pRoot->Read(llPos, cBytes, pBuffer);
}


// ------------------------------------------------------------------


MovieTrack::MovieTrack(const AtomPtr& pAtom, Movie* pMovie, DWORD idx)
: m_pRoot(),
  m_pMovie(pMovie),
  m_idx(idx),
  m_bOldFixedAudio(false)
{
}

bool 
MovieTrack::IsVideo()
{
    return m_pType->IsVideo();
}

bool 
MovieTrack::GetType(CMediaType* pmt, int nType)
{
    return m_pType->GetType(pmt, nType);
}
    
bool 
MovieTrack::SetType(const CMediaType* pmt)
{
    return m_pType->SetType(pmt);
}

FormatHandler* 
MovieTrack::Handler()
{
    return m_pType->Handler();
}

HRESULT 
MovieTrack::ReadSample(DWORD nSample, BYTE* pBuffer, DWORD cBytes)
{
    LONGLONG llPos = Index()->Offset(nSample);

    // llPos is absolute within referenced file
    return GetMovie()->ReadAbsolute(llPos, pBuffer, cBytes);
}

bool MovieTrack::CheckInSegment(REFERENCE_TIME tNext, bool bSyncBefore, size_t* pnSegment, DWORD* pnSample)
{
	for (size_t idx = 0; idx < m_Edits.size(); idx++)
	{
		EditEntry* it = &m_Edits[idx];
        LONGLONG endDuration = it->sumDurations + it->duration;
        if (tNext < endDuration)
		{
			if (it->offset == -1)
			{
				// empty edit; skip to next segment start
				tNext = it->sumDurations + it->duration;
			}
			else
			{
				// map to sample number
				LONGLONG rCTS = tNext - it->sumDurations;
				LONGLONG trackCTS = rCTS + it->offset;
				DWORD n = Index()->CTSToSample(trackCTS);
				if (n < 0)
				{
					return false;
				}
				if (bSyncBefore)
				{
					n = Index()->SyncFor(n);
				}

				*pnSample = n;
				*pnSegment = idx;
				return true;
			}
		}
	}

	return false;
}

void MovieTrack::GetTimeBySegment(
	DWORD nSample, 
	size_t segment, 
	REFERENCE_TIME* ptStart, 
	REFERENCE_TIME* pDuration)
{
	EditEntry* it = &m_Edits[segment];
	REFERENCE_TIME cts = Index()->SampleToCTS(nSample);
	REFERENCE_TIME relCTS = cts - it->offset;

	REFERENCE_TIME duration = Index()->Duration(nSample);
	if ((relCTS + duration) > it->duration)
	{
		duration = it->duration - relCTS;
	}
	*ptStart = relCTS + it->sumDurations;
	*pDuration = duration;
}

bool MovieTrack::NextBySegment(DWORD* pnSample, size_t* psegment)
{
	DWORD n = *pnSample + 1;
	EditEntry* it = &m_Edits[*psegment];


	if (n < Index()->SampleCount())
	{
		REFERENCE_TIME cts = Index()->SampleToCTS(n);
		REFERENCE_TIME relCTS = cts - it->offset;
		if (relCTS < it->duration)
		{
			*pnSample = n;
			return true;
		}
	}
	REFERENCE_TIME tEdit = it->duration + it->sumDurations;
	return CheckInSegment(tEdit, false, psegment, pnSample);
}

SIZE_T MovieTrack::GetTimes(REFERENCE_TIME** ppnStartTimes, REFERENCE_TIME** ppnStopTimes, ULONG** ppnFlags, ULONG** ppnDataSizes)
{
	ASSERT(ppnStartTimes);
	if(!Index())
		return 0;
	ppnStopTimes; ppnDataSizes;
	ASSERT(!ppnStopTimes && !ppnDataSizes); // Not Implemented
	const SIZE_T nSampleCount = Index()->Get(*ppnStartTimes);
	if(nSampleCount)
	{
		if(ppnFlags)
		{
			ULONG* pnFlags = (ULONG*) CoTaskMemAlloc(nSampleCount * sizeof *pnFlags);
			ASSERT(pnFlags);
			for(SIZE_T nSampleIndex = 0; nSampleIndex < nSampleCount; nSampleIndex++)
				pnFlags[nSampleIndex] = AM_SAMPLE_TIMEVALID;
            SIZE_T* pnIndexes = NULL;
            const SIZE_T nIndexCount = Index()->Get(pnIndexes);
            if(nIndexCount)
            {
                for(SIZE_T nIndexIndex = 0; nIndexIndex < nIndexCount; nIndexIndex++)
                {
                    const SIZE_T nSampleIndex = pnIndexes[nIndexIndex];
                    ASSERT(nSampleIndex < nSampleCount);
                    if(nSampleIndex < nSampleCount)
                        pnFlags[nSampleIndex] |= AM_SAMPLE_SPLICEPOINT;
                }
            } else
            {
                // NOTE: Missing key map means all samples are splice points (all frames are key frames)
                for(SIZE_T nSampleIndex = 0; nSampleIndex < nSampleCount; nSampleIndex++)
                    pnFlags[nSampleIndex] |= AM_SAMPLE_SPLICEPOINT;
            }
            CoTaskMemFree(pnIndexes);
			*ppnFlags = pnFlags;
		}
	}
	return nSampleCount;
}
