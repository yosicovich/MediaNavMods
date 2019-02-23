#include "stdafx.h"
#include "MP3DecoderFilter.h"
#include <audshow.h>

// --- registration tables ----------------
// filter registration information. 
static const AMOVIESETUP_MEDIATYPE filterInSubTypes[] =
{
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_MP3       // Minor type
    },
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_MPEG1AudioPayload // Minor type
    },
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_MPEG1Payload // Minor type
    },
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_MPEG1Packet // Minor type
    },
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_DIVX_ADRM // Minor type
    },
    {
        &MEDIATYPE_Audio,       // Major type
        &MEDIASUBTYPE_MPEG2_AUDIO // Minor type
    }
};

static const AMOVIESETUP_MEDIATYPE filterOutSubTypes =
{
    &MEDIATYPE_Audio,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

static const AMOVIESETUP_PIN filterPins[] =
{
    { L"Input",             // Pins string name
    FALSE,                // Is it rendered
    FALSE,                // Is it an output
    FALSE,                // Are we allowed none
    FALSE,                // And allowed many
    &CLSID_NULL,          // Connects to filter
    NULL,                 // Connects to pin
    6,                    // Number of types
    filterInSubTypes          // Pin information
    },
    { L"Output",            // Pins string name
    FALSE,                // Is it rendered
    TRUE,                 // Is it an output
    FALSE,                // Are we allowed none
    FALSE,                // And allowed many
    &CLSID_NULL,          // Connects to filter
    NULL,                 // Connects to pin
    1,                    // Number of types
    &filterOutSubTypes          // Pin information
    }
};

static const AMOVIESETUP_FILTER filter = 
{
    &__uuidof(MP3DecoderFilter),  // filter clsid
    L"MP3 decoder",   // filter name
    MERIT_NORMAL,     // ie default for auto graph building
    2,                // count of registered pins
    filterPins        // list of pins to register
};

// --- COM factory table and registration code --------------

// DirectShow base class COM factory requires this table, 
// declaring all the COM objects in this DLL
CFactoryTemplate g_Templates[] = {
    // one entry for each CoCreate-able object
    {
        filter.strName,
        filter.clsID,
        MP3DecoderFilter::CreateInstance,
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

