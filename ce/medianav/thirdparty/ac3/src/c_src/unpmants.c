/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/UNPMANTS.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 mantissa unpacking subroutine
;***************************************************************************/

#include "ac3d_com.h"
#include "ac3d_fix.h"
#include <stdio.h>

extern DSPshort babndtab[NBABNDS], hth[NFSCOD][NBABNDS];
extern DSPshort deltatab[8], latab[LATABSZ], pvtab[PVTABSZ], qntztab[16];
extern DSPushort ungrp3[27], ungrp5[125], ungrp11[121];
extern int qntz3lev[3], qntz5lev[5], qntz7lev[7];
extern int qntz11lev[11], qntz15lev[15];

#ifdef DITHBND
extern int dithscaletab[NHALFRATE][NDITHSCALE];
#endif

void unpexps(PKEXPS *pkexpptr, DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	DSPshort grpsz, npkgrps, absexp, pkexp;
	DSPshort *savpkptr, savpkbitptr, savpkdata;
	int grp, shr, bin;

	AC3_BSPK         *p_bstrm = p_decparam->ac3_str;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	//AC3_INFO_SI      *ac3_si  = p_decparam->ac3_si;

	/*	Save packed data pointers */
	savpkptr = p_bstrm->pkptr;
	savpkbitptr = p_bstrm->pkbitptr;
	savpkdata = p_bstrm->pkdata;	
	/*	Load packed exponent pointers */
	grpsz = pkexpptr->grpsz;
	npkgrps = pkexpptr->npkgrps;
	absexp = pkexpptr->absexp;
	p_bstrm->pkptr = pkexpptr->pkptr;
	p_bstrm->pkbitptr = pkexpptr->pkbitptr;
	p_bstrm->pkdata = pkexpptr->pkdata;

	bin = p_aublk->mantindex;
	/*	If not the coupling channel, save the absolute exponent */
	if (bin == 0)
	{
		p_buff->expbuf[bin++] = absexp;
	}
	/*	Unpack remaining exponents */
	for (grp = 0; grp < npkgrps; grp++)
	{
		bitunp_rj(&pkexp, 7, p_bstrm);

		pkexp = ungrp5[pkexp];

		absexp += (DSPshort)(((pkexp >> 12) & 0x000f) - 2);
		for (shr = 0; shr < grpsz; shr++)
		{
			p_buff->expbuf[bin++] = absexp;
		}
		absexp += (DSPshort)(((pkexp >> 8) & 0x000f) - 2);
		for (shr = 0; shr < grpsz; shr++)
		{
			p_buff->expbuf[bin++] = absexp;
		}
		absexp += (DSPshort)(((pkexp >> 4) & 0x000f) - 2);
		for (shr = 0; shr < grpsz; shr++)
		{
			p_buff->expbuf[bin++] = absexp;
		}
	}

	/*	Restore packed data pointers */
	p_bstrm->pkptr = savpkptr;
	p_bstrm->pkbitptr = savpkbitptr;
	p_bstrm->pkdata = savpkdata;
}

int expand_delta(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int bnd, seg, len;
	DSPshort val;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	DSPshort  *deltoffst = p_aublk->chdeltoffst[p_decparam->channum];
	DSPshort  *deltlen   = p_aublk->chdeltlen[p_decparam->channum];
	DSPshort  *deltba    = p_aublk->chdeltba[p_decparam->channum];
	DSPshort  deltnseg   = p_aublk->chdeltnseg[p_decparam->channum];
	for (bnd = 0; bnd < NBABNDS; bnd++)  
	{
		p_buff->deltary[bnd] = 0;
	}

	bnd = p_aublk->bandindex;
	for (seg = 0; seg < deltnseg; seg++)
	{
		bnd += deltoffst[seg];
		if (deltlen[seg] > 0)
		{
			val = deltba[seg];
			for (len = 0; len < deltlen[seg]; len++)
			{
				if (bnd >= NBABNDS)
				{
                 return 1;
				}
				p_buff->deltary[bnd++] = deltatab[val];
			}
		}
	}
   return 0;
}
int expand_delta_1(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int bnd, seg, len;
	DSPshort val;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	DSPshort  *deltoffst = p_aublk->cpldeltoffst;
	DSPshort  *deltlen   = p_aublk->cpldeltlen;
	DSPshort  *deltba    = p_aublk->cpldeltba;
	DSPshort  deltnseg   = p_aublk->cpldeltnseg;
	for (bnd = 0; bnd < NBABNDS; bnd++)  
	{
		p_buff->deltary[bnd] = 0;
	}
	bnd = p_aublk->bandindex;
	for (seg = 0; seg < deltnseg; seg++)
	{
		bnd += deltoffst[seg];
		if (deltlen[seg] > 0)
		{
			val = deltba[seg];
			for (len = 0; len < deltlen[seg]; len++)
			{
				if (bnd >= NBABNDS)
				{
				 return 1;
				}
				p_buff->deltary[bnd++] = deltatab[val];
			}
		}
	}
	return 0;
}

int expand_delta_0(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	int bnd, seg, len;
	DSPshort val;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	DSPshort  *deltoffst = 0;
	DSPshort  *deltlen   = 0;
	DSPshort  *deltba    = 0;
	DSPshort  deltnseg   = 0;
	for (bnd = 0; bnd < NBABNDS; bnd++)  
	{
		p_buff->deltary[bnd] = 0;
	}

	bnd = p_aublk->bandindex;
	for (seg = 0; seg < deltnseg; seg++)
	{
		bnd += deltoffst[seg];
		if (deltlen[seg] > 0)
		{
			val = deltba[seg];
			for (len = 0; len < deltlen[seg]; len++)
			{
				if (bnd >= NBABNDS)
				{
                 return 1;
				}
				p_buff->deltary[bnd++] = deltatab[val];
			}
		}
	}
	return 0;
}

void unpmants(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
{
	DSPshort      leakflag = 0, lowcomp = 0, bndpsd, nextpsd = 0, lastpsd = 0, excite, mask, bapval;
	DSPshort      bndsize, psdval, diff, index, cplgainexp, exp, mant, numbits;
	int           mantfract, dithtemp, cplgainmant;
	int           bin, count;
	AC3_AB_DATA      *p_aublk = p_decparam->ac3_aub;
	AC3_INFO_SI      *ac3_si  = p_decparam->ac3_si;
	AC3_BSPK         *p_bstrm = p_decparam->ac3_str;

	do
	{
		bndsize = (DSPshort)(DSPmin(p_aublk->lastmant, babndtab[p_aublk->bandindex]) - p_aublk->mantindex);

#ifdef MIPOPT
		if (p_buff->newbitalloc)
		{
#endif
			/*	Compute PSDs */
			bin = p_aublk->mantindex;
			if (p_aublk->bandindex < 20)
			{
				bndpsd = (DSPshort)(3072 - (p_buff->expbuf[bin] << 7));
				nextpsd = (DSPshort)(3072 - (p_buff->expbuf[bin + 1] << 7));
				p_buff->psdbuf[0] = bndpsd;
			}
			else
			{
				bndpsd = (DSPshort)(3072 - (p_buff->expbuf[bin++] << 7));
				p_buff->psdbuf[0] = bndpsd;
				for (count = 1; count < bndsize; count++)
				{
					psdval = (DSPshort)(3072 - (p_buff->expbuf[bin++] << 7));
					p_buff->psdbuf[count] = psdval;
					diff = (DSPshort)(DSPabs(bndpsd - psdval) >> 1);
					index = DSPmin(diff, (LATABSZ - 1));
					bndpsd = (DSPshort)(DSPmax(bndpsd, psdval) + latab[index]);
				}
			}
			/*	Compute excitation function */
			if (p_aublk->bandindex <= 2)
			{
				if (bndpsd == (nextpsd - 256))
				{
					lowcomp = LOWCOMP_LOW;
				}
				else if (bndpsd > nextpsd)
				{
					lowcomp = DSPmax(0, lowcomp - 64);
				}

				p_aublk->fleak = (DSPshort)(bndpsd - p_aublk->fgain);
				p_aublk->sleak = (DSPshort)(bndpsd - p_aublk->sgain);
				excite = (DSPshort)(p_aublk->fleak - lowcomp);
				lastpsd = bndpsd;
			}
			else if (p_aublk->bandindex <= 5)
			{
				if (bndpsd == (nextpsd - 256))
				{
					lowcomp = LOWCOMP_LOW;
				}
				else if (bndpsd > nextpsd)
				{
					lowcomp = DSPmax(0, lowcomp - 64);
				}	
				if (lastpsd <= bndpsd)
				{
					leakflag = 1;
				}
				if (leakflag)
				{
					p_aublk->fleak = DSPmax(p_aublk->fleak - p_aublk->fdecay, bndpsd - p_aublk->fgain);
					p_aublk->sleak = DSPmax(p_aublk->sleak - p_aublk->sdecay, bndpsd - p_aublk->sgain);
					excite = DSPmax(p_aublk->fleak - lowcomp, p_aublk->sleak);
				}
				else
				{
					p_aublk->fleak = (DSPshort)(bndpsd - p_aublk->fgain);
					p_aublk->sleak = (DSPshort)(bndpsd - p_aublk->sgain);
					excite = (DSPshort)(p_aublk->fleak - lowcomp);
				}
				lastpsd = bndpsd;
			}
			else if (p_aublk->bandindex == 6)
			{
				if (p_aublk->lastmant != NLFEMANTS)
				{
					if (bndpsd == (nextpsd - 256))
					{
						lowcomp = LOWCOMP_LOW;
					}
					else if (bndpsd > nextpsd)
					{
						lowcomp = DSPmax(0, lowcomp - 64);
					}
				}
				if (lastpsd <= bndpsd)
				{
					leakflag = 1;
				}
				if (leakflag)
				{
					p_aublk->fleak = DSPmax(p_aublk->fleak - p_aublk->fdecay, bndpsd - p_aublk->fgain);
					p_aublk->sleak = DSPmax(p_aublk->sleak - p_aublk->sdecay, bndpsd - p_aublk->sgain);
					excite = DSPmax(p_aublk->fleak - lowcomp, p_aublk->sleak);
				}
				else
				{
					p_aublk->fleak = (DSPshort)(bndpsd - p_aublk->fgain);
					p_aublk->sleak = (DSPshort)(bndpsd - p_aublk->sgain);
					excite = (DSPshort)(p_aublk->fleak - lowcomp);
				}
			}
			else if (p_aublk->bandindex <= 19)
			{
				if (bndpsd == (nextpsd - 256))
				{
					lowcomp = LOWCOMP_MID;
				}
				else if (bndpsd > nextpsd)
				{
					lowcomp = DSPmax(0, lowcomp - 64);
				}
				p_aublk->fleak = DSPmax(p_aublk->fleak - p_aublk->fdecay, bndpsd - p_aublk->fgain);
				p_aublk->sleak = DSPmax(p_aublk->sleak - p_aublk->sdecay, bndpsd - p_aublk->sgain);
				excite = DSPmax(p_aublk->fleak - lowcomp, p_aublk->sleak);
			}
			else if (p_aublk->bandindex <= 22)
			{
				lowcomp = DSPmax(0, lowcomp - 128);
				p_aublk->fleak = DSPmax(p_aublk->fleak - p_aublk->fdecay, bndpsd - p_aublk->fgain);
				p_aublk->sleak = DSPmax(p_aublk->sleak - p_aublk->sdecay, bndpsd - p_aublk->sgain);
				excite = DSPmax(p_aublk->fleak - lowcomp, p_aublk->sleak);
			}
			else
			{
				p_aublk->fleak = DSPmax(p_aublk->fleak - p_aublk->fdecay, bndpsd - p_aublk->fgain);
				p_aublk->sleak = DSPmax(p_aublk->sleak - p_aublk->sdecay, bndpsd - p_aublk->sgain);
				excite = DSPmax(p_aublk->fleak, p_aublk->sleak);
			}

			/*	Compute masking curve */

			if (bndpsd < p_aublk->dbknee)
			{
				excite = (DSPshort)(excite + ((p_aublk->dbknee - bndpsd) >> 2));
			}
#ifdef HALFRATE
			mask = DSPmax(excite, hth[ac3_si->fscod][p_aublk->bandindex >> p_decparam->halfratecod]);
#else
			mask = DSPmax(excite, hth[fscod][bandindex]);
#endif
			mask += p_buff->deltary[p_aublk->bandindex];
			mask -= p_aublk->snroff;
			mask = DSPmax(mask - p_aublk->floorval, 0);
			mask = DSPmin(0x01FE0, mask);
			mask = (DSPshort)(mask & 0x01FE0);		/* limit mask to eight bits */
			mask += p_aublk->floorval;
			if (p_aublk->minsnrflg == 0)
			{
				mask = 0x01FE0;						/* force zero mantissa bits */
			}
			/*	Compute bit allocation values */

			for (count = 0; count < bndsize; count++)
			{
				index = (DSPshort)(DSPmax(p_buff->psdbuf[count] - mask, 0) >> 5);
				index = DSPmin(index, (PVTABSZ - 1));
#ifdef MIPOPT
				p_buff->bapbuf[p_aublk->mantindex + count] = pvtab[index];
#else
				p_buff->psdbuf[count] = pvtab[index];
#endif
			}

#ifdef MIPOPT
		}
#endif
		/*	Unpack mantissas */
		for (count = 0; count < bndsize; count++)
		{
#ifdef MIPOPT
			bapval = p_buff->bapbuf[p_aublk->mantindex + count];
#else
			bapval = psdbuf[count];
#endif
			if (bapval == 0)
			{
				/*	0-level quantizer */
				if (p_aublk->dith == 1)
				{
					p_decparam->dithreg = SGN_EXTND((DSPshort)((DITHMULT * p_decparam->dithreg) & 0xffff));
					dithtemp = p_decparam->dithreg;
					p_decparam->dithreg = SGN_EXTND((DSPshort)((DITHMULT * p_decparam->dithreg) & 0xffff));
					dithtemp = (dithtemp + p_decparam->dithreg)>>1;
#ifdef DITHBND
					if (p_aublk->mantindex < DTHRESH1)
					{
						dithtemp = Vo_Multi1632_Scale((short)dithtemp, dithscaletab[p_decparam->halfratecod][0]);
					}
					else if (p_aublk->mantindex < DTHRESH2)
					{
						dithtemp = Vo_Multi1632_Scale((short)dithtemp, dithscaletab[p_decparam->halfratecod][1]);
					}
					else
					{
						dithtemp = Vo_Multi1632_Scale((short)dithtemp, dithscaletab[p_decparam->halfratecod][2]);
					}
#else
					dithtemp = Vo_Multi1632_Scale((short)dithtemp, p_decparam->dithscale);
#endif
					dithtemp = DSPrnd(DITHRND, DITHBITS, dithtemp);
					p_buff->mantbuf[count] = dithtemp;
				}
				else
				{
					p_buff->mantbuf[count] = 0;
				}
			}
			else if (bapval == 1)
			{
				/*	3-level quantizer */
				if (--p_aublk->count3 < 0)
				{
					bitunp_rj(&mant, 5, p_bstrm);
					p_aublk->mant3 = ungrp3[mant];
					p_aublk->count3 = 2;
				}

				p_buff->mantbuf[count] = qntz3lev[(p_aublk->mant3 >> 12) & 0x000f];
				p_aublk->mant3 <<= 4;
			}
			else if (bapval == 2)
			{
				/*	5-level quantizer */
				if (--p_aublk->count5 < 0)
				{
					bitunp_rj(&mant, 7, p_bstrm);
					p_aublk->mant5 = ungrp5[mant];
					p_aublk->count5 = 2;
				}
				p_buff->mantbuf[count] = qntz5lev[(p_aublk->mant5 >> 12) & 0x000f];
				p_aublk->mant5 <<= 4;
			}
			else if (bapval == 3)
			{
				/*	7-level quantizer */
				bitunp_rj(&mant, 3, p_bstrm);
				p_buff->mantbuf[count] = qntz7lev[mant];
			}
			else if (bapval == 4)
			{
				/*	11-level quantizer */
				if (--p_aublk->count11 < 0)
				{
					bitunp_rj(&mant, 7, p_bstrm);
					p_aublk->mant11 = ungrp11[mant];
					p_aublk->count11 = 1;
				}
				p_buff->mantbuf[count] = qntz11lev[(p_aublk->mant11 >> 12) & 0x000f];
				p_aublk->mant11 <<= 4;
			}
			else if (bapval == 5)
			{
				/*	15-level quantizer */
				bitunp_rj(&mant, 4, p_bstrm);
				p_buff->mantbuf[count] = qntz15lev[mant];
			}
			else
			{
				/*	Even assymetric quantizer */
				numbits = qntztab[bapval];
				bitunp_lj(&mant, numbits, p_bstrm);   
				mantfract = DSPs2f(mant << 16);
				p_buff->mantbuf[count] = DSPrnd(MANTRND, MANTBITS, mantfract);
			}
		}
		/*	Denormalize mantissas */
		bin = p_aublk->mantindex;
		if (p_aublk->cplflag)
		{
			cplgainmant = DSPrnd(MANTRND, MANTBITS, Vo_Multi32((*p_aublk->cplcoptr), p_aublk->gainmant));
			cplgainexp = (DSPshort)(p_aublk->gainexp - (*p_aublk->cplcoexpptr));
			for (count = 0; count < bndsize; count++)
			{
				exp = (DSPshort)(p_buff->expbuf[bin] - cplgainexp);
#ifdef RNDSCLMANT
				mantfract = DSPrnd(MANTRND, MANTBITS,Vo_Multi32(p_buff->mantbuf[count], cplgainmant));
#else
				mantfract = Vo_Multi32(p_buff->mantbuf[count], cplgainmant);
#endif
				if (exp > MAXSHIFT)
				{
					exp = MAXSHIFT;
				}
				if (exp >= MAXEXP)
				{
					p_aublk->tcbuf[bin++] = 0;
				}
				else if (exp > 0)
				{
					p_aublk->tcbuf[bin++] = DSPrnd(MANTRND, MANTBITS,
						mantfract >> exp);
				}
				else
				{
					p_aublk->tcbuf[bin++] = DSPrnd(MANTRND, MANTBITS,
						mantfract << (-exp));
				}
				if (++p_aublk->cplcnt == 12)
				{
					p_aublk->cplcoptr++;
					p_aublk->cplcoexpptr++;
					cplgainexp = (DSPshort)(p_aublk->gainexp - (*p_aublk->cplcoexpptr));
					cplgainmant = DSPrnd(MANTRND, MANTBITS,
						Vo_Multi32_Q((*p_aublk->cplcoptr), p_aublk->gainmant));
					p_aublk->cplcnt = 0;
				}
			}
		}
		else
		{
			for (count = 0; count < bndsize; count++)
			{
				exp = (DSPshort)(p_buff->expbuf[bin] - p_aublk->gainexp);
#ifdef RNDSCLMANT
				mantfract = DSPrnd(MANTRND, MANTBITS, Vo_Multi32(p_buff->mantbuf[count], p_aublk->gainmant));
#else
				mantfract = Vo_Multi32(p_buff->mantbuf[count], p_aublk->gainmant);
#endif

				if (exp > MAXSHIFT)
				{
					exp = MAXSHIFT;
				}
				if (exp >= MAXEXP)
				{
					p_aublk->tcbuf[bin++] = 0;
				}
				else if (exp > 0)
				{
					p_aublk->tcbuf[bin++] = DSPrnd(MANTRND, MANTBITS,
						mantfract>>exp);
				}
				else
				{
					p_aublk->tcbuf[bin++] = DSPrnd(MANTRND, MANTBITS,
						mantfract << (-exp));
				}
			}
		}
		p_aublk->mantindex += bndsize;
		p_aublk->bandindex++;
	} while (p_aublk->mantindex != p_aublk->lastmant);
}
