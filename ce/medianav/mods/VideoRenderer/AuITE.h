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
#ifndef __AUITE_H
#define __AUITE_H

#ifdef LOG_TRACE
	#undef LOG_TRACE
#endif

#include "vsample.h"

class CVideoRenderer;

class CAuITE
{
private:
	RECT			m_TargetRect;
	RECT			m_SourceRect;
	unsigned int	m_nRotation;
	BOOL			m_bStretch;
	BOOL			m_bUsingImageAllocator;
	CMediaType		*m_pMediaType;				// Pointer to the current format
	int				m_perfidRenderTime;         // Time taken to render an image
	CVideoRenderer	*m_pRenderer;				// Owning video window object
    HDC				m_hdc;                      // Main window device context
	LONG			m_PaletteVersion;			// Current palette version cookie
	mae_be_request_t m_ITERequest;
	unsigned int	m_nBytesPP;
	unsigned int	m_nPictureMode;
	unsigned int	m_nInputFormat;
	HANDLE			m_hITE;
	DWORD			m_colorSpace;
	DWORD			m_MaxWidth;
	DWORD			m_MaxHeight;
	LONG			m_AccumOutputHeight;

protected:
	void DisplaySampleTimes(IMediaSample *pSample);
	void SetStretchMode();
	void vrmaebe_render (VideoSample_t *inpic, VideoSample_t *outpic);

public:
	CAuITE(CVideoRenderer *pRenderer);
	~CAuITE();
	void SetRotation(unsigned int nRotation);
	unsigned int GetRotation();
	void SetCSC();
	void GetCSC();
	void SetSCF();
	void GetSCF();
	void SetInputFormat(unsigned int nInputFormat);
	void SetOutFormat(unsigned int nOutputFormat);
	void SetupOutPic(VideoSample_t *outpic, unsigned int Dst);
	void ProcessFrame(unsigned int Dst, unsigned int Src, ColorSpace_t cs);
	void ProcessMPEFrame(unsigned int Dst, VideoSample_t *pVideoSample);
	void SetPictureMode(unsigned int nPictureMode);

	void SetSourceRect(RECT *Rect);
	void GetSourceRect(RECT *Rect);
	void SetTargetRect(RECT *Rect);
	void GetTargetRect(RECT *Rect);
};


#endif // __AUITE_H
