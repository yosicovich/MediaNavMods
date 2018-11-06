#pragma once
#include "stdafx.h"
#include <string>
#include <vector>

#ifdef TESTMODE
#define logPrintf NKDbgPrintfW
#else
#define logPrintf
#endif

namespace Utils
{

    class RegistryAccessor
    {
    public:
        static bool setValue(HKEY hRootKey, const std::wstring& subKey, DWORD dwType, const std::wstring& valueName, const std::vector<BYTE>& data);
        static bool setString(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value);
        static bool setInt(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, unsigned int value);
        static bool setBool(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, bool value);

        static bool getValue(HKEY hRootKey, const std::wstring& subKey, DWORD dwType, const std::wstring& valueName, std::vector<BYTE>& data);
        static std::wstring getString(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& defaultValue);
        static DWORD getInt(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, DWORD defaultValue);
        static bool getBool(HKEY hRootKey, const std::wstring& subKey, const std::wstring& valueName, bool defaultValue);
    };

    class SystemWideUniqueInstance
    {
    public:
        SystemWideUniqueInstance(const std::wstring& name);
        virtual ~SystemWideUniqueInstance();
        bool isUnique();
    private:
        HANDLE hMutex_;
        bool isUnique_;
    };

    DWORD getCurrentProcessImageBase();
    DWORD getCurrentProcessCodeBase();
    void toUpper(std::wstring& str);
};

