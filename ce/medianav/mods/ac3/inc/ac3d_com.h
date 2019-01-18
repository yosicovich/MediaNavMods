/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3d_com.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3D_COM.H#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common defines for AC-3 system
;***************************************************************************/
#ifndef  __AC3D_COM_H__
#define  __AC3D_COM_H__
/**** Algorithm modifiers ****/

/*#define RNDSCLMANT 1		 Round scaled mantissas prior to shifting */

#define KAWARE 1			/* Enables karaoke aware code */
#ifdef  KAWARE
#define KCAPABLE 1			/* Enables karaoke capable code */
#endif

#define MIPOPT 1			/* Optimize for fastest average execution */
#define HALFRATE 1			/* Enable half sample-rate code */

#ifdef  HALFRATE
#define DITHBND 1		    /* frequency dependent dither scaling */
#endif

/**** Linked include files ****/
#include "vo_ac3_var.h"
#include "dolbycom.h"
#include "ac3d_equ.h"
#include "ac3d_sim.h"

#endif //__AC3D_COM_H__


