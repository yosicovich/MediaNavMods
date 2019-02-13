// bouton.cpp : Defines the entry point for the application.

#include <stdio.h>
#include "stdafx.h"
#include "bouton.h"
#include "simpleini.h"
#include <iostream> 
#include <fstream>
#include <sstream>
#include <vector>
#include <Winuser.h>
#include "Wingdi.h"
#include <time.h> 
#include <wingdi.h>
#include <windowsx.h>
#include <set>
#include <imaging.h>
#include <imgguids.h>
#include <memory>
#include "SystemMeter.h"

#define TIMER_POLL 1000
#define DEFAULT_POLL_INTERVAL_MS 200

#define YEAR_TOKEN L"{year}"
#define MONTH_TOKEN L"{month}"
#define DAY_TOKEN L"{day}"
#define WEEKDAY_TOKEN L"{wday}"
//Parameters
int cTextColorR, cTextColorG, cTextColorB;

struct ExeEntry
{
    std::wstring filePath;
    std::wstring arguments;
    ExeEntry(const std::wstring& filePath, const std::wstring& arguments)
        :filePath(filePath)
        ,arguments(arguments)
    {

    }
};
typedef std::vector<ExeEntry> ExeEntries;
enum CloseAction
{
    CloseAction_CLOSE = 0,
    CloseAction_KILL = 1
};

struct CloseEntry
{
    std::wstring filePath;
    CloseAction method;
    CloseEntry(const std::wstring& filePath, CloseAction method)
        :filePath(filePath)
        ,method(method)
    {

    }
};
typedef std::vector<CloseEntry> CloseEntries;

std::wstring dateFormat;
std::vector<std::wstring> dayMap;
std::vector<std::wstring> monthMap;


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
TCHAR FirstName[MAX_PATH];

std::wstring gButtonText;
HFONT gTextFont = NULL;
ExeEntries exeEntries;
ExeEntries longTapEntries;

CloseEntries tapCloseEntries;
CloseEntries longTapCloseEntries;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK TimerProc(  HWND     , UINT     ,  UINT_PTR ,  DWORD    );
bool printDate(HWND hWnd, bool forceRepaint = false);
void printCPULoad();
int readConfig(HWND hWnd);
void readTapConfig();

int RunApplication(HINSTANCE hInstance);

HWND gWnd;
HWND pWnd = GetForegroundWindow();
TCHAR* ponlyonapp = NULL;
bool bShowWindowClass = false;
bool bShowWindowID = false;
bool bShowDate = false;
bool bShowCPULoad = false;
int cpuLoadPollCount = 1;
int cpuLoadPollCur = 0;
bool bVisible = false;
POINT checkPoint = {0, 0};
bool alwaysOnTop = false;
std::auto_ptr<Utils::SystemMeter> pSystemMeter;

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

    hWnd = CreateWindowEx(WS_EX_NOANIMATION | WS_EX_TOPMOST, _T("bouton"), _T("The Bouton"),WS_POPUP | WS_CLIPCHILDREN,
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

    SetWindowPos(hWnd, 0, rX, rY, bmWidth, bmHeight, SWP_NOZORDER);


    ShowWindow(hWnd, SW_HIDE);
	
	gWnd = hWnd;
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

static void readExeConfig(ExeEntries& entries, const wchar_t* sSection)
{
    const CSimpleIniW::TKeyVal* exeIni = ini.GetSection(sSection);
    if(exeIni)
    {
        for(CSimpleIniW::TKeyVal::const_iterator it = exeIni->begin(); it != exeIni->end(); ++it)
        {
            entries.push_back(ExeEntry(it->first.pItem, it->second));
        }
    }
}

static void readCloseConfig(CloseEntries& entries, const wchar_t* sSection)
{
    const CSimpleIniW::TKeyVal* exeIni = ini.GetSection(sSection);
    if(exeIni)
    {
        for(CSimpleIniW::TKeyVal::const_iterator it = exeIni->begin(); it != exeIni->end(); ++it)
        {
            entries.push_back(CloseEntry(it->first.pItem, _wcsicmp(TEXT("kill"), it->second) == 0 ? CloseAction_KILL:  CloseAction_CLOSE));
        }
    }
}

static void readTapConfig()
{
    std::wstring exePath = ini.GetValue(L"Raccourci", L"exe", L"");
    if(!exePath.empty())
        exeEntries.push_back(ExeEntry(exePath, L""));

    readExeConfig(exeEntries, TEXT("StartOnTap"));
    readCloseConfig(tapCloseEntries, TEXT("CloseOnTap"));

    // LongTap
    readExeConfig(longTapEntries, TEXT("StartOnLongTap"));
    readCloseConfig(longTapCloseEntries, TEXT("CloseOnLongTap"));
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

    // Check point
    checkPoint.x = ini.GetLongValue(L"Visibility", L"WindowPointCheckX", 0);
    checkPoint.y = ini.GetLongValue(L"Visibility", L"WindowPointCheckY", 0);

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

        bShowCPULoad = ini.GetBoolValue(L"Text", L"ShowCPULoad", false);
        if(bShowCPULoad)
        {
            cpuLoadPollCount = 1000 / gStatePollInterval;
            break;
        }

        if(ini.GetBoolValue(L"Text", L"ShowDate", false))
        {
            dateFormat = ini.GetValue(L"Text", L"DateFormat", DAY_TOKEN L"/" MONTH_TOKEN L"/" YEAR_TOKEN);
            // Week Day
            dayMap = splitString(ini.GetValue(L"Text", L"DayMap", L""), L",");
            if(dayMap.size() == 1 && dayMap[0].empty())
                dayMap.clear();
            // Month
            monthMap = splitString(ini.GetValue(L"Text", L"MonthMap", L""), L",");
            if(monthMap.size() == 1 && monthMap[0].empty())
                monthMap.clear();

            printDate(hWnd, true);
            bShowDate = true;
            break;
        }

        gButtonText = ini.GetValue(L"Text", L"ButtonText", L"");
    } while (false);

    // Action commands
    readTapConfig();

    // Picture
    hBitmapBtn = LoadPicture(ini.GetValue(_T("Raccourci"), _T("bmp"), defBmpPath.c_str()), bmHasAlpha, bmWidth, bmHeight);
    if(!hBitmapBtn)
        return FALSE;

    alwaysOnTop = ini.GetBoolValue(L"Position", L"AlwaysOnTop", false);
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
    HWND nWnd;
    if(checkPoint.x == 0 && checkPoint.y == 0)
        nWnd = GetForegroundWindow();
    else
        nWnd = WindowFromPoint(checkPoint);

	if (gWnd != nWnd && pWnd != nWnd) {
		pWnd = nWnd;
	}

    TCHAR name[MAX_PATH];
    if (GetClassName(pWnd, name, MAX_PATH) <= 0) {
        _snwprintf(name,MAX_PATH,_T("DesktopRoot"));
    }
    std::wstring windowClassName(name, wcslen(name));
    unsigned short windowID = static_cast<unsigned short>(GetWindowLong(pWnd, GWL_WNDPROC));

    if(bShowWindowClass || bShowWindowID )
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
    }else if(bShowCPULoad)
    {
        if(++cpuLoadPollCur == cpuLoadPollCount)
        {
            cpuLoadPollCur = 0;
            printCPULoad();
            InvalidateRect(gWnd, NULL, TRUE);
        }
    }else if(bShowDate && printDate(gWnd))
    {
        InvalidateRect(gWnd, NULL, TRUE);
    }

	bool fenetreok = gClassFilter.check(windowClassName) && gWindowIDFilter.check(windowID);

	if(fenetreok != bVisible)
    {
        bVisible = fenetreok;
        if (fenetreok){

            ShowWindow(gWnd, SW_SHOWNOACTIVATE);
            if(alwaysOnTop)
                BringWindowToTop(gWnd);
        } else {
            ShowWindow(gWnd, SW_HIDE);
        }

    }
}
static inline bool drawPressedState()
{
    return pressedLBtn && (!exeEntries.empty() || !longTapEntries.empty() || !tapCloseEntries.empty() || !longTapCloseEntries.empty());
}

static void startOne(const std::wstring& path, const std::wstring& args)
{
    if (!IsProcessExist(path)) {
        ShellCommand(path.c_str(), args.c_str());
    } else {
        HWND hExe = FindWindowFromPath(path.c_str());
        if (hExe != NULL)
            SetForegroundWindow(hExe);
        SetFocus(hExe);
    }
}

static void closeOne(const CloseEntry& closeEntry)
{
    if(_wcsicmp(closeEntry.filePath.c_str(), TEXT("@self")) == 0)
    {
        PostQuitMessage(0);
        return;
    }

    if(closeEntry.method == CloseAction_KILL)
    {
        DWORD id = FindProcessId(closeEntry.filePath.c_str());
        if(id)
            KillProcess(id);

    }else
    {
        HWND hExe = FindWindowFromPath(closeEntry.filePath.c_str());
        if(hExe)
            PostMessage(hExe, WM_CLOSE, 0, 0);
    }

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
                if (PtInRect(&rBitmapLBtn, curs))
                {
                    if(dwTimeElapsed < 1500) 
                    {
                        for(CloseEntries::const_iterator it = tapCloseEntries.begin(); it != tapCloseEntries.end(); ++it)
                        {
                            closeOne(*it);
                        }

                        for(ExeEntries::const_iterator it = exeEntries.begin(); it != exeEntries.end(); ++it)
                        {
                            startOne(it->filePath, it->arguments);
                        }
                        BringWindowToTop(hWnd);
                    }else if(dwTimeElapsed < 3500)
                    {
                        for(CloseEntries::const_iterator it = longTapCloseEntries.begin(); it != longTapCloseEntries.end(); ++it)
                        {
                            closeOne(*it);
                        }

                        for(ExeEntries::const_iterator it = longTapEntries.begin(); it != longTapEntries.end(); ++it)
                        {
                            startOne(it->filePath, it->arguments);
                        }
                        BringWindowToTop(hWnd);
                    }else
                    {
                        PostQuitMessage(0);										
                    }
                }
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
            KillTimer(hWnd, TIMER_POLL);
            PostQuitMessage(0);
        break;
			
        // Timer
        case WM_TIMER:
            {
                switch(wParam)
                {
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

static SYSTEMTIME gPrevTime = {0, 0, 0, 0, 0, 0, 0, 0};
bool printDate(HWND hWnd, bool forceRepaint/* = false*/)
{
    SYSTEMTIME lt;

    //Get current date
    GetLocalTime(&lt);
    if(!forceRepaint && lt.wDay == gPrevTime.wDay && lt.wMonth == gPrevTime.wMonth && lt.wYear == gPrevTime.wYear)
        return false;

    gPrevTime = lt;
    wchar_t buf[5];
    
    // Year
    swprintf_s(buf, 5, L"%.4d", lt.wYear);
    std::wstring year = buf;

    // Day
    swprintf_s(buf, 5, L"%.2d", lt.wDay);
    std::wstring day = buf;
    
    // Month
    std::wstring month;
    if(lt.wMonth <= monthMap.size())
    {
        month = monthMap[lt.wMonth-1];
    }else
    {
        swprintf_s(buf, 5, L"%.2d", lt.wMonth);
        month = buf;
    }
    
    // Week Day
    if(lt.wDayOfWeek == 0)
        lt.wDayOfWeek = 7;
    std::wstring weekDay;
    if(lt.wDayOfWeek <= dayMap.size())
    {
        weekDay = dayMap[lt.wDayOfWeek - 1];
    }else
    {
        swprintf_s(buf, 5, L"%.2d", lt.wDayOfWeek);
        weekDay = buf;
    }
    
    std::wstring newDate = dateFormat;
    
    int tokenPos = newDate.find(YEAR_TOKEN);
    if(tokenPos != std::wstring::npos)
    {
        newDate.replace(tokenPos, wcslen(YEAR_TOKEN), year);
    }

    tokenPos = newDate.find(MONTH_TOKEN);
    if(tokenPos != std::wstring::npos)
    {
        newDate.replace(tokenPos, wcslen(MONTH_TOKEN), month);
    }

    tokenPos = newDate.find(DAY_TOKEN);
    if(tokenPos != std::wstring::npos)
    {
        newDate.replace(tokenPos, wcslen(DAY_TOKEN), day);
    }

    tokenPos = newDate.find(WEEKDAY_TOKEN);
    if(tokenPos != std::wstring::npos)
    {
        newDate.replace(tokenPos, wcslen(WEEKDAY_TOKEN), weekDay);
    }

    gButtonText = newDate;
    return true;
}

void printCPULoad()
{
    wchar_t text[8];
    
    if(pSystemMeter.get() == NULL)
        pSystemMeter.reset(new Utils::SystemMeter());

    swprintf_s(text, 8, L"%3d%%", pSystemMeter->getCPULoad());
    gButtonText = text;
}
