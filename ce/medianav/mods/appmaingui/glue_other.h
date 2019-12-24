#pragma once

#include "stdafx.h"
#include "common.h"
#include "glue_common.h"
#include <utils.h>
#include <medianav.h>

#define CSettings__singleton 0x00012C1C

#define CMultiLanguage__singleton 0x0001EF28
#define CMultiLanguage__getMultiLangStr 0x0001F074

#define CResManager__g_ResManager 0x0018BEC0
#define CResManager__CResManager 0x0001F118
#define CResManager_destructor_CResManager 0x0001F3B8
#define CResManager__imageTable 0x0018A784

#define CNaviManager__CNaviManager 0x000D8FA4


#define CMgrSys_setAccOff 0x000D0140
#define CMgrSys_save 0x000D0CB4
#define CMgrSys_m_instance 0x001902DC

struct CSettings
{
    static CSettings* singleton()
    {
        return reinterpret_cast<CSettings*(*)()>(CSettings__singleton)();
    }
    virtual ~CSettings();

    HWND m_appMainHWND;
    HINSTANCE m_hInstance;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;
    int m_nMapHangTimingStep;
    int field_5C;
    int field_60;
    int field_64;
    int field_68;
    int field_6C;
    int field_70;
    int field_74;
    int field_78;
    int field_7C;
    int field_80;
    int m_hMapMcmShm;
    volatile MediaNav::MicomManager::MicomManagerInfo *m_pMcmShr;
    int m_hMapIpod;
    int m_pIpodMem;
    int m_hMapIpodList;
    int m_pIpodListMem;
    int m_hMapUSB;
    int m_pUSBMem;
    int field_A4;
    int field_A8;
    int field_AC;
    int field_B0;
    int field_B4;
    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    int field_CC;
    int field_D0;
    int field_D4;
    int field_D8;
    int field_DC;
    int field_E0;
    int field_E4;
    int field_E8;
    int UI_TYPE;
    int NONAVI;
    int METER_TYPE;
    int CLOCK_TYPE;
    int LANG_TYPE;
    int RAD_COUNTRY;
    int SDVC_TYPE;
    int CNF_CMK;
    int LDNS_EN;
    BOOL ROAD_EN;
    int HEBREW_EN;
    int TOTCONSUMPTION_EN;
    int RVC_EN;
    int UPA_SND;
    int UPA_HMI;
    int AM_EN;
    int DAB_EN;
    int AHA_EN;
    int RES_EN;
    int HMI_CNF_ADAC;
    int HMI_CNF_TEMP;
    int HMI_CNF_AIR;
    int HMI_CNF_ENG;
    BOOL HMI_CNF_ECO;
    int PTT_EN;
    int SWRC_TYPE;
    int BOOT_LOGO;
    char UUID[15];
    char VIN_INFO[17];
};

class CMultiLanguage
{
public:
    virtual ~CMultiLanguage();
    static CMultiLanguage* singleton()
    {
        return reinterpret_cast<CMultiLanguage*(*)()>(CMultiLanguage__singleton)();
    }
    
    const wchar_t* getMultiLangStr(DWORD strID)
    {
        const wchar_t* str = reinterpret_cast<const wchar_t*(*)(CMultiLanguage*, DWORD)>(CMultiLanguage__getMultiLangStr)(this, strID);
#ifdef DEV_MAKET
        if(!str || wcslen(str) == 0)
            return CDevMaket::singleton()->getMultiLangStr(strID);
#endif
        return str;
    }

    inline int getLangID() const
    {
        return m_langID;
    }
protected:
    DWORD m_0x04;
    int m_langID;
};


class CResManager
{
private:
    struct ImplVTable
    {
        void (*destroy)(CResManager* pSelf, BOOL bFree);
        const wchar_t* (*getImagePathBase)(CResManager* pSelf);
        const wchar_t* (*getUITypePathPart)(CResManager* pSelf, DWORD uiType);
        const wchar_t* (*getImagePath)(CResManager* pSelf, DWORD imageID);
        void (*cleanUp)(CResManager* pSelf);
    };
public:
    CResManager()
    {
        debugPrintf(DBG, TEXT("CResManager::CResManager()\r\n"));
        reinterpret_cast<void(*)(CResManager*)>(CResManager__CResManager)(this);
        m_implVTable = *reinterpret_cast<ImplVTable**>(this);
        debugPrintf(DBG, TEXT("CResManager::m_implVTable = 0x%08X\r\n"), m_implVTable);
    }

    ~CResManager()
    {
        debugPrintf(DBG, TEXT("CResManager::~CResManager()\r\n"));
        reinterpret_cast<void(*)(CResManager*)>(CResManager_destructor_CResManager)(this);
    }

    virtual void destroy(BOOL bFree)
    {
        getImpl()->destroy(this, bFree);
    }

    virtual const wchar_t* getImagePathBase()
    {
        return getImpl()->getImagePathBase(this);
    }

    virtual const wchar_t* getUITypePathPart(DWORD uiType)
    {
        return getImpl()->getUITypePathPart(this, uiType);
    }

    virtual const wchar_t* getImagePath(DWORD imageID)
    {
        return getImpl()->getImagePath(this, imageID);
    }

    virtual void cleanUp()
    {
        getImpl()->cleanUp(this);
    }

    inline UIType getUIType() const
    {
        return static_cast<UIType>(m_uiType);
    }

protected:
    // void** __vptr;
    DWORD m_0x04;
    WORD m_0x08[44];
    DWORD m_0x60[22];
    wchar_t m_modulePath[/*MediaNav::MaxStringBufferLength*/ 0x104];
    wchar_t m_fontPath[/*MediaNav::MaxStringBufferLength*/ 0x104];
    DWORD m_maxImages;
    int m_uiType;
    DWORD m_0x4D0;
    DWORD m_topLoadedImageID;
    HDC m_hdc;
    HBITMAP *m_imageCache;
    wchar_t m_imagesPath[/*MediaNav::MaxStringBufferLength*/ 0x104];


    ImplVTable* m_implVTable;
    inline ImplVTable* getImpl()
    {
        return m_implVTable;
    }
};

class CResManagerExt: public CResManager
{
public:
    static const int cExtImageTableBase = 1600;

    static CResManagerExt* singleton();
    
    CResManagerExt();
    virtual const wchar_t* getImagePath(DWORD imageID);
private:
    static wchar_t* m_imageTable[];
    DWORD m_oldMaxImages;
};

class CNaviManager
{
public:
    CNaviManager()
    {
        reinterpret_cast<void(*)(CNaviManager*)>(CNaviManager__CNaviManager)(this);
    }

    virtual CNaviManager* close(BOOL bFree)
    {
        return NULL;
    }

    struct NaviStr
    {
        DWORD index;
        wchar_t data[MAX_PATH];// 260
    };

    DWORD m_0x04;
    UINT m_mSYSTEMtoNAVI;
    UINT m_mNAVItoSYSTEM;
    UINT m_mSYNCTOOLtoSYSTEM;
    HWND m_hNavi;
    BOOL m_bNaviStarted;
    DWORD m_0x1C;
    DWORD m_0x20;
    DWORD m_0x24;
    DWORD m_NaviActiveStatus;
    DWORD m_0x2C;
    DWORD m_0x30;
    DWORD m_0x34;
    DWORD m_0x38;
    DWORD m_0x3C;
    DWORD m_0x40;
    DWORD m_0x44;
    BOOL m_bOSOverlayEnable;
    DWORD m_0x4C;
    DWORD m_0x50;
    BOOL m_bNaviActive;
    DWORD m_0x58;
    DWORD nMapScreenType;
    DWORD m_0x60;
    DWORD m_0x64;
    DWORD m_0x68;
    DWORD m_0x6C;
    BOOL m_bUpdateStarted;
    BOOL m_bSynctoolStarted;

    NaviStr m_0x78[0x0B];
    NaviStr m_0x16FC;
    NaviStr m_0x1908;
    wchar_t m_0x1B14[270];
    wchar_t m_0x1D30[100];
    BOOL m_0x1DF8;
    DWORD m_0x1DFC;
    BOOL m_bManualTimeSet;
    NaviStr m_naviMediaString;

};

class CMgrSys
{
public:
    static CMgrSys* singleton()
    {
        return *reinterpret_cast<CMgrSys**>(CMgrSys_m_instance);
    }
    void setAccOff(BOOL bAccOff)
    {
        reinterpret_cast<void (*)(CMgrSys*, BOOL)>(CMgrSys_setAccOff)(this, bAccOff);
    }

    void save()
    {
        reinterpret_cast<void (*)(CMgrSys*)>(CMgrSys_save)(this);
    }

    DWORD m_0x00;
    DWORD m_0x04;
    DWORD m_0x08;
    DWORD m_0x0C;
    DWORD m_0x10;
    DWORD m_0x14;
    DWORD m_0x18;
    DWORD m_0x1C;

    struct struc1
    {
        DWORD m_0x00;
        DWORD m_0x04;
        DWORD m_0x08;
        DWORD m_0x0C;
        DWORD m_0x10;
        DWORD m_0x14;
        DWORD m_0x18;
        DWORD m_0x1C;
        DWORD m_0x20;
        DWORD m_0x24;
        DWORD m_0x28;
        DWORD m_0x2C;
        DWORD m_0x30;
        DWORD m_0x34;
        DWORD m_0x38;
        DWORD m_0x3C;
        DWORD m_0x40;
        DWORD m_0x44;
        DWORD m_0x48;
        DWORD m_0x4C;
        DWORD m_0x50;
        DWORD m_0x54;
        DWORD m_0x58;
        DWORD m_0x5C;
    };
    struc1 m_0x20;

    struct struc2
    {
        DWORD m_0x00;
        DWORD m_0x04;
        DWORD m_0x08;
        DWORD m_0x0C;
        DWORD m_0x10;
        DWORD m_0x14;
        DWORD m_0x18;
        DWORD m_0x1C;
        DWORD m_0x20;
        DWORD m_0x24;
        DWORD m_0x28;
        DWORD m_0x2C;
        DWORD m_0x30;
        DWORD m_0x34;
        DWORD m_0x38;
        DWORD m_0x3C;
    };
    struc2 m_0x80;

    // 0xC0
    CNaviManager m_naviManager;
    // ....
};