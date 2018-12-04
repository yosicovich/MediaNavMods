//
// demuxtypes.h: Common demux types
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk
//////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include <memory>
#include <utils.h>

class Atom;
typedef smart_ptr<Atom> AtomPtr;

// abstract interface that is implemented by the supplier of data,
// e.g. by the filter's input pin
class AtomReader
{
public:
    virtual ~AtomReader() {}

    virtual HRESULT Read(LONGLONG llOffset, long cBytes, void* pBuffer) = 0;
    virtual LONGLONG Length() = 0;

    // support for caching in memory. 
    virtual bool IsBuffered() = 0;

    // calls to Buffer and BufferRelease are refcounted and should correspond.
    virtual const BYTE* Buffer() = 0;
    virtual void BufferRelease() = 0;
};


// MPEG-4 files (based on QuickTime) are comprised of file elements
// with a length and 4-byte FOURCC type code. This basic header can
// be extended with an 8-byte length and a 16-byte type GUID. 
// 
// The payload can contain other atoms recursively.
//
// Some atoms begin with a 1-byte version number and 3-byte flags field,
// but as this is type-specific, we regard it as part of the payload.
//
// This Atom implementation accesses data
// via the AtomReader abstraction. The data source may be
// a containing atom or access to the file (perhaps via an input pin).
class Atom : public AtomReader
{
public:
    // all the header params are parsed in the "Child" method and passed
    // to the constructor. This means we can use the same class for the outer file
    // container (which does not have a header).
    Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type, long cHeader, bool canHaveChildren = true);
    virtual ~Atom() {debugPrintf(DEMUX_DBG, L"Atom::~Atom(): %08X\r\n", Type());}

    virtual HRESULT Read(LONGLONG llOffset, long cBytes, void* pBuffer);
    virtual LONGLONG Length()
    {
        return m_llLength;
    }
    virtual DWORD Type()
    {
        return m_type;
    }

    // size of size and type fields (including extended size and guid type if present)
    virtual long HeaderSize()
    {
        return m_cHeader;
    }

    virtual LONGLONG Offset() const
    {
        return m_llOffset;
    }

    virtual long ChildCount();

    // these methods return a pointer to an Atom object that is contained within
    // this Atom -- so do not delete.
    virtual Atom* Child(long nChild);
    virtual Atom* FindChild(DWORD fourcc);
    virtual Atom* FindNextChild(Atom* after, DWORD fourcc);

    virtual bool IsBuffered();
    // calls to Buffer and BufferRelease are refcounted and should correspond.
    virtual const BYTE* Buffer();
    virtual void BufferRelease();

    // call this if the child items do not start immediately after the header
    // -- llOffset is an offset from HeaderSize
    virtual void ScanChildrenAt(LONGLONG llOffset) = 0;

protected:
    long m_cHeader;
    LONGLONG m_llOffset;
    LONGLONG m_llLength;
    // list of children
    list<AtomPtr> m_Children;

private:
    AtomReader* m_pSource;
    DWORD m_type;

    // caching
    smart_array<BYTE> m_Buffer;
    long m_cBufferRefCount;
    bool m_bChildrenScaned;
};

// this class is a simple way to manage the Buffer/BufferRelease
// caching scheme for an Atom
class AtomCache
{
public:
    AtomCache(Atom* patm = NULL)
    : m_pAtom(patm)
    {
		if (patm != NULL)
		{
			m_pBuffer = patm->Buffer() + patm->HeaderSize();
		}
    }
    ~AtomCache()
    {
		if (m_pAtom != NULL)
		{
			m_pAtom->BufferRelease();
		}
    }
    const AtomCache& operator=(Atom* patm)
    {
        if (m_pAtom != NULL)
        {
            m_pAtom->BufferRelease();
        }
        m_pAtom = patm;
        if (m_pAtom != NULL)
        {
            m_pBuffer = patm->Buffer() + patm->HeaderSize();
        }
        return *this;
    }


    const BYTE* operator->() const 
    {
        return m_pBuffer;
    }
    BYTE operator[](int idx) const
    {
        return m_pBuffer[idx];
    }

    operator const BYTE*() const
    {
        return m_pBuffer;
    }
	
    const BYTE* operator *() const
    {
        return m_pBuffer;
    }

    const BYTE* getRawBuffer() const
    {
        return m_pBuffer - m_pAtom->HeaderSize();
    }

    const LONGLONG getDataSize() const
    {
        return m_pAtom->Length() - m_pAtom->HeaderSize();
    }
private:
    Atom* m_pAtom;
    const BYTE* m_pBuffer;
};

// index giving count of samples and size of each sample
// and file location of sample
class SampleSizes
{
public:
    SampleSizes();
    
    virtual long Size(long nSample) const = 0;
    long SampleCount()  const
    {
        return m_nSamples;
    }
    long MaxSize() const
    {
        return m_nMaxSize;
    }
    virtual LONGLONG Offset(long nSample) const = 0;
protected:
    long m_nSamples;
    long m_nMaxSize;
};

// map of key samples
class KeyMap
{
public:
    virtual long SyncFor(long nSample) const = 0;
    virtual long Next(long nSample) const = 0;
    virtual SIZE_T Get(SIZE_T*& pnIndexes) const = 0;
};

// time and duration of samples
// -- all times in 100ns units
class SampleTimes
{
public:
    SampleTimes();

    virtual long DTSToSample(LONGLONG tStart) = 0;
    virtual SIZE_T Get(REFERENCE_TIME*& pnTimes) const = 0;
    virtual LONGLONG SampleToCTS(long nSample) = 0;
    virtual LONGLONG Duration(long nSample) = 0;
    virtual LONGLONG CTSOffset(long nSample) const = 0;
    long CTSToSample(LONGLONG tStart);
    LONGLONG TotalDuration()    { return m_total; }

    LONGLONG TrackToReftime(LONGLONG nTrack) const;
    virtual bool HasCTSTable() const = 0;
    LONGLONG ReftimeToTrack(LONGLONG reftime) const;

protected:
    long m_scale;
    long m_rate;
    LONGLONG m_total;       // sum of durations, in reftime
};

// --- movie and track headers ---------------------------

class Movie;

struct EditEntry
{
	LONGLONG duration;
	LONGLONG offset;
	LONGLONG sumDurations;
};

// abstract interface for format handler, responsible
// for creating a specific output stream 
class FormatHandler
{
public:
    virtual ~FormatHandler() {}

    virtual long BufferSize(long MaxSize) = 0;
    virtual void StartStream() = 0;
    virtual long PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes) = 0;
};

// conversion from elementary stream descriptors to
// DirectShow media type
//
// We can offer multiple type definitions for a given stream,
// and support a format handler object specific to the type chosen
// to create the output stream.
class ElementaryType  
{
public:
    ElementaryType();

    virtual bool IsVideo() const = 0;
    string ShortName()
    {
        return m_shortname;
    }
    virtual bool GetType(CMediaType* pmt, int nType) const = 0;
    virtual void setHandler(const CMediaType* pmt, int idx) = 0;
    bool SetType(const CMediaType* pmt);
    FormatHandler* Handler() 
    {
        return m_pHandler.get();
    }

protected:
    std::auto_ptr<FormatHandler> m_pHandler;
    string m_shortname;
    CMediaType m_mtChosen;
};

class MovieTrack
{
public:
    MovieTrack(Atom* pAtom, Movie* pMovie, long idx);
    bool Valid()
    {
        return (m_pRoot != NULL);
    }
    const char* Name()
    {
        return m_strName.c_str();
    }
    bool IsVideo();
    bool GetType(CMediaType* pmt, int nType);
    bool SetType(const CMediaType* pmt);
    FormatHandler* Handler();

    SampleSizes* SizeIndex()
    {
        return m_pSizes;
    }
    KeyMap* GetKeyMap()
    {
        return m_pKeyMap;
    }
    SampleTimes* TimesIndex()
    {
        return m_pTimes;
    }
    Movie* GetMovie() const
    {
        return m_pMovie;
    }
    virtual REFERENCE_TIME Duration() const = 0;
    HRESULT ReadSample(long nSample, BYTE* pBuffer, long cBytes);
	bool IsOldAudioFormat()		{ return m_bOldFixedAudio; }
	bool CheckInSegment(REFERENCE_TIME tNext, bool bSyncBefore, size_t* pnSegment, long* pnSample);
	void GetTimeBySegment(long nSample, size_t segment, REFERENCE_TIME* ptStart, REFERENCE_TIME* pDuration);
	bool NextBySegment(long* pnSample, size_t* psegment);
	SIZE_T GetTimes(REFERENCE_TIME** ppnStartTimes, REFERENCE_TIME** ppnStopTimes, ULONG** ppnFlags, ULONG** ppnDataSizes);

protected:
    Atom* m_pRoot;
    Movie* m_pMovie;
    long m_idx;
    string m_strName;

    smart_ptr<ElementaryType> m_pType;
    smart_ptr<SampleSizes> m_pSizes;
    smart_ptr<KeyMap> m_pKeyMap;
    smart_ptr<SampleTimes> m_pTimes;
    bool m_bOldFixedAudio;

    vector<EditEntry> m_Edits;
};
typedef smart_ptr<MovieTrack> MovieTrackPtr;

class Movie
{
public:
    Movie(Atom* pRoot);
    long Tracks()
    {
        return (long)m_Tracks.size();
    }
    MovieTrack* Track(long nTrack) const
    {
        return m_Tracks[nTrack];
    }
        
    HRESULT ReadAbsolute(LONGLONG llPos, BYTE* pBuffer, long cBytes);

protected:
    smart_ptr<Atom> m_pRoot;
    vector<MovieTrackPtr> m_Tracks;
};





