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

bool IpcSendMsg(int src, int dst, int cmd, int extraSize, const void* pExtra)
{
    return false;
}

IpcMsg* IpcGetMsg(IpcMsg* ipcMsg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return ipcMsg;
}

hShmMem MSHM_Dll_CreateShmClassObj()
{
    return NULL;
}

void MSHM_Dll_DestroyShmClassObj(hShmMem hHandle)
{

}

void MSHM_Dll_Write(hShmMem hHandle, const void* pBuffer, DWORD writePos, DWORD size)
{

}

void MSHM_Dll_Read(hShmMem hHandle, void* pBuffer, DWORD readPos, DWORD size)
{

}

bool MSHM_Dll_MakeMappingReadWrite(hShmMem hHandle, const wchar_t* sName, DWORD areaSize, HANDLE hFile)
{
    return false;
}

bool MSHM_Dll_MakeMappingReadOnly(hShmMem hHandle, const wchar_t* sName, HANDLE hFile)
{
    return false;
}
