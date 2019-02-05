	/************************************************************************
	*																		*
	*		VisualOn, Inc. Confidential and Proprietary, 2003 -2009			*
	*																		*
	************************************************************************/
/*******************************************************************************
	File:		cmnMemory.c

	Contains:	memory operator implement code

	Written by:	Bangfei Jin

	Change History (most recent first):
	2009-03-15		JBF			Create file

*******************************************************************************/
#include "cmnMemory.h"
#include <string.h>
#include <stdlib.h>
#if defined LINUX
#include <string.h>
#endif

VO_MEM_OPERATOR		g_memOP;

VO_U32 cmnMemAlloc (VO_S32 uID,  VO_MEM_INFO * pMemInfo)
{
	if (!pMemInfo)
		return VO_ERR_INVALID_ARG;

	pMemInfo->VBuffer = malloc (pMemInfo->Size);

	return 0;
}

VO_U32 cmnMemFree (VO_S32 uID, VO_PTR pMem)
{
	free (pMem);

	return 0;
}

VO_U32	cmnMemSet (VO_S32 uID, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize)
{
	memset (pBuff, uValue, uSize);

	return 0;
}

VO_U32	cmnMemCopy (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize)
{
	memcpy (pDest, pSource, uSize);

	return 0;
}

VO_U32	cmnMemCheck (VO_S32 uID, VO_PTR pBuffer, VO_U32 uSize)
{
	return 0;
}

VO_S32 cmnMemCompare (VO_S32 uID, VO_PTR pBuffer1, VO_PTR pBuffer2, VO_U32 uSize)
{
	return memcmp(pBuffer1, pBuffer2, uSize);
}

VO_U32	cmnMemMove (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize)
{
	memmove (pDest, pSource, uSize);

	return 0;
}

VO_S32 cmnMemFillPointer (VO_S32 uID)
{
	g_memOP.Alloc = cmnMemAlloc;
	g_memOP.Free = cmnMemFree;
	g_memOP.Set = cmnMemSet;
	g_memOP.Copy = cmnMemCopy;
	g_memOP.Check = cmnMemCheck;
	g_memOP.Compare = cmnMemCompare;
	g_memOP.Move = cmnMemMove;

	return 0;
}
