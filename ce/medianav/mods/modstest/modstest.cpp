// modstest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "modstest.h"
//#include "player_helper.h"
#include "SystemMeter.h"
#include <medianav.h>
#include <CmnDLL.h>
#include <fileref.h>
#include <tag.h>
#include <mp4tag.h>

#include <wingdi.h>
#include <windowsx.h>
#include <imaging.h>
#include <imgguids.h>
#include <utils.h>
#include <MicomAccessor.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndCommandBar;	// command bar handle

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HBITMAP createPictureBitmap(const void* data, size_t dataSize, int width, int height)
{
    static Utils::OleInitializer oleInitializer;

    HBITMAP hBitmap = NULL;

    IImagingFactory *pFactory = NULL;
    IImage *pImage = NULL;
    HDC memHdc = NULL;
    ImageInfo imageInfo;
    bool imageOk = false;
    do 
    {
        if(CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER,  IID_IImagingFactory , (void**)&pFactory) != S_OK)
            break;
        if(pFactory->CreateImageFromBuffer(data, dataSize, BufferDisposalFlagNone, &pImage) != S_OK)
            break;

        if(pImage->GetImageInfo(&imageInfo) != S_OK)
            break;

        memHdc = CreateCompatibleDC(NULL);

        BITMAPINFO bitmapInfo;
        memset(&bitmapInfo, 0 ,sizeof(BITMAPINFO));
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 24;
        bitmapInfo.bmiHeader.biWidth = width;
        bitmapInfo.bmiHeader.biHeight = height;

        void* pdibData;
        hBitmap = CreateDIBSection(memHdc, &bitmapInfo, DIB_RGB_COLORS, &pdibData, NULL, 0);

        if(!hBitmap)
            break;

        RECT rect;
        SetRect(&rect, 0, 0, width, height);
        HGDIOBJ oldObj = SelectObject(memHdc, hBitmap);
        if(pImage->Draw(memHdc, &rect, NULL) != S_OK)
            break;
        imageOk = true;
        SelectObject(memHdc,oldObj);
    } while (false);

    if(memHdc)
        DeleteDC(memHdc);

    if(pImage)
        pImage->Release();

    if(pFactory)
        pFactory->Release();

    if(!imageOk)
    {
        if(hBitmap)
            DeleteBitmap(hBitmap);
        return NULL;
    }

    return hBitmap;
}
static HBITMAP hBitmap = NULL;
void tagTest()
{
    // flac, mp4, asf, mp3
    TagLib::FileRef f("\\MD\\picture-test.m4a");
    TagLib::Tag* tag = f.tag();
    TagLib::MP4::Tag *mp4Tag = dynamic_cast<TagLib::MP4::Tag *>(tag);
    if(mp4Tag != NULL)
    {
        TagLib::MP4::ItemListMap itemsListMap = mp4Tag->itemListMap();
        TagLib::MP4::Item coverItem = itemsListMap["covr"];
        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
        if (!coverArtList.isEmpty()) 
        {
            TagLib::MP4::CoverArt coverArt = coverArtList.front();
            hBitmap = createPictureBitmap(coverArt.data().data(), coverArt.data().size(), 287, 287);
        }        //mp4Tag->
    }
    TagLib::String artist = f.tag()->artist(); // artist == "Frank Zappa"
    TagLib::String album = f.tag()->album(); // artist == "Frank Zappa"
    TagLib::String song = f.tag()->title(); // artist == "Frank Zappa"
    printf("Artist: %s\r\nAlbum: %s\r\nSong: %s\r\n", artist.toCString(), album.toCString(), song.toCString());
    /*f.tag()->setAlbum("Fillmore East");
    f.save();
    TagLib::FileRef g("Free City Rhymes.ogg");
    TagLib::String album = g.tag()->album(); // album == "NYC Ghosts & Flowers"
    g.tag()->setTrack(1);
    g.save();*/

}
void smallTest()
{
    IpcPostMsg(0x05, 0x04, 0x64, 0, NULL);
    return;
    DWORD dataSize = 0xA46;
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"ShmMxMgrUsbAppMain");
    if(!hMutex)
        return;
    if(WaitForSingleObject(hMutex, 1000) != WAIT_OBJECT_0)
    {
        CloseHandle(hMutex);
        return;
    }
    // We own mutex now.
    HANDLE hMap = CreateFileMapping(NULL, NULL, PAGE_READWRITE , 0, dataSize, L"ShmFmMgrUsbAppMain");
    if(!hMap)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return;
    }
    LPVOID pData = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if(!pData)
    {
        CloseHandle(hMap);
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return;
    }
    ReleaseMutex(hMutex);

//    debugDump(1, pData, 0xA46);
    HANDLE hFile = CreateFile(L"\\MD\\tagdump.bin", GENERIC_WRITE , FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile)
    {
        DWORD bytesWritten;
        WriteFile(hFile,pData, dataSize, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
    MediaNav::USBPlayerStatus *pMedia = reinterpret_cast<MediaNav::USBPlayerStatus *>(pData);
    memcpy(pMedia->m_song, L"TEST", 8);
    IpcPostMsg(0x05, 0x15, 0x64, 0, NULL);
    /*pMedia->m_durHour = 2;
    pMedia->m_curHour = 1;
    pMedia->m_curMin = 2;
    pMedia->m_curSec = 3;*/
    WaitForSingleObject(hMutex, INFINITE);
    UnmapViewOfFile(pData);
    CloseHandle(hMap);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

}

void busyTest()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    DWORD endTime = GetTickCount() + 1000;
    while(GetTickCount() < endTime)
        ;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
}

void ksa_test()
{
    Utils::RegistryAccessor::setString(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\KSA", L"Dll", L"\\MD\\debug\\KSA_driver\\KSA_driver.dll");
    Utils::RegistryAccessor::setString(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\KSA", L"Prefix", L"KSA");
    Utils::RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\KSA", L"Order", 1);
    Utils::RegistryAccessor::setInt(HKEY_LOCAL_MACHINE, L"\\Drivers\\Builtin\\KSA", L"Index", 1);
    
    HANDLE devHandle = ActivateDeviceEx(L"\\Drivers\\BuiltIn\\KSA",NULL,0,NULL);
    DWORD err = GetLastError();
    HANDLE hFile = CreateFile(L"KSA1:", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD size = (0xC09FBE00 - 0xC09B1000);
    void* pBuf = malloc(size);
    DWORD bytesRead;
    SetFilePointer(hFile, 0xC09B1000, NULL, FILE_BEGIN);
    ReadFile(hFile, pBuf, size, &bytesRead, NULL);
    CloseHandle(hFile);
    hFile = CreateFile(L"\\MD\\usbware.code", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, pBuf, size, &bytesRead, NULL);
    free(pBuf);
    CloseHandle(hFile);
    DeactivateDevice(devHandle);


}

void sndTest()
{
    Utils::RegistryAccessor::setBool(HKEY_LOCAL_MACHINE, L"LGE\\SystemInfo", L"MP3_SND_TEST", !Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, L"LGE\\SystemInfo", L"MP3_SND_TEST", false));
}

void micomTest()
{
    MicomAccessor micom;
    micom.Connect();
    for(int i = 0x22; i < 0x27; ++i)
    {
        micom.SendReadCmd(0x0D, i, 0x40);
        ResRead* res = micom.GetResRead();
        if(res->len > 0)
        {
            debugDump(1, res->data, res->len);
        }
    }
    micom.SendReadCmd(0x0D, 6, 0x02);
    ResRead* res = micom.GetResRead();
    if(res->len > 0)
    {
        debugDump(1, res->data, res->len);
    }
    micom.SendReadCmd(0x0D, 0x4D, 0x01);
    res = micom.GetResRead();
    if(res->len > 0)
    {
        debugDump(1, res->data, res->len);
    }
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    micomTest();
    //sndTest();
    //return 0;
    //ksa_test();
    //return 0;
    //test();
    //smallTest();
    //tagTest();
    //return 0;
    //busyTest();
    return 0;
    
    MSG msg;
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
        return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MODSTEST));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    if(hBitmap)
        DeleteBitmap(hBitmap);

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MODSTEST));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable


    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_MODSTEST, szWindowClass, MAX_LOADSTRING);


    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    if (g_hWndCommandBar)
    {
        CommandBar_Show(g_hWndCommandBar, TRUE);
    }

    return TRUE;
}

typedef void (*TDbgSetDebugLevel)(int module, int level);
typedef void (*TDbgDebugPrint)(int module, int level, const wchar_t *format, ...);
void doDebugOn()
{
    for(int i = 0; i< 0x2a; ++i)
        DbgSetDebugLevel(i, 3);
    DbgDebugPrint(0x17, 3, L"MODTEST %s\r\n", L"SUPER");
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDC_DEBUGON:
                    doDebugOn();
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
            CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU, 0);
            CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            CommandBar_Destroy(g_hWndCommandBar);
            PostQuitMessage(0);
            break;


        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }


            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;
        case WM_ERASEBKGND:
            if(hBitmap)
            {
                HDC hdc, memDC;
                hdc = (HDC)wParam;
                memDC = CreateCompatibleDC(hdc);
                SelectObject(memDC, hBitmap);
                {
                    // paint button bitmap
                    BitBlt(hdc, 0, 0, 287, 287, memDC, 0, 0, SRCCOPY);
                }
                DeleteDC(memDC);
                return TRUE;
            }
            break;

    }
    return (INT_PTR)FALSE;
}
