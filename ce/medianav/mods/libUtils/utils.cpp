#include "utils.h"
#include <Tlhelp32.h>
#include <cctype>
#include <algorithm>
#include <cassert>
#include <exception>
#include <stdarg.h>


namespace Utils {

bool RegistryAccessor::setValue(HKEY hRootKey, const std::wstring& subKey, DWORD dwType, const std::wstring& valueName, const std::vector<BYTE>& data)
{
	HKEY hKey;
	if(RegCreateKeyEx(hRootKey, subKey.c_str(), 0, NULL, 0, NULL, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return false;

	if(RegSetValueEx(hKey, valueName.c_str(), 0, dwType, &data[0], data.size()) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

bool RegistryAccessor::setString(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value)
{
	std::vector<BYTE> data((value.size()+1) * 2);
    memcpy(&data[0], value.c_str(), data.size() + 2);
	return RegistryAccessor::setValue(hRootKey, subKey, REG_SZ, valueName, data);
}

bool RegistryAccessor::setInt(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, unsigned int value)
{
	std::vector<BYTE> data(sizeof(value));
    memcpy(&data[0], &value, sizeof(value));
	return RegistryAccessor::setValue(hRootKey, subKey, sizeof(value), valueName, data);
}

bool RegistryAccessor::setBool(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, bool value)
{
    return setInt(hRootKey, subKey, valueName, value ? 1 : 0);
}

bool RegistryAccessor::getValue(HKEY hRootKey, const std::wstring& subKey, DWORD dwType, const std::wstring& valueName, std::vector<BYTE>& data)
{
	HKEY hKey;
	if(RegCreateKeyEx(hRootKey, subKey.c_str(), 0, NULL, 0, NULL, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return false;

	DWORD sizeRequired = 0;
    DWORD dwTypeStored = 0;
    if(RegQueryValueEx(hKey, valueName.c_str(), 0, &dwTypeStored, NULL, &sizeRequired) != ERROR_SUCCESS)
	{
    	RegCloseKey(hKey);
        return false;
	}

    if(dwTypeStored != dwType)
    {
    	RegCloseKey(hKey);
        return false;
    }

    data.resize(sizeRequired);
    if(RegQueryValueEx(hKey, valueName.c_str(), 0, NULL, &data[0], &sizeRequired) != ERROR_SUCCESS)
	{
	    RegCloseKey(hKey);
		return false;
	}
	RegCloseKey(hKey);
	return true;
}

std::wstring RegistryAccessor::getString(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& defaultValue)
{
    std::vector<BYTE> pureData;
    if(!RegistryAccessor::getValue(hRootKey, subKey, REG_SZ, valueName, pureData))
        return defaultValue;

    std::wstring data;
    data.assign(reinterpret_cast<wchar_t *>(&pureData[0]), pureData.size());
    data.resize(wcslen(&data[0]));
    return data;
}

DWORD RegistryAccessor::getInt(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD defaultValue)
{
    std::vector<BYTE> pureData;
    if(!RegistryAccessor::getValue(hRootKey, subKey, REG_DWORD, valueName, pureData))
        return defaultValue;
    
    assert(pureData.size() == sizeof(DWORD));
    return *(reinterpret_cast<DWORD *>(&pureData[0]));
}

bool RegistryAccessor::getBool(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, bool defaultValue)
{
    DWORD value = defaultValue ? 1 : 0;
    return getInt(hRootKey, subKey, valueName, value) == 1 ? true : false;
}

static const int MAX_KEY_LENGTH = 255;
std::vector<std::wstring> RegistryAccessor::getSubKeys(HKEY hRootKey, const std::wstring& subKey)
{
    std::vector<std::wstring> resultData;
    HKEY hKey;
    if(RegCreateKeyEx(hRootKey, subKey.c_str(), 0, NULL, 0, NULL, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return resultData;

    DWORD    cSubKeys=0;               // number of subkeys 
    // Get the class name and the value count. 
    DWORD retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        NULL,                    // buffer for class name 
        NULL,                    // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        NULL,                    // longest subkey size 
        NULL,                    // longest class string 
        NULL,                    // number of values for this key 
        NULL,                    // longest value name 
        NULL,                    // longest value data 
        NULL,                    // security descriptor 
        NULL);                   // last write time 

    // Enumerate the subkeys, until RegEnumKeyEx fails.

    if (retCode == ERROR_SUCCESS && cSubKeys)
    {
        TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string 
        for (DWORD i=0; i<cSubKeys; ++i) 
        { 
            cbName = MAX_KEY_LENGTH;
            if(RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                continue;
            resultData.push_back(achKey);
        }
    } 
    RegCloseKey(hKey);
    return resultData;
}

    
DWORD getCurrentProcessImageBase()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
     
    DWORD curProcessID = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        
    DWORD imageBase = (DWORD)-1;

    if(snapshot != INVALID_HANDLE_VALUE && Process32First(snapshot, &entry) == TRUE)
    {
        do
        {
            if(entry.th32ProcessID == curProcessID)
            {
                imageBase = entry.th32MemoryBase;
                break;
            }
        }while (Process32Next(snapshot, &entry) == TRUE);
        CloseToolhelp32Snapshot(snapshot);
    }
    return imageBase;
}

DWORD getCurrentProcessCodeBase()
{
    SYSTEM_INFO sinf;
    GetSystemInfo(&sinf);
    void* baseAddr = sinf.lpMinimumApplicationAddress;
    MEMORY_BASIC_INFORMATION mem;
    while(VirtualQuery(baseAddr, &mem, sizeof(mem)) == sizeof(mem))
    {
        if(mem.Protect & (PAGE_READONLY | PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))
            return (DWORD)mem.BaseAddress;
        baseAddr = (void*)((DWORD)mem.BaseAddress + mem.RegionSize);
    }
    return -1;
}

void toUpper(std::wstring& str)
{
    std::for_each(str.begin(), str.end(),std::toupper);
}

SystemWideUniqueInstance::SystemWideUniqueInstance(const std::wstring& name)
:lock_(name.c_str())
,isUnique_(lock_.WaitLock(0))
{
}

bool SystemWideUniqueInstance::isUnique()
{
    return isUnique_;
}

void FileLogger::writeLog(const wchar_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    writeLog(fmt, args);
    va_end(args);
}

void FileLogger::writeLog(const wchar_t* fmt, va_list args)
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);

    CLockHolder<CLock> lock(m_lock);
    if(!m_file)
    {
        m_file = _wfopen(m_fileName.c_str(), L"ab");
        if(!m_file)
            return;
        fwprintf_s(m_file, L"\r\n\r\n--------------------- NEW LOG SESSION STARTED AT %04hu-%02hu-%02hu  %02hu:%02hu:%02hu.%03hu ---------------------\r\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
    }
    
    fwprintf_s(m_file, L"%04hu-%02hu-%02hu at %02hu:%02hu:%02hu.%03hu --- ", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);

    vfwprintf_s(m_file, fmt, args);
    fflush(m_file);
}

int OleInitializer::m_useCount = 0;
CLock OleInitializer::m_accessLock;

OleInitializer::OleInitializer()
{
    CLockHolder<CLock>  lock(m_accessLock);
    if(!m_useCount++)
    {
        if(CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
        {
            --m_useCount;
            exit(-1); // it is fatal if OLE is required.
            return;
        }
    }
}

OleInitializer::~OleInitializer()
{
    CLockHolder<CLock>  lock(m_accessLock);
    if(!--m_useCount)
    {
        CoUninitialize();
    }
}

void dumpBinary(const void* buf, size_t size)
{
    for(size_t i = 0; i < size; ++i)
    {
        if(i > 0 && (i % 16) == 0)
            NKDbgPrintfW(L"\r\n");
        NKDbgPrintfW(L" %02X", ((const char*)buf)[i] & 0xFF);
    }
    NKDbgPrintfW(L"\r\n");
}

void dumpGUID(const GUID* guid)
{
    NKDbgPrintfW(L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\r\n", guid->Data1, (unsigned int)guid->Data2, (unsigned int)guid->Data3
        ,(unsigned int)guid->Data4[0]
        ,(unsigned int)guid->Data4[1]
        ,(unsigned int)guid->Data4[2]
        ,(unsigned int)guid->Data4[3]
        ,(unsigned int)guid->Data4[4]
        ,(unsigned int)guid->Data4[5]
        ,(unsigned int)guid->Data4[6]
        ,(unsigned int)guid->Data4[7]
    );
}

std::wstring convertToWString(const std::string& str)
{
    if(!str.size()) 
        return std::wstring();

    DWORD cwch;

    std::vector<wchar_t> resultWStr;

    if(cwch = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0))//get buffer required in characters
    {
        resultWStr.resize(cwch);
        if(!::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), reinterpret_cast<wchar_t *>(&resultWStr[0]), resultWStr.size()))
        {
            if(ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                return std::wstring();
        }
    };

    return std::wstring(reinterpret_cast<wchar_t *>(&resultWStr[0]), resultWStr.size());
}

bool checkRectCompleteCovered(HWND hWnd, RECT rect, const std::set<HWND>& skipWindows/* = std::set<HWND>()*/)
{
    HRGN selfRgn = CreateRectRgnIndirect(&rect);
    HWND currentWnd = GetWindow(NULL, GW_HWNDFIRST);
    int rgnType = SIMPLEREGION;
    while (currentWnd && currentWnd !=hWnd && rgnType != NULLREGION)
    {
        if(IsWindowVisible(currentWnd))
        {
            if(GetWindowRect(currentWnd, &rect) == FALSE)
                break;
            HRGN tempRgn = CreateRectRgnIndirect(&rect);// currently examined window region
            rgnType = CombineRgn(selfRgn, selfRgn, tempRgn, RGN_DIFF); // diff intersect
            DeleteObject( tempRgn );
        }
        if (rgnType != NULLREGION) // there's a remaining portion
            currentWnd = GetWindow(currentWnd, GW_HWNDNEXT);

        while(currentWnd && skipWindows.find(currentWnd) != skipWindows.end())
            currentWnd = GetWindow(currentWnd, GW_HWNDNEXT);
    }
    DeleteObject(selfRgn);
    return rgnType == NULLREGION;
}

// CSharedLock
CSharedLock::CSharedLock(const wchar_t* name)
:m_hMutex(NULL)
{
    m_hMutex = CreateMutex(NULL, FALSE, name);

    if(m_hMutex == NULL)
    {
        throw std::exception("Unable to CREATE mutex");
    }
}

CSharedLock::~CSharedLock()
{
    Unlock();
    CloseHandle(m_hMutex);
}

void CSharedLock::Lock()
{
    WaitLock(INFINITE);
}

bool CSharedLock::WaitLock(DWORD timeoutMs)
{
    return WaitForSingleObject(m_hMutex, timeoutMs) == WAIT_OBJECT_0;
}

void CSharedLock::Unlock()
{
    ReleaseMutex(m_hMutex);
}

}; //namespace Utils

