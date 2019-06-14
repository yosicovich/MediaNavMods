#include "stdafx.h"
#include "player_helper.h"

#include "utils.h"
#include "db13xx.h"
#include <set>
#include "SimpleIni.h"
#include <pwindbas.h>
#include <Devload.h>
#include <CmnDLL.h>

using namespace Utils;

static bool g_inited = false;
static BOOL testBool = false;
static CSimpleIni g_iniFile;
static Utils::SystemWideUniqueInstance g_onceChecker(L"PlayerHelperOnce");

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
            globalEnvInit();
            fixCodecsPath();
            startOnce();
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
static std::vector<std::wstring> g_iniFilesTable;

static void globalEnvInit()
{
    // Populate  ini files table in search order
#ifndef PUBLIC_RELEASE
    if(detectPath(TEXT("MD"), 3))
        g_iniFilesTable.push_back(TEXT("\\MD\\mods.ini"));
#endif
    g_iniFilesTable.push_back(TEXT("\\Storage Card2\\mods.ini"));
    g_iniFilesTable.push_back(MODS_ROOT_PATH TEXT("mods.ini"));

    for(size_t i = 0; i < g_iniFilesTable.size(); ++i)
    {
        if(g_iniFile.LoadFile(g_iniFilesTable[i].c_str()) == SI_OK)
            break;
    }

    // In order of popularity.
    // Default
    g_mediaExts.insert(L".MP3");
    g_mediaExts.insert(L".WMA");

    // New
    //Video
    g_mediaExts.insert(L".MP4");
    g_mediaExts.insert(L".AVI");

    // Audio
    g_mediaExts.insert(L".M4A");
    g_mediaExts.insert(L".FLAC");
    g_mediaExts.insert(L".AC3");
    g_mediaExts.insert(L".AAC");
    g_mediaExts.insert(L".OGG");
    g_mediaExts.insert(L".WAV");
    //g_mediaExts.insert(L".APE"); // There is no demux filter but losslessdecfilter.dll seems to support decoding!

    // To be implemented
    //g_mediaExts.insert(L".JPG");
    //g_mediaExts.insert(L".GIF");

#ifndef STABLE_ONLY
    g_mediaExts.insert(L".MKV");
#endif
}

// Start once
struct StartProcessEntry
{
    std::wstring filePath;
    std::wstring arguments;
    StartProcessEntry(const std::wstring& filePath, const std::wstring& arguments)
        :filePath(filePath)
        ,arguments(arguments)
    {

    }
};
typedef std::vector<StartProcessEntry> StartProcesses;

static void startOnce()
{
    // Run the code below only once
    if(!g_onceChecker.isUnique())
        return;
    
    std::wstring uiPath = Utils::makeFolderPath(std::wstring() + g_iniFile.GetValue(TEXT("Player"), TEXT("UIPath"), MODS_ROOT_PATH TEXT("UI\\")));
    RegistryAccessor::setString(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX\\DirectShow\\Video Renderer"), TEXT("UIPath"), uiPath);

    const CSimpleIniW::TKeyVal* cmnDebug = g_iniFile.GetSection(TEXT("CmnDebug"));
    if(cmnDebug)
    {
        if(g_iniFile.GetBoolValue(TEXT("CmnDebug"), TEXT("All"), false))
            DbgSetAllDebugOnOff(true);

        int allLevel = g_iniFile.GetLongValue(TEXT("CmnDebug"), TEXT("AllLevel"), -1);
        if(allLevel != -1)
            DbgSetAllDebugLevel(allLevel);
        static const wchar_t* cSetLevelMark = L"SetLevel_";
        for(CSimpleIniW::TKeyVal::const_iterator it = cmnDebug->begin(); it != cmnDebug->end(); ++it)
        {
            std::wstring sName = it->first.pItem;
            if(sName.find(cSetLevelMark) != 0)
                continue;
            sName = sName.substr(wcslen(cSetLevelMark));
            if(sName.empty())
                continue;

            for(int i = 0; i < DbgModules; ++i)
            {
                if(sName != DbgGetModuleName(i))
                    continue;
                DbgSetDebugOnOff(i, true);
                DbgSetDebugLevel(i, g_iniFile.GetLongValue(TEXT("CmnDebug"), it->first.pItem, 0));
            }
        }
    }

    StartProcesses startPorcesses;
    
    long waitForMD = g_iniFile.GetLongValue(TEXT("Debug"), TEXT("WaitForMD"), 0);
    if(waitForMD)
    {
        debugPrintf(DBG, TEXT("startOnce(): Detect MD for %d seconds\r\n"), waitForMD);
        if(detectPath(TEXT("MD"), waitForMD))
            debugPrintf(DBG, TEXT("startOnce(): MD detected\r\n"));
    }
    // Populate

    // 1. Default once
    if(!g_iniFile.GetBoolValue(TEXT("Debug"), TEXT("NoDefaultStarts"), false) && (!g_iniFile.GetBoolValue(TEXT("Debug"), TEXT("NoDefaultStartsWithMD"), false) || !isPathPresent(TEXT("MD"))))
    {
        startPorcesses.push_back(StartProcessEntry(MODS_ROOT_PATH TEXT("USBTags.exe"), TEXT("")));
    }

    // 2. Load from ini
    const CSimpleIniW::TKeyVal* startIni = g_iniFile.GetSection(TEXT("RunOnStart"));
    if(startIni)
    {
        for(CSimpleIniW::TKeyVal::const_iterator it = startIni->begin(); it != startIni->end(); ++it)
        {
            startPorcesses.push_back(StartProcessEntry(it->first.pItem, it->second));
            debugPrintf(DBG, TEXT("startOnce(): Add from ini : process %s, with command line - %s\r\n"), it->first.pItem, it->second);
        }
    }

    // Activate GPS Driver
    if(!g_iniFile.GetBoolValue(TEXT("Debug"), TEXT("NoGPSDriver"), false))
    {
        Utils::RegistryAccessor::setString(HKEY_LOCAL_MACHINE, DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), DEVLOAD_DLLNAME_VALNAME, MODS_ROOT_PATH TEXT("GPS_driver.dll"));
        Utils::RegistryAccessor::setString(HKEY_LOCAL_MACHINE, DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), DEVLOAD_PREFIX_VALNAME, TEXT("GPS"));
        Utils::RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), DEVLOAD_LOADORDER_VALNAME, 1);
        Utils::RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), DEVLOAD_INDEX_VALNAME, 1);

        Utils::RegistryAccessor::setString(HKEY_LOCAL_MACHINE, DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), TEXT("DevicePathName"), TEXT("COM4:"));

        HANDLE devHandle = ActivateDeviceEx(DEVLOAD_BUILT_IN_KEY TEXT("\\GPS"), NULL, 0, NULL);
        if(devHandle == INVALID_HANDLE_VALUE || devHandle == NULL)
            debugPrintf(DBG, TEXT("startOnce(): Activate GPS Driver has failed!\r\n"));
    }

    // Inject Dll
    if(!g_iniFile.GetBoolValue(TEXT("Debug"), TEXT("NoInjectDll"), false))
    {
        std::vector<std::wstring> dlls = Utils::RegistryAccessor::getStrings(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\KERNEL"), TEXT("InjectDLL"), std::vector<std::wstring>());
        dlls.push_back(MODS_ROOT_PATH TEXT("compat.dll"));
        if(!Utils::RegistryAccessor::setStrings(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\KERNEL"), TEXT("InjectDLL"), dlls))
            debugPrintf(DBG, TEXT("startOnce(): Inject Dll has failed!\r\n"));
    }

    // Run
    for(StartProcesses::const_iterator it = startPorcesses.begin(); it != startPorcesses.end(); ++it)
    {
        if(CreateProcess(it->filePath.c_str(), it->arguments.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL) == FALSE)
            debugPrintf(DBG, TEXT("startOnce(): Unable to start process %s, with command line - %s\r\n"), it->filePath.c_str(), it->arguments.c_str());
    }


#ifndef PUBLIC_RELEASE
    // Quick access to ActiveSync
    RegistryAccessor::setBool(HKEY_LOCAL_MACHINE, L"LGE\\SystemInfo", L"TEST_MODE", true);
    RegistryAccessor::setString(HKEY_CURRENT_USER, L"ControlPanel\\Comm", L"Cnct", L"`Default USB`");
    RegistryAccessor::setBool(HKEY_CURRENT_USER, L"ControlPanel\\Comm", L"AutoCnct", true);

    // Increase storage memory
    DWORD storage, ram, pageSize;
    static const DWORD cStarageSizeMB = 16;
    if(GetSystemMemoryDivision(&storage, &ram, &pageSize))
    {
        SetSystemMemoryDivision(cStarageSizeMB*1024*1024 / pageSize);
    }

    if(g_iniFile.GetBoolValue(TEXT("Debug"), TEXT("WantDesktop"), false))
    {
        CreateProcess(TEXT("\\windows\\explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
        exit(0);
    }
#endif
}

// Fix codec paths
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

typedef std::vector<RegistryEntry> RegistryData;

bool fixCodecsPath()
{
    std::wstring value;
    if(RegistryAccessor::getString(HKEY_CLASSES_ROOT, L"\\CLSID\\{0ba13ea1-70e5-11db-9690-00e08161165f}\\InprocServer32", L"", L"") != L"VideoRenderer.dll")
        return true;// Paths are already adjusted.
    
    RegistryData registryPathInfo;

    std::wstring codecsPath = Utils::makeFolderPath(std::wstring() + g_iniFile.GetValue(TEXT("Player"), TEXT("CodecsPath"), MODS_ROOT_PATH TEXT("codecs\\")));

    // Demux
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D24C840C-C469-4368-A363-0913B44AEF5C}\\InprocServer32", L"", codecsPath + L"avidmx.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{DC56E099-C9EA-49c8-9FA2-0A173D1522F1}\\InprocServer32", L"", codecsPath + L"mp3demux.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{025BE2E4-1787-4da4-A585-C5B2B9EEB57C}\\InprocServer32", L"", codecsPath + L"mp4demux.dll"));
    // Audio
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{C8F59247-8FAA-42d9-93A6-EF32961644B2}\\InprocServer32", L"", codecsPath + L"mp3decfilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{1F3F5741-A9EE-4bd9-B64E-99C5534B3817}\\InprocServer32", L"", codecsPath + L"ac3decfilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{313F1007-5458-4275-8143-E760A1D73D0F}\\InprocServer32", L"", codecsPath + L"aacdecfilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D1E456E1-47E5-497a-ABA1-A0C57C3CE5C1}\\InprocServer32", L"", codecsPath + L"speexdecfilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{D1E456E1-47E5-497a-ABA1-A0C57C3CE5C0}\\InprocServer32", L"", codecsPath + L"vorbisdecfilter.dll"));
    // FLAC
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{B380606B-B500-4001-ABA9-635D24D95504}\\InprocServer32", L"", codecsPath + L"losslessdecfilter.dll"));
    // APE
    //registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{B380606C-B500-4001-ABA9-635D24D95504}\\InprocServer32", L"", codecsPath + L"losslessdecfilter.dll"));
    // Video
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{692100F0-01C4-4af0-BDC2-C8BA5C5DED01}\\InprocServer32", L"", codecsPath + L"DecodeFilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{0ba13ea1-70e5-11db-9690-00e08161165f}\\InprocServer32", L"", codecsPath + L"VideoRenderer.dll"));
#ifndef STABLE_ONLY
    // Unstable
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{3E4DCA25-347E-4678-B22A-6F4CC68FF2A8}\\InprocServer32", L"", codecsPath + L"audiocorefilter.dll"));
    registryPathInfo.push_back(RegistryEntry(HKEY_CLASSES_ROOT, L"\\CLSID\\{9EBFDAAE-0963-4b5a-8B2E-EDB9B943820B}\\InprocServer32", L"", codecsPath + L"matroskafilter.dll"));
#endif

    for(RegistryData::const_iterator it = registryPathInfo.begin(); it != registryPathInfo.end(); ++it)
    {
        if(!RegistryAccessor::setString(it->rootKey, it->path, it->name, it->value))
        {
            return false;
        }
    }
     return true;
}

int extCheckMediaFilesExtList(const LPWSTR extValue)
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

int extCheckMediaFileMatch(const LPWSTR fileName)
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

int extCheckMediaFileMatch2(const LPWSTR fileName)
{
    return extCheckMediaFileMatch(fileName) == 0;
}
