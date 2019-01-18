/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            dolbyequ.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DOLBYEQU.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common equates for Dolby subroutine
;***************************************************************************/
#ifndef  __DOLBYEQU_H__
#define  __DOLBYEQU_H__

#define	SIP_MAJ_REV		7
#define	SIP_MIN_REV		1
#define	SIP_REV			((SIP_MAJ_REV<<8)+SIP_MIN_REV)

/**** Miscellaneous constants ****/
#define 	NULLPTR			((void *)0)		/* value for null address */
#define		TRUE			1
#define		FALSE			0

#define		NPCMCHANS		6				/* # of SIP PCM channels */
#define		NCHANCFG		8				/* # of SIP channel configs */

enum { WARNING=0, 
       FATAL,
       EM_MAX = VO_MAX_ENUM_VALUE};				/* Error message equates */
enum { CONV=0, 
       NONCONV,
	   TRUNC,
       RT_MAX = VO_MAX_ENUM_VALUE};		/* Rounding types */

#endif  //__DOLBYEQU_H__
