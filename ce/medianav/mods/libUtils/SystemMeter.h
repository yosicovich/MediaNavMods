#pragma once
#include "stdafx.h"
namespace Utils
{
    class SystemMeter
    {
    public:
        SystemMeter();
        virtual ~SystemMeter();
        DWORD getCPULoad() const;
        MEMORYSTATUS getMemoryStatus() const;
    private:
        void getSystemLoad();
        static void idleThread(LPVOID pvParams);
        static void checkThread(LPVOID pvParams);
        static DWORD GetThreadTick(FILETIME* a, FILETIME* b);
    private:
        bool    m_threadsCreated;
        HANDLE  m_hIdleThread;
        HANDLE  m_hCheckThread;
        DWORD   m_dwLastThreadTime;
        DWORD   m_dwLastTickTime;
        HANDLE  m_hExitEvent;

        DWORD   m_dwCPULoad;
    };
};

