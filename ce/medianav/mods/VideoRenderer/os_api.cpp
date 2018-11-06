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

///////////////////////////////////////////////////////////////////////////////
//
// OS Abstraction
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include <TCHAR.h>

#include "os_api.h"

#define OS_BUFFER_SIZE 256



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////
//////	MEMPOOL STUFF
//////
//////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	mem_open_driver()
//
//	Arguments:
//
//
//	Returns:
//		
//
///////////////////////////////////////////////////////////////////////////////
DRIVER_HANDLE mem_open_driver()
{
	DRIVER_HANDLE mem_handle = 0;

	mem_handle = CreateFile(L"MEM1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (mem_handle == INVALID_HANDLE_VALUE) {
        OS_Print(1,"Cannot open MEM1: %d\r\n", GetLastError());
        return NULL;
    }

	return mem_handle;
}

///////////////////////////////////////////////////////////////////////////////
//	mem_close_driver()
//
//	Arguments:
//
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
void mem_close_driver(DRIVER_HANDLE mem_handle)
{
	CloseHandle(mem_handle);
}

///////////////////////////////////////////////////////////////////////////////
//	mem_alloc()
//
//	Arguments:
//
//
//	Returns:
//		
//
///////////////////////////////////////////////////////////////////////////////
PMEM_IOCTL mem_alloc(DRIVER_HANDLE driver, ULONG dwSize, DWORD dwRegion, DWORD dwFlags)
{
	MEM_IOCTL		*pMem = NULL;
	unsigned long	dwBytesReturned;

	// Allocate memory desciptor
	pMem = (PMEM_IOCTL)malloc(sizeof(MEM_IOCTL));
	
	if (!pMem)
	{
		return NULL;
	}

	pMem->dwSize = dwSize;
	pMem->dwRegion = dwRegion;
	pMem->dwFlags = dwFlags;

	if (DeviceIoControl(driver, MEM_REQUEST_BLOCK, pMem, sizeof(MEM_IOCTL),
					NULL, 0, &dwBytesReturned, NULL) == 0)
	{
		return NULL;
	}

	if(pMem->pPhysical == NULL)
	{
		return NULL;
	}

	return pMem;
}

///////////////////////////////////////////////////////////////////////////////
//	mem_free()
//
//	Arguments:
//
//
//	Returns:
//		
//
///////////////////////////////////////////////////////////////////////////////
void mem_free(DRIVER_HANDLE driver, PMEM_IOCTL pMem)
{
	DWORD dwBytesReturned; 

	if (DeviceIoControl(driver, MEM_FREE_BLOCK, pMem, sizeof(MEM_IOCTL),
					NULL, 0, &dwBytesReturned, NULL) == 0)
	{
		
	}

	free(pMem);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////
//////	MAE ITE STUFF
//////
//////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int ite_open_count = 0;
DRIVER_HANDLE ite_handle =  0;

///////////////////////////////////////////////////////////////////////////////
//
//	Progressive frame coefficients
//
//////////////////////////////////////////////////////////////////////////////
uint32_t FRAME_FILTER_COEFFS[32] = {
    0x00033A03, 0x00043903, 0x00053902, 0x00063802,
    0x00073801, 0x00083701, 0x00093601, 0x000B3401,
    0x000C3400, 0x000E3200, 0x000F3100, 0x00112F00,
    0x00132D00, 0x00162A00, 0x00182800, 0x001A2600,
    0x001D2300, 0x001F2100, 0x00211F00, 0x00241C00,
    0x00261A00, 0x00291700, 0x002B1500, 0x002D1300,
    0x00301000, 0x01320D00, 0x01330C00, 0x01350A00,
    0x01360900, 0x02370700, 0x02380600, 0x03380500
};

///////////////////////////////////////////////////////////////////////////////
//
//	Interlaced fields coefficients
//
//////////////////////////////////////////////////////////////////////////////
uint32_t FIELD_FILTER_COEFFS[32] = {
    0x00102010, 0x0011200f, 0x01111f0f, 0x01121f0e,
    0x02121e0e, 0x02131e0d, 0x03131d0d, 0x03141d0c,
    0x04141c0c, 0x04151c0b, 0x05151b0b, 0x05161b0a,
    0x06161a0a, 0x06171a09, 0x07171909, 0x07181908, 
	0x08181808, 0x08191807, 0x09191707, 0x091a1706,
    0x0a1a1606, 0x0a1b1605, 0x0b1b1505, 0x0b1c1504,
    0x0c1c1404, 0x0c1d1403, 0x0d1d1303, 0x0d1e1302,
    0x0e1e1202, 0x0e1f1201, 0x0f1f1101, 0x0f201100
};


///////////////////////////////////////////////////////////////////////////////
//	maeite_calc_sr()
//
//	Use this function to calculate a vertical or horizontal scale ratio. This 
//	calculation is original/scaled
//
//	Arguments:
//		int orig_size 
//			The original width/height of the image.
//
//		int scale_size
//			The width/height we are scaling the image to.
//
//	Returns:
//		unsigned int
//			Returns the scale ratio in a format compatible with the MAE ITE.
//			[31:16 Scale Ratio Integer][15:0 Scale Ratio Fractional]
//
//
//	Note 1: With some scale factors, such as 3x, an extra horizontal/vertical pixel 
//		may be created due to precision constraints. For example, an extra pixel 
//		is created in the horizontal/vertical direction, exceeding the intended 
//		frame width (3 x source frame width). In order to avoid this, software 
//		must adjust the scale ratio by adding an offset of 0x0000 0004 to the 
//		fractional portion of the scale ratio. This provides a value slightly 
//		greater than 1/3, which prevents the extra pixel. The value needs to be 
//		0x0000 0004, because the worst case subsampling (shift right) is >> 2.
//
//	Note 2: Due to precision issues, it is possible to get cases where the 
//		subsampled channels create a different number of pixels than the 
//		non-subsampled channel (Cb or Cr compared with Y). To avoid this, 
//		software should program the 2 least significant bits of the fractional 
//		portion of the horizontal/vertical scale ratio to 0.
///////////////////////////////////////////////////////////////////////////////
unsigned int maeite_calc_sr(int orig_size, int scale_size)
{
	float		scale_ratio;
	uint32_t	sri;
	uint32_t	srf;
	uint32_t	ret_val = 0;

	scale_ratio = (float)orig_size / (float)scale_size;



	sri = (uint32_t)scale_ratio;
	srf = (uint32_t) ((1 << 16) * (scale_ratio - sri)) ;

	if ((srf & 0xffff) != 0)
	{
		srf = srf + 4;			// See Note 1
		srf = srf & 0xfffffffc; // See Note 2
	}

	ret_val = (sri << 16) | srf;

	return ret_val;

}

///////////////////////////////////////////////////////////////////////////////
//	maeite_set_default_csc()
//
//	This will set the default color space conversion coefficients.
//
//	Arguments:
//		mae_ite_request ite 
//			Structure containing MAE ITE register values.  The coefficients will 
//			be stored here.
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
void maeite_set_default_csc(mae_be_request_t *ite)
{
	ite->cscxcffa = 0x000004A7;
	ite->cscxcffb = 0x00000000;
	ite->cscxcffc = 0x00000662;
	ite->cscxoff  = 0x0000F211;

	ite->cscycffa = 0x000004A7;
	ite->cscycffb = 0x00001191;
	ite->cscycffc = 0x00001340;
	ite->cscyoff  = 0x00000879;

	ite->csczcffa = 0x000004A7;
	ite->csczcffb = 0x00000811;
	ite->csczcffc = 0x00000000;
	ite->csczoff  = 0x0000EEB3;

	ite->cscalpha = 0x000000FF;
}

///////////////////////////////////////////////////////////////////////////////
//	maeite_set_default_scf()
//
//	This will set the default scale coefficients based on whether the mode is
//	interlaced or progressive.
//
//	Arguments:
//		mae_ite_request ite 
//			Structure containing MAE ITE register values.  The coefficients will 
//			be stored here.
//
//		unsigned int mode
//			Indicates the picture mode:
//			0 = Progressive
//			1 = Interlaced
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
void maeite_set_default_scf(mae_be_request_t *ite, uint32_t mode)
{
	int i;
	uint32_t *filter_coeffs = mode ? FIELD_FILTER_COEFFS : FRAME_FILTER_COEFFS;

	for(i = 0; i < 32; i++)
	{
		ite->scfhalut[i] = FRAME_FILTER_COEFFS[i];
		ite->scfvalut[i] = filter_coeffs[i];

		ite->scfhblut[i] = FRAME_FILTER_COEFFS[i];
		ite->scfvblut[i] = filter_coeffs[i];

		ite->scfhclut[i] = FRAME_FILTER_COEFFS[i];
		ite->scfvclut[i] = filter_coeffs[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
//	maeite_setup_src_regs()
//
//	Arguments:
//
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
void maeite_setup_src_regs(mae_be_request_t *ite, uint32_t fourcc, uint32_t address, 
						   uint32_t width, uint32_t height)
{
	// clear the config value
	ite->srccfg = 0;

	switch (fourcc)
	{
	case FOURCC_MPEYUV:
		{
			// Enable UV interleaving
			ite->srccfg = MAEBE_SRCCFG_ILCE | MAEBE_SRCCFG_ICM_UV;
		}
		break;
	///////////////////////////////////////////////////////////////////////////
	//	Packed YUV formats
	///////////////////////////////////////////////////////////////////////////
	case FOURCC_UYVY:
//		RETAILMSG(1, (TEXT("---- FOURCC_UYVY ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_ILE | MAEBE_SRCCFG_ILM_N(SRCCFG_ILM_UYVU) | MAEBE_SRCCFG_IF_N(SRCCFG_IF_422);
		ite->srcaaddr = ite->srcbaddr = ite->srccaddr = address;
		ite->srcastr = ite->srcbstr = ite->srccstr = width * 2;
		break;

	case FOURCC_VYUY:
//		RETAILMSG(1, (TEXT("---- FOURCC_VYUY ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_ILE | MAEBE_SRCCFG_ILM_N(SRCCFG_ILM_VYUY) | MAEBE_SRCCFG_IF_N(SRCCFG_IF_422);
		ite->srcaaddr = ite->srcbaddr = ite->srccaddr = address;
		ite->srcastr = ite->srcbstr = ite->srccstr = width * 2;
		break;

	case FOURCC_YUY2:
	case FOURCC_YUYV: // equivalent to YUY2
//		RETAILMSG(1, (TEXT("---- FOURCC_YUYV ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_ILE | MAEBE_SRCCFG_ILM_N(SRCCFG_ILM_YUYV) | MAEBE_SRCCFG_IF_N(SRCCFG_IF_422);
		ite->srcaaddr = ite->srcbaddr = ite->srccaddr = address;
		ite->srcastr = ite->srcbstr = ite->srccstr = width * 2;
		break;

	case FOURCC_YVYU:
//		RETAILMSG(1, (TEXT("---- FOURCC_YVYU ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_ILE | MAEBE_SRCCFG_ILM_N(SRCCFG_ILM_YVYU) | MAEBE_SRCCFG_IF_N(SRCCFG_IF_422);
		ite->srcaaddr = ite->srcbaddr = ite->srccaddr = address;
		ite->srcastr = ite->srcbstr = ite->srccstr = width * 2;
		break;

	///////////////////////////////////////////////////////////////////////////
	//	Planar YUV formats
	///////////////////////////////////////////////////////////////////////////
	case FOURCC_I420:
	case FOURCC_IYUV: // equivalent to I420
//		RETAILMSG(1, (TEXT("---- FOURCC_IYUV ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_IF_N(SRCCFG_IF_420);
		ite->srcaaddr = address;
		ite->srcbaddr = ite->srcaaddr + (width * height);
		ite->srccaddr = ite->srcbaddr + (width * height) / 4;
		ite->srcastr = width;
		ite->srcbstr = ite->srccstr = width / 2;
		break;

	case FOURCC_YV12:
	default:
//		RETAILMSG(1, (TEXT("---- FOURCC_YV12 ----\r\n")));
		ite->srccfg = MAEBE_SRCCFG_IF_N(SRCCFG_IF_420);
		ite->srcaaddr = address;
		// V comes before U
		ite->srccaddr = ite->srcaaddr + (width * height);
		ite->srcbaddr = ite->srccaddr + (width * height) / 4;
		ite->srcastr = width;
		ite->srcbstr = ite->srccstr = width / 2;

	}

	// Set source height and width
	ite->srcfhw &= ~MAEBE_SRCFHW_WIDTH_N(MAEBE_SRCFHW_MASK);
	ite->srcfhw &= ~MAEBE_SRCFHW_HEIGHT_N(MAEBE_SRCFHW_MASK);

	ite->srcfhw |= MAEBE_SRCFHW_WIDTH_N(width);
	ite->srcfhw |= MAEBE_SRCFHW_HEIGHT_N(height);
}

///////////////////////////////////////////////////////////////////////////////
//	maeite_submit_frame()
//
//	Arguments:
//
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
int maeite_submit_frame(DRIVER_HANDLE driver, mae_be_request_t *ite, uint32_t timeout, uint32_t flags)
{
	if (!DeviceIoControl(driver, MAEBE_IOCTL_SUBMIT_TRANSACTION,
						 ite, sizeof(mae_be_request_t), NULL, 0, NULL, NULL))
	{
		OS_Print(1,"OS_API: MAEBE_IOCTL_SUBMIT_TRANSACTION Failed!: %d\r\n", GetLastError());
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//	maeite_open_driver()
//
//	Arguments:
//
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
DRIVER_HANDLE maeite_open_driver()
{
	if (0 == ite_open_count)
	{
		ite_handle = CreateFile(L"ITE1:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	}
	ite_open_count++;

	if (ite_handle == INVALID_HANDLE_VALUE) {
		OS_Print(1,"OS_API: Cannot open ITE1: %d\r\n", GetLastError());
        return NULL;
    }

	return ite_handle;
}

///////////////////////////////////////////////////////////////////////////////
//	maeite_close_driver()
//
//	Arguments:
//
//
//	Returns:
//		none
//
///////////////////////////////////////////////////////////////////////////////
void maeite_close_driver(DRIVER_HANDLE driver)
{
	ite_open_count--;

	if (0 == ite_open_count)
	{
		if (driver == ite_handle)
		{
			CloseHandle(ite_handle);
		}
		else
		{
			OS_Print(1, "ite handle mismatch error!\r\n");
		}

	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////
//////	LCD STUFF
//////
//////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	OS_GetDesktopRotation()
//
//	Gets the current orientation of the Desktop.  The return value is degrees
//	clockwise relative to 0 degrees orientation.
//
//	Arguments:
//		None.
//
//	Returns:
//		AU_ROTATE_0		0 degrees
//		AU_ROTATE_90	90 degrees
//		AU_ROTATE_180	180 degrees
//		AU_ROTATE_270	270 degrees
//
///////////////////////////////////////////////////////////////////////////////
int OS_GetDesktopRotation()
{
	DEVMODE DevMode;
	int CurrentAngle;
	int retval; 

	// Get the current rotation angle.
	memset(&DevMode, 0, sizeof (DevMode));
	DevMode.dmSize   = sizeof (DevMode);
	DevMode.dmFields = DM_DISPLAYORIENTATION;

	if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DevMode, NULL, CDS_TEST, NULL))
	{
		CurrentAngle = DevMode.dmDisplayOrientation;
	}
	else
	{ 
		CurrentAngle = -1;
	}

	// Only angle 270 is different in windows.
	if (CurrentAngle == DMDO_270)
		retval = AU_ROTATE_270;
	else
		retval = CurrentAngle;


	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_GetScreenWidth()
{
	return GetSystemMetrics(SM_CXSCREEN);
}
 
///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_GetScreenHeight()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_ShowOverlay(unsigned int nOverlayIndex, BOOL bShow)
{
	HDC hDC = GetDC(NULL);

	if (bShow)
		ExtEscape(hDC, LCD_OVERLAY_ENABLE, sizeof(nOverlayIndex), (char*)&nOverlayIndex, 0, NULL);
	else
		ExtEscape(hDC, LCD_OVERLAY_DISABLE, sizeof(nOverlayIndex), (char*)&nOverlayIndex, 0, NULL);

	ReleaseDC(NULL, hDC);

	return NOERROR;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_SetScreenColorkey(LCD_COLORKEY_IOCTL ckIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);
	
	retval = ExtEscape(hDC, LCD_COLORKEY_SET, sizeof(ckIoctl), (LPSTR)&ckIoctl, 0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_GetScreenColorkey(LCD_COLORKEY_IOCTL *pckIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_COLORKEY_GET, 0, NULL, sizeof(*pckIoctl), (LPSTR)pckIoctl);

	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_SetScreenBackground(unsigned int nBackRGB)
{
	int retval;
	HDC hDC = GetDC(NULL);
	
	retval = ExtEscape(hDC, LCD_BACKGROUND_SET, sizeof(nBackRGB), (LPSTR)&nBackRGB, 0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
unsigned int OS_GetScreenBackground()
{
	unsigned int nBackRGB;
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_BACKGROUND_GET, sizeof(nBackRGB), (LPSTR)&nBackRGB, 0, NULL);

	ReleaseDC(NULL, hDC);	
	return nBackRGB;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_CreateOverlay(OVERLAY_IOCTL *povlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_OVERLAY_CREATE, sizeof(*povlIoctl), (LPCSTR)povlIoctl, 0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_SetOverlayConfig(OVERLAY_IOCTL ovlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	ovlIoctl.flags = 0;

	retval = ExtEscape(hDC, LCD_OVERLAY_CONFIG, sizeof(ovlIoctl), (LPCSTR)&ovlIoctl, 0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_GetOverlayConfig(OVERLAY_IOCTL *povlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	povlIoctl->flags = OVERLAY_CONFIG_GET;

	retval = ExtEscape(hDC, LCD_OVERLAY_CONFIG, sizeof(*povlIoctl), (LPCSTR)povlIoctl, 0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_UpdateOverlay(OVERLAY_UPDATE_IOCTL ovlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_OVERLAY_UPDATE, sizeof(ovlIoctl), (LPCSTR)&ovlIoctl ,0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_SetNextOverlayBuffer(OVERLAY_UPDATE_IOCTL ovlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_OVERLAY_SET_NEXT_BUFFER, sizeof(ovlIoctl), (LPCSTR)&ovlIoctl ,0, NULL);
	
	ReleaseDC(NULL, hDC);
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_DestroyOverlay(OVERLAY_IOCTL ovlIoctl)
{
	int retval;
	HDC hDC = GetDC(NULL);

	retval = ExtEscape(hDC, LCD_OVERLAY_DESTROY, sizeof(ovlIoctl), (LPCSTR)&ovlIoctl, 0, NULL);
	
	ReleaseDC(NULL, hDC); 
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////
//////	GENERAL STUFF
//////
//////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
int OS_Print(BOOL cond, const char* string, ...)
{
	WCHAR message[OS_BUFFER_SIZE];
	WCHAR wstring[OS_BUFFER_SIZE];
	va_list ptr;
	int iRequiredSize = 0;
	
	if (!cond) 
		return 0;

	// Find out required string size (if you don't have that info)
	iRequiredSize = ::MultiByteToWideChar(CP_ACP, NULL, string, -1, NULL, 0);

	if (iRequiredSize > OS_BUFFER_SIZE)
		iRequiredSize = OS_BUFFER_SIZE;

	// Do the string conversion
	::MultiByteToWideChar(CP_ACP, NULL, string , -1, wstring, iRequiredSize);
    
    va_start(ptr,string);
	vswprintf(message, wstring, ptr);
    va_end(ptr);

	RETAILMSG(1, (message));
	return 1;
}
