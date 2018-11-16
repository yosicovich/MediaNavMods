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
#define FOURCC(p)   (DWORD(p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24)))

// byte re-ordering
inline long SwapLong(const BYTE* pByte)
{
    return (pByte[0] << 24) |
        (pByte[1] << 16) |
        (pByte[2] << 8)  |
        pByte[3];
}

inline LONGLONG SwapI64(const BYTE* pByte)
{
    return ((LONGLONG)SwapLong(pByte))<< 32 |
        (unsigned long)(SwapLong(pByte + 4));
}

#define GetTypedPtr(typ, ptr) reinterpret_cast<typ*>(ptr.get())

#define TESTMODE
#include "utils.h"

#ifdef TESTMODE
#define DBG 1
#else
#define DBG 0
#endif



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


