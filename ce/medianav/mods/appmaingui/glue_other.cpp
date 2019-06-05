#include "glue_other.h"
#include "appmaingui.h"


wchar_t* CResManagerExt::m_imageTable[] = {
    TEXT("evotech\\4wd\\4wd_eco_btn.bmp"), // 1600
    TEXT("evotech\\4wd\\4wd_info_btn.bmp"), // 1601
    TEXT("evotech\\4wd\\4wd_driving_btn.bmp"), // 1602
    TEXT("evotech\\4wd\\dinamic.bmp"), // 1603
    TEXT("evotech\\4wd\\reset.bmp"), // 1604
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
        m_oldMaxImages = m_maxImages;
        m_maxImages = 0;
    }

    DWORD m_cacheSize = cExtImageTableBase + sizeof(m_imageTable) / sizeof(wchar_t*);
#ifdef DEV_MAKET
    m_cacheSize+=100;
#endif
    m_imageCache = new HBITMAP[m_cacheSize];
    memset(m_imageCache, 0, sizeof(HBITMAP) * m_cacheSize);
    m_maxImages = m_cacheSize;
}

const wchar_t* CResManagerExt::getImagePath(DWORD imageID)
{
    static const wchar_t** pImageTable = reinterpret_cast<const wchar_t**>(CResManager__imageTable);
    static wchar_t stringBuffer[MediaNav::MaxStringBufferLength];
    const wchar_t* pImagePath;

#ifdef DEV_MAKET
    if(imageID >= cExtImageTableBase + sizeof(m_imageTable) / sizeof(wchar_t*))
    {
        pImagePath = CDevMaket::singleton()->getImagePath(imageID);
        if(!pImagePath)
            pImagePath = TEXT("bad_id.bmp");
    }
    else
#endif
    if(imageID >= cExtImageTableBase)
    {
        pImagePath = m_imageTable[imageID - cExtImageTableBase];
    }else if (imageID < m_oldMaxImages)
    {
        pImagePath = pImageTable[imageID];
    }else
        pImagePath = TEXT("bad_id.bmp");

    wsprintf(&stringBuffer[0], TEXT("%s%s"), m_imagesPath, pImagePath);
    return &stringBuffer[0];
}


