#pragma once
#include "stdafx.h"
#include "utils.h"
#include <vector>
#include <map>
#include <smartptr.h>

typedef std::vector<std::wstring> TWStringVector;
typedef std::map<std::wstring, std::wstring> TWStringWStringMap;
struct CompatRec
{
    CompatRec(const TWStringVector& hookFilter, const std::vector<BYTE>& devUUID, const std::string& sdSerial)
        :m_hookFilter(hookFilter)
        ,m_devUUID(devUUID)
        ,m_sdSerial(sdSerial)
        ,m_fixDeviceID(false)
    {
    }
    TWStringVector m_hookFilter;
    TWStringVector m_createDirectoryFilter;
    TWStringWStringMap m_createFileMapping;
    bool m_fixDeviceID;
    std::vector<BYTE> m_devUUID;
    std::string m_sdManufacturer;
    std::string m_sdSerial;
};
typedef smart_ptr<CompatRec> CompatRecPtr;

typedef std::vector<CompatRecPtr> TCompatRecs;
typedef std::map<DWORD, CompatRecPtr> THookMap;

static void envInit();
int getVersion();
bool isCurrentProcessMatch(const TWStringVector& hookFilter);
static void patchImports();

BOOL hook_KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned);
BOOL hook_DeviceIoControl (HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
void* hook_GetProcAddressW(HMODULE hModule, const wchar_t* procName);
BOOL WINAPI hook_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
HANDLE WINAPI hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
