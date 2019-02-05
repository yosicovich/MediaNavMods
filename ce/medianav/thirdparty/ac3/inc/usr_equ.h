/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            usr_equ.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/USR_EQU.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common equates for AC-3 system
;***************************************************************************/
#ifndef  __USR_EQU_H__
#define  __USR_EQU_H__

#define UPDATE			0				/* executive code update number */
/**** General system equates ****/
#define NBLOCKS			6				/* # of time blocks per frame		*/
#define NCHANS			6				/* max # of discrete channels		*/
#define N				256				/* # of samples per time block		*/

/**** Miscellaneous equates ****/

#define NOUTWORDS		(3840 / 2)		/* max # words per frame			*/
#define NINFOWDS		10				/* # words needed by frame info		*/
//#define SYNC_WD			0x0b77			/* packed data stream sync word		*/
#define PCMCHANSZ		256				/* decoder overlap-add channel size	*/
#define PCM16BIT		1				/* 16-bit PCM code for Dolby SIP	*/

#ifdef  KCAPABLE
#define NKCAPABLEMODES	4				/* # defined karaoke capable modes	*/
#define NKCAPABLEVARS	6				/* # karaoke pan/mix parameters		*/
#endif

#endif  //__USR_EQU_H__
