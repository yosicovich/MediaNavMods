//
// Index.h: declarations of classes for index management of mpeg-4 files.
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Index.h"

// sample count and sizes ------------------------------------------------
/* Macro to make a numeric part of ckid for a chunk out of a stream number
** from 0-255.
*/
#define ToHex(n)	((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#define MAKEAVICKIDSTREAM(stream) MAKEWORD(ToHex(((stream) & 0xf0) >> 4), ToHex((stream) & 0x0f))

AviTrackIndex::AviTrackIndex()
: TrackIndex(),
  m_totalFrames(0),
  m_start(0),
  m_oneFramePerSample(false)
{}
     
bool 
AviTrackIndex::Parse(const AVISTREAMHEADER& streamHeader, unsigned int streamIdx, const AVIOLDINDEX* pIndexArray, unsigned int offsetOfOffset)
{
    unsigned int indexLength = pIndexArray->cb / sizeof(struct AVIOLDINDEX::_avioldindex_entry);
    if(streamHeader.dwSampleSize == 0)
    {
        // One chunk - on sample
        m_samplesArray.reserve(streamHeader.dwLength);
    }else
    {
        // One chunk - many samples, so index size/2 is a good approximation
        m_samplesArray.reserve(indexLength / 2);
    }
    
    // All samples are keys in auds streams.
    bool allSamplesAreKeys = streamHeader.fccType == streamtypeAUDIO;
    m_nMaxSize = 0;
    m_totalFrames = 0;

    WORD indexWord = MAKEAVICKIDSTREAM(streamIdx);
    debugPrintf(DBG, L"AviSampleSizes::Parse: indexLength = %u, indexWord = %hu\r\n", indexLength, indexWord);
    DWORD keySamplesCount = 0;
    DWORD curKeyFrameSample = 0;
    DWORD nCurSample = 0;
    for(unsigned int i = 0; i < indexLength; ++i)
    {
        if(static_cast<WORD>(pIndexArray->aIndex[i].dwChunkId) != indexWord)
            continue;
        DWORD offset = pIndexArray->aIndex[i].dwOffset + sizeof(RIFFCHUNK) + offsetOfOffset;
        DWORD size = pIndexArray->aIndex[i].dwSize;
        bool keyFrame = (pIndexArray->aIndex[i].dwFlags & AVIIF_KEYFRAME) != 0 || allSamplesAreKeys;
        if(keyFrame)
        {
            ++keySamplesCount;
            curKeyFrameSample = nCurSample;
        }
        if(m_nMaxSize < size)
            m_nMaxSize = size;
        DWORD framesPerSample = streamHeader.dwSampleSize == 0 ? 1 : (size / streamHeader.dwSampleSize);
        m_samplesArray.push_back(SampleRec(offset, size, framesPerSample, m_totalFrames, curKeyFrameSample));
        m_totalFrames += framesPerSample;
        ++nCurSample;
    }

    m_nSamples = static_cast<DWORD>(m_samplesArray.size());

    m_scale = streamHeader.dwScale;
    m_rate = streamHeader.dwRate;
    m_start = streamHeader.dwStart;
    m_total = TrackToReftime(m_totalFrames);
    m_oneFramePerSample = streamHeader.dwSampleSize == 0;

    debugPrintf(DBG, L"AviTrackIndex::Parse: \r\n\t m_nSamples = %u\r\n\t keySamplesCount=%u\r\n\t m_totalFrames=%u\r\n\t m_scale=%u\r\n\t m_rate=%u\r\n\t m_start=%u\r\n\t m_total=%I64u\r\n\t streamHeader.dwSampleSize=%u\r\n", m_nSamples, keySamplesCount, m_totalFrames, m_scale, m_rate, m_start, m_total, streamHeader.dwSampleSize);
    return true;
}

DWORD 
AviTrackIndex::Size(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].size;
}
                 
LONGLONG 
AviTrackIndex::Offset(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].offset;
}

DWORD 
AviTrackIndex::SyncFor(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].keyFrameSample;
}

DWORD 
AviTrackIndex::Next(DWORD nSample) const
{
    for(;nSample < m_nSamples; ++nSample)
    {
        if(m_samplesArray[nSample].keyFrameSample == nSample)
            return nSample;
    }
    return m_nSamples - 1; // Or 0?
}

SIZE_T AviTrackIndex::Get(SIZE_T*& pnIndexes) const
{
	return 0;
}

DWORD 
AviTrackIndex::DTSToSample(LONGLONG tStart) const
{
    DWORD frame = static_cast<DWORD>(ReftimeToTrack(tStart) - m_start);
    if(m_oneFramePerSample)
        return frame;
    DWORD nSample = 0;
    while(nSample < m_nSamples && (m_samplesArray[nSample].totalFramesSoFar + m_samplesArray[nSample].framesPerSample) <= frame)
    {
        ++nSample;
    }
    return nSample;
}

SIZE_T AviTrackIndex::Get(REFERENCE_TIME*& pnTimes) const
{
	return 0;
}

LONGLONG 
AviTrackIndex::SampleToCTS(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    DWORD frame = 0;
    if(!m_oneFramePerSample)
    {
        frame = m_samplesArray[nSample].totalFramesSoFar;
    }else
    {
        frame = nSample;
    }
    return TrackToReftime(m_start + frame);
}

LONGLONG 
AviTrackIndex::Duration(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    if(!m_oneFramePerSample)
        return TrackToReftime(m_samplesArray[nSample].framesPerSample);
    else
        return TrackToReftime(1);
}
