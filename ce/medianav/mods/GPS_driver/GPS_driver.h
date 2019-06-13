#include "StdAfx.h"
#include <vector>
#include <set>
#include <queue>
#include <smartptr.h>

//static const wchar_t* cGPSDriverKey = TEXT("\\Drivers\\Builtin\\GPS");

static const size_t cGPSMsgSize = 256;
typedef smart_array<BYTE> TGPSData;

struct GPSMsg
{
    GPSMsg()
        :size(0)
        ,curPos(0)
    {
    }

    GPSMsg(const TGPSData& gpsData, size_t size)
        :pMsg(gpsData)
        ,size(size)
        ,curPos(0)
    {
    }
    size_t size;
    size_t curPos;
    TGPSData pMsg;
};
//typedef BYTE GPSMsg[cGPSMsgSize];
typedef std::queue<GPSMsg> GPSMsgQueue;
struct GPSDriverData;

struct GPSInstanceData
{
    GPSMsg m_curMsg;
    Utils::CLock m_accessLock;
    GPSMsgQueue m_queue;
    GPSDriverData* m_pDriver;
    HANDLE  m_readSemaphore;
    DWORD ReadTotalTimeoutMultiplier;
    DWORD ReadTotalTimeoutConstant;
    DWORD m_accessCode;
    DWORD m_shareMode;

    GPSInstanceData(GPSDriverData* pDriver, DWORD accessCode, DWORD shareMode)
        :m_pDriver(pDriver)
        ,m_readSemaphore(NULL)
        ,ReadTotalTimeoutMultiplier(0)
        ,ReadTotalTimeoutConstant(50)
        ,m_accessCode(accessCode)
        ,m_shareMode(shareMode)
    {
    }

    virtual ~GPSInstanceData()
    {
        if(m_readSemaphore != NULL)
            CloseHandle(m_readSemaphore);
    }
};

typedef std::set<GPSInstanceData*> TInstances;
struct GPSDriverData
{
    GPSDriverData()
        :m_hExitEvent(NULL)
        ,m_port(INVALID_HANDLE_VALUE)
        ,m_hCheckThread(NULL)
    {
    }
    Utils::CLock m_accessLock;
    TInstances m_instances;
    HANDLE  m_hExitEvent;
    HANDLE  m_port;
    HANDLE  m_hCheckThread;

    virtual ~GPSDriverData()
    {
        if(m_hCheckThread)
            CloseHandle(m_hCheckThread);

        if(m_port != INVALID_HANDLE_VALUE)
            CloseHandle(m_port);

        if(m_hExitEvent)
            CloseHandle(m_hExitEvent);
    }

};

void checkThread(GPSDriverData* pGPSDriverData);
