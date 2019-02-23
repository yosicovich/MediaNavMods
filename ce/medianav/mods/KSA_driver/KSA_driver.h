#include "StdAfx.h"

struct KSADriverData
{
    Utils::CLock m_accessLock;
};

struct KSAInstanceData
{
    DWORD pCurPosition;
    KSAInstanceData(DWORD curPosition)
        :pCurPosition(curPosition)
    {
    }
    Utils::CLock m_accessLock;
};