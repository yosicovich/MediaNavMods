#include "CVehiclePerfDlg.h"

const wchar_t* CVehiclePerfDlg::cUndefResult = TEXT("--.-");

CVehiclePerfDlg::CVehiclePerfDlg()
    :m_dwStartTime(0)
    ,m_prevSpeed(0)
    ,m_b60Got(false)
    ,m_b80Got(false)
    ,m_b100Got(false)
    ,m_speedDisplayCounter(0)
    ,m_measureFlashCounter(0)
{
    m_screenID = 200;
}

void CVehiclePerfDlg::createControls(CGUIEmptyDlg* pPrevDialog)
{
    debugPrintf(DBG, TEXT("CVehiclePerfDlg::createControls(0x%08X)\r\n"), pPrevDialog);
    const CSettings* pSettings = CSettings::singleton();
    
    m_dlgWindow.SetBackgroundImage(UI_EVOTECH_4WD_PERF_BKG);

    m_pTopBarControl = CTopBar::singleton();
    m_0x2C4 = 0; // ???
    
    m_dlgWindow.addImageControl(m_headerLine, UI_4WD_INDI_LINE, UI_TOPBAR_BACKGROUND_COORDS, 1, TRUE, FALSE);
    addTextControl(m_caption, -1, ControlCoords(29, 10, 400, 35), 6, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4000), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_LEFT, 0);


    addTextControl(m_captions[0], -1, ControlCoords(130, 143, 80, 35), 10, TEXT("0-100"), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, FALSE);
    addTextControl(m_captions[1], -1, ControlCoords(69, 228, 80, 35), 10, TEXT("0-80"), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, FALSE);
    addTextControl(m_captions[2], -1, ControlCoords(129, 313, 80, 35), 10, TEXT("0-60"), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG, FALSE);
    addTextControl(m_captions[3], -1, ControlCoords(410, 265, 40, 35), 2, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4001), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_LEFT, FALSE);

    addTextControl(m_captions[4], -1, ControlCoords(660, 313, 20, 35), 10, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4002), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_LEFT, FALSE);
    addTextControl(m_captions[6], -1, ControlCoords(715, 226, 20, 35), 10, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4002), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_LEFT, FALSE);
    addTextControl(m_captions[5], -1, ControlCoords(660, 143, 20, 35), 10, CMultiLanguage::singleton()->getMultiLangStr(TEXT_4002), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_LEFT, FALSE);
    

    addTextControl(m_values[0], -1, ControlCoords(600, 143, 59, 35), 10, cUndefResult, FONT_COLOR_PERF_VAL, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_RIGHT, FALSE);
    m_values[0].setRedrawBackground(true);
    addTextControl(m_values[1], -1, ControlCoords(655, 226, 59, 35), 10, cUndefResult, FONT_COLOR_PERF_VAL, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_RIGHT, FALSE);
    m_values[1].setRedrawBackground(true);
    addTextControl(m_values[2], -1, ControlCoords(600, 313, 59, 35), 10, cUndefResult, FONT_COLOR_PERF_VAL, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_RIGHT, FALSE);
    m_values[2].setRedrawBackground(true);

    addTextControl(m_time, -1, ControlCoords(315, 110, 120, 50), 15, TEXT("00.0"), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_RIGHT | TEXT_VALIGN_BOTTOM, FALSE);
    m_time.setRedrawBackground(true);
    addTextControl(m_speed, -1, ControlCoords(355, 210, 100, 55), 21, TEXT("0"), FONT_COLOR_WHITE, TEXT_UNKNOWN_FLAG | TEXT_HALIGN_RIGHT | TEXT_VALIGN_BOTTOM, FALSE);
    m_speed.setRedrawBackground(true);
   
    addButtonControl(m_resetButton, UI_EVOTECH_4WD_RST_BTN, ControlCoords(350, 355, 100, 55), 1003, FALSE);

    if(pSettings->HMI_CNF_ECO || pSettings->ROAD_EN)
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_BACK_BTN, ControlCoords(0, 419, 109, 61), 1001, FALSE);
    }else
    {
        addButtonControl(m_backButton, UI_4WD_SOFTKEY_HOME_BTN, ControlCoords(0, 419, 109, 61), 1002, FALSE);
    }

    m_curLangID = CMultiLanguage::singleton()->getLangID();

}

void CVehiclePerfDlg::initControls(CGUIEmptyDlg* pPrevDialog)
{
    const CSettings* pSettings = CSettings::singleton();
    CMultiLanguage* pMultiLang = CMultiLanguage::singleton();
    if(m_curLangID != pMultiLang->getLangID())
    {
        m_curLangID = pMultiLang->getLangID();
        m_caption.setText(CMultiLanguage::singleton()->getMultiLangStr(TEXT_4000), FALSE);
    }

    UIType curUIType = CResManagerExt::singleton()->getUIType();
    if(m_curUIType != curUIType)
    {
        m_curUIType = curUIType;
        ButtonTextFontColorDesc fontColors = getUITextColors(m_curUIType);
        m_caption.setFontColor(fontColors.color);
        for(int i = 0; i<7; i++)
        {
            m_captions[i].setFontColor(fontColors.color);
        }
    }

    if(!m_b60Got && !m_b80Got && !m_b100Got)
        startMeasure();
}

BOOL CVehiclePerfDlg::onBtnClick(DWORD controlID, ButtonClickType clickType)
{
    switch(controlID)
    {
    case 1003:
        startMeasure();
        break;
    case 1001:
        if(clickType == ButtonClickType_LButtonUp)
            AppMain::goBack(1, FALSE);
        else if(clickType == ButtonClickType_RButtonDown)
            AppMain::goHomeBack(TRUE);
        break;
    case 1002:
        if(clickType == ButtonClickType_LButtonUp)
            AppMain::goHome();
        break;
    }
    return FALSE;
}

BOOL CVehiclePerfDlg::onExit()
{
    stopMeasure();
    return TRUE;
}

static const int cStartDetectEdge = 55;// 2 km/h
#define SPEED_EMU
BOOL CVehiclePerfDlg::onTimer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg != WM_TIMER )
        return FALSE;

    if(wParam == cMeasureTimerID)
    {
#ifdef SPEED_EMU
        static int speed;
        if(m_dwStartTime == 0)
            speed = 55;
        else
            speed += 28;
#else
        int speed;
        speed = CSettings::m_pMcmShr->m_speedCm_s;
#endif
        
        if(speed < cStartDetectEdge)
            return TRUE;

        if(m_dwStartTime == 0)
        {
            m_dwStartTime = GetTickCount();
            m_prevSpeed = speed;
        }

        if(speed < m_prevSpeed)
        {
            stopMeasure();
            return TRUE;// Slow down condition. The measurement is definitely over.
        }

        m_prevSpeed = speed;
        DWORD dTimeElapsed = GetTickCount() - m_dwStartTime;
        double timeElapsed = static_cast<double>(dTimeElapsed) /1000;
         
        // Update speed/time here
        m_time.setText(timeElapsed, 1, TRUE);
        if(++m_speedDisplayCounter == cSpeedDisplayCount)
        {
            setSpeedText(speed);
            m_speedDisplayCounter = 0;
        }

        if(!m_b60Got && speed >= 1667) //60km/h
        {
            m_b60Got = true;
            m_values[2].setVisible(true, FALSE);
            m_values[2].setText(timeElapsed, 1, TRUE);
        }
        if(!m_b80Got && speed >= 2222) //80km/h
        {
            m_b80Got = true;
            m_values[1].setVisible(true, FALSE);
            m_values[1].setText(timeElapsed, 1, TRUE);
        }
        if(!m_b100Got && speed >= 2778) //100km/h
        {
            m_b100Got = true;
            m_values[0].setVisible(true, FALSE);
            m_values[0].setText(timeElapsed, 1, TRUE);
            setSpeedText(speed);
            stopMeasure();
        }

        if(++m_measureFlashCounter == cMaesureFlashCount)
        {
            if(!m_b60Got)
            {
                m_values[2].setVisible(!m_values[2].getVisible(), TRUE);
            }
            if(!m_b80Got)
            {
                m_values[1].setVisible(!m_values[1].getVisible(), TRUE);
            }
            if(!m_b100Got)
            {
                m_values[0].setVisible(!m_values[0].getVisible(), TRUE);
            }
            m_measureFlashCounter = 0;
        }

        return TRUE;
    }

    return FALSE;
}

void CVehiclePerfDlg::startMeasure()
{
    m_dwStartTime = 0;
    m_prevSpeed = 0;

    m_b60Got = false;
    m_b80Got = false;
    m_b100Got = false;
    m_speedDisplayCounter = 0;
    m_measureFlashCounter = 0;

    m_resetButton.setEnabled(FALSE, TRUE);
    
    for(int i = 0; i < 3; ++i)
    {
        m_values[i].setText(cUndefResult, TRUE);
    }

    if(!SetTimer(m_hWnd, cMeasureTimerID, 100, NULL))
    {
        debugPrintf(DBG, TEXT("CVehiclePerfDlg::startMeasureTimer(): Failed to create timer\r\n"));
    }
}

void CVehiclePerfDlg::stopMeasure()
{
    KillTimer(m_hWnd, cMeasureTimerID);
    m_resetButton.setEnabled(TRUE, TRUE);
    m_values[2].setVisible(true, TRUE);
    m_values[1].setVisible(true, TRUE);
    m_values[0].setVisible(true, TRUE);
}

void CVehiclePerfDlg::setSpeedText(DWORD speed)
{
    int speedKmh = ((speed + 1) * 36) / 1000;
    m_speed.setText(speedKmh, TRUE);
}
