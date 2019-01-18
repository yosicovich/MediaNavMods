	/************************************************************************
	*																		*
	*		VisualOn, Inc. Confidential and Proprietary, 2003-2009			*
	*																		*
	************************************************************************/
/*******************************************************************************
	File:		cmnMemory.h

	Contains:	memory operator function define header file

	Written by:	Bangfei Jin

	Change History (most recent first):
	2009-03-10		JBF			Create file

*******************************************************************************/

#ifndef __cmnMemory_H__
#define __cmnMemory_H__

#include "voMem.h"

#ifdef _VONAMESPACE
namespace _VONAMESPACE {
#else
#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */
#endif

extern VO_MEM_OPERATOR	g_memOP;

/**
 * Alloc the memory
 * \param uID [in] The module ID
 * \param uSize [in] The size of memory
 * \return value is alloced memory address. 0 is failed.
 */
VO_U32	cmnMemAlloc (VO_S32 uID,  VO_MEM_INFO * pMemInfo);

/**
 * free the alloced memory
 * \param uID [in] The module ID
 * \param pMem [in] The address of memory
 * \return value 0 is succeded.
 */
VO_U32	cmnMemFree (VO_S32 uID, VO_PTR pBuffer);

/**
 * free the alloced memory
 * \param uID [in] The module ID
 * \param pMem [in] The address of memory
 * \return value 0 is succeded.
 */
VO_U32	cmnMemSet (VO_S32 uID, VO_PTR pBuff, VO_U8 uValue, VO_U32 uSize);

/**
 * free the alloced memory
 * \param uID [in] The module ID
 * \param pMem [in] The address of memory
 * \return value 0 is succeded.
 */
VO_U32	cmnMemCopy (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);

/**
 * free the alloced memory
 * \param uID [in] The module ID
 * \param pMem [in] The address of memory
 * \return value 0 is succeded.
 */
VO_U32	cmnMemCheck (VO_S32 uID, VO_PTR pBuffer, VO_U32 uSize);

/**
* free the alloced memory
* \param uID [in] The module ID
* \param pMem [in] The address of memory
* \return value 0 is succeded.
*/
VO_S32	cmnMemCompare (VO_S32 uID, VO_PTR pBuffer1, VO_PTR pBuffer2, VO_U32 uSize);

/**
 * free the alloced memory
 * \param uID [in] The module ID
 * \param pMem [in] The address of memory
 * \return value 0 is succeded.
 */
VO_U32	cmnMemMove (VO_S32 uID, VO_PTR pDest, VO_PTR pSource, VO_U32 uSize);

/**
* free the alloced memory
* \param uID [in] The module ID
* \param pMem [in] The address of memory
* \return value 0 is succeded.
*/
VO_S32	cmnMemFillPointer (VO_S32 uID);


VO_S32	cmnMemShowStatus (void);
        
#ifdef _VONAMESPACE
    }
#else
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _VONAMESPACE */

#endif // __cmnMemory_H__
