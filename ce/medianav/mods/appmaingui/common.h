#pragma once

template<typename T> T* simpleSingleton()
{
    static T* instance = NULL;
    if(instance)
        return instance;
    instance = new T();
    return instance;
}

#define UI_TOPBAR_BACKGROUND_COORDS ControlCoords(0, 0, 800, 102)

#define UI_HOME_BG 45
#define UI_ECO_BG 51

#define UI_4WD_ECO_BTN 462
#define UI_4WD_INFO_BTN 463
#define UI_4WD_SOFTKEY_BACK_BTN 464
#define UI_4WD_SOFTKEY_HOME_BTN 465
#define UI_4WD_INDI_LINE 466


// Text
#define TEXT_1008 1008
#define TEXT_1009 1009
#define TEXT_1010 1010
#define TEXT_1011 1011


// Font
#define FONT_COLOR_WHITE 0xFFFFFF
#define FONT_COLOR_BLACK 0
#define FONT_COLOR_DISABLED_COMMON 0x53595C
#define FONT_COLOR_DISABLED_UI0 0x666666
#define FONT_COLOR_DISABLED_UI1 0x75787A
#define FONT_COLOR_DISABLED_UI2 0x46494C
#define FONT_COLOR_DISABLED_UI3 0xA2A2A2