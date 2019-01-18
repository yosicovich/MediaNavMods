/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/UPK_SUBS.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Data unpacking subroutines
;***************************************************************************/
#include "ac3d_com.h"

#ifndef AC3D_ARM_OPT

extern DSPushort msktab[MAXDSPSHIFT+1];

void bitunp_lj(DSPshort *dataptr, DSPshort numbits, AC3_BSPK *p_bstrm)
{
	DSPushort data;
	*dataptr = (DSPshort)((p_bstrm->pkdata << p_bstrm->pkbitptr) & msktab[numbits]);
	p_bstrm->pkbitptr += numbits;

	if (p_bstrm->pkbitptr >= p_bstrm->pkwrdsz)
	{
		p_bstrm->pkptr++;
		p_bstrm->pkdata = *(p_bstrm->pkptr);
		p_bstrm->pkbitptr -= p_bstrm->pkwrdsz;
		data = (DSPushort)p_bstrm->pkdata;
		*dataptr |= ((data >> (numbits - p_bstrm->pkbitptr)) & msktab[numbits]);
	}
	*dataptr = SGN_EXTND(*dataptr);
}
void bitunp_rj(DSPshort *dataptr, DSPshort numbits, AC3_BSPK *p_bstrm)
{
	DSPushort data;
	*dataptr = (DSPshort)((p_bstrm->pkdata << p_bstrm->pkbitptr) & msktab[numbits]);
	p_bstrm->pkbitptr += numbits;	
	if (p_bstrm->pkbitptr >= p_bstrm->pkwrdsz)
	{
		p_bstrm->pkptr++;
		p_bstrm->pkdata = *(p_bstrm->pkptr);
		p_bstrm->pkbitptr -= p_bstrm->pkwrdsz;
		data = (DSPushort)p_bstrm->pkdata;
		*dataptr |= ((data >> (numbits - p_bstrm->pkbitptr)) & msktab[numbits]);
	}
	*dataptr = (DSPshort)((DSPushort)(*dataptr) >> (p_bstrm->pkwrdsz - numbits));
}
#endif
void bitskip(DSPshort numbits, AC3_BSPK *p_bstrm)
{
	p_bstrm->pkbitptr += numbits;
	while (p_bstrm->pkbitptr >= p_bstrm->pkwrdsz)
	{
		p_bstrm->pkptr++;
		p_bstrm->pkdata = *p_bstrm->pkptr;
		p_bstrm->pkbitptr -= p_bstrm->pkwrdsz;
	}
}
 //AC3D_ARM_OPT

