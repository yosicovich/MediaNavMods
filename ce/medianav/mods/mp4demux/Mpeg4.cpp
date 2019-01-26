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
#include "Mpeg4.h"
#include "ElemType.h"
#include "Index.h"
#include <sstream>

Mpeg4Atom::Mpeg4Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, DWORD cHeader)
: Atom(pReader, llOffset, llLength, type, cHeader)
{
}

void 
Mpeg4Atom::ScanChildrenAt(LONGLONG llOffset)
{
    llOffset += m_cHeader;

    while (llOffset < m_llLength)
    {
        BYTE hdr[8];
        DWORD cHeader = 8;
        Read(llOffset, 8, hdr);
        LONGLONG llLength = (DWORD)SwapLong(hdr);
        DWORD type = (DWORD)SwapLong(hdr + 4);
        if (llLength == 1)
        {
            Read(llOffset + 8, 8, hdr);
            llLength = SwapI64(hdr);
            cHeader += 8;
        }
        else if (llLength == 0)
        {
            // whole remainder
            llLength = m_llLength - llOffset;
        }
        if (type == FOURCC("uuid"))
        {
            cHeader += 16;
        }
        
        if ((llLength < 0) || ((llOffset + llLength) > (m_llLength)))
        {
            break;
        }

        AtomPtr pChild = AtomPtr(new Mpeg4Atom(this, llOffset, llLength, type, cHeader));
        m_Children.push_back(pChild);

        llOffset += llLength;
    }
}

// -- main movie header, contains list of tracks ---------------

Mpeg4Movie::Mpeg4Movie(const AtomReaderPtr& pRoot)
: Movie(pRoot)
{
    AtomPtr pMp4 = AtomPtr(new Mpeg4Atom(pRoot.get(), 0, pRoot->Length(), 0, 0));
    const AtomPtr& pMovie = pMp4->FindChild(FOURCC("moov"));
    if (pMovie != NULL)
    {
        const AtomPtr& patmHdr = pMovie->FindChild(FOURCC("mvhd"));
        if (patmHdr != NULL)
        {
            AtomCache phdr(patmHdr);

            if (phdr[0] == 1)   // version 0 or 1
            {
                m_scale = SwapLong(phdr + 20);
                m_duration = SwapI64(phdr + 24);
            } else {
                m_scale = SwapLong(phdr + 12);
                m_duration = SwapLong(phdr + 16);
            }
            m_tDuration = REFERENCE_TIME(m_duration) * UNITS / LONGLONG(m_scale);
        }

        // list tracks
        DWORD idxTrack = 0;
        for (DWORD i = 0; i < pMovie->ChildCount(); i++)
        {
            const AtomPtr& patm = pMovie->Child(i);
            if ((patm->Type() == FOURCC("trak")) ||
                (patm->Type() == FOURCC("cctk")))
            {
                MovieTrackPtr pTrack = MovieTrackPtr(new Mpeg4MovieTrack(patm, this, idxTrack++));
#ifdef _DEBUG
                if (pTrack->Valid() && pTrack->IsVideo())
#else
                if (pTrack->Valid())
#endif
                {
                    m_Tracks.push_back(pTrack);
                }
            }
        }
    }
}

// ------------------------------------------------------------------


Mpeg4MovieTrack::Mpeg4MovieTrack(const AtomPtr& pAtom, Mpeg4Movie* pMovie, DWORD idx)
: MovieTrack(AtomPtr(), pMovie, idx)
{
    // check version/flags entry for track header
    const AtomPtr& pHdr = pAtom->FindChild(FOURCC("tkhd"));
    if (pHdr != NULL)
    {
        BYTE verflags[4];
        pHdr->Read(pHdr->HeaderSize(), 4, verflags);
        if ((verflags[3] & 1) == 1)      // enabled?
        {
            // edit list may contain offset for first sample
            REFERENCE_TIME tFirst = 0;
            const AtomPtr& patmEDTS = pAtom->FindChild(FOURCC("edts"));
            if (patmEDTS != NULL)
            {
                LONGLONG first = ParseEDTS(patmEDTS);
                // convert from movie scale to 100ns
                tFirst = first * UNITS / pMovie->Scale();
            }

            const AtomPtr& patmMDIA = pAtom->FindChild(FOURCC("mdia"));
            if (patmMDIA != NULL && ParseMDIA(patmMDIA, tFirst))
            {
                // valid track -- make a name for the pin to use
                ostringstream strm;
                strm << m_pType->ShortName() << " ";
                strm << m_idx+1;
                m_strName = strm.str();

                m_pRoot = pAtom;

                // sum up and convert ELST for efficiency later
                if (m_Edits.size() > 0)
                {
                    LONGLONG sumDurations = 0;
                    for (vector<EditEntry>::iterator it = m_Edits.begin(); it != m_Edits.end(); it++)
                    {
                        // duration is in movie scale; offset is in track scale. 
                        it->duration = it->duration * UNITS / pMovie->Scale();
                        if (it->offset > 0)
                        {
                            it->offset = Index()->TrackToReftime(it->offset);
                        }
                        it->sumDurations = sumDurations;
                        sumDurations += it->duration;
                    }
                }
                else
                {
                    // create dummy edit
                    EditEntry e;
                    e.offset = 0;
                    e.sumDurations = 0;
                    e.duration = Index()->TotalDuration();
                    m_Edits.push_back(e);
                }
            }
        }
    }
}
REFERENCE_TIME Mpeg4MovieTrack::Duration() const
{
    return reinterpret_cast<const Mpeg4Movie*>(GetMovie())->Duration();
}

LONGLONG 
Mpeg4MovieTrack::ParseEDTS(const AtomPtr& patm)
{
    const AtomPtr& patmELST = patm->FindChild(FOURCC("elst"));
    if (patmELST != NULL)
    {
		AtomCache pELST = patmELST;
		DWORD cEdits = SwapLong(pELST + 4);
		for (DWORD i = 0; i < cEdits; i++)
        {
			EditEntry e;
            if (pELST[0] == 0)
            {
				e.duration = SwapLong(pELST + 8 + (i * 12));
				e.offset = SwapLong(pELST + 8 + 4 + (i * 12));
            }
			else 
            {
				e.duration = SwapI64(pELST + 8 + (i * 20));
				e.offset = SwapI64(pELST + 8 + 8 + (i * 20));
            }
			m_Edits.push_back(e);
        }
    }
	// always return 0 for start offset. We now add the ELST above this layer, so the TimesIndex()
	// should return the actual start of the media, not an offset time. Thus use 0 here.
    return 0;
}

// parse the track type information
bool
Mpeg4MovieTrack::ParseMDIA(const AtomPtr& patm, REFERENCE_TIME tFirst)
{
    // get track timescale from mdhd
    const AtomPtr& patmMDHD = patm->FindChild(FOURCC("mdhd"));
    if (!patmMDHD)
    {
        return false;
    }
    AtomCache pMDHD(patmMDHD);
    DWORD scale;
    if (pMDHD[0] == 1)  // version 0 or 1
    {
        scale = SwapLong(pMDHD + 20);
    } else {
        scale = SwapLong(pMDHD + 12);
    }


    // locate and parse the ES_Descriptor
    // that will give us the media type for this
    // track. That is in minf/stbl/stsd

    const AtomPtr& patmMINF = patm->FindChild(FOURCC("minf"));
    if (!patmMINF)
    {
        return false;
    }
    const AtomPtr& patmSTBL = patmMINF->FindChild(FOURCC("stbl"));
    if (!patmSTBL)
    {
        return false;
    }

    // initialize index
    m_pIndex = new Mpeg4TrackIndex();
    if ((!GetTypedPtr(Mpeg4TrackIndex, m_pIndex)->Parse(scale, tFirst, patmSTBL) || (m_pIndex->SampleCount() <= 0)))
    {
        return false;
    }

    // now index is ready, we can calculate average frame duration
    // for the media type
    REFERENCE_TIME tFrame = Duration() / Index()->SampleCount();

    const AtomPtr& pSTSD = patmSTBL->FindChild(FOURCC("stsd"));
    if (!pSTSD || !ParseSTSD(tFrame, pSTSD))
    {
        return false;
    }

	// check for old-format uncomp audio
	if ((GetTypedPtr(Mpeg4ElementaryType, m_pType)->StreamType() == Audio_WAVEFORMATEX) &&
		(Index()->Size(0) == 1))
	{
		CMediaType mt;
		m_pType->GetType(&mt, 0);
		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)mt.Format();
		GetTypedPtr(Mpeg4TrackIndex, Index())->AdjustFixedSize(pwfx->nBlockAlign);
		m_bOldFixedAudio = true;
	}

	if (IsVideo())
	{
		// attempt to normalise the frame rate

		// first average the first few, discarding outliers
		int cFrames = min(Index()->SampleCount(), 60L);
		REFERENCE_TIME total = 0;
		int cCounted = 0;
		// outliers are beyond +/- 20%
		REFERENCE_TIME tMin = tFrame * 80 / 100;
		REFERENCE_TIME tMax = tFrame * 120 / 100;
		for (int i = 0; i < cFrames; i++)
		{
			REFERENCE_TIME tDur = Index()->Duration(i);
			if ((tDur > tMin) && (tDur < tMax))
			{
				total += tDur;
				cCounted++;
			}
		}
		if (cCounted > 2)
		{
			tFrame = (total / cCounted);

			LONGLONG fpsk = (UNITS * 1000 / tFrame);
			if (fpsk > 23950)
			{
				if (fpsk < 23988)
				{
					fpsk = 23976;
				}
				else if (fpsk < 24100)
				{
					fpsk = 24000;
				}
				else if (fpsk > 24800)
				{
					if (fpsk < 25200)
					{
						fpsk = 25000;
					} 
					else if (fpsk > 29800)
					{
						if (fpsk < 29985)
						{
							fpsk = 29970;
						}
						else if (fpsk < 30200)
						{
							fpsk = 30000;
						}
					}
				}
			}
			tFrame = (UNITS * 1000 / fpsk);
			GetTypedPtr(Mpeg4ElementaryType, m_pType)->SetRate(tFrame);
		}

	}

    return true;
}

// locate and parse the ES_Descriptor    
bool 
Mpeg4MovieTrack::ParseSTSD(REFERENCE_TIME tFrame, const AtomPtr& pSTSD)
{
    // We don't accept files with format changes mid-track,
    // so there must be only one descriptor entry 
    // -- and the spec only defines version 0, so validate both these.
    BYTE ab[8];
    pSTSD->Read(pSTSD->HeaderSize(), 8, ab);
    if ((ab[0] !=0) || (SwapLong(ab+4) != 1))
    {
        return false;
    }
    pSTSD->ScanChildrenAt(8);
    const AtomPtr& patm = pSTSD->Child(0);
    if (!patm)
    {
        return false;
    }

    m_pType = new Mpeg4ElementaryType();
    if (!GetTypedPtr(Mpeg4ElementaryType, m_pType)->Parse(tFrame, patm))
    {
        return false;
    }
    return true;
}
