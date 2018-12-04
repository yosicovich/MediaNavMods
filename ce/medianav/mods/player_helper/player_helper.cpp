// player_helper.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "player_helper.h"

#include "utils.h"
#include "pkfuncs.h"
#include "dbgapi.h"
#include "oemioctl.h"
#include "db13xx.h"
#include <set>

using namespace Utils;

static bool g_inited = false;
static BOOL testBool = false;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
#ifdef TESTMODE
/*        DWORD oldProtect;
        testBool = VirtualProtect((LPVOID)0x10000, 4, PAGE_EXECUTE_READWRITE, &oldProtect);*/
#endif
        DisableThreadLibraryCalls((HMODULE)hModule);
        if(!g_inited)
        {
            fixCodecsPath();
            globalEnvInit();
            g_inited = true;
        }
        break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}


static std::set<std::wstring> g_mediaExts;

static void globalEnvInit()
{
    // Default
    g_mediaExts.insert(L".MP3");
    g_mediaExts.insert(L".WMA");

    // New
    g_mediaExts.insert(L".MP4");
    g_mediaExts.insert(L".FLAC");
    g_mediaExts.insert(L".MKV");
    g_mediaExts.insert(L".AVI");

    g_mediaExts.insert(L".M4A");
}



#ifdef TESTMODE
static const std::wstring cCodecsBase = L"\\MD\\vtest\\";
#else
static const std::wstring cCodecsBase = L"\\Storage Card\\system\\mods\\codecs\\";
#endif

struct RegistryEntry
{
    HKEY rootKey;
    std::wstring path;
    std::wstring name;
    std::wstring value;
    RegistryEntry(HKEY rootKey, const std::wstring& path, const std::wstring& name, const std::wstring& value)
        :rootKey(rootKey), path(path), name(name), value(value)
    {
    }
};

typedef std::vector<RegistryEntry> RegistryMap;

PLAYER_HELPER_API bool fixCodecsPath()
{
    std::wstring value;
    if(RegistryAccessor::getString(HKEY_CLASSES_ROOT, L"\\CLSID\\{0ba13ea1-70e5-11db-9690-00e08161165f}\\InprocServer32", L"", L"") != L"VideoRenderer.dll")
        return true;// Paths are already adjusted.
    
    RegistryMap registryPathMap;

    //registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{1F3F5741-A9EE-4bd9-B64E-99C5534B3817}\\InprocServer32", L"", cCodecsBase + L"ac3decfilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{313F1007-5458-4275-8143-E760A1D73D0F}\\InprocServer32", L"", cCodecsBase + L"aacdecfilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{3E4DCA25-347E-4678-B22A-6F4CC68FF2A8}\\InprocServer32", L"", cCodecsBase + L"audiocorefilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D24C840C-C469-4368-A363-0913B44AEF5C}\\InprocServer32", L"", cCodecsBase + L"avidmx.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{692100F0-01C4-4af0-BDC2-C8BA5C5DED01}\\InprocServer32", L"", cCodecsBase + L"DecodeFilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{B380606B-B500-4001-ABA9-635D24D95504}\\InprocServer32", L"", cCodecsBase + L"losslessdecfilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{9EBFDAAE-0963-4b5a-8B2E-EDB9B943820B}\\InprocServer32", L"", cCodecsBase + L"matroskafilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{025BE2E4-1787-4da4-A585-C5B2B9EEB57C}\\InprocServer32", L"", cCodecsBase + L"mp4demux.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D1E456E1-47E5-497a-ABA1-A0C57C3CE5C1}\\InprocServer32", L"", cCodecsBase + L"speexdecfilter.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{0ba13ea1-70e5-11db-9690-00e08161165f}\\InprocServer32", L"", cCodecsBase + L"VideoRenderer.dll"));
    registryPathMap.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D1E456E1-47E5-497a-ABA1-A0C57C3CE5C0}\\InprocServer32", L"", cCodecsBase + L"vorbisdecfilter.dll"));

    for(RegistryMap::const_iterator it = registryPathMap.begin(); it != registryPathMap.end(); ++it)
    {
        if(!RegistryAccessor::setString(it->rootKey, it->path, it->name, it->value))
        {
            return false;
        }
    }
     return true;
}

#ifdef TESTMODE
void mempool_test()
{
	DEVMGR_DEVICE_INFORMATION di;
	di.dwSize = sizeof(di);

	HANDLE findHandle = FindFirstDevice(DeviceSearchByLegacyName, L"MEM*", &di);
	DWORD err = GetLastError();
	FindClose(findHandle);

	BOOL bVal = DeactivateDevice(di.hDevice);

    /*RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\ITE", L"Base", 0x31800000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\ITE", L"Size", 0x01000000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\ITE", L"Index", 0x00000001);

    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\EXT", L"Base", 0x39800000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\EXT", L"Size", 0x06800000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\EXT", L"Index", 0x00000003);

    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\MAE", L"Base", 0x30800000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\MAE", L"Size", 0x01000000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\MAE", L"Index", 0x00000000);

    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\LCD", L"Base", 0x30000000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\LCD", L"Size", 0x00800000);
    RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\mempool\\Regions\\LCD", L"Index", 0x00000002);*/

	HANDLE devHandle = ActivateDeviceEx(di.szDeviceKey,NULL,0,NULL);	
	//HANDLE devHandle = ActivateDeviceEx(L"\\Drivers\\BuiltIn\\MSD",NULL,0,NULL);
}
#define GWL_HWNDPARENT (-8)

BOOL CALLBACK EnumWindowsProc(_In_ HWND   hwnd, _In_ LPARAM lParam)
{
    DWORD wndProc = GetWindowLong(hwnd, GWL_WNDPROC);
    DWORD owner = GetWindowLong(hwnd, GWL_HWNDPARENT);
    wchar_t className[256];
    className[256] = 0;
    int classNameSize = GetClassName(hwnd, (LPWSTR)&className, 255);
    return TRUE;
}

PLAYER_HELPER_API void test()
{
    EnumWindows(EnumWindowsProc, 0);


    SYSTEMID    SysID;
    ULONG        ResultSize;
    OTP            *pOtp;
    int            VideoCaps;

    /*
    * Use the kernel IOCTL to retrieve PRID/BRDID/SCRATCH/GUID
    */
    KernelIoControl(IOCTL_OEM_GET_SYSTEMID, NULL, 0, &SysID, sizeof(SysID), &ResultSize);


    pOtp = (OTP *)SysID.guid;

    return;

    /*    LPWSTR str = (LPWSTR)0x1304c;
    LPVOID testPtr = VirtualAlloc(test, 512, MEM_COMMIT, PAGE_READONLY);
    if(testPtr)
    {
        DWORD oldProtect;
        BOOL bVal = VirtualProtect(testPtr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
        VirtualFree(testPtr, 0, MEM_RELEASE);
    }*/
    wchar_t  name[256];
    GetModuleFileName(NULL, (LPWSTR)&name, 256);

    HANDLE hFile = CreateFile((LPWSTR)&name, GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_WRITECOPY , 0, 0x1000, NULL);

    void *testPtr = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0x200);
    DWORD testD = 0;
    BOOL bW = WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x11000, &testD, 4,NULL);
    DWORD xxx = GetLastError();

    MEMORY_BASIC_INFORMATION mem;
    VirtualQuery(testPtr, &mem, sizeof(mem));
    SetLastError(0);
    xxx = GetLastError();
    DWORD oldProtect;
    BOOL bVal = VirtualProtect(testPtr, 4, PAGE_WRITECOPY, NULL);
    //DWORD imageBase = Utils::getCurrentProcessCodeBase();
    xxx = GetLastError();

    MessageBox(NULL, L"TEST", L"Caption1", 0);
    //mempool_test();
}
#else
PLAYER_HELPER_API void test()
{

}
#endif


PLAYER_HELPER_API int extCheckMediaFilesExtList(const LPWSTR extValue)
{
    if(!extValue)
    {
        debugPrintf(1, L"extCheckMediaFilesExtList: extValue == NULL\r\n");
        return 1;
    }
    debugPrintf(1, L"extCheckMediaFilesExtList: extValue = %s\r\n", extValue);
    std::wstring str = extValue;
    Utils::toUpper(str);
    return g_mediaExts.find(str) != g_mediaExts.end() ? 0 : 1;
}

PLAYER_HELPER_API int extCheckMediaFileMatch(const LPWSTR fileName)
{
    if(!fileName)
    {
        debugPrintf(1, L"extCheckMediaFileMatch: fileName == NULL\r\n");
        return 1;
    }
    debugPrintf(1, L"extCheckMediaFileMatch('%s')\r\n", fileName);
    std::wstring str = fileName;
    size_t pos = str.find_last_of('.');
    if(pos == std::wstring::npos)
        return -1;
    str = str.substr(pos);
    Utils::toUpper(str);
    return g_mediaExts.find(str) != g_mediaExts.end() ? 0 : 1;
}

PLAYER_HELPER_API int extCheckMediaFileMatch2(const LPWSTR fileName)
{
    return extCheckMediaFileMatch(fileName) == 0;
}

/*ModsWindowsHook* ModsWindowsHook::self = NULL;

ModsWindowsHook::ModsWindowsHook()
    :Utils::SystemWideUniqueInstance(L"ModsWindowsHookMutex")
{
    if(!isUnique())
        return;
    self = this;
}

ModsWindowsHook::~ModsWindowsHook()
{

}

LRESULT CALLBACK ModsWindowsHook::WindowProcHookLink(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode == HC_ACTION && self != NULL)
    {
        PCWPSTRUCT params = reinterpret_cast<PCWPSTRUCT>(lParam);
        self->WindowProcHook(params->hwnd, params->message, params->wParam, params->lParam);
    }
    return CallNextHookEx(hHook_, nCode, wParam, lParam);

}

void ModsWindowsHook::WindowProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

}
*/