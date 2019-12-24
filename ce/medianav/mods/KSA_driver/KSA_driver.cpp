// KSA_driver.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "KSA_driver.h"
#include <pkfuncs.h>

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



KSADriverData* KSA_Init(const wchar_t* pRegistryPath)
{
    KSADriverData* pData = new KSADriverData();
    debugPrintf(DBG_TRACE, L"KSA: Init KSA_Init = 0x%08X, pData = 0x%08X\r\n", KSA_Init, pData);
    return pData;
}

BOOL KSA_Deinit(KSADriverData* pKSAData)
{
    debugPrintf(DBG_TRACE, L"KSA: Deinit, pData = 0x%08X\r\n", pKSAData);
    delete pKSAData;
    return TRUE;
}

KSAInstanceData* KSA_Open( 
             KSADriverData* pKSAData, // @parm Handle returned by KSA_Init.
             DWORD   accessCode,     // @parm access code.
             DWORD   shareMode       // @parm share mode - Not used in this driver.
             )
{
    debugPrintf(DBG_TRACE, L"KSA: Open pKSAData = 0x%08X\r\n", pKSAData);
    CLockHolder<CLock> lock(pKSAData->m_accessLock);

    return new KSAInstanceData(reinterpret_cast<DWORD>(KSA_Open));
}

BOOL KSA_Close(KSAInstanceData* pKSAData)
{
    debugPrintf(DBG_TRACE, L"KSA: Close pKSAData = 0x%08X\r\n", pKSAData);
    delete pKSAData;
    return TRUE;
}

VOID KSA_PowerDown(KSADriverData* pKSAData)
{
    return;
}

VOID KSA_PowerUp(KSADriverData* pKSAData)
{
    return;
}

DWORD KSA_Seek(KSAInstanceData* pKSAData, DWORD amount, WORD type)
{
    debugPrintf(DBG_TRACE, L"KSA: Seek amount = 0x%08X, type = 0x%04hX\r\n", amount, type);
    CLockHolder<CLock> lock(pKSAData->m_accessLock);
    switch (type)
    {
    case FILE_BEGIN:
        pKSAData->pCurPosition = amount;
        break;
    case FILE_CURRENT:
        pKSAData->pCurPosition += amount;
        break;
    case FILE_END:
        pKSAData->pCurPosition = 0xFFFFFFFF - amount;
        break;
    }

    return pKSAData->pCurPosition;
}

DWORD KSA_Read(KSAInstanceData* pKSAData, void* pBuffer, DWORD count)
{
    debugPrintf(DBG_TRACE, L"KSA: Read pBuffer = 0x%08X, count = 0x%08X\r\n", pKSAData, count);
    CLockHolder<CLock> lock(pKSAData->m_accessLock);
    BOOL bRes = CeSafeCopyMemory(pBuffer, reinterpret_cast<const void*>(pKSAData->pCurPosition), count);
    if(!bRes)
    {
        debugPrintf(DBG_TRACE, L"KSA: Read failed\r\n");
        return -1;
    }
    pKSAData->pCurPosition += count;
    return count;
}

DWORD KSA_Write(KSAInstanceData* pKSAData, const void* pBuffer, DWORD count)
{
    debugPrintf(DBG_TRACE, L"KSA: Write pBuffer = 0x%08X, count = 0x%08X\r\n", pKSAData, count);
    CLockHolder<CLock> lock(pKSAData->m_accessLock);
    void* pSpaceData = VirtualAllocCopyEx(GetCurrentProcess(), GetCurrentProcess(), reinterpret_cast<void* >(pKSAData->pCurPosition), count, PAGE_EXECUTE_READWRITE);
    if(!pSpaceData)
    {
        debugPrintf(DBG, L"KSA: Write VirtualAllocCopyEx failed with address = 0x%08X, count = 0x%08X\r\n", pKSAData->pCurPosition, count);
        return -1;
    }

    BOOL bRes = CeSafeCopyMemory(pSpaceData, pBuffer, count);
    if(!VirtualFree(pSpaceData, 0 ,MEM_RELEASE))
    {
        debugPrintf(DBG, L"KSA: Write VirtualFree failed\r\n");
    }

    if(!bRes)
        return -1;
    pKSAData->pCurPosition += count;
    return count;
}

BOOL KSA_IOControl( KSADriverData* pKSAData, DWORD dwCode, const void* pBufIn, DWORD dwLenIn, void* pBufOut, DWORD dwLenOut,DWORD* pdwActualOut)
{
    CLockHolder<CLock> lock(pKSAData->m_accessLock);
    return TRUE;
}
