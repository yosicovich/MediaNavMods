/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/MATRIX_D.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Rematrixing decode routine
;***************************************************************************/

#include "ac3d_com.h"

/* const table */
extern DSPshort rematstart[NREMATBNDS], rematend[NREMATBNDS];

void matrix_d(AC3_AB_DATA *p_aublk, BUFF_PARAM *p_buff)
{
	int mant_l, mant_r;
	int bnd, bin, start, end, minend;

	minend = DSPmin(p_aublk->endmant[0], p_aublk->endmant[1]);

	for (bnd = 0; bnd < p_aublk->nrematbnds; bnd++)
	{
		if (p_aublk->rematflg[bnd])
		{
			start = rematstart[bnd];
			end = DSPmin(rematend[bnd], minend);
			for (bin = start; bin < end; bin++)
			{
				mant_l = p_buff->tc1[bin];
				mant_r = p_buff->tc2[bin];
				p_buff->tc1[bin] = DSPrnd(MANTRND, MANTBITS, mant_l + mant_r);
				p_buff->tc2[bin] = DSPrnd(MANTRND, MANTBITS, mant_l - mant_r);
			}
		}
	}
}
