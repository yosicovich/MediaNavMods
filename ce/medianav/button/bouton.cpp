// bouton.cpp : Defines the entry point for the application.

#include <stdio.h>
#include "stdafx.h"
#include "bouton.h"
#include "simpleini.h"
#include <iostream> 
#include <fstream>
#include <sstream>
#include <Winuser.h>
#include "Wingdi.h"
#include <time.h> 
#include <wingdi.h>
#include <windowsx.h>
#include <set>
#include <imaging.h>
#include <imgguids.h>

#define TIMER_MIDNIGHT 1000
#define TIMER_POLL 1001
#define DEFAULT_POLL_INTERVAL_MS 200

//Parameters
int cTextColorR, cTextColorG, cTextColorB;
enum DateFormat 
{
    DateFormat_DMY = 0,
    DateFormat_YMD = 1,
    DateFormat_MDY = 2
};
DateFormat dateFormat = DateFormat_DMY;

// Global Variables:
HINSTANCE			g_hInst;			// current instance

#define TRANSPERENT_COLOR 0xFFFF
HBITMAP hBitmapBtn = NULL;
RECT rBitmapLBtn;
BOOL pressedLBtn = FALSE;
BOOL dragged = FALSE;
CSimpleIni ini = NULL;
DWORD dwOldTime = 0;
DWORD dwTimeElapsed = 0;
std::wstring iniPath;
std::wstring exePath;
TCHAR FirstName[MAX_PATH];

std::wstring gButtonText;
HFONT gTextFont = NULL;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK TimerProc(  HWND     , UINT     ,  UINT_PTR ,  DWORD    );
void printDate(HWND hWnd);
int readConfig(HWND hWnd);
int RunApplication(HINSTANCE hInstance);

HWND gWnd;
HWND pWnd = GetForegroundWindow();
TCHAR* ponlyonapp = NULL;
bool bShowWindowClass = false;
bool bShowWindowID = false;

static const wchar_t cFilterDelimiter = L'|';
static const wchar_t cNotMatchPrefix = L'!';
static const std::wstring cAnyMatchPattern = L"@";

template<typename T> class TFilter
{
public:
    bool check(const T& checkItem)
    {
        // Positive match
        if(!m_matchSet.empty() && m_matchSet.find(checkItem) == m_matchSet.end())
            return false;
        // Negative match
        return m_notMatchSet.empty() || m_notMatchSet.find(checkItem) == m_notMatchSet.end();
    }

    template<typename f> void populate(const std::wstring& src, f valueFunc, const wchar_t delimiter = cFilterDelimiter, const wchar_t notMatchPrefix = cNotMatchPrefix, const std::wstring& anyMatchPattern = cAnyMatchPattern)
    {
        std::wistringstream strStream(src);
        std::wstring str;    
        while (getline(strStream, str, delimiter))
        {
            if(str == anyMatchPattern)
            {
                clear();
                return;
            }

            if(str.empty())
                continue;

            bool notMatchFlag = false;
            if(str.at(0) == notMatchPrefix)
            {
                notMatchFlag = true;
                str = str.substr(1);
            }

            if(str.empty())
                continue;
            T val;
            if(!valueFunc(str, val))
                continue;

            addItem(val, notMatchFlag);
        }
    }

    void addItem(const T& item, bool notMatch = false)
    {
        if(!notMatch)
        {
            m_matchSet.insert(item);
        }else
        {
            m_notMatchSet.insert(item);
        }
    }

    void clear()
    {
        m_matchSet.clear();
        m_notMatchSet.clear();
    }
private:
    std::set<T> m_matchSet;
    std::set<T> m_notMatchSet;
};

typedef TFilter<std::wstring> TClassFilter;
TClassFilter gClassFilter;

typedef TFilter<unsigned short> TWindowIDFilter;
TWindowIDFilter gWindowIDFilter;

unsigned int gStatePollInterval = DEFAULT_POLL_INTERVAL_MS;

int bmWidth = 0;
int bmHeight = 0;
bool bmHasAlpha = false;


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{

	TCHAR pExe[MAX_PATH];
    WCHAR modulePath[MAX_PATH];
    memset(pExe, 0, MAX_PATH * sizeof(WCHAR));

    if ((GetModuleFileName(NULL, modulePath, MAX_PATH)) > 0) {
			HWND hExe = FindWindowFromPath(modulePath);
			if (hExe != NULL) {
				SetForegroundWindow(hExe);
				return FALSE;

			}
    }
    
    int returnValue = RunApplication(hInstance);

    if(gTextFont != NULL)
        DeleteObject(gTextFont);

    if(hBitmapBtn)
        DeleteBitmap(hBitmapBtn);

    return returnValue;
}

int RunApplication(HINSTANCE hInstance)
{
    MSG msg;
    // Perform application initialization:
    if (!InitInstance(hInstance, 8)) 
    {
        return FALSE;
    }

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0) ) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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
	wc.hIcon         = 0;
	wc.hCursor       = 0;
	wc.hbrBackground = 0;//(HBRUSH)(COLOR_BACKGROUND+1);
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

    g_hInst = hInstance; // Store instance handle in our global variable

    if (!MyRegisterClass(hInstance, _T("bouton")))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(_T("bouton"), _T("The Bouton"), WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    if(!readConfig(hWnd))
        return FALSE;

    int rX = ini.GetLongValue(_T("Position"), _T("IconXPosition"), 400);
    int rY = ini.GetLongValue(_T("Position"), _T("IconYPosition"), 200);

    if (rX > (800 - bmWidth)) rX = 800 - bmWidth;
    if (rX < 0) rX = 0;
    if (rY > (480 - bmHeight)) rY = 480 - bmHeight;
    if (rY < 0) rY = 0;

    SetRect(&rBitmapLBtn, 0, 0, bmWidth, bmHeight);

    SetWindowPos(hWnd, HWND_TOPMOST, 
	rX, rY, bmWidth, bmHeight, 0);


	//SetForegroundWindow((HWND)(((ULONG) hWnd) | 0x01));
    ShowWindow(hWnd, SW_HIDE);
	
	gWnd = hWnd;
    //UpdateWindow(hWnd);
    return TRUE;
}

bool ClassFilterValueFunc(const std::wstring& str, std::wstring& valueStr)
{
    valueStr = str;
    return true;
}

bool WindowIDFilterValueFunc(const std::wstring& str, unsigned short& val)
{
    unsigned int windowID;
    if(swscanf_s(str.c_str(), L"%X", &windowID) == 0)
        return false;
    val = static_cast<unsigned short>(windowID);
    return true;
}

int readConfig(HWND hWnd)
{
    // get ini path
    std::wstring path;
    if (!GetCurrExePath(path))
        return FALSE;

    iniPath = path + L"\\TheBouton.ini";
    std::wstring defBmpPath = path + L"\\bouton.bmp";

    // load ini file
    ini.SetUnicode();
    ini.LoadFile(iniPath.c_str());

    // Poll Interval
    gStatePollInterval = ini.GetLongValue(L"Visibility", L"StatePollIntervalMS", DEFAULT_POLL_INTERVAL_MS);

    // Class filter    
    {
        std::wstring classFilter = ini.GetValue(L"Visibility", L"ClassFilter", cAnyMatchPattern.c_str());
        gClassFilter.populate(classFilter, ClassFilterValueFunc);
    }

    // WindowProc filter    
    {
        std::wstring windowIDFilter = ini.GetValue(L"Visibility", L"WindowIDFilter", cAnyMatchPattern.c_str());
        gWindowIDFilter.populate(windowIDFilter, WindowIDFilterValueFunc);
    }

    // Font color
    cTextColorR = ini.GetLongValue(L"Text", L"TextColorR", 255) & 0xFF;
    cTextColorG = ini.GetLongValue(L"Text", L"TextColorG", 255) & 0xFF;
    cTextColorB = ini.GetLongValue(L"Text", L"TextColorB", 255) & 0xFF;

    int fontHeight = ini.GetLongValue(L"Text", L"FontSize", 14);
    const wchar_t *sFontName = ini.GetValue(L"Text", L"FontName", L"Tahoma");

    LOGFONT logFont = {fontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE};
    memset(logFont.lfFaceName , 0 ,sizeof(logFont.lfFaceName));
    wcscpy_s(logFont.lfFaceName, LF_FACESIZE, sFontName);
    gTextFont = CreateFontIndirect(&logFont);

    // Operation mode
    do 
    {
        bShowWindowID = ini.GetBoolValue(L"Text", L"ShowWindowID", false);
        if(bShowWindowID)
            break;

        bShowWindowClass = ini.GetBoolValue(L"Text", L"ShowWindowClass", false);
        if(bShowWindowClass)
            break;

        if(ini.GetBoolValue(L"Text", L"ShowDate", false))
        {
            std::wstring dateFmt = ini.GetValue(L"Text", L"DateFormat", L"DMY");
            if(dateFmt == L"YMD")
            {
                dateFormat = DateFormat_YMD;
            }else if(dateFmt == L"YMD")
            {
                dateFormat = DateFormat_MDY;
            }else
            {
                dateFormat = DateFormat_DMY;
            }
            printDate(hWnd);
            break;
        }

        gButtonText = ini.GetValue(L"Text", L"ButtonText", L"");
    } while (false);

    // Action command
    exePath = ini.GetValue(L"Raccourci", L"exe", L"");

    // Picture
    hBitmapBtn = LoadPicture(ini.GetValue(_T("Raccourci"), _T("bmp"), defBmpPath.c_str()), bmHasAlpha, bmWidth, bmHeight);
    if(!hBitmapBtn)
        return FALSE;

    return TRUE;
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

void checkShowState()
{
	HWND nWnd = GetForegroundWindow();
	if (gWnd != nWnd && pWnd != nWnd) {
		pWnd = nWnd;
	}

    TCHAR name[MAX_PATH];
    if (GetClassName(pWnd, name, MAX_PATH) <= 0) {
        _snwprintf(name,MAX_PATH,_T("DesktopRoot"));
    }
    std::wstring windowClassName(name, wcslen(name));
    unsigned short windowID = static_cast<unsigned short>(GetWindowLong(pWnd, GWL_WNDPROC));

    if(bShowWindowClass || bShowWindowID)
    {
        std::wstring matchStr;
        if(bShowWindowID)
        {
            _snwprintf(name ,MAX_PATH, _T("%04hX"), windowID);
            matchStr.assign(name, wcslen(name));
        }else if(bShowWindowClass)
        {
            matchStr = windowClassName;
        }

        if(!matchStr.empty() && gButtonText != matchStr)
        {
            gButtonText = matchStr;
            InvalidateRect(gWnd, NULL, TRUE);
        }
    }

	bool fenetreok = gClassFilter.check(windowClassName) && gWindowIDFilter.check(windowID);

	if (fenetreok){

        ShowWindow(gWnd, SW_SHOWNA);
		BringWindowToTop(gWnd);
	} else {
		ShowWindow(gWnd, SW_HIDE);
	}
}
static inline bool drawPressedState()
{
    return pressedLBtn && !exePath.empty();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
    {
    // Clicks processing
    case WM_LBUTTONDOWN: // est envoyé lorsque le bouton gauche de la souris est pressé.
        dwOldTime = GetTickCount();
        POINT curs;
        curs.x = GET_X_LPARAM(lParam);
        curs.y = GET_Y_LPARAM(lParam);

        if (PtInRect(&rBitmapLBtn, curs)) {
            dragged = FALSE;
            pressedLBtn = TRUE;
            InvalidateRect(hWnd, &rBitmapLBtn, TRUE);
            SetCapture(hWnd);
        }
        break;

    case WM_LBUTTONUP:  // est envoyé lorsque le bouton gauche de la souris est relevé.

        dwTimeElapsed = (GetTickCount() - dwOldTime);
        dwOldTime = 0;
        if (pressedLBtn) {
            ReleaseCapture();
            pressedLBtn = FALSE;
            InvalidateRect(hWnd, &rBitmapLBtn, TRUE);
            RECT mainWindowRect;
            GetWindowRect(hWnd, &mainWindowRect);
            if (dragged) {
                dwOldTime = 0;
                dwTimeElapsed = 0;
                int SaveAfterMove = ini.GetLongValue(_T("Position"), _T("SaveAfterMove"), 1);
                ini.SetLongValue(_T("Position"), _T("IconXPosition"), mainWindowRect.left);
                ini.SetLongValue(_T("Position"), _T("IconYPosition"), mainWindowRect.top);
                if (SaveAfterMove == 1 ) {
                    ini.SaveFile(iniPath.c_str());
                }
            } else {
                POINT curs;
                curs.x = GET_X_LPARAM(lParam);
                curs.y = GET_Y_LPARAM(lParam);
                if (PtInRect(&rBitmapLBtn, curs)&&(dwTimeElapsed < 2000)) {
                    if (!IsProcessExist(exePath)) {
                        ShellCommand(exePath.c_str(), NULL);
                    } else {
                        HWND hExe = FindWindowFromPath(exePath.c_str());
                        if (hExe != NULL)
                            SetForegroundWindow(hExe);
                        SetFocus(hExe);
                    }
                    BringWindowToTop(hWnd);
                }
                if (dwTimeElapsed >= 2000)
                    PostQuitMessage(0);										
            }
        }
        break;

    // Dragging support
    case WM_MOUSEMOVE: // Les coordonnées de la souris sont passées en paramètres lors de l'envoi de ce message
        if (pressedLBtn) {
            int Locked = ini.GetLongValue(_T("Position"), _T("Locked"), 0);
            if (Locked == 0 ) {
                POINT pos;
                pos.x = GET_X_LPARAM(lParam);
                pos.y = GET_Y_LPARAM(lParam);

                if (!PtInRect(&rBitmapLBtn, pos) || dragged) {
                    dwOldTime = 0;
                    dwTimeElapsed = 0;
                    RECT mainWindowRect;
                    int windowWidth, windowHeight;

                    GetWindowRect(hWnd,&mainWindowRect);
                    windowHeight = mainWindowRect.bottom - mainWindowRect.top;
                    windowWidth = mainWindowRect.right - mainWindowRect.left;

                    BOOL right = (pos.x >= windowWidth)?TRUE:FALSE;
                    BOOL down = (pos.y >= windowHeight)?TRUE:FALSE;

                    ClientToScreen(hWnd, &pos);
                    ////////////////////////////////////////////////////////////////
                    MoveWindow(hWnd, right?pos.x-windowWidth:pos.x,
                        down?pos.y-windowHeight:pos.y, windowWidth, windowHeight, TRUE);
                    ///////////////////////////////// doigt au centre du bouton /////////////////////////////////////////////////////////////////////////////////
                    dragged = TRUE;
                }
            }
        }
        break;
      // Painting
    case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;
            hdc = BeginPaint(hWnd, &ps);
            if(!gButtonText.empty())
            {
                if(gTextFont != NULL)
                    SelectObject(hdc, gTextFont);
                SetTextColor(hdc, drawPressedState() ? RGB(~cTextColorR, ~cTextColorG, ~cTextColorB): RGB(cTextColorR, cTextColorG, cTextColorB));
                RECT ClientRect;                // Client window size
                SIZE Size;                      // Size of text output
                GetClientRect(hWnd,&ClientRect);

                SetBkMode(hdc, TRANSPARENT);
                GetTextExtentPoint32(hdc,gButtonText.c_str(), gButtonText.size(),&Size);
                INT XPos = ((ClientRect.right - ClientRect.left) - Size.cx) / 2;
                INT YPos = ((ClientRect.bottom - ClientRect.top) - Size.cy) /2 ;// * 4 / 5;

                // Check the window is big enough to have sample times displayed

                if (((XPos > 0) && (YPos > 0)) || bShowWindowClass) {
                    ExtTextOut(hdc,XPos, YPos,
                        ETO_CLIPPED,// useless since no rect specified
                        NULL, gButtonText.c_str(), gButtonText.size(), NULL);
                }

            }
            EndPaint(hWnd, &ps);
            break;
        }

    case WM_ERASEBKGND: //est envoyé lorsqu'une partie de la zone client doit être redessinée.
        HDC hdc, memDC;
        hdc = (HDC)wParam;
        memDC = CreateCompatibleDC(hdc);
        SelectObject(memDC, hBitmapBtn);
#ifdef GWES_MGALPHABLEND
        if (bmHasAlpha) {
            BLENDFUNCTION bf;
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.AlphaFormat = AC_SRC_ALPHA;   // use source alpha
            bf.SourceConstantAlpha = 0xff;   // use constant alpha
            AlphaBlend(hdc, 0, 0, bmWidth, bmHeight, 
                memDC, drawPressedState() ? bmWidth : 0, 0, bmWidth, bmHeight, bf);
        } else {
#else
        {
#endif
            // paint button bitmap
            TransparentImage(hdc, 0, 0, bmWidth, bmHeight, memDC, 
                drawPressedState() ? bmWidth : 0, 0, bmWidth,
                bmHeight, TRANSPERENT_COLOR);
        }
        DeleteDC(memDC);
        return TRUE;
        break;

        // Init/de-init
        case WM_CREATE:
            SetTimer(hWnd, TIMER_POLL, gStatePollInterval, NULL);
        break;

        case WM_DESTROY: // indique que la fenêtre est détruite.
            KillTimer(hWnd, TIMER_MIDNIGHT);
            KillTimer(hWnd, TIMER_POLL);
            PostQuitMessage(0);
        break;
			
        // Timer
        case WM_TIMER:
            {
                switch(wParam)
                {
                case TIMER_MIDNIGHT:
                    {
                        KillTimer(hWnd, TIMER_MIDNIGHT);
                        printDate(hWnd);
                        InvalidateRect(hWnd, &rBitmapLBtn, TRUE);
                        break;
                    }
                case TIMER_POLL:
                    checkShowState();
                    break;
                }
                break;
            }

		default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void printDate(HWND hWnd)
{
    SYSTEMTIME lt;
    long msec;
    wchar_t date[11];

    //Get current date
    GetLocalTime(&lt);
    switch (dateFormat)
    {
    case DateFormat_YMD:
        swprintf_s(date, 11, L"%.4d-%.2d-%.2d", lt.wYear, lt.wMonth, lt.wDay);
        break;
    case DateFormat_MDY:
        //MDY
        swprintf_s(date, 11, L"%.2d/%.2d/%.4d", lt.wMonth, lt.wDay, lt.wYear);
        break;
    default:
        //DMY
        swprintf_s(date, 11, L"%.2d/%.2d/%.4d", lt.wDay, lt.wMonth, lt.wYear);
    }
    gButtonText = date;

    // start a timer to change the date at midnight
    msec = (lt.wHour * 3600000) + (lt.wMinute * 60000) + (lt.wSecond * 1000) + lt.wMilliseconds;
    msec = 86400000 - msec;
    SetTimer(hWnd, TIMER_MIDNIGHT, msec, NULL);
}
