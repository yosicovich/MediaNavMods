#pragma once


#include "stdafx.h"
#include "glue.h"
#include "glue_other.h"
#include "utils.h"
#include "appmaingui.h"
#include <vector>
#include <sstream>
#include <SimpleIni.h>

class CDevMaketDlg : public CGUIFixedEmptyDlg<CDevMaketDlg>
{
public:
    CDevMaketDlg();
    ~CDevMaketDlg();
    virtual void createControls(CGUIEmptyDlg* pPrevDialog);
    virtual void initControls(CGUIEmptyDlg* pPrevDialog);
    virtual BOOL onBtnClick(DWORD controlID, ButtonClickType clickType);
private:
    bool createImageControl(int idx);
    bool createTextControl(int idx);
    bool createButtonControl(int idx);
    bool createTextButtonControl(int idx);
    
    CSimpleIni m_iniConfig;

    std::vector<CGUITextControl*>  m_textControls;
    std::vector<CGUIImageControl*> m_imageControls;
    std::vector<CGUIButtonControl*> m_buttonControls;
    std::vector<CGUITextButtonControl*> m_textButtonControls;

    std::wostringstream m_codeStream;
};
template class CGUIFixedEmptyDlg<CDevMaketDlg>;
