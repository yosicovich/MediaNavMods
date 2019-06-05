#include "CDevMaketDlg.h"

CDevMaketDlg::CDevMaketDlg()
{
    m_iniConfig.LoadFile((Utils::getModulePath(NULL) + TEXT("\\CDevMaketDlg.ini")).c_str());
    m_screenID = m_iniConfig.GetLongValue(TEXT("CDevMaketDlg"), TEXT("ScreenID"), 200);
}

CDevMaketDlg::~CDevMaketDlg()
{
    for(size_t i = 0; i< m_imageControls.size(); ++i)
    {
        delete m_imageControls[i];
    }
    m_imageControls.clear();

    for(size_t i = 0; i< m_textControls.size(); ++i)
    {
        delete m_textControls[i];
    }
    m_textControls.clear();

    for(size_t i = 0; i< m_buttonControls.size(); ++i)
    {
        delete m_buttonControls[i];
    }
    m_buttonControls.clear();

    for(size_t i = 0; i< m_textButtonControls.size(); ++i)
    {
        delete m_textButtonControls[i];
    }
    m_textButtonControls.clear();
}

void CDevMaketDlg::createControls(CGUIEmptyDlg* pPrevDialog)
{
    debugPrintf(DBG, TEXT("CDevMaketDlg::createControls(0x%08X)\r\n"), pPrevDialog);
    const CSettings* pSettings = CSettings::singleton();
    
    std::wostringstream strStream;
    strStream << TEXT("BackgroundImage_") << pSettings->UI_TYPE;
    
    m_dlgWindow.SetBackgroundImage(m_iniConfig.GetLongValue(TEXT("CDevMaketDlg"), strStream.str().c_str(), UI_ECO_BG));
    
    if(m_iniConfig.GetLongValue(TEXT("CDevMaketDlg"), TEXT("HasTopBar"), true))
    {
        m_pTopBarControl = CTopBar::singleton();
    }
    m_0x2C4 = 0; // ???
    
    for(int i = 0; createImageControl(i); ++i)
        ;
    for(int i = 0; createTextControl(i); ++i)
        ;
    for(int i = 0; createButtonControl(i); ++i)
        ;
    for(int i = 0; createTextButtonControl(i); ++i)
        ;

    m_curLangID = CMultiLanguage::singleton()->getLangID();

    debugPrintf(DBG, m_codeStream.str().c_str());
}

void CDevMaketDlg::initControls(CGUIEmptyDlg* pPrevDialog)
{
    const CSettings* pSettings = CSettings::singleton();
    CMultiLanguage* pMultiLang = CMultiLanguage::singleton();
    if(m_curLangID != pMultiLang->getLangID())
    {
        m_curLangID = pMultiLang->getLangID();
        for(size_t i = 0; i< m_textControls.size(); ++i)
        {
            std::wostringstream strStream;
            strStream << TEXT("CGUITextControl_") << i;
            std::wstring ctlSection = strStream.str();
            int strID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("strID"), -1);
            if(strID != -1)
                m_textControls[i]->setText(CMultiLanguage::singleton()->getMultiLangStr(strID), FALSE);
        }
        for(size_t i = 0; i< m_textButtonControls.size(); ++i)
        {
            std::wostringstream strStream;
            strStream << TEXT("CGUITextButtonControl_") << i;
            std::wstring ctlSection = strStream.str();
            int strID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("strID"), -1);
            if(strID != -1)
                m_textButtonControls[i]->setText(CMultiLanguage::singleton()->getMultiLangStr(strID), FALSE);
        }
    }

    UIType curUIType = CResManagerExt::singleton()->getUIType();
    if(m_curUIType != curUIType)
    {
        m_curUIType = curUIType;
        /*ButtonTextFontColorDesc fontColors = getUITextColors(m_curUIType);
        m_caption.setFontColor(fontColors.color);
        for(int i = 0; i<3; i++)
        {
        }*/
    }
}

BOOL CDevMaketDlg::onBtnClick(DWORD controlID, ButtonClickType clickType)
{
    switch(controlID)
    {
/*    case 1001:
        AppMain::Dialogs::CEcoScoringDlg::singleton()->changeDialog(this, FALSE);
        break;
    case 1002:
        AppMain::Dialogs::C4x4InfoDlg::singleton()->changeDialog(this, FALSE);
        break;
    case 1005:
        AppMain::Dialogs::C4x4InfoDlg::singleton()->changeDialog(this, FALSE);*/
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
    case 1003:
        AppMain::exitToDesktop();
        break;
    }
    return FALSE;
}

bool CDevMaketDlg::createImageControl(int idx)
{
    std::wostringstream strStream;
    strStream << TEXT("CGUIImageControl_") << idx;
    std::wstring ctlSection = strStream.str();
    if(m_iniConfig.GetSectionSize(ctlSection.c_str()) == -1)
        return false;
    int x,y,width,height,imageID;
    x = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("x"), -1);
    y = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("y"), -1);
    width = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("width"), -1);
    height = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("height"), -1);
    imageID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("imageID"), -1);

    if(x == -1 || y == -1 || width == -1 || height == -1 || imageID == -1)
    {
        debugPrintf(DBG, TEXT("CDevMaketDlg::createImageControl: %s is invalid, x=%d, y=%d, width=%d, height=%d, imageID=%d\r\n"), ctlSection.c_str(), x, y, width, height, imageID);
        return true;
    }
    int flags = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("flags"), 1);
    bool transparent = m_iniConfig.GetBoolValue(ctlSection.c_str(), TEXT("transparent"), true);
    CGUIImageControl* pImage = new CGUIImageControl();
    m_imageControls.push_back(pImage);
    m_dlgWindow.addImageControl(*pImage, imageID, ControlCoords(x, y, width, height), flags, transparent ? TRUE : FALSE, FALSE);
    Utils::oswprintf(m_codeStream, TEXT("m_dlgWindow.addImageControl(m_%s, %d, ControlCoords(%d, %d, %d, %d), 0x%04X, %s, FALSE);\r\n"), ctlSection.c_str(), imageID, x, y, width, height, flags, transparent ? TEXT("TRUE") : TEXT("FALSE"));
    if(transparent)
        Utils::oswprintf(m_codeStream, TEXT("m_%s.setRedrawBackground(true);\r\n"), ctlSection.c_str());
    return true;
}

bool CDevMaketDlg::createTextControl(int idx)
{
    std::wostringstream strStream;
    strStream << TEXT("CGUITextControl_") << idx;
    std::wstring ctlSection = strStream.str();
    if(m_iniConfig.GetSectionSize(ctlSection.c_str()) == -1)
        return false;
    int x,y,width,height,strID, fontSize, fontColor;
    x = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("x"), -1);
    y = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("y"), -1);
    width = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("width"), -1);
    height = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("height"), -1);
    strID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("strID"), -1);
    fontSize = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("fontSize"), -1);
    fontColor = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("fontColor"), -1);

    if(x == -1 || y == -1 || width == -1 || height == -1 || strID == -1 || fontSize == -1 || fontColor == -1)
    {
        debugPrintf(DBG, TEXT("CDevMaketDlg::createTextControl: %s is invalid, x=%d, y=%d, width=%d, height=%d, strID=%d, fontSize=%d, fontColor=%d\r\n"), ctlSection.c_str(), x, y, width, height, strID, fontSize, fontColor);
        return true;
    }
    int flags = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("flags"), TEXT_UNKNOWN_FLAG);
    int imageID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("imageID"), -1);
    int transparent = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("transparent"), false);
    CGUITextControl* pText = new CGUITextControl();
    m_textControls.push_back(pText);
    addTextControl(*pText, imageID, ControlCoords(x, y, width, height), fontSize, CMultiLanguage::singleton()->getMultiLangStr(strID), fontColor, flags, transparent ? TRUE : FALSE);
    if(transparent)
        pText->setRedrawBackground(true);
    Utils::oswprintf(m_codeStream, TEXT("addTextControl(m_%s, %d, ControlCoords(%d, %d, %d, %d), %d, CMultiLanguage::singleton()->getMultiLangStr(%d), 0x%06X, 0x%04X, %s);\r\n"), ctlSection.c_str(), imageID, x, y, width, height, fontSize, strID, fontColor, flags, transparent ? TEXT("TRUE") : TEXT("FALSE"));
    if(transparent)
        Utils::oswprintf(m_codeStream, TEXT("m_%s.setRedrawBackground(true);\r\n"), ctlSection.c_str());
    return true;
}

bool CDevMaketDlg::createButtonControl(int idx)
{
    std::wostringstream strStream;
    strStream << TEXT("CGUIButtonControl_") << idx;
    std::wstring ctlSection = strStream.str();
    if(m_iniConfig.GetSectionSize(ctlSection.c_str()) == -1)
        return false;
    int x,y,width,height,imageID;
    x = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("x"), -1);
    y = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("y"), -1);
    width = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("width"), -1);
    height = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("height"), -1);
    imageID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("imageID"), -1);

    if(x == -1 || y == -1 || width == -1 || height == -1 || imageID == -1)
    {
        debugPrintf(DBG, TEXT("CDevMaketDlg::createButtonControl: %s is invalid, x=%d, y=%d, width=%d, height=%d, imageID=%d\r\n"), ctlSection.c_str(), x, y, width, height, imageID);
        return true;
    }
    int eventID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("eventID"), 10000);
    int transparent = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("transparent"), true);
    CGUIButtonControl* pBtn = new CGUIButtonControl();
    m_buttonControls.push_back(pBtn);
    addButtonControl(*pBtn, imageID, ControlCoords(x, y, width, height), eventID, transparent ? TRUE : FALSE);
    Utils::oswprintf(m_codeStream, TEXT("addButtonControl(m_%s, %d, ControlCoords(%d, %d, %d, %d), %d, %s);\r\n"), ctlSection.c_str(), imageID, x, y, width, height, eventID, transparent ? TEXT("TRUE") : TEXT("FALSE"));
    if(transparent)
        Utils::oswprintf(m_codeStream, TEXT("m_%s.setRedrawBackground(true);\r\n"), ctlSection.c_str());
    return true;
}

bool CDevMaketDlg::createTextButtonControl(int idx)
{
    std::wostringstream strStream;
    strStream << TEXT("CGUITextButtonControl_") << idx;
    std::wstring ctlSection = strStream.str();
    if(m_iniConfig.GetSectionSize(ctlSection.c_str()) == -1)
        return false;
    int x,y,width,height,strID, fontSize;
    x = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("x"), -1);
    y = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("y"), -1);
    width = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("width"), -1);
    height = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("height"), -1);
    strID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("strID"), -1);
    fontSize = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("fontSize"), -1);
    int imageID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("imageID"), -1);

    if(x == -1 || y == -1 || width == -1 || height == -1 || strID == -1 || fontSize == -1 || imageID == -1)
    {
        debugPrintf(DBG, TEXT("CDevMaketDlg::createTextButtonControl: %s is invalid, x=%d, y=%d, width=%d, height=%d, strID=%d, fontSize=%d, imageID=%d\r\n"), ctlSection.c_str(), x, y, width, height, strID, fontSize, imageID);
        return true;
    }
    int textFlags = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("textFlags"), TEXT_UNKNOWN_FLAG);
    int transparent = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("transparent"), false);
    int eventID = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("eventID"), 10000);
    ControlCoords* pTextCoords;
    ControlCoords textCoords;
    textCoords.x = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("textX"), -1);
    textCoords.y = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("textY"), -1);
    textCoords.width = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("textWidth"), -1);
    textCoords.height = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("textHeight"), -1);
    
    if(textCoords.x == textCoords.y && textCoords.width == textCoords.height && textCoords.x == textCoords.width && textCoords.x == -1)
    {
        pTextCoords = NULL;
    }else
    {
        if(textCoords.x == -1 || textCoords.y == -1 || textCoords.width == -1 || textCoords.height == -1)
        {
            debugPrintf(DBG, TEXT("CDevMaketDlg::createTextButtonControl: %s is invalid, textX=%d, textY=%d, textWidth=%d, textHeight=%d\r\n"), ctlSection.c_str(), textCoords.x, textCoords.y, textCoords.width, textCoords.height);
            return true;
        }
        pTextCoords = &textCoords;
    }
    ButtonTextFontColorDesc colors;
    colors.color = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("fontColor"), -1);
    colors.selectedColor = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("selectedColor"), -1);
    colors.disabledColor = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("disabledColor"), -1);
    colors.color2 = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("fontColor2"), -1);
    if(colors.color == -1 || colors.selectedColor == -1 || colors.disabledColor == -1 || colors.color2 == -1)
    {
        debugPrintf(DBG, TEXT("CDevMaketDlg::createTextButtonControl: %s is invalid, fontColor=%d, selectedColor=%d, disabledColor=%d, fontColor2=%d\r\n"), ctlSection.c_str(), colors.color, colors.selectedColor, colors.disabledColor, colors.color2);
        return true;
    }

    CGUITextButtonControl* pTextBtn = new CGUITextButtonControl();
    m_textButtonControls.push_back(pTextBtn);
    m_dlgWindow.addTextButtonControl(*pTextBtn, imageID, ControlCoords(x, y, width, height), eventID, fontSize, CMultiLanguage::singleton()->getMultiLangStr(strID), pTextCoords, colors, textFlags, transparent ? TRUE : FALSE);
    /*int buttonFlags = m_iniConfig.GetLongValue(ctlSection.c_str(), TEXT("buttonFlags"), -1);
    if(buttonFlags != -1)
        pTextBtn->setButtonFlags(buttonFlags);*/
    {
        std::wostringstream strStream;
        if(pTextCoords)
        {
            strStream << TEXT("&ControlCoords(") << pTextCoords->x << TEXT(", ") << pTextCoords->y << TEXT(", ") << pTextCoords->width << TEXT(", ") << pTextCoords->height << TEXT(")");
        }else
        {
            strStream << TEXT("NULL");
        }
        strStream << TEXT("CGUITextButtonControl_") << idx;
        Utils::oswprintf(m_codeStream, TEXT("m_dlgWindow.addTextButtonControl(m_%s, %d, ControlCoords(%d, %d, %d, %d), %d, %d, CMultiLanguage::singleton()->getMultiLangStr(%d), %s, ButtonTextFontColorDesc(0x%06X, 0x%06X, 0x%06X, 0x%06X), 0x%04X, %s);\r\n"), ctlSection.c_str(), imageID, x, y, width, height, eventID, fontSize, strID, strStream.str().c_str(),colors.color, colors.selectedColor, colors.disabledColor, colors.color2, textFlags, transparent ? TEXT("TRUE") : TEXT("FALSE"));
        if(transparent)
            Utils::oswprintf(m_codeStream, TEXT("m_%s.setRedrawBackground(true);\r\n"), ctlSection.c_str());
    }
    return true;
}
