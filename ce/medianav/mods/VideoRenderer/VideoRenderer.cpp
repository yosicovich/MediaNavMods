//------------------------------------------------------------------------------
// File: SampVid.cpp
//
// Desc: DirectShow sample code - illustrates a simple video renderer that
//       draws video into a text-shaped window on Windows NT or a simple
//       popup windows on Windows 95.  It shows hwo to use the base video 
//       renderer classes from the DirectX SDK.  A property page is 
//       implemented to allow users to find out quality management details.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Summary
//
// This is a sample DirectShow video renderer - the filter is based on the
// CBaseVideoRenderer base class. The base class handles all the seeking,
// synchronization and quality management necessary for video renderers.
// In particular, we override the DoRenderSample and PrepareRender methods so
// that we can draw images and realize palettes as necessary in Windows.
//
//
// Implementation
//
// The original idea was that the renderer would create a custom window that
// spelled ActiveX, and in the letters the video would be displayed. To create
// a window with a non rectangular clip region like this meant using paths.
// Unfortunately these are only truly supported on WindowsNT, so for Win95 we
// create a simple popup window (ie no system menu nor accellerator boxes).
//
// The renderer supports both IBasicVideo and IVideoWindow, which is achieved
// fairly simply by inheriting our renderer from the CBaseControlVideo and
// CBaseControlWindow base classes. To fully implement these interfaces we
// must then override and implement some more PURE virtual methods such as
// GetVideoFormat and Get/SetSourceRect (which all live in VIDEOTXT.CPP).
//
// Because we are either a simple popup window or a text shaped window we may
// not have a title bar for the user to grab to move the window around. So we
// handle WM_NCHITTEST messages (with HTCLIENT) to effectively enable window
// dragging by clicking on the video client area.
//
// We make heavy use of other base classes, notably the CImageAllocator which
// provides buffers that are really DIBSECTIONs. This enables faster drawing
// of video (and is the same method used by the real runtime renderer). We
// also use CImageDisplay to match up optimal drawing formats and for video
// type checking, CImagePalette for general palette creation and handling, and
// CDrawImage that can be used for general video drawing.
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. 
// Drag and drop an MPEG, AVI or MOV file into the tool and it will be rendered.
// Then go to the filters in the graph and find the filter (box) titled 
// "Video Renderer"
//
// This is the filter we will be replacing with the sample video renderer.
// Then click on the box and hit DELETE. After that go to the Graph menu and
// select "Insert Filters", from the dialog box find and select the "Sample
// Renderer" and then dismiss the dialog. Back in the graph layout find the
// output pin of the filter that was connected to the input of the video
// renderer you just deleted, right click and select "Render". You should
// see it being connected to the input pin of the renderer you just inserted
//
// Click Pause and Run on the GraphEdit frame and you will see the video...
//
//
// Files
//
// sampvid.cpp          Main implementation of the video renderer
// sampvid.def          What APIs the DLL will import and export
// sampvid.h            Class definition of the derived renderer
// sampvid.rc           Dialog box template for our property page
// videotxt.cpp         The code to look after a video window
// vidprop.cpp          The implementation of the property page
// vidprop.h            The class definition for the property page
//
//
// Base classes used
//
// CImageAllocator      A DIBSECTION video image allocator
// CVideoInputPin       IPin and IMemInputPin interfaces
// CImageDisplay        Manages the video display type
// CMediaType           Source connection media type
// CVideoWindow           Does the actual video rendering
// CImagePalette        Looks after managing a palette
// CDrawImage           Does the actual image drawing
//
//
#include <streams.h>
#include <initguid.h>
#include "VideoRenderer.h"
#include "os_api.h"

#define ERR		1
#define INFO    1
#ifdef TESTMODE
#define FUNC_TRACE 0
#define FUNC	0 //1
#define DBG	    1 
#else
#define FUNC_TRACE 0
#define FUNC	0
#define DBG	    0 
#endif

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

#define RendererName L"MediaNav RU mods Video Renderer"

CFactoryTemplate g_Templates[] = {
    { RendererName
    , &CLSID_VREND
    , CVideoRenderer::CreateInstance
    , NULL
    , NULL }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


//
// CreateInstance
//
// This goes in the factory template table to create new filter instances
//
CUnknown * WINAPI CVideoRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CVideoRenderer(NAME(RendererName), pUnk, phr);

} // CreateInstance


#pragma warning(disable:4355)

//
// Constructor
//
CVideoRenderer::CVideoRenderer(TCHAR *pName,
                               LPUNKNOWN pUnk,
                               HRESULT *phr) :

    CBaseVideoRendererAsync(CLSID_VREND,pName,pUnk,phr),
    m_InputPin(NAME("Video Pin"),this,&m_InterfaceLock,phr,L"Input"),
    m_ImageAllocator(this,NAME("YUV allocator"),phr),
    m_VideoWindow(NAME("Video properties"),GetOwner(),phr,&m_InterfaceLock,this),
	m_bActive(FALSE),
	m_bCacheEveryFrame(FALSE),
	m_bEnableCropAndAspectRatio(TRUE)
{
	OS_Print(FUNC, "CVideoRenderer::CVideoRenderer\r\n");
    // Store the video input pin
    m_pInputPin = &m_InputPin;

	// Read Registry for any setup info

	ReadRegistry();

    // Reset the current video size

    m_VideoSize.cx = 0;
    m_VideoSize.cy = 0;

    // Initialise the window and control interfaces

    m_VideoWindow.SetControlVideoPin(&m_InputPin);
    m_VideoWindow.SetControlWindowPin(&m_InputPin);

} // (Constructor)


//
// Destructor
//
CVideoRenderer::~CVideoRenderer()
{
	OS_Print(FUNC, "CVideoRenderer::~CVideoRenderer\r\n");
    ClearPendingSample();
    m_VideoWindow.ResetBuffers();
    m_pInputPin = NULL;
} // (Destructor)



// Called when the input pin receives an EndOfStream notification. If we have
// not got a sample, then notify EC_COMPLETE now. If we have samples, then set
// m_bEOS and check for this on completing samples. If we're waiting to pause
// then complete the transition to paused state by setting the state event

HRESULT CVideoRenderer::EndOfStream()
{
    // Ignore these calls if we are stopped
    
    if (m_State == State_Stopped) {
        return NOERROR;
    }
    
    // If we have a sample then wait for it to be rendered
    
    m_bEOS = TRUE;
	if (m_RendererQueue.Count()) 
	{
		return NOERROR;
    }
    
    // If we are waiting for pause then we are now ready since we cannot now
    // carry on waiting for a sample to arrive since we are being told there
    // won't be any. This sets an event that the GetState function picks up
    
    Ready();
    
    // Only signal completion now if we are running otherwise queue it until
    // we do run in StartStreaming. This is used when we seek because a seek
    // causes a pause where early notification of completion is misleading
    
    if (m_bStreaming) 
	{
        //m_VideoWindow.ResetBuffers();
        //m_VideoWindow.DoShowWindow(SW_HIDE);
        SendEndOfStream();
    }
    return NOERROR;
}

//
//	ReadRegistry
//
//	Check the registry for any setup info
//
void CVideoRenderer::ReadRegistry()
{
	HKEY hKey = NULL;
	ULONG nStatus;
	DWORD dwResult;
	DWORD dwSize;
	DWORD dwType;

	nStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_RENDERER_REGKEY, 0, 0, &hKey);
	
	if (nStatus != ERROR_SUCCESS) 
	{
		OS_Print(ERR, "CVideoRenderer: No video renderer registry entry\r\n");
	}	else 
	{
		dwSize = sizeof(dwResult);
		nStatus = RegQueryValueEx(hKey, TEXT("CacheEveryFrame"), NULL, &dwType, (LPBYTE)&dwResult, &dwSize);
		if (nStatus != ERROR_SUCCESS) 
		{
			// Couldn't open the key
		} else 
		{
			if (dwResult)
			{
				m_bCacheEveryFrame = TRUE;
			}
			else
			{
				m_bCacheEveryFrame = FALSE;
			}
		}
        OS_Print(DBG, "CVideoRenderer::ReadRegistry(): CacheEveryFrame=%d\r\n", m_bCacheEveryFrame);

		dwSize = sizeof(dwResult);
		nStatus = RegQueryValueEx(hKey, TEXT("EnableCropAndAspectRatio"), NULL, &dwType, (LPBYTE)&dwResult, &dwSize);
		if (nStatus != ERROR_SUCCESS) 
		{
			// Couldn't open the key
		} else 
		{
			if (dwResult)
			{
				m_bEnableCropAndAspectRatio = TRUE;
			}
			else
			{
				m_bEnableCropAndAspectRatio = FALSE;
			}
		}
		RegCloseKey(hKey);
	}
}


//
// CheckMediaType
//
// Check the proposed video media type
//
HRESULT CVideoRenderer::CheckMediaType(const CMediaType *pmtIn)
{
	OS_Print(FUNC, "CVideoRenderer::CheckMediaType\r\n");

	// if it's not a video format we can't handle it.
	if (pmtIn->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	m_VideoWindow.SetMPERender(FALSE);

	// now check if it's a video format we can handle
	if (pmtIn->subtype == MEDIASUBTYPE_UYVY)
	{
		return S_OK;
	}
	else if (pmtIn->subtype == MEDIASUBTYPE_YVYU)
	{
		return S_OK;
	}
	else if (pmtIn->subtype == MEDIASUBTYPE_YV12)
	{
		return S_OK;
	}
	else if (pmtIn->subtype == MEDIASUBTYPE_I420)
	{
		return S_OK;
	}
	else if (pmtIn->subtype == MEDIASUBTYPE_YUY2)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	else if (pmtIn->subtype == MEDIASUBTYPE_MPE1)
	{
		m_VideoWindow.SetMPERender(TRUE);
		return S_OK;
	}

	// didn't find a match... not supported
	return VFW_E_TYPE_NOT_ACCEPTED;

} // CheckMediaType



//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP CVideoRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

	OS_Print(FUNC, "CVideoRenderer::NonDelegatingQueryInterface\r\n");

    if (riid == IID_IVideoWindow) {
        return m_VideoWindow.NonDelegatingQueryInterface(riid,ppv);

    } else if (riid == IID_IBasicVideo) {
        return m_VideoWindow.NonDelegatingQueryInterface(riid,ppv);
    }

    return CBaseVideoRendererAsync::NonDelegatingQueryInterface(riid,ppv);

} // NonDelegatingQueryInterface


//
// DoRenderSample
//
// Have the drawing object render the current image
//
HRESULT CVideoRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
	PBYTE pbRGB = NULL;

	OS_Print(FUNC_TRACE, "CVideoRenderer::DoRenderSample\r\n");

	
    if ((m_State == State_Paused) || (m_bCacheEveryFrame)) 
	{
        ClearPendingSample();
		//
        // We're going to keep the sample for refreshing needs. If we
        // flush, any sample being held is going to be released
        //
        m_pMediaSample = pMediaSample;
        int c = m_pMediaSample->AddRef();
    }
    
	m_VideoWindow.RenderSample(pMediaSample);

	return NOERROR;
} // DoRenderSample

//
// Active
//
// The auto show flag is used to have the window shown automatically when we
// change state. We do this only when moving to paused or running, when there
// is no outstanding EC_USERABORT set and when the window is not already up
// This can be changed through the IVideoWindow interface AutoShow property.
// If the window is not currently visible then we are showing it because of
// a state change to paused or running, in which case there is no point in
// the video window sending an EC_REPAINT as we're getting an image anyway
//
HRESULT CVideoRenderer::Active()
{
    HWND hwnd = m_VideoWindow.GetWindowHWND();

	m_bActive = TRUE;

    if(IsWindowVisible(hwnd) == FALSE)
    {
        SetRepaintStatus(FALSE);

        m_VideoWindow.PerformanceAlignWindow();
    }
        m_VideoWindow.refreshWindowState();
    
    return CBaseVideoRendererAsync::Active();

} // Active

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CVideoRenderer::Inactive()
{
	m_bActive = FALSE;

	return CBaseVideoRendererAsync::Inactive();
}

HRESULT CVideoRenderer::BeginFlush()
{
	return CBaseVideoRendererAsync::BeginFlush();
}

void PrintVideoInfo(VIDEOINFO *pVideoInfo)
{
	OS_Print(1, "VIDEOINFO\r\n");
	OS_Print(1, "================================\r\n");
	OS_Print(1, "rcSource = (%d, %d) (%d x %d)\r\n", pVideoInfo->rcSource.left, pVideoInfo->rcSource.top, 
		(pVideoInfo->rcSource.right - pVideoInfo->rcSource.left), (pVideoInfo->rcSource.bottom - pVideoInfo->rcSource.top) );
	OS_Print(1, "rcTarget = (%d, %d) (%d x %d)\r\n", pVideoInfo->rcTarget.left, pVideoInfo->rcTarget.top, 
		(pVideoInfo->rcTarget.right - pVideoInfo->rcTarget.left), (pVideoInfo->rcTarget.bottom - pVideoInfo->rcTarget.top) );
	OS_Print(1, "dwBitRate = %d\r\n", pVideoInfo->dwBitRate);
	OS_Print(1, "dwBitErrorRate = %d\r\n", pVideoInfo->dwBitErrorRate);
	OS_Print(1, "BITMAPINFOHEADER\r\n");
	OS_Print(1, "================================\r\n");
	OS_Print(1, "biSize = %d\r\n", pVideoInfo->bmiHeader.biSize);
	OS_Print(1, "biWidth = %d\r\n", pVideoInfo->bmiHeader.biWidth);
	OS_Print(1, "biHeight = %d\r\n", pVideoInfo->bmiHeader.biHeight);
	OS_Print(1, "biPlanes = %d\r\n", pVideoInfo->bmiHeader.biPlanes);
	OS_Print(1, "biBitCount = %d\r\n", pVideoInfo->bmiHeader.biBitCount);
	OS_Print(1, "biCompression = %d\r\n", pVideoInfo->bmiHeader.biCompression);
	OS_Print(1, "biSizeImage = %d\r\n", pVideoInfo->bmiHeader.biSizeImage);
	OS_Print(1, "biXPelsPerMeter = %d\r\n", pVideoInfo->bmiHeader.biXPelsPerMeter);
	OS_Print(1, "biYPelsPerMeter = %d\r\n", pVideoInfo->bmiHeader.biYPelsPerMeter);
	OS_Print(1, "biClrUsed = %d\r\n", pVideoInfo->bmiHeader.biClrUsed);
	OS_Print(1, "biClrImportant = %d\r\n\r\n", pVideoInfo->bmiHeader.biClrImportant);
}

void PrintVideoInfoHeader(VIDEOINFOHEADER *pVideoInfo)
{
	OS_Print(1, "VIDEOINFOHEADER\r\n");
	OS_Print(1, "================================\r\n");
	OS_Print(1, "rcSource = (%d, %d) (%d x %d)\r\n", pVideoInfo->rcSource.left, pVideoInfo->rcSource.top, 
		(pVideoInfo->rcSource.right - pVideoInfo->rcSource.left), (pVideoInfo->rcSource.bottom - pVideoInfo->rcSource.top) );
	OS_Print(1, "rcTarget = (%d, %d) (%d x %d)\r\n", pVideoInfo->rcTarget.left, pVideoInfo->rcTarget.top, 
		(pVideoInfo->rcTarget.right - pVideoInfo->rcTarget.left), (pVideoInfo->rcTarget.bottom - pVideoInfo->rcTarget.top) );
	OS_Print(1, "dwBitRate = %d\r\n", pVideoInfo->dwBitRate);
	OS_Print(1, "dwBitErrorRate = %d\r\n", pVideoInfo->dwBitErrorRate);
	OS_Print(1, "BITMAPINFOHEADER\r\n");
	OS_Print(1, "================================\r\n");
	OS_Print(1, "biSize = %d\r\n", pVideoInfo->bmiHeader.biSize);
	OS_Print(1, "biWidth = %d\r\n", pVideoInfo->bmiHeader.biWidth);
	OS_Print(1, "biHeight = %d\r\n", pVideoInfo->bmiHeader.biHeight);
	OS_Print(1, "biPlanes = %d\r\n", pVideoInfo->bmiHeader.biPlanes);
	OS_Print(1, "biBitCount = %d\r\n", pVideoInfo->bmiHeader.biBitCount);
	OS_Print(1, "biCompression = %d\r\n", pVideoInfo->bmiHeader.biCompression);
	OS_Print(1, "biSizeImage = %d\r\n", pVideoInfo->bmiHeader.biSizeImage);
	OS_Print(1, "biXPelsPerMeter = %d\r\n", pVideoInfo->bmiHeader.biXPelsPerMeter);
	OS_Print(1, "biYPelsPerMeter = %d\r\n", pVideoInfo->bmiHeader.biYPelsPerMeter);
	OS_Print(1, "biClrUsed = %d\r\n", pVideoInfo->bmiHeader.biClrUsed);
	OS_Print(1, "biClrImportant = %d\r\n\r\n", pVideoInfo->bmiHeader.biClrImportant);
}
//
// SetMediaType
//
// We store a copy of the media type used for the connection in the renderer
// because it is required by many different parts of the running renderer
// This can be called when we come to draw a media sample that has a format
// change with it. We normally delay type changes until they are really due
// for rendering otherwise we will change types too early if the source has
// allocated a queue of samples. In our case this isn't a problem because we
// only ever receive one sample at a time so it's safe to change immediately
//
HRESULT CVideoRenderer::SetMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

	OS_Print(FUNC, "CVideoRenderer::SetMediaType\r\n");

    HRESULT hr = NOERROR;
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    CMediaType StoreFormat(m_mtIn);

    // Fill out the optional fields in the VIDEOINFOHEADER

    m_mtIn = *pmt;
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_mtIn.Format();
    if(INFO) 
        PrintVideoInfo(pVideoInfo);

	if (pmt->subtype == MEDIASUBTYPE_UYVY)
	{
		OS_Print(INFO, "SetMediaType -- MEDIASUBTYPE_UYVY\r\n");
		m_VideoWindow.SetInputFormat(FOURCC_UYVY);
	}
	else if (pmt->subtype == MEDIASUBTYPE_YV12)
	{
		OS_Print(INFO, "SetMediaType -- YV12\r\n");
		m_VideoWindow.SetInputFormat(FOURCC_YV12);
	}
	else if (pmt->subtype == MEDIASUBTYPE_YVYU)
	{
		OS_Print(INFO, "SetMediaType -- YVYU\r\n");
		m_VideoWindow.SetInputFormat(FOURCC_YVYU);
	}
	else if (pmt->subtype == MEDIASUBTYPE_MPE1)
	{
		OS_Print(INFO, "SetMediaType --  MEDIASUBTYPE_MPE1(FOURCC_MPEYUV)\r\n");
		m_VideoWindow.SetInputFormat(FOURCC_MPEYUV);
	}else if (pmt->subtype == MEDIASUBTYPE_I420)
	{
		OS_Print(INFO, "SetMediaType --  FOURCC_I420\r\n");
		m_VideoWindow.SetInputFormat(FOURCC_I420);
	}else	
	{
		OS_Print(INFO, "SetMediaType --  Not a valid mediatype\r\n");
		return VFW_E_INVALIDMEDIATYPE;
	}
    // Complete the initialisation

//    m_DrawImage.NotifyMediaType(&m_mtIn);
// TODO:
//	Should this call go to the video window?

    m_ImageAllocator.NotifyMediaType(&m_mtIn);
    return NOERROR;

} // SetMediaType


//
// BreakConnect
//
// This is called when a connection or an attempted connection is terminated
// and lets us to reset the connection flag held by the base class renderer
// The filter object may be hanging onto an image to use for refreshing the
// video window so that must be freed (the allocator decommit may be waiting
// for that image to return before completing) then we must also uninstall
// any palette we were using, reset anything set with the control interfaces
// then set our overall state back to disconnected ready for the next time

HRESULT CVideoRenderer::BreakConnect()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

	OS_Print(FUNC, "CVideoRenderer::BreakConnect\r\n");

    // Check we are in a valid state

    HRESULT hr = CBaseVideoRendererAsync::BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }

    // The window is not used when disconnected
    IPin *pPin = m_InputPin.GetConnected();
    if (pPin) 
        SendNotifyWindow(pPin,NULL);

    m_mtIn.ResetFormatBuffer();

    return NOERROR;

} // BreakConnect


//
// CompleteConnect
//
// When we complete connection we need to see if the video has changed sizes
// If it has then we activate the window and reset the source and destination
// rectangles. If the video is the same size then we bomb out early. By doing
// this we make sure that temporary disconnections such as when we go into a
// fullscreen mode do not cause unnecessary property changes. The basic ethos
// is that all properties should be persistent across connections if possible
//
HRESULT CVideoRenderer::CompleteConnect(IPin *pReceivePin)
{
	BITMAPINFOHEADER *pBMI;

    CAutoLock cInterfaceLock(&m_InterfaceLock);

	OS_Print(FUNC, "CVideoRenderer::CompleteConnect\r\n");
	
    CBaseVideoRendererAsync::CompleteConnect(pReceivePin);

	if (m_mtIn.formattype == FORMAT_VideoInfo)
	{
		pBMI = &(((VIDEOINFOHEADER*)m_mtIn.pbFormat)->bmiHeader);
/*        if(INFO)
            PrintVideoInfoHeader((VIDEOINFOHEADER*)m_mtIn.pbFormat);*/
	}
	else if (m_mtIn.formattype == FORMAT_VideoInfo2)
	{
		pBMI = &(((VIDEOINFOHEADER2*)m_mtIn.pbFormat)->bmiHeader);
	}
	else
	{
		return E_FAIL;
	}

	// Has the video size changed between connections
	if (pBMI->biWidth == m_VideoSize.cx) 
	{
		if (pBMI->biHeight == m_VideoSize.cy) 
		{
			return NOERROR;
		}
	}

    // Pass the video window handle upstream
    HWND hwnd = m_VideoWindow.GetWindowHWND();
//    NOTE1("Sending EC_NOTIFY_WINDOW %x",hwnd);

    SendNotifyWindow(pReceivePin,hwnd);

    // Set them for the current video dimensions
    m_VideoSize.cx = pBMI->biWidth;
    m_VideoSize.cy = pBMI->biHeight;

    m_VideoWindow.SetDefaultSourceRect();
    m_VideoWindow.SetDefaultTargetRect();
    m_VideoWindow.OnVideoSizeChange();
    m_VideoWindow.ActivateWindow();

    return NOERROR;

} // CompleteConnect


//
// OnReceiveFirstSample
//
// Use the image just delivered to display a poster frame
//
void CVideoRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);

    OS_Print(FUNC, "CVideoRenderer::OnReceiveFirstSample\r\n");
    m_VideoWindow.refreshWindowState();

	DoRenderSample(pMediaSample);
} // OnReceiveFirstSample

inline int INCLUSIVE_RECT_WIDTH(CroppingRect_t & r)
{
	return (r.right == r.left) ? 0 : (r.right - r.left + 1);
}
inline int INCLUSIVE_RECT_HEIGHT(CroppingRect_t & r)
{
	return (r.bottom == r.top) ? 0 : (r.bottom - r.top + 1);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CVideoRenderer::Receive(IMediaSample* pMediaSample)
{
	HRESULT hr;

	OS_Print(FUNC_TRACE, "CVideoRenderer::Receive\r\n");

	if (m_VideoWindow.IsMPERender())
	{
		VideoSample_t *pVideoSample;
		pMediaSample->GetPointer((BYTE **)&pVideoSample);
		if (pVideoSample->flags & VS_FLAGS_SEGMENT)
		{
			if (pVideoSample->geometry.yuv.cropping_rect.bottom < (pVideoSample->height-1))
			{
				m_VideoWindow.RenderSample(pMediaSample);
				return S_OK;
			}
		}
		if (m_bEnableCropAndAspectRatio)
		{
			int cxCrop = INCLUSIVE_RECT_WIDTH(pVideoSample->geometry.yuv.cropping_rect);
			int cyCrop = INCLUSIVE_RECT_HEIGHT(pVideoSample->geometry.yuv.cropping_rect);
			m_VideoWindow.SetAspectRatio(pVideoSample->geometry.aspect_ratio_width, pVideoSample->geometry.aspect_ratio_height, pVideoSample->geometry.aspect_ratio_type,
				cxCrop, cyCrop);
		}
	}


	hr = CBaseVideoRendererAsync::Receive(pMediaSample);

	return hr;
}

// Called when the source delivers us a sample. We go through a few checks to
// make sure the sample can be rendered. If we are running (streaming) then we
// have the sample scheduled with the reference clock, if we are not streaming
// then we have received an sample in paused mode so we can complete any state
// transition. On leaving this function everything will be unlocked so an app
// thread may get in and change our state to stopped (for example) in which
// case it will also signal the thread event so that our wait call is stopped

HRESULT CVideoRenderer::PrepareReceive(IMediaSample *pMediaSample)
{
	HRESULT hr;

	OS_Print(FUNC_TRACE, "CVideoRenderer::PrepareReceive\r\n");
	
	hr = CBaseRendererAsync::PrepareReceive(pMediaSample);

	// Process the frame through the ITE now.
	if (NOERROR == hr)
	{
		hr = m_VideoWindow.PrepareReceive(pMediaSample);
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CVideoRenderer::Stop()
{
	HRESULT hr;

	OS_Print(FUNC, "CVideoRenderer::Stop()\r\n");
		
	hr = CBaseRendererAsync::Stop();

	// TODO:
	// Fill an RGB buffer with the background color and set that as the next buffer
	//m_VideoWindow.ResetBuffers();
	//m_VideoWindow.DoShowWindow(SW_HIDE);

	return hr;
}

STDMETHODIMP CVideoRenderer::Pause()
{
    HRESULT hr;

    OS_Print(FUNC, "CVideoRenderer::Pause()\r\n");

    hr = CBaseRendererAsync::Pause();

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CVideoRenderer::Run(REFERENCE_TIME tStart)
{
  HRESULT hr;
  
  OS_Print(FUNC, "CVideoRenderer::Run\r\n");

  hr = CBaseRendererAsync::Run(tStart);
  
  return hr;
}

LONG CVideoRenderer:: RendererQueueCount()
{
	return m_RendererQueue.Count();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	Video Input Pin
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CVideoInputPin::CVideoInputPin(TCHAR *pObjectName,
                               CVideoRenderer *pRenderer,
                               CCritSec *pInterfaceLock,
                               HRESULT *phr,
                               LPCWSTR pPinName) :

    CRendererInputPin(pRenderer,phr,pPinName),
    m_pRenderer(pRenderer),
    m_pInterfaceLock(pInterfaceLock)
{
    ASSERT(m_pRenderer);
    ASSERT(pInterfaceLock);

	OS_Print(FUNC, "CVideoRenderer::CVideoInputPin\r\n");

} // (Constructor)


//
// GetAllocator
//
// This overrides the CBaseInputPin virtual method to return our allocator
// we create to pass shared memory DIB buffers that GDI can directly access
// When NotifyAllocator is called it sets the current allocator in the base
// input pin class (m_pAllocator), this is what GetAllocator should return
// unless it is NULL in which case we return the allocator we would like
//
STDMETHODIMP CVideoInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    CAutoLock cInterfaceLock(m_pInterfaceLock);

	OS_Print(FUNC, "CVideoRenderer::GetAllocator\r\n");

    // Has an allocator been set yet in the base class

    if (m_pAllocator == NULL) 
    {
        m_pAllocator = &m_pRenderer->m_ImageAllocator;
        m_pAllocator->AddRef();
    }

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;

    return NOERROR;

} // GetAllocator


//
// NotifyAllocator
//
// The COM specification says any two IUnknown pointers to the same object
// should always match which provides a way for us to see if they are using
// our DIB allocator or not. Since we are only really interested in equality
// and our object always hands out the same IMemAllocator interface we can
// just see if the pointers match. If they are we set a flag in the main
// renderer as the window needs to know whether it can do fast rendering
//
STDMETHODIMP
CVideoInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

	OS_Print(FUNC, "CVideoRenderer::NotifyAllocator\r\n");

    // Make sure the base class gets a look

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
    if (FAILED(hr))
        return hr;

    // Whose allocator is the source going to use?
	if (pAllocator != m_pAllocator)
	{
		if (m_pAllocator)
			m_pAllocator-> Release();

		m_pAllocator = pAllocator;
	}

    return NOERROR;

} // NotifyAllocator


STDMETHODIMP CVideoInputPin::Receive(IMediaSample *pSample)
{
	OS_Print(FUNC_TRACE, "InputPin -- RECEIVE\r\n");
	return CRendererInputPin::Receive(pSample);
}

HRESULT CVideoInputPin::CompleteConnect(IPin *pReceivePin)
{
	HRESULT hResult = __super::CompleteConnect(pReceivePin);

	if (SUCCEEDED(hResult))
	{
		hResult = MakeSecureConnection(pReceivePin);
	}

	return hResult;
}


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterSever
//
// Handle the registration of this filter
//
STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	OS_Print(FUNC, "DllMain\r\n");
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

