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

Mpeg4TrackIndex::Mpeg4TrackIndex()
: TrackIndex()
,m_nFixedSize(0)
,m_nMaxSamplePerChunk(0)
,m_nFixedDuration(0)
{}
     
bool 
Mpeg4TrackIndex::Parse(DWORD scale, LONGLONG CTOffset, const AtomPtr& patmSTBL)
{
    // scale - track timescale units/sec
    m_rate = scale;            
    m_scale = 1;

    m_CTOffset = CTOffset;      // offset to start of first sample in 100ns

    // need to locate three inter-related tables
    // stsz, stsc and stco/co64

    const AtomPtr& patmSTSZ = patmSTBL->FindChild(FOURCC("stsz"));
    if (!patmSTSZ)
    {
        return false;
    }
    AtomCache cacheSTSZ = patmSTSZ;
    const STSZ* pSTSZ = reinterpret_cast<const STSZ *>(*cacheSTSZ);
    if (pSTSZ->header.version != 0)  // check version 0
    {
        return false;
    }

    m_nFixedSize = SwapLong(pSTSZ->sample_size);
    m_nSamples = SwapLong(pSTSZ->sample_count);

    m_samplesArray.reserve(m_nSamples);

    // Populate sample sizes and find out the largest one.
    m_nMaxSize = m_nFixedSize;
    for (DWORD  i = 0; i < m_nSamples; i++)
    {
        DWORD sampleSize = m_nFixedSize ? m_nFixedSize : SwapLong(pSTSZ->entry_size[i]);
        m_samplesArray.push_back(SampleRec(sampleSize));
        if(sampleSize > m_nMaxSize)
            m_nMaxSize = sampleSize;
    }

    const AtomPtr& patmSTSC = patmSTBL->FindChild(FOURCC("stsc"));
    if (!patmSTSC)
    {
        return false;
    }
    AtomCache cacheSTSC = patmSTSC;
    const STSC* pSTSC = reinterpret_cast<const STSC *>(*cacheSTSC);
    DWORD nEntriesSTSC = SwapLong(pSTSC->entry_count);
    
    AtomPtr& patmSTCO = patmSTBL->FindChild(FOURCC("stco"));
    bool bCO64;
    if (patmSTCO != NULL)
    {
        bCO64 = false;
    } else {
        patmSTCO = patmSTBL->FindChild(FOURCC("co64"));
        if (!patmSTCO)
        {
            return false;
        }
        bCO64 = true;
    }
    AtomCache cacheSTCO = patmSTCO;
    DWORD nChunks = SwapLong(reinterpret_cast<const STCO*>(*cacheSTCO)->entry_count);

    // Populate offsets
    DWORD nCurSample = 0;
    for (DWORD  i = 0; i < nEntriesSTSC; i++)
    {
        DWORD first_chunk = SwapLong(pSTSC->recs[i].first_chunk) - 1;
        DWORD samplesPerChunk = SwapLong(pSTSC->recs[i].samples_per_chunks);
        DWORD next_first_chunk = i == nEntriesSTSC - 1 ? nChunks : SwapLong(pSTSC->recs[i + 1].first_chunk) - 1;

        if(first_chunk >= nChunks || next_first_chunk > nChunks)
            return false;

        for(DWORD u = first_chunk; u < next_first_chunk; ++u)
        {
            ULONGLONG chunkOffset = bCO64 ? SwapI64(reinterpret_cast<const CO64*>(*cacheSTCO)->chunk_offset[u]) : SwapLong(reinterpret_cast<const STCO*>(*cacheSTCO)->chunk_offset[u]);
            for(DWORD j = 0; j < samplesPerChunk; ++j)            
            {
                if(nCurSample >= m_nSamples)
                {
                    debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: STCO currupted!\r\n");
                    return false;
                }
                m_samplesArray[nCurSample].offset = chunkOffset;
                chunkOffset+=m_samplesArray[nCurSample].size;
                ++nCurSample;
            }
        }
    }

    if(nCurSample != m_nSamples)
    {
        debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: STCO or STSC currupted, nCurSample = %u != m_nSamples = %u !\r\n", nCurSample, m_nSamples);
        return false;
    }

    // Populate key samples
    const AtomPtr& patmSTSS = patmSTBL->FindChild(FOURCC("stss"));
    if (patmSTSS != NULL)
    {
        AtomCache cacheSTSS = patmSTSS;
        const STSS* pSTSS = reinterpret_cast<const STSS*>(*cacheSTSS);
        if (pSTSS->header.version != 0)  // check version 0
        {
            return false;
        }

        DWORD nCurSample = 0;
        DWORD nCount = SwapLong(pSTSS->entry_count);
        for(DWORD i = 0; i < nCount; ++i)
        {
            DWORD nKeySample = SwapLong(pSTSS->sample_number[i]) - 1;
            for(DWORD u=nCurSample; u < nKeySample && u < m_nSamples; ++u)
            {
                m_samplesArray[u].keyFrameSample = nCurSample;
            }
            
            nCurSample = nKeySample;

            if(i == nCount - 1)
            {
                // The last one
                for(DWORD u=nCurSample; u < m_nSamples; ++u)
                {
                    m_samplesArray[u].keyFrameSample = nCurSample;
                }
            }
        }
    }else
    {
        // no key map -- so all samples are key
        for(DWORD i = 0; i < m_nSamples; ++i)
            m_samplesArray[i].keyFrameSample = i;

    }

    // basic duration table
    const AtomPtr& patmSTTS = patmSTBL->FindChild(FOURCC("stts"));
    if (!patmSTTS)
    {
        return false;
    }

    AtomCache cacheSTTS = patmSTTS;
    const STTS* pStts = reinterpret_cast<const STTS*>(*cacheSTTS);

    DWORD nEntriesSTTS = SwapLong(pStts->entry_count);
    
    DWORD total = 0;

    nCurSample = 0;
    for (DWORD i = 0; i < nEntriesSTTS; i++)
    {
        DWORD nEntries = SwapLong(pStts->recs[i].sample_count);
        DWORD nDuration = SwapLong(pStts->recs[i].sample_delta);

        if(m_nMaxSamplePerChunk < nDuration)
            m_nMaxSamplePerChunk = nDuration;

        for(DWORD u = 0; u < nEntries; ++u)
        {
            if(nCurSample >= m_nSamples)
            {
                debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: STTS currupted!\r\n");
                return false;
            }
            m_samplesArray[nCurSample].framesPerSample = nDuration;
            m_samplesArray[nCurSample++].totalFramesSoFar = total;
            total += nDuration;
        }
    }

    if(nCurSample != m_nSamples)
    {
        debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: STTS currupted, nCurSample = %u != m_nSamples = %u !\r\n", nCurSample, m_nSamples);
        return false;
    }

    m_total = TrackToReftime(total);

    // Optimization
    if(SwapLong(pStts->entry_count) == 1 && SwapLong(pStts->recs[0].sample_count) == m_nSamples)
    {
        // Fixed duration case;
        m_nFixedDuration = SwapLong(pStts->recs[0].sample_delta);
    }

    //TODO: Check whatever we have to build frameSoFar analog using cts offsets or this offset is applied on per sample basis only?
    // optional decode-to-composition offset table
    const AtomPtr& patmCTTS = patmSTBL->FindChild(FOURCC("ctts"));
    if (patmCTTS != NULL)
    {
        debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: Track decode-to-composition offset table - CTTS!\r\n");
        AtomCache cacheCTTS = patmCTTS;
        const CTTS* pCTTS = reinterpret_cast<const CTTS*>(*cacheCTTS);
        nCurSample = 0;
        for (DWORD i = 0; i < SwapLong(pCTTS->entry_count); i++)
        {
            DWORD nEntries = SwapLong(pCTTS->recs[i].sample_count);
            DWORD ctsOffset = SwapLong(pCTTS->recs[i].sample_offset);
            for(DWORD u = 0; u < nEntries; ++u)
            {
                if(nCurSample >= m_nSamples)
                {
                    debugPrintf(DBG, L"Mpeg4TrackIndex::Parse: CTTS currupted!\r\n");
                    return false;
                }
                m_samplesArray[nCurSample++].ctsOffset = ctsOffset;
            }
        }
    }

    return true;
}

DWORD 
Mpeg4TrackIndex::Size(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].size;
}
                 
LONGLONG 
Mpeg4TrackIndex::Offset(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].offset;
}

void 
Mpeg4TrackIndex::AdjustFixedSize(DWORD nBytes)
{
	m_nFixedSize = nBytes;

    // set max buffer size based on contig samples
    if(m_nMaxSize < (m_nMaxSamplePerChunk * m_nFixedSize))
        m_nMaxSize = m_nMaxSamplePerChunk * m_nFixedSize;
}

DWORD 
Mpeg4TrackIndex::SyncFor(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_samplesArray[nSample].keyFrameSample;
}

DWORD 
Mpeg4TrackIndex::Next(DWORD nSample) const
{
    for(;nSample < m_nSamples; ++nSample)
    {
        if(m_samplesArray[nSample].keyFrameSample == nSample)
            return nSample;
    }
    return m_nSamples - 1; // Or 0?
}

SIZE_T Mpeg4TrackIndex::Get(SIZE_T*& pnIndexes) const
{
    return 0;
}

DWORD 
Mpeg4TrackIndex::CTSToSample(LONGLONG tStart) const
{
    // Does it make sense to use this at all?
    DWORD frame = static_cast<DWORD>(ReftimeToTrack(tStart - m_CTOffset));
    DWORD nSample = 0;
    while(nSample < m_nSamples && (m_samplesArray[nSample].totalFramesSoFar - m_samplesArray[nSample].ctsOffset + m_samplesArray[nSample].framesPerSample) <= frame)
    {
        ++nSample;
    }
    return nSample;
}

DWORD 
Mpeg4TrackIndex::DTSToSample(LONGLONG tStart) const
{
    DWORD frame = static_cast<DWORD>(ReftimeToTrack(tStart - m_CTOffset));
    if(m_nFixedSize)
        return frame / m_nFixedSize;
    DWORD nSample = 0;
    while(nSample < m_nSamples && (m_samplesArray[nSample].totalFramesSoFar + m_samplesArray[nSample].framesPerSample) <= frame)
    {
        ++nSample;
    }
    return nSample;
}

SIZE_T Mpeg4TrackIndex::Get(REFERENCE_TIME*& pnTimes) const
{
	return 0;
}

LONGLONG 
Mpeg4TrackIndex::SampleToCTS(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    return m_CTOffset + TrackToReftime(m_samplesArray[nSample].totalFramesSoFar + m_samplesArray[nSample].ctsOffset);
}

LONGLONG 
Mpeg4TrackIndex::Duration(DWORD nSample) const
{
    if(nSample >= m_nSamples)
        nSample = m_nSamples - 1;

    if(!m_nFixedDuration)
        return TrackToReftime(m_samplesArray[nSample].framesPerSample);
    else
        return TrackToReftime(m_nFixedDuration);

}
