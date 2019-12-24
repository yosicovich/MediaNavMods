// GPS_driver.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "GPS_driver.h"
#include <Devload.h.>
#include <pegdser.h>

using namespace Utils;
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(static_cast<HMODULE>(hModule));
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

GPSDriverData* GPS_Init(const wchar_t* pRegistryPath)
{
    std::auto_ptr<GPSDriverData> pData(new GPSDriverData());
    debugPrintf(DBG_TRACE, L"GPS: Init GPS_Init = 0x%08X, pData = 0x%08X\r\n", GPS_Init, pData);
    HKEY driverKey = OpenDeviceKey(pRegistryPath);
    if(driverKey == INVALID_HANDLE_VALUE)
        return NULL;
    std::wstring port = Utils::RegistryAccessor::getString(driverKey, TEXT(""), TEXT("DevicePathName"), TEXT("COM4:"));
    RegCloseKey(driverKey);
    debugPrintf(DBG, L"GPS: GPS_Init Port = %s, path = %s\r\n", port.c_str(), pRegistryPath);
    pData->m_port = CreateFile(
        port.c_str(),
        GENERIC_READ | GENERIC_WRITE,    // Access (read/write) mode
        0,                                // Share mode
        NULL,                         // Pointer to the security attribute
        OPEN_EXISTING,     // How to open the serial port
        0,                                // Port attributes
        NULL);// Handle to port with attribute to copy

    if(pData->m_port == INVALID_HANDLE_VALUE)
        return NULL;
    
    // Configure port here
    
    // Set to the same values as Navitel sets
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 10;
    if(!SetCommTimeouts(pData->m_port, &timeouts))
    {
        debugPrintf(DBG, TEXT("GPS: GPS_Init SetCommTimeouts() failed.\r\n"));
    }
    DCB commDCB;
    if(GetCommState(pData->m_port, &commDCB))
    {
        commDCB.BaudRate = 9600;
        commDCB.ByteSize = 8;
        // Modify parity and whatever else is needed here
        if(!SetCommState(pData->m_port, &commDCB))
        {
            debugPrintf(DBG, TEXT("GPS: GPS_Init SetCommState() failed.\r\n"));
        }
    }
    
    if(!PurgeComm(pData->m_port, PURGE_RXCLEAR |PURGE_TXCLEAR))
    {
        debugPrintf(DBG, TEXT("GPS: GPS_Init PurgeComm() failed.\r\n"));
    }
    COMSTAT comStat;
    DWORD commError;
    if(!ClearCommError(pData->m_port, &commError, &comStat))
    {
        debugPrintf(DBG, TEXT("GPS: GPS_Init ClearCommError() failed.\r\n"));
    }

    pData->m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(!pData->m_hExitEvent)
        return NULL;

    DWORD ThreadID;
    pData->m_hCheckThread = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(checkThread), pData.get(), CREATE_SUSPENDED, &ThreadID);

    if( !pData->m_hCheckThread )
        return NULL;

    ResetEvent(pData->m_hExitEvent);
    ResumeThread(pData->m_hCheckThread);

    return pData.release();
}

BOOL GPS_Deinit(GPSDriverData* pGPSData)
{
    debugPrintf(DBG_TRACE, L"GPS: Deinit\r\n", GPS_Deinit);
    SetEvent(pGPSData->m_hExitEvent);
    WaitForSingleObject(pGPSData->m_hCheckThread, 3000);
    delete pGPSData;
    return TRUE;
}

GPSInstanceData* GPS_Open( 
             GPSDriverData* pGPSData, // @parm Handle returned by GPS_Init.
             DWORD   accessCode,     // @parm access code.
             DWORD   shareMode       // @parm share mode - Not used in this driver.
             )
{
    debugPrintf(DBG_TRACE, L"GPS: Open pGPSData = 0x%08X\r\n", pGPSData);
    CLockHolder<CLock> lock(pGPSData->m_accessLock);

    std::auto_ptr<GPSInstanceData> pInstance(new GPSInstanceData(pGPSData, accessCode, shareMode));
    pInstance->m_readSemaphore = CreateSemaphore( 
        NULL,    // default security attributes
        0,       // initial count
        1,       // maximum count
        NULL);   // unnamed semaphore

    if (!pInstance->m_readSemaphore) 
        return NULL;

    pGPSData->m_instances.insert(pInstance.get());
    return pInstance.release();
}

BOOL GPS_Close(GPSInstanceData* pGPSData)
{
    debugPrintf(DBG_TRACE, L"GPS: Close pGPSData = 0x%08X\r\n", pGPSData);
    {
        CLockHolder<CLock> lock(pGPSData->m_pDriver->m_accessLock);
        pGPSData->m_pDriver->m_instances.erase(pGPSData);
    }
    delete pGPSData;
    return TRUE;
}

VOID GPS_PowerDown(GPSDriverData* pGPSData)
{
    return;
}

VOID GPS_PowerUp(GPSDriverData* pGPSData)
{
    return;
}

DWORD GPS_Seek(GPSInstanceData* pGPSData, DWORD amount, WORD type)
{
    debugPrintf(DBG_TRACE, L"GPS: Seek amount = 0x%08X, type = 0x%04hX\r\n", amount, type);
    // Not supported
    return -1;
}

DWORD readCycle(GPSInstanceData* pGPSData, void* &pBuffer, DWORD &count)
{
    CLockHolder<CLock> lock(pGPSData->m_accessLock);
    DWORD readCount = 0;
    while(count > 0)
    {
        if(!pGPSData->m_curMsg.pMsg || pGPSData->m_curMsg.curPos >= pGPSData->m_curMsg.size)
        {
            if(pGPSData->m_queue.empty())
            {
                debugPrintf(DBG_TRACE, L"GPS: Read no more data\r\n");
                return readCount;
            }
            pGPSData->m_curMsg = pGPSData->m_queue.front();
            pGPSData->m_queue.pop();
        }
        size_t copyCount = min(pGPSData->m_curMsg.size - pGPSData->m_curMsg.curPos, count);
        BOOL bRes = CeSafeCopyMemory(pBuffer, reinterpret_cast<const void*>(&pGPSData->m_curMsg.pMsg[pGPSData->m_curMsg.curPos]), copyCount);
        if(!bRes)
        {
            debugPrintf(DBG, L"GPS: Read failed\r\n");
            return -1;
        }
        pGPSData->m_curMsg.curPos += copyCount;
        readCount += copyCount;
        count -= copyCount;
        pBuffer = reinterpret_cast<BYTE*>(pBuffer) + copyCount;
    }

    return readCount;
}

DWORD GPS_Read(GPSInstanceData* pGPSData, void* pBuffer, DWORD count)
{
    debugPrintf(DBG_TRACE, L"GPS: Read pBuffer = 0x%08X, count = 0x%08X\r\n", pGPSData, count);
    if(!(pGPSData->m_accessCode & GENERIC_READ))
    {
        SetLastError(ERROR_INVALID_ACCESS);
        return -1;
    }
    DWORD ReadTotalTimeoutMultiplier;
    DWORD ReadTotalTimeoutConstant;
    {
        CLockHolder<CLock> lock(pGPSData->m_accessLock);
        ReadTotalTimeoutMultiplier = pGPSData->ReadTotalTimeoutMultiplier;
        ReadTotalTimeoutConstant = pGPSData->ReadTotalTimeoutConstant;
    }
    DWORD readCount = 0;
    while(count > 0)
    {
        readCount += readCycle(pGPSData, pBuffer, count);
        if(count > 0 && WaitForSingleObject(pGPSData->m_readSemaphore, ReadTotalTimeoutMultiplier * count + ReadTotalTimeoutConstant) == WAIT_TIMEOUT)
            return readCount;
    }

    return readCount;
}

DWORD GPS_Write(GPSInstanceData* pGPSData, const void* pBuffer, DWORD count)
{
    debugPrintf(DBG_TRACE, L"GPS: Write pBuffer = 0x%08X, count = 0x%08X\r\n", pGPSData, count);
    if(!(pGPSData->m_accessCode & GENERIC_WRITE))
    {
        SetLastError(ERROR_INVALID_ACCESS);
        return -1;
    }
    // No write supported but report OK to avoid connection breaks.
    return count;
}

BOOL GPS_IOControl( GPSInstanceData* pGPSData, DWORD dwCode, const void* pBufIn, DWORD dwLenIn, void* pBufOut, DWORD dwLenOut,DWORD* pdwActualOut)
{
    CLockHolder<CLock> lock(pGPSData->m_accessLock);
    switch ( dwCode ) 
    {
    case IOCTL_SERIAL_GET_COMMSTATUS :
        {
            debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_GET_COMMSTATUS\r\n");
            PSERIAL_DEV_STATUS pSerialDevStat;

            if ( (dwLenOut < sizeof(SERIAL_DEV_STATUS)) || (NULL == pBufOut) ||(NULL == pdwActualOut) ) 
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            pSerialDevStat = (PSERIAL_DEV_STATUS)pBufOut;

            // Set The Error Mask
            pSerialDevStat->Errors = 0;

            // Clear the ComStat structure & get PDD related status
            memset ((char *) &(pSerialDevStat->ComStat), 0, sizeof(COMSTAT));

            // Return the size
            *pdwActualOut = sizeof(SERIAL_DEV_STATUS);

            break;
        }
    case IOCTL_SERIAL_GET_PROPERTIES :
        {
            debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_GET_PROPERTIES\r\n");
            if ( (dwLenOut < sizeof(COMMPROP)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            LPCOMMPROP pProps = reinterpret_cast<LPCOMMPROP>(pBufOut);
            // Clear the ComMProp structure
            memset(pProps , 0, sizeof(COMMPROP));
            pProps->dwMaxBaud = BAUD_9600;
            pProps->dwSettableBaud = BAUD_9600;
            pProps->wSettableData = DATABITS_8;
            pProps->wSettableStopParity = /*PARITY_EVEN | PARITY_ODD | */PARITY_NONE | STOPBITS_10;

            // Return the size
            *pdwActualOut = sizeof(COMMPROP);
            break;
        }
    case IOCTL_SERIAL_SET_TIMEOUTS :
        debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_SET_TIMEOUTS\r\n");
        if ( (dwLenIn < sizeof(COMMTIMEOUTS)) || (NULL == pBufIn) ) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        {
            const COMMTIMEOUTS* pTimeouts = reinterpret_cast<const COMMTIMEOUTS*>(pBufIn);
            CLockHolder<CLock> lock(pGPSData->m_accessLock);
            pGPSData->ReadTotalTimeoutMultiplier = pTimeouts->ReadTotalTimeoutMultiplier;
            pGPSData->ReadTotalTimeoutConstant = pTimeouts->ReadTotalTimeoutConstant;
        }
        break;

    case IOCTL_SERIAL_GET_TIMEOUTS :
        {
            debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_GET_TIMEOUTS\r\n");
            if ( (dwLenOut < sizeof(COMMTIMEOUTS)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            LPCOMMTIMEOUTS pTimeouts = reinterpret_cast<LPCOMMTIMEOUTS>(pBufOut);
            memset(pTimeouts, 0, sizeof(COMMTIMEOUTS));

            {
                CLockHolder<CLock> lock(pGPSData->m_accessLock);
                pTimeouts->ReadTotalTimeoutMultiplier = pGPSData->ReadTotalTimeoutMultiplier;
                pTimeouts->ReadTotalTimeoutConstant = pGPSData->ReadTotalTimeoutConstant;
            }

            // Return the size
            *pdwActualOut = sizeof(COMMTIMEOUTS);
            break;
        }

    case IOCTL_SERIAL_PURGE :
        debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_PURGE\r\n");
        if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        if(*reinterpret_cast<const DWORD*>(pBufIn) & (PURGE_RXABORT | PURGE_RXCLEAR))
        {
            CLockHolder<CLock> lock(pGPSData->m_accessLock);
            while(!pGPSData->m_queue.empty())
                pGPSData->m_queue.pop();
        }

        break;
    case IOCTL_SERIAL_SET_QUEUE_SIZE :
        debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_SET_QUEUE_SIZE\r\n");
        if ( (dwLenIn < sizeof(SERIAL_QUEUE_SIZES)) || (NULL == pBufIn) ) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    case IOCTL_SERIAL_GET_DCB :
        {
            debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_GET_DCB\r\n");
            if ( (dwLenOut < sizeof(DCB)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
            {
                SetLastError (ERROR_INVALID_PARAMETER);
                return FALSE;
            }
            LPDCB pDCB = reinterpret_cast<LPDCB>(pBufOut);
            memset(pDCB, 0, sizeof(DCB));
            pDCB->BaudRate = BAUD_9600;
            pDCB->ByteSize = 8;

            // Return the size
            *pdwActualOut = sizeof(DCB);
            break;
        }
    case IOCTL_SERIAL_SET_DCB :
        debugPrintf(DBG, L"GPS: GPS_IOControl() IOCTL_SERIAL_SET_DCB\r\n");
        if ( (dwLenIn < sizeof(DCB)) || (NULL == pBufIn) ) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;
    case IOCTL_SERIAL_IMMEDIATE_CHAR :
    case IOCTL_SERIAL_ENABLE_IR :
    case IOCTL_SERIAL_DISABLE_IR :
        break;

    case IOCTL_SERIAL_SET_BREAK_ON :
    case IOCTL_SERIAL_SET_BREAK_OFF :
    case IOCTL_SERIAL_SET_DTR :
    case IOCTL_SERIAL_CLR_DTR :
    case IOCTL_SERIAL_SET_RTS :
    case IOCTL_SERIAL_CLR_RTS :
    case IOCTL_SERIAL_SET_XOFF :
    case IOCTL_SERIAL_SET_XON :
    case IOCTL_SERIAL_GET_WAIT_MASK :
    case IOCTL_SERIAL_SET_WAIT_MASK :
    case IOCTL_SERIAL_WAIT_ON_MASK :
    case IOCTL_SERIAL_GET_MODEMSTATUS :
    default :
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }
    return TRUE;
}

void checkThread(GPSDriverData* pGPSDriverData)
{

    while(WaitForSingleObject(pGPSDriverData->m_hExitEvent, 0) == WAIT_TIMEOUT )
    {
        debugPrintf(DBG_TRACE, TEXT("GPS: checkThread() %d\r\n"), GetTickCount());
        TGPSData gpsData(new BYTE[cGPSMsgSize]);
        DWORD readCount;
        if(!ReadFile(pGPSDriverData->m_port, *gpsData, cGPSMsgSize, &readCount, NULL))
            continue;

        if(!readCount)
            continue;
        
        CLockHolder<CLock> lock(pGPSDriverData->m_accessLock);
        if(pGPSDriverData->m_instances.empty())
            continue;
        GPSMsg msg(gpsData, readCount);
        for(TInstances::iterator it = pGPSDriverData->m_instances.begin(); it != pGPSDriverData->m_instances.end(); ++it)
        {
            CLockHolder<CLock> lock((*it)->m_accessLock);
            (*it)->m_queue.push(msg);
            ReleaseSemaphore((*it)->m_readSemaphore, 1, NULL);
        }
    }
}
