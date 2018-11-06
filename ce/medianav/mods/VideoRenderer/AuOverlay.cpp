/***********************************************************************
This product is the confidential property of NetLogic Microsystems Inc. 
(“NetLogic”), is provided under a non-disclosure agreement or license 
agreement, and is protected under applicable copyright, patent, and trade 
secret laws. 

Unauthorized use, reproduction, distribution or other dissemination without 
the prior written authorization from NetLogic is strictly prohibited.  

NetLogic disclaims all warranties of any nature, express or implied, 
including, without limitation, the warranties of fitness for a particular 
purpose, merchantability and/or non-infringement of third party rights. 

NetLogic assumes no liability for any error or omissions in this PRODUCT, 
or for the use of this PRODUCT. In no event shall NetLogic be liable to 
any other party for any special, PUNITIVE, incidental or consequential 
damages, whether based on breach of contract, tort, product liability, 
infringement of intellectual property rights or otherwise. NetLogic reserves 
the right to make changes to, or discontinue, its products At any time. 

Distribution of the product herein does not convey a license or any other right
in any patent, trademark, or other intellectual property of NetLogic.

Use of the product shall serve as acceptance of these terms and conditions.  If
you do not accept these terms, you should return or destroy the product and any 
other accompanying information immediately.

Copyright, 2009-20010, NetLogic Microsystems, Inc. All rights reserved.   
***************************_ NetLogic_3_******************************/
#include <Streams.h>
#include "AuOverlay.h"
#include "os_api.h"

DWORD CAuOverlay::m_dwOverlayRefCount = 0;
int CAuOverlay::m_nRotation = 0;
int CAuOverlay::m_nScreenWidth = 0; // This is relative to screen orientation.
int CAuOverlay::m_nScreenHeight = 0; // This is relative to screen orientation.
int CAuOverlay::m_nBitsPP = 0;

#define FUNC	0 //1
#define ERR		1

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Overlay Class implementation
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void CAuOverlay::RotateRectCW(RECT *pRect, int nRotation)
{
	RECT rclSwap = *pRect;

	OS_Print(FUNC, "+AuOverlay::RotateRectCW: RECT (%d, %d) => (%d, %d) (%d x %d)\r\n", 
		pRect->left, pRect->top, pRect->right, pRect->bottom, RECT_WIDTH(*pRect), 
		RECT_HEIGHT(*pRect));

	switch (nRotation)
	{
	case AU_ROTATE_90:
		pRect->top    = m_nScreenWidth - rclSwap.right;
		pRect->left   = rclSwap.top;
		pRect->bottom = m_nScreenWidth - rclSwap.left;
		pRect->right  = rclSwap.bottom;
		break;

	case AU_ROTATE_180:
		pRect->left   = m_nScreenWidth  - rclSwap.right;
		pRect->right  = m_nScreenWidth - rclSwap.left;
		pRect->top    = m_nScreenHeight - rclSwap.bottom;
		pRect->bottom = m_nScreenHeight - rclSwap.top;
		break;

	case AU_ROTATE_270:
		pRect->left   = m_nScreenHeight - rclSwap.bottom;
		pRect->right  = m_nScreenHeight - rclSwap.top;
		pRect->top    = rclSwap.left;
		pRect->bottom = rclSwap.right;
		break;
	
	default:
		break;
	}

	OS_Print(FUNC, "-AuOverlay::RotateRectCW: RECT (%d, %d) => (%d, %d) (%d x %d)\r\n", 
		pRect->left, pRect->top, pRect->right, pRect->bottom, RECT_WIDTH(*pRect), 
		RECT_HEIGHT(*pRect));
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuOverlay::CAuOverlay(int nOverlayIndex):
	m_nFormat(AU_RGB_565),
	m_nImageWidth(0),
	m_nImagedHeight(0),
	m_bVisible(FALSE)
{
	OS_Print(FUNC, "+AuOverlay::AuOverlay\r\n");

	
	
	m_ovlIoctl.bufctrl = 0;
	m_ovlIoctl.flags = 0;
	m_ovlIoctl.reserved0 = 0;
	m_ovlIoctl.reserved1 = 0;
	m_ovlIoctl.winctrl0 = 0;
	m_ovlIoctl.winctrl1 = 0; // Display driver will fill this pipe and priority
	m_ovlIoctl.winctrl2 = WINCTRL2_CKMODE(3);

	m_ovlIoctl.winctrl1 = WINCTRL1_FORM(m_nFormat);
	m_ovlIoctl.ndx = nOverlayIndex;

	m_ColorKey.colorkey = 0;
	m_ColorKey.colorkeymsk = 0x00FFFFFF;

	m_WindowRect.bottom = m_WindowRect.left = m_WindowRect.right = m_WindowRect.top = 0;

	CheckDisplayChange();

	CAutoLock lock(&m_csOverlay);

	if (0 == m_dwOverlayRefCount)
	{
		if (OS_CreateOverlay(&m_ovlIoctl) < 0)
		{
			// failed
			OS_Print(ERR, "!Failed to create overlay: 0x%x!\r\n", m_ovlIoctl.ndx); 
		}	
	}

	m_dwOverlayRefCount++;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuOverlay::~CAuOverlay()
{
	OS_Print(FUNC, "+AuOverlay::~AuOverlay\r\n");
	CAutoLock lock(&m_csOverlay);
	
	m_dwOverlayRefCount--;
	if (0 == m_dwOverlayRefCount)
	{
		ShowOverlay(FALSE);
		OS_DestroyOverlay(m_ovlIoctl);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::CheckDisplayChange()
{
	m_nRotation = OS_GetDesktopRotation();

	m_nScreenWidth = OS_GetScreenWidth();
	m_nScreenHeight = OS_GetScreenHeight();

	CAutoLock lock(&m_csOverlay);
	SetOverlayRect(m_WindowRect);

	OS_Print(FUNC, "AuOverlay::CheckDisplayChange %d x %d rotation: %d\r\n", 
		m_nScreenWidth, m_nScreenHeight, m_nRotation);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::GetRotation()
{
	return m_nRotation;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetOverlayRect(RECT Overlay)
{
	int HorizontalShift = 0;
	int VerticalShift = 0;
	int width = RECT_WIDTH(Overlay);
	int height = RECT_HEIGHT(Overlay);
	int curwidth = RECT_WIDTH(m_WindowRect);
	int curheight =RECT_HEIGHT(m_WindowRect);

	OS_Print(FUNC, "Incoming Rect: (%d, %d) => (%d, %d) (%d x  %d)\r\n", Overlay.left, 
		Overlay.top, Overlay.right, Overlay.bottom, RECT_WIDTH(Overlay), RECT_HEIGHT(Overlay));

	if ((width != curwidth) || (height != curheight))
	{
		m_bChangedSize = TRUE;
	}

	m_WindowRect = Overlay;

	/////////////////////////////////////////////////////
	//	Check the left
	/////////////////////////////////////////////////////
	if (Overlay.left > m_nScreenWidth)
	{
		m_WindowRect.left = m_nScreenWidth;
		m_WindowRect.right = m_nScreenWidth;
		m_bChangedSize = TRUE;
	}

	if (Overlay.left < 0)
	{
		m_WindowRect.left = 0;
		HorizontalShift = -Overlay.left;
		m_bChangedSize = TRUE;
	}

	/////////////////////////////////////////////////////
	//	Check the top
	/////////////////////////////////////////////////////
	if (Overlay.top > m_nScreenHeight)
	{
		m_WindowRect.top = m_nScreenHeight;
		m_WindowRect.bottom = m_nScreenHeight;
		m_bChangedSize = TRUE;
	}

	if (Overlay.top < 0)
	{
		m_WindowRect.top = 0;
		VerticalShift = -Overlay.top;
		m_bChangedSize = TRUE;
	}

	/////////////////////////////////////////////////////
	//	Check thet right
	/////////////////////////////////////////////////////
	if (Overlay.right > m_nScreenWidth)
	{
		m_WindowRect.right = m_nScreenWidth;
		m_bChangedSize = TRUE;
	}

	if (Overlay.right < 0)
	{
		m_WindowRect.right = 0;
		m_WindowRect.left = 0;
		m_bChangedSize = TRUE;
	}

	if (Overlay.right < m_WindowRect.left)
	{
		m_WindowRect.right = m_WindowRect.left;
		m_bChangedSize = TRUE;
	}

	/////////////////////////////////////////////////////
	//	Check thet bottom
	/////////////////////////////////////////////////////
	if (Overlay.bottom > m_nScreenHeight)
	{
		m_WindowRect.bottom = m_nScreenHeight;
		m_bChangedSize = TRUE;
	}

	if (Overlay.bottom < 0)
	{
		m_WindowRect.bottom = 0;
		m_WindowRect.top = 0;
		m_bChangedSize = TRUE;
	}

	if (Overlay.bottom < m_WindowRect.top)
	{
		m_WindowRect.bottom = m_WindowRect.top;
		m_bChangedSize = TRUE;
	}

	m_OverlayRect = m_WindowRect;

	// Rotate RECT here
	if (m_nRotation)
	{
		RotateRectCW(&m_OverlayRect, m_nRotation);
	}

	if ((RECT_WIDTH(Overlay) < 0) || (RECT_WIDTH(Overlay) > m_nScreenWidth))
	{
		m_nStride = 0;
	}
	else
	{
		m_nStride = RECT_WIDTH(Overlay)* m_nBytesPP;
	}

	m_nImageWidth = RECT_WIDTH(m_OverlayRect) - 1;
	m_nImagedHeight = RECT_HEIGHT(m_OverlayRect) - 1;

	if (m_nImageWidth < 0)
	{
		m_nImageWidth = 0;
	}

	if (m_nImagedHeight < 0)
	{
		m_nImagedHeight = 0;
	}

	m_nStartOffset = (m_nBytesPP * HorizontalShift) + (m_nStride * VerticalShift);


	// Clear and then set the overlay origins -- WINCTRL0_O_MASK
	m_ovlIoctl.winctrl0 &= ~(WINCTRL0_OX(WINCTRL0_O_MASK) | WINCTRL0_OY(WINCTRL0_O_MASK));
	m_ovlIoctl.winctrl0 |= (WINCTRL0_OX(m_OverlayRect.left) | WINCTRL0_OY(m_OverlayRect.top));

	// Clear and then set the overlay size -- WINCTRL1_SZ_MASK
	m_ovlIoctl.winctrl1 &= ~(WINCTRL1_SZX(WINCTRL1_SZ_MASK) | WINCTRL1_SZY(WINCTRL1_SZ_MASK));
	m_ovlIoctl.winctrl1 |= (WINCTRL1_SZX(m_nImageWidth) | WINCTRL1_SZY(m_nImagedHeight));
	
	// Clear and then set the stride (BX) -- WINCTRL2_BX_MASK
	m_ovlIoctl.winctrl2 &= ~(WINCTRL2_BX(WINCTRL2_BX_MASK));
	m_ovlIoctl.winctrl2 |= (WINCTRL2_BX(m_nStride));

	CAutoLock lock(&m_csOverlay);

	// if the size changed turn off the overlay
	if (TRUE == m_bChangedSize)
	{
		if (TRUE == m_bVisible)
		{
			OS_ShowOverlay(m_ovlIoctl.ndx, FALSE);
		}
		else
		{
			m_bChangedSize = FALSE;
		}
	}

	// Configure the overlay 
	OS_SetOverlayConfig(m_ovlIoctl);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::GetOverlayRect(RECT *pOverlay)
{
	*pOverlay = m_OverlayRect;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::GetOverlayBitsPP()
{
	return m_nBitsPP;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetCurrentBuffer(unsigned int BufAddr)
{
	int ret;

	OVERLAY_UPDATE_IOCTL 	ovlIoctl;

	m_CurrentBuffer = BufAddr;
	
	ovlIoctl.phys = (void *)(BufAddr + m_nStartOffset);
	ovlIoctl.ndx = m_ovlIoctl.ndx;
	ovlIoctl.flags = LCD_UPDATE_IN_VBLANK;

	CAutoLock lock(&m_csOverlay);

	ret = OS_UpdateOverlay(ovlIoctl);

	// if the size changed and the overlay should be visible, turn on overlay
	if ((TRUE == m_bChangedSize) && m_bVisible)
	{
		OS_ShowOverlay(m_ovlIoctl.ndx, TRUE);
		m_bChangedSize = FALSE;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetNextBuffer(unsigned int BufAddr)
{
	int ret;

	OVERLAY_UPDATE_IOCTL 	ovlIoctl;

	m_CurrentBuffer = BufAddr;
	
	ovlIoctl.phys = (void *)(BufAddr + m_nStartOffset);
	ovlIoctl.ndx = m_ovlIoctl.ndx;
	ovlIoctl.flags = LCD_UPDATE_IN_VBLANK;

	CAutoLock lock(&m_csOverlay);

	ret = OS_SetNextOverlayBuffer(ovlIoctl);

	// if the size changed and the overlay should be visible, turn on overlay
	if ((TRUE == m_bChangedSize) && m_bVisible)
	{
		OS_ShowOverlay(m_ovlIoctl.ndx, TRUE);
		m_bChangedSize = FALSE;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::GetOverlayBytesPP()
{
	return m_nBytesPP;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::ShowOverlay(BOOL bShow)
{
	CAutoLock lock(&m_csOverlay);
	OS_ShowOverlay(m_ovlIoctl.ndx, bShow);

	m_bVisible = bShow;
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
BOOL CAuOverlay::IsVisible()
{
	return m_bVisible;
}


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetColorKey(unsigned int nColorKey)
{
	OS_Print(FUNC, "CAuOverlay::SetColorKey 0x%x\r\n", nColorKey);
	m_ColorKey.colorkey = nColorKey;

	OS_SetScreenColorkey(m_ColorKey);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetBackColor(unsigned int nBackColor)
{
	OS_Print(FUNC, "CAuOverlay::SetColorKey 0x%x\r\n", nBackColor);
	m_nBackColor = nBackColor;

	OS_SetScreenBackground(m_nBackColor);

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
unsigned int CAuOverlay::GetColorKey()
{
	OS_GetScreenColorkey(&m_ColorKey);

	return m_ColorKey.colorkey;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
unsigned int CAuOverlay::GetBackColor()
{
	return m_nBackColor;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::SetAlphaValue(int nAlpha)
{
	m_nAlphaValue = nAlpha;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int CAuOverlay::GetAlphaValue()
{
	return m_nAlphaValue;
}

void CAuOverlay::SetOutputFormat(unsigned int nOutputFormat)
{
	switch(nOutputFormat)
	{
	case AU_RGB_8888:
		m_nFormat = AU_RGB_8888;
		m_nBytesPP = 4;
		break;

	case AU_RGB_888:
		m_nFormat = AU_RGB_888;
		m_nBytesPP = 4;
		break;

	case AU_RGB_565:
	default:
		m_nFormat = AU_RGB_565;
		m_nBytesPP = 2;
		break;
	}

	m_ovlIoctl.winctrl1 &= ~(WINCTRL1_FORM(WINCTRL1_FORM_MASK));
	m_ovlIoctl.winctrl1 |= WINCTRL1_FORM(m_nFormat);
}