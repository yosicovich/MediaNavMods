/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/WINDOW_D.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Window overlap-add for oddly-stacked TDAC
;***************************************************************************/

#include "ac3d_com.h"
#include "ac3d_fix.h"

/* const table */
extern DSPshort chantab[NCHANCFG][NPCMCHANS];
extern int  Lwindow[N];
extern int  Lwindow_Interleave[N];

#ifndef AC3D_ARM_OPT
void window_d(DSPshort chan, DECEXEC *p_decexec)
{
	int         count;
	BUFF_PARAM  *p_buff;
	DEC_PARAM   *p_decparam;
	DSPshort    outchan, offset;

	int			*dnmixptr1, *dnmixptr2;
	int			*delayptr1, *delayptr2;
	int  		*windptr1, *windptr2;
	int			*pcmptr1, *pcmptr2;
	int			samp;

	p_buff  = (BUFF_PARAM *)p_decexec->ac3_buff;
	p_decparam = (DEC_PARAM *)p_decexec->decparam;
	/*	Window downmixed data */

	dnmixptr1 = p_buff->dnmix_buf[chan] + (3 * N / 4);
	dnmixptr2 = p_buff->dnmix_buf[chan] + (N / 4 - 1);
	delayptr1 = p_buff->delay_buf[chan];
	delayptr2 = p_buff->delay_buf[chan] + (N / 2 - 1);
	windptr1 = Lwindow_Interleave;
	windptr2 = Lwindow_Interleave + 1;
	pcmptr1 = p_buff->fftbuf;
	pcmptr2 = p_buff->fftbuf + (N - 1);

	for (count = N/4; count >0; count--)
	{
		samp = (- Vo_Multi32((*windptr2), (*delayptr1)) - Vo_Multi32((*windptr1), (*dnmixptr1)))<<1;
		//*pcmptr1++ = DSPrnd(WINDRND, WINDBITS, Vo_DSPlimit(samp));
		*pcmptr1++ = samp;

		samp = (- Vo_Multi32((*windptr1), (*delayptr1++)) + Vo_Multi32((*windptr2), (*dnmixptr1++)))<<1;
		//*pcmptr2-- = DSPrnd(WINDRND, WINDBITS, Vo_DSPlimit(samp));
		*pcmptr2-- = samp;
		windptr1 += 2;
		windptr2 += 2;


		samp = (Vo_Multi32((*windptr2), (*delayptr2)) + Vo_Multi32((*windptr1), (*dnmixptr2)))<<1;
		//*pcmptr1++ = DSPrnd(WINDRND, WINDBITS, Vo_DSPlimit(samp));
		*pcmptr1++ = samp;

		samp = (Vo_Multi32((*windptr1), (*delayptr2--)) - Vo_Multi32((*windptr2), (*dnmixptr2--)))<<1;
		//*pcmptr2-- = DSPrnd(WINDRND, WINDBITS, Vo_DSPlimit(samp));
        *pcmptr2-- = samp;
		windptr1 += 2;
		windptr2 += 2;


	}
	/*	Update delay buffers */
	pcmptr1 = p_buff->dnmix_buf[chan] + (N / 4);
	pcmptr2 = p_buff->delay_buf[chan];
	for (count = N/8; count > 0; count--)
	{
		*pcmptr2++ = *pcmptr1++;
		*pcmptr2++ = *pcmptr1++;
		*pcmptr2++ = *pcmptr1++;
		*pcmptr2++ = *pcmptr1++;
	}

	/*	Copy PCM samples to output buffer */
	outchan = chantab[p_decparam->outmod][chan];
	pcmptr1 = p_buff->fftbuf;
	pcmptr2 = p_decparam->pcmbufptr[outchan];
	offset = 6; //p_decparam->pcmbufoff[outchan];
	for (count = N; count >0; count--)
	{
		*pcmptr2 = *pcmptr1++;
		pcmptr2 += offset;
	}
}
#endif 

