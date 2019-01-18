/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            usr_com.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/USR_COM.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common defines for AC-3 system
;***************************************************************************/
#ifndef  __USR_COM_H__
#define  __USR_COM_H__

/**** Algorithm modifiers ****/
#define DEBUG 1				/* Turns on debugging files */
#define KCAPABLE 1			/* Enables karaoke capable mode */
#define HALFRATE 1			/* Enable half sample-rate code */

/**** Linked include files ****/
#include "dolbysip.h"
#include "ac3i_sip.h"
#include "crc_sip.h"
#include "ac3d_sip.h"
#include "usr_equ.h"
#include "usr_sim.h"

#endif  //__USR_COM_H__
