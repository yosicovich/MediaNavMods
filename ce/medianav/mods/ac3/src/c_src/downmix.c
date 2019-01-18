/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DOWNMIX.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Channel downmixing routine
;***************************************************************************/
#include "ac3d_com.h"
#include "ac3d_fix.h"

#ifdef ARM
//#include <utils/Log.h>
#endif

//#ifdef AC3D_ARM_OPT  

extern DSPshort chantab[NCHANCFG][NPCMCHANS];
void downmix(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	DSPshort            inchan, outchan;
	DSPshort            *chantabptr;   
	int                 chan, count;
	int                 gainrng;
	int                 *tcptr, *dnmixptr, dnmixfac;

	//AC3_BSPK			*p_bstrm = p_decparam->ac3_str;
	AC3_INFO_SI         *ac3_si  = p_decparam->ac3_si;
	AC3_AB_DATA         *p_aublk = p_decparam->ac3_aub;

	/*	Do downmixing */
	inchan = chantab[ac3_si->acmod][p_decparam->channum];
	chantabptr = &chantab[p_decparam->outmod][0];
	gainrng  = p_aublk->appgainrng[p_decparam->channum];

#ifdef ARM
	//LOGD("->%d; %s(); %d\n:", __LINE__, __FUNCTION__, ac3_si->acmod);
	//LOGD("%d, %d, %d\n", p_decparam->channum, p_decparam->outmod, p_decparam->outnchans);
	//LOGD("gainrng = %d, inchan = %d, N/4 = %d \n",gainrng, inchan , N/4);
#endif

	for (chan = 0; chan < p_decparam->outnchans; chan++)
	{
		outchan = *chantabptr++; 
		if(gainrng == 0)
			dnmixfac = DSPrnd(MIXRND, MIXSTOR, p_aublk->dnmixtab[outchan][inchan] << 1);
		else
			dnmixfac = DSPrnd(MIXRND, MIXSTOR, p_aublk->dnmixtab[outchan][inchan] >> (gainrng - 1));

		if (dnmixfac != 0)
		{
			dnmixptr = p_buff->dnmix_buf[chan];
			tcptr = p_aublk->tcbuf;

			if (p_aublk->dnmixbufinu[chan] == 0)
			{
				for (count = N/4; count > 0; count--)
				{
					*dnmixptr++ = DSPrnd(DNMXRND, DNMXBITS, Vo_Multi32((*tcptr++) , dnmixfac));
					*dnmixptr++ = DSPrnd(DNMXRND, DNMXBITS, Vo_Multi32((*tcptr++) , dnmixfac));
					*dnmixptr++ = DSPrnd(DNMXRND, DNMXBITS, Vo_Multi32((*tcptr++) , dnmixfac));
					*dnmixptr++ = DSPrnd(DNMXRND, DNMXBITS, Vo_Multi32((*tcptr++) , dnmixfac));
				}
			}
			else
			{
				for (count = N/4; count > 0; count--)
				{
					*dnmixptr = DSPrnd(DNMXRND, DNMXBITS,(*dnmixptr) + Vo_Multi32((*tcptr++) , dnmixfac));
					dnmixptr++;
					*dnmixptr = DSPrnd(DNMXRND, DNMXBITS,(*dnmixptr) + Vo_Multi32((*tcptr++) , dnmixfac));
					dnmixptr++;
					*dnmixptr = DSPrnd(DNMXRND, DNMXBITS,(*dnmixptr) + Vo_Multi32((*tcptr++) , dnmixfac));
					dnmixptr++;
					*dnmixptr = DSPrnd(DNMXRND, DNMXBITS,(*dnmixptr) + Vo_Multi32((*tcptr++) , dnmixfac));
					dnmixptr++;
				}
			}
			p_aublk->dnmixbufinu[chan] = 1;
		}
	}
}

//#endif

void clr_downmix(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)		   									  
{
	int *dnmixptr;
	int chan, count;
	AC3_AB_DATA         *p_aublk = p_decparam->ac3_aub;

	/*	Clear unused downmix buffers */
	for (chan = 0; chan < p_decparam->outnchans; chan++)
	{
		if (!p_aublk->dnmixbufinu[chan])
		{
			dnmixptr = p_buff->dnmix_buf[chan];
			for (count = 0; count < N; count++)
			{
				*dnmixptr++ = 0;
			}
		}
	}
}

extern DSPshort nfront[NCHANCFG], nrear[NCHANCFG];
extern DSPushort chanetab[NCHANCFG * 2];
extern int cmixtab[4], surmixtab[4];
extern int altmixtab[8];
extern int globalgain[8];
void setup_downmix(DEC_PARAM *p_decparam)
{
	int		  cmixval, surmixval, *exttabptr;
	int		  unity, m3db, m6db, rowsum;
	int		  ltrtcmixval, ltrtsurmixval, lorocmixval, lorosurmixval;
	DSPshort  infront, inrear, outfront, outrear;
	DSPushort inchane, outchane;
	int		  inchan, outchan,downmix_active;
	AC3_AB_DATA         *p_aublk = p_decparam->ac3_aub;
	AC3_INFO_SI			*ac3_si = p_decparam->ac3_si;

#ifdef KCAPABLE
	int   v1_lev, v2_lev, m_lev, v1_pan, v2_pan, m_pan;
#endif
	unity = 0x3fffffff;
	m3db  = 0x2d413ccc;
	m6db  = 0x20000000;

	if (p_decparam->ext_dnmixtab)
	{
		/*****************************************************************************/
		/*                                                                           */
		/*	Use user-specified downmix table                                         */
		/*                                                                           */
		/*****************************************************************************/
		/*	Copy downmix table from external table */
		exttabptr = p_decparam->ext_dnmixtab;
		for (outchan = 0; outchan < NPCMCHANS; outchan++)
		{
			for (inchan = 0; inchan < NPCMCHANS; inchan++)
			{
				p_aublk->dnmixtab[outchan][inchan] = DSPrnd(MIXRND, MIXSTOR, (*exttabptr++)>>1);
			}
		}
	}
	else
	{
		/*****************************************************************************/
		/*                                                                           */
		/*	Use downmix coefficients from bitstream                                  */
		/*                                                                           */
		/*****************************************************************************/
		/***	Initialize downmix parameters	***/
		/*	Clear downmix table */
		for (outchan = 0; outchan < NPCMCHANS; outchan++)
		{
			for (inchan = 0; inchan < NPCMCHANS; inchan++)
			{
				p_aublk->dnmixtab[outchan][inchan] = 0;
			}
		}
		/*	Set up downmix parameters */
		infront = nfront[ac3_si->acmod];						/* number of front input channels */
		inrear = nrear[ac3_si->acmod];							/* number of rear input channels */
		inchane = chanetab[ac3_si->acmod];						/* input channel matrix */
		outfront = nfront[p_decparam->outmod];					/* number of front output channels */
		outrear = nrear[p_decparam->outmod];					/* number of rear output channels */
		outchane = chanetab[p_decparam->outmod];				/* output channel matrix */
		cmixval = cmixtab[ac3_si->cmixlev];						/* center mix coefficient */
		surmixval = surmixtab[ac3_si->surmixlev];				/* surround mix coefficient */
		ltrtcmixval = altmixtab[ac3_si->ltrtcmixlev];			/* Lt/Rt center mix coefficient */
		ltrtsurmixval = altmixtab[ac3_si->ltrtsurmixlev];		/* Lt/Rt surround mix coefficient */
		lorocmixval = altmixtab[ac3_si->lorocmixlev];			/* Lo/Ro center mix coefficient */
		lorosurmixval = altmixtab[ac3_si->lorosurmixlev];		/* Lo/Ro surround mix coefficient */
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for dual mono                                                    */
		/*                                                                           */
		/*****************************************************************************/
		if (ac3_si->acmod == MODE11)
		{
			/***	Mix into 1 output channel	***/
			if (outfront == 1)
			{
				if (p_decparam->dualmod == DUAL_LEFTMONO)
				{
					p_aublk->dnmixtab[CNTR][LEFT] = unity;	/* use left channel */
				}
				else if (p_decparam->dualmod == DUAL_RGHTMONO)
				{
					p_aublk->dnmixtab[CNTR][RGHT] = unity;	/* use right channel */
				}
				else
				{
					p_aublk->dnmixtab[CNTR][LEFT] = m6db;	/* mix both channels into center channel */
					p_aublk->dnmixtab[CNTR][RGHT] = m6db;
				}
			}
			/***	Mix into 2 front output channels	***/
			else if (outfront == 2)
			{
				if (p_decparam->dualmod == DUAL_STEREO)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;	/* use both channels (straight out) */
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
				}
				else if (p_decparam->dualmod == DUAL_LEFTMONO)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = m6db;	/* use left channel */
					p_aublk->dnmixtab[RGHT][LEFT] = m6db;
				}
				else if (p_decparam->dualmod == DUAL_RGHTMONO)
				{
					p_aublk->dnmixtab[LEFT][RGHT] = m6db;	/* use right channel */
					p_aublk->dnmixtab[RGHT][RGHT] = m6db;
				}
				else
				{
					p_aublk->dnmixtab[LEFT][LEFT] = m6db;	/* mix both channels into L and R */
					p_aublk->dnmixtab[LEFT][RGHT] = m6db;
					p_aublk->dnmixtab[RGHT][LEFT] = m6db;
					p_aublk->dnmixtab[RGHT][RGHT] = m6db;
				}
			}
			/***	Mix into 3 front output channels	***/
			else
			{
				if (p_decparam->dualmod == DUAL_STEREO)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;	/* use both channels (straight out) */
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
				}
				else if (p_decparam->dualmod == DUAL_LEFTMONO)
				{
					p_aublk->dnmixtab[CNTR][LEFT] = unity;	/* use left channel */
				}
				else if (p_decparam->dualmod == DUAL_RGHTMONO)
				{
					p_aublk->dnmixtab[CNTR][RGHT] = unity;	/* use right channel */
				}
				else
				{
					p_aublk->dnmixtab[CNTR][LEFT] = m6db;	/* mix both channels into center channel */
					p_aublk->dnmixtab[CNTR][RGHT] = m6db;
				}
			}
		}
#ifdef KCAPABLE
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for karaoke capable decoders                                     */
		/*                                                                           */
		/*****************************************************************************/
		else if (p_decparam->karaokeflag && (p_decparam->kcapableptr != 0))
		{
			v1_lev = p_decparam->kcapableptr[0];		/* load user parameters */
			v1_pan = p_decparam->kcapableptr[1];
			v2_lev = p_decparam->kcapableptr[2];
			v2_pan = p_decparam->kcapableptr[3];
			m_lev = p_decparam->kcapableptr[4];
			m_pan = p_decparam->kcapableptr[5];
			/***	Downmix for 1 output channel	***/
			if (outfront == 1)
			{
				p_aublk->dnmixtab[CNTR][LEFT] = m3db;
				p_aublk->dnmixtab[CNTR][CNTR] = m_lev>>1;
				p_aublk->dnmixtab[CNTR][RGHT] = m3db;
				p_aublk->dnmixtab[CNTR][LSUR] = DSPrnd(MIXRND, MIXSTOR, (v1_lev == 0 ? 0: m3db));
				p_aublk->dnmixtab[CNTR][RSUR] = DSPrnd(MIXRND, MIXSTOR, (v2_lev == 0 ? 0: m3db));
			}
			/***	Donwmix for 2 front output channels	***/
			else if (outfront == 2)
			{
				p_aublk->dnmixtab[LEFT][LEFT] = unity;
				p_aublk->dnmixtab[RGHT][RGHT] = unity;
				p_aublk->dnmixtab[RGHT][CNTR] = DSPrnd(MIXRND, MIXSTOR, SQUARE2DIV2);
				p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, SQUARE2DIV2);
				p_aublk->dnmixtab[RGHT][LSUR] = DSPrnd(MIXRND, MIXSTOR,
					/*v1_lev * cos((v1_pan + unity) * PI/4)*/
					v1_lev != 1?0:(v1_pan == 0?SQUARE2DIV2:0));
				p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR,
					v1_lev != 1?0:(v1_pan == 0?SQUARE2DIV2 : Q31MAX));
				p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR,
					v2_lev != 1?0:(v2_pan == 0?SQUARE2DIV2 : Q31MAX));
				p_aublk->dnmixtab[LEFT][RSUR] = DSPrnd(MIXRND, MIXSTOR,
					v2_lev != 1?0:(v2_pan == 0?SQUARE2DIV2:0));
			}
			/***	Downmix for 3 front output channels	***/
			else if (outfront == 3)
			{
				p_aublk->dnmixtab[LEFT][LEFT] = unity;
				p_aublk->dnmixtab[RGHT][RGHT] = unity;				
				/***	Mix center channel according to m_pan	***/
				p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 0);
				p_aublk->dnmixtab[CNTR][CNTR] = DSPrnd(MIXRND, MIXSTOR, Q31MAX);
				/***	Mix left surround channel according to v1_pan	***/
				p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR,
					v1_lev != 1?0:(v1_pan==0?0:Q31MAX));
				p_aublk->dnmixtab[CNTR][LSUR] = DSPrnd(MIXRND, MIXSTOR,
					/*v1_lev * cos(v1_pan * PI/2)*/
					v1_lev != 1?0:(v1_pan==0?Q31MAX:0));

				/***	Mix right surround channel according to v2_pan	***/
				if (v2_pan >= 0.0)
				{
					p_aublk->dnmixtab[LEFT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 0);
					p_aublk->dnmixtab[CNTR][RSUR] = DSPrnd(MIXRND, MIXSTOR,
						/*v2_lev * cos(v2_pan * PI/2)*/
						v2_lev == 0?0:Q31MAX);
				}
				else	/* -90 < V2_pan < 0 */
				{
					p_aublk->dnmixtab[CNTR][RSUR] = DSPrnd(MIXRND, MIXSTOR, 0);
					p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR, Q31MAX);
				}
			}
		}
#endif
#ifdef KAWARE
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for karaoke aware decoders                                       */
		/*                                                                           */
		/*****************************************************************************/
		else if (p_decparam->karaokeflag)
		{
			/***	Downmix into mono channel	***/
			if (outfront == 1)
			{
				p_aublk->dnmixtab[CNTR][LEFT] = m3db;
				p_aublk->dnmixtab[CNTR][RGHT] = m3db;
				p_aublk->dnmixtab[CNTR][CNTR] =
					DSPrnd(MIXRND, MIXSTOR, (Vo_Multi32(cmixval, m3db))<<1);
				p_aublk->dnmixtab[CNTR][LSUR] =
					DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
				p_aublk->dnmixtab[CNTR][RSUR] =
					DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
			}
			else if (outfront == 2)
			{
				/***	Downmix to surround compatible stereo	***/
				if (p_decparam->outmod == 0)
				{
					if (ac3_si->xbsi1e) 
					{
						/***	Here, if Annex D downmix stereo coefficients exist	***/
						p_aublk->dnmixtab[LEFT][LEFT] = globalgain[ac3_si->ltrtcmixlev]>>1;
						p_aublk->dnmixtab[RGHT][RGHT] = globalgain[ac3_si->ltrtcmixlev]>>1;
						p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(ltrtcmixval, globalgain[ac3_si->ltrtcmixlev]));
						p_aublk->dnmixtab[RGHT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(ltrtcmixval, globalgain[ac3_si->ltrtcmixlev]));
						if (inrear == 1)
						{
							p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
								-Vo_Multi32(Vo_Multi32_31(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]), m3db));
							p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(Vo_Multi32_31(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]), m3db));
						}
						else if (inrear == 2)
						{
							p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR, 
								-Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
							p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
						}
					}
					else
						/***	Here, for regular downmix coefficients	***/
					{
						p_aublk->dnmixtab[LEFT][LEFT] = unity;
						p_aublk->dnmixtab[RGHT][RGHT] = unity;
						p_aublk->dnmixtab[LEFT][CNTR] = cmixval >> 1;
						p_aublk->dnmixtab[RGHT][CNTR] = cmixval >> 1;
						if (inrear == 1)
						{
							p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, -Vo_Multi32(surmixval, m3db));
							p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
						}
						else if (inrear == 2)
						{
							p_aublk->dnmixtab[LEFT][LSUR] = -surmixval >> 1;
							p_aublk->dnmixtab[RGHT][RSUR] = surmixval >> 1;
						}
					}
				}
				/***	Downmix to stereo	***/
				else if (p_decparam->outmod == MODE20)
				{
					if (ac3_si->xbsi1e)
					{
						/***	Here, if Annex D downmix stereo coefficients exist	***/
						p_aublk->dnmixtab[LEFT][LEFT] = globalgain[ac3_si->lorocmixlev] >>1;
						p_aublk->dnmixtab[RGHT][RGHT] = globalgain[ac3_si->lorocmixlev] >>1;
						p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(lorocmixval, globalgain[ac3_si->lorocmixlev]));
						p_aublk->dnmixtab[RGHT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(lorocmixval, globalgain[ac3_si->lorocmixlev]));
						if (inrear == 1)
						{
							p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(Vo_Multi32_31(lorosurmixval, globalgain[ac3_si->lorocmixlev]), m3db));
							p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(Vo_Multi32_31(lorosurmixval, globalgain[ac3_si->lorocmixlev]), m3db));
						}
						else if (inrear == 2)
						{
							p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(lorosurmixval, globalgain[ac3_si->lorocmixlev]));
							p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 
								Vo_Multi32(lorosurmixval, globalgain[ac3_si->lorocmixlev]));
						}
					}
					else
					{
						/***	Here for regular downmix coefficients	***/
						p_aublk->dnmixtab[LEFT][LEFT] = unity;
						p_aublk->dnmixtab[RGHT][RGHT] = unity;
						p_aublk->dnmixtab[LEFT][CNTR] = cmixval >> 1;
						p_aublk->dnmixtab[RGHT][CNTR] = cmixval >> 1;
						if (inrear == 1)
						{
							p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
							p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
						}
						else if (inrear == 2)
						{
							p_aublk->dnmixtab[LEFT][LSUR] = surmixval >> 1;
							p_aublk->dnmixtab[RGHT][RSUR] = surmixval >> 1;
						}
					}
				}
			}
			/***	Downmix to 3 front channels	***/
			else if (outfront == 3)
			{
				p_aublk->dnmixtab[LEFT][LEFT] = unity;
				p_aublk->dnmixtab[CNTR][CNTR] = unity;
				p_aublk->dnmixtab[RGHT][RGHT] = unity;
				if (inrear == 1)
				{
					p_aublk->dnmixtab[CNTR][MSUR] = surmixval >>1;
				}
				else if (inrear == 2)
				{
					p_aublk->dnmixtab[LEFT][LSUR] = surmixval >>1;
					p_aublk->dnmixtab[RGHT][RSUR] = surmixval >>1;
				}
			}
		}
#endif
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for Dolby Surround compatible stereo (outmod == 0)               */
		/*                                                                           */
		/*****************************************************************************/
		else if (p_decparam->outmod == 0)
		{
			/***	Downmix for outmod == 0 and infront == 1	***/
			if (infront == 1)
			{
				p_aublk->dnmixtab[LEFT][CNTR] = m3db;
				p_aublk->dnmixtab[RGHT][CNTR] = m3db;
			}

			/***	Downmix for outmod == 0 and infront == 2	***/
			else if (infront == 2)
			{
				p_aublk->dnmixtab[LEFT][LEFT] = unity;
				p_aublk->dnmixtab[RGHT][RGHT] = unity;

				if (ac3_si->xbsi1e)
				{				
					/***	Downmix rear channels into surround compatible stereo when Annex D coefficients are present	***/
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = -ltrtsurmixval;
						p_aublk->dnmixtab[RGHT][MSUR] = ltrtsurmixval;
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = -ltrtsurmixval;
						p_aublk->dnmixtab[LEFT][RSUR] = -ltrtsurmixval;
						p_aublk->dnmixtab[RGHT][LSUR] = ltrtsurmixval;
						p_aublk->dnmixtab[RGHT][RSUR] = ltrtsurmixval;
					}
				}
				else
				{	
					/***	Downmix rear channels into surround compatible stereo when Annex D coefficients are NOT present	***/
					/***	Fixed downmix level of -3dB is used here according to ATSC spec.	***/

					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = -m3db;
						p_aublk->dnmixtab[RGHT][MSUR] = m3db;
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = -m3db;
						p_aublk->dnmixtab[LEFT][RSUR] = -m3db;
						p_aublk->dnmixtab[RGHT][LSUR] = m3db;
						p_aublk->dnmixtab[RGHT][RSUR] = m3db;
					}
				}
			}	/* if (infront == 2) */
			/***	Downmix for outmod == 0 and infront == 3	***/
			else if (infront == 3)
			{
				if (ac3_si->xbsi1e)
				{
					/***	Downmix 3/0-3/2 channels into surround compatible stereo with global gain when Annex D coefficients are present	***/
					p_aublk->dnmixtab[LEFT][LEFT] = globalgain[ac3_si->ltrtcmixlev] >>1;
					p_aublk->dnmixtab[RGHT][RGHT] = globalgain[ac3_si->ltrtcmixlev] >>1;
					p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
						Vo_Multi32(ltrtcmixval, globalgain[ac3_si->ltrtcmixlev]));
					p_aublk->dnmixtab[RGHT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
						Vo_Multi32(ltrtcmixval, globalgain[ac3_si->ltrtcmixlev]));
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
							-Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
						p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR, 
							-Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
						p_aublk->dnmixtab[LEFT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 
							-Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
						p_aublk->dnmixtab[RGHT][LSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
						p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(ltrtsurmixval, globalgain[ac3_si->ltrtcmixlev]));
					}
				}
				else	/* if (xbsi1e) */
				{
					/***	Downmix 3/0-3/2 channels into surround compatible stereo when Annex D coefficients are NOT present	***/
					/***	Fixed downmix coeffs are used for ltrt legacy downmixes according to the ATSC spec.	***/
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
					p_aublk->dnmixtab[LEFT][CNTR] = m3db;
					p_aublk->dnmixtab[RGHT][CNTR] = m3db;
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = -m3db;
						p_aublk->dnmixtab[RGHT][MSUR] = m3db;
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = -m3db;
						p_aublk->dnmixtab[LEFT][RSUR] = -m3db;
						p_aublk->dnmixtab[RGHT][LSUR] = m3db;
						p_aublk->dnmixtab[RGHT][RSUR] = m3db;
					}
				}	/* if (xbsi1e) */
			}	/* if (infront == 3) */
		}	/* if (outmod == 0) */
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for mono (outmod = MODE10)                                       */
		/*                                                                           */
		/*****************************************************************************/
		else if (p_decparam->outmod == MODE10)
		{
			if (infront == 1)
			{
				p_aublk->dnmixtab[CNTR][CNTR] = unity;
			}
			else if (infront == 2)
			{
				p_aublk->dnmixtab[CNTR][LEFT] = m6db;
				p_aublk->dnmixtab[CNTR][RGHT] = m6db;
			}
			else if (infront == 3)
			{
				p_aublk->dnmixtab[CNTR][LEFT] = m6db;
				p_aublk->dnmixtab[CNTR][RGHT] = m6db;
				p_aublk->dnmixtab[CNTR][CNTR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(cmixval, m6db));
			}
			if (inrear == 1)
			{
				p_aublk->dnmixtab[CNTR][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m6db));
			}
			else if (inrear == 2)
			{
				p_aublk->dnmixtab[CNTR][LSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m6db));
				p_aublk->dnmixtab[CNTR][RSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m6db));
			}
		}	/* if (outmod == MODE10) */
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix for stereo (Lo/Ro)                                               */
		/*                                                                           */
		/*****************************************************************************/
		else if (p_decparam->outmod == MODE20)
		{
			/***	Downmix for outmod == MODE20 and infront == 1	***/
			if (infront == 1)
			{
				p_aublk->dnmixtab[LEFT][CNTR] = m3db;
				p_aublk->dnmixtab[RGHT][CNTR] = m3db;
			}
			/***	Downmix for outmod == MODE20 and infront == 2	***/
			else if (infront == 2)
			{
				p_aublk->dnmixtab[LEFT][LEFT] = unity;
				p_aublk->dnmixtab[RGHT][RGHT] = unity;
				if (ac3_si->xbsi1e)
				{	
					/***	Downmix rear channels into stereo if Annex D coefficients are present	***/
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32_31(lorosurmixval, m3db));
						p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32_31(lorosurmixval, m3db));
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = lorosurmixval;
						p_aublk->dnmixtab[RGHT][RSUR] = lorosurmixval;
					}
				}
				else	/* if (xbsi1e) */
				{	
					/***	Downmix rear channels into stereo if Annex D coefficients are NOT present	***/
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
						p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = surmixval >>1;
						p_aublk->dnmixtab[RGHT][RSUR] = surmixval >>1;
					}
				}
			}	/* if (infront == 2) */
			/***	Downmix for outmod == MODE20 and infront == 3	***/
			else if (infront == 3)
			{
				if (ac3_si->xbsi1e)
				{
					/***	Donwmix 3/0-3/2 channels into stereo with global gain	***/
					p_aublk->dnmixtab[LEFT][LEFT] = globalgain[ac3_si->lorocmixlev] >>1;
					p_aublk->dnmixtab[RGHT][RGHT] = globalgain[ac3_si->lorocmixlev] >>1;
					p_aublk->dnmixtab[LEFT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
						Vo_Multi32(lorocmixval, globalgain[ac3_si->lorocmixlev]));
					p_aublk->dnmixtab[RGHT][CNTR] = DSPrnd(MIXRND, MIXSTOR, 
						Vo_Multi32(lorocmixval, globalgain[ac3_si->lorocmixlev]));
					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(Vo_Multi32_31(lorosurmixval, globalgain[ac3_si->lorocmixlev]), m3db));
						p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(Vo_Multi32_31(lorosurmixval, globalgain[ac3_si->lorocmixlev]), m3db));
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(lorosurmixval, globalgain[ac3_si->lorocmixlev]));
						p_aublk->dnmixtab[RGHT][RSUR] = DSPrnd(MIXRND, MIXSTOR, 
							Vo_Multi32(lorosurmixval, globalgain[ac3_si->lorocmixlev]));
					}
				}
				else
					/***	Downmix 3/0-3/2 channels into stereo when Annex D parameters are disabled	***/
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
					p_aublk->dnmixtab[LEFT][CNTR] = cmixval>>1;
					p_aublk->dnmixtab[RGHT][CNTR] = cmixval>>1;

					if (inrear == 1)
					{
						p_aublk->dnmixtab[LEFT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
						p_aublk->dnmixtab[RGHT][MSUR] = DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
					}
					else if (inrear == 2)
					{
						p_aublk->dnmixtab[LEFT][LSUR] = surmixval>>1;
						p_aublk->dnmixtab[RGHT][RSUR] = surmixval>>1;
					}
				}	/* if (xbsi1e) */
			}	/* if (infront == 3) */
		}	/* if (outmod == MODE20) */
		/*****************************************************************************/
		/*                                                                           */
		/*	Downmix all other cases                                                  */
		/*                                                                           */
		/*****************************************************************************/
		else
		{
			/***	Downmix for 2 front output channels	***/
			if (outfront == 2)
			{
				if (infront == 1)
				{
					p_aublk->dnmixtab[LEFT][CNTR] = m3db;
					p_aublk->dnmixtab[RGHT][CNTR] = m3db;
				}
				else if (infront == 2)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
				}
				else if (infront == 3)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
					p_aublk->dnmixtab[LEFT][CNTR] = cmixval>>1;
					p_aublk->dnmixtab[RGHT][CNTR] = cmixval>>1;
				}
			}
			/***	Downmix for 3 front output channels	***/
			else if (outfront == 3)
			{
				if (infront == 1)
				{
					p_aublk->dnmixtab[CNTR][CNTR] = unity;
				}
				else if (infront == 2)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
				}
				else if (infront == 3)
				{
					p_aublk->dnmixtab[LEFT][LEFT] = unity;
					p_aublk->dnmixtab[RGHT][RGHT] = unity;
					p_aublk->dnmixtab[CNTR][CNTR] = unity;
				}
			}
			/***	Downmix rear input channels into front channels	***/
			if (outrear == 0)
			{
				if (inrear == 1)
				{
					p_aublk->dnmixtab[LEFT][MSUR] =
						DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
					p_aublk->dnmixtab[RGHT][MSUR] =
						DSPrnd(MIXRND, MIXSTOR, Vo_Multi32(surmixval, m3db));
				}
				else if (inrear == 2)
				{
					p_aublk->dnmixtab[LEFT][LSUR] = surmixval>>1;
					p_aublk->dnmixtab[RGHT][RSUR] = surmixval>>1;
				}
			}
			/***	Downmix rear channel(s) into mono surround	***/
			else if (outrear == 1)
			{
				if (inrear == 1)
				{
					p_aublk->dnmixtab[MSUR][MSUR] = unity;
				}
				else if (inrear == 2)
				{
					p_aublk->dnmixtab[MSUR][LSUR] = surmixval>>1;
					p_aublk->dnmixtab[MSUR][RSUR] = surmixval>>1;
				}
			}
			/***	Donwmix rear channel(s) into stereo surrounds	***/
			else if (outrear == 2)
			{
				if (inrear == 1)
				{
					p_aublk->dnmixtab[LSUR][MSUR] = m3db;
					p_aublk->dnmixtab[RSUR][MSUR] = m3db;
				}
				else if (inrear == 2)
				{
					p_aublk->dnmixtab[LSUR][LSUR] = unity;
					p_aublk->dnmixtab[RSUR][RSUR] = unity;
				}
			}
		}
		/*****************************************************************************/
		/*                                                                           */
		/*	Enable LFE channel                                                       */
		/*                                                                           */
		/*****************************************************************************/
		if (ac3_si->lfeon && p_decparam->outlfe)
		{
			p_aublk->dnmixtab[LFE][LFE] = unity;
		}
	}	/* if (outmod) */
	/*	Determine if downmixing is active */
	downmix_active = 0;
	for (outchan = 0; outchan < NPCMCHANS; outchan++)
	{
		rowsum = 0;
		for (inchan = 0; inchan < NPCMCHANS; inchan++)
		{
			rowsum += (DSPabs(p_aublk->dnmixtab[outchan][inchan])>>1);
		}
		if (rowsum > 0x20000000L)
		{
			downmix_active = 1;
		}
	}
#ifdef KAWARE
	/*	If karaoke mode, always enable downmix active */
	if (p_decparam->karaokeflag)
	{
		downmix_active = 1;
	}
#endif
	/*	Set up digital dialog normalization flag */
	if (p_decparam->compmod == COMP_CUSTOM_A)
	{
		p_decparam->digdialnorm = 0;
	}
	else
	{
		p_decparam->digdialnorm = 1;
	}
	/*	Set up compression factor for clipping prevention */
	if (p_decparam->compmod == COMP_RF)				/* RF remod mode */
	{
		if (ac3_si->compre)
		{
			if (ac3_si->compr)
			{
				p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((ac3_si->compr + PLUS11DB_INT)<<12));
			}
			else
			{
				p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((p_aublk->dynrng + PLUS11DB_INT)<<12));
			}
		}
		else if (downmix_active)
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((p_aublk->dynrng + MINUS11DB_INT)<<12));
		}
		else
		{
			p_aublk->compfact = DSPs2f(p_aublk->dynrng<<12);
		}

		if (ac3_si->acmod == MODE11)
		{
			if (ac3_si->compr2e)
			{
				if (ac3_si->compr2)
				{
					p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((ac3_si->compr2 + PLUS11DB_INT)<<12));
				}
				else
				{
					p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((p_aublk->dynrng2 + PLUS11DB_INT)<<12));
				}
			}
			else if (downmix_active)
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, DSPs2f((p_aublk->dynrng2 + MINUS11DB_INT)<<12));
			}
			else
			{
				p_aublk->compfact2 = DSPs2f(p_aublk->dynrng2<<12);
			}
		}
	}
	else if (p_decparam->compmod == COMP_LINE)			/* line out mode */
	{
		if (p_aublk->dynrng > 0)
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng<<12, p_decparam->dynscalelow));
		}
		else if (downmix_active)
		{
			p_aublk->compfact = DSPs2f(p_aublk->dynrng<<12);
		}
		else
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng<<12, p_decparam->dynscalehigh));
		}

		if (ac3_si->acmod == MODE11)
		{
			if (p_aublk->dynrng2 > 0)
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng2<<12, p_decparam->dynscalelow));
			}
			else if (downmix_active)
			{
				p_aublk->compfact2 = DSPs2f(p_aublk->dynrng2<<12);
			}
			else
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng2<<12, p_decparam->dynscalehigh));
			}
		}
	}
	else								/* custom mode */
	{
		if (p_aublk->dynrng > 0)
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng<<12, p_decparam->dynscalelow));
		}
		else
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng<<12, p_decparam->dynscalehigh));
		}
		if (downmix_active)
		{
			p_aublk->compfact = DSPrnd(DYNSCRND, DYNSCSTOR, p_aublk->compfact + (int)(MINUS11DB_INT<<12));
		}

		if (ac3_si->acmod == MODE11)
		{
			if (p_aublk->dynrng2 > 0)
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng2<<12, p_decparam->dynscalelow));
			}
			else
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi32_28((int)p_aublk->dynrng2<<12, p_decparam->dynscalehigh));
			}
			if (downmix_active)
			{
				p_aublk->compfact2 = DSPrnd(DYNSCRND, DYNSCSTOR, p_aublk->compfact2 + (int)(MINUS11DB_INT<<12));
			}
		}
	}
}
