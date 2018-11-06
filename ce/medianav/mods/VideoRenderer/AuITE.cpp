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
#include <windows.h>
#include <streams.h>
#include "os_api.h"
#include "AuITE.h"
#include "VideoRenderer.h"
#include "ite/ite.h"
#include "vsample.h"



#define FUNC	0 //1
#define ERR		1

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuITE::CAuITE(CVideoRenderer *pRenderer)
:
m_hdc(NULL),
m_bStretch(FALSE),
m_bUsingImageAllocator(FALSE),
m_nBytesPP(0),
m_nPictureMode(ITE_MODE_PROGRESSIVE),
m_pRenderer(pRenderer),
m_AccumOutputHeight(0),
m_nRotation(0)
{
	ASSERT(m_pBaseWindow);
    SetRectEmpty(&m_TargetRect);
    SetRectEmpty(&m_SourceRect);

	OS_Print(FUNC, "CAuITE::CAuITE\r\n");

	m_hITE = maeite_open_driver();

	// Setup default ITE values
	memset(&m_ITERequest, 0x0, sizeof(m_ITERequest));
	maeite_set_default_csc(&m_ITERequest);
	maeite_set_default_scf(&m_ITERequest, m_nPictureMode);

    m_perfidRenderTime = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
CAuITE::~CAuITE()
{
	OS_Print(FUNC, "CAuITE::~CAuITE\r\n");
	maeite_close_driver(m_hITE);
}

///////////////////////////////////////////////////////////////////////////////
//
// This is called when either the source or destination rectanges change so we
// can update the stretch flag. If the rectangles don't match we scale the
// video in the MAE ITE.
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetStretchMode()
{
    // Calculate the overall rectangle dimensions
    LONG SourceWidth = m_SourceRect.right - m_SourceRect.left;
    LONG SinkWidth = m_TargetRect.right - m_TargetRect.left;
    LONG SourceHeight = m_SourceRect.bottom - m_SourceRect.top;
    LONG SinkHeight = m_TargetRect.bottom - m_TargetRect.top;
	ULONG	hsr = MAE_SCALE_RATIO_1_TO_1;
	ULONG	vsr = MAE_SCALE_RATIO_1_TO_1;

	OS_Print(FUNC, "CAuITE::CAuITE::SetStretchMode\r\n");

	OS_Print(FUNC, "Source (%d x %d)\r\n", SourceWidth, SourceHeight);
	OS_Print(FUNC, "Target (%d x %d)\r\n", SinkWidth, SinkHeight);

	// Set to defaults
    m_bStretch = FALSE;
	m_ITERequest.scfdisable = SCF_DISABLE;

    if (SourceWidth != SinkWidth) {
		// calculate HSR
		hsr = maeite_calc_sr(SourceWidth, SinkWidth);
		m_bStretch = TRUE;
    }

	if (SourceHeight != SinkHeight) {
		// calculate VSR
		vsr = maeite_calc_sr(SourceHeight, SinkHeight);
        m_bStretch = TRUE;
    }

	m_ITERequest.scfhsr = hsr;
	m_ITERequest.scfvsr = vsr;

	if (m_bStretch)
	{
		m_ITERequest.scfdisable = SCF_ENABLE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// This is called to set the target rectangle in the video window, it will be
// called whenever a WM_SIZE message is retrieved from the message queue. We
// simply store the rectangle and use it later when we do the drawing calls
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetTargetRect(RECT *pTargetRect)
{
	DWORD width = RECT_WIDTH(*pTargetRect);
	DWORD height = RECT_HEIGHT(*pTargetRect);
	OS_Print(FUNC, "CAuITE::SetTargetRect\r\n");
	OS_Print(FUNC, "Rect: (%d, %d)  => (%d, %d)\r\n",
		pTargetRect->left, pTargetRect->top, pTargetRect->right, pTargetRect->bottom);

    ASSERT(pTargetRect);

	if ( (0 == width) ||
		 (0 == height) ||
		 (width > m_MaxWidth) ||
		 (height > m_MaxHeight) )
	{
		return;
	}

	m_TargetRect = *pTargetRect;
	SetStretchMode();

	switch(m_nRotation)
	{
	case AU_ROTATE_0:
	case AU_ROTATE_180:
		m_ITERequest.dstheight = (m_TargetRect.bottom - m_TargetRect.top);
		m_ITERequest.dststr = (m_TargetRect.right - m_TargetRect.left) * m_nBytesPP;
		break;

	case AU_ROTATE_90:
	case AU_ROTATE_270:
		m_ITERequest.dstheight = (m_TargetRect.right - m_TargetRect.left);
		m_ITERequest.dststr = (m_TargetRect.bottom - m_TargetRect.top) * m_nBytesPP;
		break;
	}

	m_ITERequest.dstheight = (m_TargetRect.bottom - m_TargetRect.top);
	m_ITERequest.dststr = (m_TargetRect.right - m_TargetRect.left) * m_nBytesPP;
}

///////////////////////////////////////////////////////////////////////////////
//
// Return the current target rectangle
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::GetTargetRect(RECT *pTargetRect)
{
	OS_Print(FUNC, "CAuITE::GetTargetRect\r\n");
    ASSERT(pTargetRect);
    *pTargetRect = m_TargetRect;
}

///////////////////////////////////////////////////////////////////////////////
//
// This is called when we want to change the section of the image to draw. We
// use this information in the drawing operation calls later on. We must also
// see if the source and destination rectangles have the same dimensions. If
// not we must stretch during the drawing rather than a direct pixel copy
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetSourceRect(RECT *pSourceRect)
{
	OS_Print(FUNC, "CAuITE::SetSourceRect\r\n");

    ASSERT(pSourceRect);
    m_SourceRect = *pSourceRect;

	m_MaxWidth = (4 * RECT_WIDTH(m_SourceRect));
	m_MaxHeight = (4 * RECT_HEIGHT(m_SourceRect));

    SetStretchMode();

	m_ITERequest.srcfhw = MAEBE_SRCFHW_WIDTH_N(m_SourceRect.right - m_SourceRect.left) |
		MAEBE_SRCFHW_HEIGHT_N(m_SourceRect.bottom - m_SourceRect.top);
}

///////////////////////////////////////////////////////////////////////////////
//
// Return the current source rectangle
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::GetSourceRect(RECT *pSourceRect)
{
	OS_Print(FUNC, "CAuITE::GetSourceRect\r\n");

    ASSERT(pSourceRect);
    *pSourceRect = m_SourceRect;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetRotation(unsigned int nRotation)
{
	OS_Print(FUNC, "CAuITE::SetRotation\r\n");
	switch(nRotation)
	{
	case AU_ROTATE_0:
		m_nRotation = 0;
		break;

	case AU_ROTATE_90:
		m_nRotation = 3;
		break;

	case AU_ROTATE_180:
		m_nRotation = 2;
		break;

	case AU_ROTATE_270:
		m_nRotation = 1;
		break;
	}



	// Clear the rotation and then set the new one
	m_ITERequest.dstcfg &= ~(MAEBE_DSTCFG_ROT_N(DSTCFG_ROT_MASK));
	m_ITERequest.dstcfg |= MAEBE_DSTCFG_ROT_N(m_nRotation);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
unsigned int CAuITE::GetRotation()
{
	OS_Print(FUNC, "CAuITE::GetRotation\r\n");
	return m_nRotation;
}

///////////////////////////////////////////////////////////////////////////////
//
// Sets input format
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetInputFormat(unsigned int nInputFormat)
{
	OS_Print(FUNC, "CAuITE::SetInputFormat\r\n");

	m_nInputFormat = nInputFormat;
}

///////////////////////////////////////////////////////////////////////////////
//
// Sets output format
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::SetOutFormat(unsigned int nOutputFormat)
{
	OS_Print(FUNC, "CAuITE::SetOutFormat\r\n");

	// clear first
	m_ITERequest.dstcfg &= ~(MAEBE_DSTCFG_BGR_EN | MAEBE_DSTCFG_OF_MASK);

	switch (nOutputFormat)
	{
	case AU_RGB_8888:
//		RETAILMSG(1, (TEXT("VideoRenderer: Output -- AU_RGB_8888\r\n")));
		m_ITERequest.dstcfg |= MAEBE_DSTCFG_OF_N(DSTCFG_OF_24BPP);
		m_colorSpace = csRGB888;
		m_nBytesPP = 4;
		break;

	case AU_RGB_888:
//		RETAILMSG(1, (TEXT("VideoRenderer: Output -- AU_RGB_888\r\n")));
		m_ITERequest.dstcfg |= MAEBE_DSTCFG_OF_N(DSTCFG_OF_24BPP);
		m_colorSpace = csRGB888;
		m_nBytesPP = 4;
		break;

	case AU_RGB_565:
	default:
//		RETAILMSG(1, (TEXT("VideoRenderer: Output -- AU_RGB_565\r\n")));
		m_ITERequest.dstcfg |= MAEBE_DSTCFG_OF_N(DSTCFG_OF_16BPP);
		m_colorSpace = csRGB565;
		m_nBytesPP = 2;
		break;


		break;
	}
}

void PrintRequest(mae_be_request_t request)
{
    OS_Print(1, "scfhsr = 0x%x\r\n", request.scfhsr);
    OS_Print(1, "scfvsr = 0x%x\r\n", request.scfvsr);
    OS_Print(1, "scfdisable = 0x%x\r\n", request.scfdisable);
/*
    OS_Print(1, "scfhalut[32];
    OS_Print(1, "scfvalut[32];
    OS_Print(1, "scfhblut[32];
    OS_Print(1, "scfvblut[32];
    OS_Print(1, "scfhclut[32];
    OS_Print(1, "scfvclut[32];
*/
    OS_Print(1, "cscxcffa = 0x%x\r\n", request.cscxcffa);
    OS_Print(1, "cscxcffb = 0x%x\r\n", request.cscxcffb);
    OS_Print(1, "cscxcffc = 0x%x\r\n", request.cscxcffc);
    OS_Print(1, "cscycffa = 0x%x\r\n", request.cscycffa);
    OS_Print(1, "cscycffb = 0x%x\r\n", request.cscycffb);
    OS_Print(1, "cscycffc = 0x%x\r\n", request.cscycffb);
    OS_Print(1, "csczcffa = 0x%x\r\n", request.csczcffa);
    OS_Print(1, "csczcffb = 0x%x\r\n", request.csczcffb);
    OS_Print(1, "csczcffc = 0x%x\r\n", request.csczcffc);
    OS_Print(1, "cscxoff = 0x%x\r\n", request.cscxoff);
    OS_Print(1, "cscyoff = 0x%x\r\n", request.csczoff);
    OS_Print(1, "csczoff = 0x%x\r\n", request.csczoff);
    OS_Print(1, "cscalpha = 0x%x\r\n", request.cscalpha);

    OS_Print(1, "srccfg = 0x%x\r\n", request.srccfg);
    OS_Print(1, "srcfhw = 0x%x\r\n", request.srcfhw);
    OS_Print(1, "srcaaddr = 0x%x\r\n", request.srcaaddr);
    OS_Print(1, "srcastr = 0x%x\r\n", request.srcastr);
    OS_Print(1, "srcbaddr = 0x%x\r\n", request.srcbaddr);
    OS_Print(1, "srcbstr = 0x%x\r\n", request.srcbstr);
    OS_Print(1, "srccaddr = 0x%x\r\n", request.srccaddr);
    OS_Print(1, "srccstr = 0x%x\r\n", request.srccstr);

    OS_Print(1, "dstcfg = 0x%x\r\n", request.dstcfg);
    OS_Print(1, "dstheight = 0x%x\r\n", request.dstheight);
    OS_Print(1, "dstaddr = 0x%x\r\n", request.dstaddr);
    OS_Print(1, "dststr = 0x%x\r\n", request.dststr);

    OS_Print(1, "ctlenable = 0x%x\r\n", request.ctlenable);
    OS_Print(1, "ctlfpc = 0x%x\r\n", request.ctlfpc);
    OS_Print(1, "ctlstat = 0x%x\r\n", request.ctlstat);
    OS_Print(1, "ctlintenable = 0x%x\r\n", request.ctlintenable);
    OS_Print(1, "ctlintstat = 0x%x\r\n", request.ctlintstat);

    // returned by driver
    OS_Print(1, "begMipsCounter = 0x%x\r\n", request.begMipsCounter);
    OS_Print(1, "endMipsCounter = 0x%x\r\n", request.endMipsCounter);


}

void CAuITE::SetupOutPic(VideoSample_t *outpic, unsigned int Dst)
{
	//////////////////////////////////////////
	// Setup out pic
	//////////////////////////////////////////
//	outpic->colorSpace				= m_colorSpace;
	outpic->width					= m_TargetRect.right - m_TargetRect.left;
	outpic->height					= m_TargetRect.bottom - m_TargetRect.top;
	outpic->memory.rgb.phys				= Dst;


	switch(m_nRotation)
	{
	case AU_ROTATE_180:
		// Pointer to bottom right corner of buffer.
		OS_Print(FUNC, "AuITE: Pointer to bottom right corner of buffer (AU_ROTATE_180).\r\n");
		outpic->memory.rgb.phys = Dst + (outpic->width * m_nBytesPP * outpic->height) - (((20 * outpic->width * m_nBytesPP) / RECT_WIDTH(m_SourceRect)) & (~(m_nBytesPP - 1))) ;
		break;

	case AU_ROTATE_90:
		// Pointer to top right corner of buffer. (nDstWidth - 8) * nBytesPerPixel;  //TODO: why -8 ?
		OS_Print(FUNC, "AuITE: Pointer to top right corner of buffer (AU_ROTATE_90).\r\n");
		outpic->memory.rgb.phys = Dst + (outpic->height - 8) * m_nBytesPP;
		break;

	case AU_ROTATE_270:
		// Pointer to lower left corner of buffer.
		OS_Print(FUNC, "AuITE: Pointer to lower left corner of buffer (AU_ROTATE_270).\r\n");
		outpic->memory.rgb.phys = Dst + (outpic->width * m_nBytesPP) * (outpic->width - 1);
		break;

	case AU_ROTATE_0:
	default:
		// Pointer to top left corner of buffer
		OS_Print(FUNC, "AuITE: Pointer to top left corner of buffer (AU_ROTATE_0).\r\n");
		outpic->memory.rgb.phys = Dst;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Does the CSC and SCF conversions.
//
///////////////////////////////////////////////////////////////////////////////
void CAuITE::ProcessFrame(unsigned int Dst, unsigned int Src, ColorSpace_t cs)
{
	VideoSample_t inpic;
	VideoSample_t outpic;
	unsigned int width = m_SourceRect.right - m_SourceRect.left;
	unsigned int height = m_SourceRect.bottom - m_SourceRect.top;

	m_ITERequest.magic = MAEBE_MAGIC;

	OS_Print(FUNC, "CAuITE::ProcessFrame\r\n");

	maeite_setup_src_regs(&m_ITERequest, m_nInputFormat, Src, width, height);

	m_ITERequest.dstaddr = Dst;

//	PrintRequest(m_ITERequest);

	//////////////////////////////////////////
	// Setup out pic
	//////////////////////////////////////////
	SetupOutPic(&outpic, Dst);

	/////////////////////////////////////////
	// Setup in pic
	/////////////////////////////////////////
	memset(&inpic, 0, sizeof(inpic));
	inpic.width = width;
    inpic.height = height;
    inpic.memory.yuv.y_phys = m_ITERequest.srcaaddr;
    inpic.memory.yuv.u_phys = m_ITERequest.srcbaddr;
    inpic.memory.yuv.v_phys = m_ITERequest.srccaddr;
    inpic.memory.yuv.y_mem_width = m_ITERequest.srcastr;
    inpic.memory.yuv.u_mem_width = m_ITERequest.srcbstr;
    inpic.memory.yuv.v_mem_width = m_ITERequest.srccstr;
	inpic.geometry.yuv.yuv_format = VS_YUV_FORMAT_Y_U_V;
	inpic.flags	= VS_FLAGS_EL;
	inpic.colorSpace = cs;

	// Process the frame now
	vrmaebe_render (&inpic, &outpic);


//	maeite_submit_frame(m_hITE, &m_ITERequest, 0, 0);

}



void CAuITE::ProcessMPEFrame(unsigned int Dst, VideoSample_t *pVideoSample)
{
	VideoSample_t outpic;
	unsigned int width = m_SourceRect.right - m_SourceRect.left;
	unsigned int height = m_SourceRect.bottom - m_SourceRect.top;

	//////////////////////////////////////////
	// Setup out pic
	//////////////////////////////////////////
	SetupOutPic(&outpic, Dst);

	maeite_setup_src_regs(&m_ITERequest, m_nInputFormat, pVideoSample->memory.yuv.y_phys, width, height);

	vrmaebe_render (pVideoSample, &outpic);
}


void CAuITE::SetPictureMode(unsigned int nPictureMode)
{
	m_nPictureMode = nPictureMode;
	maeite_set_default_scf(&m_ITERequest, m_nPictureMode);
}

inline int INCLUSIVE_RECT_WIDTH(CroppingRect_t & r)
{
	return (r.right == r.left) ? 0 : (r.right - r.left + 1);
}
inline int INCLUSIVE_RECT_HEIGHT(CroppingRect_t & r)
{
	return (r.bottom == r.top) ? 0 : (r.bottom - r.top + 1);
}

void
CAuITE::vrmaebe_render (VideoSample_t *inpic, VideoSample_t *outpic)
{
	if (!(inpic->flags & VS_FLAGS_SEGMENT) || (inpic->geometry.yuv.cropping_rect.top == 0))
	{
		m_AccumOutputHeight = 0;
	}
    unsigned int in_width = inpic->width;
    unsigned int in_height = inpic->height;
    unsigned int yuv_offset = 0;
	BOOL bSizeChanged = FALSE;
	BOOL bSegmented = inpic->flags & VS_FLAGS_SEGMENT;
    mae_be_request_t *req = &m_ITERequest;

	// handle cropping and new video dimensions
	int cxCrop = INCLUSIVE_RECT_WIDTH(inpic->geometry.yuv.cropping_rect);
	if ((cxCrop > 0) && (cxCrop < (int)in_width))
	{
		// use cropping rect only if smaller than requested size
		in_width = cxCrop;
	}
	// ensure crop to multiple of two
	in_width &= ~1;
	if (in_width != RECT_WIDTH(m_SourceRect))
	{
		m_SourceRect.right = m_SourceRect.left + in_width;
		bSizeChanged = TRUE;
	}

	int cyCrop = INCLUSIVE_RECT_HEIGHT(inpic->geometry.yuv.cropping_rect);
	if ((cyCrop > 0) && (cyCrop < (int)in_height))
	{
		in_height = cyCrop;
	}
	// ensure crop to multiple of two
	in_height &= ~1;
	if (in_height != RECT_HEIGHT(m_SourceRect))
	{
		if (!bSegmented)
		{
			// for jpeg segment retain scaling based on full height
			m_SourceRect.bottom = m_SourceRect.top + in_height;
		}
		bSizeChanged = TRUE;
	}

	if (bSizeChanged)
	{
		req->srcfhw = (in_height << 16)|(in_width << 0);
		SetStretchMode();
		if (!bSegmented)
		{
			m_pRenderer->NotifyEvent(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(in_width,in_height), MAKEWPARAM(0,0));
		}
		else
		{
			// offset target by scaled start point
			uint32 vsr = m_ITERequest.scfvsr;
			float scale_factor = (vsr >> 16) + float(vsr & 0xffff) / (1 << 16);

			// get the output height using the current VSF
			uint32 ySegment = uint32(in_height / scale_factor);
			req->dstheight = ySegment;

			// then calculate the VSF based on this output height (to allow for fractional output sizes)
			vsr = maeite_calc_sr(in_height, ySegment);
			m_ITERequest.scfvsr = vsr;

			// locate in output by accumulating integral output sizes
			outpic->memory.rgb.phys += m_AccumOutputHeight * m_nBytesPP * outpic->width;
			m_AccumOutputHeight += ySegment;
		}
	}

	if ((inpic->flags & VS_FLAGS_INTERLACED) && (m_nPictureMode == ITE_MODE_PROGRESSIVE))
	{
		m_nPictureMode = ITE_MODE_INTERLACED;
		maeite_set_default_scf(&m_ITERequest, m_nPictureMode);
	}
	else if (!(inpic->flags & VS_FLAGS_INTERLACED) && (m_nPictureMode == ITE_MODE_INTERLACED))
	{
		m_nPictureMode = ITE_MODE_PROGRESSIVE;
		maeite_set_default_scf(&m_ITERequest, m_nPictureMode);
	}
/*
    if (inpic->flags & VS_FLAGS_INTERLACED)
        req = &maebe.fi_req;
    else
*/

//		RETAILMSG(1, (TEXT("vrmaebe_render: req->dstcfg = 0x%x\r\n"), req->dstcfg));
		req->magic = MAEBE_MAGIC;

//    if (vrenderDump) dump_yuv(inpic);

    // Examine MODIFIED attribute to determine if BE/ITE must do a dcache writeback
    req->flags = (inpic->flags & VS_FLAGS_MODIFIED) ? MAEBE_FLAGS_DCWB : 0;

    req->srcaaddr = inpic->memory.yuv.y_phys + yuv_offset;
    req->srcbaddr = inpic->memory.yuv.u_phys + yuv_offset;
    req->srccaddr = inpic->memory.yuv.v_phys + yuv_offset;
    req->srcastr = inpic->memory.yuv.y_mem_width;
    req->srcbstr = inpic->memory.yuv.u_mem_width;
    req->srccstr = inpic->memory.yuv.v_mem_width;

    if (inpic->flags & VS_FLAGS_MAE)
    {
        req->srccfg |= ( 0
            //| MAEBE_SRCCFG_EF
            );
    }

	if (inpic->geometry.yuv.yuv_format == VS_YUV_FORMAT_Y_U_V)
	{
		req->srccfg &= ~(MAEBE_SRCCFG_ILCE);
	}
	else
	{
		req->srccfg |= MAEBE_SRCCFG_ILCE;
	}

	req->srccfg &= ~(MAEBE_SRCCFG_IF_N(SRCCFG_IF_444));
	if (inpic->colorSpace == csYUV422)
	{
		req->srccfg |= MAEBE_SRCCFG_IF_N(SRCCFG_IF_422);
	}
	else if (inpic->colorSpace == csYUV444)
	{
		req->srccfg |= MAEBE_SRCCFG_IF_N(SRCCFG_IF_444);
	}
    if (inpic->flags & VS_FLAGS_EL)
    {
        req->srccfg |= MAEBE_SRCCFG_EF;
    }

    // Destination RGB
    req->dstaddr = outpic->memory.rgb.phys;

    // Scaler settings
    req->ctlfpc = MAEBE_CTLFPC_STR;

    req->scfdisable =  // can skip if *not* interlaced and 1:1
        (req->scfhsr == MAEBE_SCFHSR_SRI_N(1)) &&
        (req->scfvsr == MAEBE_SCFVSR_SRI_N(1)) &&
        ((inpic->flags & VS_FLAGS_INTERLACED) == 0);


	//osal_maebe_driver_submit(&maebe.h, req);
	maeite_submit_frame(m_hITE, req, 0, 0);
}
