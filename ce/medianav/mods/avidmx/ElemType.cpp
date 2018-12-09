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
#include <cassert>
#include <audshow.h>

// -----------------------------------------------------

AviElementaryType::AviElementaryType()
: ElementaryType()
{

}

bool 
AviElementaryType::Parse(const AVISTREAMHEADER& streamHeader, Atom* pFormat)
{
	if(streamHeader.fccType != streamtypeVIDEO && streamHeader.fccType != streamtypeAUDIO)
        return false;
    
    // keep Atom for our usage.
    m_pFormatAtom = pFormat;
    m_format = pFormat;
    m_streamHeader = streamHeader;
    if(m_streamHeader.fccType == streamtypeVIDEO)
    {
        debugPrintf(DBG, L"AviElementaryType::Parse: vids\r\n");

        GUID subType;
        bool extendedFormat = false;
        if(!FindFourCC(m_streamHeader.fccHandler, subType, extendedFormat))
        {
            debugPrintf(DBG, L"AviElementaryType::Parse: failed to find matching FourCC! handlerFourCC = %08X\r\n", m_streamHeader.fccHandler);
            return false;
        }

        m_shortname = "vids";
        m_mediaType.InitMediaType();
        m_mediaType.SetType(&MEDIATYPE_Video);
        m_mediaType.SetSubtype(&subType);
        assert(m_format.getDataSize() >= sizeof(BITMAPINFOHEADER));
        ULONG additionalSize = static_cast<ULONG>(m_format.getDataSize()) - sizeof(BITMAPINFOHEADER);
        if(extendedFormat)
        {
            m_mediaType.SetFormatType(&FORMAT_VideoInfoEx);
            debugPrintf(DBG, L"AviElementaryType::Parse: VIDEOINFOHEADEREX AllocFormatBuffer(%d), m_format.getDataSize() = %I64d\r\n", sizeof(VIDEOINFOHEADEREX) - sizeof(BITMAPINFOHEADER) + static_cast<ULONG>(m_format.getDataSize()), m_format.getDataSize());
            VIDEOINFOHEADEREX* pVI = reinterpret_cast<VIDEOINFOHEADEREX*>(m_mediaType.AllocFormatBuffer(sizeof(VIDEOINFOHEADEREX) + additionalSize));
            ZeroMemory(pVI, sizeof(VIDEOINFOHEADEREX));
            CopyMemory(&(pVI->bmiHeader), *m_format,  sizeof(BITMAPINFOHEADER));
            pVI->vihSize = sizeof(VIDEOINFOHEADEREX) + additionalSize;
            CopyMemory(reinterpret_cast<void*>(reinterpret_cast<DWORD>(pVI) + sizeof(VIDEOINFOHEADEREX)), reinterpret_cast<void*>(reinterpret_cast<DWORD>(*m_format) + sizeof(BITMAPINFOHEADER)), additionalSize);
            pVI->AvgTimePerFrame = (REFERENCE_TIME(m_streamHeader.dwScale) * UNITS) / m_streamHeader.dwRate;
        }else
        {
            m_mediaType.SetFormatType(&FORMAT_VideoInfo);
            debugPrintf(DBG, L"AviElementaryType::Parse: VIDEOINFOHEADER AllocFormatBuffer(%d), m_format.getDataSize() = %I64d\r\n", sizeof(VIDEOINFOHEADER) - sizeof(BITMAPINFOHEADER) + static_cast<ULONG>(m_format.getDataSize()), m_format.getDataSize());
            VIDEOINFOHEADER* pVI = reinterpret_cast<VIDEOINFOHEADER*>(m_mediaType.AllocFormatBuffer(sizeof(VIDEOINFOHEADER)  + additionalSize));
            ZeroMemory(pVI, sizeof(VIDEOINFOHEADER));
            CopyMemory(&(pVI->bmiHeader), *m_format,  static_cast<size_t>(m_format.getDataSize()));
            pVI->AvgTimePerFrame = (REFERENCE_TIME(m_streamHeader.dwScale) * UNITS) / m_streamHeader.dwRate;
        }
        std::string handlerFourCCStr(reinterpret_cast<const char*>(&m_streamHeader.fccHandler), sizeof(m_streamHeader.fccHandler));
        if(    _stricmp(handlerFourCCStr.c_str(), "xvid") == 0
            || _stricmp(handlerFourCCStr.c_str(), "divx") == 0
            || _stricmp(handlerFourCCStr.c_str(), "div3") == 0
            || _stricmp(handlerFourCCStr.c_str(), "div4") == 0
            || _stricmp(handlerFourCCStr.c_str(), "DX30") == 0
            || _stricmp(handlerFourCCStr.c_str(), "DX40") == 0
            || _stricmp(handlerFourCCStr.c_str(), "DX50") == 0
            )
        {
            m_pHandler.reset(new DivxHandler(reinterpret_cast<BYTE*>(reinterpret_cast<DWORD>(*m_format) + sizeof(BITMAPINFOHEADER)),additionalSize));
        }
    }else if(m_streamHeader.fccType == streamtypeAUDIO)
    {
        debugPrintf(DBG, L"AviElementaryType::Parse: auds\r\n");
        m_shortname = "auds";
        m_mediaType.InitMediaType();
        m_mediaType.SetType(&MEDIATYPE_Audio);
        m_mediaType.SetFormatType(&FORMAT_WaveFormatEx);
        assert(m_format.getDataSize() >= sizeof(WAVEFORMATEX));
        if(!m_mediaType.SetFormat((BYTE *)*m_format, static_cast<ULONG>(m_format.getDataSize())))
            return false;
        WORD formatTag = reinterpret_cast<const WAVEFORMATEX*>(*m_format)->wFormatTag;
        switch(formatTag)
        {
        case 0x674f:
        case 0x6750:
        case 0x6751:
        case 0x6771:
            m_mediaType.SetSubtype(&MEDIASUBTYPE_Vorbis);
            break;
        case 0x00FF:
            m_mediaType.SetSubtype(&MEDIASUBTYPE_AAC_AUDIO);
            m_pHandler.reset(new CoreAACHandler());
            break;
        default:
            {
                FOURCCMap aud(formatTag);
                m_mediaType.SetSubtype(&aud);
                break;
            }
        }
    }else
    {
        return false;
    }
    debugPrintf(DBG, L"AviElementaryType::Parse: success exit\r\n");
    return true;
}

bool 
AviElementaryType::IsVideo() const
{
    return m_streamHeader.fccType == streamtypeVIDEO;
}
    
void AviElementaryType::setHandler(const CMediaType* pmt, int idx)
{
    if(!m_pHandler.get())
        m_pHandler.reset(new NoChangeHandler());
}

bool 
AviElementaryType::GetType(CMediaType* pmt, int nType) const
{
    debugPrintf(DBG, L"AviElementaryType::GetType\r\n");
    if(nType != 0)
        return false;
    *pmt = m_mediaType;
    return true;
}

static const std::wstring cHandlersSubkey = TEXT("\\SOFTWARE\\Microsoft\\DirectX\\DirectShow\\SubMediaTypes");
bool AviElementaryType::FindFourCC(DWORD handlerFourCC, GUID& guid, bool& extendedFormat)
{
    std::vector<std::wstring> mediaKeys = Utils::RegistryAccessor::getSubKeys(HKEY_LOCAL_MACHINE, cHandlersSubkey);
    
    debugPrintf(DBG, L"AviElementaryType::FindFourCC: mediaKeys.size() = %u\r\n", mediaKeys.size());
    if(!mediaKeys.size())
        return false;

    std::string handlerFourCCStr(reinterpret_cast<const char*>(&handlerFourCC), sizeof(handlerFourCC));
    std::wstring handlerString = Utils::convertToWString(handlerFourCCStr);
    debugPrintf(DBG, L"AviElementaryType::FindFourCC: handlerString = %s, handlerFourCCStr = %S\r\n", handlerString.c_str(), handlerFourCCStr.c_str());
    for(size_t i = 0; i < mediaKeys.size(); ++i)
    {
        std::wstring valueName;
        DWORD val = Utils::RegistryAccessor::getInt(HKEY_LOCAL_MACHINE, cHandlersSubkey + TEXT("\\") + mediaKeys[i], handlerString, -1);
        if(val != -1)
        {
            debugPrintf(DBG, L"AviElementaryType::FindFourCC: match %s = %s\r\n", handlerString.c_str(), mediaKeys[i].c_str());
            if(CLSIDFromString((LPOLESTR) mediaKeys[i].c_str(), &guid) != NOERROR)
            {
                debugPrintf(DBG, L"AviElementaryType::FindFourCC: CLSIDFromString() has failed\r\n");
                return false;
            }
            extendedFormat = val == 1;
            return true;
        }
    }

    return false;
}
