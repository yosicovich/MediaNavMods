/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/MANTS_D.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 mantissa unpacking routine
;***************************************************************************/

#include "ac3d_com.h"
#include "ac3d_fix.h"
/* const table */
extern DSPshort slowdec[4], fastdec[4], slowgain[4], fastgain[8];
extern DSPshort dbpbtab[4], floortab[8], cplbndtab[16];
extern int phscortab[PHSTABSZ];
extern DSPshort phsscltab_chan[NCHANCFG - NUCPLCHANCFG][NCHANCFG - 1];
extern int phsscltab[NCHANCFG - NUCPLCHANCFG][NCHANCFG - 1];
extern int phsscltab_sml0[NCHANCFG - 4][NCHANCFG - 1];
extern DSPshort chanmask[NPCMCHANS - 1];

int mants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int    chan , temp_status = 0;
	//AC3_BSPK         *p_bstrm = p_decparam->ac3_str;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	AC3_INFO_SI      *ac3_si  = p_decparam->ac3_si;

	/*	Init control variables */
	p_aublk->count3 = 0;
	p_aublk->count5 = 0;
	p_aublk->count11 = 0;

#ifdef HALFRATE
	p_aublk->sdecay = slowdec[p_aublk->sdcycod] >> p_decparam->halfratecod;
	p_aublk->fdecay = fastdec[p_aublk->fdcycod] >> p_decparam->halfratecod;
#else
	p_aublk->sdecay = slowdec[p_aublk->sdcycod];
	p_aublk->fdecay = fastdec[p_aublk->fdcycod];
#endif
	p_aublk->sgain = slowgain[p_aublk->sgaincod];
	p_aublk->dbknee = dbpbtab[p_aublk->dbpbcod];
	p_aublk->floorval = floortab[p_aublk->floorcod];

	/*	Set up min SNR offset flag */
	p_aublk->minsnrflg = p_aublk->csnroffst;
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		p_aublk->minsnrflg |= p_aublk->fsnroffst[chan];
	}
	if (p_aublk->cplinu)
	{
		p_aublk->minsnrflg |= p_aublk->cplfsnroffst;
	}
	if (ac3_si->lfeon)
	{
		p_aublk->minsnrflg |= p_aublk->lfefsnroffst;
	}

	/*	Unpack chan mantissas */
	if (p_aublk->rematinu)
	{
		p_decparam->channum = 0;
		p_aublk->tcbuf = p_buff->tc1;
		temp_status = chmants_d(p_decparam, p_buff);
		if(temp_status == 1)
			return 1;
		p_decparam->channum = 1;
		p_aublk->tcbuf = p_buff->tc2;

		temp_status = chmants_d(p_decparam, p_buff);
		if(temp_status == 1)
			return 1;

		matrix_d(p_aublk, p_buff);
		p_decparam->channum = 0;
		p_aublk->tcbuf = p_buff->tc1;
		p_decparam->bswitch = p_aublk->blksw[0];

		idctsc(p_decparam, p_buff);
		cifft(p_decparam, p_buff);
		idctsc2(p_decparam, p_buff);
		downmix(p_decparam, p_buff);

		p_decparam->channum = 1;
		p_aublk->tcbuf = p_buff->tc2;
		p_decparam->bswitch = p_aublk->blksw[1];
		idctsc(p_decparam, p_buff);
		cifft(p_decparam, p_buff);
		idctsc2(p_decparam, p_buff);
		downmix(p_decparam, p_buff);
	}
	else
	{
		for (p_decparam->channum = 0; p_decparam->channum < ac3_si->nfchans; p_decparam->channum++)
		{
			p_aublk->tcbuf =p_buff->tc1;
			temp_status = chmants_d(p_decparam, p_buff);
			if(temp_status == 1)
				return 1;
			p_decparam->bswitch = p_aublk->blksw[p_decparam->channum];
			idctsc(p_decparam, p_buff);
			cifft(p_decparam, p_buff);
			idctsc2(p_decparam, p_buff);
			downmix(p_decparam, p_buff);
		}
	}
	/*	Unpack lfe chan mantissas */
	if (ac3_si->lfeon)
	{
		p_decparam->channum = ac3_si->nfchans;
		p_aublk->tcbuf = p_buff->tc1;
		temp_status = lfemants_d(p_decparam, p_buff);
		if(temp_status == 1)
			return 1;
		p_decparam->bswitch = 0;
		idctsc(p_decparam, p_buff);
		cifft(p_decparam, p_buff);
		idctsc2(p_decparam, p_buff);
		downmix(p_decparam, p_buff);
	}
	return 0;
}

int chmants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int bin, temp_status = 0;
	AC3_BSPK         *p_bstrm = p_decparam->ac3_str;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	AC3_INFO_SI      *ac3_si  = p_decparam->ac3_si;

	/*	Set up gain values */
	if (p_aublk->rematinu)
	{
		p_aublk->appgainrng[p_decparam->channum] = DSPmin(p_aublk->gainrng[0], p_aublk->gainrng[1]);
	}
	else
	{
		p_aublk->appgainrng[p_decparam->channum] = p_aublk->gainrng[p_decparam->channum];
	}

	if ((ac3_si->acmod == MODE11) && (p_decparam->channum == 1))
	{
		calc_gain(p_aublk->compfact2, ac3_si->dialnorm2, p_decparam);
	}
	else
	{
		calc_gain(p_aublk->compfact, ac3_si->dialnorm, p_decparam);
	}
	/*	Set up control variables */
	p_aublk->mantindex = 0;
	p_aublk->bandindex = 0;
	p_aublk->lastmant = p_aublk->endmant[p_decparam->channum];
	p_aublk->fgain = fastgain[p_aublk->fgaincod[p_decparam->channum]];
	p_aublk->snroff = (DSPshort)(((p_aublk->csnroffst - 15) << 6) + (p_aublk->fsnroffst[p_decparam->channum] << 2));
	p_aublk->dith = p_aublk->dithflag[p_decparam->channum];
	p_aublk->cplflag = 0;
	/*	Unpack exponents */
#ifdef MIPOPT
	p_buff->expbuf = p_buff->exps[p_decparam->channum];
	if (p_aublk->chexpstr[p_decparam->channum] != REUSE)
	{
		unpexps(&p_aublk->pkexps[p_decparam->channum], p_decparam, p_buff);
	}
#else
	unpexps(&p_aublk->pkexps[p_decparam->channum], p_decparam, p_buff);
#endif
	/*	Expand delta bit allocation */	
	temp_status = expand_delta(p_decparam, p_buff);
	if(temp_status == 1)
		return 1;
	/*	Unpack mantissas */
#ifdef MIPOPT
	p_buff->bapbuf = p_buff->bap[p_decparam->channum];
	if ((p_aublk->chexpstr[p_decparam->channum] != REUSE) || (p_aublk->baie) || (p_aublk->snroffste)
		|| (p_aublk->deltbaie && (p_aublk->deltbae[p_decparam->channum] != REUSE)))
	{
		p_buff->newbitalloc = 1;
	}
	else
	{
		p_buff->newbitalloc = 0;
	}
#endif
	unpmants(p_decparam, p_buff);
	/*	Check if necessary to unpack coupled mantissas */
	if (p_aublk->chincpl[p_decparam->channum])
	{
		/*	Save full bandwidth channel state */
		if (p_decparam->channum == p_aublk->cplchan)
		{
			p_aublk->cplpkptr = p_bstrm->pkptr;
			p_aublk->cplpkbitptr = p_bstrm->pkbitptr;
			p_aublk->cplpkdata = p_bstrm->pkdata;
			p_aublk->cplcount3 = p_aublk->count3;
			p_aublk->cplcount5 = p_aublk->count5;
			p_aublk->cplcount11 = p_aublk->count11;
			p_aublk->cplmant3 = p_aublk->mant3;
			p_aublk->cplmant5 = p_aublk->mant5;
			p_aublk->cplmant11 = p_aublk->mant11;
		}
		else
		{
			p_aublk->chpkptr = p_bstrm->pkptr;
			p_aublk->chpkbitptr = p_bstrm->pkbitptr;
			p_aublk->chpkdata = p_bstrm->pkdata;
			p_aublk->chcount3 = p_aublk->count3;
			p_aublk->chcount5 = p_aublk->count5;
			p_aublk->chcount11  = p_aublk->count11;
			p_aublk->chmant3 = p_aublk->mant3;
			p_aublk->chmant5 = p_aublk->mant5;
			p_aublk->chmant11 =p_aublk->mant11;

			p_bstrm->pkptr = p_aublk->cplpkptr;
			p_bstrm->pkbitptr = p_aublk->cplpkbitptr;
			p_bstrm->pkdata = p_aublk->cplpkdata;
			p_aublk->count3 = p_aublk->cplcount3;
			p_aublk->count5 = p_aublk->cplcount5;
			p_aublk->count11 = p_aublk->cplcount11;
			p_aublk->mant3 = p_aublk->cplmant3;
			p_aublk->mant5 = p_aublk->cplmant5;
			p_aublk->mant11 = p_aublk->cplmant11;
		}
		/*	Set up control variables */
		p_aublk->mantindex = p_aublk->cplstrtmant;
		p_aublk->bandindex = cplbndtab[p_aublk->cplbegf];
		p_aublk->lastmant = p_aublk->cplendmant;
		p_aublk->fgain = fastgain[p_aublk->cplfgaincod];
		p_aublk->snroff = (DSPshort)(((p_aublk->csnroffst - 15) << 6) + (p_aublk->cplfsnroffst << 2));
		p_aublk->dith = p_aublk->dithflag[p_decparam->channum];
		p_aublk->cplflag = 1;
		p_aublk->cplcoexpptr = &p_aublk->cplcoexp[p_decparam->channum][p_aublk->cplbegf];
		p_aublk->cplcoptr = &p_aublk->cplco[p_decparam->channum][p_aublk->cplbegf];
		p_aublk->cplcnt = 0;
		p_aublk->fleak = p_aublk->cplfleak;
		p_aublk->sleak = p_aublk->cplsleak;
		p_aublk->gainexp += 3;

		temp_status = scale_cplco(p_decparam);	/* overwrite cplcoptr, cplcoexpptr with ptrs to scaled arrays */
		if(temp_status == 1)
			return 1;

		/*	Unpack exponents */
#ifdef MIPOPT
		p_buff->expbuf = p_buff->cplexps;
		if (p_aublk->cplexpstr != REUSE)
		{
			unpexps(&p_aublk->cplpkexps, p_decparam, p_buff);
		}
#else
		unpexps(&p_aublk->cplpkexps, p_decparam, p_buff);
#endif
		/*	Expand delta bit allocation */
		temp_status = expand_delta_1(p_decparam, p_buff);
		if(temp_status == 1)
			return 1;
		/*	Unpack mantissas */

#ifdef MIPOPT
		p_buff->bapbuf = p_buff->cplbap;
		if ((p_aublk->cplexpstr != REUSE) || (p_aublk->baie) || (p_aublk->snroffste) || (p_aublk->cplleake)
			|| (p_aublk->deltbaie && (p_aublk->cpldeltbae != REUSE)))
		{
			p_buff->newbitalloc = 1;
		}
		else
		{
			p_buff->newbitalloc = 0;
		}
#endif
		unpmants(p_decparam, p_buff);
		/*	Restore full bandwidth channel state */
		if (p_decparam->channum != p_aublk->cplchan)
		{
			p_bstrm->pkptr = p_aublk->chpkptr;
			p_bstrm->pkbitptr = p_aublk->chpkbitptr;
			p_bstrm->pkdata = p_aublk->chpkdata;
			p_aublk->count3 = p_aublk->chcount3;
			p_aublk->count5 = p_aublk->chcount5;
			p_aublk->count11 = p_aublk->chcount11;
			p_aublk->mant3 = p_aublk->chmant3;
			p_aublk->mant5 = p_aublk->chmant5;
			p_aublk->mant11 = p_aublk->chmant11;
		}
	}

	/*	Zero out unused transform coefficients */
	for (bin = p_aublk->lastmant; bin < N; bin++)
	{
		p_aublk->tcbuf[bin] = 0;
	}
	return 0;
}

int lfemants_d(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int bin , temp_status = 0;
	//AC3_BSPK         *p_bstrm = p_decparam->ac3_str;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	AC3_INFO_SI      *ac3_si  = p_decparam->ac3_si;

	/*	Set up gain values */
	p_aublk->appgainrng[p_decparam->channum] = 0;
	calc_gain(p_aublk->compfact, ac3_si->dialnorm, p_decparam);
	/*	Set up control variables */
	p_aublk->mantindex = 0;
	p_aublk->bandindex = 0;
	p_aublk->lastmant = NLFEMANTS;
	p_aublk->fgain = fastgain[p_aublk->lfefgaincod];
	p_aublk->snroff = (DSPshort)(((p_aublk->csnroffst - 15) << 6) + (p_aublk->lfefsnroffst << 2));
	p_aublk->dith = 0;
	p_aublk->cplflag = 0;
	/*	Unpack exponents */
#ifdef MIPOPT
	p_buff->expbuf = p_buff->lfeexps;
	if (p_aublk->lfeexpstr != REUSE)
	{
		unpexps(&p_aublk->lfepkexps, p_decparam, p_buff);
	}
#else
	unpexps(&p_aublk->lfepkexps, p_decparam, p_buff);
#endif
	/*	Expand delta bit allocation */
	temp_status = expand_delta_0(p_decparam, p_buff);
	if(temp_status == 1)
		return 1;
	/*	Unpack mantissas */

#ifdef MIPOPT
	p_buff->bapbuf = p_buff->lfebap;
	if ((p_aublk->lfeexpstr != REUSE) || (p_aublk->baie) || (p_aublk->snroffste))
	{
		p_buff->newbitalloc = 1;
	}
	else
	{
		p_buff->newbitalloc = 0;
	}
#endif
	unpmants(p_decparam,  p_buff);
	/*	Zero out unused transform coefficients */
	for (bin = NLFEMANTS; bin < N; bin++)
	{
		p_aublk->tcbuf[bin] = 0;
	}
	return 0;
}	

/* const table */
extern DSPshort dialexp[32];
extern int dialmant[32];

void calc_gain(int compval, DSPshort dialval, DEC_PARAM  *p_decparam)
{
	long		complong;
	DSPshort	expval;
	DSPshort	gainrngval;
	AC3_AB_DATA *p_aublk = p_decparam->ac3_aub;

	/*	Compute gain exponent/mantissa and modified gain ranging word */
	/*	Adjust for dynamic range compression */
	//complong = (long)(compval * 0x80000000L);
	complong = (long)compval;
	p_aublk->gainexp = (DSPshort)((complong >> 24) + 1);
	p_aublk->gainmant = ((int)(((complong << 2) & 0x3ffffffL) | 0x4000000L));

	/*	Adjust for dialog normalization */
	if (p_decparam->digdialnorm)
	{
		p_aublk->gainexp += dialexp[dialval];
		p_aublk->gainmant = DSPrnd(DYNSCRND, DYNSCSTOR, -Vo_Multi28_Q(p_aublk->gainmant , (dialmant[dialval]>>4)));
	}
	/*	Adjust for pcm scale factor */
	p_aublk->gainmant = DSPrnd(DYNSCRND, DYNSCSTOR, Vo_Multi28_Q(p_aublk->gainmant, (p_decparam->pcmscale>>4)));

	p_aublk->gainmant <<=4;
	/*	Renormalize mantissa/exponent */
	expval = Vo_DSPnorm(p_aublk->gainmant, MAXSHIFT);
	p_aublk->gainmant <<= expval;
	p_aublk->gainexp -= expval;

	/*	Compute modified gain ranging word */
	if (p_aublk->gainmant == 0x40000000L)
	{
		gainrngval = (DSPshort)(p_aublk->appgainrng[p_decparam->channum] - p_aublk->gainexp + 1);
	}
	else
	{
		gainrngval = (DSPshort)(p_aublk->appgainrng[p_decparam->channum] - p_aublk->gainexp);
	}

	if (gainrngval < 0)
	{
		gainrngval = 0;
	}
	else if (gainrngval > 3)
	{
		gainrngval = 3;
	}

	p_aublk->appgainrng[p_decparam->channum] = gainrngval;
	p_aublk->gainexp += gainrngval;
}

int getphssclfac(DSPshort outmode, int chan, DEC_PARAM *p_decparam)
/*
; calc phssclfac
;	input:	surmixlev				surround mix level index
;			chan					input channel number
;			acmod,outmode			encoder and decoder channel mode
;	output:	returns phssclfac		phscor adjust factor for current outmod
;*/
{
	int phssclfac;
	AC3_INFO_SI     *ac3_si = p_decparam->ac3_si;

	phssclfac = 0;
	if (outmode != 7)
	{
		/* if chan needs adjusting */
		if (phsscltab_chan[ac3_si->acmod - NUCPLCHANCFG][outmode] & chanmask[chan])
		{
			if ((ac3_si->surmixlev == 2) && (ac3_si->acmod > 3))
			{
				phssclfac = phsscltab_sml0[ac3_si->acmod - 4][outmode];
			}
			else
			{
				phssclfac = phsscltab[ac3_si->acmod - NUCPLCHANCFG][outmode];
			}
		}
	}	/* end (outmode != 7) */
	return(phssclfac);
}

int scale_cplco(DEC_PARAM *p_decparam)
/*
;	modify cpl coords to account for couple/downmix phase correlation
;	input:	cplcoptr				ptr to cpl coord mantissas for current chan
;			cplcoexpptr				ptr to cpl coord exponents
;			acmod					audio coding mode
;			outmod					decoder output mode
;			phsoutmod				outmod assumed by encoder
;			surmixlev				surround mix level index
;			channum					input channel number
;			cplbegf, cplendf		cpl beginning and ending bands
;	output:	cplcoptr				ptr to new couple coord mants
;			cplcoexpptr				ptr to new couple coord exp
*/
{
	int bnd;
	DSPshort index;
	int dphssclfac, ephssclfac;
	int sclmant;
	DSPshort sclexp;
	DSPshort expval;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;

	dphssclfac = getphssclfac(p_decparam->outmod, p_decparam->channum, p_decparam);
	ephssclfac = getphssclfac(p_aublk->phsoutmod, p_decparam->channum, p_decparam);

	for (bnd = p_aublk->cplbegf; bnd < p_aublk->cplendf; bnd++)
	{
		index = PHSTABOFFST;
		//index += (DSPshort)(p_aublk->phscor[bnd] * dphssclfac + 0.5);
		//index -= (DSPshort)(p_aublk->phscor[bnd] * ephssclfac + 0.5);
		index += Vo_MultiShortQ31(p_aublk->phscor[bnd], dphssclfac);
		index -= Vo_MultiShortQ31(p_aublk->phscor[bnd], ephssclfac);

		if ((index < 0) || (index >= PHSTABSZ))
		{
			return 1;
			//error_msg("PHSCOR table index out of range", FATAL);
		}
		sclmant = Vo_Multi1632((*p_aublk->cplcoptr++), phscortab[index]);
		sclexp = *p_aublk->cplcoexpptr++ - 2;

		/*	Re-normalize, limit and round coupling coordinate */
		expval = Vo_DSPnorm(sclmant, (DSPshort)(MAXSHIFT - sclexp));
		sclmant <<= expval;
		sclexp += expval;
		if (sclexp < 0)
		{
			sclexp = 0;
			if (sclmant > 0)
			{
				sclmant = (int)0x7fffffffL;     //MAXDSPFRACT;				/* limit cpl coord to <1 */
			}
			else
			{
				sclmant = (int)0x80000000L;     //MINDSPFRACT;				/* limit cpl coord to -1 */
			}
		}
		sclmant = DSPrnd(MANTRND, MANTBITS, sclmant);
		p_aublk->sclcplco[bnd] = DSPrnd(TRUNC, MANTBITS - 5, sclmant);
		p_aublk->sclcplcoexp[bnd] = sclexp;
	}	/* for (bnd) */

	p_aublk->cplcoptr = &p_aublk->sclcplco[p_aublk->cplbegf];
	p_aublk->cplcoexpptr = &p_aublk->sclcplcoexp[p_aublk->cplbegf];
	return 0;
}
