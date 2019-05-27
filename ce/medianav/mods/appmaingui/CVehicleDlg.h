#pragma once

#include "stdafx.h"
#include "glue.h"
#include "glue_other.h"
#include "utils.h"
#include "appmaingui.h"

class CVehicleDlg : public CGUIFixedEmptyDlg<CVehicleDlg>
{
public:
    CVehicleDlg();
    virtual void createControls(CGUIEmptyDlg* pPrevDialog);
    virtual void initControls(CGUIEmptyDlg* pPrevDialog);
    virtual bool onBtnClick(DWORD controlID, ButtonClickType clickType);
private:
    CGUITextControl m_caption;
    CGUIImageControl m_headerLine;
    CGUITextButtonControl m_ecoButton;
    CGUITextButtonControl m_infoButton;
    CGUIButtonControl m_backButton;
};
template class CGUIFixedEmptyDlg<CVehicleDlg>;
