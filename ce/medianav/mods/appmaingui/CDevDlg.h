#pragma once

#include "stdafx.h"
#include "glue.h"
#include "glue_other.h"
#include "utils.h"
#include "appmaingui.h"

class CDevDlg : public CGUIFixedEmptyDlg<CDevDlg>
{
public:
    CDevDlg();
    virtual void createControls(CGUIEmptyDlg* pPrevDialog);
    virtual void initControls(CGUIEmptyDlg* pPrevDialog);
    virtual BOOL onBtnClick(DWORD controlID, ButtonClickType clickType);
    virtual void onInactivityTimeout(void);
private:
    CGUITextControl m_caption;
    CGUIImageControl m_headerLine;
    CGUITextButtonControl m_ecoButton;
    CGUITextButtonControl m_infoButton;
    CGUITextButtonControl m_perfButton;
    CGUIButtonControl m_backButton;

    CGUITextControl m_captiont;
    CGUISwitchButtonControl m_testButton;
    CGUIProgressBarControl m_progress;
};
template class CGUIFixedEmptyDlg<CDevDlg>;
