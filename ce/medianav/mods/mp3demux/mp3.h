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

class MP3Atom : public Atom
{
public:
    // all the header params are parsed in the "Child" method and passed
    // to the constructor. This means we can use the same class for the outer file
    // container (which does not have a header).
    MP3Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type);
    // call this if the child items do not start immediately after the header
    // -- llOffset is an offset from HeaderSize
    virtual void ScanChildrenAt(LONGLONG llOffset);
private:
    static DWORD m_scanWithin;
};
// --- movie and track headers ---------------------------
class MP3Movie;

class MP3MovieTrack: public MovieTrack
{
public:
    MP3MovieTrack(const AtomPtr& pAtom, MP3Movie* pMovie, DWORD idx);
    virtual REFERENCE_TIME Duration() const;
};

class MP3Movie: public Movie
{
public:
    MP3Movie(const AtomReaderPtr& pRoot);
};





