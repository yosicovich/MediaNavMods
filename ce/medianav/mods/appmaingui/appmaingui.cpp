// appmaingui.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "appmaingui.h"

#include "utils.h"
#include <CmnDLL.h>

wchar_t* CResManager_extImageTable[] = {
    TEXT("")
};

using namespace Utils;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
        DisableThreadLibraryCalls((HMODULE)hModule);
        break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

CEcoCoachingDlg::CEcoCoachingDlg()
:m_testStatic(),
 m_testButton(),
 m_testImage(),
 m_testTextButton()
{
}

void CEcoCoachingDlg::createControls(CGUIEmptyDlg* pPrevDialog)
{
    debugPrintf(DBG, TEXT("CEcoCoachingDlg::createControls(0x%08X)\r\n"), pPrevDialog);
    m_dlgWindow.SetBackgroundImage(459);
    m_pTopBarControl = CTopBar::singleton();
    //addTextControl(m_testStatic, 457, ControlCoords(200, 60, 400, 66), 19, TEXT("TEST TEST"), 0x2A40F8, TEXT_HALIGN_CENTER, 0);
    addButtonControl(m_testButton, 457, ControlCoords(0, 419, 108, 61), 1000, 1);
    m_dlgWindow.addTextButtonControl(m_testTextButton, 1600/*456*/, ControlCoords(109, 419, 691, 61), 1001, 8, CMultiLanguage::singleton()->getMultiLangStr(1009), NULL, ButtonTextFontColorDesc(0xFFFFFF, 0x555555, 0x53595C, 0), 0x825, TRUE);
    m_dlgWindow.addImageControl(m_testImage, 458, ControlCoords(108, 419, 1, 61), 4, TRUE, 0);

    m_dlgWindow.addTextButtonControl(m_radioButton, 1602, ControlCoords(76, 114, 210, 137), 1002, 9, CMultiLanguage::singleton()->getMultiLangStr(1001), &ControlCoords(14, 91, 180, 35), ButtonTextFontColorDesc(0xFFFFFF, 0, 0x46494C, 0xFFFFFF), 0x801, TRUE);
    m_dlgWindow.addTextButtonControl(m_naviButton, 462, ControlCoords(295, 214, 253, 166), 1003, 9, CMultiLanguage::singleton()->getMultiLangStr(1008), &ControlCoords(5, 118, 243, 35), ButtonTextFontColorDesc(0xFFFFFF, 0, 0x46494C, 0), 0x801, FALSE);


    CSettings *pSettings = CSettings::singleton();
    m_screenID = 0;
   
}

void CEcoCoachingDlg::initControls(CGUIEmptyDlg* pPrevDialog)
{
    //m_testStatic.setText(TEXT("TEST TEST"), false);
}

BOOL CEcoCoachingDlg::onBtnClick(DWORD controlID, ButtonClickType clickType)
{
    switch(controlID)
    {
    case 1000:
        AppMain::goBack(2, TRUE);
        //MessageBox(m_hWnd, TEXT("This box is generation within native AppMain button control"), TEXT("Test"), MB_OK);
        return true;
    case 1003:
        {
            reinterpret_cast<CGUIEmptyDlg*(*)()>(0x0002592C)()->changeDialog(this, FALSE);
            return true;
        }
    }
    return FALSE;
}
