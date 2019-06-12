// player_helper.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "compat.h"

#include "utils.h"
#include <PETools.h>
#include "SimpleIni.h"
#include <vector>
#include <sstream>

#include "pkfuncs.h"
#include "dbgapi.h"
#include "oemioctl.h"
#include "db13xx.h"
#include <pwindbas.h>
#include <diskio.h>
#include <CmnDLL.h>

using namespace Utils;

static bool g_inited = false;
static CSimpleIni g_iniFile;
static std::string compatSDManufacturer;
static std::string compatSDSerial;
static std::vector<BYTE> compatDevUUID;
static SYSTEMID    g_sysID;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
        DisableThreadLibraryCalls((HMODULE)hModule);
        if(!g_inited)
        {
            envInit();
            g_inited = true;
        }
        if(!isCurrentProcessMatch())
            break;
        patchImports();
        break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}


static std::vector<std::wstring> g_iniFilesTable;
static std::vector<std::wstring> g_hookFilter;
static std::vector<std::wstring> g_createDirectoryFilter;

static void envInit()
{
    // Populate  ini files table in search order
#ifndef PUBLIC_RELEASE
    if(detectPath(TEXT("MD"), 3))
        g_iniFilesTable.push_back(TEXT("\\MD\\mods.ini"));
#endif
    g_iniFilesTable.push_back(TEXT("\\Storage Card2\\mods.ini"));
    g_iniFilesTable.push_back(TEXT("\\Storage Card\\System\\mods\\mods.ini"));

    for(size_t i = 0; i < g_iniFilesTable.size(); ++i)
    {
        if(g_iniFile.LoadFile(g_iniFilesTable[i].c_str()) == SI_OK)
            break;
    }


    g_hookFilter = Utils::splitWString(Utils::toUpper(std::wstring(g_iniFile.GetValue(TEXT("Compat"), TEXT("ApplyFilter"), TEXT("")))), L',');
    std::transform(g_hookFilter.begin(), g_hookFilter.end(), g_hookFilter.begin(), Utils::trim<std::wstring>);

    DWORD rSize;
    POTP  pOtp;

    if(KernelIoControl(IOCTL_OEM_GET_SYSTEMID, NULL, 0, &g_sysID, sizeof(g_sysID), &rSize) && rSize == sizeof(g_sysID))
    {
        pOtp = reinterpret_cast<POTP>(g_sysID.guid);

        compatDevUUID.resize(sizeof(OTP));
        memcpy(&compatDevUUID[0], g_sysID.guid, sizeof(OTP));

        char buf[8+1];
        buf[sizeof(buf) - 1] = '\0';
        _snprintf(buf, sizeof(buf), "%08X", pOtp->ChipIDLo);
        compatSDSerial = buf;
    }

    // Manufacturer
    {
        std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(TEXT("Compat"), TEXT("SDManufacturer"), TEXT("")));
        if(!sTmp.empty())
        {
            // STORAGE_IDENTIFICATION ATA max Manufacturer
            if(sTmp.size() > 20)
                sTmp.resize(20);
            compatSDManufacturer = sTmp;
        }
    }
    // Serial
    {
        std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(TEXT("Compat"), TEXT("SDSerial"), TEXT("")));
        if(!sTmp.empty())
        {
            // STORAGE_IDENTIFICATION ATA max Serial
            if(sTmp.size() > 8)
                sTmp.resize(8);
            compatSDSerial = sTmp;
        }
    }
    // UUID
    {
        std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(TEXT("Compat"), TEXT("DevUUID"), TEXT("")));
        if(!sTmp.empty())
        {
            if(sTmp.size() > sizeof(OTP))
                sTmp.resize(sizeof(OTP));
            compatDevUUID.resize(sTmp.size());
            memcpy(&compatDevUUID[0], sTmp.c_str(), sTmp.size());
        }
    }

    g_createDirectoryFilter = Utils::splitWString(Utils::toUpper(std::wstring(g_iniFile.GetValue(TEXT("Compat"), TEXT("CreateDirectoryFilter"), TEXT("")))), L',');
    std::transform(g_createDirectoryFilter.begin(), g_createDirectoryFilter.end(), g_createDirectoryFilter.begin(), Utils::trim<std::wstring>);

};

int getVersion()
{
    return 0;
}

bool isCurrentProcessMatch()
{
    std::wstring curExe = Utils::toUpper(Utils::getModuleName(NULL));
    for(size_t i = 0; i < g_hookFilter.size(); ++i)
    {
        if(curExe.find(g_hookFilter[i]) != std::wstring::npos)
            return true;
    }
    return false;
}

void patchImports()
{
    PETools::ImportsAccesor patcher;

    const void** pFunc = patcher.getFunctionPtr("COREDLL.DLL", 557);
    if(pFunc)
        *pFunc = hook_KernelIoControl;

    pFunc = patcher.getFunctionPtr("COREDLL.DLL", 179);
    if(pFunc)
        *pFunc = hook_DeviceIoControl;

    pFunc = patcher.getFunctionPtr("COREDLL.DLL", 530);
    if(pFunc)
        *pFunc = hook_GetProcAddressW;

    pFunc = patcher.getFunctionPtr("COREDLL.DLL", 160);
    if(pFunc)
        *pFunc = hook_CreateDirectoryW;
}

BOOL hook_KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned)
{
    debugPrintf(DBG, TEXT("hook_KernelIoControl(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X)\r\n"), dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned);
    if(dwIoControlCode ==  IOCTL_HAL_GET_UUID)
    {
        if(nOutBufSize < compatDevUUID.size())
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        memcpy(lpOutBuf, &compatDevUUID[0], compatDevUUID.size());
        *lpBytesReturned = compatDevUUID.size();
        debugDump(DBG, lpOutBuf, *lpBytesReturned);
        return TRUE;
    }
    BOOL bResult = KernelIoControl(dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned);
    if(bResult)
    {
        debugDump(DBG, lpOutBuf, *lpBytesReturned);
    }
    return bResult;
}

BOOL hook_DeviceIoControl (HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
    debugPrintf(DBG, TEXT("hook_DeviceIoControl(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X)\r\n"), hDevice, dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);
    if(dwIoControlCode == IOCTL_DISK_GET_STORAGEID)
    {
        if(compatSDManufacturer.empty() && compatSDSerial.empty())
            return FALSE;
        PSTORAGE_IDENTIFICATION pInfo = reinterpret_cast<PSTORAGE_IDENTIFICATION>(lpOutBuf);
        DWORD reqSize = sizeof(STORAGE_IDENTIFICATION) + (!compatSDManufacturer.empty() ? compatSDManufacturer.size() + 1 : 0) + (!compatSDSerial.empty() ? compatSDSerial.size() + 1 : 0);
        if(reqSize > nOutBufSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        if(!compatSDManufacturer.empty())
        {
            pInfo->dwManufactureIDOffset = sizeof(STORAGE_IDENTIFICATION);
            char* pManfacturer = reinterpret_cast<char*>(reinterpret_cast<DWORD>(pInfo) + pInfo->dwManufactureIDOffset);
            strcpy(pManfacturer, compatSDManufacturer.c_str());
        }else
        {
            pInfo->dwManufactureIDOffset = 0;
            pInfo->dwFlags |= MANUFACTUREID_INVALID;
        }

        if(!compatSDSerial.empty())
        {
            if(pInfo->dwManufactureIDOffset != 0)
                pInfo->dwSerialNumOffset = pInfo->dwManufactureIDOffset + compatSDManufacturer.size() + 1;
            else
                pInfo->dwSerialNumOffset = sizeof(STORAGE_IDENTIFICATION);
            char* pSerial = reinterpret_cast<char*>(reinterpret_cast<DWORD>(pInfo) + pInfo->dwSerialNumOffset);
            strcpy(pSerial, compatSDSerial.c_str());
        }else
        {
            pInfo->dwSerialNumOffset = 0;
            pInfo->dwFlags |= SERIALNUM_INVALID;
        }

        pInfo->dwSize = reqSize;
        *lpBytesReturned = reqSize;
        debugDump(DBG, lpOutBuf, *lpBytesReturned);
        return TRUE;
    
    }

    BOOL bResult = DeviceIoControl(hDevice, dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);
    if(bResult)
    {
        debugDump(DBG, lpOutBuf, *lpBytesReturned);
    }
    return bResult;
}

void* hook_GetProcAddressW(HMODULE hModule, const wchar_t* procName)
{
    if(procName)
    {
        if(reinterpret_cast<DWORD>(procName) >= 0x10000)
        {
            if(wcscmp(procName, TEXT("DeviceIoControl")) == 0)
            {
                return hook_DeviceIoControl;
            }
        }else
        {
            if(reinterpret_cast<DWORD>(procName) == 530)
            {
                return hook_DeviceIoControl;
            }
        }
    }
    return GetProcAddress(hModule, procName);
}

BOOL WINAPI hook_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    debugPrintf(DBG, TEXT("hook_CreateDirectoryW(%s)\r\n"), lpPathName);
    std::wstring createPath = Utils::toUpper(std::wstring(lpPathName));
    for(size_t i = 0; i < g_createDirectoryFilter.size(); ++i)
    {
        if(createPath.find(g_createDirectoryFilter[i]) != std::wstring::npos)
        {
            debugPrintf(DBG, TEXT("hook_CreateDirectoryW(%s) - denied\r\n"), lpPathName);
            return FALSE;
        }
    }
    return CreateDirectory(lpPathName, lpSecurityAttributes);
}
