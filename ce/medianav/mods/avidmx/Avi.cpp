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
#include "Avi.h"
#include "ElemType.h"
#include "Index.h"
#include <sstream>

#define ckidAVI FCC('AVI ')
#define ckidRIFF FCC('RIFF')
#define ckidLIST FCC('LIST')
#define listtypeAVIHEADER FCC('hdrl')
#define listtypeAVIMOVIE FCC('movi')
#define ckidSTREAMNAME FCC('strn')

AviAtom::AviAtom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader, bool canHaveChildren/* = true*/)
: Atom(pReader, llOffset, llLength, type, cHeader, canHaveChildren)
{
}

void 
AviAtom::ScanChildrenAt(LONGLONG llOffset)
{
    debugPrintf(DBG, L"AviAtom::ScanChildrenAt: llOffset=%I64u\r\n", llOffset);
    llOffset += m_cHeader;

    while (llOffset < m_llLength)
    {
        RIFFCHUNK chunk;
        long cHeader = sizeof(chunk);
        if(Read(llOffset, sizeof(chunk), reinterpret_cast<BYTE *>(&chunk)) != S_OK)
            break;
        debugPrintf(DBG, L"AviAtom::ScanChildrenAt: chunk.fcc=%08X, chunk.cb=%u\r\n", chunk.fcc, chunk.cb);
        
        LONGLONG llLength = chunk.cb + sizeof(chunk);
        DWORD type = chunk.fcc;
        bool canHaveChildren = false;
        if(type == ckidRIFF || type == ckidLIST)
        {
            debugPrintf(DBG, L"AviAtom::ScanChildrenAt: RIFF or LIST found\r\n");
            if(Read(llOffset + cHeader, sizeof(type), reinterpret_cast<BYTE *>(&type)) != S_OK)
                break;
            debugPrintf(DBG, L"AviAtom::ScanChildrenAt: LIST type %08X\r\n", type);
            cHeader += sizeof(type);
            canHaveChildren = true;
        }
        
        if ((llLength < 0) || ((llOffset + llLength) > (m_llLength)))
        {
            debugPrintf(DBG, L"AviAtom::ScanChildrenAt: bad list size llLength=%I64d\r\n", llLength);
            break;
        }

        AtomPtr pChild = new AviAtom(this, llOffset, llLength, type, cHeader, canHaveChildren);
        m_Children.push_back(pChild);
        debugPrintf(DBG, L"AviAtom::ScanChildrenAt: child has been created\r\n");

        llOffset += llLength;
    }
}

// -- main movie header, contains list of tracks ---------------

AviMovie::AviMovie(Atom* pRoot)
: Movie(pRoot)
{
    debugPrintf(DBG, L"AviMovie::AviMovie\r\n");
/*    Atom* pMovie = m_pRoot->FindChild(ckidRIFF);
    if(pMovie == NULL)
        return;*/

    //debugPrintf(DBG, L"AviMovie::AviMovie: found RIFF\r\n");
    Atom* pMovie = m_pRoot->FindChild(ckidAVI);
    if(pMovie == NULL)
        return; // Badly formed file or not AVI format

    debugPrintf(DBG, L"AviMovie::AviMovie: found 'AVI '\r\n");
    Atom* pHdrl = pMovie->FindChild(listtypeAVIHEADER);
    if(pHdrl == NULL)
        return;

    debugPrintf(DBG, L"AviMovie::AviMovie: found hdrl\r\n");
    Atom* pAvih = pHdrl->FindChild(ckidMAINAVIHEADER);
    if(pAvih == NULL)
        return;

    debugPrintf(DBG, L"AviMovie::AviMovie: found avih\r\n");
    // Read main header
    if(pAvih->Length() < sizeof(AVIMAINHEADER))
        return;
    AVIMAINHEADER avimainHeader;
    if(pAvih->Read(0, sizeof(avimainHeader), &avimainHeader) != S_OK)
        return;

    debugPrintf(DBG, L"AviMovie::AviMovie: avih is valid\r\n");
    Atom* pMovi = pMovie->FindChild(listtypeAVIMOVIE);
    if(pMovi == NULL)
        return; // No movi list!!!
    
    debugPrintf(DBG, L"AviMovie::AviMovie: found movi\r\n");
    Atom* pIdx1 = pMovie->FindChild(ckidAVIOLDINDEX);
    // We don't support non-indexed files now!
    if(pIdx1 == NULL)
        return;

    debugPrintf(DBG, L"AviMovie::AviMovie: found idx1\r\n");
    AtomCache pIndex(pIdx1);

    debugPrintf(DBG, L"AviMovie::AviMovie: pIndex.getRawBuffer() = %08x\r\n", pIndex.getRawBuffer());
    const AVIOLDINDEX* pIndexArray = reinterpret_cast<const AVIOLDINDEX*>(pIndex.getRawBuffer());
    unsigned int indexLength = pIndexArray->cb / sizeof(struct AVIOLDINDEX::_avioldindex_entry);
    
    if(!indexLength)
        return; //Bad index table

    debugPrintf(DBG, L"AviMovie::AviMovie: idx1 is valid\r\n");
    // Determine "Offset of offset"
    unsigned int offsetOfOffset = static_cast<unsigned int>(pMovi->Offset() + pMovi->HeaderSize() - pIndexArray->aIndex[0].dwOffset);

    debugPrintf(DBG, L"AviMovie::AviMovie: offsetOfOffset=%d, pMovi->Offset() = %I64d, pMovi->HeaderSize() = %u, pIndexArray->aIndex[0].dwOffset = %u\r\n", offsetOfOffset, pMovi->Offset(), pMovi->HeaderSize(), pIndexArray->aIndex[0].dwOffset);
    Atom* pStrl = NULL;
    // Read streams
    long idxTrack = 0;
    while((pStrl = pHdrl->FindNextChild(pStrl, ckidSTREAMLIST)))
    {
        debugPrintf(DBG, L"AviMovie::AviMovie: enumirating tracks, idxTrack=%d\r\n", idxTrack);
        MovieTrackPtr pTrack = new AviMovieTrack(pStrl, this, idxTrack++, pIndex, offsetOfOffset);
        if (pTrack->Valid())
        {
            m_Tracks.push_back(pTrack);
        }
    }
    debugPrintf(DBG, L"AviMovie::AviMovie: Finished\r\n");
}

// ------------------------------------------------------------------


AviMovieTrack::AviMovieTrack(Atom* pAtom, Movie* pMovie, long idx, const AtomCache& pIndex, unsigned int offsetOfOffset)
: MovieTrack(NULL, pMovie, idx)
{
    Atom* pStrh = pAtom->FindChild(ckidSTREAMHEADER);
    if(pStrh == NULL)
        return;

    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: found strh\r\n");
    AVISTREAMHEADER streamHeader;
    if(pStrh->Read(0,sizeof(AVISTREAMHEADER), &streamHeader) != S_OK)
        return;

    Atom* pStrf = pAtom->FindChild(ckidSTREAMFORMAT);
    if(pStrf == NULL)
        return;

    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: found strf\r\n");
    m_pType = new AviElementaryType();
    if (!GetTypedPtr(AviElementaryType, m_pType)->Parse(streamHeader, pStrf))
    {
        return;
    }
    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: AviElementaryType created\r\n");
    
    const AVIOLDINDEX* pIndexArray = reinterpret_cast<const AVIOLDINDEX*>(pIndex.getRawBuffer());

    m_pSizes = new AviSampleSizes;
    if ((!GetTypedPtr(AviSampleSizes, m_pSizes)->Parse(streamHeader, m_idx, pIndexArray, offsetOfOffset) || (m_pSizes->SampleCount() <= 0)))
    {
        return;
    }
    m_pKeyMap = new AviKeyMap(m_pSizes);
    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: AviSampleSizes and AviKeyMap are created\r\n");

    m_pTimes = new AviSampleTimes(streamHeader, m_pSizes);

    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: AviSampleTimes created\r\n");
    Atom* pStrn = pAtom->FindChild(ckidSTREAMNAME);
    if(pStrn)
    {
       AtomCache pbuf(pStrn);
       m_strName.assign(reinterpret_cast<const char *>(*pbuf), static_cast<unsigned int>(pbuf.getDataSize()));
       debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: strn found\r\n");
    }else
    {
       ostringstream strm;
       strm << m_pType->ShortName() << " ";
       strm << m_idx+1;
       m_strName = strm.str();
    }
    debugPrintf(DBG, L"AviMovieTrack::AviMovieTrack: m_strName=%S\r\n", m_strName.c_str());

    // create dummy edit
    EditEntry e;
    e.offset = 0;
    e.sumDurations = 0;
    e.duration = TimesIndex()->TotalDuration();
    m_Edits.push_back(e);

    m_pRoot = pAtom;
}

REFERENCE_TIME AviMovieTrack::Duration() const
{
    return m_pTimes->TotalDuration();
}
