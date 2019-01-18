/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            vo_ac3_var.h
*
* Description: 
*            Packaging all global variables to data structure
*
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    12-19-2008        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/
#ifndef  __VO_AC3_VAR_H__
#define  __VO_AC3_VAR_H__

#include   "vo_ac3_ghdr.h"
#include   "ac3d_equ.h"
#include   "stream.h"
#include   "usr_equ.h"

/* AC-3 data structure declarations */
typedef struct {
	Word16    grpsz;
	Word16    npkgrps;
	Word16    absexp;
	Word16    *pkptr;
	Word16    pkbitptr;
	Word16    pkdata;
} PKEXPS;	

/* AC-3 frame information structure declarations */
typedef struct {
	Word16     size;              /* input parameter list size */
	Word16     *iptr;             /* input buffer pointer */
	Word16     ioff;              /* input offset pointer */
	Word16     imod;              /* input modulo pointer */
	Word16     icfg;              /* input buffer config */	
} AC3_INFO_PL;

typedef struct {
	Word16     size;              /* return parameter list size */
	Word16     bscfg;             /* bitstream configuration */
	Word16     frmsize;           /* frame size */
	Word16     crcsize;           /* First CRC buffer size */
	Word16     bsinfo;            /* bitstream info */
	Word16     dialnorm;          /* dialog normalization values */
	Word16     langcod;           /* language code values */
	Word16     audprod;           /* audio production values */
	Word16     timecod1;          /* time code values, first half */
	Word16     timecod2;          /* time code values, second half */
} AC3_INFO_RL;

/* AC-3 decoder structure declaration */
typedef struct {
	Word16   size;               /* input parameter list size */
	Word16   *iptr;              /* input buffer pointer */
	Word16   ioff;               /* input offset pointer */
	Word16   imod;               /* input module pointer */
	Word16   icfg;               /* input buffer config */
	Word16   *ooff;              /* output packed buffer offset */
	Word16   *omod;              /* output packed buffer module */
	Word16   ocfg;               /* output buffer config */
	Word16   blknum;             /* current block number */
	Word16   rptmax;             /* maximum repeat value before muting */
	Word16   debug;              /* Dolby internal use only */ 
	Word32   **optr;             /* output packed buffer pointer */
	Word32   dynsclh;            /* dynamic range scale high value */
	Word32   dynscll;            /* dynamic range scale low value */
	Word32   pcmscl;             /* pcm scale factor */
	Word32   *dnmxptr;           /* user-specified downmix table */
	Word32   *krkptr;            /* karaoke capable mix/pan parameterw */
} AC3_DEC_PL;

/* CRC calculation structure declarations */
typedef struct {
	Word16   size;               /* input parameter list size */
	Word16   *iptr;              /* input packed buffer pointer */
	Word16   ioff;               /* input packed buffer offset */
	Word16   imod;               /* input packed buffer modulo */
	Word16   icfg;               /* input buffer config */
	Word16   count;              /* CRC word count */
}CRC_CALC_PL;


/* AC3 decoder frame data structure */
typedef struct {
	/*AC-3 decoder syncword information structure, include 40bits*/
	Word16   syncword;            //16bits
	Word16   crc1;                //16bits
	Word16   fscod;	              //2 bits
	Word16   frmsizecod;          //6 bits

	/*AC-3 decoder bitstream information structure,include bit length is variable*/

	Word16   bsid;   		//5bits bitstream identification
	Word16   bsmod;  		//3bits bitstream mode
	Word16   acmod;        		//3bits audio coding mode
	Word16   cmixlev;      		//2bits center mix level
	Word16   surmixlev;    		//2bits surround mix level
	Word16   dsurmod;      		//2bits Dobly surround mode
	Word16   lfeon;        		//1bit  low frequency effect chan flag
	//ch #1
	Word16   dialnorm;     		//5bits dialog normalization word
	Word16   compre;       		//1bit  compression gain word exists
	Word16   compr;        		//8bits compression gain word
	Word16   langcode;     		//1bit  language code exit
	Word16   langcod;      		//8bits language code
	Word16   audprodie;    		//1bit  audio production information exits
	Word16   mixlevel;     		//5bits mixing level
	Word16   roomtyp;      		//2bits room type
	//ch #2
	Word16   dialnorm2;    		//5bits dialog normalization word
	Word16   compr2e;      		//1bit
	Word16   compr2;       		//8bits
	Word16   langcod2e;    		//1bit
	Word16   langcod2;     		//8bits
	Word16   audprodi2e;   		//1bit
	Word16   mixlevel2;    		//5bits
	Word16   roomtyp2;     		//2bits

	Word16   copyrightb;   		//1bit copyright bit
	Word16   origbs;       		//1bit original bit stream
	Word16   timecod1e;            	//1bit time code first half exists
	Word16   timecod1;             	//14bits time code first half
	Word16   timecod2e;            	//1bit time code second half exits
	Word16   timecod2;             	//14bits time code second half
	Word16   addbsie;              	//1bit additional BSI exists
	Word16   addbsil;              	//6bits additional BSI length
	Word16   nfchans;              	//number of full bw channels
	Word16   nchans;               	//number of channels

	/*AC-3 decoder extended bitstream information, contains 1 to 64 bytes*/
	Word16   xbsi1e;                // extra BSI1 info exists
	Word16   dmixmod;               // preferred downmix mode
	Word16   ltrtcmixlev;           // ltrt downmix center mix level
	Word16   ltrtsurmixlev;         // ltrt downmix surround mix level
	Word16   lorocmixlev;           // loro downmix center mix level
	Word16   lorosurmixlev;         // loro downmix surround mix level
	Word16   xbsi2e;                // extra BSI2 info exist
	Word16   dsurexmod;             // surround EX mode flag
	Word16   dheadphonmod;          // Dolby Headphone encoded flag
	Word16   adconvtyp;             // Advanced converter flag
	Word16   xbsi2;                 // Reserved BSI parameters
	Word16   encinfo;               // Encoder Information bit
	Word16   dmixmodd;              // dmixmod defined
	Word16   dsurexmodd;            // dsurexmod defined
	Word16   dheadphonmodd;         // dheadphonmod defined

}AC3_INFO_SI;


typedef struct {
	/* Packed data pointers*/
	Word16   *pkbufptr;             // pointer to start of frame
	Word16   *pkptr;                // packed buffer pointer
	Word16   pkbitptr;              // packed buffer bit pointer
	Word16   pkdata;                // packed biffer data
	Word16   pkmod;                 // packed buffer modulo
	Word16   pkwrdsz;               // packed buffer word size	
}AC3_BSPK;

/*AC-3 decoder Audio block, have six blocks*/
typedef struct {
	Word16   blksw[NFCHANS];        //1bit block switch flags
	Word16   dithflag[NFCHANS];     //1bit dither flags
	//CH#1
	Word16   dynrnge;               //1bit dynamic range word exists
	Word16   dynrng;                //8bit dynamic range word
	//CH#2
	Word16   dynrng2e;              //1bit 
	Word16   dynrng2;               //8bit
	Word16   gainexp;               //gain exponent

	/*Coupling strategy*/
	Word16   cplstre;               //1bit coupling strategy exists
	Word16   cplinu;                //1bit coupling is use flag
	Word16   chincpl[NFCHANS];      //1bit channel in coupling flags
	Word16   phsflginu;             //1bit phase flags in use flag
	Word16   cplbegf;               //4bits coupling begin frequency code
	Word16   cplendf;               //4bits coupling end frequency code
	Word16   cplbndstrc[NCPLBNDS];  //1bit coupling band structure
	Word16   cplchan;               // first coupled channel index
	Word16   cplstrtmant;           // coupling start mantissa
	Word16   cplendmant;            // cuopling end mantissa

	/*Coupling coordinates*/
	Word16   cplcoe[NFCHANS];       //1bit coupling coordinates exit flag
	Word16   mstrcplco;             //2bits master coupling coordinate
	Word16   cplcoexp[NFCHANS][NCPLBNDS];
	//4bits coupling coordinate exps

	//4bits coupling coordinate mants
	Word16   phsflg;                //1bit coupling coordinate phase flag

	/* Phasecor variables */
	Word16   phscore;               //phase corelation info exist flag
	Word16   phsoutmod;             //phscor encoder correction factor
	Word16   phscorstr;             //phscor strategy:per band or not
	Word16   phscor[NCPLBNDS];      //phscor coefficients

	Word16   sclcplcoexp[NCPLBNDS]; //scaled coupling coord exps

	/* Rematrixing*/
	Word16   nrematbnds;            // # rematrixing bands
	Word16   rematstr;              //1bit rematrixing strategy
	Word16   rematflg[NREMATBNDS];  //1bit rematrixing flags
	Word16   rematinu;              //rematrixing in use

	/* Exponent strategy*/
	Word16   cplexpstr;             //2bits coupling packed exp pointer
	Word16   chexpstr[NFCHANS];     //2bits channel exponent strategy
	Word16   lfeexpstr;             //1bit  lfe exponent strategy
	Word16   chbwcod[NFCHANS];      //6bits channel bandwidth code
	Word16   endmant[NFCHANS];      //channel end mantissa

	/* Exponents */
	PKEXPS   cplpkexps;             //coupling packed exp pointer
	PKEXPS   pkexps[NFCHANS];       //channel packed exp pointer
	PKEXPS   lfepkexps;             //lfe packed exp pointer
	Word16   gainrng[NFCHANS];      //gain ranging word
	Word16   appgainrng[NCHANS];    //applied gain ranging word

	/* Bit allocation information*/
	Word16   baie;                  //1bit bit alloc information exists
	Word16   sdcycod;               //2bits slow decay code
	Word16   fdcycod;               //2bits fast decay code
	Word16   sgaincod;              //2bits slow gain code
	Word16   dbpbcod;               //2bits db-per-bit code
	Word16   floorcod;              //3bits floor code
	Word16   snroffste;             //1bit snr offsets exist flag
	Word16   csnroffst;             //6bits coarse snr offset
	Word16   cplfsnroffst;		    /* coupling fine snr offset */
	Word16   fsnroffst[NFCHANS];    //4bits channel fine snr offset
	Word16   lfefsnroffst;          //4bits lfe fine snr offset
	Word16   cplfgaincod;           //3bits coupling fast gain code
	Word16   fgaincod[NFCHANS];     //3bits channel fast gain code
	Word16   lfefgaincod;           //3bits lfe fast gain code
	Word16   cplleake;              //1bit coupling leak terms exist flag
	Word16   cplfleak;              //3bits coupling fast leak term
	Word16   cplsleak;              //3bits coupling slow leak term

	/*Delta bit allocation*/
	Word16   deltbaie;              //1bit delta bit alloc info exists flag
	Word16   cpldeltbae;            //2bits coupling delta bit alloc exists
	Word16   deltbae[NFCHANS];      //2bits channel delta bit alloc exists
	Word16   cpldeltnseg;           //3bits coupling delta # of segments
	Word16   cpldeltoffst[NDELTS];  //5bits coupling delta offset
	Word16   cpldeltlen[NDELTS];    //4bits coupling delta length
	Word16   cpldeltba[NDELTS];     //3bits coupling bit alloc
	Word16   chdeltnseg[NFCHANS];   //channel delta #of segments
	Word16   chdeltoffst[NFCHANS][NDELTS];
	//5bits channel delta offset
	Word16   chdeltlen[NFCHANS][NDELTS];  //4bits channel delta length
	Word16   chdeltba[NFCHANS][NDELTS];   //3bits channel delta bit alloc

	/* Skip field*/
	Word16   skiple;                      //1bit skip length exists flag
	Word16   skipl;                       //9bits skip length

	/* Mantissa unpacking control variables */
	Word16   count3;                      //3-level grouped mantissa count
	Word16   count5;                      //5-level grouped mantissa count
	Word16   count11;                     //11-level grouped mantissa count
	UWord16  mant3;                       //3-level grouped mantissa
	UWord16  mant5;                       //5-level grouped mantissa
	UWord16  mant11;                      //11-level grouped mantissa
	Word16   chcount3;                    //saved channel 3-level count
	Word16   chcount5;                    //saved channel 5-level count
	Word16   chcount11;                   //saved channel 11-level count
	UWord16  chmant3;                     //saved channel 3-level mantissa
	UWord16  chmant5;                     //saved channel 5-level mantissa
	UWord16  chmant11;                    //saved channel 11-level mantissa
	Word16   *chpkptr;                    //saved channel packed pointer
	Word16   chpkbitptr;                  //saved channel packed bit pointer
	Word16   chpkdata;                    //saved channel packed data
	Word16   cplcount3;                   //saved coupling 3-level count
	Word16   cplcount5;                   //saved coupling 5-level count
	Word16   cplcount11;                  //saved coupling 11-level count
	UWord16  cplmant3;                    //saved coupling 3-level mantissa
	UWord16  cplmant5;                    //saved coupling 5-level mantissa
	UWord16  cplmant11;                   //saved coupling 11-levell mantissa
	Word16   *cplpkptr;                    //saved coupling packed pointer
	Word16   cplpkbitptr;                 //saved coupling packed bit ptr
	Word16   cplpkdata;                   //saved coupling packed data

	/* Bit allocation control varialbes */
	Word16   sdecay;                      //slow decay value
	Word16   fdecay;                      //fast decay value
	Word16   sgain;                       //slow gain value
	Word16   fgain;                       //fast gain value
	Word16   dbknee;                      //db-per-bit value
	Word16   floorval;                    //floor value
	Word16   snroff;                      //snr offsset
	Word16   minsnrflg;                   //minimum snr offset flag

	Word16   bandindex;                   //bit allocation band index
	Word16   mantindex;                   //mantissa index
	Word16   lastmant;                    //last mantissa index
	Word16   dith;                        //dither flag
	Word16   cplflag;                     //coupling flag

	Word16   *cplcoexpptr;                //coupling coord exponent pointer
	Word16   cplcnt;                      //coupling band count
	Word16   fleak;                       //fast leak value
	Word16   sleak;                       //slow leak value
	Word16   dnmixbufinu[NCHANS];  
	/* Downmixing*/
	Word32   dnmixtab[NPCMCHANS][NPCMCHANS];    /*downmixing matrix*/
	Word32   compfact;              	      //compression factor
	Word32   compfact2;             	      //compression factor #2
	Word32   cplco[NFCHANS][NCPLBNDS];
	Word32   gainmant;             		      //gain mantissa
	Word32   sclcplco[NCPLBNDS];    	      //scaled coupling coords
	Word32   *tcbuf;                          //transform coeff buffer pointer
	Word32   *cplcoptr;                       //coupling coord mantissa pointer

}AC3_AB_DATA;

typedef struct {
	AC3_BSPK          *ac3_str;
	AC3_INFO_SI       *ac3_si;
	AC3_AB_DATA       *ac3_aub;
	/* Miscellaneous variables */
	Word16      blknum;
	Word16      channum;
	Word16      bswitch;
	Word16      rptcnt;
	Word16      dithreg;                   /*random number register*/

//#ifndef  DITHBND
#if 0
	Word32      dithscale;                 /* dither scaling factor*/
#endif
	/* SIP control variable */
	//AC3_DEC_PL  *ac3_in_ptr;
	//AC3_DEC_PL  ac3_in;
	//AC3_INFO_RL ac3i_out;
	Word16      in_stat;
	Word16      out_stat;
	Word16      ac3raminit;

	/* User-specified variables*/
	Word16      outmod;
	Word16      outlfe;
	Word16      outnchans;
	Word16      lastoutmod;
	Word16      lastoutlfe;
	Word16      pcmbufoff[NCHANS];
	Word16      pcmbufmod[NCHANS];
	Word16      compmod;
	Word16      digdialnorm;
	Word16      dualmod;
	Word16      stereomod;
	/* Half sample-rate variables */
#if 1
//#ifdef   HALFRATE
	Word16      halfratecod;
#endif
	Word32      useverbose;
	/* Debug variables */
#if 1
//#ifdef   KAWARE
	Word16      karaokeflag;                 /* karaoke bistream flag */
#if 1
//#ifdef   KCAPABLE
	Word32      *kcapableptr;
#endif
#endif
	Word32      dynscalehigh;
	Word32      dynscalelow;
	Word32      pcmscale;
	Word32      *ext_dnmixtab;
	Word32      *pcmbufptr[NCHANS];

}DEC_PARAM;

typedef struct {
	/* Transform buffers */
	Word32      fftbuf[N];                	/* fft/window working buffer*/
	Word32      tc1[N];                 	/* transform coeff buffer #1*/
	Word32      tc2[N];                 	/* transform coeff buffer #2*/
	Word32      dnmix_buf[NCHANS][N];   	/* downmix buffers */
	Word32      delay_buf[NCHANS][N/2]; 	/* delay buffers */
	Word32      mantbuf[BANDBUFSZ];        /* may be overlayed with fftbuf*/

	/* Overlay buffers */
	Word16      psdbuf[BANDBUFSZ];         /* may be overlayed with fftbuf*/
#if 1
//#ifdef   MIPOPT
	Word16      *expbuf;                   /* pointer to exponents */
	Word16      cplexps[NMANTS];           /* unpacked cpl exponents */
	Word16      exps[NFCHANS][NMANTS];     /* unpacked exponents */
	Word16      lfeexps[NLFEMANTS];        /* unpacked lfe exponent */
	Word16      *bapbuf;                   /* pointer to bit allocation */
	Word16      newbitalloc;               /* new bit allocation flag */
	Word16      cplbap[NMANTS];            /* cpl bit allocation */
	Word16      bap[NFCHANS][NMANTS];      /* bit allocation */
	Word16      lfebap[NLFEMANTS];         /* lfe bit allocation */
#else
	Word16      expbuf[N];                 /* may be overlayed with tc1/tc2*/
#endif
	Word16      deltary[NBABNDS];          /* may be overlayed with fftbuf*/
}BUFF_PARAM;

typedef struct {
	Word16      funcnum;                   /* function number */
	Word16      status;                    /* error status */
	void        *param_ptr;                /* pointer to parameter list structure*/
}DOLBY_SIP;

typedef struct {
	DEC_PARAM           *decparam;
	AC3D_CONF_PARAM     *inparam;
	BUFF_PARAM          *ac3_buff;
	AC3_INFO_PL         *ac3_info_list;
	AC3_INFO_RL         *ac3_info_rptr;
	CRC_CALC_PL         *crc_calc_list;
	AC3_DEC_PL          *ac3_dec_list;
	FrameStream			*stream;
	VO_MEM_OPERATOR     *pvoMemop;
	VO_MEM_OPERATOR     voMemoprator;
	/* input & output buffer pointer */
	unsigned char       *pk_buf;
	Word32              input_length;
	Word16              output_length;
	//Word16              work_buf[NOUTWORDS];
	//char                errstr[80];
	void                *pContext;
	Word32              *pcm_buf;
    Word32              *output_buf;
	void                *hCheck;
	Word32              nEndianFlag;       //0 = little endian, 1 = big endian
}DECEXEC;

#endif //__VO_AC3_VAR_H__
