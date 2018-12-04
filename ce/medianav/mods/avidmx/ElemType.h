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

// descriptors have a tag byte, then a length
// encoded with a "next byte" bit and 7 bit length
//
// This class refers to a buffer held elsewhere

#include "demuxtypes.h"
#include <aviriff.h>

// conversion from elementary stream descriptors to
// DirectShow media type
//
// We can offer multiple type definitions for a given stream,
// and support a format handler object specific to the type chosen
// to create the output stream.
class AviElementaryType: public ElementaryType
{
public:
    AviElementaryType();

    bool Parse(const AVISTREAMHEADER& streamHeader, Atom* pFormat); // atom should be stsd descriptor mp4v, jvt1, mp4a
	bool IsVideo() const;
    bool GetType(CMediaType* pmt, int nType) const;
    virtual void setHandler(const CMediaType* pmt, int idx);
private:
    bool GetType_Audio(CMediaType* pmt) const;
    bool GetType_Video(CMediaType* pmt) const;
    bool static FindFourCC(DWORD handlerFourCC, GUID& guid, bool& extendedFormat);
private:
    AtomCache m_format;
    AtomPtr m_pFormatAtom;
    AVISTREAMHEADER m_streamHeader;
    CMediaType m_mediaType;
    
};
