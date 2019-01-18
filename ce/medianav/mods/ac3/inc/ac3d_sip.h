/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3d_sip.h
*
* Project:
* contents/description:vo fixed version porting 
*            
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    12-19-2008        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/AC3D_SIP.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 Decoder Software Interface Protocol equates
;***************************************************************************/
#ifndef  __AC3D_SIP_H__
#define  __AC3D_SIP_H__

/**** Function number equates ****/
#define	DD_AC3_DEC			3		/* AC-3 decoding */

/**** AC-3 decode return status equates ****/
#define	AC3D_ERR_NONE		0		/* no error, data stream was valid */
#define	AC3D_ERR_RPT		1		/* user block repeat request */
#define	AC3D_ERR_MUTE		2		/* user mute request */
#define	AC3D_ERR_REV		3		/* unsupported bitstream revision */
#define	AC3D_ERR_CHANS		4		/* too many channels in data stream */
#define	AC3D_ERR_MISC		5		/* miscellaneous data error */

#endif //__AC3_SIP_H__
