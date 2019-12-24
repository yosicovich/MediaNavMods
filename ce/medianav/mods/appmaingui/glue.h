
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
#define CGUIEmptyDlg_addSwitchButtonControl 0x0012BD44
#define CGUIEmptyDlg_addProgressBarControl 0x0012BDE4
#define CGUIEmptyDlg__registerClass 0x0012D1F0
#define CGUIEmptyDlg__wndProc 0x0012AD2C
#define CGUIEmptyDlg__getActiveDialog 0x00011000
#define CGUIEmptyDlg__createDialog 0x0012B270
#define CGUIEmptyDlg_changeDialog 0x0012C3AC

#define CGUIDlgWindow_SetBackgroundImage 0x00131988
#define CGUIDlgWindow__addImageControl 0x00131C58
#define CGUIDlgWindow_addTextButtonControl 0x00131D1C
#define CGUIDlgWindow_addControl 0x0012EF84

#define CGUIControl_draw 0x0012F39C
#define CGUIControl_refresh 0x0012F11C

#define CGUIImageControl_CGUIImageControl 0x0012E5BC
#define CGUIImageControl_dCGUIImageControl 0x00013F38

#define CGUIProgressBarControl_CGUIProgressBarControl 0x001342EC

#define CGUITextControl_CGUITextControl 0x0012F534
#define CGUITextControl_dCGUITextControl 0x0001445C
#define CGUITextControl_setText 0x0012F788

#define CGUIButtonControl_CGUIButtonControl 0x00133048

#define CGUISwitchButtonControl_vtable 0x00181BD8

#define CGUITextButtonControl_CGUITextButtonControl 0x001338AC
#define CGUITextButtonControl_dCGUITextButtonControl 0x00133A04

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

#define TEXT_VALIGN_TOP 0x00
#define TEXT_VALIGN_CENTER 0x04
#define TEXT_VALIGN_BOTTOM 0x08

#define TEXT_UNKNOWN_FLAG1 0x20
#define TEXT_FIRST_CHAR_UNDERSCORE 0x80

#define TEXT_SHOW_HIDDEN 0x400
#define TEXT_UNKNOWN_FLAG 0x800

// Button flags
#define IMAGE_FLAG_INITIALIZED 0x40
#define IMAGE_FLAG_VISIBLE 0x80
#define IMAGE_FLAG_ENABLED 0x100
#define IMAGE_FLAG_TRANSPARENT 0x200
#define IMAGE_FLAG_REDRAW_BACKGROUND 0x400
#define IMAGE_FLAG_IMAGE_32BIT 0x800
#define IMAGE_FLAG_RESIZE 0x1000

enum ButtonClickType
{
    ButtonClickType_LButtonUp = 2,
    ButtonClickType_RButtonDown = 3,
    ButtonClickType_RButtonStillHeld = 4,
    ButtonClickType_RButtonUp = 5
};

enum ButtonState
{
    ButtonState_Normal = 0,
    ButtonState_Pressed = 1,
    ButtonState_Disabled = 2,
    ButtonState_0x38 = 3,
};
struct MousePoint
{
    DWORD x;
    DWORD y;
};

struct ControlCoords
{
    ControlCoords()
        :x(0), y(0), width(0), height(0)
    {

    }
    ControlCoords(DWORD x, DWORD y, DWORD width, DWORD height)
        :x(x), y(y), width(width), height(height)
    {

    }
    DWORD x;
    DWORD y;
    DWORD width;
    DWORD height;
};

struct ControlMoveDelta
{
    ControlMoveDelta()
        :deltaX(0), deltaY(0)
    {

    }

    ControlMoveDelta(int deltaX, int deltaY)
        :deltaX(0), deltaY(0)
    {

    }
    int deltaX;
    int deltaY;
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

class CGUIDlgWindow;

class CTopBar
{
public:
    static CTopBar* singleton()
    {
        return reinterpret_cast<CTopBar*(*)()>(CTopBar_singleton)();
    };

    virtual ~CTopBar()
    {

    }

    BYTE m_vars[0xA78];
};

class CGUIControl
{
public:
    inline void setRedrawBackground(bool bRedraw)
    {
        if(bRedraw)
            m_flags |= IMAGE_FLAG_REDRAW_BACKGROUND;
        else
            m_flags &= ~IMAGE_FLAG_REDRAW_BACKGROUND;
    }

    inline void setImageTransparent(bool bImageTransparent)
    {
        if(bImageTransparent)
            m_flags |= IMAGE_FLAG_TRANSPARENT;
        else
            m_flags &= ~IMAGE_FLAG_TRANSPARENT;
    }

    inline void setVisible(bool bVisible, BOOL bRedraw)
    {
        if(bVisible)
            m_flags |= IMAGE_FLAG_VISIBLE;
        else
            m_flags &= ~IMAGE_FLAG_VISIBLE;
        
        if(bRedraw)
            draw();
    }
    
    inline bool getVisible() const
    {
        return (m_flags & IMAGE_FLAG_VISIBLE) != 0;
    }
    inline void draw()
    {
        return reinterpret_cast<void(*)(CGUIControl*)>(CGUIControl_draw)(this);
    }

    inline void refresh()
    {
        return reinterpret_cast<void(*)(CGUIControl*)>(CGUIControl_refresh)(this);
    }

    inline void setButtonFlags(DWORD flags)
    {
        m_flags |= flags;
    }

    inline void resetButtonFlags(DWORD flags)
    {
        m_flags &= ~flags;
    }

// Virtual function    
    virtual void eraseBackground()
    {

    };

    virtual BOOL onMouseDown(const MousePoint& eventPoint)
    {
        return FALSE;
    };

    virtual BOOL onMouseUp(const MousePoint& eventPoint)
    {
        return FALSE;
    };

    virtual bool onMouseMove(const MousePoint& eventPoint)
    {
        return FALSE;

    };

    virtual void unused1()
    {
    };
    virtual void unused2()
    {

    };
    virtual void init(DWORD imageID, const ControlCoords& coords, CGUIDlgWindow* pWindow, BOOL bNativeFormatImage)
    {

    };
    virtual void drawControl()
    {

    };

    virtual void setEnabled(BOOL bEnabled, BOOL bRedraw)
    {
        if(bRedraw)
            m_flags |= IMAGE_FLAG_ENABLED;
        else
            m_flags &= ~IMAGE_FLAG_ENABLED;

        if(bRedraw)
            draw();
    };

    virtual void setPositionXY(DWORD x, DWORD y)
    {
        m_coords.x = x;
        m_coords.y = y;
    };

    virtual void setPosition(const POINT& pt)
    {
        m_coords.x = pt.x;
        m_coords.y = pt.y;
    };

    virtual void moveXY(int deltaX, int deltaY)
    {
        m_coords.x += deltaX;
        m_coords.y += deltaY;
    };

    virtual void move(const ControlMoveDelta& delta)
    {
        m_coords.x += delta.deltaX;
        m_coords.y += delta.deltaY;
    };

    CGUIDlgWindow* m_pWindow;
    ControlCoords m_coords;
    ControlCoords m_unknownCoords;
    DWORD m_imageID;
    BOOL m_bNativeFormatImage;
    DWORD m_flags;
};


// 0x58 in total
class CGUIImageControl: public CGUIControl
{
public:
    CGUIImageControl()
    {
        debugPrintf(DBG, TEXT("CGUIImageControl::CGUIImageControl()\r\n"));
        reinterpret_cast<void(*)(CGUIImageControl*)>(CGUIImageControl_CGUIImageControl)(this);
    }
    ~CGUIImageControl()
    {
        reinterpret_cast<void(*)(CGUIImageControl*)>(CGUIImageControl_dCGUIImageControl)(this);
    }

    BYTE m_frameIndex;
    BYTE m_maxFrames;
    WORD m_drawFlags;// 0x01 - stretch
    DWORD m_0x38;
    BOOL  m_bAdvancedClicks;
    HBITMAP m_hImage;
    CGUIImageControl* m_pChildren;
    DWORD m_childrenCount;
    DWORD m_eventID;
    DWORD m_0x50;
    DWORD m_0x54;
//    BYTE m_vars[0x40];
};

class CGUIProgressBarControl: public CGUIControl
{
public:
    CGUIProgressBarControl()
    {
        debugPrintf(DBG, TEXT("CGUIProgressBarControl::CGUIProgressBarControl()\r\n"));
        reinterpret_cast<void(*)(CGUIProgressBarControl*)>(CGUIProgressBarControl_CGUIProgressBarControl)(this);
    }

    BOOL m_bOwnProgressRect;
    DWORD m_eventID;
    BOOL  m_min;
    DWORD m_max;
    DWORD m_current;
    ControlCoords m_progressRect;
};

// 0x54C in total
class CGUITextControl: public CGUIImageControl
{
public:
    CGUITextControl()
    {
        debugPrintf(DBG, TEXT("CGUITextControl::CGUITextControl()\r\n"));
        reinterpret_cast<void(*)(CGUITextControl*)>(CGUITextControl_CGUITextControl)(this);
    }
    
    ~CGUITextControl()
    {
        reinterpret_cast<void(*)(CGUITextControl*)>(CGUITextControl_dCGUITextControl)(this);
    }

    void setText(const wchar_t* text, BOOL bRefresh)
    {
        reinterpret_cast<void(*)(CGUITextControl*, const wchar_t*, BOOL)>(CGUITextControl_setText)(this, text, bRefresh);
    }

    inline void setFontColor(DWORD fontColor)
    {
        *reinterpret_cast<DWORD*>(&m_vars[0x60]) = fontColor;
    }

    void setText(double value, int precision, BOOL bRefresh)
    {
        static wchar_t textBuf[129];
        int wrote = _snwprintf(reinterpret_cast<wchar_t*>(&textBuf), 128, TEXT("%.*f"), precision, value);
        if(wrote <0 || wrote > 128)
            setText(TEXT("n/a"), bRefresh);
        else
            setText(reinterpret_cast<wchar_t*>(&textBuf), bRefresh);
    }

    void setText(int value, BOOL bRefresh)
    {
        static wchar_t textBuf[129];
        int wrote = _snwprintf(reinterpret_cast<wchar_t*>(&textBuf), 128, TEXT("%d"), value);
        if(wrote <0 || wrote > 128)
            setText(TEXT("n/a"), bRefresh);
        else
            setText(reinterpret_cast<wchar_t*>(&textBuf), bRefresh);
    }

    BYTE m_vars[0x4F4];
};

// 0x78 in total
class CGUIButtonControl: public CGUIImageControl
{
public:
    CGUIButtonControl()
    {
        debugPrintf(DBG, TEXT("CGUIButtonControl::CGUIButtonControl()\r\n"));
        reinterpret_cast<void(*)(CGUIButtonControl*)>(CGUIButtonControl_CGUIButtonControl)(this);
    }
    
    virtual void drawButton(ButtonState buttonState)
    {

    }
    BYTE m_vars[0x20];
};

class CGUISwitchButtonControl: public CGUIButtonControl
{
public:
    CGUISwitchButtonControl()
    {
        debugPrintf(DBG, TEXT("CGUISwitchButtonControl::CGUISwitchButtonControl()\r\n"));
        reinterpret_cast<void(*)(CGUIButtonControl*)>(CGUIButtonControl_CGUIButtonControl)(this);
        *reinterpret_cast<DWORD*>(this)=CGUISwitchButtonControl_vtable;
        m_imagesIndex = 0;
        m_0x79 = 0;
    }

    BYTE m_imagesIndex;// Multi image index. 0 - first button in image(0-3 frames), 1 - second button (4-7 frames) and so on.
    BYTE m_0x79;
};

// 0x810 in total
class CGUITextButtonControl: public CGUIButtonControl
{
public:
    CGUITextButtonControl()
    {
        debugPrintf(DBG, TEXT("CGUITextButtonControl::CGUITextButtonControl()\r\n"));
        reinterpret_cast<void(*)(CGUITextButtonControl*)>(CGUITextButtonControl_CGUITextButtonControl)(this);
    }
    
    ~CGUITextButtonControl()
    {
        reinterpret_cast<void(*)(CGUITextButtonControl*)>(CGUITextButtonControl_dCGUITextButtonControl)(this);
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

    void addImageControl(CGUIImageControl& pControl, int imageIndex, ControlCoords& coords, int flags/*unknown probably ignored*/, BOOL bImageTransparent, BOOL bNativeFormatImage)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, CGUIImageControl&, int, ControlCoords&, int, BOOL, int, int, int)>(CGUIDlgWindow__addImageControl)(this, pControl, imageIndex, coords, flags, bImageTransparent, 0, 0, bNativeFormatImage);
        pControl.setRedrawBackground(bImageTransparent == TRUE);
    }

    void addTextButtonControl(CGUITextButtonControl& pControl, int imageIndex, ControlCoords& coords, int eventID, int fontSize, const wchar_t *pButtonText, ControlCoords* textCoords, ButtonTextFontColorDesc& fontColor, int textFlags, BOOL bTransparent)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, CGUITextButtonControl&, int, ControlCoords&, int, int, const wchar_t *, ControlCoords*, ButtonTextFontColorDesc&, int, int, BOOL, int)>(CGUIDlgWindow_addTextButtonControl)(this, pControl, imageIndex, coords, eventID, fontSize, pButtonText, textCoords, fontColor, textFlags, 0, bTransparent, 0);
        pControl.setRedrawBackground(bTransparent == TRUE);
    }

    void addControl(CGUIControl& pControl)
    {
        reinterpret_cast<void(*)(CGUIDlgWindow*, CGUIControl&, BOOL)>(CGUIDlgWindow_addControl)(this, pControl, TRUE);
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
    HDC m_backgroundHdc;
    DWORD m_drawDC;
    DWORD m_windowDC;
    DWORD m_backgroundImageID;
    ControlCoords m_coords;
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
        BOOL (*msgHandle)(CGUIEmptyDlg* pSelf, const IpcMsg* msg);
        void (*unknownFunc8)(CGUIEmptyDlg* pSelf);
        void (*unknownFuncC)(CGUIEmptyDlg* pSelf, UnknownFuncCArg* unknown);
        void (*unknownFunc10)(CGUIEmptyDlg* pSelf);
        HDC (*createDrawDC)(CGUIEmptyDlg* pSelf, HWND hWnd);
        BOOL (*onTimer)(CGUIEmptyDlg* pSelf, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
        BOOL (*onBtnClick)(CGUIEmptyDlg* pSelf, DWORD controlID, ButtonClickType clickType);
        BOOL (*onMouseDown)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        BOOL (*onMouseUp)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        BOOL (*onMouseOut)(CGUIEmptyDlg* pSelf, DWORD controlID, const MousePoint& eventPoint);
        BOOL (*onProgressBarEnter)(CGUIEmptyDlg* pSelf, DWORD controlID, BOOL bMove, DWORD curValue);
        BOOL (*onProgressBarExit)(CGUIEmptyDlg* pSelf, DWORD controlID);
        BOOL (*onExit)(CGUIEmptyDlg* pSelf);
        BOOL (*onBeforeHide)(CGUIEmptyDlg* pSelf);
        BOOL (*onActivate)(CGUIEmptyDlg* pSelf, BOOL bInActivated);
        void (*onInactivityTimeout)(CGUIEmptyDlg* pSelf);
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
    virtual BOOL msgHandle(const IpcMsg* msg)
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
    virtual BOOL onTimer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    virtual BOOL onBtnClick(DWORD controlID, ButtonClickType clickType)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onBtnClick(0x%04X, 0x%02X)\r\n"), controlID, clickType);
        return getImpl()->onBtnClick(this, controlID, clickType);
    };
    virtual BOOL onMouseDown(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseDown(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseDown(this, controlID, eventPoint);
    };
    virtual BOOL onMouseUp(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseUp(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseUp(this, controlID, eventPoint);
    };
    virtual BOOL onMouseOut(DWORD controlID, const MousePoint& eventPoint)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onMouseOut(0x%04X, {x=0x%04X,y=0x%04X))\r\n"), controlID, eventPoint.x, eventPoint.y);
        return getImpl()->onMouseOut(this, controlID, eventPoint);
    };
    virtual BOOL onProgressBarEnter(DWORD controlID, BOOL bMove, DWORD curValue)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onProgressBarEnter(0x%04X, %s, %d)\r\n"), controlID, bMove == TRUE ? TEXT("TRUE") : TEXT("FALSE") , curValue);
        return getImpl()->onProgressBarEnter(this, controlID, bMove, curValue);
    };
    virtual BOOL onProgressBarExit(DWORD controlID)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onProgressBarExit(0x%04X)\r\n"), controlID);
        return getImpl()->onProgressBarExit(this, controlID);
    };
    virtual BOOL onExit(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onExit()\r\n"));
        return getImpl()->onExit(this);
    };
    virtual BOOL onBeforeHide(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onBeforeHide()\r\n"));
        return getImpl()->onBeforeHide(this);
    };
    virtual BOOL onActivate(BOOL bInActivated)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onActivate(%s)\r\n"), bInActivated ? TEXT("TRUE") : TEXT("FALSE"));
        return getImpl()->onActivate(this, bInActivated);
    };
    virtual void onInactivityTimeout(void)
    {
        debugPrintf(DBG, TEXT("CGUIEmptyDlg::onInactivityTimeout()\r\n"));
        getImpl()->onInactivityTimeout(this);
    };


    void addTextControl(CGUITextControl& pControl, int imageIndex, ControlCoords& coords, int fontSize, const wchar_t *pButtonText, int fontColor, int textFlags, int flag4)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUITextControl&, int, ControlCoords&, int, const wchar_t *, int, int, int)>(CGUIEmptyDlg_addTextControl)(this, pControl, imageIndex, coords, fontSize, pButtonText, fontColor, textFlags, flag4);
    }

    void addButtonControl(CGUIButtonControl& pControl, int imageIndex, ControlCoords& coords, int controlID, BOOL bImageTransparent)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUIButtonControl&, int, ControlCoords& coords, int, BOOL)>(CGUIEmptyDlg_addButtonControl)(this, pControl, imageIndex, coords, controlID, bImageTransparent);
        pControl.setRedrawBackground(bImageTransparent == TRUE);
    }
    
    void addSwitchButtonControl(CGUISwitchButtonControl& pControl, int imageIndex, ControlCoords& coords, int controlID, BOOL bImageTransparent)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUISwitchButtonControl&, int, ControlCoords& coords, int, BOOL)>(CGUIEmptyDlg_addButtonControl)(this, pControl, imageIndex, coords, controlID, bImageTransparent);
        pControl.setRedrawBackground(bImageTransparent == TRUE);
    }

    void addProgressBarControl(CGUIProgressBarControl& pControl, int imageIndex, ControlCoords& coords, int controlID, BOOL bImageTransparent)
    {
        reinterpret_cast<void(*)(CGUIEmptyDlg*, CGUIProgressBarControl&, int, ControlCoords& coords, int, BOOL)>(CGUIEmptyDlg_addProgressBarControl)(this, pControl, imageIndex, coords, controlID, bImageTransparent);
        pControl.setRedrawBackground(bImageTransparent == TRUE);
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
    DWORD m_backgroundImageID;
    DWORD m_0x34;
    DWORD m_0x38;
    DWORD m_width;
    DWORD m_height;*/
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
    CGUIEmptyDlg* m_pActivePopup;
    CTopBar* m_pTopBarControl;
    DWORD m_inactivityTimeout;
    DWORD m_0x84; // Unknown, probably popup keep related.
    DWORD m_rightBtnClickDetectTimout;
    DWORD m_rightBtnStillHeldNotifyTimeout;
    WNDCLASS m_windowClass;

    // Here is a small hack to maintain original class size and use additional vars. 
    wchar_t m_wndClassStrBuf[MAX_PATH];
    //wchar_t m_wndClassStrBuf[MAX_PATH - 2];
    //ImplVTable* m_implVTable;

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