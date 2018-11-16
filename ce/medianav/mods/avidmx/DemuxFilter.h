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


#include "mpeg4.h"
#include "ElemType.h"
#include "DShowDemuxFilter.h"
//
// Partially derived from GDCL sample MPEG-1 parser filter
// available in source form at http://www.gdcl.co.uk/articles
//

// Declaration of Mpeg-4 Demultiplexor Filter
//
// The filter has an IAsyncReader input pin and creates an output pin
// for each valid stream in the input pin.

//class DECLSPEC_UUID("025BE2E4-1787-4da4-A585-C5B2B9EEB57C") // mp4demux
class DECLSPEC_UUID("D24C840C-C469-4368-A363-0913B44AEF5C") // avidmx
Mpeg4Demultiplexor: public DShowDemultiplexor
{
public:
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    // filter registration tables
    static const AMOVIESETUP_FILTER m_sudFilter;

protected:
    virtual Atom* createAtom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader);
    virtual Movie* createMovie(Atom* pRoot);

private:
    // construct only via class factory
    Mpeg4Demultiplexor(LPUNKNOWN pUnk, HRESULT* phr);
};



