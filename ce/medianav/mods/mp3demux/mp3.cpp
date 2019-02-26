//
// mp3.cpp: implementation of MP3 parsing classes
//

#include "stdafx.h"
#include "mp3.h"
#include "ElemType.h"
#include "Index.h"
#include <sstream>

// byte re-ordering
static __forceinline DWORD SwapLong(const DWORD val)
{
    return _byteswap_ulong(val);
}

MP3Atom::MP3Atom(AtomReader* pReader, LONGLONG llOffset, LONGLONG llLength, DWORD type)
: Atom(pReader, llOffset, llLength, type, 0)
{
}

#pragma pack(push,1)
struct ID3v2Header
{
    BYTE tag[3]; // ID3
    BYTE version;
    BYTE versionSub;
    BYTE flags;
    BYTE size[4]; // 7bit encoded
};

static const int cMPAHeaderSize = 4; // MPEG-Audio Header Size 32bit

union FrameHeader
{
    DWORD value_;
    struct 
    {
        DWORD emphasis:2;
        DWORD original:1;
        DWORD copyright:1;
        DWORD modeExtension:2;
        DWORD channelMode:2;
        DWORD privateBit:1;
        DWORD padding:1;
        DWORD sampleRateIndex:2;
        DWORD bitrateIndex:4;
        DWORD protection:1;
        DWORD layer:2;
        DWORD version:2;
        DWORD sync:11;
    };
};

struct VBRIHeader
{
    BYTE  ID[4]; //VBRI
    WORD  version;
    WORD  delay;
    WORD  quality;
    DWORD size;
    DWORD frames;
    WORD  tocTableSize;
    WORD  tocTableScale;
    WORD  tocTableEntrySize;
    WORD  tocFramesPerTableEntry;
    // Dynamic data follows
};
static const int cMPAVBRIHeaderOffest = 32;
#pragma pack(pop)

enum MPAVersion
{
    MPEG25 = 0,
    MPEGReserved,
    MPEG2,
    MPEG1		
};

enum MPALayer
{
    Layer1 = 0,
    Layer2,
    Layer3,
    LayerReserved
};

enum Emphasis
{
    EmphNone = 0,
    Emph5015,
    EmphReserved,
    EmphCCITJ17
};

enum ChannelMode
{
    Stereo = 0,
    JointStereo,
    DualChannel,
    SingleChannel
};

// sampling rates in hertz: 1. index = MPEG Version ID, 2. index = sampling rate index
const DWORD cMPASampleRate[4][3] = 
{ 
    {11025, 12000, 8000,  },	// MPEG 2.5
    {0,     0,     0,     },	// reserved
    {22050, 24000, 16000, },	// MPEG 2
    {44100, 48000, 32000  }		// MPEG 1
};

// bitrates: 1. index = LSF, 2. index = Layer, 3. index = bitrate index
const DWORD cMPABitrates[2][3][15] =
{
    {	// MPEG 1
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},	// Layer1
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},	// Layer2
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}	// Layer3
    },
    {	// MPEG 2, 2.5		
        {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},		// Layer1
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},			// Layer2
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}			// Layer3
    }
};

const DWORD cMPASamplesPerFrames[2][3] =
{
    {	// MPEG 1
        384,	// Layer1
        1152,	// Layer2	
        1152	// Layer3
    },
    {	// MPEG 2, 2.5
        384,	// Layer1
        1152,	// Layer2
        576		// Layer3
    }	
};

const DWORD cMPASideInfoSizes[2][2] =
{
    // MPEG 1
    {32,17},
    // MPEG 2/2.5
    {17,9}
};

// Samples per Frame / 8
const DWORD cMPACoefficients[2][3] =
{
    {	// MPEG 1
        12,		// Layer1	(must be multiplied with 4, because of slot size)
        144,	// Layer2
        144		// Layer3
    },
    {	// MPEG 2, 2.5
        12,		// Layer1	(must be multiplied with 4, because of slot size)
        144,	// Layer2
        72		// Layer3
    }	
};

// slot size per layer
const DWORD cMPASlotSizes[3] =
{
    4,			// Layer1
    1,			// Layer2
    1			// Layer3
};

// for XING VBR Header flags
#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

void MP3Atom::ScanChildrenAt(LONGLONG llOffset)
{
    llOffset += m_cHeader;

    while (llOffset < m_llLength)
    {
        BYTE hdr[4];
        if(Read(llOffset, 4, hdr) != S_OK)
            break;
        LONGLONG llLength = 0;
        DWORD type;
        if(memcmp("TAG", hdr, 3) == 0)
        {
            llLength = 128;
            type = FOURCC("TAG ");
        }else if(memcmp("TAG+", hdr, 4) == 0)
        {
            llLength = 227;
            type = FOURCC("TAG+");
        }else if(memcmp("ID3", hdr, 3) == 0)
        {
            ID3v2Header id3Header;
            if(Read(llOffset, sizeof(ID3v2Header), &id3Header) != S_OK)
                break;
            DWORD size = id3Header.size[3];
            size += static_cast<WORD>(id3Header.size[2]) << 7;
            size += static_cast<DWORD>(id3Header.size[1]) << 14;
            size += static_cast<DWORD>(id3Header.size[0]) << 21;
            llLength = size;
            type = FOURCC("ID3 ");
        }else if(hdr[0] == 0xFF && (hdr[1] & 0xE0) == 0xE0)
        {
            llLength = m_llLength - llOffset;
            type = FOURCC("STRM");
        }else
        {
            if(hdr[0] != 0)
                break; // Most likely bad data. Stop further parsing.
            ++llOffset;
            continue;
        }

        AtomPtr pChild = AtomPtr(new MP3Atom(this, llOffset, llLength, type));
        m_Children.push_back(pChild);

        llOffset += llLength;
    }
}

MP3Movie::MP3Movie(const AtomReaderPtr& pRoot)
: Movie(pRoot)
{
    AtomPtr pMp3 = AtomPtr(new MP3Atom(pRoot.get(), 0, pRoot->Length(), 0));
    const AtomPtr& pStream = pMp3->FindChild(FOURCC("STRM"));
    if (pStream == NULL)
        return;
    MovieTrackPtr pTrack = MovieTrackPtr(new MP3MovieTrack(pStream, this, 0));
    if (pTrack->Valid())
    {
        m_Tracks.push_back(pTrack);
    }
    return;
}

static const int cMPAJoinFramesCount = 64; // Join this number of frames to a chunk to reduce media read operations overhead.

MP3MovieTrack::MP3MovieTrack(const AtomPtr& pAtom, MP3Movie* pMovie, DWORD idx)
: MovieTrack(AtomPtr(), pMovie, idx)
{
    FrameHeader frameHeader;
    DWORD hdrVal;
    if(pAtom->Read(0, sizeof(DWORD), &hdrVal) != S_OK)
    {
        debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Unable to read frame header\r\n");
        return;
    }
    frameHeader.value_ = SwapLong(hdrVal);
    if(frameHeader.sync != 0x7FF)
    {
        debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: frame header - bad sync %d\r\n", frameHeader.sync);
        return;
    }

    MPAVersion mpaVersion = static_cast<MPAVersion>(frameHeader.version);
    MPALayer mpaLayer = static_cast<MPALayer>(LayerReserved - frameHeader.layer);
    if(mpaVersion == MPEGReserved || mpaLayer == LayerReserved)
    {
        debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: frame header - bad version/layer = %d/%d\r\n", frameHeader.version, frameHeader.layer);
        return;
    }
    DWORD sampleRate = cMPASampleRate[mpaVersion][frameHeader.sampleRateIndex];
    DWORD bitrate = cMPABitrates[mpaVersion != MPEG1][mpaLayer][frameHeader.bitrateIndex] * 1000;
    DWORD samplesPerFrame = cMPASamplesPerFrames[mpaVersion != MPEG1][mpaLayer];
    ChannelMode channelMode = static_cast<ChannelMode>(frameHeader.channelMode);

    // Setup for default CBR case without headers
    DWORD cbrFrameSize = static_cast<DWORD>((cMPACoefficients[mpaVersion != MPEG1][mpaLayer] * bitrate / sampleRate) + ((cMPACoefficients[mpaVersion != MPEG1][mpaLayer] * bitrate % sampleRate) > 0 ? 1 : 0)) * cMPASlotSizes[mpaLayer];
    DWORD frames = static_cast<DWORD>(pAtom->Length() / cbrFrameSize);
    do
    {
        // Check for Xing header
        DWORD vbrOffset = cMPAHeaderSize + cMPASideInfoSizes[mpaVersion != MPEG1][channelMode == SingleChannel];
        {
        	/*  XING VBR-Header
	            size    description
	            4       'Xing' or 'Info'
	            4       flags (indicates which fields are used)
	            4       frames (optional)
	            4       bytes (optional)
	            100     toc (optional)
	            4       a VBR quality indicator: 0=best 100=worst (optional)
        	*/
            debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Trying Xing header at %d\r\n", vbrOffset);
            BYTE vbrHeader[8];
            if(pAtom->Read(vbrOffset, 8, &vbrHeader) != S_OK)
            {
                debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Unable to read Xing vbr header, offset = %d\r\n", vbrOffset);
                return;
            }

            if(memcmp(&vbrHeader, "Xing", 4) == 0 || memcmp(&vbrHeader, "Info", 4) == 0)
            {
                debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Found Xing header at %d\r\n", vbrOffset);
                DWORD flags = SwapLong(*reinterpret_cast<DWORD *>(&vbrHeader[4]));
                vbrOffset +=8; // Xing + flags
                DWORD sizeToRead = 0;
                if(flags & FRAMES_FLAG)
                    sizeToRead += sizeof(DWORD);
                if(flags & BYTES_FLAG)
                    sizeToRead += sizeof(DWORD);
                if(flags & TOC_FLAG)
                    sizeToRead += sizeof(BYTE) * 100;
                smart_array<BYTE> hdrBuf = new BYTE[sizeToRead];
                if(pAtom->Read(vbrOffset, sizeToRead, hdrBuf) != S_OK)
                {
                    debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Unable to read vbr Xing header data, size = %d\r\n", sizeToRead);
                    return;
                }
                size_t hdrBufOffset = 0;
                if(flags & FRAMES_FLAG)
                {
                    DWORD dTmp = SwapLong(*(reinterpret_cast<DWORD *>(&((*hdrBuf)[hdrBufOffset]))));
                    if(dTmp == 0)
                    {
                        debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Xing frames is 0. Broken header! Ignore it.\r\n");
                        break;
                    }
                    frames = dTmp;
                    hdrBufOffset += sizeof(DWORD);
                    cbrFrameSize = static_cast<DWORD>(pAtom->Length() / frames);
                }
                if(flags & BYTES_FLAG)
                {
                    DWORD totalSize = SwapLong(*(reinterpret_cast<DWORD *>(&((*hdrBuf)[hdrBufOffset]))));
                    hdrBufOffset += sizeof(DWORD);
                    cbrFrameSize = totalSize / frames;
                }
                break;
            }
        }

        // Check for VBRI header
        {
            /*  FhG VBRI Header
            	size	description
            	4       'VBRI' (ID)
            	2       version
            	2       delay
            	2       quality
            	4       # bytes
            	4       # frames
            	2       table size (for TOC)
            	2       table scale (for TOC)
            	2       size of table entry (max. size = 4 byte (must be stored in an integer))
            	2       frames per table entry

                ??      dynamic table consisting out of frames with size 1-4
            			whole length in table size! (for TOC)
            */
            debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Trying VBRI\r\n");
            vbrOffset = cMPAHeaderSize + cMPAVBRIHeaderOffest;
            VBRIHeader vrbiHeader;
            if(pAtom->Read(vbrOffset, sizeof(vrbiHeader), &vrbiHeader) != S_OK)
            {
                debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Unable to read VBRI vbr header, offset = %d\r\n", vbrOffset);
                return;
            }

            if(memcmp(&vrbiHeader.ID, "VBRI", 4) == 0)
            {
                debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Found VBRI header at %d\r\n", vbrOffset);
                DWORD dTmp = SwapLong(vrbiHeader.frames);
                if(dTmp == 0)
                {
                    debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: vrbiHeader.frames is 0. Broken header! Ignore it.\r\n");
                    break;
                }
                
                frames = dTmp;
                cbrFrameSize = SwapLong(vrbiHeader.size) / frames;
                break;
            }
        }
 
        // Fall back to CBR without info.
        debugPrintf(DBG, L"MP3MovieTrack::MP3MovieTrack: Fall back to CBR without header\r\n");
    }while(false);

    m_pIndex = new MP3TrackIndex(pAtom->Offset(), samplesPerFrame, sampleRate, cbrFrameSize, frames, cMPAJoinFramesCount);
    m_pType = new MP3ElementaryType(sampleRate, bitrate, channelMode != SingleChannel ? 2 : 1, mpaVersion != MPEG1);

    ostringstream strm;
    strm << m_pType->ShortName() << " ";
    strm << m_idx+1;
    m_strName = strm.str();

    // create dummy edit
    EditEntry e;
    e.offset = 0;
    e.sumDurations = 0;
    e.duration = Index()->TotalDuration();
    m_Edits.push_back(e);

    m_pRoot = pAtom;
}

REFERENCE_TIME MP3MovieTrack::Duration() const
{
    return Index()->TotalDuration();
}