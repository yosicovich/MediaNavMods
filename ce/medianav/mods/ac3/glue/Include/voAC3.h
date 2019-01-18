/************************************************************************
VisualOn Proprietary
Copyright (c) 2012, VisualOn Incorporated. All Rights Reserved

VisualOn, Inc., 4675 Stevens Creek Blvd, Santa Clara, CA 95051, USA

All data and information contained in or disclosed by this document are
confidential and proprietary information of VisualOn, and all rights
therein are expressly reserved. By accepting this material, the
recipient agrees that this material and the information contained
therein are held in confidence and in trust. The material may only be
used and/or disclosed as authorized in a license agreement controlling
such use and disclosure.
************************************************************************/



#ifndef  __VOAC3_H__
#define  __VOAC3_H__

#include  "voAudio.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#pragma pack(push, 4)


#define    VO_PID_AC3_Module			      0x422C1000 
#define    VO_PID_AC3_FORMAT                          (VO_PID_AC3_Module | 0x0001)
#define    VO_PID_AC3_CHANNELS                        (VO_PID_AC3_Module | 0x0002)
#define    VO_PID_AC3_SAMPLERATE                      (VO_PID_AC3_Module | 0x0003)
#define    VO_PID_AC3_WORDSIZE		              (VO_PID_AC3_Module | 0x0004)
#define    VO_PID_AC3_KCAPABLEMODE	              (VO_PID_AC3_Module | 0x0005)
#define    VO_PID_AC3_DRCMODE    	              (VO_PID_AC3_Module | 0x0006)
#define    VO_PID_AC3_OUTLFEON                        (VO_PID_AC3_Module | 0x0007)
#define    VO_PID_AC3_OUTPUTMODE                      (VO_PID_AC3_Module | 0x0008)
#define    VO_PID_AC3_NUMCHANS                        (VO_PID_AC3_Module | 0x0009)
#define    VO_PID_AC3_STEREOMODE                      (VO_PID_AC3_Module | 0x000A)
#define    VO_PID_AC3_DUALMONOMODE                    (VO_PID_AC3_Module | 0x000B)
#define    VO_PID_AC3_USEVERBOSE                      (VO_PID_AC3_Module | 0x000C)
#define    VO_PID_AC3_DYNX                            (VO_PID_AC3_Module | 0x000D)
#define    VO_PID_AC3_DYNY                            (VO_PID_AC3_Module | 0x000E)
#define    VO_PID_AC3_OUTPUTFLAG                      (VO_PID_AC3_Module | 0x0010)
#define    VO_PID_AC3_CHARI                           (VO_PID_AC3_Module | 0x0011)       

/**
 * Get Audio codec API interface
 * \param pDecHandle [out] Return the AC3 Decoder handle.
 * \retval VO_ERR_OK Succeeded.
 */
VO_S32 VO_API yyGetAC3DecFunc (VO_AUDIO_CODECAPI * pDecHandle);

#pragma pack(pop)
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif   //__VOAC3_H__

