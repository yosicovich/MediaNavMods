#include "CVehicleDlg.h"

CVehicleDlg::CVehicleDlg()
{
    m_screenID = 95;
}

void CVehicleDlg::createControls(CGUIEmptyDlg* pPrevDialog)
{
    debugPrintf(DBG, TEXT("CVehicleDlg::createControls(0x%08X)\r\n"), pPrevDialog);
    const CSettings* pSettings = CSettings::singleton();
    
    if(!pSettings->UI_TYPE)
        m_dlgWindow.SetBackgroundImage(UI_HOME_BG);
    else
        m_dlgWindow.SetBackgroundImage(UI_ECO_BG);

    m_pTopBarControl = CTopBar::singleton();
    m_0x2C4 = 0; // ???
    
    m_dlgWindow.addImageControl(m_headerLine, UI_4WD_INDI_LINE, UI_TOPBAR_BACKGROUND_COORDS, 1, TRUE, 0, 0, 0);
    addTextControl(m_caption, -1, ControlCoords(29, 10, 400, 35), 6, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1010), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, 0);

    if(pSettings->BOOT_LOGO == 0 || pSettings->BOOT_LOGO == 1)
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_4WD_ECO_BTN, ControlCoords(140, 163, 253, 166), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1008), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);
    }else
    {
        m_dlgWindow.addTextButtonControl(m_ecoButton, UI_4WD_ECO_BTN, ControlCoords(140, 163, 253, 166), 1001, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1011), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);
    }
    m_dlgWindow.addTextButtonControl(m_infoButton, UI_4WD_INFO_BTN, ControlCoords(407, 163, 253, 166), 1002, 9, CMultiLanguage::singleton()->getMultiLangStr(TEXT_1009), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(FONT_COLOR_WHITE, FONT_COLOR_BLACK, FONT_COLOR_DISABLED_COMMON, FONT_COLOR_BLACK), TEXT_UNKNOWN_FLAG | TEXT_HALIGN_CENTER, FALSE);
    m_ecoButton.setButtonFlags(BUTTON_FLAG_0x200);
    m_infoButton.setButtonFlags(BUTTON_FLAG_0x200);

    if(pSettings->HMI_CNF_ECO && pSettings->ROAD_EN)
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_HOME_BTN, ControlCoords(0, 419, 109, 61), 1004, 0);
    }else
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_BACK_BTN, ControlCoords(0, 419, 109, 61), 1003, 0);
    }

    m_curLangID = CMultiLanguage::singleton()->getLangID();
}

void CVehicleDlg::initControls(CGUIEmptyDlg* pPrevDialog)
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
    }

    UIType curUIType = CResManagerExt::singleton()->getUIType();
    if(m_curUIType != curUIType)
    {
        m_curUIType = curUIType;
        ButtonTextFontColorDesc fontColors = getUITextColors(m_curUIType);
        m_caption.setFontColor(fontColors.color);
        m_ecoButton.setFontColors(fontColors);
        m_infoButton.setFontColors(fontColors);
    }
}

bool CVehicleDlg::onBtnClick(DWORD controlID, ButtonClickType clickType)
{
    switch(controlID)
    {
    case 1001:
        AppMain::Dialogs::CEcoScoringDlg::singleton()->changeDialog(this, FALSE);
    case 1002:
        AppMain::Dialogs::C4x4InfoDlg::singleton()->changeDialog(this, FALSE);
    case 1003:
        if(clickType == ButtonClickType_2)
            AppMain::goBack(1, FALSE);
        else if(clickType == ButtonClickType_3)
            AppMain::goHomeBack(TRUE);
    case 1004:
        if(clickType == ButtonClickType_2)
            AppMain::goHome();
    }
    return false;
}
