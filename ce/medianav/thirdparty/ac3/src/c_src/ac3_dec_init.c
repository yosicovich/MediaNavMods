/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3_dec_init.c
*
* Project:
* contents/description: ac3 decoder data structure init 
*            
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    12-19-2008        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "ac3_dec_init.h"
#include "mem_align.h"

/* init AC3 decoder some structure */
Word16   init_buff_param(BUFF_PARAM  **ac3_buff, VO_MEM_OPERATOR *pMemOP)
{
	BUFF_PARAM    *st;
	if(ac3_buff == (BUFF_PARAM **) NULL)
	{
		return -1;
	}
	*ac3_buff = NULL;
	/* allocate memory */
	if((st = (BUFF_PARAM *) voAC3Dec_mem_malloc(pMemOP, sizeof(BUFF_PARAM), 32)) == NULL)
	{
		return -1;
	}

	*ac3_buff = st;
	return 0;
}

Word16  init_ac3_dec_pl(AC3_DEC_PL  **dec_pl, VO_MEM_OPERATOR *pMemOP)
{
	AC3_DEC_PL    *st;
	if(dec_pl == (AC3_DEC_PL **) NULL)
	{
		return -1;
	}
	*dec_pl = NULL;
	/* allocate memory */
	if((st = (AC3_DEC_PL *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_DEC_PL), 32)) == NULL)
	{
		return -1;
	}

	*dec_pl = st;
	return 0;
}

Word16   init_ac3_info_pl(AC3_INFO_PL  **info_pl, VO_MEM_OPERATOR *pMemOP)
{
	AC3_INFO_PL    *st;
	if(info_pl == (AC3_INFO_PL **) NULL)
	{
		return -1;
	}
	*info_pl = NULL;
	/* allocate memory */
	if((st = (AC3_INFO_PL *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_INFO_PL), 32)) == NULL)
	{
		return -1;
	}

	*info_pl = st;
	return 0;
}

Word16   init_ac3_info_rl(AC3_INFO_RL  **info_rl, VO_MEM_OPERATOR *pMemOP)
{
	AC3_INFO_RL    *st;
	if(info_rl == (AC3_INFO_RL **) NULL)
	{
		return -1;
	}
	*info_rl = NULL;
	/* allocate memory */
	if((st = (AC3_INFO_RL *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_INFO_RL), 32)) == NULL)
	{
		return -1;
	}
	*info_rl = st;

	return 0;
}

Word16   init_crc_calc_pl(CRC_CALC_PL  **calc_pl, VO_MEM_OPERATOR *pMemOP)
{
	CRC_CALC_PL    *st;
	if(calc_pl == (CRC_CALC_PL **) NULL)
	{
		return -1;
	}
	*calc_pl = NULL;
	/* allocate memory */
	if((st = (CRC_CALC_PL *) voAC3Dec_mem_malloc(pMemOP, sizeof(CRC_CALC_PL), 32)) == NULL)
	{
		return -1;
	}
	*calc_pl = st;
	return 0;
}

Word16  init_ac3_info_si(AC3_INFO_SI **ac3_si, VO_MEM_OPERATOR *pMemOP)
{
	AC3_INFO_SI    *st;
	if(ac3_si == (AC3_INFO_SI **) NULL)
	{
		return -1;
	}
	*ac3_si = NULL;
	/* allocate memory */
	if((st = (AC3_INFO_SI *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_INFO_SI), 32)) == NULL)
	{
		return -1;
	}
	*ac3_si = st;
	return 0;
}


Word16   init_ac3_aub(AC3_AB_DATA **ac3_aub, VO_MEM_OPERATOR *pMemOP)
{
	AC3_AB_DATA   *st;
	if(ac3_aub == (AC3_AB_DATA **) NULL)
	{
		return -1;
	}
	*ac3_aub = NULL;
	/* allocate memory */
	if((st = (AC3_AB_DATA *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_AB_DATA), 32)) == NULL)
	{
		return -1;
	}
	*ac3_aub = st;
	return 0;
}

Word16  init_ac3_str(AC3_BSPK **ac3_str, VO_MEM_OPERATOR *pMemOP)
{
	AC3_BSPK    *st;
	if(ac3_str == (AC3_BSPK **) NULL)
	{
		return -1;
	}
	*ac3_str = NULL;
	/* allocate memory */
	if((st = (AC3_BSPK *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3_BSPK), 32)) == NULL)
	{
		return -1;
	}
	st->pkptr    = NULL;
	st->pkbufptr = NULL;
	st->pkbitptr = 0;
	st->pkdata = 0;
	st->pkmod = 0;
	st->pkwrdsz = 0;

	*ac3_str = st;

	return 0;
}

Word16  init_dec_param(DEC_PARAM  **param, VO_MEM_OPERATOR *pMemOP)
{
	DEC_PARAM    *st;
	if(param == (DEC_PARAM **) NULL)
	{
		return -1;
	}
	*param = NULL;
	/* allocate memory */
	if((st = (DEC_PARAM *) voAC3Dec_mem_malloc(pMemOP, sizeof(DEC_PARAM), 32)) == NULL)
	{
		return -1;
	}

	st->ac3raminit = 0;
	st->ac3_si  = NULL;          //AC3_INFO_SI 
	st->ac3_aub = NULL;          //AC3_AB_DATA
	st->ac3_str = NULL;          //AC3_BSPK

	init_ac3_info_si(&(st->ac3_si),pMemOP);
	init_ac3_aub(&(st->ac3_aub), pMemOP);
	init_ac3_str(&(st->ac3_str), pMemOP); 

	*param = st;
	return 0;
}

Word16  init_ac3_conf_param(AC3D_CONF_PARAM **conf_param, VO_MEM_OPERATOR *pMemOP)
{
	AC3D_CONF_PARAM   *st;
	if(conf_param == (AC3D_CONF_PARAM **) NULL)
	{
		return -1;
	}  

	*conf_param = NULL;
	/* allocate memory */
	if((st = (AC3D_CONF_PARAM *) voAC3Dec_mem_malloc(pMemOP, sizeof(AC3D_CONF_PARAM), 32)) == NULL)
	{
		return -1;
	}
	/* AC3D_CONF_PARAM init */
	st->blockcount = 0;					                 /* block counter */
/*	for(i=0; i<6; i++)
		st->chanptr[i] = i;   */  
    st->chanptr[0] = 0;                                  /* L */                             
	st->chanptr[1] = 2;                                  /* C */                                
	st->chanptr[2] = 1;                                  /* R */
	st->chanptr[3] = 3;                                  /* sl */
	st->chanptr[4] = 4;                                  /* sr */
	st->chanptr[5] = 5;                                  /* LEF */
	st->compmode = 2;                                    /* compression mode k=2 line out mode */
	st->debug_arg = 0;                                   /* debug argument */
	st->dualmonomode = 0;                                /* dual mono reproduction mode */
	st->dynrngscalehi = 0x7fffffff;                      /* dynamic range scale factor (low) */
	st->dynrngscalelow = 0x7fffffff;                     /* dynamic range scale factor (high) */
	st->framecount = 0;                                  /* frame counter */
	st->frameend = -1;                                   /* ending frame */
	st->framestart = 0;                                  /* starting frame */
	st->outlfeon = 1;                                    /* output subwoofer present flag */
	st->outputflg = 1;                                   /* enable output file flag */
	st->outputmode = 2;                                  /* output channel configuration */
	st->pcmscalefac = 0x7fffffff;                        /* PCM scale factor */
	st->stereomode = 2;                                  /* stereo downmix mode s = 1 Lt/Rt*/
	st->useverbose = 0;                                  /* verbose message flag */
	st->wordsize = 0;                                    /* output word size code */
	st->kcapablemode = 3;                                /* karaoke capable mode */
	st->numchans = 2;

	*conf_param = st;

	return 0;
}

Word16   buff_param_exit(BUFF_PARAM  **p_buff, VO_MEM_OPERATOR *pMemOP)                  /* uninit BUFF_PARAM */  
{
	if(p_buff == NULL || *p_buff == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *p_buff);
	*p_buff = NULL;
	return 0; 
}
Word16	 ac3_dec_pl_exit(AC3_DEC_PL **ac3_dec_ptr, VO_MEM_OPERATOR *pMemOP)               /* uninit AC3_DEC_PL */
{
	if(ac3_dec_ptr == NULL || *ac3_dec_ptr == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *ac3_dec_ptr);
	*ac3_dec_ptr = NULL;
	return 0;

}
Word16	 ac3_info_pl_exit(AC3_INFO_PL  **ac3_pl, VO_MEM_OPERATOR *pMemOP)                 /* uninit AC3_INFO_PL */
{
	if(ac3_pl == NULL || *ac3_pl == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *ac3_pl);
	*ac3_pl = NULL;
	return 0;
}
Word16	 ac3_info_rl_exit(AC3_INFO_RL  **ac3_rl, VO_MEM_OPERATOR *pMemOP)                 /* uninit AC3_INFO_RL */
{
	if(ac3_rl == NULL || *ac3_rl == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *ac3_rl);
	*ac3_rl = NULL;
	return 0;
}
Word16	 crc_calc_pl_exit(CRC_CALC_PL  **crc_pl, VO_MEM_OPERATOR *pMemOP)                 /* uninit CRC_CALC_PL */
{
	if(crc_pl == NULL || *crc_pl == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *crc_pl);
	//*crc_pl = NULL;
	return 0;
}
Word16	 dec_param_exit(DEC_PARAM **p_decparam, VO_MEM_OPERATOR *pMemOP)                  /* uninit DEC_PARAM */
{
	if(p_decparam == NULL || *p_decparam == NULL)
		return 0;

	ac3_info_si_exit(&((*p_decparam)->ac3_si), pMemOP);
	ac3_aub_exit(&((*p_decparam)->ac3_aub), pMemOP);
	ac3_str_exit(&((*p_decparam)->ac3_str), pMemOP); 

	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *p_decparam);
	*p_decparam = NULL;
	return 0;
}
Word16	 ac3_conf_param_exit(AC3D_CONF_PARAM **p_confparam, VO_MEM_OPERATOR *pMemOP)      /* uninit AC3D_CONF_PARAM */
{
	if(p_confparam == NULL || *p_confparam == NULL)
		return 0;
	/*deallocate memory */
	voAC3Dec_mem_free(pMemOP, *p_confparam);
	*p_confparam = NULL;
	return 0;
}
Word16   ac3_info_si_exit(AC3_INFO_SI **ac3_si, VO_MEM_OPERATOR *pMemOP)                 /* uninit AC3_INFO_SI */
{
	if(ac3_si == NULL || *ac3_si == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *ac3_si);
	*ac3_si = NULL;
	return 0;

}
Word16   ac3_aub_exit(AC3_AB_DATA **p_aublk, VO_MEM_OPERATOR *pMemOP)                    /* uninit AC3_AB_DATA */
{
	if(p_aublk == NULL || *p_aublk == NULL)
		return 0;
	/*deallocate memory */
	voAC3Dec_mem_free(pMemOP, *p_aublk);
	*p_aublk = NULL;
	return 0;
}
Word16   ac3_str_exit(AC3_BSPK **p_bstrm, VO_MEM_OPERATOR *pMemOP)                       /* uninit AC3_BSPK */
{
	if(p_bstrm == NULL || *p_bstrm == NULL)
		return 0;
	/* deallocate memory */
	voAC3Dec_mem_free(pMemOP, *p_bstrm);
	*p_bstrm = NULL;
	return 0;
}