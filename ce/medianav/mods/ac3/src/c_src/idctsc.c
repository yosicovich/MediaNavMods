/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/IDCTSC.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Inverse DCT / DST / pre-twiddle routine
;***************************************************************************/

#include "ac3d_com.h"
#include "ac3d_fix.h"

//#ifdef AC3D_ARM_OPT

extern DSPshort bitrevary[N/2];
extern int z1mix[N], z2mix[N/2];

void idctsc(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int     *fftrptr, *fftiptr, *fftrptr2, *fftiptr2;
	int     *tcrptr,*tciptr;
	int     ar, ai;
	int     *mixptr;
	int     cr, ci;
	int     count;
	AC3_AB_DATA   *p_aublk = p_decparam->ac3_aub;
	/*	Do complex multiply */
    fftrptr = p_buff->fftbuf;
	tciptr = p_aublk->tcbuf;
	if (p_decparam->bswitch)
	{
		fftiptr = fftrptr + 3*N/4;
		fftrptr2 = fftrptr + N/4;
		fftiptr2 = fftrptr + N/2;
		tcrptr = tciptr + N - 2;
		mixptr = z2mix;

		for (count = N/4; count > 0; count--)
		{
			cr = *mixptr++;
			ci = *mixptr++;
			ar = *tcrptr++;
			ai = *tciptr++;
			*fftrptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ar) - Vo_Multi32DIV2(ci, ai));
			*fftiptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(ci, ar) + Vo_Multi32DIV2(cr, ai));
			ar = *tcrptr--;
			ai = *tciptr--;

			*fftrptr2++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ar) - Vo_Multi32DIV2(ci, ai));
			*fftiptr2++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(ci, ar) + Vo_Multi32DIV2(cr, ai));

			tcrptr -= 4;
			tciptr += 4;
		}
	}
	else
	{
		fftiptr = fftrptr + N/2;
		tcrptr = tciptr + N - 1;
		mixptr = z1mix;

		for (count = N/4; count > 0; count--)
		{
			cr = *mixptr++;
			ci = *mixptr++;
			ar = *tcrptr;
			ai = *tciptr;
			*fftrptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ar) - Vo_Multi32DIV2(ci, ai));
			*fftiptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ai) + Vo_Multi32DIV2(ci, ar));

			tcrptr -= 2;
			tciptr += 2;

			cr = *mixptr++;
			ci = *mixptr++;
			ar = *tcrptr;
			ai = *tciptr;
			*fftrptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ar) - Vo_Multi32DIV2(ci, ai));
			*fftiptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32DIV2(cr, ai) + Vo_Multi32DIV2(ci, ar));

			tcrptr -= 2;
			tciptr += 2;
		}
	}
}

void idctsc2(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int     *fftrptr, *fftiptr, *fftrptr2, *fftiptr2;
	int     *tcrptr, *tciptr, *tcrptr2, *tciptr2;
	int     ar, ai;
	int     cr, ci;
	int     *mixptr;

	int     count, index;
	AC3_AB_DATA   *p_aublk = p_decparam->ac3_aub;

	/*	Do complex multiply */
	fftrptr = p_buff->fftbuf;
	tcrptr = p_aublk->tcbuf;
	if (p_decparam->bswitch)
	{	
		fftiptr = fftrptr + 3*N/4;
		fftrptr2 = fftrptr + N/4;
		fftiptr2 = fftrptr + N/2;	
		tciptr = tcrptr + 3*N/4;
		tcrptr2 = tcrptr + N/4;
		tciptr2 = tcrptr + N/2;
		mixptr = z2mix;

		for (count = 0; count < N/4; count++)
		{
			index = bitrevary[2 * count];
			cr = *mixptr++;
			ci = *mixptr++;
			ar = fftrptr[index];
			ai = fftiptr[index];
			*tcrptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ar) - Vo_Multi32(ci, ai));
			*tciptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ai) + Vo_Multi32(ci, ar));

			ar = fftrptr2[index];
			ai = fftiptr2[index];

			*tcrptr2++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ar) - Vo_Multi32(ci, ai));
			*tciptr2++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ai) + Vo_Multi32(ci, ar));
		}
	}
	else
	{
		fftiptr = fftrptr + N/2;	
		tciptr = tcrptr + N/2;
		mixptr = z1mix;

		for (count = 0; count < N/2; count++)
		{
			index = bitrevary[count];
			cr = *mixptr++;
			ci = *mixptr++;
			ar = fftrptr[index];
			ai = fftiptr[index];
			*tcrptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ar) - Vo_Multi32(ci, ai));
			*tciptr++ = DSPrnd(DCTRND, DCTBITS, Vo_Multi32(cr, ai) + Vo_Multi32(ci, ar));
		}
	}
}

//#endif

