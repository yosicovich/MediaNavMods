#include "stdafx.h"
#include "AC3DecoderFilter.h"

// --- COM factory table and registration code --------------

// DirectShow base class COM factory requires this table, 
// declaring all the COM objects in this DLL
CFactoryTemplate g_Templates[] = {
    // one entry for each CoCreate-able object
    {
        AC3DecoderFilter::m_sudFilter.strName,
        AC3DecoderFilter::m_sudFilter.clsID,
        AC3DecoderFilter::CreateInstance,
        NULL,
        &AC3DecoderFilter::m_sudFilter
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

