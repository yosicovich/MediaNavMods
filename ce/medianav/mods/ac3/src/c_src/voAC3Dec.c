/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3_dec_api.c
*
* Project:
* contents/description:vo fixed version porting, the file will define 
*                      some application interface function.
*            
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    12-19-2008        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/
/* ac3_dec_init();       the function malloc space for data structure
* ac3_dec_process();    the function decoder one frame ac3 stream bits
* ac3_dec_uinit();      the function free spece for data structure
* ac3_dec_setparam();   the function set ac3 decoder diff modle
* ac3_dec_getparam();   the function get ac3 decoder diff model information
*/
#include   <stdio.h>
#include   "vo_ac3_var.h"
#include   "vo_ac3_ghdr.h"
#include   "ac3d_com.h"
#include   "ac3_dec_init.h"
#include   "crc_com.h"
#include   "usr_equ.h"
#include   "stream.h"
#include   "mem_align.h"
#include   "voIndex.h"
#include   "voAC3.h"

extern DSPshort frmsizetab[NFSCOD][NDATARATE];
const  int sampleratetab[NFSCOD] = {48000,44100,32000};
#ifdef KCAPABLE
extern int kcapabletab[4][6];
#endif
void *g_hAC3DecInst = NULL;

//#define DUMP
#define MS_CHANNEL_CONFIG

#ifdef DUMP
FILE *dump = NULL;
#endif

VO_U32 VO_API voAC3Dec_Init(VO_HANDLE * phCodec,VO_AUDIO_CODINGTYPE vType, VO_CODEC_INIT_USERDATA * pUserData)
{
	VO_U32 nRet = 0;
	DECEXEC   *st;
	//VO_MEM_OPERATOR voMemoprator;
	VO_MEM_OPERATOR *pMemOP;
	//int interMem = 0;

	if(pUserData == NULL || (pUserData->memflag & 0x0F) != VO_IMF_USERMEMOPERATOR || pUserData->memData == NULL )
	{

		return VO_ERR_INVALID_ARG;
		//voMemoprator.Alloc = cmnMemAlloc;
		//voMemoprator.Copy = cmnMemCopy;
		//voMemoprator.Free = cmnMemFree;
		//voMemoprator.Set = cmnMemSet;
		//voMemoprator.Check = cmnMemCheck;
		//interMem = 1;
		//pMemOP = &voMemoprator;
	}
	else
	{
		pMemOP = (VO_MEM_OPERATOR *)pUserData->memData;
	} 

	/* Memory allocation for coder state */
	st = (DECEXEC *)voAC3Dec_mem_malloc(pMemOP, sizeof(DECEXEC), 32);
	if(st == NULL)
		return VO_ERR_OUTOF_MEMORY;

	st->ac3_buff = NULL;							/* BUFF_PARAM  */ 
	st->ac3_dec_list = NULL;						/* AC3_DEC_PL  */
	st->ac3_info_list = NULL;						/* AC3_INFO_PL */
	st->ac3_info_rptr = NULL;						/* AC3_INFO_RL */
	st->crc_calc_list = NULL;						/* CRC_CALC_PL */
	st->decparam = NULL;							/* DEC_PARAM   */
	st->inparam = NULL;							/* AC3_CONF_PARAM */

	st->pcm_buf = NULL;              
	st->pk_buf = NULL;
	st->pContext = NULL;
	st->nEndianFlag = 0;                        //default 0

	init_dec_param(&(st->decparam), pMemOP);                /* init DEC_PARAM */
	init_ac3_conf_param(&(st->inparam), pMemOP);            /* init AC3_CONF_PARAM */
	init_buff_param(&(st->ac3_buff), pMemOP);               /* init BUFF_PARAM */                         
	init_ac3_info_pl(&(st->ac3_info_list), pMemOP);         /* init AC3_INFO_PL */
	init_ac3_info_rl(&(st->ac3_info_rptr), pMemOP);         /* init AC3_INFO_RL */
	init_crc_calc_pl(&(st->crc_calc_list), pMemOP);         /* init CRC_CALC_PL */
	init_ac3_dec_pl(&(st->ac3_dec_list), pMemOP);           /* init AC3_DEC_PL */

	if((st->stream = (FrameStream *)voAC3Dec_mem_malloc(pMemOP, sizeof(FrameStream), 32)) == NULL)
	{
		return VO_ERR_OUTOF_MEMORY;
	}

	//if((st->stream = (FrameStream *)mem_malloc(pMemOP, sizeof(FrameStream), 32)) == NULL)
	//{
	//	return VO_ERR_OUTOF_MEMORY;
	//}

	StreamInit(st->stream);

	if(InitStreamBuf(st->stream, BUFFER_STORE, pMemOP) < 0)
		return VO_ERR_OUTOF_MEMORY;

	//if(interMem)
	//{
	//	st->voMemoprator.Alloc = cmnMemAlloc;
	//	st->voMemoprator.Copy = cmnMemCopy;
	//	st->voMemoprator.Free = cmnMemFree;
	//	st->voMemoprator.Set = cmnMemSet;
	//	st->voMemoprator.Check = cmnMemCheck;
	//	pMemOP = &st->voMemoprator;
	//}
	st->pvoMemop = pMemOP;

	//VO_U32 VO_API voCheckLibInit (VO_PTR * phCheck, VO_U32 nID, VO_U32 nFlag, VO_HANDLE hInst);
#ifdef LCHECK 
	if((pUserData->memflag & 0xF0) == 0x10  && pUserData->libOperator != NULL)
		nRet = voCheckLibInit(&(st->hCheck), VO_INDEX_DEC_AC3, pUserData->memflag|1, g_hAC3DecInst, pUserData->libOperator);
	else
		nRet = voCheckLibInit(&(st->hCheck), VO_INDEX_DEC_AC3, pUserData->memflag|1, g_hAC3DecInst, NULL);

    if (nRet != VO_ERR_NONE)
    {
		if (st->hCheck)
		{
			voCheckLibUninit(st->hCheck);
			st->hCheck = NULL;
		}

		if (st->stream)
		{
			voAC3Dec_mem_free(pMemOP, st->stream);
		}
		if (st)
		{
			voAC3Dec_mem_free(pMemOP, st);
		}
        return nRet;
    }
#endif

	*phCodec = (void *) st;

#ifdef DUMP
	///data/local/lhp/
	dump = fopen("/data/local/lhp/data1.log", "wb");
#endif

	return VO_ERR_NONE;
}

VO_U32 VO_API voAC3Dec_SetInputData(VO_HANDLE hCodec, VO_CODECBUFFER * pInput)
{
	DECEXEC     *gData;
	FrameStream *stream;
	VO_MEM_OPERATOR *pMemOP;

	if(NULL == hCodec)
	{
		return VO_ERR_INVALID_ARG;
	}

	gData = (DECEXEC *)hCodec;
	stream = gData->stream;
	pMemOP = gData->pvoMemop;

	if(NULL == pInput || NULL == pInput->Buffer || 0 > pInput->Length)
	{
		return VO_ERR_INVALID_ARG;
	}

	stream->set_ptr    = pInput->Buffer;
	stream->set_len    = pInput->Length;
	stream->used_len   = 0;

#ifdef DUMP
	//fwrite(pInput->Buffer, pInput->Length , 1, dump);
	//fflush(dump);
#endif
	return VO_ERR_NONE;
}

VO_U32 VO_API voAC3Dec_GetOutputData(VO_HANDLE hCodec, VO_CODECBUFFER * pOutput, VO_AUDIO_OUTPUTINFO * pAudioFormat)
{
	int		    nwords, count, sampratecode, chan;
	int 		    outlongval, maxpcmval;
	Word32              dblval;
	Word16              skiplen = 0;
	char		    *outbyteptr;
	Word32              fpcm_buf[NCHANS * N];
	Word32              *pcmbufptr;
	Word32              outshortval;
	Word16		    crcsyndrome = 0, ac3_status;
	Word16		    *outshortptr = NULL;
	unsigned char   *temp;
	DOLBY_SIP	    in_sip, ret_sip;
	DECEXEC		    *p_decexec;
	AC3_INFO_PL	    *ac3_info_list;
	AC3_INFO_RL	    *ac3_info_rptr;
	CRC_CALC_PL	    *crc_calc_list;
	AC3_DEC_PL	    *ac3_dec_list;
	AC3D_CONF_PARAM	    *ac3_gparam; 
	FrameStream	    *stream;
	VO_MEM_OPERATOR     *pMemOP;
	unsigned char  word, byte1;

#ifdef HALFRATE
	int halfratecode;
	int halfrateary[3] = { 1, 2, 4 };
#endif

	if((hCodec == NULL) && (pOutput == NULL))
		return VO_ERR_INVALID_ARG;

	p_decexec = (DECEXEC *)hCodec;

	if (pOutput->Length < 1536*4)  //output buffer length check(only for 2channels)
	{
		return VO_ERR_OUTPUT_BUFFER_SMALL;
	}
	ac3_info_list = p_decexec->ac3_info_list;
	ac3_info_rptr = p_decexec->ac3_info_rptr;
	ac3_gparam    = p_decexec->inparam;
	crc_calc_list = p_decexec->crc_calc_list;
	ac3_dec_list  = p_decexec->ac3_dec_list;
	stream	      = p_decexec->stream;
	pMemOP        = p_decexec->pvoMemop;

	StreamRefillBuffer(stream, stream->set_ptr, stream->set_len, pMemOP);

	pMemOP        = p_decexec->pvoMemop;
	p_decexec->pk_buf = stream->buffer;                                      //Input buffer pointer
	p_decexec->input_length = (Word32)(stream->bufend - stream->buffer);     //Input buffer Data length

	p_decexec->pcm_buf = fpcm_buf;                                              //frame  pcm buffer                        
	p_decexec->output_buf = (Word32*)pOutput->Buffer;                           //Output buffer pointer
	pOutput->Length = 0;                                                        //Output buffer length

	temp = p_decexec->pk_buf;
	while(p_decexec->input_length > 20)
	{
		if (temp[0] == 0x0b && temp[1] == 0x77)
		{
			p_decexec->nEndianFlag = 1;
			break;

		}else if(temp[0] == 0x77 && temp[1] == 0x0b)
		{
			p_decexec->nEndianFlag = 0;
			break;
		}
		else 
		{
			skiplen++;
			p_decexec->input_length--;
		}

		temp += 1;
	}

	//if(skiplen & 0x1)
	//	printf("hehe!\n");
	stream->buffer += skiplen; 

	if(p_decexec->input_length <= 20)
	{
		pAudioFormat->InputUsed = stream->used_len;
		return  VO_ERR_INPUT_BUFFER_SMALL;
	}
	//if(skiplen & 0x1)
	//{
	//	pAudioFormat->InputUsed = stream->used_len;
	//      return VO_ERR_INPUT_BUFFER_SMALL;
	//}
	//else
	//{
	//	stream->buffer += skiplen; 
	//	temp1 = *(stream->buffer + 4);
	//	temp2 = (temp1 >> 6);
	//	temp1 = temp1 & 0x3F;
	//	Flen = (frmsizetab[temp2][temp1]<<1);
	//	if(p_decexec->input_length < Flen)
	//	{
	//		pAudioFormat->InputUsed = stream->used_len;
	//		return VO_ERR_INPUT_BUFFER_SMALL;
	//	}
	//	if(p_decexec->input_length > Flen)
	//	{
	//		if(((*(stream->buffer + Flen) != 0x0b) && (*(stream->buffer + Flen + 1) != 0x77)))
	//		{
	//               stream->buffer += Flen;
	//			p_decexec->input_length -= Flen;
	//			pAudioFormat->InputUsed = stream->used_len;
	//			return VO_ERR_INPUT_BUFFER_SMALL;
	//		}
	//		else if((skiplen & 0x1))
	//		{
	//			pAudioFormat->InputUsed = stream->used_len;
	//               return VO_ERR_INPUT_BUFFER_SMALL;
	//		}
	//	}
	//}

	//stream->buffer += skiplen;

	p_decexec->pk_buf = p_decexec->pk_buf + skiplen;
	if(p_decexec->nEndianFlag == 1)
	{
		/*	Byte swap the buffer words on little-endian machines */
		for (count = 0; count < 20; count+=2)
		{
			word = p_decexec->pk_buf[count];
			byte1 = p_decexec->pk_buf[count+1];
			p_decexec->pk_buf[count+1] = word;
			p_decexec->pk_buf[count] = byte1;       
		}
	}

	ac3_info_list->size = 5;
	ac3_info_list->iptr = (Word16*)p_decexec->pk_buf;
	ac3_info_list->imod = 0;
	ac3_info_list->ioff = 0;
	ac3_info_list->icfg = 0;

	ret_sip = ac3_info(p_decexec);

	if (ret_sip.status != AC3I_ERR_NONE)
	{
		if (ret_sip.status == AC3I_ERR_REV)
		{ 
			p_decexec->decparam->ac3raminit = 0;
			stream->buffer += 20;
			return VO_ERR_INVALID_ARG;
		}
		else if (ret_sip.status == AC3I_ERR_SAMPRATE)
		{
			p_decexec->decparam->ac3raminit = 0;
			stream->buffer += 20;
			return VO_ERR_INVALID_ARG;
		}
		else if (ret_sip.status == AC3I_ERR_DATARATE)
		{
			p_decexec->decparam->ac3raminit = 0;
			stream->buffer += 20;
			return VO_ERR_INVALID_ARG;
		}
		else
		{
			p_decexec->decparam->ac3raminit = 0;
			stream->buffer += 20;
			return VO_ERR_INVALID_ARG;
		}
	}

	nwords = ac3_info_rptr->frmsize;
	sampratecode = ((ac3_info_rptr->bscfg) >> 14) & 0x0003;

#ifdef HALFRATE
	halfratecode = ((ac3_info_rptr->bscfg) >> 4) & 0x0003;
#endif
	if(p_decexec->input_length < (nwords<<1))
	{
		if(p_decexec->nEndianFlag == 1)
		{
			/*	Byte swap the buffer words on little-endian machines */
			for (count = 0; count < 20; count+=2)
			{
				word = p_decexec->pk_buf[count];
				byte1 = p_decexec->pk_buf[count+1];
				p_decexec->pk_buf[count+1] = word;
				p_decexec->pk_buf[count] = byte1;       
			}
		}
		pAudioFormat->InputUsed = stream->used_len;
		return  VO_ERR_INPUT_BUFFER_SMALL;
	}

	if(p_decexec->nEndianFlag == 1)
	{
		/*	Byte swap the buffer on little-endian machines */
		for (count = 20; count < (nwords<<1); count+=2)
		{
			word = p_decexec->pk_buf[count];
			byte1 = p_decexec->pk_buf[count+1];
			p_decexec->pk_buf[count+1] = word;
			p_decexec->pk_buf[count] = byte1; 
		}
	}

	stream->buffer += (nwords<<1);

	for (count = 0; count < (NCHANS * N); count++)
	{
		fpcm_buf[count] = 0;
	}

	ac3_status = 0;				/* Init status to no errors */
	/* decoder one AC3 frame */
	for (ac3_gparam->blockcount = 0; ac3_gparam->blockcount < NBLOCKS; ac3_gparam->blockcount++)
	{
		if (ac3_gparam->blockcount == 0)
		{
			/*	Call CRC routine in block 0 for first CRC */
			crc_calc_list->size = 6;
			crc_calc_list->iptr = (Word16 *)(p_decexec->pk_buf + 2);
			crc_calc_list->ioff = 0;
			crc_calc_list->imod = 0;
			crc_calc_list->icfg = 0;
			crc_calc_list->count = ac3_info_rptr->crcsize;

			in_sip.status = 0;
			in_sip.param_ptr = crc_calc_list;

			ret_sip = crc_calc(in_sip);	 

			if ((crcsyndrome = ret_sip.status) != 0)
			{
				ac3_status = 1;		/* Request block repeat */
			}
		}
		else if (ac3_gparam->blockcount == 2)
		{
			/*	Call CRC routine in block 2 for second CRC */
			crc_calc_list->size = 6;
			crc_calc_list->iptr = (Word16*)(p_decexec->pk_buf + 2 + (ac3_info_rptr->crcsize<<1));
			crc_calc_list->ioff = 0;
			crc_calc_list->imod = 0;
			crc_calc_list->icfg = 0;
			crc_calc_list->count = (DSPshort)(ac3_info_rptr->frmsize - ac3_info_rptr->crcsize - 1);

			in_sip.status = crcsyndrome;
			in_sip.param_ptr = crc_calc_list;

			ret_sip = crc_calc(in_sip);

			if ((crcsyndrome = ret_sip.status) != 0)
			{
				ac3_status = 1;		/* Request block repeat */
			}
		}

#ifdef MS_CHANNEL_CONFIG
		/*	Call AC-3 decoder */
		// lhp update 2011-07-08
		if (p_decexec->decparam->ac3_si->acmod == MODE10)
		{
			ac3_gparam->numchans = 1;
			ac3_gparam->chanptr[0] = 0;
			ac3_gparam->chanptr[1] = 1;
		}
		else if ((p_decexec->decparam->ac3_si->acmod == MODE20)||(p_decexec->inparam->outputmode == 2))
		{
			ac3_gparam->chanptr[1] = 2;
			ac3_gparam->chanptr[2] = 1;
		}
		/* Following Microsoft Multi-channels configure
		-L -R -C -LFE -Left Surround -Right Surround */
		if (p_decexec->decparam->ac3_si->acmod >= MODE30)
		{
			ac3_gparam->chanptr[0] = 0;
			ac3_gparam->chanptr[1] = 2;
			ac3_gparam->chanptr[2] = 1;
			ac3_gparam->chanptr[3] = 4;
			ac3_gparam->chanptr[4] = 5;
			ac3_gparam->chanptr[5] = 3;
		}
#endif 
		for (chan = 0; chan < NCHANS; chan++)
		{
			ac3_gparam->pcmptr[chan] = p_decexec->pcm_buf + ac3_gparam->chanptr[chan];
			ac3_gparam->pcmoff[chan] = NCHANS;
			ac3_gparam->pcmmod[chan] = 0;
		}

#ifdef MS_CHANNEL_CONFIG
		if (p_decexec->inparam->numchans > 2)
		{
			ac3_gparam->outputmode = p_decexec->decparam->ac3_si->acmod;
		}
#endif


#ifdef KCAPABLE
		ac3_dec_list->size = 17;
#else
		ac3_dec_list->size = 16;
#endif
		ac3_dec_list->iptr = (Word16 *)(p_decexec->pk_buf + 10);
		ac3_dec_list->ioff = 0;
		ac3_dec_list->imod = 0;
		ac3_dec_list->icfg = 0;
		ac3_dec_list->optr = ac3_gparam->pcmptr;
		ac3_dec_list->ooff = ac3_gparam->pcmoff;
		ac3_dec_list->omod = ac3_gparam->pcmmod;
		ac3_dec_list->ocfg = (DSPshort)((ac3_gparam->compmode << 12) + (ac3_gparam->stereomode << 10)
			+ (ac3_gparam->dualmonomode << 8) + (ac3_gparam->outlfeon << 3) + ac3_gparam->outputmode);
		ac3_dec_list->blknum = (DSPshort)ac3_gparam->blockcount;
		ac3_dec_list->dynsclh = ac3_gparam->dynrngscalehi;
		ac3_dec_list->dynscll = ac3_gparam->dynrngscalelow;
		ac3_dec_list->pcmscl  = ac3_gparam->pcmscalefac;
		ac3_dec_list->rptmax = 0;
		ac3_dec_list->dnmxptr = 0;

#ifdef KCAPABLE
		ac3_dec_list->krkptr = kcapabletab[ac3_gparam->kcapablemode];
#endif				
		ac3_dec_list->debug = (DSPshort)ac3_gparam->debug_arg;

		in_sip.status = ac3_status;
		in_sip.param_ptr = ac3_dec_list;

		ret_sip = ac3_dec(in_sip, p_decexec);

		if(ret_sip.status != 0)
		{
			p_decexec->decparam->ac3raminit = 0;
			pOutput->Length = 0;
			return VO_ERR_INVALID_ARG;
		}
#if 1
		/*	Write PCM to output file */
		if (ac3_gparam->outputflg)
		{
			if (ac3_gparam->wordsize == 0)	/* 16-bit integer */
			{
				outshortptr = (DSPshort *)p_decexec->output_buf;
				pcmbufptr = fpcm_buf;
				for (count = 0; count < N; count++)
				{
					for (chan = 0; chan < ac3_gparam->numchans; chan++)
					{
						dblval = *pcmbufptr++;
						outshortval = (dblval >> 13);
						if(outshortval > 32767)
							outshortval = 32767;
						if(outshortval < -32767)
							outshortval = -32768;
						*outshortptr++ = outshortval;
					}
					p_decexec->output_buf = (int *)outshortptr;
					pcmbufptr += NCHANS - ac3_gparam->numchans;
					pOutput->Length += ac3_gparam->numchans;
				}
			}
			else if (ac3_gparam->wordsize == 1)		/* 64-bit floating point */
			{

				pOutput->Length = sizeof(DSPfract) * ac3_gparam->numchans * N;

				//if (fwrite((void *)pcm_buf, sizeof(DSPfract), numchans * N, pcmfile) != (size_t) (numchans * N))
				//{
				//	sprintf(errstr, "decode: Unable to write to output file, %s", pcmfile);
				//	error_msg(errstr, FATAL);
				//}
			}
			else	/* 17 <= wordsize <= 24 */
			{
				maxpcmval = 1L << (ac3_gparam->wordsize - 1);
				outbyteptr = (char *)p_decexec->output_buf;
				pcmbufptr = fpcm_buf;
				for (count = 0; count < N; count++)
				{
					for (chan = 0; chan < ac3_gparam->numchans; chan++)
					{                      
						dblval = *pcmbufptr++;
						outlongval = (dblval >> (29 - ac3_gparam->wordsize));

						if (outlongval > (maxpcmval - 1))
						{
							outlongval = maxpcmval - 1;
						}
						else if(outlongval < -(maxpcmval - 1))
						{
							outlongval = -maxpcmval;
						}
						outlongval <<= (32 - ac3_gparam->wordsize);
						*outbyteptr++ = (char)(outlongval >> 8);
						*outbyteptr++ = (char)(outlongval >> 16);
						*outbyteptr++ = (char)(outlongval >> 24);
					}
					p_decexec->output_buf = (int *)outshortptr;
					pcmbufptr += NCHANS - ac3_gparam->numchans;
					pOutput->Length += outbyteptr - (char *)p_decexec->output_buf;
				}
			}
		}
#endif
	}
	pOutput->Length *= 2;

#ifdef DUMP
	fwrite(pOutput->Buffer, pOutput->Length , 1, dump);
	fflush(dump);
#endif

	//VO_U32 VO_API voCheckLibCheckAudio (VO_PTR hCheck, VO_CODECBUFFER * pOutBuffer, VO_AUDIO_FORMAT * pFormat);

#ifdef LCHECK
	voCheckLibCheckAudio(p_decexec->hCheck, pOutput, &(pAudioFormat->Format));
#endif

	if(pAudioFormat)
	{
		pAudioFormat->Format.Channels = ac3_gparam->numchans;
		pAudioFormat->Format.SampleBits = 16;
		pAudioFormat->Format.SampleRate = sampleratetab[sampratecode];
#ifdef HALFRATE
		pAudioFormat->Format.SampleRate = pAudioFormat->Format.SampleRate/halfrateary[halfratecode];
#endif
		pAudioFormat->InputUsed = stream->used_len;
	}

	return VO_ERR_NONE;
}

VO_U32 VO_API voAC3Dec_SetParam(VO_HANDLE hCodec, VO_S32 uParamID, VO_PTR pData)
{
	DECEXEC  *p_decexc;
	AC3D_CONF_PARAM *p_confparam;
	Word32  *ptr;

	if(hCodec == NULL)
		return VO_ERR_WRONG_STATUS;

	p_decexc = (DECEXEC *)hCodec;
	p_confparam = (AC3D_CONF_PARAM *)p_decexc->inparam;
	ptr = (Word32 *)pData;

	switch(uParamID)
	{
	case VO_PID_AC3_WORDSIZE:
		if(*(int *)pData < 0 || *(int *)pData > 24)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->wordsize = *(int *)pData;
		break;
	case VO_PID_AC3_KCAPABLEMODE:
		if(*(int *)pData < 0 || *(int *)pData > 3)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->kcapablemode = *(int *)pData;
		break;
	case VO_PID_COMMON_FLUSH:
		p_decexc->decparam->ac3raminit = 0;
		StreamFlush(p_decexc->stream);
		break;
	case VO_PID_AC3_DRCMODE:
		if(*(int *)pData < 0 || *(int *)pData > 3)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->compmode = *(int *)pData;
		break;
	case VO_PID_AC3_OUTLFEON:
		if(*(int *)pData < 0 || *(int *)pData > 1)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->outlfeon = *(int *)pData;
		break;
	case VO_PID_AC3_OUTPUTMODE:
		if(*(int *)pData < 0 || *(int *)pData > 7)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->outputmode = *(int *)pData;
		break;
	case VO_PID_AC3_NUMCHANS:
		p_confparam->numchans = *(int *)pData;
		break;
	case VO_PID_AUDIO_CHANNELS:
		p_confparam->numchans = *(int *)pData;
		break;
	case VO_PID_AC3_STEREOMODE:
		if(p_confparam->outputmode == 2)
			p_confparam->stereomode = *(int *)pData;
		break;
	case VO_PID_AC3_DUALMONOMODE:
		if(*(int *)pData < 0 || *(int *)pData > 3)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->dualmonomode = *(int *)pData;
		break;
	case VO_PID_AC3_USEVERBOSE:
		if(*(int *)pData != 0 || *(int *)pData != 1)
			return VO_ERR_WRONG_PARAM_ID;
		p_confparam->useverbose = *(int *)pData;
		break;
	case VO_PID_AC3_DYNX:
		p_confparam->dynrngscalehi = *(int *)pData;
		break;
	case VO_PID_AC3_DYNY:
		p_confparam->dynrngscalelow = *(int *)pData;
		break;
	case VO_PID_AC3_OUTPUTFLAG:
		p_confparam->outputflg = *(int *)pData;
		break;
	case VO_PID_AC3_CHARI:       //channel routing information 
		p_confparam->chanptr[0] = *(ptr++);
		p_confparam->chanptr[1] = *(ptr++);
		p_confparam->chanptr[2] = *(ptr++);
		p_confparam->chanptr[3] = *(ptr++);
		p_confparam->chanptr[4] = *(ptr++);
		p_confparam->chanptr[5] = *(ptr++);
		break;                 // need to add some control code
	case VO_PID_COMMON_HEADDATA:
		break;
	default:
		break;
	}

	return VO_ERR_NONE;
}

VO_U32 VO_API voAC3Dec_GetParam(VO_HANDLE hCodec, VO_S32 uParamID, VO_PTR pData)
{
	//DECEXEC *p_decexec = (DECEXEC *)hCodec;
	return VO_ERR_NONE;
}

VO_U32 VO_API voAC3Dec_Uninit(VO_HANDLE hCodec)
{
	DECEXEC *p_decexc = (DECEXEC *)hCodec;
	VO_MEM_OPERATOR *pMemOP;
	//FrameStream     *stream;
	if(hCodec)
	{
		pMemOP = p_decexc->pvoMemop;
		//VO_U32 VO_API voCheckLibUninit (VO_PTR hCheck);

#ifdef LCHECK
		voCheckLibUninit(p_decexc->hCheck);
#endif

		buff_param_exit(&(p_decexc->ac3_buff), pMemOP);               /* uninit BUFF_PARAM */                         
		ac3_dec_pl_exit(&(p_decexc->ac3_dec_list), pMemOP);           /* uninit AC3_DEC_PL */
		ac3_info_pl_exit(&(p_decexc->ac3_info_list), pMemOP);         /* uninit AC3_INFO_PL */
		ac3_info_rl_exit(&(p_decexc->ac3_info_rptr), pMemOP);         /* uninit AC3_INFO_RL */
		crc_calc_pl_exit(&(p_decexc->crc_calc_list), pMemOP);         /* uninit CRC_CALC_PL */
		dec_param_exit(&(p_decexc->decparam), pMemOP);                /* uninit DEC_PARAM */
		ac3_conf_param_exit(&(p_decexc->inparam), pMemOP);            /* uninit AC3_CONF_PARAM */
		StreamFinish(p_decexc->stream, pMemOP);
		voAC3Dec_mem_free(pMemOP, p_decexc->stream);
		voAC3Dec_mem_free(pMemOP, hCodec);
		hCodec = NULL;
	}
#ifdef DUMP
	fclose(dump);
#endif

	return VO_ERR_NONE;

}

VO_S32 VO_API yyGetAC3DecFunc (VO_AUDIO_CODECAPI * pDecHandle)
{
	if(NULL == pDecHandle)
		return VO_ERR_INVALID_ARG;
	pDecHandle->Init = voAC3Dec_Init;
	pDecHandle->SetInputData = voAC3Dec_SetInputData;
	pDecHandle->GetOutputData = voAC3Dec_GetOutputData;
	pDecHandle->SetParam = voAC3Dec_SetParam;
	pDecHandle->GetParam = voAC3Dec_GetParam;
	pDecHandle->Uninit = voAC3Dec_Uninit;

	return VO_ERR_NONE;
}
