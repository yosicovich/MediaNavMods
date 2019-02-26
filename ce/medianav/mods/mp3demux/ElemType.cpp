//
// ElemType.h: implementations of elementary stream type classes.
//

#include "stdafx.h"
#include "ElemType.h"
#include <handlers.h>
#include <mmreg.h>
#include <audshow.h>

MP3ElementaryType::MP3ElementaryType(DWORD sampleRate, DWORD bitrate, BYTE channels, bool mpeg2)
:ElementaryType()
{
    m_shortname = "auds";
    m_mediaType.InitMediaType();
    m_mediaType.SetType(&MEDIATYPE_Audio);
    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof(WAVEFORMATEX));
    if(mpeg2)
    {
        m_mediaType.SetSubtype(&MEDIASUBTYPE_MPEG2_AUDIO);
        wfx.wFormatTag = WAVE_FORMAT_MPEG;
    }
    else
    {
        m_mediaType.SetSubtype(&MEDIASUBTYPE_MP3);
        wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
    }
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = 1;
    wfx.nChannels = channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.nAvgBytesPerSec = bitrate / 8;
    m_mediaType.SetFormatType(&FORMAT_WaveFormatEx);        
    if(!m_mediaType.SetFormat(reinterpret_cast<BYTE *>(&wfx), sizeof(WAVEFORMATEX)))
    {
        debugPrintf(DBG, L"MP3ElementaryType::MP3ElementaryType: unable to set format\r\n");
        return;
    }
}

bool MP3ElementaryType::GetType(CMediaType* pmt, int nType) const
{
    debugPrintf(DBG, L"MP3ElementaryType::GetType\r\n");
    if(nType != 0)
        return false;
    *pmt = m_mediaType;
    return true;
}

void MP3ElementaryType::setHandler(const CMediaType* pmt, int idx)
{
    if(!m_pHandler)
        m_pHandler = new NoChangeHandler();
}
