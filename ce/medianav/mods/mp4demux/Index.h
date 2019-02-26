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

#pragma once

#include "demuxtypes.h"

#pragma warning(disable: 4200)
#pragma pack(push,1)
struct MP4BoxHeader
{
    BYTE version;
    BYTE flags[3];
};
// STTS
struct STTSRec
{
    DWORD sample_count;
    DWORD sample_delta;
};

struct STTS
{
    MP4BoxHeader header;
    DWORD entry_count;
    STTSRec recs[0];
};

// CTTS
struct CTTSRec
{
    DWORD sample_count;
    DWORD sample_offset;
};

struct CTTS
{
    MP4BoxHeader header;
    DWORD entry_count;
    CTTSRec recs[0];
};

// STSZ
struct STSZ
{
    MP4BoxHeader header;
    DWORD sample_size;
    DWORD sample_count;
    DWORD entry_size[0];
};

// STSC
struct STSCRec
{
    DWORD first_chunk;
    DWORD samples_per_chunks;
    DWORD sample_description_index;
};

struct STSC
{
    MP4BoxHeader header;
    DWORD entry_count;
    STSCRec recs[0];
};

// STCO
struct STCO
{
    MP4BoxHeader header;
    DWORD entry_count;
    DWORD chunk_offset[0];
};

// CO64
struct CO64
{
    MP4BoxHeader header;
    DWORD entry_count;
    ULONGLONG chunk_offset[0];
};

// STSS
struct STSS
{
    MP4BoxHeader header;
    DWORD entry_count;
    DWORD sample_number[0];
};

#pragma pack(pop)
// currently all index tables are kept in memory. This is
// typically a few hundred kilobytes. For very large files a
// more sophisticated scheme might be worth considering?

// index giving count of samples and size of each sample
// and file location of sample
class Mpeg4TrackIndex: public TrackIndex
{
public:
    Mpeg4TrackIndex();

    bool Parse(DWORD scale, LONGLONG CTOffset, const AtomPtr& patmSTBL);
    DWORD Size(DWORD nSample) const;
    LONGLONG Offset(DWORD nSample) const;
    DWORD Next(DWORD nSample) const;

    DWORD SyncFor(DWORD nSample) const;
    DWORD NextSync(DWORD nSample) const;

    DWORD CTSToSample(LONGLONG nSample) const;
    DWORD DTSToSample(LONGLONG tStart) const;
    LONGLONG SampleToCTS(DWORD nSample) const;
    LONGLONG Duration(DWORD nSample) const;

	// support for old-style uncompressed audio, where fixedsize =1 means 1 sample
	void AdjustFixedSize(DWORD nBytes);

private:
    DWORD m_nFixedSize;   
    DWORD m_nMaxSamplePerChunk;

    LONGLONG m_CTOffset;        // CT offset of first sample
    DWORD m_nFixedDuration;

    struct SampleRec
    {
        SampleRec(DWORD size)
            :offset(0), size(size), framesPerSample(0), totalFramesSoFar(0), ctsOffset(0),keyFrameSample(0)
        {
        }

        ULONGLONG offset;
        DWORD size;
        DWORD framesPerSample;
        DWORD totalFramesSoFar;
        DWORD ctsOffset;
        DWORD keyFrameSample;
    };
    typedef std::vector<SampleRec> SamplesArray;

    SamplesArray m_samplesArray;

};
