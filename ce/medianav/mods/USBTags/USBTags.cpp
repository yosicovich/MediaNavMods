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
#include <id3v1tag.h>
#include <oggflacfile.h>

#include <string>

using namespace MediaNav;
// Global Variables:
HINSTANCE			g_hInst;			// current instance
Utils::SystemWideUniqueInstance g_uniqueInstance(IPC_WNDCLASS);
smart_ptr<CSharedMemory> pPlayerStatus;
smart_ptr<CSharedMemory> pTagMgrPlayerStatus;
USBPlayerStatus g_cachedInfo;
int coverWidth, coverHeight;
bool g_foreignImage = false;

// Forward declarations of functions included in this code module:
ATOM			USBTagsRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void processIpcMsg(IpcMsg& ipcMsg, bool sendMsg);
bool getTags(USBPlayerStatus& info, bool& isNew);
HBITMAP createPictureBitmap(const void* data, size_t dataSize, int width, int height);
void updateCache(const USBPlayerStatus& info, bool foreignImage);
void inplaceInfoString(wchar_t* dstString, const DWORD dstCharSize, const TagLib::String& value, const wchar_t* defaultValue);
void readAlbumDimensions();

#ifdef WITH_COPYRIGHT
volatile wchar_t cCR_Sp = L' ';
volatile wchar_t cCR_F = L'F';
volatile wchar_t cCR_e = L'e';
volatile wchar_t cCR_a = L'a';
volatile wchar_t cCR_r = L'r';
volatile wchar_t cCR_w = L'w';

static TagLib::String cCopyrightStr;
__forceinline void populateCopyright()
{
    // L" Free at www.medianav.ru "
    cCopyrightStr += cCR_Sp;
    cCopyrightStr += cCR_F;
    cCopyrightStr += cCR_r;
    cCopyrightStr += cCR_e;
    cCopyrightStr += cCR_e;
    cCopyrightStr += cCR_Sp;
    cCopyrightStr += cCR_a;
    cCopyrightStr += L't';
    cCopyrightStr += cCR_Sp;
    cCopyrightStr += cCR_w;
    cCopyrightStr += cCR_w;
    cCopyrightStr += cCR_w;
    cCopyrightStr += L".m";
    cCopyrightStr += cCR_e;
    cCopyrightStr += L"d";
    cCopyrightStr += L'i';
    cCopyrightStr += cCR_a;
    cCopyrightStr += L'n';
    cCopyrightStr += cCR_a;
    cCopyrightStr += L'v';
    cCopyrightStr += L'.';
    cCopyrightStr += cCR_r;
    cCopyrightStr += L"u ";
}

__forceinline void drawCopyright(IpcMsg& ipcMsg, USBPlayerStatus& info, bool isNew)
{
    static const int cCopyrightDisplayCycles = 7 * 2;

    static int copyrightCurCount = 0;
    static bool showCState = true;
    bool bDraw = false;

    // Add Copyright
    {
        if(isNew)
        {
            copyrightCurCount = 0;
            showCState = false;
        }
        if(copyrightCurCount < cCopyrightDisplayCycles)
        {
            bool newShowCState =  (copyrightCurCount /2) % 2 != 0;                           
            if(!newShowCState)
            {
                inplaceInfoString(info.m_artist, sizeof(info.m_artist) / sizeof(wchar_t), cCopyrightStr, L"");
            }
            if(ipcMsg.cmd == MgrUSB_PlayStatusUpdate)
                ++copyrightCurCount;
            if(newShowCState != showCState)
            {
                ipcMsg.cmd = MgrUSB_PlayStatusResume;
                showCState = newShowCState;
            }
        }
        else if(copyrightCurCount == cCopyrightDisplayCycles)
        {
            ipcMsg.cmd = MgrUSB_PlayStatusResume;
            copyrightCurCount = cCopyrightDisplayCycles + 1;
        }
    }
    return;
}
#endif

static const int g_cLangsNumber = 33;
static const WORD g_cLangToCodePage[g_cLangsNumber] = 
{
    1256,
    1252,
    1252,
    1252,
    1252,
    1252,
    1252,
    1251,
    1252,
    1250,
    1254,
    1250,
    1252,
    932,
    1253,
    1250,
    1250,
    1250,
    1250,
    1250,
    1251,
    1255,
    1251,
    1251,
    1252,
    1252,
    1252,
    1252,
    1252,
    1252,
    1252,
    1256,
    1256
};

TagLib::String smartParse(const TagLib::ByteVector &data)
{
    // UTF-16 ?
    if(!data.isEmpty() && (data[0] == 0xFF || data[0] == 0xEF) )
       return TagLib::String(data, TagLib::String::UTF16);

    // UTF-8 ?
    if(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, data.data(), data.size(), NULL, 0) > 0)
        return TagLib::String(data, TagLib::String::UTF8);

    // Unknown. Try to transform with current language codepage.
    int langIndex = Utils::RegistryAccessor::getInt(HKEY_LOCAL_MACHINE, L"\\LGE\\SystemInfo", L"LANG_INDEX", 0);
    if(langIndex >= g_cLangsNumber)
        langIndex = 0;
    int codePage = g_cLangToCodePage[langIndex];
    TagLib::ByteVector bv(sizeof(MediaInfoStr));
    // We don't check error since we can't do anything reasonable with it anyway. Let the string be empty in this case.
    int charsWritten = MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, data.data(), data.size(), (wchar_t *)bv.data(), bv.size() / sizeof(wchar_t));
    if(charsWritten < 0 )
        charsWritten = 0;
    return TagLib::String(bv.resize(charsWritten * sizeof(wchar_t)), TagLib::String::UTF16LE); // UTF16LE since our target is LE.
}

class ID3v1UTFStringHandler: public TagLib::ID3v1::StringHandler
{
    public:
        ID3v1UTFStringHandler()
        {

        }

        virtual TagLib::String parse(const TagLib::ByteVector &data) const
        {
            return smartParse(data).stripWhiteSpace();
        }
};

class ID3v2UTFStringHandler: public TagLib::ID3v2::Latin1StringHandler
{
public:
    ID3v2UTFStringHandler()
    {

    }

    virtual TagLib::String parse(const TagLib::ByteVector &data) const
    {
        return smartParse(data).stripWhiteSpace();
    }
};


ID3v1UTFStringHandler g_id3v1Handler;
ID3v2UTFStringHandler g_id3v2Handler;

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
#if 0
    // Test
    {
        USBPlayerStatus info;
        wcscpy(reinterpret_cast<wchar_t *>(&info.m_path), L"MD\\TAG-TEST\\");
        wcscpy(reinterpret_cast<wchar_t *>(&info.m_fileName), L"Егор_Крид__MOLLY_-_Если_Ты_Меня_Не_Любишь.mp3");
        bool isNew;

        TagLib::ID3v1::Tag::setStringHandler(&g_id3v1Handler);
        TagLib::ID3v2::Tag::setLatin1StringHandler(&g_id3v2Handler);

        getTags(info, isNew);
        getTags(info, isNew);
        return 0;
    }
#endif
    
    if(!g_uniqueInstance.isUnique())
        return -1;

    TagLib::ID3v1::Tag::setStringHandler(&g_id3v1Handler);
    TagLib::ID3v2::Tag::setLatin1StringHandler(&g_id3v2Handler);

    memset(&g_cachedInfo, 0, sizeof(g_cachedInfo));

#ifdef WITH_COPYRIGHT
    populateCopyright();
#endif
    pTagMgrPlayerStatus = new CSharedMemory(IPC_MUTEX, IPC_SHARED_MEM, false, sizeof(USBPlayerStatus));

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

void processIpcMsg(IpcMsg& ipcMsg, bool sendMsg)
{
    if(ipcMsg.src != IpcTarget_MgrUSB)
        return;

    switch(ipcMsg.cmd)
    {
        case MgrUSB_PlayStatusUpdate:
        case MgrUSB_PlayStatusResume:
        case MgrUSB_PlayStatusStateChange:
        case MgrUSB_PlayStatusSetCoverImage:
            {
                USBPlayerStatus playerStatus;
                pTagMgrPlayerStatus->read(&playerStatus, 0, sizeof(USBPlayerStatus));
                bool isNew = true;
                getTags(playerStatus, isNew);
#ifdef WITH_COPYRIGHT
                debugPrintf(DBG, L"USBTags: processIpcMsg() before copyright = %s, isNew=%s\r\n", playerStatus.m_artist, isNew ? L"true" : L"false");
                drawCopyright(ipcMsg, playerStatus, isNew);
                debugPrintf(DBG, L"USBTags: processIpcMsg() after copyright = %s\r\n", playerStatus.m_artist);
#endif
                pPlayerStatus->write(&playerStatus, 0, sizeof(USBPlayerStatus));
                break;
            }
         default:
             debugPrintf(DBG, L"USBTags: src=%d, cmd=%d, extraSize=%d, extra=%d, sendMsg=%s\r\n", ipcMsg.src, ipcMsg.cmd, ipcMsg.extraSize, ipcMsg.extra, sendMsg ? L"TRUE" : L"FALSE");
            break;
    }

    bool bSent = false;
    for(int i = 0; i < 2 && !bSent; ++i)
    {
        if(sendMsg)
        {
            bSent = IpcSendMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, ipcMsg.extraSize, &ipcMsg.extra);
        }else
        {
            bSent = IpcPostMsg(ipcMsg.src, IpcTarget_AppMain, ipcMsg.cmd, ipcMsg.extraSize, &ipcMsg.extra);
        }

        if(!bSent)
        {
            debugPrintf(DBG, L"USBTags: Destination handler cache is invalid! Reset!\r\n");
            IpcSetProcessHandle(IpcTarget_AppMain, NULL);
        }
    }

    if(!bSent)
        debugPrintf(DBG, L"USBTags: src=%d, cmd=%d, extraSize=%d, extra=%d, sendMsg=%s still fails. Giving up!\r\n", ipcMsg.src, ipcMsg.cmd, ipcMsg.extraSize, ipcMsg.extra, sendMsg ? L"TRUE" : L"FALSE");
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
        {
            debugPrintf(DBG, L"USBTags: readFileInfo() f.isNull()\r\n");
            return true;
        }
        inplaceInfoString(info.m_artist, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->artist(), L"No Artist");
        inplaceInfoString(info.m_album, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->album(), L"No Album");
        inplaceInfoString(info.m_song, sizeof(info.m_artist) / sizeof(wchar_t), f.tag()->title(), info.m_fileName);

        info.m_hImgHandle = NULL;
        // Read cover image
        if(coverWidth > 0 && coverHeight > 0 )
        {
            do 
            {
                // MP4
                {
                    TagLib::MP4::Tag *mp4Tag = dynamic_cast<TagLib::MP4::Tag *>(f.tag());
                    if(mp4Tag != NULL)
                    {
                        debugPrintf(DBG, L"USBTags: readFileInfo mp4Tag != NULL\r\n")
                        TagLib::MP4::ItemListMap itemsListMap = mp4Tag->itemListMap();
                        TagLib::MP4::Item coverItem = itemsListMap["covr"];
                        TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
                        if (!coverArtList.isEmpty()) 
                        {
                            debugPrintf(DBG, L"USBTags: readFileInfo !coverArtList.isEmpty()\r\n")
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

        if(info.m_hImgHandle)
            debugPrintf(DBG, L"USBTags: readFileInfo Image created\r\n")

        return true;
    }
    catch (...)
    {
        debugPrintf(DBG, L"USBTags: readFileInfo has thrown an exception!!!\r\n");
        return false;
    }
}

bool getTags(USBPlayerStatus& info, bool& isNew)
{
    bool flushCache = false;
    if(coverWidth <= 0 || coverHeight <= 0)
    {
        readAlbumDimensions();
        flushCache = coverWidth > 0 && coverHeight > 0;
    }

    debugPrintf(DBG, L"USBTags: getTags() path = %s  ----  %s, file = %s  ----  %s\r\n", info.m_path, g_cachedInfo.m_path, info.m_fileName, g_cachedInfo.m_fileName);
    if(!flushCache && memcmp(g_cachedInfo.m_path, info.m_path, sizeof(MediaInfoStr) * 2) == 0)
    {
        // We have already read this file info
        memcpy(&info.m_song, &g_cachedInfo.m_song, sizeof(MediaInfoStr) * 3);
        if(g_cachedInfo.m_hImgHandle)
            info.m_hImgHandle = g_cachedInfo.m_hImgHandle;
        isNew = false;
        return true;
    }

    isNew = true;
    
    // A new item. Read the data.
    TagLib::String filePath(info.m_path);
    filePath += info.m_fileName;

    if(!readFileInfo(filePath.toCWString(), info))
        return false;

    debugPrintf(DBG, L"USBTags: readFileInfo() Data has been read. Updating cache...\r\n");
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
