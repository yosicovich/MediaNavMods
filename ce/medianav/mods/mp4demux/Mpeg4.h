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

// MPEG-4 files (based on QuickTime) are comprised of file elements
// with a length and 4-byte FOURCC type code. This basic header can
// be extended with an 8-byte length and a 16-byte type GUID. 
// 
// The payload can contain other atoms recursively.
//
// Some atoms begin with a 1-byte version number and 3-byte flags field,
// but as this is type-specific, we regard it as part of the payload.
//
// This Atom implementation accesses data
// via the AtomReader abstraction. The data source may be
// a containing atom or access to the file (perhaps via an input pin).
class Mpeg4Atom : public Atom
{
public:
    // all the header params are parsed in the "Child" method and passed
    // to the constructor. This means we can use the same class for the outer file
    // container (which does not have a header).
    Mpeg4Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader);
    // call this if the child items do not start immediately after the header
    // -- llOffset is an offset from HeaderSize
    virtual void ScanChildrenAt(LONGLONG llOffset);
};
// --- movie and track headers ---------------------------
class Mpeg4Movie;

class Mpeg4MovieTrack: public MovieTrack
{
public:
    Mpeg4MovieTrack(const AtomPtr& pAtom, Mpeg4Movie* pMovie, long idx);
    virtual REFERENCE_TIME Duration() const;

private:
    bool ParseMDIA(const AtomPtr& patm, REFERENCE_TIME tFirst);
    bool ParseSTSD(REFERENCE_TIME tFrame, const AtomPtr& pSTSD);
    LONGLONG ParseEDTS(const AtomPtr& patm);
};

class Mpeg4Movie: public Movie
{
public:
    Mpeg4Movie(const AtomReaderPtr& pRoot);
    LONGLONG Scale() const
    {
        return m_scale;
    }
    REFERENCE_TIME Duration() const
    {
        return m_tDuration;
    }
private:
    long m_scale;
    LONGLONG m_duration;
    REFERENCE_TIME m_tDuration;
};





