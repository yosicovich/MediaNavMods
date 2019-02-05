/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            dolbysip.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/DOLBYSIP.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Dolby Software Interface Protocol equates
;***************************************************************************/
#ifndef  __DOLBYSIP_H__
#define  __DOLBYSIP_H__

#include  "vo_ac3_var.h"
/**** DSP type definitions ****/
typedef short DSPshort;					/* DSP integer */
typedef unsigned short DSPushort;		/* DSP unsigned integer */
typedef int DSPint;						/* native integer */
typedef double DSPfract;				/* DSP fixed-point fractional */

/**** Function number equates ****/
#define	DD_SYS_INIT			0		/* System initialization */
/**** Return status equates ****/
#define	DOLBY_ERR_NONE		0		/* no error */
#define	DOLBY_ERR_NAVAIL	-1		/* function not available */
/**** Channel ordering equates ****/

#define	LEFT				0		/* left channel */
#define	CNTR				1		/* center channel */
#define	RGHT				2		/* right channel */
#define	LSUR				3		/* left surround channel */
#define	RSUR				4		/* right surround channel */
#define	LFE					5		/* low frequency effects channel */
#define	MSUR				3		/* mono surround channel */
#define	NONE				-1		/* channel not in use */

#endif  //__DOLBYSIP_H__
