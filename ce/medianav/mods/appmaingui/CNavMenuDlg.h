#pragma once

#include "stdafx.h"
#include "glue.h"
#include "glue_other.h"
#include "utils.h"
#include "appmaingui.h"

class CNavMenuDlg : public CGUIFixedEmptyDlg<CNavMenuDlg>
{
public:
    CNavMenuDlg();
    virtual void createControls(CGUIEmptyDlg* pPrevDialog);
    virtual void initControls(CGUIEmptyDlg* pPrevDialog);
    virtual BOOL onBtnClick(DWORD controlID, ButtonClickType clickType);
private:
    CGUITextControl m_caption;
    CGUIImageControl m_headerLine;
    CGUIButtonControl m_backButton;
    CGUITextButtonControl m_igoButton;
    CGUITextButtonControl m_navitelButton;
    // 0x1918
    BYTE m_dummy[0xD14];
    BOOL m_bIgoReady;
    BOOL m_bAgreePressed;
};
template class CGUIFixedEmptyDlg<CNavMenuDlg>;
