/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            dolbysim.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DOLBYSIM.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Dolby simulator header file
;***************************************************************************/
#ifndef  __DOLBYSIM_H__
#define  __DOLBYSIM_H__

#if defined(DEBUG)
#define FILEIO 1
#endif

#ifdef FILEIO
/**** Platform-related modifiers ****/

#ifdef SUN4			/* Sun 4 running UNIX */
#define UNIX 1		/* Use UNIX-style include files */
#define LITEND 1	/* Data storage is little-endian */
#endif

#ifdef PCDOS		/* PC running MS-DOS */
#define DOS 1		/* Use DOS-style include files */
#define BIGEND 1	/* Data storage is big-endian */
#endif

#ifdef WIN32
#define DOS 1		/* Use DOS-style include files */
#define BIGEND 1	/* Data storage is big-endian */
#endif

#ifdef UNIX
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#endif	/* UNIX */

#ifdef DOS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#endif	/* DOS */

#endif	/* FILEIO */

/**** DSP macros ****/
#define MAXDSPSHIFT		16				/* DSPshort bit length */
#define MAXDSPSHORT		0x8000L			/* fract -> short multiplier */
#define MAXDSPFRACT		0.999999999		/* largest representable fractional */
#define MINDSPFRACT		-1.0			/* smallest representable fractional */
#define DSPf2s(a)		((DSPshort)((a)*MAXDSPSHORT))

//#define DSPs2f(a)		(((DSPfract)(a))/MAXDSPSHORT)
#define DSPs2f(a)       (a)

#define DSPabs(a)		(((a)>=0)?(a):(-(a)))		/* absolute value */
#define DSPcpow(a,b)	(((a)*(a))+((b)*(b)))		/* complex power */
#define DSPmax(a,b)		(((a)>=(b))?(a):(b))		/* maximum value */
#define DSPmin(a,b)		(((a)>=(b))?(b):(a))		/* minimum value */

#ifdef THIRTYTWO
#define SGN_EXTND(a)	(((a)>32767)?((a)-65536):(a))
#else
#define SGN_EXTND(a)	(a)				/* Null function */
#endif

/**** DSP function prototypes ****/
#ifdef DSPPREC
DSPfract DSPrnd(DSPshort type, DSPshort bits, DSPfract a);
#else /* !DSPPREC */
#define DSPrnd(a,b,c)	(c)				/* null function */
#endif /* !DSPPREC */

DSPfract DSPlimit(DSPfract a);
//int      Vo_DSPlimit(int a);

DSPshort DSPsat(DSPshort a, DSPshort b);
DSPshort DSPnorm(DSPfract arg, DSPshort maxnorm);
DSPshort Vo_DSPnorm(int arg, DSPshort maxnorm);
/**** Common routine function prototypes ****/
/**** User code function prototypes ****/
#ifdef DEBUG
void filetw(char *filename, char *text);
void filefw(char *filename, DSPfract *buf, int start, int count);
void filesw(char *filename, DSPshort *buf, int start, int count);
#endif /* DEBUG */

#ifdef ERRMSG
void error_msg(char *msg, int errcode);
#else
#define error_msg(a,b) while (b)
#endif

#endif    //__DOLBYSIM_H__
