/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/DSP_MISC.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	Miscellaneous DSP-specific routines
;***************************************************************************/

#include "dolbycom.h"

#ifdef DSPPREC
DSPfract DSPrnd(DSPshort type, DSPshort bits, DSPfract a)
{
	long maxval, longval;
	DSPfract fractval;

	if ((bits == 0) || (bits < -1))
	{
		error_msg("DSPrnd: illegal word width", FATAL);
	}
	if (bits != -1)
	{
		maxval = 1L << (bits - 1);
		if (maxval == 0)
		{
			error_msg("DSPrnd: illegal word width", FATAL);
		}

		if (type == NONCONV)			/* non-convergent rounding */
		{
			fractval = a * maxval + 0.5;
			longval = (long)fractval;
			if ((DSPfract)longval > fractval)
			{
				longval--;
			}
			if (longval > (maxval - 1))
			{
				if (longval > maxval)
				{
					error_msg("DSPrnd: positive rounding occurred", WARNING);
				}
				longval = maxval - 1;
			}
			else if (longval < (-maxval))
			{
				error_msg("DSPrnd: negative rounding occurred", WARNING);
				longval = -maxval;
			}
			return ((DSPfract)longval / maxval);
		}
		else if (type == CONV)			/* convergent rounding */
		{
			fractval = a * maxval;
			longval = (long)fractval;
			if (fractval >= 0)
			{
				if (((fractval - longval) > 0.5) ||
					(((fractval - longval) == 0.5) && (longval % 2)))
				{
					longval++;
				}
				if (longval > (maxval - 1))
				{
					if (longval > maxval)
					{
						error_msg("DSPrnd: positive rounding occurred",
							WARNING);
					}
					longval = maxval - 1;
				}
			}
			else
			{
				if (longval != fractval)
				{
					longval--;
				}
				if (((fractval - longval) > 0.5) ||
					(((fractval - longval) == 0.5) && (longval % 2)))
				{
					longval++;
				}
				if (longval < (-maxval))
				{
					error_msg("DSPrnd: negative rounding occurred", WARNING);
					longval = -maxval;
				}
			}
			return((DSPfract)longval / maxval);
		}
		else if (type == TRUNC)			/* truncation */
		{
			fractval = a * maxval;
			longval = (long)fractval;
			if ((DSPfract)longval > fractval)
			{
				longval--;
			}
			if (longval > (maxval - 1))
			{
				if (longval > maxval)
				{
					error_msg("DSPrnd: positive rounding occurred", WARNING);
				}
				longval = maxval - 1;
			}
			else if (longval < (-maxval))
			{
				error_msg("DSPrnd: negative rounding occurred", WARNING);
				longval = -maxval;
			}
			return ((DSPfract)longval / maxval);
		}
		else							/* unknown rounding type */
		{
			error_msg("DSPrnd: unknown rounding type", FATAL);
		}
	}
	else
	{
		if (a > MAXDSPFRACT)
		{
			if (a > 1.0)
			{
				error_msg("DSPrnd: positive rounding occurred", WARNING);
			}
			a = MAXDSPFRACT;
		}
		else if (a < MINDSPFRACT)
		{
			a = MINDSPFRACT;
			error_msg("DSPrnd: negative rounding occurred", WARNING);
		}
		return(a);
	}
}
#endif

DSPshort DSPsat(DSPshort a, DSPshort b)	/* DSP saturated addition */
{
	long sum;

	sum = ((long)(a)) + ((long)(b));
	if (sum > MAXDSPSHORT - 1)
	{
		sum = MAXDSPSHORT - 1;
	}
	else if (sum < -MAXDSPSHORT)
	{
		sum = -MAXDSPSHORT;
	}
	return((DSPshort)sum);
}

/*
	Normalization:  y = min(trunc(-log2(mant)), maxnorm)
*/
//DSPnorm function fixed
DSPshort Vo_DSPnorm(int arg, DSPshort maxnorm)
{
	DSPshort ex;
    long     temp;
	temp = (long)arg;
	
	if (temp == 0)
	{
		return(maxnorm);
	}
	else
	{
		ex = 0;
		if ((temp > 0) && (temp < 0x7fffffffL))
		{
			while (temp < 0x40000000L)
			{
				temp <<= 1;
				ex++;
				if (!(--maxnorm)) break;
			}
		}
		else if ((temp < 0) && (temp >= (int)0x80000000L))
		{
			while (temp >= (int)0xC0000000L)
			{
				temp <<= 1;
				ex++;
				if (!(--maxnorm)) break;
			}
		}
		else if ((temp > 0) && (temp >= 0x7fffffffL))
		{
			while (temp >= 0x7fffffffL)
			{
				temp >>= 1;
				ex--;
				if (!(--maxnorm)) break;
			}
		}
		else	/* ((arg < 0.0) && (arg < -1.0)) */
		{
			while (temp < (int)0x80000000L)
			{
				temp >>= 1;
				ex--;
				if (!(--maxnorm)) break;
			}
		}
		return(ex);
	}
}


DSPshort DSPnorm(DSPfract arg, DSPshort maxnorm)
{
	DSPshort ex;

	if (arg == 0.0)
	{
		return(maxnorm);
	}
	else
	{
		ex = 0;
		if ((arg > 0.0) && (arg < 1.0))
		{
			while (arg < 0.5)
			{
				arg *= 2.0;
				ex++;
				if (!(--maxnorm)) break;
			}
		}
		else if ((arg < 0.0) && (arg >= -1.0))
		{
			while (arg >= -0.5)
			{
				arg *= 2.0;
				ex++;
				if (!(--maxnorm)) break;
			}
		}
		else if ((arg > 0.0) && (arg >= 1.0))
		{
			while (arg >= 1.0)
			{
				arg *= 0.5;
				ex--;
				if (!(--maxnorm)) break;
			}
		}
		else	/* ((arg < 0.0) && (arg < -1.0)) */
		{
			while (arg < -1.0)
			{
				arg *= 0.5;
				ex--;
				if (!(--maxnorm)) break;
			}
		}
		return(ex);
	}
}
