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

#define MP3_DEMUX_UUID "DC56E099-C9EA-49c8-9FA2-0A173D1522F1"
#ifdef _DEBUG
#define TESTMODE
#else
//#define TESTMODE
#endif

//#define LOGTOFILE
#include "utils.h"

#ifdef TESTMODE
#define DBG 1
#else
#define DBG 0
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


