#include "CDevDlg.h"
#include "CVehiclePerfDlg.h"
#include "CDevMaketDlg.h"

CDevDlg::CDevDlg()
{
    m_screenID = 95;
}

void CDevDlg::createControls(CGUIEmptyDlg* pPrevDialog)
{
    debugPrintf(DBG, TEXT("CDevDlg::createControls(0x%08X)\r\n"), pPrevDialog);
    const CSettings* pSettings = CSettings::singleton();
    
    if(!pSettings->UI_TYPE)
        m_dlgWindow.SetBackgroundImage(UI_HOME_BG);
    else
        m_dlgWindow.SetBackgroundImage(UI_ECO_BG);

    m_pTopBarControl = CTopBar::singleton();
    m_0x2C4 = 0; // ???
    m_rightBtnClickDetectTimout = 1000;
    //m_timer1004Timeout = 500;


    //m_inactivityTimeout = 3000;
    
    /*bool hasAlpha;
    int height;
    int width;
    HBITMAP hb = Utils::LoadPicture(TEXT("\\MD\\VTEST\\knopka\\bouton.bmp"), hasAlpha, width, height);*/
    m_dlgWindow.addImageControl(m_headerLine, UI_4WD_INDI_LINE, UI_TOPBAR_BACKGROUND_COORDS, 1, TRUE, 0);
    addTextControl(m_caption, -1, ControlCoords(29, 10, 400, 35), 6, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1010), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, 0);
    addTextControl(m_captiont, -1, ControlCoords(29, 55, 400, 35), 6, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1010), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, 0x200);

    addProgressBarControl(m_progress, 159, ControlCoords(75, 369, 243, 61), 1010, FALSE);
    m_progress.setButtonFlags(0x400);
    //reinterpret_cast<void (*)(CGUIProgressBarControl&)>(0x001346C8)(m_progress);
    m_progress.m_max = 100;
    m_progress.m_min = 0;
    m_progress.m_current = 30;

/*    if(pSettings->BOOT_LOGO == 0 || pSettings->BOOT_LOGO == 1)
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_4WD_ECO_BTN, ControlCoords(140, 163, 253, 166), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1008), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);
    }else
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_4WD_ECO_BTN, ControlCoords(140, 163, 253, 166), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1011), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);
    }
    m_dlgWindow.addTextButtonControl(m_infoButton, UI_4WD_INFO_BTN, ControlCoords(407, 163, 253, 166), 1002, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1009), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);*/
    static const int btnGap = 10;
    int beginX = (800 - ((UI_SMALL_ICON_WIDTH + btnGap) * 3 - btnGap)) / 2;
    int coordY = 163;
    if(pSettings->BOOT_LOGO == 0 || pSettings->BOOT_LOGO == 1)
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_EVOTECH_4WD_ECO_BTN, ControlCoords(beginX, coordY+30, UI_SMALL_ICON_WIDTH, UI_SMALL_ICON_HEIGHT), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1008), &UI_SMALL_ICON_TEXT_RECT, ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, TRUE);
    }else
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_EVOTECH_4WD_ECO_BTN, ControlCoords(beginX, coordY, UI_SMALL_ICON_WIDTH, UI_SMALL_ICON_HEIGHT), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1011), &UI_SMALL_ICON_TEXT_RECT, ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, TRUE);
    }
    beginX += UI_SMALL_ICON_WIDTH + btnGap;
    m_dlgWindow.addTextButtonControl(m_infoButton, UI_EVOTECH_4WD_INFO_BTN, ControlCoords(beginX, coordY, UI_SMALL_ICON_WIDTH, UI_SMALL_ICON_HEIGHT), 1002, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1009), &UI_SMALL_ICON_TEXT_RECT, ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, TRUE);
    beginX += UI_SMALL_ICON_WIDTH + btnGap;
    m_dlgWindow.addTextButtonControl(m_perfButton, UI_EVOTECH_4WD_PERF_BTN, ControlCoords(beginX, coordY, UI_SMALL_ICON_WIDTH, UI_SMALL_ICON_HEIGHT), 1005, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4000), &UI_SMALL_ICON_TEXT_RECT, ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, TRUE);
    beginX += UI_SMALL_ICON_WIDTH + btnGap;

    m_perfButton.m_bAdvancedClicks = TRUE;

    if(pSettings->HMI_CNF_ECO && pSettings->ROAD_EN)
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_HOME_BTN, ControlCoords(0, 419, 109, 61), 1004, FALSE);
    }else
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_BACK_BTN, ControlCoords(0, 419, 109, 61), 1003, FALSE);
    }

    addSwitchButtonControl(m_testButton, UI_4WD_SOFTKEY_BACK_BTN, ControlCoords(150, 419, 109, 61), 1010, FALSE);

    m_curLangID = CMultiLanguage::singleton()->getLangID();
}

void CDevDlg::initControls(CGUIEmptyDlg* pPrevDialog)
{
    const CSettings* pSettings = CSettings::singleton();
    CMultiLanguage* pMultiLang = CMultiLanguage::singleton();
    if(m_curLangID != pMultiLang->getLangID())
    {
        m_curLangID = pMultiLang->getLangID();
        m_caption.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_1010), FALSE);
        if(pSettings->BOOT_LOGO == 0 || pSettings->BOOT_LOGO == 1)
        {
            m_ecoButton.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_1008), FALSE);
        }else
        {
            m_ecoButton.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_1011), FALSE);
        }
        m_infoButton.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_1009), FALSE);
        m_perfButton.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_4000), FALSE);
    }

    UIType curUIType = CResManagerExt::singleton()->getUIType();
    if(m_curUIType != curUIType)
    {
        m_curUIType = curUIType;
        ButtonTextFontColorDesc fontColors = getUITextColors(m_curUIType);
        m_caption.setFontColor(fontColors.color);
        m_ecoButton.setFontColors(fontColors);
        m_infoButton.setFontColors(fontColors);
        m_perfButton.setFontColors(fontColors);
    }
}

static BOOL xxx = FALSE;
BOOL CDevDlg::onBtnClick(DWORD controlID, ButtonClickType clickType)
{
    debugPrintf(DBG, TEXT("CDevDlg::onBtnClick(%d, %d)\r\n"), controlID, clickType);
    switch(controlID)
    {
    case 1001:
        AppMain::Dialogs::CEcoScoringDlg::singleton()->changeDialog(this, FALSE);
        break;
    case 1002:
        AppMain::Dialogs::C4x4InfoDlg::singleton()->changeDialog(this, FALSE);
        break;
    case 1005:
        //CDevMaketDlg::singleton()->changeDialog(this, FALSE);
        //CVehiclePerfDlg::singleton()->changeDialog(this, FALSE);
        m_progress.m_current++;
        m_progress.draw();
        if(clickType == ButtonClickType_RButtonStillHeld)
        {
            m_infoButton.setEnabled(xxx, TRUE);
            xxx = !xxx;
        }
        //m_ecoButton.moveXY(0, -30);
        //m_ecoButton.setButtonFlags(0x1000);
        //m_ecoButton.refresh();
        break;
    case 1003:
        if(clickType == ButtonClickType_LButtonUp)
            AppMain::goBack(1, FALSE);
        else if(clickType == ButtonClickType_RButtonDown)
            AppMain::goHomeBack(TRUE);
        break;
    case 1004:
        if(clickType == ButtonClickType_LButtonUp)
            AppMain::goHome();
        break;
    }
    return FALSE;
}

void CDevDlg::onInactivityTimeout(void)
{
}
