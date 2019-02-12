#pragma once
#include <windows.h>

struct IpcMsg
{
    IpcMsg()
        :src(0), cmd(0), extraSize(0), extra(0)
    {

    }

    bool isValid() const
    {
        return cmd != 0;
    }

    DWORD src;
    DWORD cmd;
    DWORD extraSize;
    DWORD extra;
};
typedef void* hShmMem;

#define DbgModules 42

void DbgPrintDLLVersion();
void DbgSetDebugLevel(int module, int level);
void DbgSetAllDebugOnOff(bool bOnOff);
void DbgSetAllDebugLevel(int level);
void DbgSetDebugOnOff(int module, bool bOnOff);
bool DbgGetDebugOnOff(int module);
int DbgGetDebugLevel(int module);
const wchar_t* DbgGetModuleName(int module);
void DbgDebugPrint(int module, int level, const wchar_t *format, ...);
bool IpcPostMsg(int src, int dst, int cmd, int extraSize/* max 4*/, const void* pExtra);
bool IpcSendMsg(int src, int dst, int cmd, int extraSize, const void* pExtra);
IpcMsg* IpcGetMsg(IpcMsg* ipcMsg, UINT message, WPARAM wParam, LPARAM lParam);

hShmMem MSHM_Dll_CreateShmClassObj();
void MSHM_Dll_DestroyShmClassObj(hShmMem hHandle);
bool MSHM_Dll_MakeMappingReadOnly(hShmMem hHandle, const wchar_t* sName, HANDLE hFile);
bool MSHM_Dll_MakeMappingReadWrite(hShmMem hHandle, const wchar_t* sName, DWORD areaSize, HANDLE hFile);
void MSHM_Dll_Read(hShmMem hHandle, void* pBuffer, DWORD readPos, DWORD size);
void MSHM_Dll_Write(hShmMem hHandle, const void* pBuffer, DWORD writePos, DWORD size);


