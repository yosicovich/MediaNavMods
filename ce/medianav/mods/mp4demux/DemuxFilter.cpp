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
#include "DemuxFilter.h"

// --- registration tables ----------------

// filter registration information. 
const AMOVIESETUP_FILTER 
Mpeg4Demultiplexor::m_sudFilter = 
{
    &__uuidof(Mpeg4Demultiplexor),  // filter clsid
    L"Mpeg-4 Demultiplexor",        // filter name
    MERIT_NORMAL,                   // ie default for auto graph building
    3,                              // count of registered pins
    DShowDemultiplexor::m_sudPin    // list of pins to register
};

// ---- construction/destruction and COM support -------------

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
Mpeg4Demultiplexor::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new Mpeg4Demultiplexor(pUnk, phr);
}

Mpeg4Demultiplexor::Mpeg4Demultiplexor(LPUNKNOWN pUnk, HRESULT* phr)
:DShowDemultiplexor(pUnk, phr, *m_sudFilter.clsID)
{
    setAlwaysSeekToKeyFrame(Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, TEXT("\\SOFTWARE\\Microsoft\\DirectX\\DirectShow\\MP4Demux"), TEXT("KeyFrameSeeking"), true));
}

Atom* Mpeg4Demultiplexor::createAtom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader)
{
    return new Mpeg4Atom(pReader, llOffset, llLength, type, cHeader);
}

Movie* Mpeg4Demultiplexor::createMovie(Atom* pRoot)
{
    return new Mpeg4Movie(pRoot);
}
