//
// Index.h
//

#pragma once

#include "demuxtypes.h"
class MP3TrackIndex: public TrackIndex
{
public:
    MP3TrackIndex(LONGLONG offset, DWORD frameSamples, DWORD sampleRate, DWORD frameSize, DWORD frames, DWORD joinCount = 1);

    DWORD Size(DWORD nSample) const;
    LONGLONG Offset(DWORD nSample) const;
    DWORD Next(DWORD nSample) const;

    DWORD SyncFor(DWORD nSample) const;
    DWORD NextSync(DWORD nSample) const;

    DWORD CTSToSample(LONGLONG tStart) const { return DTSToSample(tStart);}
    DWORD DTSToSample(LONGLONG tStart) const;
    LONGLONG SampleToCTS(DWORD nSample) const;
    LONGLONG Duration(DWORD nSample) const;

private:
    REFERENCE_TIME m_oneFrameDuration;
    LONGLONG m_offset;
    DWORD m_joinCount;
    DWORD m_oneFrameSize;

};
