
#include "handlers.h"

// ----- format-specific handlers -------------------

// default no-translation
long 
NoChangeHandler::BufferSize(long MaxSize)
{
    return MaxSize;
}

void 
NoChangeHandler::StartStream()
{
}

long 
NoChangeHandler::PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes)
{
    BYTE* pBuffer;
    pSample->GetPointer(&pBuffer);

    if (pMovie->ReadAbsolute(llPos, pBuffer, cBytes) == S_OK)
    {
        return cBytes;
    }
    return 0;
}

// DivX
DivxHandler::DivxHandler(const BYTE* pDSI, long cDSI)
: m_cBytes(0)
{
    // The divx codec requires the stream to start with a VOL
    // header from the DecSpecificInfo. We search for
    // a VOL start code in the form 0x0000012x.
    while (cDSI > 4)
    {
        if ((pDSI[0] == 0) &&
            (pDSI[1] == 0) &&
            (pDSI[2] == 1) &&
            ((pDSI[3] & 0xF0) == 0x20))
        {
            m_cBytes = cDSI;
            m_pPrepend = new BYTE[m_cBytes];
            CopyMemory(m_pPrepend, pDSI,  m_cBytes);
            break;
        }
        pDSI++;
        cDSI--;
    }
}

long 
DivxHandler::BufferSize(long MaxSize)
{
    // we need to prepend the media type data
    // to the first sample, and with seeking, we don't know which
    // that will be.
    return MaxSize + m_cBytes; 
}

void 
DivxHandler::StartStream()
{
    m_bFirst = true;
}

long 
DivxHandler::PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes)
{
    if (m_bFirst)
    {
        m_bFirst = false;

        if (pSample->GetSize() < (cBytes + m_cBytes))
        {
            return 0;
        }

        BYTE* pBuffer;
        pSample->GetPointer(&pBuffer);

        // stuff the VOL header at the start of the stream
        CopyMemory(pBuffer,  m_pPrepend,  m_cBytes);
        pBuffer += m_cBytes;
        if (pMovie->ReadAbsolute(llPos, pBuffer, cBytes) != S_OK)
        {
            return 0;
        }
        return m_cBytes + cBytes;

    } else {
        return NoChangeHandler::PrepareOutput(pSample, pMovie, llPos, cBytes);
    }
}

// H264
H264ByteStreamHandler::H264ByteStreamHandler(const BYTE* pDSI, long cDSI)
: m_cLength(0),
m_cPrepend(cDSI)
{
    m_pPrepend = new BYTE[m_cPrepend];
    CopyMemory(m_pPrepend, pDSI, m_cPrepend);
    if (cDSI > 4)
    {
        m_cLength = (pDSI[4] & 3) + 1;
    }
}

int ReadMSW(const BYTE* p)
{
    return (p[0] << 8) + (p[1]);
}

// convert length-preceded format to start code format
long
H264ByteStreamHandler::PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes)
{
    // check that length field is in valid range
    if ((m_cLength == 0) || (m_cLength > 5))
    {
        return 0;
    }

    BYTE* pDest;
    pSample->GetPointer(&pDest);
    long cRemain = pSample->GetSize();

    if (m_bFirst)
    {
        m_bFirst = false;

        const BYTE* pSrc = m_pPrepend + 6;
        int cSPS = m_pPrepend[5] & 0x1f;
        while (cSPS--)
        {
            int c = ReadMSW(pSrc);
            pSrc += 2;
            pDest[0] = 0;
            pDest[1] = 0;
            pDest[2] = 0;
            pDest[3] = 1;
            pDest += 4;
            cRemain -= 4;

            CopyMemory(pDest, pSrc, c);
            pDest += c;
            cRemain -= c;
            pSrc += c;
        }
        int cPPS = *pSrc++;
        while (cPPS--)
        {
            int c = ReadMSW(pSrc);
            pSrc += 2;
            pDest[0] = 0;
            pDest[1] = 0;
            pDest[2] = 0;
            pDest[3] = 1;
            pDest += 4;
            cRemain -= 4;

            CopyMemory(pDest, pSrc, c);
            pDest += c;
            cRemain -= c;
            pSrc += c;
        }
    }

    while (cBytes > m_cLength)
    {
        // read length field
        BYTE abLength[5];
        if (pMovie->ReadAbsolute(llPos, abLength, m_cLength) != S_OK)
        {
            return 0;
        }
        llPos += m_cLength;
        cBytes -= m_cLength;
        long cThis = 0;
        for (int i = 0; i < m_cLength; i++)
        {
            cThis <<= 8;
            cThis += abLength[i];
        }
        if ((cThis > cBytes) || ((cThis + 4) > cRemain))
        {
            return 0;
        }
        // output start code and then cThis bytes
        pDest[0] = 0;
        pDest[1] = 0;
        pDest[2] = 0;
        pDest[3] = 1;
        pDest += 4;
        cRemain -= 4;
        if (pMovie->ReadAbsolute(llPos, pDest, cThis) != S_OK)
        {
            return 0;
        }
        pDest += cThis;
        cRemain -= cThis;
        llPos += cThis;
        cBytes -= cThis;
    }
    BYTE* pStart;
    pSample->GetPointer(&pStart);
    return long(pDest - pStart);		// 32-bit consumption per packet is safe
}

