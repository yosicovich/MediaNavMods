/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3i_sip.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/AC3I_SIP.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 Frame Information Software Interface Protocol equates
;***************************************************************************/
#ifndef  __AC3I_SIP_H__
#define  __AC3I_SIP_H__

/**** Function number equates ****/
#define	DD_AC3_INFO			1		/* AC-3 frame information */
/**** AC-3 frame information return status equates ****/

#define	AC3I_ERR_NONE		0		/* no error, data stream was valid */
#define	AC3I_ERR_SYNC		1		/* invalid sync word */
#define	AC3I_ERR_SAMPRATE	2		/* invalid sample rate */
#define	AC3I_ERR_DATARATE	3		/* invalid data rate */
#define	AC3I_ERR_REV		4		/* invalid bitstream revision */

#endif  //__AC3I_SIP_H__
