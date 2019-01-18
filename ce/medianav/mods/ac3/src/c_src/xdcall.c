/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/XDCALL.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 fixed data unpacking routine
;***************************************************************************/

#include "ac3d_com.h"

extern DSPshort grpsize[D45];

int xdcall(DEC_PARAM  *p_decparam)
{
	int					chan, bnd, seg;
	DSPshort			grpsz, absexp, npkgrps, index;
	DSPshort			cplexpmax, cplexpbias, cplcomant;
	DSPshort			skipcnt;
	AC3_BSPK			*p_bstrm = p_decparam->ac3_str;
	AC3_INFO_SI         *ac3_si  = p_decparam->ac3_si;
	AC3_AB_DATA         *p_aublk = p_decparam->ac3_aub;

	/*	Block switch flags */
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		bitunp_rj(&p_aublk->blksw[chan], 1, p_bstrm);
	}

	/*	Dither flags */
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		bitunp_rj(&p_aublk->dithflag[chan], 1, p_bstrm);
	}
	/*	Dynamic range control fields */
	bitunp_rj(&p_aublk->dynrnge, 1, p_bstrm);
	if (p_aublk->dynrnge)
	{
		bitunp_lj(&p_aublk->dynrng, 8, p_bstrm);
		p_aublk->dynrng >>= 1;
	}
	if (ac3_si->acmod == MODE11)
	{
		bitunp_rj(&p_aublk->dynrng2e, 1, p_bstrm);
		if (p_aublk->dynrng2e)
		{
			bitunp_lj(&p_aublk->dynrng2, 8, p_bstrm);
			p_aublk->dynrng2 >>= 1;
		}
	}
	/*	Coupling strategy fields */
	bitunp_rj(&p_aublk->cplstre, 1, p_bstrm);

	if ((p_decparam->blknum == 0) && (p_aublk->cplstre == REUSE))
	{
		return 1;       
		//error_msg("xdcall: trying to reuse coupling strategy in block 0", FATAL);
	}

	if (p_aublk->cplstre != REUSE)
	{
		bitunp_rj(&p_aublk->cplinu, 1, p_bstrm);
		if (p_aublk->cplinu)
		{
			if (ac3_si->acmod == MODE11)
			{
				return 1; 
				//error_msg("xdcall: can't couple in 1+1 dual mono mode", FATAL);
			}
			for (chan = 0; chan < ac3_si->nfchans; chan++)
			{
				bitunp_rj(&p_aublk->chincpl[chan], 1, p_bstrm);
			}
			for (chan = 0; chan < ac3_si->nfchans; chan++)
			{
				if (p_aublk->chincpl[chan])
				{
					p_aublk->cplchan = (DSPshort)chan;	/* lowest chan # in cpl */
					break;
				}
			}
			if (ac3_si->acmod == MODE20)
			{
				bitunp_rj(&p_aublk->phsflginu, 1, p_bstrm);
			}
			else
			{
				p_aublk->phsflginu = 0;
			}
			bitunp_rj(&p_aublk->cplbegf, 4, p_bstrm);
			bitunp_rj(&p_aublk->cplendf, 4, p_bstrm);
			p_aublk->cplendf = (DSPshort)(p_aublk->cplendf + 3);
			p_aublk->cplstrtmant = (DSPshort)(37 + (p_aublk->cplbegf * CPLBNDSZ));
			p_aublk->cplendmant = (DSPshort)(37 + (p_aublk->cplendf * CPLBNDSZ));
			p_aublk->cplbndstrc[p_aublk->cplbegf] = 0;
			for (bnd = p_aublk->cplbegf + 1; bnd < p_aublk->cplendf; bnd++)
			{
				bitunp_rj(&p_aublk->cplbndstrc[bnd], 1, p_bstrm);
			}
		}
		else
		{
			for (chan = 0; chan < ac3_si->nfchans; chan++)
			{
				p_aublk->chincpl[chan] = 0;
			}
			p_aublk->phsflginu = 0;
			p_aublk->cplchan = 0;					/* set to known value */
		}
	}
	/*	Coupling coordinates fields */
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		if (p_aublk->chincpl[chan])
		{
			bitunp_rj(&p_aublk->cplcoe[chan], 1, p_bstrm);
			if (p_aublk->cplcoe[chan])
			{
				bitunp_rj(&p_aublk->mstrcplco, 2, p_bstrm);
				cplexpbias = (DSPshort)(3 * p_aublk->mstrcplco);
				cplexpmax = (DSPshort)(cplexpbias + 15);

				for (bnd = p_aublk->cplbegf; bnd < p_aublk->cplendf; bnd++)
				{
					if (p_aublk->cplbndstrc[bnd] == 0)
					{
						bitunp_rj(&p_aublk->cplcoexp[chan][bnd], 4, p_bstrm);
						bitunp_lj(&cplcomant, 4, p_bstrm);
						p_aublk->cplcoexp[chan][bnd] =
							(DSPshort)(cplexpbias + p_aublk->cplcoexp[chan][bnd]);
						if (p_aublk->cplcoexp[chan][bnd] == cplexpmax)
						{
							p_aublk->cplco[chan][bnd] =
								DSPs2f((cplcomant >> 1) & 0x7fff);
						}
						else
						{
							p_aublk->cplco[chan][bnd] =
								DSPs2f(((cplcomant >> 2) & 0x3fff) | 0x4000);
						}
					}
					else
					{
						p_aublk->cplco[chan][bnd] = p_aublk->cplco[chan][bnd - 1];
						p_aublk->cplcoexp[chan][bnd] = p_aublk->cplcoexp[chan][bnd - 1];
					}
				}
			}
		}
	}

	if (p_aublk->phsflginu)
	{
		if ((p_aublk->cplcoe[0]) || (p_aublk->cplcoe[1]))
		{
			for (bnd = p_aublk->cplbegf; bnd < p_aublk->cplendf; bnd++)
			{
				if (p_aublk->cplbndstrc[bnd] == 0)
				{
					bitunp_rj(&p_aublk->phsflg, 1, p_bstrm);
				}
				if (p_aublk->phsflg)
				{
					p_aublk->cplco[1][bnd] = -DSPabs(p_aublk->cplco[1][bnd]);
				}
				else
				{
					p_aublk->cplco[1][bnd] = DSPabs(p_aublk->cplco[1][bnd]);
				}
			}
		}
	}
	/*	Rematrixing operation in the 2/0 mode */
	if (ac3_si->acmod == MODE20)
	{
		if (p_aublk->cplinu)
		{
			if (p_aublk->cplbegf == 0)
			{
				p_aublk->nrematbnds = 2;
			}
			else if (p_aublk->cplbegf <= 2)
			{
				p_aublk->nrematbnds = 3;
			}
			else
			{
				p_aublk->nrematbnds = 4;
			}
		}
		else
		{
			p_aublk->nrematbnds = 4;
		}
		bitunp_rj(&p_aublk->rematstr, 1, p_bstrm);
		if ((p_decparam->blknum == 0) && (p_aublk->rematstr == REUSE))
		{
			return 1; 
			//error_msg("xdcall: trying to reuse rematrix flags in block 0", FATAL);
		}
		if (p_aublk->rematstr)
		{
			p_aublk->rematinu = 0;
			for (bnd = 0; bnd < p_aublk->nrematbnds; bnd++)
			{
				bitunp_rj(&p_aublk->rematflg[bnd], 1, p_bstrm);
				if (p_aublk->rematflg[bnd])
				{
					p_aublk->rematinu = 1;
				}
			}
		}
	}
	else
	{
		p_aublk->rematinu = 0;
	}
	/*	Exponent strategy fields */
	if (p_aublk->cplinu)
	{
		bitunp_rj(&p_aublk->cplexpstr, 2, p_bstrm);
		if ((p_decparam->blknum == 0) && (p_aublk->cplexpstr == REUSE))
		{
			return 1; 
			//error_msg("xdcall: trying to reuse CPL exp strategy in block 0", FATAL);
		}
	}
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		bitunp_rj(&p_aublk->chexpstr[chan], 2, p_bstrm);

		if ((p_decparam->blknum == 0) && (p_aublk->chexpstr[chan] == REUSE))
		{
			return 1; 
			//error_msg("xdcall: trying to reuse exponent strategy in block 0", FATAL);
		}
	}
	if (ac3_si->lfeon)
	{
		bitunp_rj(&p_aublk->lfeexpstr, 1, p_bstrm);

		if ((p_decparam->blknum == 0) && (p_aublk->lfeexpstr == REUSE))
		{
			return 1; 
			//error_msg("xdcall: trying to reuse LFE exp strategy in block 0", FATAL);
		}
	}

	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		if (p_aublk->chexpstr[chan] != REUSE)
		{
			if (p_aublk->chincpl[chan])
			{
				p_aublk->endmant[chan] = p_aublk->cplstrtmant;
			}
			else
			{
				bitunp_rj(&p_aublk->chbwcod[chan], 6, p_bstrm);
				if (p_aublk->chbwcod[chan] > 60)
				{
					return 1; 
					//error_msg("xdcall: chbwcod too high", FATAL);
				}
				p_aublk->endmant[chan] = (DSPshort)(p_aublk->chbwcod[chan] * 3 + 73);
			}
		}
	}
	/*	Exponent fields */
	if (p_aublk->cplinu)
	{
		if (p_aublk->cplexpstr != REUSE)
		{
			index = (DSPshort)(p_aublk->cplexpstr - 1);
			grpsz = grpsize[index];
			npkgrps = (DSPshort)((p_aublk->cplendmant - p_aublk->cplstrtmant) / (grpsz * 3));

			bitunp_rj(&absexp, 4, p_bstrm);
			absexp <<= 1;

			p_aublk->cplpkexps.grpsz = grpsz;
			p_aublk->cplpkexps.npkgrps = npkgrps;
			p_aublk->cplpkexps.absexp = absexp;
			p_aublk->cplpkexps.pkptr = p_bstrm->pkptr;
			p_aublk->cplpkexps.pkbitptr = p_bstrm->pkbitptr;
			p_aublk->cplpkexps.pkdata = p_bstrm->pkdata;

			bitskip((DSPshort)(npkgrps * 7), p_bstrm);	/* skip past exponents */
		}
	}
	for (chan = 0; chan < ac3_si->nfchans; chan++)
	{
		if (p_aublk->chexpstr[chan] != REUSE)
		{
			index = (DSPshort)(p_aublk->chexpstr[chan] - 1);
			grpsz = grpsize[index];
			npkgrps = (DSPshort)((p_aublk->endmant[chan] - 1) / (grpsz * 3));
			if ((npkgrps * grpsz * 3) < (p_aublk->endmant[chan] - 1))
			{
				npkgrps++;
			}
			bitunp_rj(&absexp, 4, p_bstrm);

			p_aublk->pkexps[chan].grpsz = grpsz;
			p_aublk->pkexps[chan].npkgrps = npkgrps;
			p_aublk->pkexps[chan].absexp = absexp;
			p_aublk->pkexps[chan].pkptr = p_bstrm->pkptr;
			p_aublk->pkexps[chan].pkbitptr = p_bstrm->pkbitptr;
			p_aublk->pkexps[chan].pkdata = p_bstrm->pkdata;

			bitskip((DSPshort)(npkgrps * 7), p_bstrm);	/* skip past exponents */
			bitunp_rj(&p_aublk->gainrng[chan], 2, p_bstrm);
		}
	}
	if (ac3_si->lfeon)
	{
		if (p_aublk->lfeexpstr != REUSE)
		{
			grpsz = 1;
			npkgrps = 2;

			bitunp_rj(&absexp, 4, p_bstrm);

			p_aublk->lfepkexps.grpsz = grpsz;
			p_aublk->lfepkexps.npkgrps = npkgrps;
			p_aublk->lfepkexps.absexp = absexp;
			p_aublk->lfepkexps.pkptr = p_bstrm->pkptr;
			p_aublk->lfepkexps.pkbitptr = p_bstrm->pkbitptr;
			p_aublk->lfepkexps.pkdata = p_bstrm->pkdata;

			bitskip(14, p_bstrm);						/* skip past exponents */
		}
	}
	/*	Bit allocation strategy fields */
	bitunp_rj(&p_aublk->baie, 1, p_bstrm);
	if ((p_decparam->blknum == 0) && (p_aublk->baie == 0))
	{
		return 1; 
		//error_msg("xdcall: trying to reuse bit allocation info in block 0", FATAL);
	}
	if (p_aublk->baie)
	{
		bitunp_rj(&p_aublk->sdcycod, 2, p_bstrm);
		bitunp_rj(&p_aublk->fdcycod, 2, p_bstrm);
		bitunp_rj(&p_aublk->sgaincod, 2, p_bstrm);
		bitunp_rj(&p_aublk->dbpbcod, 2, p_bstrm);
		bitunp_rj(&p_aublk->floorcod, 3, p_bstrm);
	}
	bitunp_rj(&p_aublk->snroffste, 1, p_bstrm);

	if ((p_decparam->blknum == 0) && (p_aublk->snroffste == 0))
	{
		return 1; 
		//error_msg("xdcall: trying to reuse SNR offsets in block 0", FATAL);
	}
	if (p_aublk->snroffste)
	{
		bitunp_rj(&p_aublk->csnroffst, 6, p_bstrm);

		if (p_aublk->cplinu)
		{
			bitunp_rj(&p_aublk->cplfsnroffst, 4, p_bstrm);
			bitunp_rj(&p_aublk->cplfgaincod, 3, p_bstrm);
		}
		for (chan = 0; chan < ac3_si->nfchans; chan++)
		{
			bitunp_rj(&p_aublk->fsnroffst[chan], 4, p_bstrm);
			bitunp_rj(&p_aublk->fgaincod[chan], 3, p_bstrm);
		}
		if (ac3_si->lfeon)
		{
			bitunp_rj(&p_aublk->lfefsnroffst, 4, p_bstrm);
			bitunp_rj(&p_aublk->lfefgaincod, 3, p_bstrm);
		}
	}
	if (p_aublk->cplinu)
	{
		bitunp_rj(&p_aublk->cplleake, 1, p_bstrm);
		if ((p_decparam->blknum == 0) && (p_aublk->cplleake == 0))
		{
			return 1; 
			//error_msg("xdcall: trying to reuse cpl leak terms in block 0", FATAL);
		}
		if (p_aublk->cplleake)
		{
			bitunp_rj(&p_aublk->cplfleak, 3, p_bstrm);
			p_aublk->cplfleak <<= 8;
			p_aublk->cplfleak += 0x0300;
			bitunp_rj(&p_aublk->cplsleak, 3, p_bstrm);
			p_aublk->cplsleak <<= 8;
			p_aublk->cplsleak += 0x0300;
		}
	}
	/*	Delta bit allocation */
	bitunp_rj(&p_aublk->deltbaie, 1, p_bstrm);
	if (p_aublk->deltbaie)
	{
		if (p_aublk->cplinu)
		{
			bitunp_rj(&p_aublk->cpldeltbae, 2, p_bstrm);
		}
		for (chan = 0; chan < ac3_si->nfchans; chan++)
		{
			bitunp_rj(&p_aublk->deltbae[chan], 2, p_bstrm);
		}
		if (p_aublk->cplinu)
		{
			if (p_aublk->cpldeltbae == DELTNEW)
			{
				bitunp_rj(&p_aublk->cpldeltnseg, 3, p_bstrm);
				p_aublk->cpldeltnseg += 1;
				for (seg = 0; seg < p_aublk->cpldeltnseg; seg++)
				{
					bitunp_rj(&p_aublk->cpldeltoffst[seg], 5, p_bstrm);
					bitunp_rj(&p_aublk->cpldeltlen[seg], 4, p_bstrm);
					bitunp_rj(&p_aublk->cpldeltba[seg], 3, p_bstrm);
				}
			}
			else if (p_aublk->cpldeltbae >= DELTSTOP)
			{
				p_aublk->cpldeltnseg = 0;
			}
		}
		for (chan = 0; chan < ac3_si->nfchans; chan++)
		{
			if (p_aublk->deltbae[chan] == DELTNEW)
			{
				bitunp_rj(&p_aublk->chdeltnseg[chan], 3, p_bstrm);
				p_aublk->chdeltnseg[chan] += 1;
				for (seg = 0; seg < p_aublk->chdeltnseg[chan]; seg++)
				{
					bitunp_rj(&p_aublk->chdeltoffst[chan][seg], 5, p_bstrm);
					bitunp_rj(&p_aublk->chdeltlen[chan][seg], 4, p_bstrm);
					bitunp_rj(&p_aublk->chdeltba[chan][seg], 3, p_bstrm);
				}
			}
			else if (p_aublk->deltbae[chan] >= DELTSTOP)
			{
				p_aublk->chdeltnseg[chan] = 0;
			}
		}
	}
	/*	Skip field */
	bitunp_rj(&p_aublk->skiple, 1, p_bstrm);
	if (p_aublk->skiple)
	{
		bitunp_rj(&p_aublk->skipl, 9, p_bstrm);
		skipcnt = (DSPshort)(p_aublk->skipl * 8);
		if ((skipcnt > 0) && (p_aublk->cplinu))
		{
			bitunp_rj(&p_aublk->phscore, 1, p_bstrm);
			skipcnt -= 1;
			if (p_aublk->phscore)
			{
				bitunp_rj(&p_aublk->phsoutmod, NPHSOUTMODBITS, p_bstrm);
				skipcnt -= NPHSOUTMODBITS;
				bitunp_rj(&p_aublk->phscorstr, 1, p_bstrm);
				skipcnt -= 1;
				if (p_aublk->phscorstr == COEF_PER_BAND)
				{	
					/*	Read new phscor value for each coupled band */
					for (bnd = p_aublk->cplbegf; bnd < p_aublk->cplendf; bnd++)
					{
						if (p_aublk->cplbndstrc[bnd] == 0)
						{
							bitunp_rj(&p_aublk->phscor[bnd], NPHSCORBITS, p_bstrm);
							skipcnt -= NPHSCORBITS;
						}
						else
						{
							p_aublk->phscor[bnd] = p_aublk->phscor[bnd - 1];
						}
					}
				}
				else
				{
					/*	Read one phscor value for all coupled bands */
					bitunp_rj(&p_aublk->phscor[p_aublk->cplbegf], NPHSCORBITS, p_bstrm);
					skipcnt -= NPHSCORBITS;

					for (bnd = (p_aublk->cplbegf + 1); bnd < p_aublk->cplendf; bnd++)
					{
						p_aublk->phscor[bnd] = p_aublk->phscor[bnd - 1];
					}
				}
			}	/* phscore */
		}	/* skipcnt > 0 */
		bitskip(skipcnt, p_bstrm);
	}
	return 0;
}
