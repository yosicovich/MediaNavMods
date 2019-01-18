/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DOLBYTAB.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Common Dolby subroutine tables
;***************************************************************************/
#include "ac3d_com.h"

DSPushort chanetab[NCHANCFG * 2] = {
	0xa000,		/* 1+1 mode */
	0x4000,		/* 1/0 mode */
	0xa000,		/* 2/0 mode */
	0xe000,		/* 3/0 mode */
	0xb000,		/* 2/1 mode */
	0xf000,		/* 3/1 mode */
	0xb800,		/* 2/2 mode */
	0xf800,		/* 3/2 mode */
	0xa400,		/* 1+1 mode w/lfe */
	0x4400,		/* 1/0 mode w/lfe */
	0xa400,		/* 2/0 mode w/lfe */
	0xe400,		/* 3/0 mode w/lfe */
	0xb400,		/* 2/1 mode w/lfe */
	0xf400,		/* 3/1 mode w/lfe */
	0xbc00,		/* 2/2 mode w/lfe */
	0xfc00		/* 3/2 mode w/lfe */
};

DSPshort chanary[NCHANCFG] =
{	2, 1, 2, 3, 3, 4, 4, 5 };

DSPshort nfront[NCHANCFG] = 
{	2, 1, 2, 3, 2, 3, 2, 3 };

DSPshort nrear[NCHANCFG] = 
{	0, 0, 0, 0, 1, 1, 2, 2 };

DSPshort chantab[NCHANCFG][NPCMCHANS] = 
{	{LEFT,	RGHT,	LFE,	NONE,	NONE,	NONE},		/* 1+1 */
	{CNTR,	LFE,	NONE,	NONE,	NONE,	NONE},		/* 1/0 */
	{LEFT,	RGHT,	LFE,	NONE,	NONE,	NONE},		/* 2/0 */
	{LEFT,	CNTR,	RGHT,	LFE,	NONE,	NONE},		/* 3/0 */
	{LEFT,	RGHT,	MSUR,	LFE,	NONE,	NONE},		/* 2/1 */
	{LEFT,	CNTR,	RGHT,	MSUR,	LFE,	NONE},		/* 3/1 */
	{LEFT,	RGHT,	LSUR,	RSUR,	LFE,	NONE},		/* 2/2 */
	{LEFT,	CNTR,	RGHT,	LSUR,	RSUR,	LFE}	};	/* 3/2 */

#ifdef KCAPABLE
int kcapabletab[NKCAPABLEMODES][NKCAPABLEVARS] =
{{	0, 0, 0, 0, 1, 0},			/* no vocals */
{	1, 0, 0, 0, 1, 0},			/* left vocal */
{	0, 0, 1, 0, 1, 0},			/* right vocal */
{	1, 1, 1, -1, 1,0}};		/* both vocals */
#endif
