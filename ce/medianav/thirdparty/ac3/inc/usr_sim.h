/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            usr_sim.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/DECODE/USR_SIM.H#1 $ */
/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 simulator miscellaneous parameters
;***************************************************************************/
#ifndef  __USR_SIM_H__
#define  __USR_SIM_H__

/**** Coder Version ****/
#define DECODERSTR	"Dolby AC-3 Decoder Simulation"
#define COPYRIGHT	"Copyright (c) 1993-2002 Dolby Laboratories, Inc.  All rights reserved."

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

#ifdef 	UNIX
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#endif	/* UNIX */

#ifdef DOS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#endif	/* DOS */

/**** User code function prototypes ****/
void error_msg(char *msg, int errcode);

#ifdef DEBUG
void filetw(char *filename, char *text);
void filefw(char *filename, DSPfract *buf, int start, int count);
void filesw(char *filename, DSPshort *buf, int start, int count);
void fileinit(char *filename);
#endif

enum { WARNING, FATAL };				/* Error message types				*/

#endif  //__USR_SIM_H__
