// USBTags.cpp 

#include "stdafx.h"
#include "USBTags.h"

#include <wingdi.h>
#include <windowsx.h>
#include <imaging.h>
#include <imgguids.h>

#include <CmnDLL.h>
#include <medianav.h>
#include <smartptr.h>

#include <fileref.h>
#include <tfilestream.h>
#include <tag.h>
#include <mp4tag.h>
#include <flacfile.h>
#include <asffile.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <oggflacfile.h>

using namespace MediaNav;
// Global Variables:
HINSTANCE			g_hInst;			// current instance
Utils::SystemWideUniqueInstance g_uniqueInstance(IPC_WNDCLASS);
smart_ptr<CSharedMemory> pPlayerStatus;
USBPlayerStatus g_cachedInfo;
int coverWidth, coverHeight;
bool g_foreignImage = false;

// Forward declarations of functions included in this code module:
ATOM			USBTagsRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void processIpcMsg(const IpcMsg& ipcMsg, bool sendMsg);
bool getTags(USBPlayerStatus& info);
HBITMAP createPictureBitmap(const void* data, size_t dataSize, int width, int height);
void updateCache(const USBPlayerStatus& info, bool foreignImage);
void readAlbumDimensions();

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

    readAlbumDimensions();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
	}
    
    if(!g_foreignImage && g_cachedInfo.m_hImgHandle)
        DeleteBitmap(g_cachedInfo.m_hImgHandle);

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


    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    IpcMsg ipcMsg;
    IpcGetMsg(&ipcMsg, message, wParam, lParam);
    if(ipcMsg.isValid())
    {
        processIpcMsg(ipcMsg, message == WM_COPYDATA);
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


void processIpcMsg(const IpcMsg& ipcMsg, bool sendMsg)
{
    if(ipcMsg.src != IpcTarget_MgrUSB)
        return;
    switch(ipcMsg.cmd)
    {
        case MgrUSB_PlayStatusResume:
        case MgrUSB_PlayStatusUpdate:
        case MgrUSB_PlayStatusStateChange:
            {
                USBPlayerStatus playerStatus;
                pPlayerStatus->read(&playerStatus, 0, sizeof(USBPlayerStatus));
                if(getTags(playerStatus))
                {
                    // Modify song info only to avoid race condition against flags updates with MgrUSB
                    pPlayerStatus->write(reinterpret_cast<BYTE *>(&playerStatus) + sizeof(PlayerTimeData)
                    , sizeof(PlayerTimeData)
                    , sizeof(MediaInfoStr) * 3); // m_song + m_artist + m_album
                    
#ifdef NO_INFO_OVERRIDE
                    if(playerStatus.m_hImgHandle)
#endif
                        pPlayerStatus->write(&playerStatus.m_hImgHandle, sizeof(USBPlayerStatus) - sizeof(DWORD) * 2, sizeof(DWORD));
                }
                break;
            }
         default:
             debugPrintf(DBG, L"USBTag: src=%d, cmd=%d, extraSize=%d, extra=%d, sendMsg=%s\r\n", ipcMsg.src, ipcMsg.cmd, ipcMsg.extraSize, ipcMsg.extra, sendMsg ? L"TRUE" : L"FALSE");
            break;
    }

    if(sendMsg)
    {
        IpcSendMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, ipcMsg.extraSize, &ipcMsg.extra);
    }else
    {
        IpcPostMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, ipcMsg.extraSize, &ipcMsg.extra);
    }
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
        TagLib::FileStream fileStream(fileName, true);// Open file in readonly mode.
        TagLib::FileRef f(&fileStream, false);
        if(f.isNull())
            return false;
        inplaceInfoString(info.m_artist, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->artist(), L"No Artist");
        inplaceInfoString(info.m_album, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->album(), L"No Album");
        inplaceInfoString(info.m_song, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->title(), info.m_fileName);

#ifndef NO_INFO_OVERRIDE
        info.m_hImgHandle = NULL;
#endif
        // Read cover image
        if(coverWidth > 0 && coverHeight > 0 )
        {
            do 
            {
#ifdef NO_INFO_OVERRIDE
                if(info.m_hImgHandle != NULL)
                    break;
#endif
                // MP4
                {
                    TagLib::MP4::Tag *mp4Tag = dynamic_cast<TagLib::MP4::Tag *>(f.tag());
                    if(mp4Tag != NULL)
                    {
                        TagLib::MP4::ItemListMap itemsListMap = mp4Tag->itemListMap();
                        TagLib::MP4::Item coverItem = itemsListMap["covr"];
                        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
                        if (!coverArtList.isEmpty()) 
                        {
                            TagLib::MP4::CoverArt coverArt = coverArtList.front();
                            info.m_hImgHandle = createPictureBitmap(coverArt.data().data(), coverArt.data().size(), coverWidth, coverHeight);
                        }
                        break;
                    }
                }
                // Flac
                {
                    TagLib::FLAC::File *flacFile = dynamic_cast<TagLib::FLAC::File*>(f.file());
                    if(flacFile != NULL)
                    {
                        TagLib::List<TagLib::FLAC::Picture*> pictures = flacFile->pictureList();
                        if(!pictures.isEmpty())
                        {
                            TagLib::FLAC::Picture* pPicture = pictures[0];
                            if(pPicture)
                            {
                                info.m_hImgHandle = createPictureBitmap(pPicture->data().data(), pPicture->data().size(), coverWidth, coverHeight);
                            }
                        }
                        break;
                    }
                }
                // ASF(wma)
                {
                    TagLib::ASF::File *asfFile = dynamic_cast<TagLib::ASF::File*>(f.file());
                    if(asfFile != NULL)
                    {
                        const TagLib::ASF::AttributeListMap& attrListMap = asfFile->tag()->attributeListMap();
                        const TagLib::ASF::AttributeList& pictures = attrListMap["WM/Picture"];
                        if(!pictures.isEmpty())
                        {
                            const TagLib::ASF::Picture& picture = pictures[0].toPicture();
                            if(picture.isValid())
                            {
                                info.m_hImgHandle = createPictureBitmap(picture.picture().data(), picture.picture().size(), coverWidth, coverHeight);
                            }
                        }
                        break;
                    }
                }
                // ID3v2
                {
                    TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(f.file());
                    if(mpegFile != NULL)
                    {
                        TagLib::ID3v2::Tag *pId3v2tag = mpegFile->ID3v2Tag();
                        if(pId3v2tag)
                        {
                            const TagLib::ID3v2::FrameList& frame = pId3v2tag->frameListMap()["APIC"] ;
                            if (!frame.isEmpty() )
                            {
                                for(TagLib::ID3v2::FrameList::ConstIterator it = frame.begin(); it != frame.end(); ++it)
                                {
                                    TagLib::ID3v2::AttachedPictureFrame *pPicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);
                                    if(pPicFrame)
                                    {
                                        info.m_hImgHandle = createPictureBitmap(pPicFrame->picture().data(), pPicFrame->picture().size(), coverWidth, coverHeight);
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                // Xiph
                {
                    TagLib::Ogg::XiphComment *xiphTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.tag());
                    if(xiphTag != NULL)
                    {
                        TagLib::List<TagLib::FLAC::Picture*> pictures = xiphTag->pictureList();
                        if(!pictures.isEmpty())
                        {
                            TagLib::FLAC::Picture* pPicture = pictures[0];
                            if(pPicture)
                            {
                                info.m_hImgHandle = createPictureBitmap(pPicture->data().data(), pPicture->data().size(), coverWidth, coverHeight);
                            }
                        }
                        break;
                    }
                }
            } while (false);
        }
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
    bool flushCache = false;
    if(coverWidth <= 0 || coverHeight <= 0)
    {
        readAlbumDimensions();
        flushCache = coverWidth > 0 && coverHeight > 0;
    }

    if(!flushCache && memcmp(g_cachedInfo.m_path, info.m_path, sizeof(MediaInfoStr) * 2) == 0)
    {
        // We have already read this file info
        memcpy(&info.m_song, &g_cachedInfo.m_song, sizeof(MediaInfoStr) * 3);
        if(g_cachedInfo.m_hImgHandle)
            info.m_hImgHandle = g_cachedInfo.m_hImgHandle;
        return true;
    }
    
#ifdef NO_INFO_OVERRIDE
    // Check if info already provided
    if(wcsncmp(info.m_fileName, info.m_song, MediaInfoStringLenght) != 0)
    {
        // Song title is set, so re-use provided info.
        updateCache(info, true);
        return true;
    }
#endif 

    // A new item. Read the data.
    TagLib::String filePath(info.m_path);
    filePath += info.m_fileName;

    if(!readFileInfo(filePath.toCWString(), info))
        return false;

    updateCache(info, false);
    return true;
}

void updateCache(const USBPlayerStatus& info, bool foreignImage)
{
    if(!g_foreignImage && g_cachedInfo.m_hImgHandle)
        DeleteBitmap(g_cachedInfo.m_hImgHandle);
    g_cachedInfo = info;
    g_foreignImage = foreignImage;
}

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

void readAlbumDimensions()
{
    coverWidth = Utils::RegistryAccessor::getInt(HKEY_LOCAL_MACHINE, L"\\LGE\\SystemInfo", L"ALBUMART_WIDTH", -1);
    coverHeight = Utils::RegistryAccessor::getInt(HKEY_LOCAL_MACHINE, L"\\LGE\\SystemInfo", L"ALBUMART_HEIGHT", -1);
}
