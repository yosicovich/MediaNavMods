// stdafx.cpp : source file that includes just the standard includes
// player_helper.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <stdarg.h>

#ifdef LOGTOFILE
Utils::FileLogger g_fileLogger(L"\\MD\\player_helper.log");
void WINAPIV NKDbgPrintfW(LPCWSTR lpszFmt, ...)
{
    va_list args;
    va_start(args, lpszFmt);
    g_fileLogger.writeLog(lpszFmt, args);
    va_end(args);
}
#endif
