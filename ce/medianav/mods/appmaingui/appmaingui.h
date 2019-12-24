#pragma once

#include "stdafx.h"
#include "glue.h"
#include "glue_other.h"
#include "utils.h"
#include <SimpleIni.h>

// 0x2DC in total
template<typename T> class CGUIFixedEmptyDlg: public CGUIEmptyDlg
{
public:
    static T* singleton()
    {
        if(m_instance)
            return m_instance;
        m_instance = new T();
        return m_instance;
    }

    CGUIFixedEmptyDlg()
        :m_curLangID(-1)
        ,m_curUIType(static_cast<UIType>(-1))
    {

    }
    virtual HWND createWindow(HINSTANCE hInstance, HWND parentWindow, CGUIEmptyDlg* pPrevDialog, BOOL bShow)
    {
        const WNDCLASS* wndClass = registerClass(hInstance, CGUIFixedEmptyDlg<T>::windowProc);
        m_hInstance = hInstance;

        m_hWnd = CreateWindowEx(WS_EX_NOANIMATION, wndClass->lpszClassName, wndClass->lpszClassName,  WS_POPUP | WS_VISIBLE, 0, 0, 800, 480, parentWindow, NULL, hInstance, NULL);

        if(!m_hWnd)
            return NULL;

        CGUIEmptyDlg* pActiveDialog = getActiveDialog();
        if(pActiveDialog)
            pActiveDialog->onBeforeHide();

        createDialog(pPrevDialog, bShow);

        return m_hWnd;
    }
protected:
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        debugPrintf(DBG, TEXT("CGUIFixedEmptyDlg::windowProc() uMsg = 0x%04X\r\n"), uMsg);
        return m_instance->wndProc(hWnd, uMsg, wParam, lParam);
    }
    int m_curLangID;
    UIType m_curUIType;
private:
    static T* m_instance;
};
template<typename T> T* CGUIFixedEmptyDlg<T>::m_instance = NULL;


class CEcoCoachingDlg : public CGUIFixedEmptyDlg<CEcoCoachingDlg>
{
public:
    CEcoCoachingDlg();
    virtual void createControls(CGUIEmptyDlg* pPrevDialog);
    virtual void initControls(CGUIEmptyDlg* pPrevDialog);
    virtual BOOL onBtnClick(DWORD controlID, ButtonClickType clickType);
private:
    CGUITextControl m_testStatic;
    CGUIButtonControl m_testButton;
    CGUIImageControl m_testImage;
    CGUITextButtonControl m_testTextButton;

    CGUITextButtonControl m_radioButton;
    CGUITextButtonControl m_naviButton;

};
template class CGUIFixedEmptyDlg<CEcoCoachingDlg>;

namespace AppMain
{
    inline void exitToDesktop()
    {
        CMgrSys::singleton()->setAccOff(TRUE);
        CMgrSys::singleton()->save();
        PostMessage(CSettings::singleton()->m_appMainHWND, WM_CLOSE, 0, 0);
        CreateProcess(TEXT("\\windows\\explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
    }
}