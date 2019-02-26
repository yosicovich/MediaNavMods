#pragma once
#include "demuxtypes.h"

class MP3ElementaryType: public ElementaryType
{
public:
    MP3ElementaryType(DWORD sampleRate, DWORD bitrate, BYTE channels, bool mpeg2);

    bool IsVideo() const {return false;};
    bool GetType(CMediaType* pmt, int nType) const;
    virtual void setHandler(const CMediaType* pmt, int idx);
private:
    CMediaType m_mediaType;

};
