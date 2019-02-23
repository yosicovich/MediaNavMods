//
// MP3DecoderFilter.h

#pragma once

#include "StdAfx.h"
#include <CAudioDecodeFilter.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <msvc++/config.h>
#include <stream.h>
#include <frame.h>
#include <synth.h>

#ifdef __cplusplus
}
#endif

class LinearDither
{
public:
    LinearDither(BYTE bits, BYTE fractBits, int limit)
        :m_bits(bits)
        ,m_fractBits(fractBits)
        ,m_limit(limit)
    {

    }
    
    inline int ditherSample(int sample)
    {
        unsigned int scalebits;
        int output, mask, random;

        /* noise shape */
        sample += m_dither.error[0] - m_dither.error[1] + m_dither.error[2];

        m_dither.error[2] = m_dither.error[1];
        m_dither.error[1] = m_dither.error[0] / 2;

        /* bias */
        output = sample + (1L << (m_fractBits + 1 - m_bits - 1));

        scalebits = m_fractBits + 1 - m_bits;
        mask = (1L << scalebits) - 1;

        /* dither */
        random  = prng(m_dither.random);
        output += (random & mask) - (m_dither.random & mask);

        m_dither.random = random;

        /* clip */
        if (output > m_limit) 
        {
            output = m_limit;
            if (sample > m_limit)
                sample = m_limit;
        }

        if (output < -m_limit) 
        {
            output = -m_limit;
            if (sample < -m_limit)
                sample = -m_limit;
        }

        /* quantize */
        output &= ~mask;

        /* error feedback */
        m_dither.error[0] = sample - output;

        /* scale */
        return output >> scalebits;
    }

private:
    struct audio_dither 
    {
        int error[3];
        int random;
    };

    BYTE m_bits;
    BYTE m_fractBits;
    int m_limit;
    // Leave it un-initialized to do more randomness.
    audio_dither m_dither;
    static inline unsigned long prng(unsigned long state)
    {
        return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
    }
};


class DECLSPEC_UUID("C8F59247-8FAA-42d9-93A6-EF32961644B2") // mp3decfilter
MP3DecoderFilter: public CAudioDecodeFilter
{
public:
    DECLARE_IUNKNOWN;
    virtual ~MP3DecoderFilter();
    // constructor method used by class factory
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

protected:
    HRESULT SetOutputMediaType(const CMediaType *pmt);
    HRESULT decodeOneFrame(TransformBuffersState& buffersState, bool isDiscontinuity);
    DWORD getFrameBufferSize();
    OutBufferDesc getOutBufferDesc();

private:
    // construct only via class factory
    MP3DecoderFilter(LPUNKNOWN pUnk, HRESULT* phr);

    bool storeSamples(TransformBuffersState& buffersState, const mad_pcm& pcm);
    static __forceinline void updateInBuffer(TransformBuffersState& buffersState, const mad_stream& stream)
    {
        const BYTE* framePtr;
        if (stream.next_frame)
            framePtr = stream.next_frame;
        else
            framePtr = stream.this_frame;
        buffersState.inDataSize -= framePtr - buffersState.inData;
        buffersState.inData = framePtr;
    }

private:
    // Codec specific
    mad_stream m_madStream;
    mad_frame m_madFrame;
    mad_synth m_madSynth;

    OutBufferDesc m_outBufferDesc;
    DWORD m_oneInFrameSize;
    DWORD m_maxFramesPerChunk;

    LinearDither m_leftDither;
    LinearDither m_rightDither;
};
