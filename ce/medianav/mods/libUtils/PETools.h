#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <pkfuncs.h>
#include "utils.h"

namespace PETools
{
    class ImportsAccesor
    {
    private:
        struct PE_IMPORT_FUNCTION {
            PE_IMPORT_FUNCTION()
                :FunctionId(0), VirtualAddress(0)
            {

            }
            std::string			FunctionName;
            int					FunctionId;
            DWORD               VirtualAddress;
        };

        struct PE_IMPORT_DLL {
            std::string				DllName;
            std::vector<PE_IMPORT_FUNCTION> Functions;
        };

    public:
        typedef std::map<std::string, PE_IMPORT_DLL> TImportsMap;

        ImportsAccesor();
        const void** getFunctionPtr(const std::string& dllName, const std::string& functionName) const;
        const void** getFunctionPtr(const std::string& dllName, int ordinal) const;
        const TImportsMap getImports() const;
    private:
        DWORD m_imageBase;
        TImportsMap m_imports;

        inline DWORD rvaToVa(DWORD rva)
        {
            return m_imageBase + rva;
        }
    };

    void* getCurrentProcessImageBase();
    DWORD getCurrentProcessCodeBase();
}