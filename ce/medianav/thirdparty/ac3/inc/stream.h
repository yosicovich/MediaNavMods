#ifndef LIBAC3_STREAM_H
#define LIBAC3_STREAM_H

#include	<stdlib.h>
#include	<string.h>
#include    "voMem.h"

#define		BUFFER_STORE	32*1024
#define		BUFFER_GUARD	20

typedef struct _FrameStream{
  unsigned char	*buffer;				
  unsigned char	*buffer_bk;				
  unsigned char	*bufend;
  unsigned char *set_ptr;
  int           set_len;
  int			maxframesize;
  int           used_len;
}FrameStream;

void StreamInit(FrameStream *fstream);
void StreamFinish(FrameStream *fstream, VO_MEM_OPERATOR *pMemOP);
int InitStreamBuf(FrameStream *stream, int MaxFrameSize, VO_MEM_OPERATOR *pMemOP);
int  StreamRefillBuffer(FrameStream* fstream, unsigned char *psrc, int length, VO_MEM_OPERATOR *pMemOP);
void StreamFlush(FrameStream *fstream);

# endif
