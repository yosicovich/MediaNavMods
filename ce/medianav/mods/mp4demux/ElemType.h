//
// ElemType.h: declarations of elementary stream type classes.
//
// Media type and other format-specific data that depends
// on the format of the contained elementary streams.
//
// Geraint Davies, April 2004
//
// Copyright (c) GDCL 2004-6. All Rights Reserved. 
// You are free to re-use this as the basis for your own filter development,
// provided you retain this copyright notice in the source.
// http://www.gdcl.co.uk

#pragma once

// descriptors have a tag byte, then a length
// encoded with a "next byte" bit and 7 bit length
//
// This class refers to a buffer held elsewhere

#include "demuxtypes.h"

class Descriptor
{
public:
    Descriptor()
    : m_pBuffer(NULL),
      m_cBytes(0),
      m_cHdr(0),
      m_type(InvalidTag)
    {
    }

    bool Parse(const BYTE* pBuffer, long cBytes);
    
    enum eType
    {
        InvalidTag = 0,
        ES_DescrTag = 3,
        DecoderConfigDescrTag = 4,
        DecSpecificInfoTag = 5,
    };
    eType Type() {
        return m_type;
    }
    const BYTE* Start() {
        return m_pBuffer + Header();
    }
    long Header() {
        return m_cHdr;
    }
    long Length() {
        return m_cBytes;
    }
    bool DescriptorAt(long cOffset, Descriptor& desc);

private:
    eType m_type;
    long m_cHdr;
    long m_cBytes;
    const BYTE* m_pBuffer;
};

// these are the types we currently understand
enum eESType {
	First_Video = 0,
    Video_Mpeg4,
    Video_H264,
	Video_H263,
	Video_FOURCC,
	Video_Mpeg2,
	First_Audio,
    Audio_AAC = First_Audio,
    Audio_WAVEFORMATEX,
	Audio_Mpeg2,
	First_Data,
	Text_CC608,
};

// conversion from elementary stream descriptors to
// DirectShow media type
//
// We can offer multiple type definitions for a given stream,
// and support a format handler object specific to the type chosen
// to create the output stream.
class Mpeg4ElementaryType: public ElementaryType
{
public:
    Mpeg4ElementaryType();

    bool Parse(REFERENCE_TIME tFrame, Atom* patm); // atom should be stsd descriptor mp4v, jvt1, mp4a
	bool IsVideo() const;
    bool GetType(CMediaType* pmt, int nType) const;
    virtual void setHandler(const CMediaType* pmt, int idx);

    void SetRate(REFERENCE_TIME tFrame)
    {
        m_tFrame = tFrame;
    }

	eESType StreamType()
	{
		return m_type;
	}
private:
    bool ParseDescriptor(Atom* patmESD);
    bool GetType_AVCC(CMediaType* pmt) const;
    bool GetType_H264(CMediaType* pmt) const;
	bool GetType_H264ByteStream(CMediaType* pmt) const;
    bool GetType_Mpeg4V(CMediaType* pmt, int n) const;
    bool GetType_AAC(CMediaType* pmt, int n) const;
    bool GetType_WAVEFORMATEX(CMediaType* pmt) const;
	bool GetType_FOURCC(CMediaType* pmt) const;
	bool GetType_JPEG(CMediaType* pmt) const;

private:
    REFERENCE_TIME m_tFrame;
    eESType m_type;
    smart_array<BYTE> m_pDecoderSpecific;
    long m_cDecoderSpecific;

    long m_cx;
    long m_cy;
    static const int SamplingFrequencies[];
    static const GUID* AAC_GUIDS[];

	// fourcc and bitdepth -- for uncompressed or RLE format
	DWORD m_fourcc;
	int m_depth;

};

// --- directshow type info

class DECLSPEC_UUID("000000FF-0000-0010-8000-00AA00389B71") MEDIASUBTYPE_AAC;
class DECLSPEC_UUID("4134504D-0000-0010-8000-00AA00389B71") MEDIASUBTYPE_MP4A;
class DECLSPEC_UUID("00001600-0000-0010-8000-00AA00389B71") MEDIASUBTYPE_MPEG_ADTS_AAC;



