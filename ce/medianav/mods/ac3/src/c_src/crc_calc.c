/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/CRC_CALC.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	CRC calculation routines
;***************************************************************************/
#include "crc_com.h"

#ifndef AC3D_ARM_OPT
extern DSPushort crctab[CRCTABSZ];

DOLBY_SIP crc_calc(DOLBY_SIP input_sip)
{
	DOLBY_SIP ret_sip;
	DSPshort syndrome;
	CRC_CALC_PL *crc_in_ptr;
	DSPshort *bufptr, buflen;
	int i;

	syndrome = input_sip.status;
	crc_in_ptr = input_sip.param_ptr;
	bufptr = crc_in_ptr->iptr;
	buflen = crc_in_ptr->count;

	for (i = 0; i < buflen; i++)
	{
		syndrome = (DSPshort)(((syndrome << 8) & 0xff00)
			^ ((bufptr[i] >> 8) & 0xff) ^ crctab[(syndrome >> 8) & 0xff]);
		syndrome = (DSPshort)(((syndrome << 8) & 0xff00)
			^ (bufptr[i] & 0xff) ^ crctab[(syndrome >> 8) & 0xff]);
	}

	ret_sip.funcnum = SIP_REV;
	ret_sip.status = syndrome;
	ret_sip.param_ptr = NULLPTR;
	return(ret_sip);
}
#endif

