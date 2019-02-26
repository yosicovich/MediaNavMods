//
// Mpeg1AudioDemultiplexor.h
//
#pragma once

#include "mp3.h"
#include "ElemType.h"
#include "DShowDemuxFilter.h"

// Declaration of Mpeg1 Audio Demultiplexor Filter
//
// The filter has an IAsyncReader input pin and creates an output pin
// for each valid stream in the input pin.

class DECLSPEC_UUID("DC56E099-C9EA-49c8-9FA2-0A173D1522F1") // mp3demux
Mpeg1AudioDemultiplexor: public DShowDemultiplexor
{
public:
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

protected:
    virtual MoviePtr createMovie(const AtomReaderPtr& pRoot);

private:
    // construct only via class factory
    Mpeg1AudioDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr);
};



