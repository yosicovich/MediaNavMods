/* $Header: //depot/licensing/dddec/c/v7.1.0/AC3SUBD/AC3D_VAR.C#1 $ */

/****************************************************************************
;	Unpublished work.  Copyright 1993-2002 Dolby Laboratories, Inc.
;	All Rights Reserved.
;
;	project:				Dolby Digital Decoder
;	contents/description:	AC-3 decoder common data file
;***************************************************************************/

#include "ac3d_com.h"

/*	Transform buffers */

DSPfract fftbuf[N];						/* fft/window working buffer		*/
DSPfract tc1[N];						/* transform coeff buffer #1		*/
DSPfract tc2[N];						/* transform coeff buffer #2		*/
DSPfract dnmix_buf[NCHANS][N];			/* downmix buffers					*/
DSPfract delay_buf[NCHANS][N/2];		/* delay buffers					*/

/*	Overlay buffers */

DSPshort psdbuf[BANDBUFSZ];				/* may be overlayed with fftbuf		*/
DSPfract mantbuf[BANDBUFSZ];			/* may be overlayed with fftbuf		*/
#ifdef MIPOPT
DSPshort *expbuf;						/* pointer to exponents				*/
DSPshort cplexps[NMANTS];				/* unpacked cpl exponents			*/
DSPshort exps[NFCHANS][NMANTS];			/* unpacked exponents				*/
DSPshort lfeexps[NLFEMANTS];			/* unpacked lfe exponents			*/
DSPshort *bapbuf;						/* pointer to bit allocation		*/
DSPshort newbitalloc;					/* new bit allocation flag			*/
DSPshort cplbap[NMANTS];				/* cpl bit allocation				*/
DSPshort bap[NFCHANS][NMANTS];			/* bit allocation					*/
DSPshort lfebap[NLFEMANTS];				/* lfe bit allocation				*/
#else
DSPshort expbuf[N];						/* may be overlayed with tc1/tc2	*/
#endif
DSPshort deltary[NBABNDS];				/* may be overlayed with fftbuf		*/

/*	Sync info */

DSPshort syncword;						/* synchronization word				*/
DSPshort crc1;							/* first CRC word (start of frame)	*/
DSPshort fscod;							/* sampling frequency code			*/
DSPshort frmsizecod;					/* frame size code					*/

/*	Bit stream info */

DSPshort bsid;							/* bitstream identification			*/
DSPshort bsmod;							/* bitstream mode					*/
DSPshort acmod;							/* audio coding mode				*/
DSPshort cmixlev;						/* center mix level					*/
DSPshort surmixlev;						/* surround mix level				*/
DSPshort dsurmod;						/* Dolby surround mode				*/
DSPshort lfeon;							/* low frequency effects chan flag	*/
DSPshort dialnorm;						/* dialog normalization word		*/
DSPshort compre;						/* channel 3 comp word exists		*/
DSPshort compr;							/* channel 3 comp word 				*/
DSPshort langcode;						/* language code exists				*/
DSPshort langcod;						/* language code					*/
DSPshort audprodie;						/* audio production info exists		*/
DSPshort mixlevel;						/* mixing level						*/
DSPshort roomtyp;						/* room type						*/
DSPshort dialnorm2;						/* dialog normalization word #2		*/
DSPshort compr2e;						/* channel 3 comp word #2 exists	*/
DSPshort compr2;						/* channel 3 comp word #2 			*/
DSPshort langcod2e;						/* language code #2 exists			*/
DSPshort langcod2;						/* language code #2					*/
DSPshort audprodi2e;					/* audio production info #2 exists	*/
DSPshort mixlevel2;						/* mixing level #2					*/
DSPshort roomtyp2;						/* room type #2						*/
DSPshort copyrightb;					/* copyright bit					*/
DSPshort origbs;						/* original bitstream flag			*/
DSPshort timecod1e;						/* time code first half exists		*/
DSPshort timecod1;						/* time code first half				*/
DSPshort timecod2e;						/* time code second half exists		*/
DSPshort timecod2;						/* time code second half			*/
DSPshort addbsie;						/* additional BSI exists			*/
DSPshort addbsil;						/* additional BSI length			*/
DSPshort nfchans;						/* number of full bw channels		*/
DSPshort nchans;						/* number of channels				*/

/*  Extended Bitstream Parameters */
DSPshort xbsi1e;						/* extra BSI1 info exists			*/
DSPshort dmixmod;						/* preferred downmix mode			*/
DSPshort ltrtcmixlev;					/* ltrt downmix center mix level	*/
DSPshort ltrtsurmixlev;					/* ltrt downmix surround mix level	*/
DSPshort lorocmixlev;					/* loro downmix center mix level	*/
DSPshort lorosurmixlev;					/* loro downmix surround mix level	*/
DSPshort xbsi2e;						/* extra BSI2 info exists			*/		
DSPshort dsurexmod;						/* Surround EX mode flag			*/
DSPshort dheadphonmod;					/* Dolby Headphone encoded flag		*/
DSPshort adconvtyp;						/* Advanced converter flag			*/
DSPshort xbsi2;							/* Reserved BSI parameters			*/
DSPshort encinfo;						/* Encoder Information bit			*/

DSPshort dmixmodd;						/* dmixmod defined					*/
DSPshort dsurexmodd;					/* dsurexmod defined				*/
DSPshort dheadphonmodd;					/* dheadphonmod defined				*/

/*	Audio block */

DSPshort blksw[NFCHANS];				/* block switch flags				*/
DSPshort dithflag[NFCHANS];				/* dither flags						*/
DSPshort dynrnge;						/* dynamic range word exists		*/
DSPshort dynrng;						/* dynamic range word				*/
DSPshort dynrng2e;						/* dynamic range word #2 exists		*/
DSPshort dynrng2;						/* dynamic range word #2			*/
DSPfract compfact;						/* compression factor				*/
DSPfract compfact2;						/* compression factor #2			*/
DSPshort gainexp;						/* gain exponent					*/
DSPfract gainmant;						/* gain mantissa					*/

/*	Coupling strategy */

DSPshort cplstre;						/* coupling strategy exists			*/
DSPshort cplinu;						/* coupling in use flag				*/
DSPshort chincpl[NFCHANS];				/* channel in coupling flags		*/
DSPshort phsflginu;						/* phase flags in use flag			*/
DSPshort cplbegf;						/* coupling begin frequency code	*/
DSPshort cplendf;						/* coupling end frequency code		*/
DSPshort cplbndstrc[NCPLBNDS];			/* coupling band structure			*/
DSPshort cplchan;						/* first coupled channel index		*/
DSPshort cplstrtmant;					/* coupling start mantissa			*/
DSPshort cplendmant;					/* coupling end mantissa			*/

/*	Coupling coordinates */

DSPshort cplcoe[NFCHANS];				/* coupling coordinates exist flag	*/
DSPshort mstrcplco;						/* master coupling coordinate		*/
DSPshort cplcoexp[NFCHANS][NCPLBNDS];	/* coupling coordinate exps			*/
DSPfract cplco[NFCHANS][NCPLBNDS];		/* coupling coordinate mants		*/
DSPshort phsflg;						/* coupling coordinate phase flag	*/

/*	Phasecor variables */

DSPshort phscore;						/* phase corelation info exist flag	*/
DSPshort phsoutmod;						/* phscor encoder correction factor */
DSPshort phscorstr;						/* phscor strategy: per band or not */
DSPshort phscor[NCPLBNDS];				/* phscor coefficients				*/
DSPfract sclcplco[NCPLBNDS];			/* scaled coupling coords			*/
DSPshort sclcplcoexp[NCPLBNDS];			/* scaled coupling coord exps		*/

/*	Rematrixing */

DSPshort nrematbnds;					/* # rematrixing bands				*/
DSPshort rematstr;						/* rematrixing strategy				*/
DSPshort rematflg[NREMATBNDS];			/* rematrixing flags				*/
DSPshort rematinu;						/* rematrixing in use				*/

/*	Exponent strategy */

DSPshort cplexpstr;						/* coupling exponent strategy		*/
DSPshort chexpstr[NFCHANS];				/* channel exponent strategy 		*/
DSPshort lfeexpstr;						/* lfe exponent strategy			*/
DSPshort chbwcod[NFCHANS];				/* channel bandwidth code			*/
DSPshort endmant[NFCHANS];				/* channel end mantissa				*/

/*	Exponents */

PKEXPS cplpkexps;						/* coupling packed exp pointer		*/
PKEXPS pkexps[NFCHANS];					/* channel packed exp pointer		*/
PKEXPS lfepkexps;						/* lfe packed exp pointer			*/
DSPshort gainrng[NFCHANS];				/* gain ranging word				*/
DSPshort appgainrng[NCHANS];			/* applied gain ranging word		*/

/*	Bit allocation information */

DSPshort baie;							/* bit alloc information exists		*/
DSPshort sdcycod;						/* slow decay code					*/
DSPshort fdcycod;						/* fast decay code					*/
DSPshort sgaincod;						/* slow gain code					*/
DSPshort dbpbcod;						/* db-per-bit code					*/
DSPshort floorcod;						/* floor code						*/

DSPshort snroffste;						/* snr offsets exist flag			*/
DSPshort csnroffst;						/* coarse snr offset				*/
DSPshort cplfsnroffst;					/* coupling fine snr offset			*/
DSPshort fsnroffst[NFCHANS];			/* channel fine snr offset			*/
DSPshort lfefsnroffst;					/* lfe fine snr offset				*/
DSPshort cplfgaincod;					/* coupling fast gain code			*/
DSPshort fgaincod[NFCHANS];				/* channel fast gain code			*/
DSPshort lfefgaincod;					/* lfe fast gain code				*/

DSPshort cplleake;						/* coupling leak terms exist flag	*/
DSPshort cplfleak;						/* coupling fast leak term			*/
DSPshort cplsleak;						/* coupling slow leak term			*/

/*	Delta bit allocation */

DSPshort deltbaie;						/* delta bit alloc info exists flag	*/
DSPshort cpldeltbae;					/* coupling delta bit alloc exists	*/
DSPshort deltbae[NFCHANS];				/* channel delta bit alloc exists	*/
DSPshort cpldeltnseg;					/* coupling delta # of segments		*/
DSPshort cpldeltoffst[NDELTS];			/* coupling delta offset			*/
DSPshort cpldeltlen[NDELTS];			/* coupling delta length			*/
DSPshort cpldeltba[NDELTS];				/* coupling delta bit alloc			*/
DSPshort chdeltnseg[NFCHANS];			/* channel delta # of segments		*/
DSPshort chdeltoffst[NFCHANS][NDELTS];	/* channel delta offset				*/
DSPshort chdeltlen[NFCHANS][NDELTS];	/* channel delta length				*/
DSPshort chdeltba[NFCHANS][NDELTS];		/* channel delta bit alloc			*/

/*	Skip field */

DSPshort skiple;						/* skip length exists flag			*/
DSPshort skipl;							/* skip length						*/

/*	Packed data pointers */

DSPshort *pkbufptr;						/* pointer to start of frame		*/
DSPshort *pkptr;						/* packed buffer pointer			*/
DSPshort pkbitptr;						/* packed buffer bit pointer		*/
DSPshort pkdata;						/* packed buffer data				*/
DSPshort pkmod;							/* packed buffer modulo				*/
DSPshort pkwrdsz;						/* packed buffer word size			*/

/*	Mantissa unpacking control variables */

DSPshort count3;						/* 3-level grouped mantissa count	*/
DSPshort count5;						/* 5-level grouped mantissa count	*/
DSPshort count11;						/* 11-level grouped mantissa count	*/
DSPushort mant3;							/* 3-level grouped mantissa			*/
DSPushort mant5;							/* 5-level grouped mantissa			*/
DSPushort mant11;						/* 11-level grouped mantissa		*/
DSPshort chcount3;						/* saved channel 3-level count		*/
DSPshort chcount5;						/* saved channel 5-level count		*/
DSPshort chcount11;						/* saved channel 11-level count		*/
DSPushort chmant3;						/* saved channel 3-level mantissa	*/
DSPushort chmant5;						/* saved channel 5-level mantissa	*/
DSPushort chmant11;						/* saved channel 11-level mantissa	*/
DSPshort *chpkptr;						/* saved channel packed pointer		*/
DSPshort chpkbitptr;					/* saved channel packed bit pointer	*/
DSPshort chpkdata;						/* saved channel packed data		*/
DSPshort cplcount3;						/* saved coupling 3-level count		*/
DSPshort cplcount5;						/* saved coupling 5-level count		*/
DSPshort cplcount11;					/* saved coupling 11-level count	*/
DSPushort cplmant3;						/* saved coupling 3-level mantissa	*/
DSPushort cplmant5;						/* saved coupling 5-level mantissa	*/
DSPushort cplmant11;					/* saved coupling 11-level mantissa	*/
DSPshort *cplpkptr;						/* saved coupling packed pointer	*/
DSPshort cplpkbitptr;					/* saved coupling packed bit ptr	*/
DSPshort cplpkdata;						/* saved coupling packed data		*/

/*	Bit allocation control variables */

DSPshort sdecay;						/* slow decay value					*/
DSPshort fdecay;						/* fast decay value					*/
DSPshort sgain;							/* slow gain value					*/
DSPshort fgain;							/* fast gain value					*/
DSPshort dbknee;						/* db-per-bit value					*/
DSPshort floorval;						/* floor value						*/
DSPshort snroff;						/* snr offset						*/
DSPshort minsnrflg;						/* minimum snr offset flag			*/

DSPfract *tcbuf;						/* transform coeff buffer pointer	*/
DSPshort bandindex;						/* bit allocation band index		*/
DSPshort mantindex;						/* mantissa index					*/
DSPshort lastmant;						/* last mantissa index				*/
DSPshort dith;							/* dither flag						*/
DSPshort cplflag;						/* coupling flag					*/
DSPfract *cplcoptr;						/* coupling coord mantissa pointer	*/
DSPshort *cplcoexpptr;					/* coupling coord exponent pointer	*/
DSPshort cplcnt;						/* coupling band count				*/
DSPshort fleak;							/* fast leak value					*/
DSPshort sleak;							/* slow leak value					*/

/*	Downmixing */

DSPfract dnmixtab[NPCMCHANS][NPCMCHANS]; /* downmixing matrix				*/
DSPshort dnmixbufinu[NCHANS];

/*	Miscellaneous variables */

DSPshort blknum;
DSPshort channum;
DSPshort bswitch;
DSPshort rptcnt;
DSPshort dithreg;						/* random number register			*/

#ifndef DITHBND
DSPfract dithscale;						/* dither scaling factor			*/
#endif

/*	SIP control variables */

AC3_DEC_PL *ac3_in_ptr;
//AC3_DEC_PL ac3_in;
AC3_INFO_RL ac3i_out;
DSPshort out_stat;
DSPshort ac3raminit;

/*	User-specified variables */

DSPshort outmod;
DSPshort outlfe;
DSPshort outnchans;
DSPshort lastoutmod;
DSPshort lastoutlfe;
DSPfract *pcmbufptr[NCHANS];
DSPshort pcmbufoff[NCHANS];
DSPshort pcmbufmod[NCHANS];
DSPshort compmod;
DSPshort digdialnorm;
DSPshort dualmod;
DSPshort stereomod;

DSPfract dynscalehigh;
DSPfract dynscalelow;
DSPfract pcmscale; 
DSPfract *ext_dnmixtab;

/*	Debug variables */

#ifdef DEBUG
DSPshort debuglev;
char dbgstr[80];
#endif

/* karaoke variables */

#ifdef KAWARE
DSPshort karaokeflag;				/* karaoke bitstream flag */
#ifdef KCAPABLE
DSPfract *kcapableptr;
#endif
#endif

/*	Half sample-rate variables */

#ifdef HALFRATE
DSPshort halfratecod;
#endif
