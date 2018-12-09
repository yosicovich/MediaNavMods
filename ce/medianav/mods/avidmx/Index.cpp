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

AviSampleSizes::AviSampleSizes()
: SampleSizes(),
  m_totalFrames(0)
{}
     
bool 
AviSampleSizes::Parse(const AVISTREAMHEADER& streamHeader, unsigned int streamIdx, const AVIOLDINDEX* pIndexArray, unsigned int offsetOfOffset)
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
    long keyFramesCount = 0;
    for(unsigned int i = 0; i < indexLength; ++i)
    {
        if(static_cast<WORD>(pIndexArray->aIndex[i].dwChunkId) != indexWord)
            continue;
        long offset = pIndexArray->aIndex[i].dwOffset + sizeof(RIFFCHUNK) + offsetOfOffset;
        long size = pIndexArray->aIndex[i].dwSize;
        bool keyFrame = (pIndexArray->aIndex[i].dwFlags & AVIIF_KEYFRAME) != 0 || allSamplesAreKeys;
        if(keyFrame)
            ++keyFramesCount;
        if(m_nMaxSize < size)
            m_nMaxSize = size;
        long framesPerSample = streamHeader.dwSampleSize == 0 ? 1 : (size / streamHeader.dwSampleSize);
        m_totalFrames += framesPerSample;
        m_samplesArray.push_back(SampleRec(offset, size, framesPerSample, keyFrame));
    }

    m_nSamples = static_cast<long>(m_samplesArray.size());
    debugPrintf(DBG, L"AviSampleSizes::Parse: m_nSamples = %u, keyFramesCount=%u, m_totalFrames=%u\r\n", m_nSamples, keyFramesCount, m_totalFrames);
    return true;
}

long 
AviSampleSizes::Size(long nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].size;
}
                 
LONGLONG 
AviSampleSizes::Offset(long nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].offset;
}

bool AviSampleSizes::isKeyFrame(long nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].keyFrame;
}

long AviSampleSizes::GetSampleFrames(long nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].framesPerSample;
}

// --- sync sample map --------------------------------

AviKeyMap::AviKeyMap(const smart_ptr<SampleSizes>& sampleSizes)
: KeyMap(),
  m_sampleSizes(reinterpret_cast<const smart_ptr<AviSampleSizes>& >(sampleSizes))
{
}

long 
AviKeyMap::SyncFor(long nSample) const
{
    for(;nSample > 0; --nSample)
    {
        if(m_sampleSizes->isKeyFrame(nSample))
            return nSample;
    }
    return 0;
}

long 
AviKeyMap::Next(long nSample) const
{
    for(;nSample < m_sampleSizes->SampleCount(); ++nSample)
    {
        if(m_sampleSizes->isKeyFrame(nSample))
            return nSample;
    }
    return m_sampleSizes->SampleCount() - 1; // Or 0?
}

SIZE_T AviKeyMap::Get(SIZE_T*& pnIndexes) const
{
	return 0;
}

// ----- times index ----------------------------------

AviSampleTimes::AviSampleTimes(const AVISTREAMHEADER& streamHeader, const smart_ptr<SampleSizes>& sampleSizes)
: SampleTimes(),
  m_sampleSizes(reinterpret_cast<const smart_ptr<AviSampleSizes>& >(sampleSizes)),
  m_oneFramePerSample(false)
{  
    m_scale = streamHeader.dwScale;
    m_rate = streamHeader.dwRate;
    m_start = streamHeader.dwStart;
    m_total = TrackToReftime(m_sampleSizes->TotalFrames());
    m_oneFramePerSample = streamHeader.dwSampleSize == 0;
    debugPrintf(DBG, L"AviSampleTimes::AviSampleTimes: m_scale=%u, m_rate=%u, m_start=%u, m_total=%I64u, streamHeader.dwSampleSize=%u\r\n", m_scale, m_rate, m_start, m_total, streamHeader.dwSampleSize);
}

long 
AviSampleTimes::DTSToSample(LONGLONG tStart)
{
    long frame = static_cast<long>(ReftimeToTrack(tStart) - m_start);
    if(m_oneFramePerSample)
        return frame;
    long nSample = 0;
    long sampleFrames = 0;
    while((sampleFrames = m_sampleSizes->GetSampleFrames(nSample)) <= frame)
    {
        ++nSample;
        frame -= sampleFrames;
    }
    return nSample;
}

SIZE_T AviSampleTimes::Get(REFERENCE_TIME*& pnTimes) const
{
	return 0;
}

LONGLONG 
AviSampleTimes::SampleToCTS(long nSample)
{
    long frame = 0;
    if(!m_oneFramePerSample)
    {
        for(long i = 0; i < nSample; ++i)
            frame += m_sampleSizes->GetSampleFrames(i);
    }else
    {
        frame = nSample;
    }
    return TrackToReftime(m_start + frame);
}

// offset from decode to composition time for this sample
LONGLONG 
AviSampleTimes::CTSOffset(long nSample) const
{
    // Always 0 for AVI
    return 0;
}

LONGLONG 
AviSampleTimes::Duration(long nSample)
{
    if(!m_oneFramePerSample)
        return TrackToReftime(m_sampleSizes->GetSampleFrames(nSample));
    else
        return TrackToReftime(1);
}
