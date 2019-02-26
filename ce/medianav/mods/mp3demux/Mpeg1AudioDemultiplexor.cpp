//
// Mpeg1AudioDemultiplexor.cpp
// 

#include "stdafx.h"
#include "Mpeg1AudioDemultiplexor.h"

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
Mpeg1AudioDemultiplexor::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new Mpeg1AudioDemultiplexor(pUnk, phr);
}

Mpeg1AudioDemultiplexor::Mpeg1AudioDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr)
:DShowDemultiplexor(pUnk, phr, __uuidof(Mpeg1AudioDemultiplexor))
{
}

MoviePtr Mpeg1AudioDemultiplexor::createMovie(const AtomReaderPtr& pRoot)
{
    return MoviePtr(new MP3Movie(pRoot));
}
