#include "medianav.h"
#include <exception>
#include <CmnDLL.h>

using namespace MediaNav;
using namespace Utils;

MediaNav::CSharedMemory::CSharedMemory(const wchar_t* mutexName, const wchar_t* memName, bool readOnly, DWORD size)
:m_lock(mutexName)
,m_hMem(MSHM_Dll_CreateShmClassObj())
{
    CLockHolder<CSharedLock> lock(m_lock);
    bool bResult;
    if(readOnly)
    {
        bResult = MSHM_Dll_MakeMappingReadOnly(m_hMem, memName, INVALID_HANDLE_VALUE);
    }else
    {
        bResult = MSHM_Dll_MakeMappingReadWrite(m_hMem, memName, size, INVALID_HANDLE_VALUE);
    }

    if(!bResult)
        throw std::exception("Unable to map shared memory");
}

MediaNav::CSharedMemory::~CSharedMemory()
{
    CLockHolder<CSharedLock> lock(m_lock);
    MSHM_Dll_DestroyShmClassObj(m_hMem);
}

void MediaNav::CSharedMemory::read(void* pBuf, DWORD readPos, DWORD size)
{
    MSHM_Dll_Read(m_hMem, pBuf, readPos, size);
}

void MediaNav::CSharedMemory::write(const void* pBuf, DWORD writePos, DWORD size)
{
    MSHM_Dll_Write(m_hMem, pBuf, writePos, size);
}
