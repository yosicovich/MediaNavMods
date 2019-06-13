#include "utils.h"
#include <exception>
#include <stdarg.h>
#include <sstream>

#include <wingdi.h>
#include <windowsx.h>
#include <imaging.h>
#include <imgguids.h>


namespace Utils {

SystemWideUniqueInstance::SystemWideUniqueInstance(const std::wstring& name)
:lock_(name.c_str())
,isUnique_(lock_.WaitLock(0))
{
}

bool SystemWideUniqueInstance::isUnique()
{
    return isUnique_;
}

void FileLogger::writeLog(const wchar_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    writeLog(fmt, args);
    va_end(args);
}

void FileLogger::writeLog(const wchar_t* fmt, va_list args)
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);

    CLockHolder<CLock> lock(m_lock);
    if(!m_file)
    {
        m_file = _wfopen(m_fileName.c_str(), L"ab");
        if(!m_file)
            return;
        fwprintf_s(m_file, L"\r\n\r\n--------------------- NEW LOG SESSION STARTED AT %04hu-%02hu-%02hu  %02hu:%02hu:%02hu.%03hu ---------------------\r\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
    }
    
    fwprintf_s(m_file, L"%04hu-%02hu-%02hu at %02hu:%02hu:%02hu.%03hu --- ", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);

    vfwprintf_s(m_file, fmt, args);
    fflush(m_file);
}

int OleInitializer::m_useCount = 0;
CLock OleInitializer::m_accessLock;

OleInitializer::OleInitializer()
{
    CLockHolder<CLock>  lock(m_accessLock);
    if(!m_useCount++)
    {
        if(CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
        {
            --m_useCount;
            exit(-1); // it is fatal if OLE is required.
            return;
        }
    }
}

OleInitializer::~OleInitializer()
{
    CLockHolder<CLock>  lock(m_accessLock);
    if(!--m_useCount)
    {
        CoUninitialize();
    }
}

void dumpBinary(const void* buf, size_t size)
{
    for(size_t i = 0; i < size; ++i)
    {
        if(i > 0 && (i % 16) == 0)
            NKDbgPrintfW(L"\r\n");
        NKDbgPrintfW(L" %02X", ((const char*)buf)[i] & 0xFF);
    }
    NKDbgPrintfW(L"\r\n");
}

void dumpGUID(const GUID* guid)
{
    NKDbgPrintfW(L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\r\n", guid->Data1, (unsigned int)guid->Data2, (unsigned int)guid->Data3
        ,(unsigned int)guid->Data4[0]
        ,(unsigned int)guid->Data4[1]
        ,(unsigned int)guid->Data4[2]
        ,(unsigned int)guid->Data4[3]
        ,(unsigned int)guid->Data4[4]
        ,(unsigned int)guid->Data4[5]
        ,(unsigned int)guid->Data4[6]
        ,(unsigned int)guid->Data4[7]
    );
}

std::wstring convertToWString(const std::string& str)
{
    if(!str.size()) 
        return std::wstring();

    DWORD cwch;

    std::vector<wchar_t> resultWStr;

    if(cwch = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0))//get buffer required in characters
    {
        resultWStr.resize(cwch);
        if(!::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), reinterpret_cast<wchar_t *>(&resultWStr[0]), resultWStr.size()))
        {
            if(ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                return std::wstring();
        }
    };

    return std::wstring(reinterpret_cast<wchar_t *>(&resultWStr[0]), resultWStr.size());
}

std::string convertToAString(const std::wstring& str)
{
    if(!str.size()) 
        return std::string();

    size_t cch;

    std::vector<char> resultAStr;

    if(cch = wcstombs(NULL, str.c_str(), str.size()))//get buffer required in characters
    {
        if(cch == -1)
            return std::string();
        resultAStr.resize(cch);
        if(wcstombs(reinterpret_cast<char *>(&resultAStr[0]), str.c_str(), str.size()) != cch)
        {
            return std::string();
        }
    }

    return std::string(reinterpret_cast<char *>(&resultAStr[0]), resultAStr.size());
}

std::vector<std::wstring> splitWString(const std::wstring& str, wchar_t token)
{
    std::vector<std::wstring> results;
    std::wistringstream iStr(str);
    std::wstring sTmp;    
    while (getline(iStr, sTmp, token)) 
    {
        results.push_back(sTmp);
    }
    return results;
}

bool checkRectCompleteCovered(HWND hWnd, RECT rect, const std::set<HWND>& skipWindows/* = std::set<HWND>()*/)
{
    HRGN selfRgn = CreateRectRgnIndirect(&rect);
    HWND currentWnd = GetWindow(NULL, GW_HWNDFIRST);
    int rgnType = SIMPLEREGION;
    while (currentWnd && currentWnd !=hWnd && rgnType != NULLREGION)
    {
        if(IsWindowVisible(currentWnd))
        {
            if(GetWindowRect(currentWnd, &rect) == FALSE)
                break;
            HRGN tempRgn = CreateRectRgnIndirect(&rect);// currently examined window region
            rgnType = CombineRgn(selfRgn, selfRgn, tempRgn, RGN_DIFF); // diff intersect
            DeleteObject( tempRgn );
        }
        if (rgnType != NULLREGION) // there's a remaining portion
            currentWnd = GetWindow(currentWnd, GW_HWNDNEXT);

        while(currentWnd && skipWindows.find(currentWnd) != skipWindows.end())
            currentWnd = GetWindow(currentWnd, GW_HWNDNEXT);
    }
    DeleteObject(selfRgn);
    return rgnType == NULLREGION;
}

bool HasAlphaChannel(HBITMAP hBmp)
{
    DIBSECTION ds;
    GetObject(hBmp, sizeof(DIBSECTION), &ds);

    return (ds.dsBm.bmBitsPixel == 32);
}

HBITMAP LoadPicture(const TCHAR* pathName, bool& hasAlpha, int& width, int& height)
{
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
        if(pFactory->CreateImageFromFile(pathName, &pImage) != S_OK)
            break;

        if(pImage->GetImageInfo(&imageInfo) != S_OK)
            break;

        HDC windowDC = GetDC(NULL);
        if(!windowDC)
            break;

        memHdc = CreateCompatibleDC(windowDC);

        if(!memHdc)
        {
            ReleaseDC(NULL, windowDC);
            break;
        }

        hBitmap = CreateCompatibleBitmap(windowDC, imageInfo.Width, imageInfo.Height);
        ReleaseDC(NULL, windowDC);

        if(!hBitmap)
            break;

        RECT rect;
        SetRect(&rect, 0, 0, imageInfo.Width, imageInfo.Height);
        SelectObject(memHdc, hBitmap);
        if(pImage->Draw(memHdc, &rect, NULL) != S_OK)
            break;
        imageOk = true;
    } while (false);

    if(memHdc)
        DeleteDC(memHdc);

    if(pImage)
        pImage->Release();

    if(pFactory)
        pFactory->Release();

    if(imageOk)
    {
        hasAlpha = IsAlphaPixelFormat(imageInfo.PixelFormat) == TRUE;
        width = imageInfo.Width;
        height = imageInfo.Height;
        return hBitmap;
    }

    if(hBitmap)
        DeleteBitmap(hBitmap);

    // Try to load with SH routine
    hBitmap = SHLoadDIBitmap(pathName);
    if(!hBitmap)
        return NULL;

    hasAlpha = HasAlphaChannel(hBitmap);

    BITMAP bm;
    if(!GetObject(hBitmap, sizeof(bm), &bm))
    {
        DeleteBitmap(hBitmap);
        return NULL;
    }

    width = bm.bmWidth;
    height = bm.bmHeight;

    return hBitmap;
}

bool detectPath(const std::wstring& path, DWORD timeoutS)
{
    DWORD endTime = GetTickCount() + timeoutS * 1000;
    while(GetTickCount() <= endTime)
    {
        if(isPathPresent(path))
        {
            return true;
        }
        Sleep(100);
    }
    return false;

}

// CSharedLock
CSharedLock::CSharedLock(const wchar_t* name)
:m_hMutex(NULL)
{
    m_hMutex = CreateMutex(NULL, FALSE, name);

    if(m_hMutex == NULL)
    {
        throw std::exception("Unable to CREATE mutex");
    }
}

CSharedLock::~CSharedLock()
{
    Unlock();
    CloseHandle(m_hMutex);
}

void CSharedLock::Lock()
{
    WaitLock(INFINITE);
}

bool CSharedLock::WaitLock(DWORD timeoutMs)
{
    return WaitForSingleObject(m_hMutex, timeoutMs) == WAIT_OBJECT_0;
}

void CSharedLock::Unlock()
{
    ReleaseMutex(m_hMutex);
}

}; //namespace Utils

