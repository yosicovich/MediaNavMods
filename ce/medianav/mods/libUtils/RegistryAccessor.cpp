#include "utils.h"
#include <cassert>

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
        //	if(RegCreateKeyEx(hRootKey, subKey.c_str(), 0, NULL, 0, NULL, NULL, &hKey, NULL) != ERROR_SUCCESS)
        if(RegOpenKeyEx(hRootKey, subKey.c_str(), 0, NULL, &hKey) != ERROR_SUCCESS)
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

}// Utils