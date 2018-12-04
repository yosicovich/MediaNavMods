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
#include <aviriff.h>

// currently all index tables are kept in memory. This is
// typically a few hundred kilobytes. For very large files a
// more sophisticated scheme might be worth considering?

// index giving count of samples and size of each sample
// and file location of sample
class AviSampleSizes: public SampleSizes
{
public:
    AviSampleSizes();

    bool Parse(const AVISTREAMHEADER& streamHeader, unsigned int streamIdx, const AVIOLDINDEX* pIndexArray, unsigned int offsetOfOffset);
    long Size(long nSample) const;
    LONGLONG Offset(long nSample) const;
    bool isKeyFrame(long nSample) const;
private:
    struct SampleRec
    {
        SampleRec(long offset, long size, bool keyFrame)
            :offset(offset), size(size), keyFrame(keyFrame)
        {
        }
        long offset;
        long size;
        bool keyFrame;
    };
    typedef std::vector<SampleRec> SamplesArray;

    SamplesArray m_samplesArray;

    long m_nFixedSize;
};

// map of key samples
class AviKeyMap: public KeyMap
{
public:
    AviKeyMap(const smart_ptr<SampleSizes>& sampleSizes);

    long SyncFor(long nSample) const;
	long Next(long nSample) const;
	SIZE_T Get(SIZE_T*& pnIndexes) const;

private:
    smart_ptr<AviSampleSizes> m_sampleSizes;
};

// time and duration of samples
// -- all times in 100ns units
class AviSampleTimes: public SampleTimes
{
public:
    AviSampleTimes(const AVISTREAMHEADER& streamHeader);

    long DTSToSample(LONGLONG tStart);
	SIZE_T Get(REFERENCE_TIME*& pnTimes) const;
    LONGLONG SampleToCTS(long nSample);
    LONGLONG Duration(long nSample);
    LONGLONG CTSOffset(long nSample) const;

    bool HasCTSTable() const { return false; }

private:
    long m_start;        // Offset of first sample
};


