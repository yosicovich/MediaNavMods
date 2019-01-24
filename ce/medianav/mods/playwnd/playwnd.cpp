//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//=========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//
//=========================================================================

#include <windows.h>
#include <mmsystem.h>
#include <streams.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dbgapi.h>

#include "playwnd.h"
#include <utils.h>


//
// Global data
//
HWND      ghApp=0, ghCB=0;
HINSTANCE ghInst;

TCHAR g_szFileName[MAX_PATH]={0};
PLAYSTATE g_psCurrent=psSTOPPED;
BOOL g_bAudioOnly=FALSE;
LONG g_lVolume=VOLUME_FULL;

// Collection of interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IVideoWindow  *pVW = NULL;
IBasicAudio *pBA=NULL;
IBasicVideo   *pBV = NULL;
IMediaSeeking *pMS = NULL;



HRESULT PlayMovieInWindow(LPCTSTR szFile)
{
    WCHAR wFile[MAX_PATH];
    HRESULT hr;

    RETAILMSG(1, (TEXT("Playing media %s...\r\n"), szFile));

    // Clear open dialog remnants before the slow-running RenderFile()
    UpdateWindow(ghApp);
    if(FAILED(StringCchCopy(wFile, MAX_PATH, szFile)))
        return E_FAIL;

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        // Have the graph construct its the appropriate graph automatically
        JIF(pGB->RenderFile(wFile, NULL));

        // QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));

        // Query for video interfaces, which may not be relevant for audio files
        JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        JIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        // Is this an audio-only file (no video component)?
        CheckVisibility();

        if (!g_bAudioOnly)
        {
            JIF(pVW->put_Owner((OAHWND)ghApp));
            JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));
        }

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

        if (g_bAudioOnly)
        {
            JIF(InitAudioWindow());
        }
        else
        {
            JIF(InitVideoWindow());
        }

        // Let's get ready to rumble!
        ShowWindow(ghApp, SW_SHOWNORMAL);
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);

        UpdateMainTitle();

        // Run the graph to play the media file
        JIF(pMC->Run());
        g_psCurrent=psRUNNING;

        SetFocus(ghApp);
    }

    return hr;
}


HRESULT InitVideoWindow(void)
{
    LONG lHeight, lWidth;
    HRESULT hr = S_OK;
    RECT rect;

    JIF(pBV->GetVideoSize(&lWidth, &lHeight));
//  RETAILMSG(1, (TEXT("  Video size: %d x %d\r\n"), lWidth, lHeight));
    //SetWindowPos(ghApp, NULL, 200, 200, 200/*lWidth*/, 200/*lHeight*/, SWP_NOOWNERZORDER);

    int nTitleHeight  = GetSystemMetrics(SM_CYCAPTION);
    int nBorderWidth  = GetSystemMetrics(SM_CXBORDER);
    int nBorderHeight = GetSystemMetrics(SM_CYBORDER);

    // Account for size of title bar and borders for exact match
    // of window client area to default video size
    /*SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2*nBorderWidth,
            lHeight + nTitleHeight + 2*nBorderHeight,
            SWP_NOMOVE | SWP_NOOWNERZORDER);*/

    //GetClientRect(ghApp, &rect);
    //JIF(pVW->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom));

    return hr;
}


HRESULT InitAudioWindow(void)
{
    SetWindowPos(ghApp, NULL, 0, 0,
                 DEFAULT_AUDIO_WIDTH,
                 DEFAULT_AUDIO_HEIGHT,
                 SWP_NOMOVE | SWP_NOOWNERZORDER);

    CommandBar_DrawMenuBar(ghCB, 0);

    // Do something different for audio-only files
    PaintAudioWindow();

    return S_OK;
}


void PaintAudioWindow(void)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    HBRUSH hBrush;

    hdc = BeginPaint(ghApp, &ps);

    GetClientRect(ghApp, &rect);

    // Show white if playing an audio file, gray if not
    if (pMC)
        hBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
    else
        hBrush = (HBRUSH) GetStockObject(LTGRAY_BRUSH);

    FillRect(hdc, &rect, hBrush);

    EndPaint(ghApp, &ps);
}


void CheckVisibility(void)
{
    long lVisible;
    HRESULT hr;

    g_bAudioOnly = FALSE;

    if (!pVW)
    {
        g_bAudioOnly = TRUE;
        RETAILMSG(1, (TEXT("No VideoWindow interface.  Assuming audio/MIDI file or unsupported video codec.\r\n")));
        return;
    }

    if (!pBV)
    {
        g_bAudioOnly = TRUE;
        RETAILMSG(1, (TEXT("No BasicVideo interface.  Assuming audio/MIDI file or unsupported video codec.\r\n")));
        return;
    }

    hr = pVW->get_Visible(&lVisible);
    if (FAILED(hr))
    {
        // If this is an audio-only clip, get_Visible() won't work.
        //
        // Also, if this video is encoded with an unsupported codec,
        // we won't see any video, although the audio will work if it is
        // of a supported format.
        //
        if (hr == E_NOINTERFACE)
        {
            g_bAudioOnly = TRUE;
            RETAILMSG(1, (TEXT("Video window not visible.  Assuming audio/MIDI file or unsupported video codec.\r\n")));
        }
        else
        {
            RETAILMSG(1, (TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr));
        }

        return;
    }
}


void PauseClip(void)
{
    if (!pMC)
        return;

    // Play/pause
    if((g_psCurrent == psPAUSED) || (g_psCurrent == psSTOPPED))
    {
        pMC->Run();
        g_psCurrent = psRUNNING;
    }
    else
    {
        pMC->Pause();
        g_psCurrent = psPAUSED;
    }

    UpdateMainTitle();
}


void StopClip(void)
{
    if ((!pMC) || (!pMS))
        return;

    // Stop and reset postion to beginning
    if((g_psCurrent == psPAUSED) || (g_psCurrent == psRUNNING))
    {
        LONGLONG pos = 0;
        pMC->Stop();
        g_psCurrent = psSTOPPED;

        pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
            NULL, AM_SEEKING_NoPositioning);

        // Display the first frame to indicate the reset condition
        pMC->Pause();
    }

    UpdateMainTitle();
}


void OpenClip()
{
    HRESULT hr;

    // If no filename specified by command line, show open dlg
    if(g_szFileName[0] == L'\0')
    {
        TCHAR szFilename[MAX_PATH];

        UpdateMainTitle();

        // If no filename was specified on the command line, then our video
        // window has not been created or made visible.  Make our main window
        // visible and bring to the front to allow file selection.
        InitAudioWindow();
        ShowWindow(ghApp, SW_SHOWNORMAL);
        SetForegroundWindow(ghApp);

        if (! GetClipFileName(szFilename))
        {
            DWORD dwDlgErr = CommDlgExtendedError();

            // Don't show output if user cancelled the selection (no dlg error)
            if (dwDlgErr)
            {
                RETAILMSG(1, (TEXT("GetClipFileName Failed! Error=0x%x\r\n"), GetLastError()));
                RETAILMSG(1, (TEXT("   Dlg Error=0x%x\r\n"), dwDlgErr));
            }
            return;
        }
        StringCchCopy(g_szFileName, MAX_PATH, szFilename);
    }

    hr = PlayMovieInWindow(g_szFileName);

    if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("PlayMovieInWindow failed! Error=0x%x\r\n"), hr));
        PostMessage(ghApp, WM_COMMAND, ID_FILE_EXIT, 0);
    }
}


BOOL GetClipFileName(LPTSTR szName)
{
    OPENFILENAME ofn={0};

    *szName = 0;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = ghApp;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrFilter       = FILE_FILTER_TEXT;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrInitialDir   = DEFAULT_MEDIA_PATH;
    ofn.lpstrTitle        = TEXT("Open Media File...\0");
    ofn.lpstrFileTitle    = NULL;
    ofn.lpstrDefExt       = TEXT("*\0");
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;

    return GetOpenFileName((LPOPENFILENAME)&ofn);
}


void CloseClip()
{
    if(pMC)
        pMC->Stop();

    g_psCurrent = psSTOPPED;

    CloseInterfaces();
}


void CloseInterfaces(void)
{
    // Relinquish ownership (IMPORTANT!) after hiding
    if(pVW)
    {
        pVW->put_Visible(OAFALSE);
        pVW->put_Owner(NULL);
    }

    HELPER_RELEASE(pMC);
    HELPER_RELEASE(pME);
    HELPER_RELEASE(pMS);
    HELPER_RELEASE(pBV);
    HELPER_RELEASE(pBA);
    HELPER_RELEASE(pVW);
    HELPER_RELEASE(pGB);

    // Clear file name to allow selection of new file with open dialog
    g_szFileName[0] = L'\0';

    // Redraw menu bar since we've probably resized the main window
    CommandBar_DrawMenuBar(ghCB, 0);

    // No current media state
    g_psCurrent = psINIT;

    RECT rect;
    GetClientRect(ghApp, &rect);
    InvalidateRect(ghApp, &rect, TRUE);
}


HRESULT ToggleMute(void)
{
    HRESULT hr=S_OK;

    if ((!pGB) || (!pBA))
        return S_OK;

    // Read current volume
    hr = pBA->get_Volume(&g_lVolume);
    if (hr == E_NOTIMPL)
    {
        // Fail quietly if this is a video-only media file
        return hr;
    }
    else if (FAILED(hr))
    {
        RETAILMSG(1, (TEXT("Failed in pBA->get_Volume!  hr=0x%x\r\n"), hr));
        return hr;
    }

    if (g_lVolume == VOLUME_FULL)
        g_lVolume = VOLUME_SILENCE;
    else
        g_lVolume = VOLUME_FULL;

    // Set new volume
    JIF(pBA->put_Volume(g_lVolume));

    UpdateMainTitle();
    return hr;
}


void UpdateMainTitle(void)
{
    TCHAR szTitle[MAX_PATH], szFile[MAX_PATH];

    // If no file is loaded, just show the application title
    if (g_szFileName[0] == L'\0')
    {
        StringCchPrintf(szTitle, MAX_PATH, TEXT("%s"), APPLICATIONNAME);
    }

    // Otherwise, show useful information, including filename and play state
    else
    {
        // Get file name without full path
        GetFilename(g_szFileName, MAX_PATH, szFile, MAX_PATH);

        // Update the window title to show muted/normal status
        StringCchPrintf(szTitle, MAX_PATH, TEXT("%s %s%s"), szFile,
                (g_lVolume == VOLUME_SILENCE) ? TEXT("(Muted)") : TEXT(""),
                (g_psCurrent == psPAUSED) ? TEXT("(Paused)") : TEXT(""));
    }

    SetWindowText(ghApp, szTitle);
}

void GetFilename(LPCTSTR pszFull, int nFullBuffSize, LPTSTR pszFile, int nFileBuffSize)
{
    int nLength;
    TCHAR szPath[MAX_PATH]={0};
    BOOL bSetFilename=FALSE;

    StringCchCopy(szPath, MAX_PATH, pszFull);
    nLength = _tcslen(szPath);

    for (int i=nLength-1; i>=0; i--)
    {
        if ((szPath[i] == '\\') || (szPath[i] == '/'))
        {
            szPath[i] = '\0';
            StringCchCopy(pszFile, nFileBuffSize, &szPath[i+1]);
            bSetFilename = TRUE;
            break;
        }
    }

    // If there was no path given (just a file name), then
    // just copy the full path to the target path.
    if (!bSetFilename)
        StringCchCopy(pszFile, nFileBuffSize, pszFull);
}

HRESULT ToggleFullScreen(void)
{
    static const LPCTSTR films[]= {
        L"\\MD\\video\\Part 1_01 - Lullaby Time Soothing Sounds for Baby ENG_FR.avi",
        L"\\MD\\video\\Rio.2.2014_HDRip_r5__[scarabey.org]+.avi",
        L"\\MD\\v2\\R.I.O. - Shine On (Official Video HD) (download-lagu-mp3.com).mp4"
    };
    static int roundIndex = 0;
    static int matchCount = 0;
#if 0
    if(matchCount++ % 10 == 0)
    {
        RETAILMSG(1, (TEXT("CloseClip()\r\n")));
        CloseClip();
        RETAILMSG(1, (TEXT("PlayMovieInWindow()\r\n")));
        PlayMovieInWindow(films[roundIndex++]);
        if(roundIndex == 4)
            roundIndex = 0;
        RETAILMSG(1, (TEXT("PlayMovieInWindow() - OK\r\n")));
        return S_OK;
    }
#endif
    if(g_psCurrent != psRUNNING)
        return S_OK;
    LONGLONG minPos=0, maxPos=0, pos=0;
    RETAILMSG(1, (TEXT("GetAvailable()\r\n")));
    pMS->GetAvailable(&minPos, &maxPos);
    int a=maxPos / RAND_MAX;
    pos = minPos + (LONGLONG)rand() * a;

    // Reset to first frame of movie
    RETAILMSG(1, (TEXT("Pause()\r\n")));
    pMC->Stop();
    RETAILMSG(1, (TEXT("SetPositions(%I64d,...); maxPos=%I64d\r\n"), minPos, maxPos));
    pMS->SetPositions(&minPos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame,
        &maxPos, AM_SEEKING_AbsolutePositioning);
    RETAILMSG(1, (TEXT("SetPositions(%I64d,...); maxPos=%I64d\r\n"), pos, maxPos));
    pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame,
        &maxPos, AM_SEEKING_AbsolutePositioning);
    RETAILMSG(1, (TEXT("Run()\r\n")));
    pMC->Run();
    RETAILMSG(1, (TEXT("Run() - OK\r\n")));

    return S_OK;
    

    HRESULT hr=S_OK;
    LONG lMode;
    static HWND hDrain=0;
    BOOL bPausedForTransition=FALSE;

    // Don't bother with full-screen for audio-only files
    if (g_bAudioOnly)
        return S_OK;

    if (!pVW)
        return S_OK;

    // If we're currently playing a media file, pause temporarily
    // to prevent audio/video breakup during/after the transition.
    if (g_psCurrent == psRUNNING)
    {
        PauseClip();
        bPausedForTransition = TRUE;
    }

    // Read current state
    JIF(pVW->get_FullScreenMode(&lMode));

    if (lMode == OAFALSE)
    {
        // Save current message drain
        LIF(pVW->get_MessageDrain((OAHWND *) &hDrain));

        // Set message drain to application main window
        LIF(pVW->put_MessageDrain((OAHWND) ghApp));

        // Switch to full-screen mode
        lMode = OATRUE;
        JIF(pVW->put_FullScreenMode(lMode));
    }
    else
    {
        // Switch back to windowed mode
        lMode = OAFALSE;
        JIF(pVW->put_FullScreenMode(lMode));

        // Undo change of message drain
        LIF(pVW->put_MessageDrain((OAHWND) hDrain));

        // Reset video window
        InitVideoWindow();
        LIF(pVW->SetWindowForeground(-1));

        // Reclaim keyboard focus for player application
        UpdateWindow(ghApp);
        SetForegroundWindow(ghApp);
        SetFocus(ghApp);
    }

    // Restore playback state if we previously paused
    if (bPausedForTransition)
    {
        PauseClip();
    }

    return hr;
}


HRESULT HandleGraphEvent(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    while(SUCCEEDED(pME->GetEvent(&evCode, &evParam1, &evParam2, 0)))
    {
        // Spin through the events
        hr = pME->FreeEventParams(evCode, evParam1, evParam2);

        NKDbgPrintfW(L"evCode=%X, evParam1=%X, evParam2=%X\r\n", evCode, evParam1, evParam2);
        if(EC_COMPLETE == evCode)
        {
            LONGLONG pos=0;

            // Reset to first frame of movie
            hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                   NULL, AM_SEEKING_NoPositioning);
            if (FAILED(hr))
            {
                // Some filters (like MIDI) may not implement interfaces
                // for IMediaSeeking to allow seek to the start.  In that
                // case, just stop and restart for the same effect.
                if (FAILED(hr = pMC->Stop()))
                {
                    RETAILMSG(1, (TEXT("\r\n*** Failed(%08lx) to stop media clip!\r\n"), hr));
                    break;
                }

                if (FAILED(hr = pMC->Run()))
                {
                    RETAILMSG(1, (TEXT("\r\n*** Failed(%08lx) to reset media clip!\r\n"), hr));
                    break;
                }
            }
        }
    }

    return hr;
}


void MoveVideoWindow(void)
{
    RECT rect, client;

    GetClientRect(ghApp, &client);
    if(pVW)
    {
        pVW->SetWindowPosition(client.left, client.top, client.right, client.bottom);
    }

    GetWindowRect(ghApp, &rect);
}



LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch(message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 10000, 3000, NULL);
        break;
    case WM_DESTROY:
        KillTimer(hWnd, 10000);
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        switch (wParam)
        {
        case 10000:
            {
                ToggleFullScreen();
                break;
            }
        }
        break;

        case WM_PAINT:
            if ((hWnd == ghApp) && (g_bAudioOnly))
                PaintAudioWindow();
            else
                return DefWindowProc(hWnd, message, wParam, lParam);
            break;

        case WM_SIZE:
        case WM_MOVE:
            MoveVideoWindow();
            break;

        case WM_KEYDOWN:

            switch(wParam)
            {
                case 'P':
                    PauseClip();
                    break;

                case 'S':
                    StopClip();
                    break;

                case 'M':
                    ToggleMute();
                    break;

                case 'F':
                    ToggleFullScreen();
                    break;

                case VK_ESCAPE:
                case VK_F12:
                case 'Q':
                case 'X':
                    CloseClip();
                    break;
            }
            break;

        case WM_COMMAND:

            switch(wParam)
            { // Menus

                case ID_FILE_OPENCLIP:
                    // If we have ANY file open, close it and shut down DShow
                    if (g_psCurrent != psINIT)
                        CloseClip();

                    // Open the new clip
                    OpenClip();
                    break;

                case ID_FILE_EXIT:
                    KillTimer(hWnd, 3000);
                    CloseClip();
                    PostQuitMessage(0);
                    break;

                case ID_FILE_PAUSE:
                    PauseClip();
                    break;

                case ID_FILE_STOP:
                    StopClip();
                    break;

                case ID_FILE_CLOSE:
                    CloseClip();
                    break;

                case ID_FILE_MUTE:
                    ToggleMute();
                    break;

                case ID_FILE_FULLSCREEN:
                    ToggleFullScreen();
                    break;

            } // Menus
            break;


        case WM_GRAPHNOTIFY:
            HandleGraphEvent();
            break;

        case WM_CLOSE:
            PostMessage(ghApp, WM_COMMAND, ID_FILE_EXIT, 0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);

    } // Window msgs handling

    return FALSE;

}


int PASCAL WinMain(HINSTANCE hInstC, HINSTANCE hInstP, LPTSTR lpCmdLine, int nCmdShow)
{
#ifdef DEBUG
//    Utils::RegistryAccessor::setString(HKEY_CLASSES_ROOT, L"\\CLSID\\{D1E456E1-47E5-497a-ABA1-A0C57C3CE5C0}\\InprocServer32", L"", L"\\Program Files\\avidmx\\avidmx.dll");
#endif
    MSG msg = { 0 };
    WNDCLASS wc;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        RETAILMSG(1, (TEXT("CoInitialize Failed!\r\n")));
        exit(1);
    }

    // Was a filename specified on the command line?
    if(lpCmdLine[0] != L'\0')
        StringCchCopy(g_szFileName, MAX_PATH, lpCmdLine);

    // Set initial media state
    g_psCurrent = psINIT;

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc = WndMainProc;
    ghInst = wc.hInstance = hInstC;
    wc.lpszClassName = CLASSNAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor =      LoadCursor(NULL, IDC_ARROW);
    wc.hIcon =        LoadIcon(hInstC, MAKEINTRESOURCE(IDI_INWINDOW));
    if(!RegisterClass(&wc))
    {
        RETAILMSG(1, (TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError()));
        CoUninitialize();
        exit(1);
    }

    ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
                    WS_OVERLAPPED | WS_CAPTION,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    0, 0, ghInst, 0);

    if(ghApp)
    {
        // Add menu bar
        ghCB = CommandBar_Create(hInstC, ghApp, 1);
        if(ghCB)
        {
            CommandBar_InsertMenubar(ghCB, hInstC, IDR_MENU, 0);
            CommandBar_AddAdornments(ghCB, 0, 0);
            CommandBar_DrawMenuBar(ghCB, 0);
        }

        // Open the specified media file or prompt for a title
        PostMessage(ghApp, WM_COMMAND, ID_FILE_OPENCLIP, 0);

        // Main message loop
        while(GetMessage(&msg,NULL,0,0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    else
    {
        RETAILMSG(1, (TEXT("CreateWindow Failed! Error=0x%x\r\n"), GetLastError()));
    }

    // Finished with COM
    CoUninitialize();

    return msg.wParam;
}
