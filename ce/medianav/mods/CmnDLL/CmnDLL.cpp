// CmnDLL.cpp : Defines the entry point for the DLL application.
//
#include "CmnDLL.h"

void DbgSetDebugLevel(int module, int level)
{

}

void DbgDebugPrint(int module, int level, const wchar_t *format, ...)
{

}

bool IpcPostMsg(int src, int dst, int cmd, int extraSize/* max 4*/, const void* pExtra)
{
    return false;
}
