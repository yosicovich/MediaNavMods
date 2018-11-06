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
#ifndef __OS_API_H
#define __OS_API_H

#include "ctypes.h"

#include "maeioctl.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	Types
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef HANDLE DRIVER_HANDLE;




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	MAE ITE Stuff
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "mem_ioctl.h"

DRIVER_HANDLE mem_open_driver();
void mem_close_driver(DRIVER_HANDLE driver);
PMEM_IOCTL mem_alloc(DRIVER_HANDLE driver, ULONG dwSize, DWORD dwRegion, DWORD dwFlags);
void mem_free(DRIVER_HANDLE driver, PMEM_IOCTL pMem);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	MAE ITE Stuff
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MAE_SCALE_RATIO_1_TO_1	0x10000


// Apron defines
#define APRON_BYTES_TB    96  // Top & bottom rows
#define APRON_BYTES_LR    48  // Left & right bytes
#define APRON_BYTES       24
#define APRON_BYTES_UV    12

// Format of YUV coming out of the MPE
#define FOURCC_MPEYUV	0x44444444

// Packed YUV formats (1 buffer)
#define FOURCC_UYVY		0x59565955
#define FOURCC_VYUY		0x59555956
#define FOURCC_YUY2		0x32595559
#define FOURCC_YUYV		0x56595559 // equivalent to YUY2
#define FOURCC_YVYU		0x55595659

// Planar YUV formats (3 buffers)
#define FOURCC_YV16		0x36315659
#define FOURCC_YV12		0x32315659
#define FOURCC_I420		0x30323449
#define FOURCC_IYUV		0x56555949 // equivalent to I420


///////////////////////////////////////////////////////////////////////////////
//	Scaler/Filter (SCF) Registers
///////////////////////////////////////////////////////////////////////////////
//	Scaler registers
#define	SCF_DISABLE				1
#define SCF_ENABLE				0

///////////////////////////////////////////////////////////////////////////////
//	Color Space Converter (CSC) Registers
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	Src Registers
///////////////////////////////////////////////////////////////////////////////

// MAEBE Source Config registers macros and defines
#define MAEBE_SRCCFG_EF		(1 << 10)
#define MAEBE_SRCCFG_BYE	(1 << 9)
#define MAEBE_SRCCFG_BM_N(x)	(x << 7)
// possible values for BM (Bayer Mode)
#define SRCCFG_BM_RGRG		0
#define SRCCFG_BM_GRGR		1
#define SRCCFG_BM_BGBG		2
#define SRCCFG_BM_GBGB		3
#define MAEBE_SRCCFG_ILE	(1 << 6)
#define MAEBE_SRCCFG_ILM_N(x) (x << 4)
// possible values for ILM (Interleaved Mode)
#define SRCCFG_ILM_UYVU		0
#define SRCCFG_ILM_VYUY		1
#define SRCCFG_ILM_YUYV		2
#define SRCCFG_ILM_YVYU		3
#define MAEBE_SRCCFG_IF_N(x)	(x << 2)
// possible values for IF (Input Format)
#define SRCCFG_IF_420		0
#define SRCCFG_IF_422		1
#define SRCCFG_IF_411		2
#define SRCCFG_IF_444		3
#define MAEBE_SRCCFG_ILCE	(1 << 1)
#define MAEBE_SRCCFG_ICM_UV	0x00
#define MAEBE_SRCCFG_ICM_VU	0x01



#define MAEBE_SRCFHW_WIDTH			0
#define MAEBE_SRCFHW_HEIGHT			16
#define MAEBE_SRCFHW_MASK			0x7FF
#define MAEBE_SRCFHW_WIDTH_N(x)		((x & MAEBE_SRCFHW_MASK) << MAEBE_SRCFHW_WIDTH)
#define MAEBE_SRCFHW_HEIGHT_N(x)	((x & MAEBE_SRCFHW_MASK) << MAEBE_SRCFHW_HEIGHT)



///////////////////////////////////////////////////////////////////////////////
//	Dst Registers
///////////////////////////////////////////////////////////////////////////////
#define DSTCFG_ROT_MASK		0x3
#define MAEBE_DSTCFG_ROT_N(x) ((x & DSTCFG_ROT_MASK) << 5)

#define MAEBE_DSTCFG_BGR_EN	(1 << 4)

#define MAEBE_DSTCFG_OF_MASK 0x3
#define MAEBE_DSTCFG_OF_N(x)	((x & MAEBE_DSTCFG_OF_MASK) << 2)
// possible values for output format
#define DSTCFG_OF_24BPP		0
#define DSTCFG_OF_16BPP		2
#define DSTCFG_OF_15BPP		3
/*
struct mae_ite_request {
    uint32_t  scfhsr;
    uint32_t  scfvsr;
    uint32_t  scfdisable;
    uint32_t  scfhalut[32];
    uint32_t  scfvalut[32];
    uint32_t  scfhblut[32];
    uint32_t  scfvblut[32];
    uint32_t  scfhclut[32];
    uint32_t  scfvclut[32];

    uint32_t  cscxcffa;
    uint32_t  cscxcffb;
    uint32_t  cscxcffc;
    uint32_t  cscycffa;
    uint32_t  cscycffb;
    uint32_t  cscycffc;
    uint32_t  csczcffa;
    uint32_t  csczcffb;
    uint32_t  csczcffc;
    uint32_t  cscxoff;
    uint32_t  cscyoff;
    uint32_t  csczoff;
    uint32_t  cscalpha;

    uint32_t  srccfg;
    uint32_t  srcfhw;
    uint32_t  srcaaddr;
    uint32_t  srcastr;
    uint32_t  srcbaddr;
    uint32_t  srcbstr;
    uint32_t  srccaddr;
    uint32_t  srccstr;

    uint32_t  dstcfg;
    uint32_t  dstheight;
    uint32_t  dstaddr;
    uint32_t  dststr;

    uint32_t  ctlenable;
    uint32_t  ctlfpc;
    uint32_t  ctlstat;
    uint32_t  ctlintenable;
    uint32_t  ctlintstat;

    // returned by driver
    uint32_t begMipsCounter;
    uint32_t endMipsCounter;
};
*/
#define ITE_MODE_PROGRESSIVE	0
#define ITE_MODE_INTERLACED		1

unsigned int maeite_calc_sr(int orig_size, int scale_size);
void maeite_set_default_csc(mae_be_request_t *ite);
void maeite_set_default_scf(mae_be_request_t *ite, uint32_t mode);
void maeite_setup_src_regs(mae_be_request_t *ite, uint32_t fourcc, uint32_t address, 
						   uint32_t width, uint32_t height);
int maeite_submit_frame(DRIVER_HANDLE driver, mae_be_request_t *ite, uint32_t timeout, uint32_t flags);
DRIVER_HANDLE maeite_open_driver();
void maeite_close_driver(DRIVER_HANDLE driver);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	LCD Stuff
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "lcd_ioctl.h"

#define AU_ROTATE_0		0
#define AU_ROTATE_90	1
#define AU_ROTATE_180	2
#define AU_ROTATE_270	3

// TODO:
#define AU_RGB_565		0x6
#define AU_RGB_888		0xC
#define AU_RGB_8888		0xD

#define RECT_WIDTH(x) ((x).right - (x).left)
#define RECT_HEIGHT(x) ((x).bottom - (x).top)


#define WINCTRL0_O_MASK 0x7FF   
#define WINCTRL0_A_MASK 0xFF   

#define WINCTRL0_OX(x) ((x & WINCTRL0_O_MASK) << 21)
#define WINCTRL0_OY(x) ((x & WINCTRL0_O_MASK)<< 10)
#define WINCTRL0_A(x) ((x & WINCTRL0_A_MASK) << 2)
#define WINCTRL0_AEN  (1 << 1)


#define WINCTRL1_PRI_MASK 0x3
#define WINCTRL1_PIPE_MASK 0x1

#define WINCTRL1_PRI(x) ((x & WINCTRL1_PRI_MASK) << 30)
#define WINCTRL1_PIPE(x) ((x & WINCTRL1_PIPE_MASK) << 29)

#define WINCTRL1_SZ_MASK 0x7FF
#define WINCTRL1_FORM_MASK	0xF
#define WINCTRL1_PO_MASK	0x3

#define WINCTRL1_FORM(x) ((x & 0xF) << 25)
#define WINCTRL1_CCO_BGR (1 << 24)
#define WINCTRL1_PO(x) ((x & WINCTRL1_PO_MASK) << 22)
#define WINCTRL1_SZX(x) ((x & WINCTRL1_SZ_MASK) << 11)
#define WINCTRL1_SZY(x) (x & WINCTRL1_SZ_MASK)

#define WINCTRL2_BX_MASK 0x1FFF
#define WINCTRL2_RAM_MASK 0x3
#define WINCTRL2_SC_MASK 0xF

#define WINCTRL2_CKMODE(x) (x << 24)
#define WINCTRL2_DBM (1 << 23)
#define WINCTRL2_RAM(x) ((x & WINCTRL2_RAM_MASK) << 21)
#define WINCTRL2_BX(x) ((x & WINCTRL2_BX_MASK) << 8)
#define WINCTRL2_SCX(x) ((x & WINCTRL2_SC_MASK) << 4)
#define WINCTRL2_SCY(x) (x & WINCTRL2_SC_MASK)

#define COLORKEY_COLOR_MASK 0xFF
#define COLORKEY_RED_OFFSET 16
#define COLORKEY_GRN_OFFSET 8
#define COLORKEY_BLU_OFFSET 0
#define GET_COLORKEY_RED(x) ((x & (COLORKEY_COLOR_MASK << COLORKEY_RED_OFFSET)) >> COLORKEY_RED_OFFSET)
#define GET_COLORKEY_GRN(x) ((x & (COLORKEY_COLOR_MASK << COLORKEY_GRN_OFFSET)) >> COLORKEY_GRN_OFFSET)
#define GET_COLORKEY_BLU(x) ((x & (COLORKEY_COLOR_MASK << COLORKEY_BLU_OFFSET)) >> COLORKEY_BLU_OFFSET)


int OS_GetDesktopRotation();
int OS_GetScreenWidth();
int OS_GetScreenHeight();
int OS_ShowOverlay(unsigned int nOverlayIndex, BOOL bShow);
int OS_SetScreenColorkey(LCD_COLORKEY_IOCTL ckIoctl);
int OS_GetScreenColorkey(LCD_COLORKEY_IOCTL *pckIoctl);
int OS_SetScreenBackground(unsigned int nBackRGB);
unsigned int OS_GetScreenBackground();
int OS_CreateOverlay(OVERLAY_IOCTL *povlIoctl);
int OS_SetOverlayConfig(OVERLAY_IOCTL ovlIoctl);
int OS_GetOverlayConfig(OVERLAY_IOCTL *povlIoctl);
int OS_UpdateOverlay(OVERLAY_UPDATE_IOCTL ovlIoctl);
int OS_SetNextOverlayBuffer(OVERLAY_UPDATE_IOCTL ovlIoctl);
int OS_DestroyOverlay(OVERLAY_IOCTL ovlIoctl);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////	More General Stuff
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int OS_Print(BOOL cond, const char* string, ...);

#endif // __OS_API_H
