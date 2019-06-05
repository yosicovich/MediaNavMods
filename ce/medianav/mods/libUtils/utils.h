#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
#include <set>

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

    class CSharedLock
    {
    public:
        CSharedLock(const wchar_t* name) ;
        ~CSharedLock() ;

        void Lock() ;
        bool WaitLock(DWORD timeoutMs);
        void Unlock();
    private:
        CSharedLock(const CSharedLock &refLock);
        CSharedLock &operator=(const CSharedLock &refLock);

        HANDLE m_hMutex;
    };

    template <typename CLock> class CLockHolder
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

    class SystemWideUniqueInstance
    {
    public:
        SystemWideUniqueInstance(const std::wstring& name);
        bool isUnique();
    private:
        CSharedLock lock_;
        bool isUnique_;
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

    class OleInitializer
    {
    public:
        OleInitializer();
        virtual ~OleInitializer();
    private:
        static int m_useCount;
        static CLock m_accessLock;
    };

    DWORD getCurrentProcessImageBase();
    DWORD getCurrentProcessCodeBase();
    void toUpper(std::wstring& str);

    void dumpBinary(const void* buf, size_t size);
    void dumpGUID(const GUID* guid);

    std::wstring convertToWString(const std::string& str);

    bool checkRectCompleteCovered(HWND hWnd, RECT rect, const std::set<HWND>& skipWindows = std::set<HWND>());

#pragma pack(push,1)
    template <typename T>
    struct unaligned_read
    {
        T val;
    };
    template <typename T> __forceinline T readUnaligned(const void* ptr)
    {
        return reinterpret_cast<const unaligned_read<T> *>(ptr)->val;
    }
#pragma pack(pop)

    bool HasAlphaChannel(HBITMAP hBmp);
    HBITMAP LoadPicture(const TCHAR* pathName, bool& hasAlpha, int& width, int& height);

    inline std::wstring& makeFolderPath(std::wstring& str)
    {
        if(str.length() > 0 && str.at(str.length() - 1) != L'\\')
            str += L'\\';
        return str;
    }

    inline std::wstring getModulePath(HMODULE hModule)
    {
        wchar_t *pathBuf[MAX_PATH];
        DWORD nSize = GetModuleFileName(hModule, reinterpret_cast<wchar_t *>(pathBuf), MAX_PATH);
        if(nSize == MAX_PATH)
            return TEXT("");
        std::wstring path(reinterpret_cast<wchar_t *>(pathBuf), nSize);
        size_t lastBackSlash = path.find_last_of(L'\\');
        if(lastBackSlash == std::wstring::npos)
            return TEXT("");
        return path.substr(0, lastBackSlash + 1);
    }

    inline void oswprintf(std::wostream & oStream, const wchar_t* pFormat, ... )
    {
        va_list vaArgs;
        va_start( vaArgs, pFormat );
        wchar_t textBuf[1024];
        int wrote = _vsnwprintf(reinterpret_cast<wchar_t*>(&textBuf), 1024, pFormat, vaArgs);
        if(wrote <0 || wrote > 1024)
            oStream << TEXT("<text buffer overflow required size:") << wrote << TEXT(">");
        else
            oStream << textBuf;
        va_end(vaArgs);

    }

    inline bool isPathPresent(const std::wstring& path)
    {
        return GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES;
    }

    bool detectPath(const std::wstring& path, DWORD timeoutS);
};

