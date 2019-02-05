/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/CIFFT.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Inverse FFT routine
;***************************************************************************/
#include "ac3d_com.h"
#include "ac3d_fix.h"

#ifndef AC3D_ARM_OPT

extern int brxmix[N/2];

void cifft(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int i, j, k, m, bg, gp;
	DSPshort fftn, fftnlg2m3, nstep;

	int    ar, ai, br, bi, cr, ci, dr, di;
	int    aici, brdr, arcr, bidi, rtemp, itemp;
	int    *fftrptr, *fftiptr;
	int    *bfyrptr1,*bfyiptr1,*bfyrptr2,*bfyiptr2;
	int    *bfyrptr3,*bfyiptr3,*bfyrptr4,*bfyiptr4;
	int    cr_t, ci_t;
    int    *brxmixptr;

	if (p_decparam->bswitch)
	{
		fftn = N / 4;
		fftnlg2m3 = FFTNLG2M3 - 1;
		nstep = 2;
		fftrptr = p_buff->fftbuf;
		fftiptr = p_buff->fftbuf + 3*N/4;
	}
	else
	{
		fftn = N / 2;
		fftnlg2m3 = FFTNLG2M3;
		nstep = 1;
		fftrptr = p_buff->fftbuf;
		fftiptr = p_buff->fftbuf + N/2;
	}
	for (m = nstep; m > 0; m--)
	{
		/*	Do first radix-4 pass */
		bfyrptr1 = fftrptr;
		bfyiptr1 = fftiptr;
		bfyrptr2 = fftrptr + (fftn / 4);
		bfyiptr2 = fftiptr + (fftn / 4);
		bfyrptr3 = fftrptr + (fftn / 2);
		bfyiptr3 = fftiptr + (fftn / 2);
		bfyrptr4 = fftrptr + (3 * fftn / 4);
		bfyiptr4 = fftiptr + (3 * fftn / 4);

		for (i = fftn/4; i > 0; i--)
		{
			ar = *bfyrptr1;
			ai = *bfyiptr1;
			br = *bfyrptr2;
			bi = *bfyiptr2;
			cr = *bfyrptr3;
			ci = *bfyiptr3;
			dr = *bfyrptr4;
			di = *bfyiptr4;

			aici = ai + ci;
			brdr = br + dr;
			arcr = ar + cr;
			bidi = bi + di;

			*bfyrptr1++ = DSPrnd(DCTRND, DCTBITS, arcr + brdr);
			*bfyiptr1++ = DSPrnd(DCTRND, DCTBITS, aici + bidi);
			*bfyrptr2++ = DSPrnd(DCTRND, DCTBITS, arcr - brdr);
			*bfyiptr2++ = DSPrnd(DCTRND, DCTBITS, aici - bidi);

			aici = ai - ci;
			brdr = br - dr;
			arcr = ar - cr;
			bidi = bi - di;

			*bfyrptr3++ = DSPrnd(DCTRND, DCTBITS, arcr - bidi);
			*bfyiptr3++ = DSPrnd(DCTRND, DCTBITS, aici + brdr);
			*bfyrptr4++ = DSPrnd(DCTRND, DCTBITS, arcr + bidi);
			*bfyiptr4++ = DSPrnd(DCTRND, DCTBITS, aici - brdr);
		}
		/*	Do all radix-2 passes except first two and last */
		bg = fftn / 8;					/* butterflies per group */
		gp = 4;							/* groups per pass */
		for (k = fftnlg2m3; k > 0; k--)
		{
			bfyrptr1 = fftrptr;
			bfyiptr1 = fftiptr;
			bfyrptr2 = fftrptr + bg;
			bfyiptr2 = fftiptr + bg;
			brxmixptr = brxmix;
			for (j = gp; j > 0; j--)
			{
				cr_t = *brxmixptr++;
				ci_t = *brxmixptr++;		
				for (i = bg; i > 0; i--)
				{
					ar = *bfyrptr1;
					ai = *bfyiptr1;
					br = *bfyrptr2;
					bi = *bfyiptr2;

					rtemp = Vo_Multi32(cr_t,br) - Vo_Multi32(ci_t, bi);
					itemp = Vo_Multi32(ci_t,br) + Vo_Multi32(cr_t, bi);

					*bfyrptr1++ = DSPrnd(DCTRND, DCTBITS, ar - rtemp);
					*bfyiptr1++ = DSPrnd(DCTRND, DCTBITS, ai - itemp);
					*bfyrptr2++ = DSPrnd(DCTRND, DCTBITS, ar + rtemp);
					*bfyiptr2++ = DSPrnd(DCTRND, DCTBITS, ai + itemp);
				}
				bfyrptr1 += bg;
				bfyiptr1 += bg;
				bfyrptr2 += bg;
				bfyiptr2 += bg;
			}
			bg >>= 1;
			gp <<= 1;
		}
		/*	Do last radix-2 pass */
		brxmixptr = brxmix;
		bfyrptr1 = fftrptr;
		bfyiptr1 = fftiptr;
		for (i = fftn / 2; i > 0; i--)
		{
			ar = *bfyrptr1++;
			ai = *bfyiptr1++;
			br = *bfyrptr1--;
			bi = *bfyiptr1--; 
			cr_t = *brxmixptr++;
			ci_t = *brxmixptr++;

			rtemp = Vo_Multi32(cr_t, br) - Vo_Multi32(ci_t, bi);
			itemp = Vo_Multi32(ci_t, br) + Vo_Multi32(cr_t, bi);

			*bfyrptr1++ = DSPrnd(DCTRND, DCTBITS, ar - rtemp);
			*bfyiptr1++ = DSPrnd(DCTRND, DCTBITS, ai - itemp);
			*bfyrptr1++ = DSPrnd(DCTRND, DCTBITS, ar + rtemp);
			*bfyiptr1++ = DSPrnd(DCTRND, DCTBITS, ai + itemp);
		}
		fftrptr = p_buff->fftbuf + N/4;
		fftiptr = p_buff->fftbuf + N/2;
	}
}
#endif

