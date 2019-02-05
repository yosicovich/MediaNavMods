#include <stdlib.h>
#include "stream.h"
#include "mem_align.h"

void StreamInit(FrameStream *stream)
{
	stream->buffer = 0;
	stream->bufend = 0;
	stream->buffer_bk = 0;
	stream->maxframesize = 0;
	stream->used_len = 0;
}

int InitStreamBuf(FrameStream *stream, int MaxFrameSize, VO_MEM_OPERATOR *pMemOP)
{
	if(stream->maxframesize < MaxFrameSize)
	{
		stream->maxframesize = MaxFrameSize;

		if(stream->buffer_bk)
			voAC3Dec_mem_free(pMemOP, stream->buffer_bk);
	
		stream->buffer_bk = (unsigned char *)voAC3Dec_mem_malloc(pMemOP, stream->maxframesize, 32);
		if(stream->buffer_bk == NULL)
			return -1;
	}
	
	stream->buffer = stream->buffer_bk;
	stream->bufend = stream->buffer;

	return 0;
}

void StreamFinish(FrameStream *fstream, VO_MEM_OPERATOR *pMemOP)
{
	if(fstream->buffer_bk)
	{
		voAC3Dec_mem_free(pMemOP, fstream->buffer_bk);
		fstream->buffer_bk = NULL;
	}

	fstream->buffer = 0;
	fstream->bufend = 0;
	fstream->maxframesize = 0;
}

int StreamRefillBuffer(FrameStream *stream, unsigned char *buffer, int length, VO_MEM_OPERATOR *pMemOP)
{
	int len = 0;
	
	if(!length)
		return length;

	len  = (int)(stream->bufend - stream->buffer);

	if(len)
		pMemOP->Copy(VO_INDEX_DEC_AC3, stream->buffer_bk, stream->buffer, len);

	stream->buffer = stream->buffer_bk;
	stream->bufend = stream->buffer + len;

	if(len + length > stream->maxframesize)
		len = stream->maxframesize - len;
	else
		len = length;

	pMemOP->Copy(VO_INDEX_DEC_AC3, stream->bufend, buffer, len);
	stream->set_len -= len;
	stream->set_ptr += len;
	stream->bufend += len;
	stream->used_len += len;

	return len;
}

void StreamFlush(FrameStream *stream)
{
	stream->buffer = stream->buffer_bk;
	stream->bufend = stream->buffer;
}