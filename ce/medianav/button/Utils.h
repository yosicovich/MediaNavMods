#pragma once

#include <tlhelp32.h>
#include <string>
#include <Windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include <imaging.h>
#include <imgguids.h>

struct EnumData {
    DWORD dwPID;
    HWND hWnd;
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

class OleInitializer
{
public:
    OleInitializer();
    virtual ~OleInitializer();
private:
    static int m_useCount;
    static CLock m_accessLock;
};

bool IsProcessExist(const std::wstring& fullpath);
BOOL FileExist(const TCHAR *file);
FILETIME FileModifyTime(const TCHAR *file);
BOOL ShellCommand(const TCHAR* pCmdLine, const TCHAR* pParameters);

BYTE* LoadFileData(const TCHAR *file);
BOOL SaveFileData(const TCHAR *file, const BYTE* data, int dataLen);
BOOL SaveFileData(const TCHAR *file, const BYTE* data);

DWORD FindProcessId(const TCHAR *processName);
void KillProcess(DWORD PID);
void KillProcess(const TCHAR *processName);

BYTE* StrReplace(const char *search , const char *replace , char *subject);
bool GetCurrExePath(std::wstring& path);
HWND FindWindowFromProcessId(DWORD dwProcessId);
HWND FindWindowFromPath(const TCHAR *fullpath);
BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);
BOOL HasAlphaChannel(HBITMAP hBmp);
int GetSmallCurrExePath(LPWSTR smallpath);
HBITMAP LoadPicture(const std::wstring& pathName, bool& hasAlpha, int& width, int& height);
