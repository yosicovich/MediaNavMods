#pragma once
#include "stdafx.h"
#include <string>
#include <vector>

#ifdef TESTMODE
#define logPrintf NKDbgPrintfW
#else
#define logPrintf
#endif

#define debugPrintf(cond, ...) if(cond){ logPrintf(__VA_ARGS__);}
#define debugDump(cond, buf, size) if(cond) {Utils::dumpBinary(buf, size);}

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

        static std::vector<std::wstring> getSubKeys(HKEY hRootKey, const std::wstring& subKey);
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

    class CLock
    {
    public:
        CLock() 
        {
            InitializeCriticalSection(&m_CritSec);
        };

        ~CLock() 
        {
            DeleteCriticalSection(&m_CritSec);
        };

        void Lock() 
        {
            EnterCriticalSection(&m_CritSec);
        };

        void Unlock() 
        {
            LeaveCriticalSection(&m_CritSec);
        };
    private:
        CLock(const CLock &refLock);
        CLock &operator=(const CLock &refLock);

        CRITICAL_SECTION m_CritSec;
    };

    class CLockHolder
    {
    public:
        CLockHolder(CLock& lock)
            :m_lock(lock)
        {
            m_lock.Lock();
        };

        ~CLockHolder() 
        {
            m_lock.Unlock();
        };

    private:
        CLockHolder(const CLockHolder &refLockHolder);
        CLockHolder &operator=(const CLockHolder &refLockHolder);

        CLock& m_lock;
    };

    class FileLogger
    {
    public:
        FileLogger(const std::wstring& fileName)
            :m_file(NULL),
             m_fileName(fileName)
        {
        }

        ~FileLogger()
        {
            if(m_file)
                fclose(m_file);
        }

        void writeLog(const wchar_t* fmt, ...);
        void writeLog(const wchar_t* fmt, va_list args);

    private:
        CLock m_lock;
        std::wstring m_fileName;
        FILE* m_file;
    };

    DWORD getCurrentProcessImageBase();
    DWORD getCurrentProcessCodeBase();
    void toUpper(std::wstring& str);

    void dumpBinary(const void* buf, size_t size);
    void dumpGUID(const GUID* guid);

    std::wstring convertToWString(const std::string& str);
    
};

