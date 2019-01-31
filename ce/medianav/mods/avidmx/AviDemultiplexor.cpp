//
// AviDemultiplexor.cpp
// 

#include "stdafx.h"
#include "AviDemultiplexor.h"

// the class factory calls this to create the filter
//static 
CUnknown* WINAPI 
AviDemultiplexor::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return new AviDemultiplexor(pUnk, phr);
}

AviDemultiplexor::AviDemultiplexor(LPUNKNOWN pUnk, HRESULT* phr)
:DShowDemultiplexor(pUnk, phr, __uuidof(AviDemultiplexor))
{
    setAlwaysSeekToKeyFrame(Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, TEXT("\\SOFTWARE\\Microsoft\\DirectX\\DirectShow\\AVIDemux"), TEXT("KeyFrameSeeking"), true));
}

MoviePtr AviDemultiplexor::createMovie(const AtomReaderPtr& pRoot)
{
    return MoviePtr(new AviMovie(pRoot));
}
