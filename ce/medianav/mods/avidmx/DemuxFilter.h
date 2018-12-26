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


#include "Avi.h"
#include "ElemType.h"
#include "DShowDemuxFilter.h"
//
// Partially derived from GDCL sample MPEG-4 parser filter
// available in source form at http://www.gdcl.co.uk/articles
//

// Declaration of AVI Demultiplexor Filter
//
// The filter has an IAsyncReader input pin and creates an output pin
// for each valid stream in the input pin.

class DECLSPEC_UUID("D24C840C-C469-4368-A363-0913B44AEF5C") // avidmx
AviDemultiplexor: public DShowDemultiplexor
{
public:
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    // filter registration tables
    static const AMOVIESETUP_FILTER m_sudFilter;

protected:
    virtual MoviePtr createMovie(const AtomReaderPtr& pRoot);

private:
    // construct only via class factory
    AviDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr);
};



