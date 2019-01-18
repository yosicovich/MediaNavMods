/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3_DEC.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Top level AC-3 decode routine
;***************************************************************************/
#include "ac3d_com.h"

extern DSPshort frmsizetab[NFSCOD][NDATARATE];

DOLBY_SIP ac3_dec(DOLBY_SIP input_sip,DECEXEC *p_decexec)
{
	int chan, frmsize;
	DOLBY_SIP     ret_sip;

	DEC_PARAM        *p_decparam;
	AC3_INFO_SI      *ac3_si;
	BUFF_PARAM       *p_buff;
	AC3_AB_DATA      *p_aublk;
	AC3_BSPK         *p_bstrm;
	AC3_DEC_PL       *ac3_in_ptr;

	p_decparam = p_decexec->decparam;
	ac3_si     = p_decparam->ac3_si;
	p_bstrm    = p_decparam->ac3_str;
	p_aublk    = p_decparam->ac3_aub;
	p_buff     = p_decexec->ac3_buff;

	ac3_in_ptr = p_decexec->ac3_dec_list;
    p_decparam->in_stat = input_sip.status;

	ac3_init(p_decexec);
	if (p_decparam->out_stat == AC3D_ERR_NONE)
	{
		bsi_d(p_decparam);
		//bsi_d(p_decexec);
	}
	if (p_decparam->out_stat == AC3D_ERR_NONE)
	{
		p_decparam->out_stat = xdcall(p_decparam);                                          //fixed data
		if(p_decparam->out_stat == 1)
			goto exit;		
		if (ac3_in_ptr->blknum == 5)
		{
			frmsize = frmsizetab[ac3_si->fscod][ac3_si->frmsizecod];
			if ((p_bstrm->pkptr - p_bstrm->pkbufptr) < (frmsize * 5 / 8))
			{
				p_decparam->out_stat = 0;
			}
		}
	}
	if (p_decparam->out_stat == AC3D_ERR_NONE)
	{
		setup_downmix(p_decparam); 
		p_decparam->out_stat = mants_d(p_decparam, p_buff);
		if(p_decparam->out_stat == 1)
			goto exit;
		clr_downmix(p_decparam, p_buff);
		if (p_decparam->blknum == 1)
		{
			frmsize = frmsizetab[ac3_si->fscod][ac3_si->frmsizecod];
			if (((p_bstrm->pkptr - p_bstrm->pkbufptr) > (frmsize * 5 / 8)) ||
				(((p_bstrm->pkptr - p_bstrm->pkbufptr) == (frmsize * 5 / 8)) && (p_bstrm->pkbitptr)))
			{
				p_decparam->out_stat = 0;
			}
		}
	}
	else if (p_decparam->out_stat == AC3D_ERR_RPT)
	{
		p_decparam->out_stat = 0;
	}
	else
	{
		p_decparam->out_stat = 0;
		clr_downmix(p_decparam, p_buff);
	}
	for (chan = 0; chan < p_decparam->outnchans; chan++)
	{
		window_d((DSPshort)chan, p_decexec);
	}

exit:
	ret_sip.status = p_decparam->out_stat;
	ret_sip.funcnum = SIP_REV;
	ret_sip.param_ptr = NULLPTR;
	return(ret_sip);
}

/* const tables */
extern DSPshort  pkwrdszary[3], chanary[NCHANCFG];

void ac3_init(DECEXEC *p_decexec)
{
	int i, j, ch;
	DEC_PARAM        *p_decparam;
	BUFF_PARAM       *p_buff;
	AC3_DEC_PL       *ac3_in_ptr;
	AC3_BSPK         *p_bstrm;
	AC3_AB_DATA      *p_aublk;

	p_decparam  = p_decexec->decparam;
	p_buff      = p_decexec->ac3_buff;
	ac3_in_ptr  = p_decexec->ac3_dec_list;
	p_bstrm     = p_decparam->ac3_str;
	p_aublk     = p_decparam->ac3_aub;

	if (!(p_decparam->ac3raminit))
	{
		/*	Initialize frame-to-frame static data values */
		p_decparam->dithreg = DITHSEED;
		p_decparam->rptcnt = 0;
		p_decparam->lastoutlfe = -1;
		p_decparam->lastoutmod = -1;
#ifndef DITHBND
		p_decparam->dithscale = DITHSCALE;
#endif
		p_decparam->ac3raminit = 1;	
	}
	/*	Set up internal parameter list */
	//ac3_in = *ac3_in_ptr;				/* copy parameter list structure */
	/*	Set up control variables */
	for (ch = 0; ch < NCHANS; ch++)
	{
		p_decparam->pcmbufptr[ch] = ac3_in_ptr->optr[ch];
		p_decparam->pcmbufoff[ch] = ac3_in_ptr->ooff[ch];
		p_decparam->pcmbufmod[ch] = ac3_in_ptr->omod[ch];
	}
	p_decparam->blknum = ac3_in_ptr->blknum;
	p_bstrm->pkwrdsz = pkwrdszary[((ac3_in_ptr->icfg) >> 14) & 0x3];
	p_decparam->compmod = (DSPshort)(((ac3_in_ptr->ocfg) >> 12) & 0x3);
	p_decparam->stereomod = (DSPshort)(((ac3_in_ptr->ocfg) >> 10) & 0x3);
	p_decparam->dualmod = (DSPshort)(((ac3_in_ptr->ocfg) >> 8) & 0x3);
	p_decparam->outlfe = (DSPshort)(((ac3_in_ptr->ocfg) >> 3) & 0x1);
	p_decparam->outmod = (DSPshort)((ac3_in_ptr->ocfg) & 0x7);
	p_decparam->outnchans = (DSPshort)(chanary[p_decparam->outmod] + p_decparam->outlfe);
	p_decparam->dynscalehigh = DSPrnd(DYNSCRND, DYNSCSTOR, ac3_in_ptr->dynsclh);
	p_decparam->dynscalelow = DSPrnd(DYNSCRND, DYNSCSTOR, ac3_in_ptr->dynscll);
	p_decparam->pcmscale = DSPrnd(DYNSCRND, DYNSCSTOR, ac3_in_ptr->pcmscl);
	p_decparam->ext_dnmixtab = ac3_in_ptr->dnmxptr;

#ifdef KCAPABLE
	p_decparam->kcapableptr = ac3_in_ptr->krkptr;
#endif

	/*	If output mode changed, clear delay buffers */
	if ((p_decparam->outmod != p_decparam->lastoutmod) || (p_decparam->outlfe != p_decparam->lastoutlfe))
	{
		for (i = 0; i < p_decparam->outnchans; i++)
		{
			for (j = 0; j < N/2; j++)
			{
				p_buff->delay_buf[i][j] = 0;
			}
		}
		p_decparam->lastoutmod = p_decparam->outmod;
		p_decparam->lastoutlfe = p_decparam->outlfe;
	}
	/*	Indicate downmix buffers clear */
	for (i = 0; i < p_decparam->outnchans; i++)
	{
		p_aublk->dnmixbufinu[i] = 0;
	}
	/*	If block 0, set up the return status */
	if (ac3_in_ptr->blknum == 0)
	{
		p_decparam->out_stat = AC3D_ERR_NONE;
	}
	/*	Check error status */
	if (p_decparam->out_stat == AC3D_ERR_NONE)
	{
		if (p_decparam->in_stat > 0)
		{
			p_decparam->out_stat = AC3D_ERR_RPT;
		}
		else if (p_decparam->in_stat < 0)
		{
			p_decparam->out_stat = AC3D_ERR_MUTE;
			p_decparam->rptcnt = 0;
		}
		else
		{
			p_decparam->rptcnt = 0;
		}
	}
	if (p_decparam->out_stat == AC3D_ERR_RPT)
	{
		if (ac3_in_ptr->rptmax != 0)
		{
			if (p_decparam->rptcnt >= ac3_in_ptr->rptmax)
			{
				p_decparam->out_stat = AC3D_ERR_MUTE;
			}
			else
			{
				p_decparam->rptcnt++;
			}
		}
	}
}
