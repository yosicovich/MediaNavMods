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

#ifndef __voMem_H__
#define __voMem_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "voIndex.h"

typedef struct
{
	VO_S32				Size;				/*!< Buffer stride */
	VO_S32				Flag;
	VO_PTR				VBuffer;			/*!< user data pointer */
	VO_PTR				PBuffer;			/*!< user data pointer */
}
VO_MEM_INFO;

typedef struct
{
	VO_U32 (VO_API * Alloc) (VO_S32 uID, VO_MEM_INFO * pMemInfo);
	VO_U32 (VO_API * Free) (VO_S32 uID, VO_PTR pBuff);
	VO_U32 (VO_API * Set) (VO_S32 uID, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize);
	VO_U32 (VO_API * Copy) (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);
	VO_U32 (VO_API * Check) (VO_S32 uID, VO_PTR pBuffer, VO_U32 uSize);
	VO_S32 (VO_API * Compare) (VO_S32 uID, VO_PTR pBuffer1, VO_PTR pBuffer2, VO_U32 uSize);
	VO_U32 (VO_API * Move) (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);
} VO_MEM_OPERATOR;

#define voMemAlloc(pBuff, pMemOP, ID, nSize) \
{ \
	VO_MEM_INFO voMemInfo; \
	voMemInfo.Size=nSize; \
	pMemOP->Alloc(ID, &voMemInfo); \
	pBuff=(VO_PBYTE)voMemInfo.VBuffer; \
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __voMem_H__
