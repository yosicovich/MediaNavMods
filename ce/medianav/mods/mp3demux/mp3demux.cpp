// mp4demux.cpp : Defines the entry point for the DLL.
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk

#include "stdafx.h"
#include "Mpeg1AudioDemultiplexor.h"

// --- registration tables ----------------

// filter registration -- these are the types that our
// pins accept and produce
static const AMOVIESETUP_MEDIATYPE filterInputSubTypes[] =
{
    {
        &MEDIATYPE_Stream,      // Major type
        &MEDIASUBTYPE_NULL      // Minor type
    },
    {
        &MEDIATYPE_Stream,      // Major type
        &MEDIASUBTYPE_MPEG1Audio// Minor type
    }
};

static const AMOVIESETUP_MEDIATYPE filterOutputSubTypes =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};


// registration of our pins for auto connect and render operations
static const AMOVIESETUP_PIN filterPins[] = 
{
    {
            L"Input",           // pin name
            FALSE,              // is rendered?    
            FALSE,              // is output?
            FALSE,              // zero instances allowed?
            FALSE,              // many instances allowed?
            &CLSID_NULL,        // connects to filter (for bridge pins)
            NULL,               // connects to pin (for bridge pins)
            1,                  // count of registered media types
            filterInputSubTypes// list of registered media types    
    },
    {
            L"Output",           // pin name
            FALSE,              // is rendered?    
            TRUE,               // is output?
            FALSE,              // zero instances allowed?
            FALSE,              // many instances allowed?
            &CLSID_NULL,        // connects to filter (for bridge pins)
            NULL,               // connects to pin (for bridge pins)
            1,                  // count of registered media types
            &filterOutputSubTypes// list of registered media types    
    }
};

// filter registration information. 
const AMOVIESETUP_FILTER filter = 
{
    &__uuidof(Mpeg1AudioDemultiplexor),  // filter clsid
    L"Mpeg1 Audio Demultiplexor",// filter name
    MERIT_NORMAL,           // ie default for auto graph building
    2,                      // count of registered pins
    filterPins              // list of pins to register
};

// --- COM factory table and registration code --------------

// DirectShow base class COM factory requires this table, 
// declaring all the COM objects in this DLL
CFactoryTemplate g_Templates[] = {
    // one entry for each CoCreate-able object
    {
        filter.strName,
        filter.clsID,
        Mpeg1AudioDemultiplexor::CreateInstance,
        NULL,
        &filter
    },
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// self-registration entrypoint
STDAPI DllRegisterServer()
{
    // base classes will handle registration using the factory template table
    HRESULT hr = AMovieDllRegisterServer2(true);
    return hr;
}

STDAPI DllUnregisterServer()
{
    // base classes will handle de-registration using the factory template table
    HRESULT hr = AMovieDllRegisterServer2(false);
    return hr;
}

// if we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
    return DllEntryPoint(reinterpret_cast<HINSTANCE>(hDllHandle), dwReason, lpReserved);
}

