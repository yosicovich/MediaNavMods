/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3_INFO.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Dolby parameter list equates
;***************************************************************************/

#include   "ac3d_com.h"
#include   "vo_ac3_var.h" 

/* const tables */
extern DSPshort pkwrdszary[3], frmsizetab[NFSCOD][NDATARATE];

DOLBY_SIP ac3_info(DECEXEC *ac3_dec)
{
	DOLBY_SIP ret_sip;
	DSPshort framesize, ret_status;
	AC3_INFO_PL       *ac3i_in;
	AC3_INFO_RL       *ac3i_out;
	AC3_BSPK          *p_bitspk;
	AC3_INFO_SI       *ac3_si;

	/*	Init packed data pointers */
	ac3i_in  = ac3_dec->ac3_info_list;
	ac3i_out = ac3_dec->ac3_info_rptr;
	ac3_si   = ac3_dec->decparam->ac3_si;
	p_bitspk = ac3_dec->decparam->ac3_str;

	p_bitspk->pkptr    = ac3i_in->iptr;
	p_bitspk->pkbitptr = ac3i_in->ioff; 
	p_bitspk->pkdata   = *p_bitspk->pkptr;
	p_bitspk->pkmod    = ac3i_in->imod;
	p_bitspk->pkwrdsz  = pkwrdszary[((ac3i_in->icfg) >> 14) & 0x3];

	/*	Clear BSI data to force default values to zero */
	ac3_si->cmixlev = 0;				//center mix level 
	ac3_si->surmixlev = 0;				//surround mix level
	ac3_si->dsurmod = 0;				//dolby surround mode
	ac3_si->langcod = 0;				//language code
	ac3_si->mixlevel = 0;
	ac3_si->roomtyp = 0;
	ac3_si->dialnorm2 = 0;
	ac3_si->langcod2 = 0;
	ac3_si->audprodi2e = 0;
	ac3_si->mixlevel2 = 0;
	ac3_si->roomtyp2 = 0;
	ac3_si->timecod1 = 0;
	ac3_si->timecod2 = 0;
	ac3_si->xbsi1e = 0;
	ac3_si->dmixmod = 0;
	ac3_si->ltrtcmixlev = 4;		/* 4 = -3 dB */
	ac3_si->ltrtsurmixlev = 4;		/* 4 = -3 dB */		
	ac3_si->lorocmixlev = 4;		/* 4 = -3 dB */
	ac3_si->lorosurmixlev = 4;		/* 4 = -3 dB */
	ac3_si->xbsi2e = 0;
	ac3_si->dsurexmod = 0;
	ac3_si->dheadphonmod = 0;
	ac3_si->adconvtyp = 0;
	ac3_si->xbsi2 = 0;
	ac3_si->encinfo = 0;

	ac3_si->dmixmodd = 0;
	ac3_si->dsurexmodd = 0;
	ac3_si->dheadphonmodd = 0;

	/*	Unpack BSI data */

	unp_bsi(ac3_dec);

	/*	Set up return parameter list */

	framesize = frmsizetab[ac3_si->fscod][ac3_si->frmsizecod];

	ac3i_out->size = 10;  
	ac3i_out->bscfg = (DSPshort)((ac3_si->fscod << 14) + (ac3_si->frmsizecod << 8)
#ifdef HALFRATE
		+ (ac3_dec->decparam->halfratecod << 4)
#endif
		+ (ac3_si->lfeon << 3) + (ac3_si->acmod));
	ac3i_out->frmsize = framesize;
	ac3i_out->crcsize = (DSPshort)((framesize >> 3) + (framesize >> 1) - 1);
	ac3i_out->bsinfo = (DSPshort)((ac3_si->bsid << 11) + (ac3_si->bsmod << 8)
		+ (ac3_si->cmixlev << 6) + (ac3_si->surmixlev << 4) + (ac3_si->dsurmod << 2) 
		+ (ac3_si->copyrightb << 1) + (ac3_si->origbs));
	ac3i_out->dialnorm = (DSPshort)((ac3_si->dialnorm2 << 8) + (ac3_si->dialnorm));
	ac3i_out->langcod = (DSPshort)((ac3_si->langcod2 << 8) + (ac3_si->langcod));
	ac3i_out->audprod = (DSPshort)((ac3_si->audprodi2e << 15) + (ac3_si->roomtyp2 << 13)
		+ (ac3_si->mixlevel2 << 8) + (ac3_si->audprodie << 7) + (ac3_si->roomtyp << 5) + (ac3_si->mixlevel));

	if (ac3_si->bsid == 6)
	{
		ac3i_out->timecod1 = (DSPshort)((ac3_si->xbsi1e << 14) + (ac3_si->dmixmod << 12)
			+ (ac3_si->ltrtcmixlev << 9) + (ac3_si->ltrtsurmixlev << 6) 
			+ (ac3_si->lorocmixlev << 3) + (ac3_si->lorosurmixlev));
		ac3i_out->timecod2 = (DSPshort)((ac3_si->xbsi2e << 14) + (ac3_si->dsurexmod << 12)
			+ (ac3_si->dheadphonmod << 10) + (ac3_si->adconvtyp << 9) 
			+ (ac3_si->xbsi2 << 1) + (ac3_si->encinfo));
	}
	else  
	{
		ac3i_out->timecod1 = (DSPshort)((ac3_si->timecod1e << 14) + (ac3_si->timecod1));
		ac3i_out->timecod2 = (DSPshort)((ac3_si->timecod2e << 14) + (ac3_si->timecod2));
	}

	/*	Set up return error code */

	if (ac3_si->syncword != SYNC_WD)
	{
		ret_status = AC3I_ERR_SYNC;			/* invalid sync */
	}
	else if (ac3_si->bsid > MAXBSID)
	{
		ret_status = AC3I_ERR_REV;
	}
	else if (ac3_si->fscod == 3)
	{
		ret_status = AC3I_ERR_SAMPRATE;		/* invalid sample rate */
	}
	else if (ac3_si->frmsizecod >= 38)
	{
		ret_status = AC3I_ERR_DATARATE;		/* invalid sample rate */
	}
	else
	{
		ret_status = AC3I_ERR_NONE;			/* no error */
	}

	ret_sip.status = ret_status;
	ret_sip.funcnum = SIP_REV;
	ret_sip.param_ptr = ac3i_out;

	return(ret_sip);
}
