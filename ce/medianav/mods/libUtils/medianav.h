// Define MediaNav specifics
#pragma once

#include <windows.h>
#include "utils.h"
#include "smartptr.h"
#include <CmnDLL.h>

namespace MediaNav
{
#pragma warning(disable: 4200)
#pragma pack(push,1)

    typedef wchar_t MediaInfoStr[0x104];
    struct PlayerTimeData
    {
        BYTE m_curMin;
        BYTE m_curSec;
        BYTE m_curHour;
        BYTE m_totMin;
        BYTE m_totSec;
        BYTE m_totHour;
    };

    struct PlayerConfig
    {
        DWORD m_bRepeatMode; //0 - none; 1 - track; 2 - folder; 3 - all
        DWORD m_bShuffle;
    };

    struct USBPlayerStatus
    {
        PlayerTimeData m_timeData;
        MediaInfoStr m_song;
        MediaInfoStr m_artist;
        MediaInfoStr m_album;
        MediaInfoStr m_path;
        MediaInfoStr m_fileName;
        DWORD m_playItemIndex;
        PlayerConfig m_playerConfig;
        DWORD m_state; // 2,3 - paused, stopped;? 1 - run
        HANDLE m_hImgHandle;
        DWORD m_reserved;
    };
#pragma pack(pop)
    
    static const wchar_t* cUSBPlayerStatusMutexName = L"ShmMxMgrUsbAppMain";
    static const wchar_t* cUSBPlayerStatusMemName = L"ShmFmMgrUsbAppMain";

    enum IpcTarget
    {
        IpcTarget_MgrUSB = 0x05,
        IpcTarget_AppMain = 0x15
    };

    enum MgrUSBCommand
    {
        MgrUSB_PlayStatusResume = 0x64,
        MgrUSB_PlayStatusUpdate = 0x65
    };

    class CSharedMemory
    {
    public:
        CSharedMemory(const wchar_t* mutexName, const wchar_t* memName, bool readOnly, DWORD size);
        virtual ~CSharedMemory();

        void read(void* pBuf, DWORD readPos, DWORD size);
        void write(const void* pBuf, DWORD writePos, DWORD size);

    private:
        Utils::CSharedLock m_lock;
        hShmMem m_hMem;
    };
}