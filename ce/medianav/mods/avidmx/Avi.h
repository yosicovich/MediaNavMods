//
// Mpeg4.cpp: definition of Mpeg-4 parsing classes
//
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////

#pragma once

#include "StdAfx.h"
#include "Index.h"
#include "ElemType.h"
#include <aviriff.h>

// This Atom implementation accesses data
// via the AtomReader abstraction. The data source may be
// a containing atom or access to the file (perhaps via an input pin).
class AviAtom : public Atom
{
public:
    // all the header params are parsed in the "Child" method and passed
    // to the constructor. This means we can use the same class for the outer file
    // container (which does not have a header).
    AviAtom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader, bool canHaveChildren = true);
    // call this if the child items do not start immediately after the header
    // -- llOffset is an offset from HeaderSize
    virtual void ScanChildrenAt(LONGLONG llOffset);
};
// --- movie and track headers ---------------------------

class AviMovieTrack: public MovieTrack
{
public:
    AviMovieTrack(const AtomPtr& pAtom, Movie* pMovie, long idx, const AtomCache& pIndex, unsigned int offsetOfOffset);
    virtual REFERENCE_TIME Duration() const;
    bool isDisabled() const
    {
        return m_disabled;
    }

private:
    bool m_disabled;
};

class AviMovie: public Movie
{
public:
    AviMovie(const AtomReaderPtr& pRoot);

private:
    bool m_hasVideo;
    bool m_hasAudio;
    bool setTracks(const MovieTracks& sourceTracks);
};





