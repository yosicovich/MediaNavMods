//
// ElemType.h: declarations of elementary stream type classes.
//
// Media type and other format-specific data that depends
// on the format of the contained elementary streams.
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk

#pragma once

#include "demuxtypes.h"
#include <aviriff.h>

class AviElementaryType: public ElementaryType
{
public:
    AviElementaryType();

    bool Parse(const AVISTREAMHEADER& streamHeader, const AtomPtr& pFormat); 
	bool IsVideo() const;
    bool GetType(CMediaType* pmt, int nType) const;
    virtual void setHandler(const CMediaType* pmt, int idx);
private:
    bool static FindFourCC(DWORD handlerFourCC, GUID& guid, bool& extendedFormat);
private:
    AtomCache m_format;
    AtomPtr m_pFormatAtom;
    AVISTREAMHEADER m_streamHeader;
    CMediaType m_mediaType;
    
};
