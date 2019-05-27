
#pragma once
#include "stdafx.h"
#include <utils.h>
#include <wingdi.h>
#include <windowsx.h>
#include <CmnDLL.h>
#include "common.h"
#include "glue_common.h"

#define CGUIEmptyDlg_CGUIEmptyDlg 0x0012A8A8
#define CGUIEmptyDlg_destructor_CGUIEmptyDlg 0x0012A9CC
#define CGUIEmptyDlg_addTextControl 0x0012BC00
#define CGUIEmptyDlg_addButtonControl 0x0012BB64
#define CGUIEmptyDlg__registerClass 0x0012D1F0
#define CGUIEmptyDlg__wndProc 0x0012AD2C
#define CGUIEmptyDlg__getActiveDialog 0x00011000
#define CGUIEmptyDlg__createDialog 0x0012B270
#define CGUIEmptyDlg_changeDialog 0x0012C3D4

#define CGUIDlgWindow_SetBackgroundImage 0x00131988
#define CGUIDlgWindow__addImageControl 0x00131C58
#define CGUIDlgWindow_addTextButtonControl 0x00131D1C

#define CGUIImageControl_CGUIImageControl 0x0012E5BC

#define CGUITextControl_CGUITextControl 0x0012F534
#define CGUITextControl_setText 0x0012F788

#define CGUIButtonControl_CGUIButtonControl 0x00133048

#define CGUITextButtonControl_CGUITextButtonControl 0x001338AC

#define CTopBar_singleton 0x00014A44

#define AppMain_goHome 0x000E1FFC
#define AppMain_goHomeBack 0x0012CFE0
#define AppMain_goBack 0x0012C898

// Dialogs
#define CEcoScoringDlg__singleton 0x0001ED34
#define C4x4InfoDlg__singleton 0x0001D5D4


// Text
#define TEXT_HALIGN_LEFT 0x00
#define TEXT_HALIGN_CENTER 0x01
#define TEXT_HALIGN_RIGHT 0x02

#define TEXT_VALIGN_BOTTOM 0x04
#define TEXT_VALIGN_BOTTOM_MOST 0x08

#define TEXT_UNKNOWN_FLAG1 0x20
#define TEXT_FIRST_CHAR_UNDERSCORE_MOST 0x80

#define TEXT_SHOW_HIDDEN 0x400
#define TEXT_UNKNOWN_FLAG 0x800

// Button flags
#define BUTTON_FLAG_0x40 0x40
#define BUTTON_FLAG_0x80 0x80
#define BUTTON_FLAG_0x100 0x100
#define BUTTON_FLAG_0x200 0x200
#define BUTTON_FLAG_0x400 0x400

enum ButtonClickType
{
    ButtonClickType_2 = 2,
    ButtonClickType_3 = 3,
    ButtonClickType_4 = 4,
    ButtonClickType_5 = 5
};

struct MousePoint
{
    DWORD x;
    DWORD y;
};
struct ControlCoords
{
    ControlCoords(DWORD x, DWORD y, DWORD width, DWORD height)
        :x(x), y(y), width(width), height(height)
    {

    }
    DWORD x;
    DWORD y;
    DWORD width;
    DWORD height;
};

struct ButtonTextFontColorDesc
{
    ButtonTextFontColorDesc()
        :color(FONT_COLOR_WHITE), selectedColor(FONT_COLOR_BLACK), disabledColor(FONT_COLOR_DISABLED_COMMON), color2(FONT_COLOR_BLACK)
    {

    }
    ButtonTextFontColorDesc(DWORD color, DWORD selectedColor, DWORD disabledColor, DWORD color2)
        :color(color), selectedColor(selectedColor),disabledColor(disabledColor), color2(color2)
    {

    };
    DWORD color;
    DWORD selectedColor;
    DWORD disabledColor;
    DWORD color2;
};

class CTopBar
{
public:
    static CTopBar* singleton()
    {
        return reinterpret_cast<CTopBar*(*)()>(CTopBar_singleton)();
    };

    BYTE m_vars[0xA78];
};

class CGUIImageControl
{
public:
    CGUIImageControl()
    {
        debugPrintf(DBG, TEXT("CGUIImageControl::CGUIImageControl()\r\n"));
        reinterpret_cast<void(*)(CGUIImageControl*)>(CGUIImageControl_CGUIImageControl)(this);
    }

    virtual void func(){};

    BYTE m_vars[0x54];
};

class CGUITextControl
{
public:
    CGUITextControl()
    {
        debugPrintf(DBG, TEXT("CGUITextControl::CGUITextControl()\r\n"));
        reinterpret_cast<void(*)(CGUITextControl*)>(CGUITextControl_CGUITextControl)(this);
    }

    void setText(const wchar_t* text, BOOL bRefresh)
    {
        reinterpret_cast<void(*)(CGUITextControl*, const wchar_t*, BOOL)>(CGUITextControl_setText)(this, text, bRefresh);
    }

    inline void setFontColor(DWORD fontColor)
    {
        *reinterpret_cast<DWORD*>(&m_vars[0x60]) = fontColor;
    }

    virtual void func(){};

    BYTE m_vars[0x548];
};

class CGUIButtonControl
{
public:
    CGUIButtonControl()
    {
        debugPrintf(DBG, TEXT("CGUIButtonControl::CGUIButtonControl()\r\n"));
        reinterpret_cast<void(*)(CGUIButtonControl*)>(CGUIButtonControl_CGUIButtonControl)(this);
    }

    inline void setButtonFlags(DWORD flags)
    {
        *reinterpret_cast<DWORD*>(&m_vars[0x2C]) |= flags;
    }
    virtual void func(){};

    BYTE m_vars[0x74];
};

class CGUITextButtonControl: public CGUIButtonControl
{
public:
    CGUITextButtonControl()
    {
        debugPrintf(DBG, TEXT("CGUITextButtonControl::CGUITextButtonControl()\r\n"));
        reinterpret_cast<void(*)(CGUITextButtonControl*)>(CGUITextButtonControl_CGUITextButtonControl)(this);
    }
    void setText(const wchar_t* text, BOOL bRefresh)
    {
        reinterpret_cast<void(*)(CGUITextControl*, const wchar_t*, BOOL)>(CGUITextControl_setText)(&this->m_textControl, text, bRefresh);
    }

    inline void setFontColors(const ButtonTextFontColorDesc& fontColors)
    {
        *reinterpret_cast<ButtonTextFontColorDesc*>(&m_vars[0x110]) = fontColors;
    }

protected:
    CGUITextControl m_textControl;
    BYTE m_textControlChildVars[0x118];
    BYTE m_vars[0x134];
    //BYTE m_vars[0x80C];
};

class CGUIEmptyDlg;
class CGUIDlgWindow
{
public:
    void SetBackgroundImage(DWORD imageID)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, DWORD)>(CGUIDlgWindow_SetBackgroundImage)(this, imageID);
    }

    void addImageControl(CGUIImageControl& pControl, int imageIndex, ControlCoords& coords, int flags, BOOL bUnknown, DWORD unknown1, DWORD unknown2, DWORD unknown3)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, CGUIImageControl&, int, ControlCoords&, int, BOOL, int, int, int)>(CGUIDlgWindow__addImageControl)(this, pControl, imageIndex, coords, flags, bUnknown, unknown1, unknown2, unknown3);
    }

    void addTextButtonControl(CGUITextButtonControl& pControl, int imageIndex, ControlCoords& coords, int eventID, int fontSize, const wchar_t *pButtonText, ControlCoords* textCoords, ButtonTextFontColorDesc& fontColor, int textFlags, BOOL bTransparent)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, CGUITextButtonControl&, int, ControlCoords&, int, int, const wchar_t *, ControlCoords*, ButtonTextFontColorDesc&, int, int, BOOL, int)>(CGUIDlgWindow_addTextButtonControl)(this, pControl, imageIndex, coords, eventID, fontSize, pButtonText, textCoords, fontColor, textFlags, 0, bTransparent, 0);
    }

    //virtual void func() {};
    // void** __vptr;
    void* m_0x04;
    HLOCAL m_0x08;
    DWORD m_0x0C;
    DWORD m_0x10;
    DWORD m_0x14;
    DWORD m_0x18;
    DWORD m_drawWindow;
    CGUIEmptyDlg* m_pDialog;
    DWORD m_0x24;
    DWORD m_drawDC;
    DWORD m_windowDC;
    DWORD m_backgroundImageID;
};

class CGUIEmptyDlg
{
public:
    struct UnknownFuncCArg
    {
        DWORD arg1;
        DWORD arg2;
    };
private:
    struct ImplVTable
    {
        void (*unknownFunc0)(CGUIEmptyDlg* pSelf, bool bEnable);
        bool (*msgHandle)(CGUIEmptyDlg* pSelf, const IpcMsg* msg);
        void (*unknownFunc8)(CGUIEmptyDlg* pSelf);
        void (*unknownFuncC)(CGUIEmptyDlg* pSelf, UnknownFuncCArg* unknown);
        void (*unknownFunc10)(CGUIEmptyDlg* pSelf);
        HDC (*createDrawDC)(CGUIEmptyDlg* pSelf, HWND hWnd);
        bool (*onTimer)(CGUIEmptyDlg* pSelf, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        void (*createControls)(CGUIEmptyDlg* pSelf, CGUIEmptyDlg* pPrevDialog);
        void (*initControls)(CGUIEmptyDlg* pSelf, CGUIEmptyDlg* pPrevDialog);
        void (*drawControl)(CGUIEmptyDlg* pSelf);
        void (*onAfterInitControls)(CGUIEmptyDlg* pSelf);
        void (*onDraw)(CGUIEmptyDlg* pSelf);
        void (*onPaint)(CGUIEmptyDlg* pSelf);
        void (*onCreate)(CGUIEmptyDlg* pSelf);
        void (*onDialogActivated)(CGUIEmptyDlg* pSelf);
        HWND (*createWindow)(CGUIEmptyDlg* pSelf, HINSTANCE hInstance, HWND parentWindow, CGUIEmptyDlg* pPrevDialog, BOOL bShow);
        void (*closeWindow)(CGUIEmptyDlg* pSelf);
        bool (*onBtnClick)(CGUIEmptyDlg* pSelf, DWORD controlID, ButtonClickType clickType);
        bool (*onMouseDown)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        bool (*onMouseUp)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        bool (*onMouseOut)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        bool (*unknownFunc54)(CGUIEmptyDlg* pSelf, DWORD controlID, DWORD unknown, DWORD unknown1);
        bool (*unknownFunc58)(CGUIEmptyDlg* pSelf);
        bool (*onBeforeDestroy)(CGUIEmptyDlg* pSelf);
        bool (*onBeforeHide)(CGUIEmptyDlg* pSelf);
        bool (*onActivate)(CGUIEmptyDlg* pSelf, BOOL bInActivated);
        bool (*onAutoClose)(CGUIEmptyDlg* pSelf);
    };
public:
    CGUIEmptyDlg()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::CGUIEmptyDlg()\r\n"));
        reinterpret_cast<void(*)(CGUIEmptyDlg*)>(CGUIEmptyDlg_CGUIEmptyDlg)(this);
        m_implVTable = *reinterpret_cast<ImplVTable**>(this);
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::m_implVTable = 0x%08X\r\n"), m_implVTable);
    }

    ~CGUIEmptyDlg()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::~CGUIEmptyDlg()\r\n"));
        reinterpret_cast<void(*)(CGUIEmptyDlg*)>(CGUIEmptyDlg_destructor_CGUIEmptyDlg)(this);
    }
    virtual void unknownFunc0(bool bEnable)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFunc0(%s)\r\n"), bEnable ? TEXT("TRUE") : TEXT("FALSE"));
        getImpl()->unknownFunc0(this, bEnable);
    };
    virtual bool msgHandle(const IpcMsg* msg)
    {
        IpcMsg newMsg = *msg;
        newMsg.src &=0xFFFF;
        newMsg.cmd &=0xFFFF;
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::msgHandle(0x%02X, 0x%02X, 0x%08X, 0x%08X)\r\n"), newMsg.src, newMsg.cmd, newMsg.extraSize, newMsg.extra);
        if(newMsg.extraSize)
            debugDump(DBG, reinterpret_cast<const void*>(newMsg.extra), newMsg.extraSize);
        return getImpl()->msgHandle(this, &newMsg);
    };
    virtual void unknownFunc8()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFunc8()\r\n"));
        getImpl()->unknownFunc8(this);
    };
    virtual void unknownFuncC(UnknownFuncCArg* unknown)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFuncC() 0x%08X, 0x%08X\r\n"), unknown->arg1, unknown->arg2);
        getImpl()->unknownFuncC(this, unknown);
    };
    virtual void unknownFunc10()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFunc10()\r\n"));
        getImpl()->unknownFunc10(this);
    };
    virtual HDC createDrawDC(HWND hWnd)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::createDrawDC(0x%08X)\r\n"), hWnd);
        return getImpl()->createDrawDC(this, hWnd);
    };
    virtual bool onTimer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onTimer(0x%08X, 0x%04X, 0x%08X, 0x%08X)\r\n"), hWnd, uMsg, wParam, lParam);
        return getImpl()->onTimer(this, hWnd, uMsg, wParam, lParam);
    };
    virtual void createControls(CGUIEmptyDlg* pPrevDialog) = 0;
    virtual void initControls(CGUIEmptyDlg* pPrevDialog)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::initControls(0x%08X)\r\n"), pPrevDialog);
        getImpl()->initControls(this, pPrevDialog);
    };
    virtual void drawControl(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::drawControl()\r\n"));
        getImpl()->drawControl(this);
    };
    virtual void onAfterInitControls(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onAfterInitControls()\r\n"));
        getImpl()->onAfterInitControls(this);
    };
    virtual void onDraw(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onDraw()\r\n"));
        getImpl()->onDraw(this);
    };
    virtual void onPaint(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onPaint()\r\n"));
        getImpl()->onPaint(this);
    };
    virtual void onCreate(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onCreate()\r\n"));
        getImpl()->onCreate(this);
    };
    virtual void onDialogActivated()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onDialogActivated()\r\n"));
        getImpl()->onDialogActivated(this);
    };
    virtual HWND createWindow(HINSTANCE hInstance, HWND parentWindow, CGUIEmptyDlg* pPrevDialog, BOOL bShow)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::createWindow(0x%08X, 0x%08X, 0x%08X, %s)\r\n"), hInstance, parentWindow, pPrevDialog, bShow ? TEXT("TRUE") : TEXT("FALSE"));
        return getImpl()->createWindow(this, hInstance, parentWindow, pPrevDialog, bShow);
    };
    virtual void closeWindow(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::closeWindow()\r\n"));
        getImpl()->closeWindow(this);
    };
    virtual bool onBtnClick(DWORD controlID, ButtonClickType clickType)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onBtnClick(0x%04X, 0x%02X)\r\n"), controlID, clickType);
        return getImpl()->onBtnClick(this, controlID, clickType);
    };
    virtual bool onMouseDown(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseDown(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseDown(this, controlID, eventPoint);
    };
    virtual bool onMouseUp(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseUp(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseUp(this, controlID, eventPoint);
    };
    virtual bool onMouseOut(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseOut(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseOut(this, controlID, eventPoint);
    };
    virtual bool unknownFunc54(DWORD controlID, DWORD unknown, DWORD unknown1)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFunc54(0x%04X, 0x%08X, 0x%08X)\r\n"), controlID, unknown, unknown1);
        return getImpl()->unknownFunc54(this, controlID, unknown, unknown1);
    };
    virtual bool unknownFunc58()
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::unknownFunc58()\r\n"));
        return getImpl()->unknownFunc58(this);
    };
    virtual bool onBeforeDestroy(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onBeforeDestroy()\r\n"));
        return getImpl()->onBeforeDestroy(this);
    };
    virtual bool onBeforeHide(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onBeforeHide()\r\n"));
        return getImpl()->onBeforeHide(this);
    };
    virtual bool onActivate(BOOL bInActivated)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onActivate(%s)\r\n"), bInActivated ? TEXT("TRUE") : TEXT("FALSE"));
        return getImpl()->onActivate(this, bInActivated);
    };
    virtual bool onAutoClose(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onAutoClose()\r\n"));
        return getImpl()->onAutoClose(this);
    };


    void addTextControl(CGUITextControl& pControl, int imageIndex, ControlCoords& coords, int fontSize, const wchar_t *pButtonText, int fontColor, int textFlags, int flag4)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUITextControl&, int, ControlCoords&, int, const wchar_t *, int, int, int)>(CGUIEmptyDlg_addTextControl)(this, pControl, imageIndex, coords, fontSize, pButtonText, fontColor, textFlags, flag4);
    }

    void addButtonControl(CGUIButtonControl& pControl, int imageIndex, ControlCoords& coords, int controlID, DWORD unknown)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUIButtonControl&, int, ControlCoords& coords, int, DWORD)>(CGUIEmptyDlg_addButtonControl)(this, pControl, imageIndex, coords, controlID, unknown);
    }
    
    const WNDCLASS* registerClass(HINSTANCE hInstance, WNDPROC wndProc)
    {
        return reinterpret_cast<const WNDCLASS*(*)(CGUIEmptyDlg*, HINSTANCE, WNDPROC)>(CGUIEmptyDlg__registerClass)(this, hInstance, wndProc);
    }

    LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return reinterpret_cast<LRESULT(*)(CGUIEmptyDlg*, HWND, UINT, WPARAM, LPARAM)>(CGUIEmptyDlg__wndProc)(this, hWnd, uMsg, wParam, lParam);
    }

    HWND createDialog(CGUIEmptyDlg* pPrevDialog, BOOL bShow)
    {
        return reinterpret_cast<HWND(*)(CGUIEmptyDlg*, CGUIEmptyDlg*, BOOL)>(CGUIEmptyDlg__createDialog)(this, pPrevDialog, bShow);
    }

    void changeDialog(CGUIEmptyDlg* pPrevDialog, BOOL bGoBack)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUIEmptyDlg*, BOOL)>(CGUIEmptyDlg_changeDialog)(this, pPrevDialog, bGoBack);
    }

    static CGUIEmptyDlg* getActiveDialog()
    {
        return reinterpret_cast<CGUIEmptyDlg*(*)()>(CGUIEmptyDlg__getActiveDialog)();
    }

    static ButtonTextFontColorDesc getUITextColors(UIType uiType)
    {
        ButtonTextFontColorDesc fontColors;
        switch(uiType)
        {
        case UIType_M0:
            fontColors.color = FONT_COLOR_WHITE;
            fontColors.selectedColor = FONT_COLOR_WHITE;
            fontColors.disabledColor = FONT_COLOR_DISABLED_UI0;
            fontColors.color2 = FONT_COLOR_WHITE;
            break;
        case UIType_M1:
            fontColors.color = FONT_COLOR_WHITE;
            fontColors.selectedColor = FONT_COLOR_BLACK;
            fontColors.disabledColor = FONT_COLOR_DISABLED_UI1;
            fontColors.color2 = FONT_COLOR_BLACK;
            break;
        case UIType_M0_INV:
            fontColors.color = FONT_COLOR_WHITE;
            fontColors.selectedColor = FONT_COLOR_WHITE;
            fontColors.disabledColor = FONT_COLOR_DISABLED_UI2;
            fontColors.color2 = FONT_COLOR_WHITE;
            break;
        case UIType_M1_INV:
            fontColors.color = FONT_COLOR_BLACK;
            fontColors.selectedColor = FONT_COLOR_BLACK;
            fontColors.disabledColor = FONT_COLOR_DISABLED_UI3;
            fontColors.color2 = FONT_COLOR_BLACK;
            break;
        }
        return fontColors;
    }

    // void** __vptr;
    CGUIDlgWindow m_dlgWindow;
/*    void* m_0x04;
    HLOCAL m_0x08;
    DWORD m_0x0C;
    DWORD m_0x10;
    DWORD m_0x14;
    DWORD m_0x18;
    DWORD m_drawWindow;
    CGUIEmptyDlg* m_pSelf;
    DWORD m_0x24;
    DWORD m_drawDC;
    DWORD m_windowDC;
    DWORD m_backgroundImageID;*/
    DWORD m_0x34;
    DWORD m_0x38;
    DWORD m_width;
    DWORD m_height;
    DWORD m_0x44;
    HINSTANCE m_hInstance;
    DWORD m_hBackgroundDC;
    HWND m_hWnd;
    DWORD m_hBackgroundBitmap;
    DWORD m_hBackgroundSaveObj;
    DWORD m_bClassRegistered;
    DWORD m_mouseCaptured;
    DWORD m_0x64;
    DWORD m_bInitialized;
    DWORD m_bCreated;
    DWORD m_bVisible;
    DWORD m_0x74;
    DWORD m_0x78;
    CTopBar* m_pTopBarControl;
    DWORD m_autoCloseTimeout;
    DWORD m_0x84;
    DWORD m_0x88;
    DWORD m_timer1004Timeout;
    WNDCLASS m_windowClass;
    BYTE m_0xB8[520];
    DWORD m_0x2C0;
    DWORD m_0x2C4;
    DWORD m_screenID;
    DWORD m_0x2CC;

private:
    ImplVTable* m_implVTable;
    inline ImplVTable* getImpl()
    {
        return m_implVTable;
    }
};

namespace AppMain
{
    inline void goHome()
    {
        reinterpret_cast<void(*)()>(AppMain_goHome)();
    }
    
    inline void goHomeBack(BOOL bShow)
    {
        reinterpret_cast<void(*)(BOOL)>(AppMain_goHomeBack)(bShow);       
    }

    inline void goBack(DWORD backSteps, BOOL bUnknown)
    {
        reinterpret_cast<void(*)(DWORD, BOOL)>(AppMain_goBack)(backSteps, bUnknown);       
    }

    namespace Dialogs
    {
        class CEcoScoringDlg :public CGUIEmptyDlg
        {
        public:
            static CEcoScoringDlg* singleton()
            {
                return reinterpret_cast<CEcoScoringDlg*(*)()>(CEcoScoringDlg__singleton)();   
            }
        };

        class C4x4InfoDlg :public CGUIEmptyDlg
        {
        public:
            static C4x4InfoDlg* singleton()
            {
                return reinterpret_cast<C4x4InfoDlg*(*)()>(C4x4InfoDlg__singleton)();   
            }
        };
    }
}