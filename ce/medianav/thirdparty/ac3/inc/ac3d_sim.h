/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3d_sim.h
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

/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3D_SIM.H#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 simulator miscellaneous parameters
;***************************************************************************/
#ifndef  __AC3D_SIM_H__
#define  __AC3D_SIM_H__

#include  "vo_ac3_var.h"
/**** AC-3 subroutine function prototypes ****/
DOLBY_SIP ac3_info(DECEXEC *ac3_dec);
DOLBY_SIP ac3_dec(DOLBY_SIP input_sip, DECEXEC *p_decexec);
void ac3_init(DECEXEC *p_decexec);
void bsi_d(DEC_PARAM *ac3_decparam);
void unp_bsi(DECEXEC *ac3_dec);
int  xdcall(DEC_PARAM  *p_decparam);
int  mants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
int  chmants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
int  lfemants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void calc_gain(int compval, DSPshort dialval, DEC_PARAM  *p_decparam);
int  scale_cplco(DEC_PARAM *p_decparam);
void unpexps(PKEXPS *pkexpptr, DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);

int expand_delta(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
int expand_delta_0(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
int expand_delta_1(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);

void unpmants(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void bitunp_lj(DSPshort *dataptr, DSPshort numbits, AC3_BSPK *p_bstrm);
void bitunp_rj(DSPshort *dataptr, DSPshort numbits, AC3_BSPK *p_bstrm);
void bitskip(DSPshort numbits, AC3_BSPK *p_bstrm);
void matrix_d(AC3_AB_DATA *p_aublk, BUFF_PARAM *p_buff);
void idctsc(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void cifft(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void idctsc2(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void downmix(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff);
void clr_downmix(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff); 
void setup_downmix(DEC_PARAM *p_decparam);
void window_d(DSPshort chan, DECEXEC *p_decexec);

#ifdef DSPPREC
/**** DSP word widths ****/
#define	DEF_WIDTH	20
#define WTABSTOR	DEF_WIDTH			/* window table */
#define TABSTOR		DEF_WIDTH			/* cosine/sine tables */
#define QNTZSTOR	DEF_WIDTH			/* symmetric mantissa tables */
#define DYNSCSTOR	DEF_WIDTH			/* dynamic range scaling */
#define MIXSTOR		DEF_WIDTH			/* downmixing coeff tables */
#define PHSSTOR		DEF_WIDTH			/* phscor tables */

#define MANTBITS	DEF_WIDTH			/* unnormalized mantissas */
#define DITHBITS	DEF_WIDTH			/* dither scaling */
#define DCTBITS		DEF_WIDTH			/* inverse DCT */
#define DNMXBITS	DEF_WIDTH			/* downmixing */
#define WINDBITS	DEF_WIDTH			/* inverse windowing */

/**** DSP rounding types ****/
#define WTABRND		NONCONV				/* window function */
#define TABRND		NONCONV				/* sine tables */
#define QNTZRND		NONCONV				/* symmetric mantissa tables */
#define DYNSCRND	NONCONV				/* dynamic range scaling */
#define MIXRND		NONCONV				/* downmixing coeff tables */
#define PHSRND		NONCONV				/* phscor tables */
#define MANTRND		NONCONV				/* unnormalized mantissas */
#define DITHRND		NONCONV				/* dither scaling */
#define DCTRND		NONCONV				/* inverse DCT */
#define DNMXRND		NONCONV				/* downmixing */
#define WINDRND		NONCONV				/* inverse windowing */
#define	MAXSHIFT	DEF_WIDTH			/* max decoder shift */
#else
#define	MAXSHIFT	MAXEXP				/* max decoder shift */
#endif /* DSPPREC */

#ifdef DEBUG
/**** Debugging output defines ****/
#define DBG_BITSTR	0x0001				/* Bitstream data					*/
#define DBG_XPACK	0x0002				/* Packed exponents					*/
#define DBG_XDATA	0x0004				/* Expanded exponents				*/
#define DBG_GBITA	0x0008				/* Bit allocation data				*/
#define DBG_MPACK	0x0010				/* Packed mantissa data				*/
#define DBG_DITHR	0x0020				/* Dither values					*/
#define	DBG_PHSCOR	0x0040				/* PHSCOR data						*/
#define DBG_MANT	0x0080				/* Expanded mantissa data			*/
#define DBG_COUPLE	0x0100				/* Coupling data					*/
#define DBG_MIXTAB	0x0200				/* Downmix table					*/
#define DBG_DNMIX	0x0400				/* Transient detector				*/
#define DBG_DCST	0x0800				/* DCT/DST data						*/
#define DBG_PCM		0x1000				/* PCM data							*/
				/*	0x2000	*/
				/*	0x4000	*/
#define DBG_BITPOS	0x8000				/* Bit stream position info			*/
#endif /* DEBUG */

#endif //__AC3D_SIM_H__
