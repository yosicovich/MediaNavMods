#include "common.h"
#include <sstream>

#ifdef DEV_MAKET
CDevMaket::CDevMaket()
:m_iniConfig(true)
{
    m_iniConfig.LoadFile((Utils::getModulePath(NULL) + TEXT("\\CDevMaketDlg.ini")).c_str());
}

const wchar_t* CDevMaket::getImagePath(DWORD imageID)
{
    std::wostringstream strStream;
    strStream << imageID;
    return m_iniConfig.GetValue(TEXT("Images"),strStream.str().c_str(), NULL);
}

const wchar_t* CDevMaket::getMultiLangStr(DWORD strID)
{
    std::wostringstream strStream;
    strStream << strID;
    return  m_iniConfig.GetValue(TEXT("Strings"),strStream.str().c_str(), NULL);
}
#endif