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

    static const int MediaInfoStringLenght = 0x104;

    typedef wchar_t MediaInfoStr[MediaInfoStringLenght];
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
    
    enum PlayerState
    {
        ST_PLAY = 1,
        ST_PAUSE = 2,
        ST_STOP = 3
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
        HBITMAP m_hImgHandle;
        DWORD m_reserved;
    };
#pragma pack(pop)
    
    static const wchar_t* cUSBPlayerStatusMutexName = L"ShmMxMgrUsbAppMain";
    static const wchar_t* cUSBPlayerStatusMemName = L"ShmFmMgrUsbAppMain";

    enum IpcTarget
    {
        IpcTarget_MgrVid = 0x04,
        IpcTarget_MgrUSB = 0x05,
        IpcTarget_AppMain = 0x15
    };

    enum AppMainCommand
    {
        AppMain_IDM_AMAIN_MUSB_SELECT_CATEGORY = 112,
        AppMain_IDM_AMAIN_MUSB_MOVE_PLAY_FOLDER = 114,
        AppMain_IDM_AMAIN_MUSB_ERROR_NEXT_PLAY = 118,
        AppMain_IDM_AMAIN_MUSB_NEXT_TRACK = 101,
        AppMain_IDM_AMAIN_MUSB_PREV_TRACK = 100,
        AppMain_IDM_AMAIN_MUSB_CHANGE_SHUFFLE = 106,
        AppMain_IDM_AMAIN_MUSB_CHANGE_REPEAT = 107,
        AppMain_IDM_AMAIN_MUSB_CHANGE_PLAY_STATUS = 108,
        AppMain_IDM_AMAIN_MUSB_FAST_RWD_START = 102,
        AppMain_IDM_AMAIN_MUSB_FAST_RWD_END = 103,
        AppMain_IDM_AMAIN_MUSB_FAST_FWD_START = 104,
        AppMain_IDM_AMAIN_MUSB_FAST_FWD_END = 105,
        AppMain_SaveResumeData = 115,
        AppMain_IDM_AMAIN_MUSB_AUDIO_PATH_USB_STATUS = 122,
        AppMain_DeleteResumeData = 124
    };

    enum MgrUSBCommand
    {
        MgrUSB_PlayStatusResume = 0x64,
        MgrUSB_PlayStatusUpdate = 0x65,
        MgrUSB_PlayStatusStateChange = 0x66,
        MgrUSB_PlayStatusSetCoverImage = 0x67,
        MgrUSB_IDM_MUSB_INDEXING_COMPLETE = 0x6A
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