// stdafx.cpp : source file that includes just the standard includes
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#include <ObjBase.h>
#include <initguid.h>
#include <audshow.h>
#include "AACDecoderFilter.h"
#include <stdarg.h>

#ifdef LOGTOFILE
Utils::FileLogger g_fileLogger(L"\\MD\\aacdecfilter.log");
void WINAPIV NKDbgPrintfW(LPCWSTR lpszFmt, ...)
{
    va_list args;
    va_start(args, lpszFmt);
    g_fileLogger.writeLog(lpszFmt, args);
    va_end(args);
}
#endif
