/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            dolbycom.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DOLBYCOM.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common defines for Dolby subroutines
;***************************************************************************/
#ifndef  __DOLBYCOM_H__
#define  __DOLBYCOM_H__

/**** Algorithm modifiers ****/
//#define DEBUG 0 			/* Enable debug file support */
#define ERRMSG 0			/* Enable error message support */
/*#define DSPPREC 1			// Enable finite wordlength modelling code */
/*#define THIRTYTWO 1		// Enable if short int is a 32-bit value */

/**** Linked include files ****/

#include "dolbysip.h"
#include "ac3i_sip.h"
#include "crc_sip.h"
#include "ac3d_sip.h"
#include "dolbyequ.h"
#include "dolbysim.h"

#endif   //__DOLBYCOM_H__
