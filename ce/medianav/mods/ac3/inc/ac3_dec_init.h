/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3_dec_init.h
*
* Project:
* contents/description: ac3 decoder data structure init header 
*            
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    12-19-2008        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/
#ifndef __AC3_DEC_INIT_H__
#define __AC3_DEC_INIT_H__

#include  "vo_ac3_var.h"

Word16   init_buff_param(BUFF_PARAM **ac3_buff, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_dec_pl(AC3_DEC_PL **dec_pl, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_info_pl(AC3_INFO_PL **info_pl, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_info_rl(AC3_INFO_RL **info_rl, VO_MEM_OPERATOR *pMemOP);
Word16   init_crc_calc_pl(CRC_CALC_PL **calc_pl, VO_MEM_OPERATOR *pMemOP);
Word16   init_dec_param(DEC_PARAM **param, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_conf_param(AC3D_CONF_PARAM **conf_param, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_info_si(AC3_INFO_SI **ac3_si, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_aub(AC3_AB_DATA **ac3_aub, VO_MEM_OPERATOR *pMemOP);
Word16   init_ac3_str(AC3_BSPK **ac3_str, VO_MEM_OPERATOR *pMemOP);

Word16   buff_param_exit(BUFF_PARAM **p_buff, VO_MEM_OPERATOR *pMemOP);                 /* uninit BUFF_PARAM */                         
Word16	 ac3_dec_pl_exit(AC3_DEC_PL **ac3_pl, VO_MEM_OPERATOR *pMemOP);                 /* uninit AC3_DEC_PL */
Word16	 ac3_info_pl_exit(AC3_INFO_PL **ac3_info_ptr, VO_MEM_OPERATOR *pMemOP);         /* uninit AC3_INFO_PL */
Word16	 ac3_info_rl_exit(AC3_INFO_RL **ac3_rl, VO_MEM_OPERATOR *pMemOP);               /* uninit AC3_INFO_RL */
Word16	 crc_calc_pl_exit(CRC_CALC_PL **crc_calc, VO_MEM_OPERATOR *pMemOP);             /* uninit CRC_CALC_PL */
Word16	 dec_param_exit(DEC_PARAM **p_decparam, VO_MEM_OPERATOR *pMemOP);               /* uninit DEC_PARAM */
Word16	 ac3_conf_param_exit(AC3D_CONF_PARAM **p_confparam, VO_MEM_OPERATOR *pMemOP);   /* uninit AC3D_CONF_PARAM */

Word16   ac3_info_si_exit(AC3_INFO_SI **ac3_si, VO_MEM_OPERATOR *pMemOP);         /* uninit AC3_INFO_SI */
Word16   ac3_aub_exit(AC3_AB_DATA **ac3_aub, VO_MEM_OPERATOR *pMemOP);            /* uninit AC3_AB_DATA */
Word16   ac3_str_exit(AC3_BSPK **ac3_str, VO_MEM_OPERATOR *pMemOP);               /* uninit AC3_BSPK */

#endif  //__AC3_DEC_INIT_H__

