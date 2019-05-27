#include "glue_other.h"

wchar_t* CResManagerExt::m_imageTable[] = {
    TEXT("evotech\\4wd\\4wd_eco_btn.bmp"),
    TEXT("evotech\\4wd\\4wd_info_btn.bmp"),
    TEXT("evotech\\4wd\\4wd_driving_btn.bmp")
};

static CResManagerExt** g_ResManager = reinterpret_cast<CResManagerExt**>(CResManager__g_ResManager);
CResManagerExt* CResManagerExt::singleton()
{
    if(*g_ResManager)
        return *g_ResManager;
    *g_ResManager = new CResManagerExt();
    return *g_ResManager;
}

CResManagerExt::CResManagerExt()
{
    if(m_imageCache)
    {
        delete[] m_imageCache;
        m_maxImages = 0;
    }

    DWORD m_cacheSize = cExtImageTableBase + sizeof(m_imageTable) / sizeof(wchar_t*);
    m_imageCache = new HBITMAP[m_cacheSize];
    memset(m_imageCache, 0, sizeof(HBITMAP) * m_cacheSize);
    m_maxImages = m_cacheSize;
}

const wchar_t* CResManagerExt::getImagePath(DWORD imageID)
{
    static const wchar_t** pImageTable = reinterpret_cast<const wchar_t**>(CResManager__imageTable);
    static wchar_t stringBuffer[MediaNav::MaxStringBufferLength];
    const wchar_t* pImagePath;

    if(imageID >= cExtImageTableBase)
    {
        pImagePath = m_imageTable[imageID - cExtImageTableBase];
    }else
    {
        pImagePath = pImageTable[imageID];
    }

    wsprintf(&stringBuffer[0], TEXT("%s%s"), m_imagesPath, pImagePath);
    return &stringBuffer[0];
}


