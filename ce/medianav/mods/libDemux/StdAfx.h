// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// ignore deprecated warnings for standard headers
#pragma warning(push)
#pragma warning(disable:4995)

#include <streams.h>

#include <comdef.h>
_COM_SMARTPTR_TYPEDEF(IMediaSample, IID_IMediaSample);
_COM_SMARTPTR_TYPEDEF(IAsyncReader, IID_IAsyncReader);
_COM_SMARTPTR_TYPEDEF(IPin, IID_IPin);

#include "smartptr.h"
#include <list>
#include <vector>

#include <string>
using namespace std;

#pragma warning(pop)

#ifdef _DEBUG
#define TESTMODE
#else
//#define TESTMODE
#endif

#ifdef TESTMODE
#define DEMUX_DBG 1
#define DEMUX_TRACE 0
#define ADECODE_DBG 1
#define ADECODE_TRACE 0
#ifndef _DEBUG
#define ADECODE_PERF 1
#else
#define ADECODE_PERF 0
#endif
#else
#define DEMUX_DBG 0
#define DEMUX_TRACE 0
#define ADECODE_DBG 0
#define ADECODE_TRACE 0
#define ADECODE_PERF 0
#endif

#define filterDebugPrintf(lev, format, ...) debugPrintf(lev, L"INSTANCE(0x%08X) --- " format, this, __VA_ARGS__)

__forceinline bool checkCopyrightProtection()
{
#ifdef PUBLIC_RELEASE
    return *reinterpret_cast<ULONGLONG *>(0x0002F48C) == 0x005400720067004DL; // 'MgrT'
#else
    return true;
#endif
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


