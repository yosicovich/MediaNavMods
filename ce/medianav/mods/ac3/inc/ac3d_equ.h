/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3d_equ.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3D_EQU.H#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common equates for AC-3 system
;***************************************************************************/
#ifndef  __AC3D_EQU_H__
#define  __AC3D_EQU_H__

#include "voType.h"
#ifdef HALFRATE
#define		MAXBSID			10			/* max bsid revision number */
#define 	NHALFRATE		3
#else
#define		MAXBSID			8			/* max bsid revision number */
#endif

/**** General system equates ****/

#define		NCHANS			6			/* max # channels */
#define		NFCHANS			5			/* max # full bw channels */
#define		NPCMCHANS		6			/* max # output channels */

/**** Filter bank equates ****/

#define		N				256			/* block size */
#define		FFTN			(N/2)		/* FFT transform size */
#define		FFTNLG2M3		4			/* log2(FFTN) - 3 */
#define		NMANTS			253			/* max # mantissas per channel */
#define		NLFEMANTS		7			/* # lfe mantissas */

/**** Exponent, mantissa equates ****/

#define		MAXEXP			32			/* max decoder exponent */

/**** Coupling equates ****/

#define		CPLBNDSZ		12			/* coupling band size */
#define		NCPLBNDS		18			/* max # coupling bands */
#define		NUCPLCHANCFG	2			/* acmod 0 and 1 never coupled */

/**** Bit allocation equates ****/

#define		LATABSZ 		256			/* log adder table size */
#define		PVTABSZ			64			/* bit alloc pointer table size */
#define		NBABNDS			50			/* max # bit allocation bands */
#define		NLFEBABNDS		NLFEMANTS	/* # lfe bit allocation bands */
#define		NDELTS			8			/* max # delta bit alloc segments */
#define		BANDBUFSZ		24			/* max bit alloc band size */
#define		LOWCOMP_LOW		384			/* lowcomp init for band<=6		*/
#define		LOWCOMP_MID		320			/* lowcomp init for 6<band<=19	*/


/**** Random number generator equates ****/

#define		DITHSEED		1			/* random number generator seed */
#define		DITHMULT		47989		/* linear congruential multiplier */
#define		DITHSCALE		0x5a827999L /* 0.707106781 dither scale factor Q31 */

#ifdef DITHBND
#define 	NDITHSCALE		3
#define		DTHRESH1		29
#define		DTHRESH2		37
#endif

/**** Miscellaneous equates ****/

#define		SYNC_WD			((DSPshort)0x0b77)	/* AC-3 frame sync word */
#define		NFSCOD			3			/* # defined sample rates */
#define		NDATARATE		38			/* # defined data rates */
#define		NREMATBNDS		4			/* max # rematrixing bands */

#define 	UNITY			0x7FFFFFFFL
#define		M1_5DB			0x6ba27e64L
#define 	M3DB			0x5a827999L
#define		M4_5DB			0x4c1bf827L
#define 	M6DB			0x40000000L

/**** Enumerations ****/

enum { REUSE,
       D15, 
	   D25, 
	   D45,
       EX_MAX = VO_MAX_ENUM_VALUE};			/* exponent strategy */

enum { MODE11, 
       MODE10, 
	   MODE20, 
	   MODE30,	/* audio coding mode */
	   MODE21, 
	   MODE31, 
	   MODE22, 
	   MODE32,
       AM_MAX = VO_MAX_ENUM_VALUE};

enum { DUAL_STEREO, 
       DUAL_LEFTMONO,		/* dual mono downmix mode */
	   DUAL_RGHTMONO, 
	   DUAL_MIXMONO,
       DM_MAX = VO_MAX_ENUM_VALUE};

enum {AUTO, 
      LTRT,
	  LORO,
      PS_MAX = VO_MAX_ENUM_VALUE};				/* preferred stereo mode */

enum { COMP_CUSTOM_A, 
       COMP_CUSTOM_D,	/* compression mode */
	   COMP_LINE, 
	   COMP_RF,
       CM_MAX = VO_MAX_ENUM_VALUE};

enum { DELTREUSE, 
       DELTNEW, 
	   DELTSTOP,
       DB_MAX = VO_MAX_ENUM_VALUE};	/* delta bit allocation strategy */

/**** Downmixing Equates ****/

#define PLUS11DB_INT	7509
#define MINUS11DB_INT	-7509
//#define PLUS11DB	DSPs2f(7509)
//#define MINUS11DB	DSPs2f(-7509)

/**** Phasecor Equates ****/

#define		NPHSCORBITS		4			/* # bits per phscor value */
#define		DEFPHSCOR		0x0			/* default phscor value */
#define		NPHSOUTMODBITS	3			/* # bits per phsoutmod value */
#define		DEFPHSOUTMOD	MODE32		/* default phsoutmod value */
#define		PHSTABSZ		31
#define		PHSTABOFFST		15			/* phscor table offset */

enum {									/* phscor bitstream strategies	*/
	COEF_PER_BLOCK=0,
	COEF_PER_BAND,
	PB_MAX = VO_MAX_ENUM_VALUE
	};

/**** Karaoke Mode Equate ****/

#ifdef KAWARE
#define	KARAOKE_MODE	7				/* bsmod used to indicate karaoke */

#ifdef KCAPABLE
#define PI				3.14159265358
#endif
#endif

#endif //__AC3D_EQU_H__
