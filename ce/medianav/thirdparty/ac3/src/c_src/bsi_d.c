/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/BSI_D.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 decoder BSI data unformatting
;***************************************************************************/

#include "ac3d_com.h"

/* const table */
extern DSPshort chanary[NCHANCFG];

void bsi_d(DEC_PARAM *ac3_decparam)
{
	int i;
	DEC_PARAM         *p_decparam  = (DEC_PARAM *)ac3_decparam;
	AC3_AB_DATA       *p_aublk     = (AC3_AB_DATA *)p_decparam->ac3_aub;
	AC3_INFO_SI       *ac3_si     = (AC3_INFO_SI *)p_decparam->ac3_si;

#if 1   //LHP delete unp_bsi() redundancy operation
	if (p_decparam->blknum == 0)
	{
		/*	Set up defaults */
		p_aublk->dynrng = 0;
		p_aublk->dynrng2 = 0;
		p_aublk->cpldeltnseg = 0;
		for (i = 0; i < NFCHANS; i++)
		{
			p_aublk->chdeltnseg[i] = 0;
		}
		for (i = 0; i < NCPLBNDS; i++)
		{
			p_aublk->phscor[i] = DEFPHSCOR;
		}
		p_aublk->phsoutmod = DEFPHSOUTMOD;
		/*	Check for error conditions */
		ac3_si->nfchans = chanary[ac3_si->acmod];
		ac3_si->nchans = (DSPshort)(ac3_si->nfchans + ac3_si->lfeon);

		if (ac3_si->bsid > MAXBSID)
		{
			p_decparam->out_stat = AC3D_ERR_REV;
		}
		else if ((ac3_si->nfchans > NFCHANS) || (ac3_si->nchans > NCHANS))
		{
			p_decparam->out_stat = AC3D_ERR_CHANS;
		}
	}
#else
	if (blknum == 0)
	{
		/*	Reset packed pointers to start of frame */
		pkbufptr = ac3_in.iptr;
		pkptr = ac3_in.iptr;
		pkbitptr = ac3_in.ioff;
		pkdata = *pkptr;
		/*	Unpack BSI */
		unp_bsi();
		/*	Set up defaults */
		dynrng = 0;
		dynrng2 = 0;
		cpldeltnseg = 0;
		for (i = 0; i < NFCHANS; i++)
		{
			chdeltnseg[i] = 0;
		}
		for (i = 0; i < NCPLBNDS; i++)
		{
			phscor[i] = DEFPHSCOR;
		}
		phsoutmod = DEFPHSOUTMOD;
		/*	Check for error conditions */
		nfchans = chanary[acmod];
		nchans = (DSPshort)(nfchans + lfeon);

		if (bsid > MAXBSID)
		{
			out_stat = AC3D_ERR_REV;
		}
		else if ((nfchans > NFCHANS) || (nchans > NCHANS))
		{
			out_stat = AC3D_ERR_CHANS;
		}
	} 
#endif
	if (p_decparam->outmod == 2)						/* implement preferred stereo downmix mode */
	{
		if (p_decparam->stereomod == AUTO)				/* let bitstream choose stereomode */
		{
			if (ac3_si->dmixmodd)					/* if dmixmod defined */
			{
				switch (ac3_si->dmixmod)			
				{
				case LORO:
					p_decparam->outmod = 2;				/* stereo */
					break;
				case LTRT:
					p_decparam->outmod = 0;				/* Dolby Surround compatible */
					break;
				default:
					p_decparam->outmod = 0;				/* default to Dolby Surround compatible */
				}
			}
			else 
			{
				p_decparam->outmod = 0;								/* default to Dolby Surround compatible */
			}
		}
		else if (p_decparam->stereomod == LORO)
		{
			p_decparam->outmod = 2;						/* stereo */
		}
		else  /* stereomod == LTRT */
		{
			p_decparam->outmod = 0;						/* Dolby Surround compatible */
		}
	}
}

void unp_bsi(DECEXEC *ac3_dec)
{
	DEC_PARAM          *p_decparam = ac3_dec->decparam;
	AC3_INFO_SI        *ac3_si     = p_decparam->ac3_si;
	AC3_BSPK           *p_bstrm    = p_decparam->ac3_str;

	/*	Unpack SI fields */
	bitunp_rj(&ac3_si->syncword, 16, p_bstrm);
	bitunp_rj(&ac3_si->crc1, 16, p_bstrm);
	bitunp_rj(&ac3_si->fscod, 2, p_bstrm);
	bitunp_rj(&ac3_si->frmsizecod, 6, p_bstrm);

	/*	Unpack BSI */
	bitunp_rj(&ac3_si->bsid, 5, p_bstrm);

#ifdef HALFRATE
	/*	Check for half sample-rate */
	if (ac3_si->bsid <= 8)
	{
		p_decparam->halfratecod = 0;			/* normal sample rates */
	}
	else if (ac3_si->bsid == 9)
	{
		p_decparam->halfratecod = 1;			/* half sample rates */
	}
	else if (ac3_si->bsid == 10)
	{
		p_decparam->halfratecod = 2;			/* quarter sample rates */
	}
#endif
	bitunp_rj(&ac3_si->bsmod, 3, p_bstrm);
	bitunp_rj(&ac3_si->acmod, 3, p_bstrm);
	if ((ac3_si->acmod != MODE10) && (ac3_si->acmod & 0x1))
	{
		bitunp_rj(&ac3_si->cmixlev, 2, p_bstrm);
	}
	if (ac3_si->acmod & 0x4)
	{
		bitunp_rj(&ac3_si->surmixlev, 2, p_bstrm);
	}
	if (ac3_si->acmod == MODE20)
	{
		bitunp_rj(&ac3_si->dsurmod, 2, p_bstrm);
	}
	bitunp_rj(&ac3_si->lfeon, 1, p_bstrm);    //low frequency effects on

#ifdef KAWARE
	/*	Check for karaoke bitstream */
	if ((ac3_si->acmod >= MODE20) && (ac3_si->bsmod == KARAOKE_MODE))
	{
		p_decparam->karaokeflag = 1;
	}
	else
	{
		p_decparam->karaokeflag = 0;
	}
#endif
	bitunp_rj(&ac3_si->dialnorm, 5, p_bstrm);              //dialogue normalization word
	bitunp_rj(&ac3_si->compre, 1, p_bstrm);                //compression gain word exists
	if (ac3_si->compre)
	{
		bitunp_lj(&ac3_si->compr, 8, p_bstrm);
	}
	bitunp_rj(&ac3_si->langcode, 1, p_bstrm);
	if (ac3_si->langcode)
	{
		bitunp_rj(&ac3_si->langcod, 8, p_bstrm);
	}
	bitunp_rj(&ac3_si->audprodie, 1, p_bstrm);
	if (ac3_si->audprodie)
	{
		bitunp_rj(&ac3_si->mixlevel, 5, p_bstrm);
		bitunp_rj(&ac3_si->roomtyp, 2, p_bstrm);
	}
	if (ac3_si->acmod == MODE11)
	{
		bitunp_rj(&ac3_si->dialnorm2, 5, p_bstrm);
		bitunp_rj(&ac3_si->compr2e, 1, p_bstrm);
		if (ac3_si->compr2e)
		{
			bitunp_lj(&ac3_si->compr2, 8, p_bstrm);
		}
		bitunp_rj(&ac3_si->langcod2e, 1, p_bstrm);
		if (ac3_si->langcod2e)
		{
			bitunp_rj(&ac3_si->langcod2, 8, p_bstrm);
		}
		bitunp_rj(&ac3_si->audprodi2e, 1, p_bstrm);
		if (ac3_si->audprodi2e)
		{
			bitunp_rj(&ac3_si->mixlevel2, 5, p_bstrm);
			bitunp_rj(&ac3_si->roomtyp2, 2, p_bstrm);
		}
	} /* (acmod == MODE11) */
	bitunp_rj(&ac3_si->copyrightb, 1, p_bstrm);
	bitunp_rj(&ac3_si->origbs, 1, p_bstrm);

	if (ac3_si->bsid == 6)
	{
		bitunp_rj(&ac3_si->xbsi1e, 1, p_bstrm);
		if (ac3_si->xbsi1e)
		{
			if (ac3_si->acmod >= 3)
			{
				bitunp_rj(&ac3_si->dmixmod, 2, p_bstrm);
				bitunp_rj(&ac3_si->ltrtcmixlev, 3, p_bstrm);
				bitunp_rj(&ac3_si->ltrtsurmixlev, 3, p_bstrm);
				bitunp_rj(&ac3_si->lorocmixlev, 3, p_bstrm);
				bitunp_rj(&ac3_si->lorosurmixlev, 3, p_bstrm);

				ac3_si->dmixmodd = 1;
				/* limit lxrxsurmixlev to -1.5dB max */
				if (ac3_si->ltrtsurmixlev < 3)
					ac3_si->ltrtsurmixlev = 3;
				if (ac3_si->lorosurmixlev < 3)
					ac3_si->lorosurmixlev = 3;
			}
			else
			{
				bitskip(14, p_bstrm);
			}
		}
		bitunp_rj(&ac3_si->xbsi2e, 1, p_bstrm);
		if (ac3_si->xbsi2e)
		{
			if (ac3_si->acmod >= 6)
			{
				bitunp_rj(&ac3_si->dsurexmod, 2, p_bstrm);
				ac3_si->dsurexmodd = 1;
			}
			else
			{
				bitskip(2, p_bstrm);
			}

			if (ac3_si->acmod == 2)
			{
				bitunp_rj(&ac3_si->dheadphonmod, 2, p_bstrm);
				ac3_si->dheadphonmodd = 1;
			}
			else
			{
				bitskip(2, p_bstrm);
			}

			bitunp_rj(&ac3_si->adconvtyp, 1, p_bstrm);
			bitunp_rj(&ac3_si->xbsi2, 8, p_bstrm);
			bitunp_rj(&ac3_si->encinfo, 1, p_bstrm);
		}
	}
	else /* if bsid != 6 */
	{
		bitunp_rj(&ac3_si->timecod1e, 1, p_bstrm);
		if (ac3_si->timecod1e)
		{
			bitunp_rj(&ac3_si->timecod1, 14, p_bstrm);
		}
		bitunp_rj(&ac3_si->timecod2e, 1, p_bstrm);
		if (ac3_si->timecod2e)
		{
			bitunp_rj(&ac3_si->timecod2, 14, p_bstrm);
		}
	}
	bitunp_rj(&ac3_si->addbsie, 1, p_bstrm);
	if (ac3_si->addbsie)
	{
		bitunp_rj(&ac3_si->addbsil, 6, p_bstrm);
		//error_msg("bsi_d: additional bsi being skipped", WARNING);
		bitskip((DSPshort)((ac3_si->addbsil + 1) * 8), p_bstrm);
	}
}
