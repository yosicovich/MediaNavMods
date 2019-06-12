#include "PETools.h"

#include <pehdr.h>
#include <Psapi.h>
#include <pkfuncs.h>
#define WINCEMACRO
#include <psyscall.h>
#include <mkfuncs.h>

namespace PETools
{
    ImportsAccesor::ImportsAccesor()
        :m_imageBase(0)
    {
        void* pImage = getCurrentProcessImageBase();
        if(pImage == NULL)
            throw std::exception();
        m_imageBase = reinterpret_cast<DWORD>(pImage);

        PIMAGE_IMPORT_DESCRIPTOR pImports;
#ifdef UNDER_CE
        e32_lite e32;
        if(!ReadPEHeader(GetCurrentProcess(), RP_READ_PE_BY_MODULE, NULL, &e32, sizeof(e32)))
            throw std::exception();
        pImports = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(rvaToVa(e32.e32_unit[IMP].rva));
#else
        wchar_t moduleFileName[MAX_PATH];
        DWORD dwResult = GetModuleFileName(NULL, reinterpret_cast<wchar_t *>(moduleFileName), MAX_PATH);
        if(dwResult == 0 || dwResult == MAX_PATH)
            throw std::exception();

        HANDLE hFile = CreateFile(reinterpret_cast<wchar_t *>(moduleFileName), GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
            throw std::exception();

        DWORD bytesRead = 0;
        IMAGE_DOS_HEADER dosHeader;
        if(!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, NULL) || bytesRead != sizeof(dosHeader))
        {
            CloseHandle(hFile);
            throw std::exception();
        }

        if(SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN) != dosHeader.e_lfanew)
        {
            CloseHandle(hFile);
            throw std::exception();
        }

        IMAGE_NT_HEADERS ntHeader;
        if(!ReadFile(hFile, &ntHeader, sizeof(ntHeader), &bytesRead, NULL) || bytesRead != sizeof(ntHeader))
        {
            CloseHandle(hFile);
            throw std::exception();
        }
        CloseHandle(hFile);
        pImports = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(rvaToVa(ntHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));
#endif

        while(pImports->Name != NULL)
        {
            std::string name = Utils::toUpper(std::string(reinterpret_cast<char*>(rvaToVa(pImports->Name))));
            PE_IMPORT_DLL& importDll = m_imports[name];
            importDll.DllName = name;

            PIMAGE_THUNK_DATA pFuncs;
            if(pImports->OriginalFirstThunk > 0)    
                pFuncs = reinterpret_cast<PIMAGE_THUNK_DATA>(rvaToVa(pImports->OriginalFirstThunk));
            else
                pFuncs = reinterpret_cast<PIMAGE_THUNK_DATA>(rvaToVa(pImports->FirstThunk));

            PIMAGE_THUNK_DATA pFuncAddrs = reinterpret_cast<PIMAGE_THUNK_DATA>(rvaToVa(pImports->FirstThunk));
            while(pFuncs->u1.AddressOfData != 0)
            {
                PE_IMPORT_FUNCTION importFunction;
                if (pFuncs->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
                    importFunction.FunctionId = IMAGE_ORDINAL32(pFuncs->u1.Ordinal);
                } else {
                    importFunction.FunctionName = reinterpret_cast<char*>(rvaToVa(reinterpret_cast<DWORD>(&pFuncs->u1.AddressOfData->Name[0])));
                }
                importFunction.VirtualAddress = reinterpret_cast<DWORD>(pFuncAddrs);

                importDll.Functions.push_back(importFunction);

                ++pFuncs;
                ++pFuncAddrs;
            }
            ++pImports;
        }
    }

    const void** ImportsAccesor::getFunctionPtr(const std::string& dllName, const std::string& functionName) const
    {
        TImportsMap::const_iterator it = m_imports.find(Utils::toUpper(std::string(dllName)));
        if(it == m_imports.end())
            return NULL;

        const PE_IMPORT_DLL& dll = it->second;
        for(size_t i = 0; i < dll.Functions.size(); ++i)
        {
            if(dll.Functions[i].FunctionName == functionName)
                return reinterpret_cast<const void**>(dll.Functions[i].VirtualAddress);
        }
        return NULL;
    }

    const void** ImportsAccesor::getFunctionPtr(const std::string& dllName, int ordinal) const
    {
        TImportsMap::const_iterator it = m_imports.find(Utils::toUpper(std::string(dllName)));
        if(it == m_imports.end())
            return NULL;

        const PE_IMPORT_DLL& dll = it->second;
        for(size_t i = 0; i < dll.Functions.size(); ++i)
        {
            if(dll.Functions[i].FunctionId == ordinal)
                return reinterpret_cast<const void**>(dll.Functions[i].VirtualAddress);
        }
        return NULL;
    }

    const PETools::ImportsAccesor::TImportsMap ImportsAccesor::getImports() const
    {
        return m_imports;
    }

    void* getCurrentProcessImageBase()
    {
        MODULEINFO moduleInfo;
        BOOL bResult = GetModuleInformation(GetCurrentProcess(), NULL, &moduleInfo, sizeof(moduleInfo));
        if(bResult)
        {
            return moduleInfo.lpBaseOfDll;
        }
        return NULL;
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

}

