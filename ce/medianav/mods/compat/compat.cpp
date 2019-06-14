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
#include <WindowsX.h>
#include <CmnDLL.h>

using namespace Utils;

static CSimpleIni g_iniFile;
static CompatRecPtr g_pCompatRec;

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
        debugPrintf(DBG_TRACE, TEXT("Compat: do envInit() GetCurrentProcessId() = 0x%08X\r\n"), GetCurrentProcessId());
        envInit();
        if(g_pCompatRec != NULL)
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

static void envInit()
{
    SYSTEMID    sysID;
    std::vector<BYTE> realDevUUID;
    std::string realSDSerial;
    TWStringVector iniFilesTable;

    // Read real device ID
    {
        DWORD rSize;
        POTP  pOtp;

        if(KernelIoControl(IOCTL_OEM_GET_SYSTEMID, NULL, 0, &sysID, sizeof(sysID), &rSize) && rSize == sizeof(sysID))
        {
            pOtp = reinterpret_cast<POTP>(sysID.guid);

            realDevUUID.resize(sizeof(OTP));
            memcpy(&realDevUUID[0], sysID.guid, sizeof(OTP));

            char buf[8+1];
            buf[sizeof(buf) - 1] = '\0';
            _snprintf(buf, sizeof(buf), "%08X", pOtp->ChipIDLo);
            realSDSerial = buf;
        }
    }


    // Populate  ini files table in search order
#ifndef PUBLIC_RELEASE
    if(detectPath(TEXT("MD"), 3))
        iniFilesTable.push_back(TEXT("\\MD\\mods.ini"));
#endif
    iniFilesTable.push_back(TEXT("\\Storage Card2\\mods.ini"));
    iniFilesTable.push_back(TEXT("\\Storage Card\\System\\mods\\mods.ini"));

    for(size_t i = 0; i < iniFilesTable.size(); ++i)
    {
        if(g_iniFile.LoadFile(iniFilesTable[i].c_str()) == SI_OK)
            break;
    }

    int idx = 0;
    do 
    {
        std::wostringstream strStream;
        strStream << TEXT("Compat_") << idx++;
        std::wstring ctlSection = strStream.str();
        if(g_iniFile.GetSectionSize(ctlSection.c_str()) == -1)
            break;
        const wchar_t* pSection = ctlSection.c_str();
        TWStringVector& hookFilter = Utils::splitWString(Utils::toUpper(std::wstring(g_iniFile.GetValue(pSection, TEXT("ExeMatchFilter"), TEXT("")))), L',');
        std::transform(hookFilter.begin(), hookFilter.end(), hookFilter.begin(), Utils::trim<std::wstring>);
        
        if(hookFilter.empty())
            continue;
        if(!isCurrentProcessMatch(hookFilter))
            continue;

        g_pCompatRec = new CompatRec(hookFilter, realDevUUID, realSDSerial);

        g_pCompatRec->m_createDirectoryFilter = Utils::splitWString(Utils::toUpper(std::wstring(g_iniFile.GetValue(pSection, TEXT("CreateDirectoryFilter"), TEXT("")))), L',');
        std::transform(g_pCompatRec->m_createDirectoryFilter.begin(), g_pCompatRec->m_createDirectoryFilter.end(), g_pCompatRec->m_createDirectoryFilter.begin(), Utils::trim<std::wstring>);

        g_pCompatRec->m_fixDeviceID = g_iniFile.GetBoolValue(pSection, TEXT("FixDeviceID"), false);
        if(g_pCompatRec->m_fixDeviceID)
        {
            // Manufacturer
            {
                std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(pSection, TEXT("SDManufacturer"), TEXT("")));
                if(!sTmp.empty())
                {
                    // STORAGE_IDENTIFICATION ATA max Manufacturer
                    if(sTmp.size() > 20)
                        sTmp.resize(20);
                    g_pCompatRec->m_sdManufacturer = sTmp;
                }
            }
            
            // Serial
            {
                std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(pSection, TEXT("SDSerial"), TEXT("")));
                if(!sTmp.empty())
                {
                    // STORAGE_IDENTIFICATION ATA max Serial
                    if(sTmp.size() > 8)
                        sTmp.resize(8);
                    g_pCompatRec->m_sdSerial = sTmp;
                }
            }
            // UUID
            {
                std::string sTmp = Utils::convertToAString(g_iniFile.GetValue(pSection, TEXT("DevUUID"), TEXT("")));
                if(!sTmp.empty())
                {
                    if(sTmp.size() > sizeof(OTP))
                        sTmp.resize(sizeof(OTP));
                    g_pCompatRec->m_devUUID.resize(sTmp.size());
                    memcpy(&g_pCompatRec->m_devUUID[0], sTmp.c_str(), sTmp.size());
                }
            }
        }

        TWStringVector& createFilePairs = Utils::splitWString(Utils::toUpper(std::wstring(g_iniFile.GetValue(pSection, TEXT("CreateFileMapping"), TEXT("")))), L',');
        for(size_t i = 0; i < createFilePairs.size(); ++i)
        {
            TWStringVector& createFilePair = Utils::splitWString(createFilePairs[i], L'=');
            if(createFilePair.size() != 2)
                continue;
            g_pCompatRec->m_createFileMapping.insert(std::make_pair(Utils::trim(createFilePair[0]), Utils::trim(createFilePair[1])));
        }

        g_pCompatRec->m_navitel = g_iniFile.GetBoolValue(pSection, TEXT("Navitel"), false);
        break;
    } while (true);

#ifdef PUBLIC_RELEASE
    if(!g_pCompatRec)
    {
        TWStringVector hookFilter;
        hookFilter.push_back(TEXT("NNGNAVI.EXE")); // Must be upper-case.
        if(isCurrentProcessMatch(hookFilter))
        {
            g_pCompatRec = new CompatRec(hookFilter, realDevUUID, realSDSerial);
            g_pCompatRec->m_createFileMapping.insert(std::make_pair(TEXT("COM4:"), TEXT("GPS1:")));
        }
    }
#endif

    if(g_pCompatRec != NULL)
        debugPrintf(DBG, TEXT("compat.dll: envInit() ExeMatchFilter has matched with %s\r\n"), Utils::getModuleName(NULL).c_str());
};

int getVersion()
{
    return 0;
}

bool isCurrentProcessMatch(const TWStringVector& hookFilter)
{
    std::wstring curExe = Utils::toUpper(Utils::getModuleName(NULL));
    for(size_t i = 0; i < hookFilter.size(); ++i)
    {
        if(curExe.find(hookFilter[i]) != std::wstring::npos)
            return true;
    }
    return false;
}

void patchImports()
{
    PETools::ImportsAccesor patcher;
    const void** pFunc;

    pFunc = patcher.getFunctionPtr("COREDLL.DLL", 530);
    if(pFunc)
        *pFunc = hook_GetProcAddressW;

    if(g_pCompatRec->m_fixDeviceID)
    {
        pFunc = patcher.getFunctionPtr("COREDLL.DLL", 557);
        if(pFunc)
            *pFunc = hook_KernelIoControl;

        pFunc = patcher.getFunctionPtr("COREDLL.DLL", 179);
        if(pFunc)
            *pFunc = hook_DeviceIoControl;
    }

    if(!g_pCompatRec->m_createDirectoryFilter.empty())
    {
        pFunc = patcher.getFunctionPtr("COREDLL.DLL", 160);
        if(pFunc)
            *pFunc = hook_CreateDirectoryW;
    }

    if(!g_pCompatRec->m_createFileMapping.empty())
    {
        pFunc = patcher.getFunctionPtr("COREDLL.DLL", 168);
        if(pFunc)
            *pFunc = hook_CreateFileW;
    }

    if(g_pCompatRec->m_navitel)
    {
        pFunc = patcher.getFunctionPtr("COREDLL.DLL", 95);
        if(pFunc)
            *pFunc = hook_RegisterClassW;
    }
}

BOOL hook_KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned)
{
    debugPrintf(DBG_TRACE, TEXT("hook_KernelIoControl(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X)\r\n"), dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned);
    if(dwIoControlCode ==  IOCTL_HAL_GET_UUID)
    {
        if(nOutBufSize < g_pCompatRec->m_devUUID.size())
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        memcpy(lpOutBuf, &g_pCompatRec->m_devUUID[0], g_pCompatRec->m_devUUID.size());
        *lpBytesReturned = g_pCompatRec->m_devUUID.size();
        debugDump(DBG_TRACE, lpOutBuf, *lpBytesReturned);
        return TRUE;
    }
    BOOL bResult = KernelIoControl(dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned);
    if(bResult)
    {
        if(lpOutBuf)
            debugDump(DBG_TRACE, lpOutBuf, *lpBytesReturned);
    }
    return bResult;
}

BOOL hook_DeviceIoControl (HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
    debugPrintf(DBG_TRACE, TEXT("hook_DeviceIoControl(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X)\r\n"), hDevice, dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);
    if(dwIoControlCode == IOCTL_DISK_GET_STORAGEID)
    {
        if(g_pCompatRec->m_sdManufacturer.empty() && g_pCompatRec->m_sdSerial.empty())
            return FALSE;
        PSTORAGE_IDENTIFICATION pInfo = reinterpret_cast<PSTORAGE_IDENTIFICATION>(lpOutBuf);
        DWORD reqSize = sizeof(STORAGE_IDENTIFICATION) + (!g_pCompatRec->m_sdManufacturer.empty() ? g_pCompatRec->m_sdManufacturer.size() + 1 : 0) + (!g_pCompatRec->m_sdSerial.empty() ? g_pCompatRec->m_sdSerial.size() + 1 : 0);
        if(reqSize > nOutBufSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        if(!g_pCompatRec->m_sdManufacturer.empty())
        {
            pInfo->dwManufactureIDOffset = sizeof(STORAGE_IDENTIFICATION);
            char* pManfacturer = reinterpret_cast<char*>(reinterpret_cast<DWORD>(pInfo) + pInfo->dwManufactureIDOffset);
            strcpy(pManfacturer, g_pCompatRec->m_sdManufacturer.c_str());
        }else
        {
            pInfo->dwManufactureIDOffset = 0;
            pInfo->dwFlags |= MANUFACTUREID_INVALID;
        }

        if(!g_pCompatRec->m_sdSerial.empty())
        {
            if(pInfo->dwManufactureIDOffset != 0)
                pInfo->dwSerialNumOffset = pInfo->dwManufactureIDOffset + g_pCompatRec->m_sdSerial.size() + 1;
            else
                pInfo->dwSerialNumOffset = sizeof(STORAGE_IDENTIFICATION);
            char* pSerial = reinterpret_cast<char*>(reinterpret_cast<DWORD>(pInfo) + pInfo->dwSerialNumOffset);
            strcpy(pSerial, g_pCompatRec->m_sdSerial.c_str());
        }else
        {
            pInfo->dwSerialNumOffset = 0;
            pInfo->dwFlags |= SERIALNUM_INVALID;
        }

        pInfo->dwSize = reqSize;
        *lpBytesReturned = reqSize;
        debugDump(DBG_TRACE, lpOutBuf, *lpBytesReturned);
        return TRUE;   
    }

    BOOL bResult = DeviceIoControl(hDevice, dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);
    if(bResult)
    {
        if(lpOutBuf)
            debugDump(DBG_TRACE, lpOutBuf, *lpBytesReturned);
    }
    return bResult;
}

void* hook_GetProcAddressW(HMODULE hModule, const wchar_t* procName)
{
    if(procName)
    {
        if(reinterpret_cast<DWORD>(procName) >= 0x10000)
        {
            if(g_pCompatRec->m_fixDeviceID && wcscmp(procName, TEXT("DeviceIoControl")) == 0)
            {
                return hook_DeviceIoControl;
            }

            if(!g_pCompatRec->m_createFileMapping.empty() && wcscmp(procName, TEXT("CreateFileW")) == 0)
            {
                return hook_CreateFileW;
            }

            if(g_pCompatRec->m_navitel && wcscmp(procName, TEXT("RegisterClassW")) == 0)
            {
                return hook_RegisterClassW;
            }
        }else
        {
            if(g_pCompatRec->m_fixDeviceID && reinterpret_cast<DWORD>(procName) == 530)
            {
                return hook_DeviceIoControl;
            }

            if(!g_pCompatRec->m_createFileMapping.empty() && reinterpret_cast<DWORD>(procName) == 168)
            {
                return hook_CreateFileW;
            }

            if(g_pCompatRec->m_navitel && reinterpret_cast<DWORD>(procName) == 95)
            {
                return hook_RegisterClassW;
            }
        }
    }
    return GetProcAddress(hModule, procName);
}

BOOL WINAPI hook_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    debugPrintf(DBG_TRACE, TEXT("hook_CreateDirectoryW(%s)\r\n"), lpPathName);
    std::wstring createPath = Utils::toUpper(std::wstring(lpPathName));
    for(size_t i = 0; i < g_pCompatRec->m_createDirectoryFilter.size(); ++i)
    {
        if(createPath.find(g_pCompatRec->m_createDirectoryFilter[i]) != std::wstring::npos)
        {
            debugPrintf(DBG_TRACE, TEXT("hook_CreateDirectoryW(%s) - denied\r\n"), lpPathName);
            return FALSE;
        }
    }
    return CreateDirectory(lpPathName, lpSecurityAttributes);
}

HANDLE WINAPI hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    debugPrintf(DBG_TRACE, TEXT("hook_CreateFileW(%s)\r\n"), lpFileName);
    std::wstring fileName = Utils::toUpper(std::wstring(lpFileName));
    TWStringWStringMap::const_iterator it = g_pCompatRec->m_createFileMapping.find(fileName);
    if(it != g_pCompatRec->m_createFileMapping.end())
    {
        lpFileName = it->second.c_str();
    }

    return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

ATOM hook_RegisterClassW(const WNDCLASSW *lpWndClass)
{
    debugPrintf(DBG_TRACE, TEXT("hook_RegisterClassW() lpszClassName=%s, lpfnWndProc=0x%08X\r\n"), lpWndClass->lpszClassName, lpWndClass->lpfnWndProc);
    if(g_pCompatRec->m_navitel && wcscmp(lpWndClass->lpszClassName, TEXT("navitel")) == 0)
    {
        WNDCLASSW wndClass = *lpWndClass;
        g_pCompatRec->m_navitelRealWindowProc = lpWndClass->lpfnWndProc;
        wndClass.lpfnWndProc = navitelWindowProc;
        return RegisterClassW(&wndClass);
    }
    return RegisterClassW(lpWndClass);
}

LRESULT CALLBACK navitelWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool timerActive = false;
    static bool longTapDetected = false;
    static UINT cLongTapDetectTimerID = 1412;
    static RECT cHideButtonRect = {0, 419, 134,480};
    static LPARAM clickPoint = 0;
    POINT pt;

    switch(uMsg)
    {
    case WM_DESTROY:
        if(timerActive)
        {
            KillTimer(hwnd, cLongTapDetectTimerID);
            timerActive = false;
        }
        break;
    case WM_LBUTTONDOWN:
        {
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if(PtInRect(&cHideButtonRect, pt))
            {
                clickPoint = lParam;
                longTapDetected = false;
                SetTimer(hwnd, cLongTapDetectTimerID, 500, NULL);
                timerActive = true;
                return TRUE;
            }else
            {
                if(timerActive)
                {
                    KillTimer(hwnd, cLongTapDetectTimerID);
                    timerActive = false;
                }
                longTapDetected = true;
            }
            break;
        }
    case WM_LBUTTONUP:
        {
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if(timerActive)
            {
                KillTimer(hwnd, cLongTapDetectTimerID);
                timerActive = false;
            }

            if(!PtInRect(&cHideButtonRect, pt) || longTapDetected)
                break;

            ShowWindow(hwnd, SW_HIDE);
            return TRUE;
            break;
        }
    case WM_TIMER:
        if(wParam == cLongTapDetectTimerID)
        {
            longTapDetected = true;
            KillTimer(hwnd, cLongTapDetectTimerID);
            timerActive = false;
            lParam = clickPoint;
            uMsg = WM_LBUTTONDOWN;
        }
        break;
    };

    if(g_pCompatRec->m_navitelRealWindowProc)
        return g_pCompatRec->m_navitelRealWindowProc(hwnd, uMsg, wParam, lParam);
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
