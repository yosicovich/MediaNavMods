#include "stdafx.h"
#include "Utils.h"
#include <stdlib.h>

HWND FindWindowFromPath(const TCHAR *fullpath) 
{
    const TCHAR* processName = wcsrchr(fullpath, '\\');
    if (processName == NULL || lstrlen(processName) == 1)
        return FALSE;

    processName += 1;

    DWORD dwProcessId = FindProcessId(processName);
    EnumData ed = { dwProcessId };

    if (EnumWindows(EnumProc, (LPARAM)&ed))
            return ed.hWnd;

    return NULL;
}

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
{
    EnumData& ed = *(EnumData*)lParam;
    DWORD dwID = 0;

    GetWindowThreadProcessId(hWnd, &dwID);

    if (ed.dwPID == dwID) {
        ed.hWnd = hWnd;
        SetLastError(ERROR_SUCCESS);
        return FALSE;
    }

    return TRUE ;
}

bool IsProcessExist(const std::wstring& fullpath) 
{
    size_t index = fullpath.rfind('\\');
    if (index == std::wstring::npos || index == fullpath.size() - 1)
        return FALSE;

    fullpath.substr(index + 1);

    return FindProcessId(fullpath.substr(index + 1).c_str()) > 0;
}

BYTE* LoadFileData(const TCHAR *file)
{
	HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;
	
	BY_HANDLE_FILE_INFORMATION fi;
	GetFileInformationByHandle(hFile, &fi);

	BYTE* bFile = new BYTE[fi.nFileSizeLow];
	DWORD numberOfBytesRead;
	ReadFile(hFile, bFile, fi.nFileSizeLow, &numberOfBytesRead, NULL);

	CloseHandle(hFile);

	return bFile;
}

BOOL SaveFileData(const TCHAR *file, const BYTE* data, int dataLen)
{
	HANDLE hFile = CreateFile(file, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD numberOfBytesWritten;
	WriteFile(hFile, data, dataLen, &numberOfBytesWritten, NULL);
	CloseHandle(hFile);

	return (numberOfBytesWritten == sizeof(data));
}

BOOL SaveFileData(const TCHAR *file, const BYTE* data)
{
	return SaveFileData(file, data, strlen((char*)data));
}

BOOL FileExist(const TCHAR *file)
{
	BOOL rc = 0;

	DWORD attribs = GetFileAttributes(file);
	if (attribs != -1) {
		if ( (attribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
			rc = 1;
	}

	return rc;
}

FILETIME FileModifyTime(const TCHAR *file)
{
	FILETIME filetime = {0};
	HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		GetFileTime(hFile, NULL, NULL, &filetime);
		CloseHandle(hFile);
	}

	return filetime;
}

BOOL ShellCommand(const TCHAR* pCmdLine, const TCHAR* pParameters)
{
	
	if (pCmdLine == NULL || !FileExist(pCmdLine))
		return FALSE;

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.nShow = SW_SHOWNORMAL;
	sei.hwnd = NULL;
	sei.lpParameters = pParameters;
	sei.lpFile = pCmdLine;

	return ShellExecuteEx(&sei);
}

DWORD FindProcessId(const TCHAR *processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot == INVALID_HANDLE_VALUE )
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (_tcscmp(processName, processInfo.szExeFile) == 0) {
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo)) {
		if (_tcscmp(processName, processInfo.szExeFile) == 0) {
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	
	CloseHandle(processesSnapshot);
	return 0;
}

void KillProcess(DWORD PID)
{
	HANDLE hApp = OpenProcess(PROCESS_TERMINATE, FALSE, PID);
	TerminateProcess(hApp, 0);
	CloseHandle(hApp);
}

void KillProcess(const TCHAR *processName)
{
	KillProcess(FindProcessId(processName));
}

BYTE* StrReplace(const char *search , const char *replace , char *subject)
{
	char  *p = NULL , *old = NULL , *new_subject = NULL ;
	int c = 0, search_size;

	search_size = strlen(search);
	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
		c++;

	c = (strlen(replace) - search_size)*c + strlen(subject) + 1;

	new_subject = new char[c];
	old = subject;

	for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search)) {
		StringCchCopyNA(new_subject + strlen(new_subject) , c, old , p - old);
		StringCchCopyA(new_subject + strlen(new_subject), c, replace);

		old = p + search_size;
	}

	int len = strlen(new_subject);
	StringCchCopyA(new_subject + len , c, old);

	return (BYTE*)new_subject;
}

bool GetCurrExePath(std::wstring& path)
{
    WCHAR modulePath[MAX_PATH];

    if ((GetModuleFileName(NULL, modulePath, MAX_PATH)) > 0) {
        LPWSTR lastBackSlash = wcsrchr(modulePath, '\\');
        if (lastBackSlash) 
        {
            path.assign(modulePath, lastBackSlash - modulePath);
            return true;
        }
    }

    return false;
}
int GetSmallCurrExePath(LPWSTR path)
{
    WCHAR modulePath[MAX_PATH];
    memset(path, 0, MAX_PATH * sizeof(WCHAR));

    if ((GetModuleFileName(NULL, modulePath, MAX_PATH)) > 0) {
        LPWSTR lastBackSlash = wcsrchr(modulePath, '\\');
        if (lastBackSlash) {
        lastBackSlash += 1;

        
            int nbcar = wcslen(lastBackSlash) + 1; // +1 pour prendre en compte le caractère à zéro
            memcpy(path, lastBackSlash, nbcar * sizeof(WCHAR)); // c'est classe le sizeof mais ça revient à 2 ^^
            return 1;
        }
    }

    return 0;
}
HWND FindWindowFromProcessId(DWORD dwProcessId) 
{
	EnumData ed = { dwProcessId };

	if (!EnumWindows(EnumProc, (LPARAM)&ed) &&
		(GetLastError() == ERROR_SUCCESS)) {
			return ed.hWnd;
	}

	return NULL;
}

BOOL HasAlphaChannel(HBITMAP hBmp)
{
    DIBSECTION ds;
    GetObject(hBmp, sizeof(DIBSECTION), &ds);

    return (ds.dsBm.bmBitsPixel == 32);
}

int OleInitializer::m_useCount = 0;
CLock OleInitializer::m_accessLock;

OleInitializer::OleInitializer()
{
    CLockHolder  lock(m_accessLock);
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
    CLockHolder  lock(m_accessLock);
    if(!--m_useCount)
    {
        CoUninitialize();
    }
}

HBITMAP LoadPicture(const std::wstring& pathName, bool& hasAlpha, int& width, int& height)
{
    static OleInitializer oleInitializer;
    
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
        if(pFactory->CreateImageFromFile(pathName.c_str(), &pImage) != S_OK)
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
        width = imageInfo.Width/2;
        height = imageInfo.Height;
        return hBitmap;
    }
    
    if(hBitmap)
        DeleteBitmap(hBitmap);

    // Try to load with SH routine
    hBitmap = SHLoadDIBitmap(pathName.c_str());
    if(!hBitmap)
        return NULL;

    hasAlpha = HasAlphaChannel(hBitmap) == TRUE;

    BITMAP bm;
    if(!GetObject(hBitmap, sizeof(bm), &bm))
    {
        DeleteBitmap(hBitmap);
        return NULL;
    }

    width = bm.bmWidth/2;
    height = bm.bmHeight;

    return hBitmap;
}
