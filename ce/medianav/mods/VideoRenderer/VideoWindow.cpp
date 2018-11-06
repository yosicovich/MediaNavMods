//------------------------------------------------------------------------------
// File: VideoTxt.cpp
//
// Desc: DirectShow sample code - implements DirectShow video renderer.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


// This is a sample renderer that displays video in a window. What is special
// about that is that the window is non rectangular - the region it has is a
// word (whatever VideoString is defined to be as shown below). Using various
// GDI methods we create a region from the word and select it into our window,
// Windows will then handle clipping and mouse interactions, so all we have
// to do is handle the painting and drawing. This sample can work if it uses
// either its own allocator (the preferred approach) or a source filter's allocator
// (like when it connects to the tee filter). It has separate code paths to
// handle the drawing, depending whose allocator it is using on this connection.
#include <windows.h>
#include <streams.h>
#include "lcd.h"
#include "os_api.h"
#include "VideoRenderer.h"
#include "dshow\AuMedia.h"
#include <utils.h>

#define FUNC	0 //1
#define ERR		1
#define DBG	    1 


#define ASPECT_PRECISION 0x10000
#define MAX_DOWNSCALE		32
#define MAX_UPSCALE			4

CCritSec CVideoWindow::m_csMempool;

//
// Constructor
//
CVideoWindow::CVideoWindow(TCHAR *pName,             // Object description
                       LPUNKNOWN pUnk,               // Normal COM ownership
                       HRESULT *phr,                 // OLE failure code
                       CCritSec *pInterfaceLock,     // Main critical section
                       CVideoRenderer *pRenderer) :  // Delegates locking to

    CBaseControlVideo(pRenderer,pInterfaceLock,pName,pUnk,phr),
    CBaseControlWindow(pRenderer,pInterfaceLock,pName,pUnk,phr),
    m_pRenderer(pRenderer),
	m_CurOverlayBuffer(0),
	m_OSD_enabled(FALSE),
	m_ITE(pRenderer),
	m_bSizeChanged(FALSE),
    m_aspect_ratio_width(1),
    m_aspect_ratio_height(1),
	m_aspect_ratio_type(arSample),
    m_cxCrop(0),
    m_cyCrop(0),
    m_visible(FALSE)
{
    SetRectEmpty(&m_rcTarget);

	CreateOverlay();
	GetRegistryValues();
	GetColorKey();
	SetOutputFormat();
	PrepareWindow();

	// Get the current display information to initialize the overlay and ITE with
	m_pOverlay->CheckDisplayChange();
	m_ITE.SetRotation(m_pOverlay->GetRotation());
	
	AllocateBuffers();

} // (Constructor)


//
// Destructor
//
CVideoWindow::~CVideoWindow()
{
	InactivateWindow();
    DoneWithWindow();
	DestroyOverlay();
	FreeBuffers();
} // (Destructor)

void CVideoWindow::AllocateBuffers()
{
	int Width = OS_GetScreenWidth();
	int Height = OS_GetScreenHeight();

	m_hMem = mem_open_driver();

	m_nOverlayBufferSize = Width * Height * m_pOverlay->GetOverlayBytesPP();

	///////////////////////////////////////////////////////////////////////////
	// Lock here and do not unlock until we've deallocated this.  This protects
	// us if we have multiple renderers created at once during playlists.
	//////////////////////////////////////////////////////////////////////////////
	m_csMempool.Lock();



	for (int i = 0; NUM_OVERLAY_BUFFERS > i; i++)
	{
		m_pOverlayBuffers[i] = mem_alloc(m_hMem, m_nOverlayBufferSize, REGION_LCD, 0);
	}

	ResetBuffers();
}

void CVideoWindow::FreeBuffers()
{
	CAutoLock lock(&m_csWindow);


	for (int i = 0; NUM_OVERLAY_BUFFERS > i; i++)
	{

		if (m_pOverlayBuffers)
		{
			mem_free(m_hMem, m_pOverlayBuffers[i]);
		}
	}

	m_csMempool.Unlock();

	mem_close_driver(m_hMem);
}

void CVideoWindow::IncBufferCounter()
{
	m_CurOverlayBuffer++;

	if (m_CurOverlayBuffer == NUM_OVERLAY_BUFFERS)
	{
		m_CurOverlayBuffer = 0;
	}

}

void CVideoWindow::ResetBuffers()
{
	CAutoLock lock(&m_csWindow);

	// Initializing to black in case we drop pictures and ensures no tearing on the first frame.
	memset((void *)m_pOverlayBuffers[m_CurOverlayBuffer]->pVirtual, 0x00, m_pOverlayBuffers[m_CurOverlayBuffer]->dwSize);
	m_pOverlay->SetCurrentBuffer((unsigned int)m_pOverlayBuffers[m_CurOverlayBuffer]->pPhysical);
	IncBufferCounter();
}

//
// NonDelegatingQueryInterface
//
// Overriden to say what interfaces we support and where
//
STDMETHODIMP CVideoWindow::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IVideoWindow)
    {
        return CBaseVideoWindow::NonDelegatingQueryInterface(riid,ppv);
    }
    else
    {
        ASSERT(riid == IID_IBasicVideo);
        return CBaseBasicVideo::NonDelegatingQueryInterface(riid,ppv);
    }
}


//
// GetClassWindowStyles
//
// When we call PrepareWindow in our constructor it will call this method as
// it is going to create the window to get our window and class styles. The
// return code is the class name and must be allocated in static storage. We
// specify a normal window during creation although the window styles as well
// as the extended styles may be changed by the application via IVideoWindow
//
LPTSTR CVideoWindow::GetClassWindowStyles(DWORD *pClassStyles,
                                        DWORD *pWindowStyles,
                                        DWORD *pWindowStylesEx)
{
	CheckPointer(pClassStyles,NULL);
	CheckPointer(pWindowStyles,NULL);
	CheckPointer(pWindowStylesEx,NULL);

	OSVERSIONINFO VersionInfo;
	VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	EXECUTE_ASSERT(GetVersionEx(&VersionInfo));

	*pClassStyles    = CS_HREDRAW | CS_VREDRAW; // | CS_BYTEALIGNCLIENT;
	*pWindowStyles   = WS_POPUP | WS_CLIPCHILDREN;
	*pWindowStylesEx = (DWORD) 0;

	return TEXT("VideoRenderer\0");
} // GetClassWindowStyles

static const int buttonsBarHeigh = 61;

void CVideoWindow::AjustWindowSize(BOOL maximized)
{
	if(maximized)
	{
        SetWindowPosition(0, 0, OS_GetScreenWidth(), OS_GetScreenHeight());
        //SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, OS_GetScreenWidth(), OS_GetScreenHeight(), 0);
	}else
	{
        SetWindowPosition(0, 0, OS_GetScreenWidth(), OS_GetScreenHeight() - buttonsBarHeigh);
		//SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, OS_GetScreenWidth(), OS_GetScreenHeight() - buttonsBarHeigh, 0);
	}
    setFullScreen(maximized);
}

HRESULT CVideoWindow::DoShowWindow(LONG ShowCmd)
{
  	BOOL bShow = TRUE;

	switch (ShowCmd)
	{
	///////////////////////////////////////////////////////////////////////////
	// These commands hide the overlay
	///////////////////////////////////////////////////////////////////////////
	case SW_HIDE:
		// Hides the window and activates another window.
	case SW_MINIMIZE:
		// Minimizes the specified window and activates the next top-level
		// window in the Z order.
		bShow = FALSE;
		break;

	///////////////////////////////////////////////////////////////////////////
	// These commands show the overlay
	///////////////////////////////////////////////////////////////////////////
	case SW_SHOWNA:
		// Displays the window in its current size and position. This value is
		// similar to SW_SHOW, except the window is not activated.
	case SW_SHOW:
		// Activates the window and displays it in its current size and position.
		bShow = TRUE;
		break;
	case SW_RESTORE:
		// Activates and displays the window. If the window is minimized or
		// maximized, the system restores it to its original size and position.
		// An application should specify this flag when restoring a minimized
		// window.
	case SW_SHOWNOACTIVATE:
		// Displays a window in its most recent size and position. This value is
		// similar to SW_SHOWNORMAL, except the window is not actived.
	case SW_SHOWNORMAL:
		// Activates and displays a window. If the window is minimized or maximized,
		// the system restores it to its original size and position. An application
		// should specify this flag when displaying the window for the first time.
		AjustWindowSize(FALSE);
		bShow = TRUE;
		break;
	case SW_MAXIMIZE:
		// Maximizes the specified window.
	case SW_SHOWMAXIMIZED:
		// Activates the window and displays it as a maximized window.
		AjustWindowSize(TRUE);
		bShow = TRUE;
		break;

	///////////////////////////////////////////////////////////////////////////
	// These commands arent valid
	///////////////////////////////////////////////////////////////////////////
	default:
		RETAILMSG(1, (TEXT("CVideoWindow::DoShowWindow -- Invalid Show Command (0x%x)\r\n"), ShowCmd));
	}

	m_visible = bShow;

    m_pOverlay->ShowOverlay(bShow && !isNoShowState());

	return CBaseWindow::DoShowWindow(ShowCmd);
}


//
// GetDefaultRect
//
// Return the default window rectangle
//
RECT CVideoWindow::GetDefaultRect()
{
	SIZE VideoSize = m_pRenderer->m_VideoSize;
	
	RECT DefaultRect = {0, 0, VideoSize.cx, VideoSize.cy};
    ASSERT(m_hwnd);
    ASSERT(m_hdc);

    return DefaultRect;

} // GetDefaultRect


//
// OnReceiveMessage
//
// This is the derived class window message handler methods
//
LRESULT CVideoWindow::OnReceiveMessage(HWND hwnd,          // Window handle
                                     UINT uMsg,          // Message ID
                                     WPARAM wParam,      // First parameter
                                     LPARAM lParam)      // Other parameter
{
    IBaseFilter *pFilter = NULL;

	switch(uMsg)
	{
	case WM_PAINT:
		OnPaint(hwnd);
		break;

	case WM_SIZE:
		{
			RECT WinRect;

			GetWindowRect(hwnd, &WinRect);
			SetTargetRect(&WinRect);
		}
		break;

	case WM_ERASEBKGND:
		return OnEraseBackground(reinterpret_cast<HDC>(wParam), lParam, hwnd);
		break;

	case WM_CLOSE:
		// Handle WM_CLOSE by aborting the playback
		m_pRenderer->NotifyEvent(EC_USERABORT,0,0);
        DoShowWindow(SW_HIDE);

        return CBaseWindow::OnClose();
		break;

	case WM_SETCURSOR:
		// The base class that implements IVideoWindow looks after a flag
		// that says whether or not the cursor should be hidden. If so we
		// hide the cursor and return (LRESULT) 1. Otherwise we pass to
		// the DefWindowProc to show the cursor as normal. This is used
		// when our window is made fullscreen to imitate the Modex filter
		if (IsCursorHidden() == TRUE)
        {
            SetCursor(NULL);
            return (LRESULT) 1;
        }
		break;

	case WM_WINDOWPOSCHANGED:
		break;
	case WM_LBUTTONUP:
		AjustWindowSize(!getFullScreen());
		break;
	};

    return CBaseWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

} // OnReceiveMessage


LRESULT CVideoWindow::OnEraseBackground(HDC hdc, LPARAM lParam, HWND hwnd)
{
	RECT ClientRect;
	HBRUSH hBrush;

	GetClientRect(hwnd, &ClientRect);

	hBrush = CreateSolidBrush(m_KeyColor);
    EXECUTE_ASSERT(FillRect(hdc,&ClientRect,hBrush));
    EXECUTE_ASSERT(DeleteObject(hBrush));

    return 0;
}

//
//
//
void CVideoWindow::RepaintLastImage()
{
	IMediaSample *pMediaSample;

	pMediaSample = m_pRenderer->GetCurrentSample();

	if (pMediaSample)
	{
		RenderSample(pMediaSample);
		pMediaSample->Release();
	}
}

LRESULT CVideoWindow::OnPaint(HWND hwnd)
{
	RECT WinRect;
	RECT ClientRect;
	HBRUSH hBrush;
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwnd, &ps);

	GetWindowRect(hwnd, &WinRect);
	GetClientRect(hwnd, &ClientRect);

	OS_Print(FUNC, "WinRect: (%d, %d) (%d x %d)\r\n", WinRect.left, WinRect.top,
		(WinRect.right - WinRect.left), (WinRect.bottom - WinRect.top));

	m_pOverlay->CheckDisplayChange();

	m_ITE.SetRotation(m_pOverlay->GetRotation());

	SetTargetRect(&WinRect);

	if (m_bSizeChanged)
	{
		RepaintLastImage();
		m_bSizeChanged = FALSE;
	}

	// Paint the colorkey only when player window active
	hBrush = CreateSolidBrush(m_KeyColor);
	EXECUTE_ASSERT(FillRect(hdc,&ClientRect,hBrush));
	EXECUTE_ASSERT(DeleteObject(hBrush));

	EndPaint(hwnd, &ps);

	return NOERROR;
}

#define CheckConnected(pin,code)              \
{                                             \
  if (pin == NULL)                            \
  {                                           \
    ASSERT(!TEXT("Pin not set"));             \
  } else if (pin->IsConnected() == FALSE)     \
  {                                           \
    return (code);                            \
  }                                           \
}

//
// SetDefaultTargetRect
//
// This is called when we reset the default target rectangle
//
HRESULT CVideoWindow::SetDefaultTargetRect()
{
    // VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pRenderer->m_mtIn.Format();
    // BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

	SIZE VideoSize = m_pRenderer->m_VideoSize;

    RECT TargetRect = {0, 0, VideoSize.cx ,VideoSize.cy};
	SetTargetRect(&TargetRect);

    return NOERROR;

} // SetDefaultTargetRect


//
// IsDefaultTargetRect
//
// Return S_OK if using the default target otherwise S_FALSE
//
HRESULT CVideoWindow::IsDefaultTargetRect()
{
    RECT TargetRect;
	SIZE VideoSize = m_pRenderer->m_VideoSize;

    m_ITE.GetTargetRect(&TargetRect);

    // Check the destination matches the initial client area

    if (TargetRect.left != 0  ||
        TargetRect.top  != 0  ||
        TargetRect.right  != VideoSize.cx ||
        TargetRect.bottom != VideoSize.cy)
    {
        return S_FALSE;
    }

    return S_OK;

} // IsDefaultTargetRect


void 
CVideoWindow::SetAspectRatio(int cx, int cy, int aspect_type, int cxCrop, int cyCrop)
{
    CAutoLock lock(&m_csWindow);
    if ((cx != m_aspect_ratio_width) || (cy != m_aspect_ratio_height) || (cxCrop != m_cxCrop) || (cyCrop != m_cyCrop))
    {
        m_aspect_ratio_height = cy;
        m_aspect_ratio_width = cx;
		m_aspect_ratio_type = aspect_type;
        m_cxCrop = cxCrop;
        m_cyCrop = cyCrop;
        if (!IsRectEmpty(&m_rcTarget))
        {
            RECT rc = m_rcTarget;
            SetTargetRect(&rc);
        }
    }
}

//////////////////////////////////////////////////////////////////////
static void
vrenderClampUpscale(int *dest_a, int *dest_b, int src_a)
{
    const int max_a = src_a * MAX_UPSCALE;
    if((*dest_a) > max_a)
    {
        float clamp = (float) max_a / (*dest_a);
        (*dest_b) = (int)((float)(*dest_b) * clamp);
        (*dest_a) = max_a;
    }
}

//////////////////////////////////////////////////////////////////////
static void
vrenderClampDownscale(int *dest_a, int *dest_b, int src_a)
{
    const int min_a = (int)((float)src_a / (float) MAX_DOWNSCALE);
    if((*dest_a) < min_a)
    {
        float clamp = (float) min_a / (*dest_a);
        (*dest_b) = (int)((float)(*dest_b) * clamp);
        (*dest_a) = min_a;
    }
}

int CVideoWindow::AdjustAspectRatio(int *max_output_width, int *max_output_height)
{

	float	pixel_aspect_ratio;
    float	rgb_panel_aspect_ratio;
	RECT	SourceRect;
	int		yuv_width;
	int		yuv_height;
	int		out_width;
    int		out_height;

	m_ITE.GetSourceRect(&SourceRect);

	yuv_width = RECT_WIDTH(SourceRect);
	yuv_height = RECT_HEIGHT(SourceRect);

    if(m_aspect_ratio_width > 0 && m_aspect_ratio_height > 0)
    {
        pixel_aspect_ratio = ((float) m_aspect_ratio_width / m_aspect_ratio_height);

        OS_Print(FUNC,"pixel_aspect_ratio: %f %d %d\r\n",
              pixel_aspect_ratio,
              m_aspect_ratio_width,
              m_aspect_ratio_height);
    }
    else
    {
        pixel_aspect_ratio = 1.0;
    }

    rgb_panel_aspect_ratio = (float) *max_output_width / *max_output_height;

    if(arSample == m_aspect_ratio_type)
    {
        float original_pixel_aspect_ratio = pixel_aspect_ratio * (float) yuv_width / yuv_height;
        OS_Print(FUNC,"OAR: %0.3f, IAR: %0.3f\r\n", rgb_panel_aspect_ratio, original_pixel_aspect_ratio);

        pixel_aspect_ratio = original_pixel_aspect_ratio;
  
    }


    OS_Print(FUNC,"YUV ar: %f, RGB ar: %f\r\n", pixel_aspect_ratio, rgb_panel_aspect_ratio);

    if(pixel_aspect_ratio > rgb_panel_aspect_ratio)
    {
        // Clamp to max width
        out_width  = *max_output_width;
        out_height = (unsigned) ((float) (*max_output_width) / pixel_aspect_ratio);
    }
    else
    {
        // Clamp to max height
        out_width  = (unsigned) ((*max_output_height) * pixel_aspect_ratio);
        out_height = *max_output_height;
    }

    vrenderClampUpscale(&out_width,  &out_height, yuv_width);
    vrenderClampUpscale(&out_height, &out_width,  yuv_height);

    vrenderClampDownscale(&out_width,  &out_height, yuv_width);
    vrenderClampDownscale(&out_height, &out_width,  yuv_height);

	*max_output_width = out_width;
	*max_output_height = out_height;

	return 0;
}


//
// SetTargetRect
//
// This is called to set the target rectangle in the video window, it will be
// called whenever a WM_SIZE message is retrieved from the message queue. We
// simply store the rectangle and use it later when we do the drawing calls
//
HRESULT CVideoWindow::SetTargetRect(RECT *pTargetRect)
{
	int		OutWidth;
	int		OutHeight;
	int		InWidth = OutWidth = RECT_WIDTH(*pTargetRect);
	int		InHeight = OutHeight = RECT_HEIGHT(*pTargetRect);
	int		MaxWidth;
	int		MaxHeight;
	RECT	SourceRect;
	RECT	IntermediateRect = *pTargetRect;
	RECT	NewRect;
	RECT	OldRect;
	DWORD	dwScreenHeight = m_pOverlay->GetScreenHeight();
	DWORD	dwScreenWidth = m_pOverlay->GetScreenWidth();
	DWORD	dwScreenAspectRatio = (dwScreenWidth * ASPECT_PRECISION) / dwScreenHeight;
	DWORD	dwMovieAspectRatio = ASPECT_PRECISION;
	int		XOffset = 0;
	int		YOffset = 0;

	m_pOverlay->GetOverlayRect(&OldRect);

	OS_Print(FUNC, "CVideoWindow::SetTargetRect\r\n");
	OS_Print(FUNC, "InRect: (%d, %d) => (%d, %d)  (%d x %d)\r\n",
		pTargetRect->left, pTargetRect->top, pTargetRect->right, pTargetRect->bottom, OutWidth, OutHeight);
	
	AdjustAspectRatio(&OutWidth, &OutHeight);

	// If we shrank the size we need to center it.
	if (InWidth > OutWidth)
		XOffset = (InWidth - OutWidth) / 2;

	if (InHeight > OutHeight)
		YOffset = (InHeight - OutHeight) / 2;

	// Set the new rect
	IntermediateRect.left = IntermediateRect.left + XOffset;
	IntermediateRect.top = IntermediateRect.top + YOffset;
	IntermediateRect.right = IntermediateRect.left + OutWidth;
	IntermediateRect.bottom = IntermediateRect.top + OutHeight;

	// Check if the TargetRect > 4x the SourceRect.  If it is, then set the TargetRect to 4x
	// and center it in the original RECT that was passed in.
	m_ITE.GetSourceRect(&SourceRect);
	MaxWidth = RECT_WIDTH(SourceRect) * 4;
	MaxHeight = RECT_HEIGHT(SourceRect) * 4;

	if ((InWidth > MaxWidth) || (InHeight > MaxHeight))
	{
		DWORD	XOffset = (InWidth - MaxWidth)/2;
		DWORD	YOffset = (InHeight - MaxHeight)/2;

		OS_Print(FUNC, "Exceeding 4x interoplation. Adjusting picture size...\r\n");

		NewRect.left = IntermediateRect.left + XOffset;
		NewRect.top = IntermediateRect.top + YOffset;
		NewRect.right = NewRect.left + MaxWidth;
		NewRect.bottom = NewRect.top + MaxHeight;
	}
	else
	{
		NewRect = IntermediateRect;
	}

	CAutoLock lock(&m_csWindow);
	m_ITE.SetTargetRect(&NewRect);
	m_pOverlay->SetOverlayRect(NewRect);

	m_pOverlay->GetOverlayRect(&NewRect);
	if ( (RECT_WIDTH(NewRect) != RECT_WIDTH(OldRect)) ||
		 (RECT_HEIGHT(NewRect) != RECT_HEIGHT(OldRect)) )
	{
		m_bSizeChanged = TRUE;
	}

	OS_Print(FUNC, "OutRect: (%d, %d) => (%d, %d)  (%d x %d)\r\n",
		NewRect.left, NewRect.top, NewRect.right, NewRect.bottom, RECT_WIDTH(NewRect), RECT_HEIGHT(NewRect));

    return NOERROR;
} // SetTargetRect

//
// GetTargetRect
//
// Return the current destination rectangle
//
HRESULT CVideoWindow::GetTargetRect(RECT *pTargetRect)
{
    ASSERT(pTargetRect);

    m_ITE.GetTargetRect(pTargetRect);
    return NOERROR;

} // GetTargetRect


//
// SetDefaultSourceRect
//
// This is called when we reset the default source rectangle
//
HRESULT CVideoWindow::SetDefaultSourceRect()
{
    SIZE VideoSize = m_pRenderer->m_VideoSize;
    RECT SourceRect = {0,0,VideoSize.cx,VideoSize.cy};

    m_ITE.SetSourceRect(&SourceRect);
    return NOERROR;

} // SetDefaultSourceRect


//
// IsDefaultSourceRect
//
// Return S_OK if using the default source otherwise S_FALSE
//
HRESULT CVideoWindow::IsDefaultSourceRect()
{
    RECT SourceRect;

    // Does the source match the native video size

    SIZE VideoSize = m_pRenderer->m_VideoSize;
    CAutoLock cWindowLock(&m_WindowLock);
    m_ITE.GetSourceRect(&SourceRect);

    // Check the coordinates match the video dimensions

    if (SourceRect.right == VideoSize.cx)
    {
        if (SourceRect.bottom == VideoSize.cy)
        {
            if (SourceRect.left == 0)
            {
                if (SourceRect.top == 0)
                {
                    return S_OK;
                }
            }
        }
    }

    return S_FALSE;

} // IsDefaultSourceRect


//
// SetSourceRect
//
// This is called when we want to change the section of the image to draw. We
// use this information in the drawing operation calls later on. We must also
// see if the source and destination rectangles have the same dimensions. If
// not we must stretch during the rendering rather than a direct pixel copy
//
HRESULT CVideoWindow::SetSourceRect(RECT *pSourceRect)
{
    m_ITE.SetSourceRect(pSourceRect);
    return NOERROR;

} // SetSourceRect


//
// GetSourceRect
//
// Return the current source rectangle
//
HRESULT CVideoWindow::GetSourceRect(RECT *pSourceRect)
{
    ASSERT(pSourceRect);

    m_ITE.GetSourceRect(pSourceRect);
    return NOERROR;

} // GetSourceRect


//
// GetStaticImage
//
// Return a copy of the current image in the video renderer
//
HRESULT CVideoWindow::GetStaticImage(long *pBufferSize,long *pDIBImage)
{
//    NOTE("Entering GetStaticImage");

    IMediaSample *pMediaSample;
    pMediaSample = m_pRenderer->GetCurrentSample();
    RECT SourceRect;

    // Is there an image available

    if (pMediaSample == NULL)
        return E_UNEXPECTED;

    // Find a scaled source rectangle for the current bitmap

    m_ITE.GetSourceRect(&SourceRect);
// TODO:
// FIX THIS!
//	SourceRect = m_ITE.ScaleSourceRect(&SourceRect);
    VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) m_pRenderer->m_mtIn.Format();

    // Call the base class helper method to do the work

    HRESULT hr = CopyImage(pMediaSample,        // Buffer containing image
                           pVideoInfo,          // Type representing bitmap
                           pBufferSize,         // Size of buffer for DIB
                           (BYTE*) pDIBImage,   // Data buffer for output
                           &SourceRect);        // Current source position

    pMediaSample->Release();
    return hr;

} // GetStaticImage


//
// GetVideoFormat
//
// Derived classes must override this to return a VIDEOINFOHEADER representing
// the video format. We cannot call IPin ConnectionMediaType to get this
// format because various filters dynamically change the type when using
// DirectDraw such that the format shows the position of the logical
// bitmap in a frame buffer surface, so the size might be returned as
// 1024x768 pixels instead of 320x240 which are the real video dimensions
//
VIDEOINFOHEADER *CVideoWindow::GetVideoFormat()
{
    return (VIDEOINFOHEADER *)m_pRenderer->m_mtIn.Format();

} // GetVideoFormat

ULONG CVideoWindow::GetColorKey(void)
{
	ULONG	nColorKey = m_pOverlay->GetColorKey();
	DWORD	dwColorRed = GET_COLORKEY_RED(nColorKey);
	DWORD	dwColorGrn = GET_COLORKEY_GRN(nColorKey);
	DWORD	dwColorBlu = GET_COLORKEY_BLU(nColorKey);

	m_KeyColor = RGB(dwColorRed, dwColorGrn, dwColorBlu);

	return nColorKey;
}

ULONG CVideoWindow::GetRegistryValues(void)
{
	HKEY hKey = NULL;
	ULONG nStatus, dwSize, dwType;
	DWORD dwColorRed=0, dwColorGreen=0, dwColorBlue=0;
	char szData[8];

	nStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_RENDERER_REGKEY, 0, 0, &hKey);

	if (nStatus != ERROR_SUCCESS)
	{
		OS_Print(ERR, "*****No video renderer registry entry\r\n");
	}	else
	{

		// read back color registry entry
		dwSize = sizeof(szData);
		nStatus = RegQueryValueEx(hKey, TEXT("BackColor"), NULL, &dwType, (LPBYTE)szData, &dwSize); // don't use L"string"
		if (nStatus != ERROR_SUCCESS)
		{
			// if the registry entry does not exist
			RETAILMSG(1, (TEXT("No Background color found in Registry.  Defaulting to RGB(0,0,0)\r\n")));
			dwColorRed = 0x0;
			dwColorGreen = 0x0;
			dwColorBlue = 0x0;
		} else
		{
			dwColorRed =  szData[1];
			dwColorGreen = szData[2];
			dwColorBlue = szData[3];
		}

		m_BackColor = RGB(dwColorRed, dwColorGreen, dwColorBlue);

		// check if we should display OSD stuff
		dwSize = sizeof(szData);
		nStatus = RegQueryValueEx(hKey, TEXT("OnScreenDisplay"), NULL, &dwType, (LPBYTE)szData, &dwSize); // don't use L"string"
		if (nStatus != ERROR_SUCCESS)
		{
			RETAILMSG(1, (TEXT("FAILED to read OnScreenDisplay setting\r\n")));
			m_OSD_enabled = FALSE;
		}
		else
		{
			m_OSD_enabled = szData[0];
		}

		RegCloseKey(hKey);
	}

	return nStatus;
}

///////////////////////////////////////////////////////////////////////////////
//
// Overlay the image time stamps on the picture. Access to this method is
// serialised by the caller. We display the sample start and end times on
// top of the video using TextOut on the device context we are handed. If
// there isn't enough room in the window for the times we don't show them
//
///////////////////////////////////////////////////////////////////////////////
void CVideoWindow::OnScreenDisplay(IMediaSample *pSample)
{
    TCHAR szTimes[TIMELENGTH];      // Time stamp strings
    ASSERT(pSample);                // Quick sanity check
    RECT ClientRect;                // Client window size
    SIZE Size;                      // Size of text output

	OS_Print(FUNC, "CAuITE::DisplaySampleTimes\r\n");

    // Get the time stamps and window size

    pSample->GetTime((REFERENCE_TIME*)&m_StartSample, (REFERENCE_TIME*)&m_EndSample);
    HWND hwnd = m_hwnd;
    EXECUTE_ASSERT(GetClientRect(hwnd,&ClientRect));

    // Format the sample time stamps

    wsprintf(szTimes,TEXT("%08d : %08d"),
             m_StartSample.Millisecs(),
             m_EndSample.Millisecs());

    ASSERT(lstrlen(szTimes) < TIMELENGTH);

    // Put the times in the middle at the bottom of the window

    GetTextExtentPoint32(m_hdc,szTimes,lstrlen(szTimes),&Size);
    INT XPos = ((ClientRect.right - ClientRect.left) - Size.cx) / 2;
    INT YPos = ((ClientRect.bottom - ClientRect.top) - Size.cy) * 4 / 5;

    // Check the window is big enough to have sample times displayed

    if ((XPos > 0) && (YPos > 0)) {
        ExtTextOut(m_hdc,XPos, YPos,
                   ETO_CLIPPED | ETO_OPAQUE, // useless since no rect specified
                   NULL, szTimes, lstrlen(szTimes), NULL);
    }
}

HRESULT CVideoWindow::PrepareReceive(IMediaSample *pMediaSample)
{
	return NOERROR;
}

void CVideoWindow::SetMPERender(BOOL bMPE)
{
	OS_Print(DBG, "CVideoWindow::SetMPERender(%s)\r\n", bMPE ? L"TRUE":L"FALSE");
	m_bMPE = bMPE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CVideoWindow::RenderSample(IMediaSample *pMediaSample)
{
	HRESULT hr;

	CAutoLock lock(&m_csWindow);
    
    if(isNoShowState())
    {
        if(m_pOverlay->IsVisible())
        {
            m_pOverlay->ShowOverlay(FALSE);
            OS_Print(DBG, "VideoRenderer: switch to noShowState\r\n");
        }
        return NOERROR;
    }
    if(!m_pOverlay->IsVisible() && m_visible)
    {
        m_pOverlay->ShowOverlay(TRUE);
        OS_Print(DBG, "VideoRenderer: switch to normalState\r\n");
    }

	bool bAdvance = true;

	if (m_bMPE)
	{
		VideoSample_t *pVideoSample;

		pMediaSample->GetPointer((BYTE **)&pVideoSample);

		if (pVideoSample->flags & VS_FLAGS_EMPTY)
		{
			RETAILMSG(0, (TEXT("Video Decoder sent and empty buffer\r\n")));
			return NOERROR;
		}
		if (pVideoSample->flags & VS_FLAGS_SEGMENT)
		{
			//if (pVideoSample->geometry.yuv.cropping_rect.bottom < (pVideoSample->height-1))
			{
				bAdvance = false;
			}
		}

		m_ITE.ProcessMPEFrame((unsigned int)m_pOverlayBuffers[m_CurOverlayBuffer]->pPhysical, pVideoSample);
		m_LastYUVColorSpace = pVideoSample->colorSpace;
	}
	else
	{
		IAuMediaSample *pAuSample;
		BYTE*	PhysPointer;

		hr = pMediaSample->QueryInterface(IID_AuMediaSample, (void**)(&pAuSample));

		pAuSample->GetPhysPointer(&PhysPointer);

		// this is hardwired?
		ColorSpace_t cs = csYUV420;
		m_ITE.ProcessFrame((unsigned int)m_pOverlayBuffers[m_CurOverlayBuffer]->pPhysical, (unsigned int)PhysPointer, cs);
		m_LastYUVColorSpace = cs;

	}

//	m_pOverlay->SetNextBuffer(m_OverlayBuffers[m_CurOverlayBuffer]);
	m_pOverlay->SetCurrentBuffer((unsigned int)m_pOverlayBuffers[m_CurOverlayBuffer]->pPhysical);

	if (m_OSD_enabled)
	{
		OnScreenDisplay(pMediaSample);
	}

	if (bAdvance)
	{
		IncBufferCounter();
	}

	return NOERROR;
}


void CVideoWindow::SetInputFormat(unsigned int nInputFormat)
{
	// Set ITE intput
	m_ITE.SetInputFormat(nInputFormat);
}

void CVideoWindow::SetOutputFormat()
{
	HKEY	hRendererKey;
	DWORD	dwType;
	DWORD	dwLen;
	DWORD	dwDisplayBpp = 0;

	// Read registry for overlay BPP
	// This is application specific, so check the application's registry
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_RENDERER_REGKEY,0, 0, &hRendererKey))
	{
		return;
	}

	dwType = REG_DWORD;
	dwLen = sizeof(dwDisplayBpp);
	RegQueryValueEx (hRendererKey, TEXT("DisplayBpp"), NULL,
						&dwType, (LPBYTE)&(dwDisplayBpp), &dwLen);

	RegCloseKey( hRendererKey );

	RETAILMSG(1, (TEXT("VideoRender: Setting Bpp %d\r\n"), dwDisplayBpp));


	// Set overlay output
	m_pOverlay->SetOutputFormat(dwDisplayBpp);

	// Set ITE output
	m_ITE.SetOutFormat(dwDisplayBpp);

}

void CVideoWindow::CreateOverlay()
{
	HKEY	hWindowKey, hMAEKey;
	DWORD	dwType;
	DWORD	dwLen;
	DWORD	dwOverlayPlane = MAE_PLANE;

	// Read registry for overlay plane
	// This is BSP specific, so check the display driver's registry
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, DISPLAY_WINDOWS_REGKEY,0, 0, &hWindowKey))
	{
		return;
	}

	RegOpenKeyExW(hWindowKey, TEXT("MAE"), 0, 0, &hMAEKey);

	dwType = REG_DWORD;
	dwLen = sizeof(dwOverlayPlane);
	RegQueryValueEx (hMAEKey, TEXT("Index"), NULL,
						&dwType, (LPBYTE)&(dwOverlayPlane), &dwLen);

	RegCloseKey( hMAEKey );
	RegCloseKey( hWindowKey );

	RETAILMSG(1, (TEXT("VideoRender: Creating MAE plane %d\r\n"), dwOverlayPlane));

	// Create the overlay
	m_pOverlay = new CAuOverlay(dwOverlayPlane);
}

void CVideoWindow::DestroyOverlay()
{
	// Destroy the overlay
	delete m_pOverlay;
}

static const LPWSTR cSystemInfoKey = L"\\LGE\\SystemInfo";
static const LPWSTR cFullScreenName = L"VIDEO_FULLSCREEN";
void CVideoWindow::setFullScreen(BOOL bFullScreen)
{
    Utils::RegistryAccessor::setBool(HKEY_LOCAL_MACHINE, cSystemInfoKey, cFullScreenName, bFullScreen == TRUE ? true : false);
}

BOOL CVideoWindow::getFullScreen()
{
    return Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, cSystemInfoKey, cFullScreenName, false);
}

BOOL CVideoWindow::isNoShowState()
{
    return FindWindow(L"RVC", NULL)!= NULL || Utils::RegistryAccessor::getBool(HKEY_LOCAL_MACHINE, L"\\LGE\\SystemInfo", L"OGL_ENABLE", false);
}

// This allows a client to set the complete window size and position in one
// atomic operation. The same affect can be had by changing each dimension
// in turn through their individual properties although some flashing will
// occur as each of them gets updated (they are better set at design time)

STDMETHODIMP
CVideoWindow::SetWindowPosition(long Left,long Top,long Width,long Height)
{
	HRESULT hr = CBaseControlWindow::SetWindowPosition(Left, Top, Width, Height);

    return hr;
}

