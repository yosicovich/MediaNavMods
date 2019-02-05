/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            vo_ac3_ghdr.h
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
#ifndef  __VO_AC3_GHDR_H__
#define  __VO_AC3_GHDR_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define   KCAPABLE       1
typedef   short            Word16;
typedef   unsigned short   UWord16;
typedef   int              Word32;

typedef  struct {
	Word32   *pcmptr[6];
	Word16   pcmoff[6];
	Word16   pcmmod[6];
	Word32   chanptr[6];
	Word32   numchans;
	Word32   wordsize;                /*output word size code*/
	Word32   dynrngscalelow;          /*dynamic range scale factor(low)*/
	Word32   dynrngscalehi;           /*dynamic range scale factor(high)*/
	Word32   pcmscalefac;             /*PCM scale factor*/
	Word32   compmode;                /*compression mode*/
	Word32   stereomode;              /*stereo downmix mode*/
	Word32   dualmonomode;            /*dual mono reproduction mode*/
	Word32   outputmode;              /*output channel configuration*/
	Word32   outlfeon;                /*output subwoofer present flag*/ 
	Word32   outputflg;               /*enable output file flag*/
	long     framecount;              /*frame counter*/
	Word32   blockcount;              /*block counter*/
	Word32   framestart;              /*starting frame*/
	Word32   frameend;                /*ending frame*/
	Word32   useverbose;              /*verbose message flag*/
	Word32   debug_arg;               /*debug argument*/

#ifdef    KCAPABLE
	Word32   kcapablemode;            /*karaoke capable mode*/
#endif

}AC3D_CONF_PARAM;

extern void *g_hAC3DecInst;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__VO_AC3_GHDR_H__
