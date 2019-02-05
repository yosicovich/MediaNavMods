/************************************************************************
*																		*
*		VisualOn, Inc. Confidential and Proprietary, 2005				*
*																		*
************************************************************************/
#include	"mem_align.h"

void *
voAC3Dec_mem_malloc(VO_MEM_OPERATOR *pMemop, unsigned int size, unsigned char alignment)
{
	int ret;
	unsigned char *mem_ptr;
	VO_MEM_INFO MemInfo;

	if (!alignment) {

		MemInfo.Flag = 0;
		MemInfo.Size = size + 1;
		ret = pMemop->Alloc(VO_INDEX_DEC_AC3, &MemInfo);
		if(ret != 0)
			return 0;
		mem_ptr = (unsigned char *)MemInfo.VBuffer;

		pMemop->Set(VO_INDEX_DEC_AC3, mem_ptr, 0, size + 1);
	
		*mem_ptr = (unsigned char)1;

		/* Return the mem_ptr pointer */
		return ((void *)(mem_ptr+1));
	} else {
		unsigned char *tmp;
		
		MemInfo.Flag = 0;
		MemInfo.Size = size + alignment;
		ret = pMemop->Alloc(VO_INDEX_DEC_AC3, &MemInfo);
		if(ret != 0)
			return 0;

		tmp = (unsigned char *)MemInfo.VBuffer;

		pMemop->Set(VO_INDEX_DEC_AC3, tmp, 0, size + alignment);

		/* Align the tmp pointer */
		mem_ptr =
			(unsigned char *) ((unsigned int) (tmp + alignment - 1) &
			(~((unsigned int) (alignment - 1))));

		/* Special case where malloc have already satisfied the alignment
		* We must add alignment to mem_ptr because we must store
		* (mem_ptr - tmp) in *(mem_ptr-1)
		* If we do not add alignment to mem_ptr then *(mem_ptr-1) points
		* to a forbidden memory space */
		if (mem_ptr == tmp)
			mem_ptr += alignment;

		/* (mem_ptr - tmp) is stored in *(mem_ptr-1) so we are able to retrieve
		* the real malloc block allocated and free it in xvid_free */
		*(mem_ptr - 1) = (unsigned char) (mem_ptr - tmp);

		/* Return the aligned pointer */
		return ((void *)mem_ptr);
	}

	return(0);
}

void
voAC3Dec_mem_free(VO_MEM_OPERATOR *pMemop, void *mem_ptr)
{

	unsigned char *ptr;

	if (mem_ptr == 0)
		return;

	/* Aligned pointer */
	ptr = mem_ptr;

	/* *(ptr - 1) holds the offset to the real allocated block
	 * we sub that offset os we free the real pointer */
	ptr -= *(ptr - 1);

	/* Free the memory */
	pMemop->Free(VO_INDEX_DEC_AC3, ptr);
}
