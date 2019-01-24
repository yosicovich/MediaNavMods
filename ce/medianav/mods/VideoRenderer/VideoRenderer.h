//------------------------------------------------------------------------------
// File: SampVid.h
//
// Desc: DirectShow sample code - header file for simple video renderer
//       application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#include "os_api.h"
#include "AuITE.h"
#include "AuAllocator.h"
#include "AuOverlay.h"
#include "dshow\SecureConnection.h"
#include <memory>
#include "SystemMeter.h"

#ifndef __VIDEORENDERER_H
#define __VIDEORENDERER_H

//#define TESTMODE
//#define NO_HIDE
// Forward declarations

class CVideoRenderer;
class CVideoInputPin;
class CControlVideo;
class CVideoWindow;

#include "AuAllocator.h"

#define NUM_OVERLAY_BUFFERS 3

// This is the video renderer window it supports IBasicVideo and IVideoWindow
// by inheriting from the CBaseControlWindow and CbaseControlVideo classes.
// Those classes leave a few PURE virtual methods that we have to override to
// complete their implementation such as the handling of source and target
// rectangles. The class also looks after creating the window with a custom
// clip region in the shape of the word ActiveX (only applies to Windows/NT)

#define VIDEO_RENDERER_REGKEY (TEXT("Software\\Microsoft\\DirectX\\DirectShow\\Video Renderer"))

enum OSDInfo
{
    OSDInfo_First = 0,
    OSDInfo_VideoTimestamps = OSDInfo_First,
    OSDInfo_ChipInfo = 1,
    OSDInfo_MemUsage = 2,
    OSDInfo_Last = OSDInfo_MemUsage

};

class CVideoWindow : public CBaseControlWindow, public CBaseControlVideo
{
protected:

    CVideoRenderer	*m_pRenderer;        // Owning sample renderer object
	COLORREF        m_KeyColor;          // Overlay Color Key
	COLORREF        m_BackColor;         // Window Background Color
	CAuITE			m_ITE;
	CRefTime		m_StartSample;				// Start time for the current sample
    CRefTime		m_EndSample;				// And likewise it's end sample time
	BOOL			m_bMPE;

	HANDLE			m_hMem;
	DWORD			m_CurOverlayBuffer;
	DWORD			m_nOverlayBufferSize;
	PMEM_IOCTL		m_pOverlayBuffers[NUM_OVERLAY_BUFFERS];
	BOOL			m_OSD_enabled;
	CCritSec		m_csWindow;
	static CCritSec	m_csMempool;
	BOOL			m_bSizeChanged;
	ColorSpace_t	m_LastYUVColorSpace;

    RECT            m_rcTarget;
    int             m_aspect_ratio_width;
    int             m_aspect_ratio_height;
	int				m_aspect_ratio_type;
    int             m_cxCrop;
    int             m_cyCrop;

    BOOL            m_visible;
    
    OSDInfo         m_osdInfo;
    bool            m_longTapDetect;
    DWORD           m_tapStartTime;
    std::auto_ptr<Utils::SystemMeter> m_pSystemMeter;


	void CreateOverlay();
	void DestroyOverlay();

    void setFullScreen(BOOL bFullScreen);
    BOOL isNoShowState();
    void toggleOnScreenInfo();
    void switchOSDInfo();
    void stopSystemMeter();

public:
	CAuOverlay		*m_pOverlay;

    CVideoWindow(TCHAR *pName,                 // Object description
               LPUNKNOWN pUnk,               // Normal COM ownership
               HRESULT *phr,                 // OLE failure code
               CCritSec *pInterfaceLock,     // Main critical section
               CVideoRenderer *pRenderer);   // Delegates locking to

    virtual ~CVideoWindow();
	HRESULT DoShowWindow(LONG ShowCmd);
	LRESULT OnEraseBackground(HDC hdc, LPARAM lParam, HWND hwnd);
	LRESULT OnPaint(HWND hwnd);
	ULONG GetRegistryValues(void);
	ULONG GetColorKey(void);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
	STDMETHODIMP SetWindowPosition(long Left,long Top,long Width,long Height);

    // Pure virtual methods for the IBasicVideo interface

    HRESULT IsDefaultTargetRect();
    HRESULT SetDefaultTargetRect();
    HRESULT SetTargetRect(RECT *pTargetRect);
    HRESULT GetTargetRect(RECT *pTargetRect);
    HRESULT IsDefaultSourceRect();
    HRESULT SetDefaultSourceRect();
    HRESULT SetSourceRect(RECT *pSourceRect);
    HRESULT GetSourceRect(RECT *pSourceRect);
    HRESULT GetStaticImage(long *pBufferSize,long *pDIBImage);
	int		GetBytesPP() {return m_pOverlay->GetOverlayBytesPP();};
	void OnScreenDisplay(IMediaSample *pSample);
	HRESULT PrepareReceive(IMediaSample *pMediaSample);
	HRESULT RenderSample(IMediaSample *pMediaSample);
	void SetMPERender(BOOL bMPE);
	BOOL IsMPERender()		{ return m_bMPE; }
	void SetInputFormat(unsigned int nInputFormat);
	void SetOutputFormat();
	void AllocateBuffers();
	void FreeBuffers();
	void Active(BOOL bActive);
	void ResetBuffers();
	void IncBufferCounter();
	void RepaintLastImage();

	int AdjustAspectRatio(int *max_output_width, int *max_output_height);

    // Prepare the window with a text region

    void InitRenderer(TCHAR *pStringName);
    HRESULT InitWindow();
    HFONT CreateVideoFont();
    RECT GetDefaultRect();
    VIDEOINFOHEADER *GetVideoFormat();

    // Overriden from CBaseWindow return our window and class styles

    LPTSTR GetClassWindowStyles(DWORD *pClassStyles,
                                DWORD *pWindowStyles,
                                DWORD *pWindowStylesEx);

    // Method that gets all the window messages

    LRESULT OnReceiveMessage(HWND hwnd,          // Window handle
                             UINT uMsg,          // Message ID
                             WPARAM wParam,      // First parameter
                             LPARAM lParam);     // Other parameter

    void SetAspectRatio(int cx, int cy, int aspect_type, int cxCrop, int cyCrop);
	void AdjustWindowSize(BOOL maximized);
    BOOL getFullScreen();
    void checkSetWindowVisibility();
    void refreshWindowState();
}; // CVideoWindow


// This class supports the renderer input pin. We have to override the base
// class input pin because we provide our own special allocator which hands
// out buffers based on GDI DIBSECTIONs. We have an extra limitation which
// is that we only connect to filters that agree to use our allocator. This
// stops us from connecting to the tee for example. The extra work required
// to use someone elses allocator and select the buffer into a bitmap and
// that into the HDC is not great but would only really confuse this sample

class CVideoInputPin : public CRendererInputPin
, CSecureConnection
{
    CVideoRenderer	*m_pRenderer;        // The renderer that owns us
    CCritSec		*m_pInterfaceLock;   // Main filter critical section

public:

    // Constructor

    CVideoInputPin(
        TCHAR *pObjectName,             // Object string description
        CVideoRenderer *pRenderer,      // Used to delegate locking
        CCritSec *pInterfaceLock,       // Main critical section
        HRESULT *phr,                   // OLE failure return code
        LPCWSTR pPinName);              // This pins identification

    // Manage our DIBSECTION video allocator
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly);
	STDMETHODIMP Receive(IMediaSample *pSample);
	HRESULT CompleteConnect(IPin *pReceivePin);

}; // CVideoInputPin


// This is the COM object that represents a simple rendering filter. It
// supports IBaseFilter and IMediaFilter and a single input stream (pin)
// The classes that support these interfaces have nested scope NOTE the
// nested class objects are passed a pointer to their owning renderer
// when they are created but they should not use it during construction

class CVideoRenderer : public CBaseVideoRendererAsync
{
private:
	BOOL m_bActive;
public:

    // Constructor and destructor

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);
    CVideoRenderer(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *phr);
    ~CVideoRenderer();

	void ReadRegistry();

    // Implement the ISpecifyPropertyPages interface

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

    // Override these from the filter and renderer classes

    HRESULT Active();
	HRESULT Inactive();
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmtIn);
    HRESULT CheckVideoType(const VIDEOINFO *pDisplay,const VIDEOINFO *pInput);
    HRESULT UpdateFormat(VIDEOINFO *pVideoInfo);
    HRESULT DoRenderSample(IMediaSample *pMediaSample);
    void OnReceiveFirstSample(IMediaSample *pMediaSample);
	HRESULT Receive(IMediaSample* pMediaSample);
	HRESULT PrepareReceive(IMediaSample *pMediaSample);
	STDMETHODIMP Stop();
    STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	HRESULT BeginFlush();
	LONG RendererQueueCount();	
	HRESULT EndOfStream();

public:

    CVideoInputPin  m_InputPin;        // IPin based interfaces
    CMediaType      m_mtIn;            // Source connection media type
	SIZE            m_VideoSize;       // Size of the current video stream
	CAuAllocator	m_ImageAllocator;  // Our DIBSECTION allocator -- AUALLOCATOR
    CVideoWindow	m_VideoWindow;     // Does the actual video rendering -- OVERLAY
	BOOL			m_bCacheEveryFrame; // If this is set we cache every rendered frame until the next rendererd frame
	BOOL			m_bEnableCropAndAspectRatio;
}; // CVideoRenderer

#endif // __VIDEORENDERER_H

