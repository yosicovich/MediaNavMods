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
#ifndef __AUOVERLAY_H
#define __AUOVERLAY_H

#include <windows.h>
#include "lcd_ioctl.h"

///////////////////////////
// abstract
//////////////////////////

class CAuOverlay
{
private:
	static int m_nRotation;
	static int m_nScreenWidth; // This is relative to screen orientation.
	static int m_nScreenHeight; // This is relative to screen orientation.
	static int m_nBitsPP;
	RECT  m_WindowRect; // This is relative to screen orientation.
	RECT  m_OverlayRect; 
	BOOL m_bVisible;
	BOOL m_bChangedSize;
	OVERLAY_IOCTL m_OvIoctl;
	LCD_COLORKEY_IOCTL m_ColorKey;
	ULONG m_nBackColor;
	int m_nAlphaValue;
	int m_nStride;
	int m_nImageWidth;
	int m_nImagedHeight;
	int m_nStartOffset;
	int m_nBytesPP;
	unsigned int m_nFormat;
	OVERLAY_IOCTL m_ovlIoctl;
	unsigned int m_CurrentBuffer;

	// protects access to overlay
	CCritSec m_csOverlay;
	static DWORD m_dwOverlayRefCount;

public:
	CAuOverlay(int nOverlayIndex);
	~CAuOverlay();
	int SetColorKey(unsigned int nColorKey);
	int SetBackColor (unsigned int nBackColor);
	unsigned int GetColorKey();
	unsigned int GetBackColor();
	int SetAlphaValue(int nAlpha);
	int GetAlphaValue();
	int CheckDisplayChange();
	int GetRotation();
	int SetOverlayRect(RECT Overlay);
	int GetOverlayRect(RECT *pOverlay);
	int GetOverlayBitsPP();
	int GetOverlayBytesPP();
	int ShowOverlay(BOOL bShow);
	BOOL IsVisible();
	void RotateRectCW(RECT *pRect, int nRotation);
	int SetCurrentBuffer(unsigned int);
	unsigned int GetCurrentBuffer();
	int SetNextBuffer(unsigned int);
	unsigned int GetNextBuffer();
	void SetOutputFormat(unsigned int nOutputFormat);
	DWORD GetScreenHeight() { return m_nScreenHeight; } ;
	DWORD GetScreenWidth() { return m_nScreenWidth; };
};


#endif // __AUOVERLAY_H