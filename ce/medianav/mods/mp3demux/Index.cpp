//
// Index.cpp
//

#include "stdafx.h"
#include "Index.h"

MP3TrackIndex::MP3TrackIndex(LONGLONG offset, DWORD frameSamples, DWORD sampleRate, DWORD frameSize, DWORD frames, DWORD joinCount)
: TrackIndex()
,m_offset(offset)
,m_joinCount(joinCount)
{
    m_scale = frameSamples;
    m_rate = sampleRate;
    m_oneFrameSize = frameSize;
    m_nMaxSize = m_oneFrameSize * m_joinCount;
    m_nSamples = frames;
    m_total = TrackToReftime(m_nSamples);
    m_oneFrameDuration = TrackToReftime(1);
}

DWORD MP3TrackIndex::Size(DWORD nSample) const
{
    if(Next(nSample) >= SampleCount())
    {
        return m_oneFrameSize * (SampleCount() - nSample);
    }else
    {
        return m_nMaxSize;
    }
}

LONGLONG MP3TrackIndex::Offset(DWORD nSample) const
{
    return m_offset + nSample * m_oneFrameSize;
}

DWORD MP3TrackIndex::Next(DWORD nSample) const
{
    return nSample + m_joinCount;
}

DWORD MP3TrackIndex::SyncFor(DWORD nSample) const
{
    return nSample;
}

DWORD MP3TrackIndex::NextSync(DWORD nSample) const
{
    return Next(nSample);
}

LONGLONG MP3TrackIndex::SampleToCTS(DWORD nSample) const
{
    return TrackToReftime(nSample);
}

LONGLONG MP3TrackIndex::Duration(DWORD nSample) const
{
    if(Next(nSample) >= SampleCount())
    {
        return m_oneFrameDuration * (SampleCount() - nSample);
    }else
    {
        return m_oneFrameDuration * m_joinCount;
    }
}

DWORD MP3TrackIndex::DTSToSample(LONGLONG tStart) const
{
    return static_cast<DWORD>(ReftimeToTrack(tStart));
}
