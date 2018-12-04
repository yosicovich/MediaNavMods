
#pragma once

#include "stdafx.h"
#include "demuxtypes.h"
#include <utils.h>

class NoChangeHandler : public FormatHandler
{
public:
    long BufferSize(long MaxSize);
    void StartStream();
    long PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes);
};

// for CoreAAC, minimum buffer size is 8192 bytes.
class CoreAACHandler : public NoChangeHandler
{
public:
    long BufferSize(long MaxSize)
    {
        if (MaxSize < 8192)
        {
            MaxSize = 8192;
        }
        return MaxSize;
    }
};

// for DivX, need to prepend data to the first buffer
// from the media type
class DivxHandler : public NoChangeHandler
{
public:
    DivxHandler(const BYTE* pDSI, long cDSI);
    long BufferSize(long MaxSize);
    void StartStream();
    long PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes);
private:
    smart_array<BYTE> m_pPrepend;
    long m_cBytes;
    bool m_bFirst;
};

// for H624 byte stream, need to re-insert the 00 00 01 start codes

class H264ByteStreamHandler : public NoChangeHandler
{
public:
    H264ByteStreamHandler(const BYTE* pDSI, long cDSI);
    void StartStream()
    {
        m_bFirst = true;
    }
    long BufferSize(long MaxSize)
    {
        // we need to add 00 00 00 01 for each NALU. There
        // could potentially be several NALUs for each frame. Assume a max of 12.
        if (m_cLength < 4)
        {		
            MaxSize += (12 * (4 - m_cLength));
        }
        return MaxSize + m_cPrepend;
    }
    long PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes);

private:
    long m_cLength;
    smart_array<BYTE> m_pPrepend;
    int m_cPrepend;
    bool m_bFirst;
};

