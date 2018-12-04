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

// currently all index tables are kept in memory. This is
// typically a few hundred kilobytes. For very large files a
// more sophisticated scheme might be worth considering?

// index giving count of samples and size of each sample
// and file location of sample
class Mpeg4SampleSizes: public SampleSizes
{
public:
    Mpeg4SampleSizes();

    bool Parse(Atom* patmSTBL);
    long Size(long nSample) const;
    LONGLONG Offset(long nSample) const;

	// support for old-style uncompressed audio, where fixedsize =1 means 1 sample
	void AdjustFixedSize(long nBytes);
private:
    Atom* m_patmSTSZ;
	AtomCache m_pBuffer;
    long m_nFixedSize;
    
    long m_nEntriesSTSC;
    long m_nChunks;
    bool m_bCO64;
    Atom* m_patmSTSC;
	AtomCache m_pSTSC;
    Atom* m_patmSTCO;
	AtomCache m_pSTCO;
};

// map of key samples
class Mpeg4KeyMap: public KeyMap
{
public:
    Mpeg4KeyMap();
    ~Mpeg4KeyMap();

    bool Parse(Atom* patmSTBL);
    long SyncFor(long nSample) const;
	long Next(long nSample) const;
	SIZE_T Get(SIZE_T*& pnIndexes) const;

private:
    Atom* m_patmSTSS;
    const BYTE* m_pSTSS;
    long m_nEntries;
};

// time and duration of samples
// -- all times in 100ns units
class Mpeg4SampleTimes: public SampleTimes
{
public:
    Mpeg4SampleTimes();

	bool Parse(long scale, LONGLONG CTOffset, Atom* patmSTBL);

    long DTSToSample(LONGLONG tStart);
	SIZE_T Get(REFERENCE_TIME*& pnTimes) const;
    LONGLONG SampleToCTS(long nSample);
    LONGLONG Duration(long nSample);
    LONGLONG CTSOffset(long nSample) const;

    bool HasCTSTable() const { return m_nCTTS > 0; }

private:
    LONGLONG m_CTOffset;        // CT offset of first sample

    Atom* m_patmSTTS;
    Atom* m_patmCTTS;
    AtomCache m_pSTTS;
    AtomCache m_pCTTS;

    long m_nSTTS;
    long m_nCTTS;

    // Duration, DTSToSample and SampleToCTS need to
    // add up durations from the start of the table. We
    // cache the current position to reduce effort
    long m_nBaseSample;     // sample number at m_idx
    long m_idx;             // table index corresponding to m_nBaseSample

    LONGLONG m_tAtBase;     // total of durations at m_nBaseSample
};


