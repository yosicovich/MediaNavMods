//
// Mpeg4.cpp: implementation of Mpeg-4 parsing classes
//
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "demuxtypes.h"
#include <sstream>

Atom::Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader, bool canHaveChildren/* = true*/)
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
Atom::Read(LONGLONG llOffset, long cBytes, void* pBuffer)
{
    HRESULT hr = S_OK;
    if (IsBuffered())
    {
        const BYTE* pSrc = Buffer() + long(llOffset);
        CopyMemory(pBuffer,  pSrc,  cBytes);
        BufferRelease();
    } else {
        hr = m_pSource->Read(llOffset + m_llOffset, cBytes, pBuffer);
    }
    return hr;
}
   

long 
Atom::ChildCount()
{
    if (m_Children.size() == 0 && !m_bChildrenScaned)
    {
        ScanChildrenAt(0);
        m_bChildrenScaned = true;
    }
    return (long)m_Children.size();
}

Atom* 
Atom::Child(long nChild)
{
    if (nChild >= ChildCount())
    {
        return NULL;
    }
    list<AtomPtr>::iterator it = m_Children.begin();
    while ((nChild-- > 0) && (it != m_Children.end()))
    {
        it++;
    }
    if (it == m_Children.end())
    {
        return NULL;
    }
    return *it;
}

Atom* 
Atom::FindChild(DWORD fourcc)
{
    if (ChildCount() == 0) // force enum of children
    {
        return NULL;
    }

    list<AtomPtr>::iterator it = m_Children.begin();
    while (it != m_Children.end())
    {
        Atom* pChild = *it;
        if (pChild->Type() == fourcc)
        {
            return pChild;
        }
        it++;
    }
    return NULL;
}

Atom* Atom::FindNextChild(Atom* after, DWORD fourcc)
{
    if (ChildCount() == 0) // force enum of children
    {
        return NULL;
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
        Atom* pChild = *it;
        if (pChild->Type() == fourcc)
        {
            return pChild;
        }
        it++;
    }
    return NULL;
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
            return m_pSource->Buffer() + long(m_llOffset);
        }
        m_Buffer = new BYTE[long(m_llLength)];
        Read(0, long(m_llLength), m_Buffer);
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

SampleSizes::SampleSizes()
: m_nSamples(0),
  m_nMaxSize(0)
{}

// ----- times index ----------------------------------

SampleTimes::SampleTimes()
: m_scale(1),
  m_rate(1),
  m_total(0)
{
}

long SampleTimes::CTSToSample(LONGLONG tStart)
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
    long n = DTSToSample(pos); 
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
SampleTimes::TrackToReftime(LONGLONG nTrack) const
{
    // convert times in the track timescale to 100ns
    return (REFERENCE_TIME(nTrack) * m_scale * UNITS) / m_rate;
}

LONGLONG SampleTimes::ReftimeToTrack(LONGLONG reftime) const
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

Movie::Movie(Atom* pRoot)
: m_pRoot(pRoot)
{
}
    
HRESULT 
Movie::ReadAbsolute(LONGLONG llPos, BYTE* pBuffer, long cBytes)
{
    return m_pRoot->Read(llPos, cBytes, pBuffer);
}


// ------------------------------------------------------------------


MovieTrack::MovieTrack(Atom* pAtom, Movie* pMovie, long idx)
: m_pRoot(NULL),
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
MovieTrack::ReadSample(long nSample, BYTE* pBuffer, long cBytes)
{
    LONGLONG llPos = m_pSizes->Offset(nSample);

    // llPos is absolute within referenced file
    return GetMovie()->ReadAbsolute(llPos, pBuffer, cBytes);
}

bool MovieTrack::CheckInSegment(REFERENCE_TIME tNext, bool bSyncBefore, size_t* pnSegment, long* pnSample)
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
				long n = TimesIndex()->CTSToSample(trackCTS);
				if (n < 0)
				{
					return false;
				}
				if (bSyncBefore)
				{
					n = GetKeyMap()->SyncFor(n);
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
	long nSample, 
	size_t segment, 
	REFERENCE_TIME* ptStart, 
	REFERENCE_TIME* pDuration)
{
	EditEntry* it = &m_Edits[segment];
	REFERENCE_TIME cts = TimesIndex()->SampleToCTS(nSample);
	REFERENCE_TIME relCTS = cts - it->offset;

	REFERENCE_TIME duration = TimesIndex()->Duration(nSample);
	if ((relCTS + duration) > it->duration)
	{
		duration = it->duration - relCTS;
	}
	*ptStart = relCTS + it->sumDurations;
	*pDuration = duration;
}

bool MovieTrack::NextBySegment(long* pnSample, size_t* psegment)
{
	int n = *pnSample + 1;
	EditEntry* it = &m_Edits[*psegment];


	if (n < SizeIndex()->SampleCount())
	{
		REFERENCE_TIME cts = TimesIndex()->SampleToCTS(n);
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
	if(!TimesIndex())
		return 0;
	ppnStopTimes; ppnDataSizes;
	ASSERT(!ppnStopTimes && !ppnDataSizes); // Not Implemented
	const SIZE_T nSampleCount = TimesIndex()->Get(*ppnStartTimes);
	if(nSampleCount)
	{
		if(ppnFlags)
		{
			ULONG* pnFlags = (ULONG*) CoTaskMemAlloc(nSampleCount * sizeof *pnFlags);
			ASSERT(pnFlags);
			for(SIZE_T nSampleIndex = 0; nSampleIndex < nSampleCount; nSampleIndex++)
				pnFlags[nSampleIndex] = AM_SAMPLE_TIMEVALID;
			if(GetKeyMap())
			{
				SIZE_T* pnIndexes = NULL;
				const SIZE_T nIndexCount = GetKeyMap()->Get(pnIndexes);
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
			}
			*ppnFlags = pnFlags;
		}
	}
	return nSampleCount;
}
