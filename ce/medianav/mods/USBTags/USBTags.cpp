// USBTags.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "USBTags.h"
#include <CmnDLL.h>
#include <medianav.h>
#include <smartptr.h>
#include <fileref.h>
#include <tag.h>

using namespace MediaNav;
// Global Variables:
HINSTANCE			g_hInst;			// current instance
Utils::SystemWideUniqueInstance g_uniqueInstance(IPC_WNDCLASS);
smart_ptr<CSharedMemory> pPlayerStatus;
static USBPlayerStatus g_cachedInfo;

// Forward declarations of functions included in this code module:
ATOM			USBTagsRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void processIpcMsg(const IpcMsg& ipcMsg);
bool getTags(USBPlayerStatus& info);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
#if 0
    // Test
    {
        USBPlayerStatus info;
        wcscpy(reinterpret_cast<wchar_t *>(&info.m_path), L"MD\\");
        wcscpy(reinterpret_cast<wchar_t *>(&info.m_fileName), L"pink.flac");
        getTags(info);
        getTags(info);
        return 0;
    }
#endif
    
    if(!g_uniqueInstance.isUnique())
        return -1;

    memset(&g_cachedInfo, 0, sizeof(g_cachedInfo));

    pPlayerStatus = new CSharedMemory(cUSBPlayerStatusMutexName, cUSBPlayerStatusMemName, false, sizeof(USBPlayerStatus));
    MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

ATOM USBTagsRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = 0;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    g_hInst = hInstance;

    if (!USBTagsRegisterClass(hInstance, IPC_WNDCLASS))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(IPC_WNDCLASS, L"USB Tags", WS_POPUP | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    IpcMsg ipcMsg;
    IpcGetMsg(&ipcMsg, message, wParam, lParam);
    if(ipcMsg.isValid())
    {
        processIpcMsg(ipcMsg);
    }

    switch (message) 
    {
        case WM_CREATE:
            debugPrintf(DBG, L"USBTags: Started\r\n");
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void processIpcMsg(const IpcMsg& ipcMsg)
{
    if(ipcMsg.src != IpcTarget_MgrUSB)
        return;
    switch(ipcMsg.cmd)
    {
        case MgrUSB_PlayStatusResume:
        case MgrUSB_PlayStatusUpdate:
            {
                USBPlayerStatus playerStatus;
                pPlayerStatus->read(&playerStatus, 0, sizeof(USBPlayerStatus));
                if(getTags(playerStatus))
                    pPlayerStatus->write(&playerStatus, 0, sizeof(USBPlayerStatus));
                break;
            }
         default:
             debugPrintf(DBG, L"USBTag: src=%d, cmd=%d, extraSize=%d, extra=%d\r\n", ipcMsg.src, ipcMsg.cmd, ipcMsg.extraSize, ipcMsg.extra);
            break;
    }

    if(ipcMsg.extraSize)
        IpcPostMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, ipcMsg.extraSize, &ipcMsg.extra);
    else
        IpcPostMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, 0, NULL);
}

void inplaceInfoString(wchar_t* dstString, const DWORD dstCharSize, const TagLib::String& value, const wchar_t* defaultValue)
{
    dstString[dstCharSize - 1] = L'\0';// Last is a null terminator always
    if(value.isEmpty())
        wcsncpy(dstString, defaultValue, dstCharSize - 1);
    else
        wcsncpy(dstString, value.toCWString(), dstCharSize - 1);
}

bool readFileInfo(const wchar_t* fileName, USBPlayerStatus& info)
{
    try
    {
        TagLib::FileRef f(fileName, false);
        if(f.isNull())
            return false;
        TagLib::String artist = f.tag()->artist();
        inplaceInfoString(reinterpret_cast<wchar_t *>(&info.m_artist), sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->artist(), L"No Artist");
        inplaceInfoString(reinterpret_cast<wchar_t *>(&info.m_album), sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->album(), L"No Album");
        inplaceInfoString(reinterpret_cast<wchar_t *>(&info.m_song), sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->title(), reinterpret_cast<const wchar_t *>(info.m_fileName));
        return true;
    }
    catch (...)
    {
        debugPrintf(DBG, L"USBTags: readFileInfo has thrown an exception!!!\r\n");
        return false;
    }
}

bool getTags(USBPlayerStatus& info)
{
    if(memcmp(&g_cachedInfo.m_path, &info.m_path, sizeof(MediaInfoStr) * 2) == 0)
    {
        // We have already read this file info
        memcpy(&info.m_song, &g_cachedInfo.m_song, sizeof(MediaInfoStr) * 3);
        return true;
    }

    // A new item. Read the data.
    TagLib::String filePath(info.m_path);
    filePath += info.m_fileName;

    if(!readFileInfo(filePath.toCWString(), info))
        return false;

    // Cache info
    g_cachedInfo = info;
    return true;
}
