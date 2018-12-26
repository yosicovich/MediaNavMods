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
//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//
//--------------------------------------------------------------------------;
//
//   Header file for Video in window movie player sample
//

//
// Function prototypes
//
HRESULT InitAudioWindow(void);
HRESULT InitVideoWindow(void);
HRESULT HandleGraphEvent(void);

BOOL GetClipFileName(LPTSTR szName);

void PaintAudioWindow(void);
void MoveVideoWindow(void);
void CheckVisibility(void);
void CloseInterfaces(void);

void OpenClip(void);
void PauseClip(void);
void StopClip(void);
void CloseClip(void);

void UpdateMainTitle(void);
void GetFilename(__in_ecount(nFullBuffSize) LPCTSTR pszFull, int nFullBuffSize, __out_ecount(nFileBuffSize) LPTSTR pszFile, int nFileBuffSize);

//
// Constants
//
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

enum PLAYSTATE { psRUNNING, psPAUSED, psSTOPPED, psINIT };

// File filter for OpenFile dialog
#define FILE_FILTER_TEXT \
    TEXT("All Files (*.*)\0*.*;\0")\
    TEXT("Video Files (*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v)\0*.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v\0")\
    TEXT("Audio files (*.wav; *.mpa; *.mp2; *.au; *.aif; *.aiff; *.snd)\0*.wav; *.mpa; *.mp2; *.au; *.aif; *.aiff; *.snd\0")\
    TEXT("WMT Files (*.asf; *.asx; *.wma)\0*.asf; *.wma; *.asx;\0")\
    TEXT("MIDI Files (*.mid, *.midi, *.rmi)\0*.mid; *.midi; *.rmi\0\0")

// Begin default media search at root directory
#define DEFAULT_MEDIA_PATH  TEXT("\\\0")


// Used with audio-only files
#define DEFAULT_AUDIO_WIDTH     200
#define DEFAULT_AUDIO_HEIGHT    100

#define APPLICATIONNAME TEXT("Movie Player")
#define CLASSNAME       TEXT("PlayWndMoviePlayer")

#define WM_GRAPHNOTIFY  WM_USER+13

//
// Macros
//
#define HELPER_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {RETAILMSG(1, (TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr)); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {RETAILMSG(1, (TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr));}


//
// Resource constants
//
#define IDI_INWINDOW                    100
#define IDR_MENU                        101
#define ID_FILE_OPENCLIP                40001
#define ID_FILE_EXIT                    40002
#define ID_FILE_PAUSE                   40003
#define ID_FILE_STOP                    40004
#define ID_FILE_CLOSE                   40005
#define ID_FILE_MUTE                    40006
#define ID_FILE_FULLSCREEN              40007


