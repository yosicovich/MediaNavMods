#include "SystemMeter.h"


namespace Utils {

SystemMeter::SystemMeter()
:m_threadsCreated(false)
,m_hCheckThread(NULL)
,m_hIdleThread(NULL)
,m_dwLastThreadTime(0)
,m_dwLastTickTime(0)
,m_bCheckThreadExit(false)
,m_bIdleThreadExit(false)
,m_dwCPULoad(0)
{
    DWORD ThreadID;
    m_hIdleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SystemMeter::idleThread, (LPVOID)this, CREATE_SUSPENDED, &ThreadID);

    if( m_hIdleThread == NULL )
    {
        return;
    }
    SetThreadPriority(m_hIdleThread, THREAD_PRIORITY_IDLE);
    ResumeThread(m_hIdleThread);
    
    m_hCheckThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SystemMeter::checkThread, (LPVOID)this, CREATE_SUSPENDED, &ThreadID);

    if( m_hCheckThread == NULL )
    {
        m_bIdleThreadExit = true;
        WaitForSingleObject(m_hIdleThread, 3000);
        return;
    }
    ResumeThread(m_hCheckThread);

    m_threadsCreated = true;
}

SystemMeter::~SystemMeter()
{
    if(!m_threadsCreated)
        return;

    m_bCheckThreadExit = true;
    WaitForSingleObject(m_hCheckThread, 3000);

    m_bIdleThreadExit = true;
    WaitForSingleObject(m_hIdleThread, 3000);

}

DWORD SystemMeter::getCPULoad() const
{
    return m_dwCPULoad;
}

MEMORYSTATUS SystemMeter::getMemoryStatus() const
{
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(MEMORYSTATUS);

    // memory history
    GlobalMemoryStatus(&ms);
    
    return ms;
}

void SystemMeter::getSystemLoad()
{
    DWORD dwCurrentThreadTime =0;
    DWORD dwCurrentTickTime = 0;

    FILETIME ftCreationTime;
    FILETIME ftExitTime;
    FILETIME ftKernelTime;
    FILETIME ftUserTime;

    DWORD dwCpuPower;

    // search status
    SuspendThread(m_hIdleThread);

    dwCurrentTickTime = GetTickCount();
    GetThreadTimes(m_hIdleThread, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime);
    dwCurrentThreadTime = GetThreadTick(&ftKernelTime, &ftUserTime);

    // calculate cpu power
    if( dwCurrentTickTime != m_dwLastTickTime || m_dwLastThreadTime != 0 || m_dwLastTickTime != 0)
        dwCpuPower = 100 - (((dwCurrentThreadTime - m_dwLastThreadTime) * 100) / (dwCurrentTickTime - m_dwLastTickTime));
    else
        dwCpuPower = 0;	// avoid 0div

    // save status
    m_dwLastTickTime = GetTickCount();
    m_dwLastThreadTime = dwCurrentThreadTime;

    ResumeThread(m_hIdleThread);
    
    // Update data
    m_dwCPULoad = dwCpuPower;
}

void SystemMeter::idleThread(LPVOID pvParams)
{
    SystemMeter* pSelf = reinterpret_cast<SystemMeter*>(pvParams);

    while(!pSelf->m_bIdleThreadExit);
}

void SystemMeter::checkThread(LPVOID pvParams)
{
    SystemMeter* pSelf = reinterpret_cast<SystemMeter*>(pvParams);

    while(!pSelf->m_bCheckThreadExit)
    {
        pSelf->getSystemLoad();
        Sleep(1000);
    }
}

DWORD SystemMeter::GetThreadTick(FILETIME* a, FILETIME* b)
{
    __int64 a64 = 0;
    __int64 b64 = 0;
    a64 = a->dwHighDateTime;
    a64 <<= 32;
    a64 += a->dwLowDateTime;

    b64 = b->dwHighDateTime;
    b64 <<= 32;
    a64 += b->dwLowDateTime;

    a64 += b64;

    // nano sec to milli sec
    a64 /= 10000;

    return (DWORD)a64;
}

}; //namespace Utils

