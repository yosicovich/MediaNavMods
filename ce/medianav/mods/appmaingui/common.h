#pragma once
#include "stdafx.h"
#include <utils.h>
#include <SimpleIni.h>

template<typename T> T* simpleSingleton()
{
    static T* instance = NULL;
    if(instance)
        return instance;
    instance = new T();
    return instance;
}

#define UI_TOPBAR_BACKGROUND_COORDS ControlCoords(0, 0, 800, 102)

#define UI_SMALL_ICON_WIDTH 210
#define UI_SMALL_ICON_HEIGHT 137
#define UI_SMALL_ICON_TEXT_RECT ControlCoords(13, 91, 184, 35)

#define UI_LARGE_ICON_WIDTH 253
#define UI_LARGE_ICON_HEIGHT 166
#define UI_LARGE_ICON_TEXT_RECT ControlCoords(5, 118, 243, 35)

#define UI_HOME_BG 45
#define UI_ECO_BG 51

#define UI_ECO_RESET_BTN 426
#define UI_ECO_DOT 427

#define UI_4WD_ECO_BTN 462
#define UI_4WD_INFO_BTN 463
#define UI_4WD_SOFTKEY_BACK_BTN 464
#define UI_4WD_SOFTKEY_HOME_BTN 465
#define UI_4WD_INDI_LINE 466

#define UI_EVOTECH_4WD_ECO_BTN 1600
#define UI_EVOTECH_4WD_INFO_BTN 1601
#define UI_EVOTECH_4WD_PERF_BTN 1602
#define UI_EVOTECH_4WD_RST_BTN 1604

#define UI_EVOTECH_4WD_PERF_BKG 1603

// Text
#define TEXT_1008 1008
#define TEXT_1009 1009
#define TEXT_1010 1010
#define TEXT_1011 1011

#define TEXT_KM_H_2006 2006

#define TEXT_4000 4000
#define TEXT_4001 4001
#define TEXT_4002 4002


// Font
#define FONT_COLOR_WHITE 0xFFFFFF
#define FONT_COLOR_BLACK 0
#define FONT_COLOR_DISABLED_COMMON 0x53595C
#define FONT_COLOR_DISABLED_UI0 0x666666
#define FONT_COLOR_DISABLED_UI1 0x75787A
#define FONT_COLOR_DISABLED_UI2 0x46494C
#define FONT_COLOR_DISABLED_UI3 0xA2A2A2
#define FONT_COLOR_ECO_GREEN 0x009F52

#define FONT_COLOR_PERF_VAL 0x18FFCE


#ifdef DEV_MAKET
class CDevMaket
{
public:
    template<typename T> friend  T* simpleSingleton();
    static CDevMaket* singleton()
    {
        return simpleSingleton<CDevMaket>();
    }

    const wchar_t* getImagePath(DWORD imageID);
    const wchar_t* getMultiLangStr(DWORD strID);
private:
    CDevMaket();

    CSimpleIni m_iniConfig;
    int m_codePage;

};
#endif