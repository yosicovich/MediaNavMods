//
// ElemType.h: implementations of elementary stream type classes.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ElemType.h"
#include <dvdmedia.h>
#include <handlers.h>
#include <audshow.h>

// Debug stuff
#define AAC_DEBUG 0

inline WORD Swap2Bytes(USHORT x)
{
	return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
}
inline DWORD Swap4Bytes(DWORD x)
{
	return ((x & 0xff) << 24) |
		   ((x & 0xff00) << 8) |
		   ((x & 0xff0000) >> 8) |
		   ((x >> 24) & 0xff);
}

class BigEndianAudioHandler : public NoChangeHandler
{
public:
    long PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes);
};

long 
BigEndianAudioHandler::PrepareOutput(IMediaSample* pSample, Movie* pMovie, LONGLONG llPos, long cBytes)
{
    cBytes = __super::PrepareOutput(pSample, pMovie, llPos, cBytes);
    if (cBytes > 0)
    {
        BYTE* pBuffer;
        pSample->GetPointer(&pBuffer);
        if (cBytes%2) 
        {
            cBytes--;
        }
        BYTE* pEnd = pBuffer + cBytes;
        while (pBuffer < pEnd)
        {
            WORD w = *(WORD*)pBuffer;
            w = Swap2Bytes(w);
            *(WORD*)pBuffer = w;
            pBuffer += 2;
        }
    }
    return cBytes;
}

// -----------------------------------------------------

Mpeg4ElementaryType::Mpeg4ElementaryType()
: ElementaryType(),
  m_tFrame(0),
  m_cDecoderSpecific(0),
  m_depth(0)
{
}


bool 
Mpeg4ElementaryType::ParseDescriptor(const AtomPtr& patmESD)
{
    AtomCache pESD(patmESD);
    if (pESD[0] != 0)
    {
        return false;   // only version 0 is speced
    }

	long cPayload = long(patmESD->Length() - patmESD->HeaderSize());

    // parse the ES_Descriptor to get the decoder info
    Descriptor ESDescr;
    if (!ESDescr.Parse(pESD+4, cPayload-4) || (ESDescr.Type() != Descriptor::ES_DescrTag))
    {
        return false;
    }
	long cOffset = 3;
    BYTE flags = ESDescr.Start()[2];
    if (flags & 0x80)
    {
        cOffset += 2;   // depends-on stream
    }
    if (flags & 0x40)
    {
        // URL string -- count of chars precedes string
        cOffset += ESDescr.Start()[cOffset] + 1;
    }
    if (flags & 0x20)
    {
        cOffset += 2;   // OCR id
    }
    Descriptor dscDecoderConfig;
    if (!ESDescr.DescriptorAt(cOffset, dscDecoderConfig))
    {
        return false;
    }
    Descriptor dscSpecific;
    if (!dscDecoderConfig.DescriptorAt(13, dscSpecific))
    {
		// could be mpeg-1/2 audio
		if (*dscDecoderConfig.Start() == 0x6b)
		{
			m_type = Audio_Mpeg2;
			return true;
		}
        return false;
    }

    // store decoder-specific info
    m_cDecoderSpecific = dscSpecific.Length();
    m_pDecoderSpecific = new BYTE[dscSpecific.Length()];
    CopyMemory(m_pDecoderSpecific,  dscSpecific.Start(),  m_cDecoderSpecific);

    return true;
}

struct QTVideo 
{
	BYTE	reserved1[6];		// 0
	USHORT	dataref;
	
	USHORT	version;			// 0
	USHORT	revision;			// 0
	ULONG	vendor;

	ULONG	temporal_compression;
	ULONG	spatial_compression;

	USHORT	width;
	USHORT	height;
	
	ULONG	horz_resolution;	// 00 48 00 00
	ULONG	vert_resolution;	// 00 48 00 00
	ULONG	reserved2;			// 0
	USHORT	frames_per_sample;	// 1
	BYTE	codec_name[32];		// pascal string - ascii, first byte is char count
	USHORT	bit_depth;
	USHORT	colour_table_id;		// ff ff
};


bool 
Mpeg4ElementaryType::Parse(REFERENCE_TIME tFrame, const AtomPtr& patm)
{
	m_shortname = "Unknown";
    m_tFrame = tFrame;

    // the ESD atom is at the end of a type-specific
    // structure.
    AtomCache pSD(patm);
    long cOffset;
	bool bDescriptor = true;
    if (patm->Type() == FOURCC("mp4v"))
    {
        m_type = Video_Mpeg4;
		m_shortname = "MPEG4 Video";
        m_cx = (pSD[24] << 8) + pSD[25];
        m_cy = (pSD[26] << 8) + pSD[27];
        cOffset = 78;
    } else 
	if (patm->Type() == FOURCC("jvt1")) 
    {
		// this is an older format that I think is not in use
        m_type = Video_H264;
		m_shortname = "H264 Video";
        m_cx = (pSD[24] << 8) + pSD[25];
        m_cy = (pSD[26] << 8) + pSD[27];
        cOffset = 78;
	} else 
	if ((patm->Type() == FOURCC("s263")) || (patm->Type() == FOURCC("M263")))
	{
		m_type = Video_H263;
		m_shortname = "H263 Video";
        m_cx = (pSD[24] << 8) + pSD[25];
        m_cy = (pSD[26] << 8) + pSD[27];
        cOffset = 78;
    } else 
	if (patm->Type() == FOURCC("avc1"))
	{
		// this is 14496-15: there is no descriptor in this case
        m_type = Video_H264;
		m_shortname = "H264 Video";
        m_cx = (pSD[24] << 8) + pSD[25];
        m_cy = (pSD[26] << 8) + pSD[27];
		cOffset = 78;
		bDescriptor = false;
	} else 
	#pragma region 24/32-bit RGB
	if ((patm->Type() == BI_RGB) || (patm->Type() == Swap4Bytes(MEDIASUBTYPE_RGB24.Data1)) || (patm->Type() == Swap4Bytes(MEDIASUBTYPE_RGB32.Data1)))
	{
		m_type = Video_FOURCC;
		m_fourcc = BI_RGB;

		// casting this to the structure makes it clearer what
		// info is there, but remember all shorts and longs are byte swapped
		const QTVideo* pformat = (const QTVideo*)(const BYTE*)pSD;
		m_cx = Swap2Bytes(pformat->width);
		m_cy = Swap2Bytes(pformat->height);
		m_depth = Swap2Bytes(pformat->bit_depth);

		// no further info needed
		return true;
	} else 
	#pragma endregion
	if ((patm->Type() == FOURCC("rle ")) ||
		(patm->Type() == FOURCC("YUY2")) ||
		(patm->Type() == FOURCC("UYVY")) ||
		(patm->Type() == FOURCC("HDYC")) ||
		(patm->Type() == FOURCC("YV12")) ||
		(patm->Type() == FOURCC("NV12")) ||
		(patm->Type() == FOURCC("I420")) ||
		(patm->Type() == FOURCC("dvc ")) ||
		(patm->Type() == FOURCC("dvcp")) ||
		(patm->Type() == FOURCC("dvsd")) ||
		(patm->Type() == FOURCC("dvh5")) ||
		(patm->Type() == FOURCC("dvhq")) ||
		(patm->Type() == FOURCC("dvhd")) ||
		(patm->Type() == FOURCC("jpeg")) ||
		(patm->Type() == FOURCC("mjpg")) ||
		(patm->Type() == FOURCC("MJPG")) ||
		(patm->Type() == FOURCC("mjpa")) ||
		(patm->Type() == FOURCC("apcn")) ||
        (patm->Type() == FOURCC("hap1")) ||
        (patm->Type() == FOURCC("HAP1")) ||
        (patm->Type() == FOURCC("Hap1")) ||
        (patm->Type() == FOURCC("hap5")) ||
        (patm->Type() == FOURCC("HAP5")) ||
        (patm->Type() == FOURCC("Hap5")) ||
        (patm->Type() == FOURCC("hapy")) ||
        (patm->Type() == FOURCC("HAPY")) ||
        (patm->Type() == FOURCC("HapY"))
				)
	{
		m_type = Video_FOURCC;
		m_shortname = "Video";

		// some decoders (eg mainconcept) only accept basic dv types, so map to these types:
		DWORD fcc = patm->Type();
		if((fcc == FOURCC("dvc ")) || (fcc == FOURCC("dvcp")))
		{
			fcc = FOURCC("dvsd");
			m_shortname = "DV Video";
		} else 
		if((fcc == FOURCC("dvhq")) || (fcc == FOURCC("dvh5")))
		{
			fcc = FOURCC("dvhd");
			m_shortname = "DV Video";
		} else 
		if((fcc == FOURCC("jpeg")) || (fcc == FOURCC("mjpa")) || (fcc == FOURCC("mjpg")))
		{
			fcc = FOURCC("MJPG");
			m_shortname = "Motion JPEG Video";
		} else 
		if((fcc == FOURCC("hap1")) || (fcc == FOURCC("HAP1")) || (fcc == FOURCC("Hap1")))
		{
			fcc = FOURCC("hap1");
			m_shortname = "Hap";
		} else 
		if((fcc == FOURCC("hap5")) || (fcc == FOURCC("HAP5")) || (fcc == FOURCC("Hap5")))
		{
			fcc = FOURCC("hap5");
			m_shortname = "Hap Alpha";
		} else 
		if((fcc == FOURCC("hapy")) || (fcc == FOURCC("HAPY")) || (fcc == FOURCC("HapY")))
		{
			fcc = FOURCC("hapy");
			m_shortname = "Hap Alpha";
		}

		m_fourcc = Swap4Bytes(fcc);

		// casting this to the structure makes it clearer what
		// info is there, but remember all shorts and longs are byte swapped
		const QTVideo* pformat = (const QTVideo*)(const BYTE*)pSD;
		m_cx = Swap2Bytes(pformat->width);
		m_cy = Swap2Bytes(pformat->height);
		m_depth = Swap2Bytes(pformat->bit_depth);

		// no further info needed
		return true;
	} else if ((patm->Type() == FOURCC("mpg2")) ||
			   (patm->Type() == FOURCC("xdvc")) ||
			   (patm->Type() == FOURCC("xdv3"))
			   )
	{
        m_type = Video_Mpeg2;
		m_shortname = "MPEG-2 Video";
        m_cx = (pSD[24] << 8) + pSD[25];
        m_cy = (pSD[26] << 8) + pSD[27];
        cOffset = 78;
		return true;
	}else if (patm->Type() == FOURCC("mp4a"))
    {
        m_type = Audio_AAC;
		m_shortname = "AAC Audio";
        cOffset = 28;

		// parse audio sample entry and store in case we don't find a descriptor
		m_cDecoderSpecific = sizeof(WAVEFORMATEX);
		m_pDecoderSpecific = new BYTE[m_cDecoderSpecific];
		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)(BYTE*)m_pDecoderSpecific;
		pwfx->cbSize = 0;
		WORD w = *(USHORT*)(pSD + 16);
		pwfx->nChannels = Swap2Bytes(w);
		w = *(USHORT*)(pSD + 18);
		pwfx->wBitsPerSample = Swap2Bytes(w);

		// rate is fixed point 16.16
		pwfx->nSamplesPerSec = (SwapLong(pSD + 24) >> 16) & 0xffff;
		pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
		pwfx->wFormatTag = WAVE_FORMAT_PCM;


		// add support for some non-ISO files
		int version = (pSD[8] << 8) + pSD[9];
		if (version == 1)
		{
			cOffset += 16;
		}
		else if (version == 2)
		{
			cOffset += 16 + 20;
		}
		if ((DWORD)SwapLong(pSD+cOffset + 4) != FOURCC("wave"))
		{
			if (version > 0)
			{
				long cSearch = 28;
				while (cSearch < (patm->Length()-8))
				{
					if ((DWORD)SwapLong(pSD + cSearch + 4) == FOURCC("wave"))
					{
						cOffset = cSearch;
						break;
					}
					cSearch++;
				}
			}
		}
    } else if ((patm->Type() == FOURCC("lpcm")) ||
               (patm->Type() == FOURCC("alaw")) ||
               (patm->Type() == FOURCC("ulaw")) ||
			   (patm->Type() == FOURCC("samr")) ||
			   (patm->Type() == FOURCC("samw")))
    {
        m_type = Audio_WAVEFORMATEX;
		m_shortname = "Audio";
        cOffset = 28;
	}
	else if ((patm->Type() == FOURCC("sowt")) ||
			(patm->Type() == FOURCC("twos")) ||
			(patm->Type() == FOURCC("in24")) ||
			(patm->Type() == FOURCC("in32")) ||
			(patm->Type() == FOURCC("raw "))
			)
	{
		m_type = Audio_WAVEFORMATEX;
		m_shortname = "Audio";

		m_fourcc = patm->Type();

		// basic pcm audio type - parse this audio atom as a QT sample description
		m_cDecoderSpecific = sizeof(WAVEFORMATEX);
		m_pDecoderSpecific = new BYTE[m_cDecoderSpecific];
		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)(BYTE*)m_pDecoderSpecific;

		pwfx->cbSize = 0;
		WORD w = *(USHORT*)(pSD + 16);
		pwfx->nChannels = Swap2Bytes(w);
		if (patm->Type() == FOURCC("in24"))
		{
			pwfx->wBitsPerSample = 24;
		}
		else if (patm->Type() == FOURCC("in32"))
		{
			pwfx->wBitsPerSample = 32;
		}
		else
		{
			w = *(USHORT*)(pSD + 18);
			pwfx->wBitsPerSample = Swap2Bytes(w);
		}

		// rate is fixed point 16.16
		pwfx->nSamplesPerSec = (SwapLong(pSD + 24) >> 16) & 0xffff;
		pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
		pwfx->wFormatTag = WAVE_FORMAT_PCM;
		
		return true;
	}
	else if (patm->Type() == FOURCC("c608"))
	{
		m_type = Text_CC608;
		m_shortname = "CC 608 Data";
		return true;
    } else {
        return false;
    }

    patm->ScanChildrenAt(cOffset);
	for (int i = 0; i < patm->ChildCount(); i++)
	{
	    AtomPtr& patmESD = patm->Child(i);
		if (!patmESD)
		{
			return false;
		}
    
		AtomCache pESD(patmESD);
		long cPayload = long(patmESD->Length() - patmESD->HeaderSize());
		if ((m_type == Video_H264) && !bDescriptor)
		{
			// iso 14496-15
			if (patmESD->Type() == FOURCC("avcC"))
			{
				// store the whole payload as decoder specific
				m_cDecoderSpecific = cPayload;
				m_pDecoderSpecific = new BYTE[cPayload];
				CopyMemory(m_pDecoderSpecific,  pESD,  m_cDecoderSpecific);
				break;
			}

		} else if (patmESD->Type() == FOURCC("d263"))
		{
			// 4 byte vendor code
			// 1 byte version
			// 1 byte level
			// 1 byte profile
			break;
		}
		else if (patmESD->Type() == FOURCC("damr"))
		{
			// 4 byte vendor code
			// 1 byte version
			// 2-byte mode index
			// 1-byte number of packet mode changes
			// 1-byte number of samples per packet
			break;
		}
		else if (patmESD->Type() == FOURCC("esds"))
		{
			if (ParseDescriptor(patmESD))
			{
				break;
			}
			else
			{
				return false;
			}
		}
		else if (patmESD->Type() == FOURCC("wave"))
		{
			// this appears to be a non-ISO file (prob quicktime)
			// search for esds in children of this atom
			for (int j = 0; j < patmESD->ChildCount(); j++)
			{
				AtomPtr& pwav = patmESD->Child(j);
				if (pwav->Type() == FOURCC("esds"))
				{
					if (ParseDescriptor(pwav))
					{
						break;
					}
				}
			}
			if (m_cDecoderSpecific > 0)
			{
				break;
			}
		}
	}
    // check that we have picked up the seq header or other data
    // except for the few formats where it is not needed.
    if ((m_type != Audio_AAC) && (m_type != Audio_WAVEFORMATEX))
    {
        if (m_cDecoderSpecific <= 0)
        {
            return false;
        }
    }
    return true;
}

bool 
Mpeg4ElementaryType::IsVideo() const
{
    return (m_type > First_Video) && (m_type < First_Audio);
}
    
void 
Mpeg4ElementaryType::setHandler(const CMediaType* pmt, int idx)
{
    // handler based on m_type and idx
    if (m_type == Audio_AAC)
    {
        debugPrintf(AAC_DEBUG, L"Mpeg4ElementaryType::setHandler() m_type == Audio_AAC\r\n");
        m_pHandler = new CoreAACHandler();
    } else 
        // bugfix pointed out by David Hunter --
        // Use the divxhandler to prepend VOL header for divx and xvid types.
        // index must match index values in GetType_Mpeg4V
        // (should really compare subtypes here I think)
        if ((m_type == Video_Mpeg4) && (idx > 0))
        {
            m_pHandler = new DivxHandler(m_pDecoderSpecific, m_cDecoderSpecific);
        } 
        else if ((m_type == Video_H264) && (idx > 0) && (*pmt->FormatType() != FORMAT_MPEG2Video))
        {
            debugPrintf(DBG, L"Mpeg4ElementaryType::setHandler() choose H264ByteStreamHandler\r\n");
            m_pHandler = new H264ByteStreamHandler(m_pDecoderSpecific, m_cDecoderSpecific);
        }
        else if ((m_type == Audio_WAVEFORMATEX) &&
            (m_fourcc == FOURCC("twos"))
            )
        {
            m_pHandler = new BigEndianAudioHandler();
        } else {
            debugPrintf(DBG, L"Mpeg4ElementaryType::setHandler() choose NoChangeHandler\r\n");
            m_pHandler = new NoChangeHandler();
        }
}

bool 
Mpeg4ElementaryType::GetType(CMediaType* pmt, int nType) const
{
    // !! enable more type choices (eg dicas instead of divx for mpeg4)

    switch (m_type)
    {
    case Video_H264:
        if(nType == 0)
        {
            return GetType_AVCC(pmt);
        }else if (nType == 1)
        {
            return GetType_H264(pmt);
        }else if (nType == 2)
		{
            return GetType_H264ByteStream(pmt);
		}
        break;

    case Video_Mpeg4:
	case Video_H263:
        return GetType_Mpeg4V(pmt, nType);
        // !! dicas 
        break;

	case Video_FOURCC:
		if (nType == 0)
		{
			return GetType_FOURCC(pmt);
		}
		break;

	case Video_Mpeg2:
		if (nType == 0)
		{
			// for standard mpeg-4 video, we set the media type
			// up for the DivX decoder
			pmt->InitMediaType();
			pmt->SetType(&MEDIATYPE_Video);
			pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
			pmt->SetFormatType(&FORMAT_MPEG2_VIDEO);
			MPEG2VIDEOINFO* pVI = (MPEG2VIDEOINFO*)pmt->AllocFormatBuffer(sizeof(MPEG2VIDEOINFO) + m_cDecoderSpecific);
			ZeroMemory(pVI, sizeof(MPEG2VIDEOINFO));
			pVI->hdr.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pVI->hdr.bmiHeader.biBitCount = 24;
			pVI->hdr.bmiHeader.biWidth = m_cx;
			pVI->hdr.bmiHeader.biHeight = m_cy;
			pVI->hdr.bmiHeader.biSizeImage = DIBSIZE(pVI->hdr.bmiHeader);
			pVI->hdr.bmiHeader.biCompression = Swap4Bytes(FOURCC("mpg2"));
			pVI->hdr.AvgTimePerFrame = m_tFrame;

			if (m_cDecoderSpecific)
			{
				BYTE* pDecSpecific = (BYTE*)(pVI+1);
				CopyMemory(pDecSpecific, m_pDecoderSpecific,  m_cDecoderSpecific);
			}
			return true;
		}
		break;

    case Audio_AAC:
        return GetType_AAC(pmt, nType);
    case Audio_WAVEFORMATEX:
 	case Audio_Mpeg2:
       if (nType == 0)
        {
            return GetType_WAVEFORMATEX(pmt);
        }
        break;
	case Text_CC608:
		if (nType == 0)
		{
			pmt->InitMediaType();
			pmt->SetType(&MEDIATYPE_AUXLine21Data);
			pmt->SetSubtype(&MEDIASUBTYPE_Line21_BytePair);
			return true;
		}
    }

    return false;
}

FOURCCMap AVC1(FOURCC("1CVA"));
bool
Mpeg4ElementaryType::GetType_H264(CMediaType* pmt) const
{
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&AVC1);
    pmt->SetFormatType(&FORMAT_MPEG2Video);

	// following the Nero/Elecard standard, we use an mpeg2videoinfo block
	// with dwFlags for nr of bytes in length field, and param sets in 
	// sequence header (allows 1 DWORD already -- extend this).

	const BYTE* pconfig = m_pDecoderSpecific;
	// count param set bytes (including 2-byte length)
	int cParams = 0;
	int cSeq = pconfig[5] & 0x1f;
	const BYTE* p = &pconfig[6];
	for (int i = 0; i < cSeq; i++)
	{
		int c = 2 + (p[0] << 8) + p[1];
		cParams += c;
		p += c;
	}
	int cPPS = *p++;
	for (int i = 0; i < cPPS; i++)
	{
		int c = 2 + (p[0] << 8) + p[1];
		cParams += c;
		p += c;
	}


	int cFormat = sizeof(MPEG2VIDEOINFO) + cParams - sizeof(DWORD);
    MPEG2VIDEOINFO* pVI = (MPEG2VIDEOINFO*)pmt->AllocFormatBuffer(cFormat);
    ZeroMemory(pVI, cFormat);
	pVI->hdr.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pVI->hdr.bmiHeader.biWidth = m_cx;
    pVI->hdr.bmiHeader.biHeight = m_cy;
    pVI->hdr.AvgTimePerFrame = m_tFrame;
	pVI->hdr.bmiHeader.biCompression = FOURCC("1cva");

	pVI->dwProfile = pconfig[1];
	pVI->dwLevel = pconfig[3];

	// nr of bytes used in length field, for length-preceded NALU format.
	pVI->dwFlags = (pconfig[4] & 3) + 1;

	pVI->cbSequenceHeader = cParams;
	// copy all param sets
	cSeq = pconfig[5] & 0x1f;
	p = &pconfig[6];
	BYTE* pDest = (BYTE*) &pVI->dwSequenceHeader;
	for (int i = 0; i < cSeq; i++)
	{
		int c = 2 + (p[0] << 8) + p[1];
		CopyMemory(pDest, p, c);
		pDest += c;
		p += c;
	}
	cPPS = *p++;
	for (int i = 0; i < cPPS; i++)
	{
		int c = 2 + (p[0] << 8) + p[1];
		CopyMemory(pDest, p, c);
		pDest += c;
		p += c;
	}

    return true;
}
	
FOURCCMap H264(FOURCC("462H"));
bool 
Mpeg4ElementaryType::GetType_H264ByteStream(CMediaType* pmt) const
{
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&H264);

	pmt->SetFormatType(&FORMAT_VideoInfo2);
	VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2) + m_cDecoderSpecific);
	ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
	pvi2->bmiHeader.biWidth = m_cx;
	pvi2->bmiHeader.biHeight = m_cy;
    pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
    pvi2->bmiHeader.biCompression = FOURCC("1cva");
	pvi2->AvgTimePerFrame = m_tFrame;
	SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
	pvi2->rcTarget = pvi2->rcSource;

	pvi2->dwPictAspectRatioX = m_cx;
	pvi2->dwPictAspectRatioY = m_cy;
		
	if(m_cDecoderSpecific)
    {
        CopyMemory(reinterpret_cast<void*>(reinterpret_cast<DWORD>(pvi2) + sizeof(VIDEOINFOHEADER2)), m_pDecoderSpecific, m_cDecoderSpecific);
    }
	return true;
}

bool 
Mpeg4ElementaryType::GetType_AVCC(CMediaType* pmt) const
{
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&AVC1);

    pmt->SetFormatType(&FORMAT_VideoInfo2);
    VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2) + m_cDecoderSpecific);
    ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
    pvi2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER) + m_cDecoderSpecific;
    pvi2->bmiHeader.biWidth = m_cx;
    pvi2->bmiHeader.biHeight = m_cy;
    pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
    pvi2->bmiHeader.biCompression = FOURCC("1cva");
    pvi2->AvgTimePerFrame = m_tFrame;
    SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
    pvi2->rcTarget = pvi2->rcSource;

    pvi2->dwPictAspectRatioX = m_cx;
    pvi2->dwPictAspectRatioY = m_cy;

    if(m_cDecoderSpecific)
    {
        CopyMemory(reinterpret_cast<void*>(reinterpret_cast<DWORD>(pvi2) + sizeof(VIDEOINFOHEADER2)), m_pDecoderSpecific, m_cDecoderSpecific);
    }
    return true;
}

bool
Mpeg4ElementaryType::GetType_Mpeg4V(CMediaType* pmt, int n) const
{
	DWORD fourcc;
	if (n == 0)
	{
		fourcc = FOURCC("V4PM");
	}
	else if (n == 1)
	{
		fourcc = FOURCC("XVID");
	}
	else if (n == 2)
	{
		fourcc = FOURCC("DIVX");
	}
	else
	{
		return false;
	}


    // for standard mpeg-4 video, we set the media type
    // up for the DivX decoder
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    FOURCCMap divx(fourcc);
    pmt->SetSubtype(&divx);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    VIDEOINFOHEADER* pVI = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + m_cDecoderSpecific);
    ZeroMemory(pVI, sizeof(VIDEOINFOHEADER));
    pVI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pVI->bmiHeader.biPlanes = 1;
    pVI->bmiHeader.biBitCount = 24;
    pVI->bmiHeader.biWidth = m_cx;
    pVI->bmiHeader.biHeight = m_cy;
    pVI->bmiHeader.biSizeImage = DIBSIZE(pVI->bmiHeader);
    pVI->bmiHeader.biCompression = fourcc;
    pVI->AvgTimePerFrame = m_tFrame;

	BYTE* pDecSpecific = (BYTE*)(pVI+1);
	CopyMemory(pDecSpecific, m_pDecoderSpecific,  m_cDecoderSpecific);

    return true;
}

bool 
Mpeg4ElementaryType::GetType_FOURCC(CMediaType* pmt) const
{
	pmt->InitMediaType();
	pmt->SetType(&MEDIATYPE_Video);
	pmt->SetFormatType(&FORMAT_VideoInfo);

	VIDEOINFOHEADER* pVI = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
    ZeroMemory(pVI, sizeof(VIDEOINFOHEADER));
    pVI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pVI->bmiHeader.biPlanes = 1;
    pVI->bmiHeader.biBitCount = (WORD)m_depth;
    pVI->bmiHeader.biWidth = m_cx;
    pVI->bmiHeader.biHeight = m_cy;
    pVI->bmiHeader.biSizeImage = DIBSIZE(pVI->bmiHeader);
    pVI->bmiHeader.biCompression = BI_RGB;
    pVI->AvgTimePerFrame = m_tFrame;

	pmt->SetSampleSize(pVI->bmiHeader.biSizeImage);

	if (m_fourcc == FOURCC("rle "))
	{
		pmt->SetSubtype(&MEDIASUBTYPE_QTRle);
	}
	else 
	#pragma region 24/32-bit RGB
	if (m_fourcc == BI_RGB)
	{
		switch(m_depth)
		{
		case 24:
			pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
			break;
		case 32:
			pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
			break;
		default:
			ASSERT(m_depth == 24 || m_depth == 32);
			pmt->SetSubtype(&MEDIASUBTYPE_NULL);
		}
	}
	else
	#pragma endregion
	{
		FOURCCMap fcc(m_fourcc);
		pmt->SetSubtype(&fcc);
		pVI->bmiHeader.biCompression = m_fourcc;
	}

	return true;
}

// static
const int Mpeg4ElementaryType::SamplingFrequencies[] = 
{
    96000,
    88200,
    64000,
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
    8000,
    7350,
    0,
    0,
    0,
};


const GUID* Mpeg4ElementaryType::AAC_GUIDS[] = 
{
    &MEDIASUBTYPE_AAC_AUDIO,
    &__uuidof(MEDIASUBTYPE_AAC),
    &__uuidof(MEDIASUBTYPE_MP4A),
    &__uuidof(MEDIASUBTYPE_MPEG_ADTS_AAC)
};

bool
Mpeg4ElementaryType::GetType_AAC(CMediaType* pmt, int n) const
{
    debugPrintf(AAC_DEBUG, L"ElementaryType::GetType_AAC(..., %d)\r\n", n);
    if(n >= sizeof(AAC_GUIDS)/sizeof(void*))
        return false;
    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Audio);
    pmt->SetSubtype(AAC_GUIDS[n]);
    pmt->SetFormatType(&FORMAT_WaveFormatEx);
    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX) + m_cDecoderSpecific);
    ZeroMemory(pwfx,  sizeof(WAVEFORMATEX));
    pwfx->cbSize = WORD(m_cDecoderSpecific);
    CopyMemory((pwfx+1),  m_pDecoderSpecific,  m_cDecoderSpecific);

    // parse decoder-specific info to get rate/channels
    long samplerate = ((m_pDecoderSpecific[0] & 0x7) << 1) + ((m_pDecoderSpecific[1] & 0x80) >> 7);
    pwfx->nSamplesPerSec = SamplingFrequencies[samplerate];
    pwfx->nBlockAlign = 1;
    pwfx->wBitsPerSample = 16;
    if(n == 3)
        pwfx->wFormatTag = 0x1600;//WAVE_FORMAT_ADTS_AAC;
    else
        pwfx->wFormatTag = 0x00FF;//WAVE_FORMAT_AAC;
    pwfx->nChannels = (m_pDecoderSpecific[1] & 0x78) >> 3;
    
    debugPrintf(AAC_DEBUG, L"m_pDecoderSpecific:\r\n");
    debugDump(AAC_DEBUG, &(*m_pDecoderSpecific), m_cDecoderSpecific);
    return true;
}

bool
Mpeg4ElementaryType::GetType_WAVEFORMATEX(CMediaType* pmt) const
{
    // common to standard audio types that have known atoms
    // in the mpeg-4 file format

    // the dec-specific info is a waveformatex
    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)(BYTE*)m_pDecoderSpecific;
    if ((m_cDecoderSpecific < sizeof(WAVEFORMATEX)) ||
        (m_cDecoderSpecific < int(sizeof(WAVEFORMATEX) + pwfx->cbSize)))
    {
        return false;
    }

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Audio);
	if (m_type == Audio_Mpeg2)
	{
		pwfx->wFormatTag = 0x55;// WAVE_FORMAT_MPEG;
		pmt->SetSubtype(&MEDIASUBTYPE_MPEG2_AUDIO);
	}
	else
	{
		FOURCCMap subtype(pwfx->wFormatTag);
		pmt->SetSubtype(&subtype);
	}
	pmt->SetFormatType(&FORMAT_WaveFormatEx);

    int cLen = pwfx->cbSize + sizeof(WAVEFORMATEX);
    WAVEFORMATEX* pwfxMT = (WAVEFORMATEX*)pmt->AllocFormatBuffer(cLen);
    CopyMemory(pwfxMT, pwfx, cLen);

    return true;
}

// --- descriptor parsing ----------------------
bool 
Descriptor::Parse(const BYTE* pBuffer, long cBytes)
{
    m_pBuffer = pBuffer;
    m_type = (eType)pBuffer[0];
    long idx = 1;
    m_cBytes = 0;
    do {
        m_cBytes = (m_cBytes << 7) + (pBuffer[idx] & 0x7f);
    } while ((idx < cBytes) && (pBuffer[idx++] & 0x80));

    m_cHdr = idx;
    if ((m_cHdr + m_cBytes) > cBytes)
    {
        m_cHdr = m_cBytes = 0;
        return false;
    }
    return true;
}

    
bool 
Descriptor::DescriptorAt(long cOffset, Descriptor& desc)
{
    return desc.Parse(Start() + cOffset, Length() - cOffset); 
}



