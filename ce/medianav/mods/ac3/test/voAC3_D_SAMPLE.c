/*
*  Copyright 2008 by Visualon software Incorporated.
*  All rights reserved. Property of Visualon software Incorporated.
*  Restricted rights to use, duplicate or disclose this code are
*  granted through contract.
*  
*/

/***************************** Change History**************************
;* 
;*    DD/MMM/YYYY     Code Ver         Description             Author
;*    -----------     --------     ------------------        -----------
;*    11-25-2008        1.0        File imported from        Huaping Liu
;*                                             
;**********************************************************************/
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#ifdef _WIN32_WCE
#include 	<windows.h>
#include 	<objbase.h>
#include 	<Winbase.h>
#endif // _WIN32_WCE
#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include    "voAC3.h"
#include    "cmnMemory.h"

#ifdef LINUX
#include <dlfcn.h>
#endif


#define   L_FRAME    1024*10
void    parse_cmd(char *input);
void    range(char *inputarg, int *lower, int *upper);
void    show_help(void);
void    show_usage(void);

/*set parameter lValue */
int	wordsize = 0;                      /* default wordsize = 0, 16bits short integer */
int	kcapablemode = 3;                  /* default kcapablemode = 0, both vocals */
int	debug_arg = 0;
char	*ac3fname;                         /* input file name */
char	*pcmfname;                         /* output file name */
int	compmode = 2;                      /* compression mode */
int	outlfeon = 0;                      /* output subwoofer present flag */ 
int	outputmode = 2;					   /* output channel configuration */
int	numchans = 2;
int 	pcmscalefac = 0x7FFFFFFF;          	   /* PCM scale factor */
int	stereomode = 0;					   /* stereo downmix mode */
int     dualmonomode = 0;				   /* dual mono reproduction mode */
int     useverbose = 0;					   /* verbose messages flag */
int     dynrngscalelow = 0x7FFFFFFF;	           /* dynamic range scale factor (low) */
int     dynrngscalehi = 0x7FFFFFFF;		       /* dynamic range scale factor (high) */
int     outputflg = 1;  				   /* enable output file flag */
int     framestart = 0;					   /* starting frame */
int     frameend = -1;					   /* ending frame */
int     chanptr[6] = {0, 1, 2, 3, 4, 5};                  

typedef int (VO_API * VOGETAUDIODECAPI) (VO_AUDIO_CODECAPI * pDecHandle);

int decoder(const char* srcfile, const char* dstfile)
{
	FILE	*fsrc = NULL;
	FILE	*fdst = NULL;
	int	ret = 0;
	int     readlen = 0;
	int     uselen  = 0;
	int     leavelen = 0;
	int     lastframe = 0;
	int     returnCode;
	int     framecount = 0;
	VO_AUDIO_CODECAPI AudioAPI;
	VO_MEM_OPERATOR moper;
	VO_CODEC_INIT_USERDATA useData;
	VO_HANDLE hCodec;
	VO_CODECBUFFER inData;
	VO_CODECBUFFER outData;
	VO_AUDIO_OUTPUTINFO outFormat;

	short   done = 0;
	unsigned char    new_speech[L_FRAME] = {0};         /* Pointer to new speech data        */
	short   packed_bits[9216*3] = {0};
	unsigned char    *input = new_speech;

#ifdef LINUX
	void  *handle;
	void  *pfunc;
	VOGETAUDIODECAPI pGetAPI;
#endif

#ifdef _WIN32_WCE
	TCHAR msg[256];
	DWORD  total = 0.0;
	int t1, t2 = 0;	
#else
	clock_t   start, finish;
	double    duration = 0.0;
#endif
	if ((fsrc = fopen (srcfile, "rb")) == NULL)
	{
		ret = 1;
		goto safe_exit;
	}

	if ((fdst = fopen (dstfile, "wb")) == NULL)
	{
		ret = 2;
		goto safe_exit;
	}

	moper.Alloc = cmnMemAlloc;
	moper.Copy = cmnMemCopy;
	moper.Free = cmnMemFree;
	moper.Set = cmnMemSet;
	moper.Check = cmnMemCheck;

	useData.memflag = VO_IMF_USERMEMOPERATOR;
	useData.memData = (VO_PTR)(&moper);	

#ifdef LINUX
	handle = dlopen("/data/local/tmp/ac3/libvoAC3Dec.so", RTLD_NOW);
	if(handle == 0)
	{
		printf("open dll error......");
		return -1;
	}
	pfunc = dlsym(handle, "voGetAC3DecAPI");	
	if(pfunc == 0)
	{
		printf("open function error......");
		return -1;
	}
	pGetAPI = (VOGETAUDIODECAPI)pfunc;
	returnCode  = pGetAPI(&AudioAPI);
	if(returnCode)
		return -1;
#else
	returnCode = yyGetAC3DecFunc(&AudioAPI);
	if(returnCode)
	{
		ret = -1;
		goto safe_exit;
	}
#endif 

	printf("hehe!\n");
	//#######################################   Init Encoding Section   #########################################
	returnCode = AudioAPI.Init(&hCodec, VO_AUDIO_CodingAC3, &useData);

	if(returnCode < 0)
	{
		ret = -1;
		goto safe_exit;
	}

	//####################################### set Parameters ####################################################
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_WORDSIZE, &wordsize);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_KCAPABLEMODE, &kcapablemode);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_DRCMODE, &compmode);
	returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_OUTLFEON, &outlfeon);
	returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_OUTPUTMODE, &outputmode);
    //returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_STEREOMODE, &stereomode);
	returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_NUMCHANS, &numchans);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_DUALMONOMODE, &dualmonomode);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_USEVERBOSE, &useverbose);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_DYNX, &dynrngscalehi);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_DYNY, &dynrngscalelow);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_OUTPUTFLAG, &outputflg);
	//returnCode = AudioAPI.SetParam(hCodec, VO_PID_AC3_CHARI,&(chanptr[0]));

	//#######################################   Decoding Section   #########################################
	readlen = fread((void *)new_speech, 1, L_FRAME, fsrc);
	inData.Length = readlen;
	if(readlen == 0)
		done = 1;

	do {
		inData.Buffer    = input;
		inData.Length    = readlen;
		outData.Buffer   = (unsigned char *)packed_bits;

		outData.Length = sizeof(packed_bits);

		/* decode one amr block */
		returnCode = AudioAPI.SetInputData(hCodec,&inData);

		do {
#ifdef _WIN32_WCE
			t1 = GetTickCount();
#else
			start = clock();
#endif
			outData.Length = sizeof(packed_bits);
			returnCode = AudioAPI.GetOutputData(hCodec,&outData, &outFormat);

			if(returnCode == 0)
			{
#ifdef _WIN32_WCE
				t2 = GetTickCount();
				total += t2 - t1;
#else
				finish = clock();
				duration = finish - start;
#endif
				//if((duration/CLOCKS_PER_SEC) > 0.005)
				//{
    //                 printf( "\n%2.5f seconds\n", (double)duration/CLOCKS_PER_SEC);
				//}
				framecount++;
				if (framecount == 205)
				{
					printf("hehe!\n");
				}
				fwrite(outData.Buffer, 1, outData.Length, fdst);
				fflush(fdst);
			}
		} while(returnCode != VO_ERR_INPUT_BUFFER_SMALL);

#ifdef _WIN32_WCE
		t2 = GetTickCount();
		total += t2 - t1;
#else
		finish = clock();
		duration += finish - start;
#endif
		if (!done) {
			readlen = fread((void *)new_speech, 1, L_FRAME, fsrc);
			input = new_speech;
			if (feof(fsrc) && readlen == 0)
				done = 1;
		}

	} while (!done && returnCode);

	//#######################################   End Encoding Section   #########################################

safe_exit:
	returnCode = AudioAPI.Uninit(hCodec);

#ifdef _WIN32_WCE
	wsprintf(msg, TEXT("Decode Time: %d clocks, decode frames: %d"), total, framecount);
	MessageBox(NULL, msg, TEXT("AC3 decode Finished"), MB_OK);
#else
	printf( "\n%2.5f seconds\n", (double)duration/CLOCKS_PER_SEC);
#endif

	if (fsrc)
		fclose(fsrc);
	if (fdst)
		fclose(fdst);
#ifdef LINUX
	dlclose(handle);
#endif
	return ret;
}

#ifdef _WIN32_WCE
int WinMain(int argc, TCHAR **argv) 
#else // _WIN32_WCE
int main(int argc, char **argv)  // for gcc compiler;
#endif//_WIN32_WCE
{
	int   r, count;
	char   *inFileName,*outFileName;

	printf("hehe!\n");
	for (count = 1; count < argc; count++)
	{
		if (*(argv[count]) == '-')
		{
			parse_cmd(argv[count] + 1);
			count++;
		}
		else
		{
			show_help();
		}
	}

#ifdef _WIN32_WCE
	inFileName  = "/My Storage/ac3/BSI18.ac3";
	outFileName = "/My Storage/ac3/BSI18.pcm";
#else
	inFileName  = ac3fname;
	outFileName = pcmfname;
#endif
	r = decoder(inFileName, outFileName);
	if(r)
	{
		fprintf(stderr, "error: %d\n", r);
	}
	return r;
}

void parse_cmd(char *input)
{
	double temp;
	int chanval;
	char *routeptr;

	switch (*input)
	{
	case 'b':
	case 'B':
		wordsize = atoi(input + 1);
		if (!((wordsize == 0) || (wordsize == 1)
			|| ((wordsize >= 17) && (wordsize <= 24))))
		{
			//error_msg("parse_cmdline: PCM word size must be 0, 1, or"
			//	" and integer number between 17 and 24.", FATAL);
		}
		break;
#ifdef KCAPABLE
	case 'c':
	case 'C':
		kcapablemode = atoi(input + 1);
		if ((kcapablemode < 0) || (kcapablemode > 3))
		{
			//error_msg("decode: Karaoke capable mode out of range", FATAL);
		}
		break;
#endif
	case 'd':
	case 'D':
		if (!sscanf(input + 1, "%X", &debug_arg))
		{
			//error_msg("decode: Invalid debug level", FATAL);
		}
		break;
	case 'h':
	case 'H':
		show_usage();
		break;
	case 'i':
	case 'I':
		ac3fname = input + 2;
		break;
	case 'k':
	case 'K':
		compmode = atoi(input + 1);
		if ((compmode < 0) || (compmode > 3))
		{
			//error_msg("decode: Compression mode out of range", FATAL);
		}
		break;
	case 'l':
	case 'L':
		outlfeon = atoi(input + 1);
		if ((outlfeon < 0) || (outlfeon > 1))
		{
			//error_msg("decode: LFE output mode out of range", FATAL);
		}
		break;
	case 'm':
	case 'M':
		outputmode = (short)(atoi(input + 1));
		if ((outputmode < 0) || (outputmode > 7))
		{
			//error_msg("decode: Output mode must be between 0 and 7",
			//	FATAL);
		}
		break;
	case 'n':
	case 'N':
		numchans = atoi(input + 1);
		if ((numchans < 1) || (numchans > 6))
		{
			//error_msg("parse_cmdline: Total number of channels must be"
			//	" between 1 and 6.", FATAL);
		}
		break;
	case 'o':
	case 'O':
		pcmfname = input + 2;
		break;
	case 'p':
	case 'P':
		pcmscalefac = atof(input + 1);
		if ((pcmscalefac < 0.0) || (pcmscalefac > 1.0))
		{
			//error_msg("decode: PCM scale factor must be between 0.0"
			//	" and 1.0", FATAL);
		}
		break;
	case 'r':
	case 'R':
		range(input + 1, &framestart, &frameend);
		if (framestart < 0)
		{
			//error_msg("decode: starting frame number must be positive",
			//	FATAL);
		}
		if ((frameend >= 0) && (frameend <= framestart))
		{
			//error_msg("decode: ending frame number must be >= than"
			//	" starting frame number", FATAL);
		}
		break;
	case 's':
	case 'S':
		stereomode = (short)(atoi(input + 1));
		if ((stereomode < 0) || (stereomode > 2))
		{
			//error_msg("decode: Stereo output mode must be between 0 and 2",
			//	FATAL);
		}
		break;
	case 'u':
	case 'U':
		dualmonomode = (short)(atoi(input + 1));
		if ((dualmonomode < 0) || (dualmonomode > 3))
		{
			//error_msg("decode: Dual mono mode must be between 0 and 3",
			//	FATAL);
		}
		break;
	case 'v':
	case 'V':
		useverbose = 1;
		break;
	case 'x':
	case 'X':
		temp = atof(input + 1);
		dynrngscalehi = (int)(temp * 2147483647);

		//dynrngscalehi = strtol(input+1, NULL, 16);
		break;
	case 'y':
	case 'Y':
		temp = atof(input + 1);
		dynrngscalelow = (int)(temp * 2147483647);
		//dynrngscalelow = atof(input + 1);
		//dynrngscalelow = strtol (input+1, NULL, 16);
		break;
	case 'z':
	case 'Z':
		outputflg = 0;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		chanval = *input - '0';
		routeptr = input + 1;
		while (*routeptr)
		{
			switch (*routeptr)
			{
			case 'L':
				chanptr[0] = chanval;
				break;
			case 'C':
				chanptr[1] = chanval;
				break;
			case 'R':
				chanptr[2] = chanval;
				break;
			case 'l':
				chanptr[3] = chanval;
				break;
			case 'r':
				chanptr[4] = chanval;
				break;
			case 's':
				chanptr[5] = chanval;
				break;
			default:
				break;
			}
			routeptr += 1;
		}
		break;
	default:
		show_help();
		break;
	}
}

/*	Parse form start[:[end]] */

void range(char *inputarg, int *lower, int *upper)
{
	char *colonptr;

	if ((colonptr = strchr(inputarg, ':')) == NULL)
	{
		*lower = atoi(inputarg);
		*upper = -1;
	}
	else
	{
		*colonptr = '\0';
		*lower = atoi(inputarg);
		if (*(colonptr + 1))
		{
			*upper = atoi(colonptr + 1);
		}
		else
		{
			*upper = -1;
		}
	}
}

void show_help(void)
{
	puts("Invalid argument.  Type DECODE -h for help.\n");
	exit(1);
}

void show_usage(void)
{
	puts(
#ifdef KCAPABLE
		"Usage: DECODE [-b<mode>] [-c<mode>] [-d<level>] [-h]\n"
		"              [-i<filename.ext>] [-k<mode>] [-l<0/1>] [-m<mode>]\n"
		"              [-n<#chans>] [-o<filename.ext>] [-p<scale val>]\n"
		"              [-r[<framestart>][:<frameend>]] [-u<mode>] [-v]\n"
		"              [-x<scale val>] [-y<scale val>] [-z] [-0..5<channel>]\n\n"
#else
		"Usage: DECODE [-b<mode>] [-d<level>] [-h] [-i<filename.ext>]\n"
		"              [-k<mode>] [-l<0/1>] [-m<mode>] [-n<#chans>]\n"
		"              [-o<filename.ext>] [-p<scale val>] [-r[<framestart>]\n"
		"              [:<frameend>]] [-u<mode>] [-v] [-x<scale val>] \n"
		"              [-y<scale val>] [-z] [-0..5<channel>]\n\n"
#endif
		"         -b     Output file word size:\n"
		"                  0 = 16-bit short integer\n"
		"                  1 = 64-bit double precision floating point\n"
		"                  17:24 = 32-bit long integer with specified precision\n"
#ifdef KCAPABLE
		"         -c     Karaoke capable reproduction mode (default 3)\n"
		"                  0 = no vocal\n"
		"                  1 = left vocal\n"
		"                  2 = right vocal\n"
		"                  3 = both vocals\n"
#endif
		"         -d     Debug level (hex, default 0)\n"
		"                  0001 = bitstream values\n"
		"                  0002 = packed exponents\n"
		"                  0004 = expanded exponents\n"
		"                  0008 = bit allocation data\n"
		"                  0010 = packed mantissas"
		);
	puts(
		"                  0020 = dither values\n"
		"                  0080 = expanded mantissas\n"
		"                  0100 = coupling data\n"
		"                  0200 = downmix table\n"
		"                  0400 = downmix buffers\n"
		"                  0800 = inverse transform data\n"
		"                  1000 = PCM data\n"
		"                  8000 = Bitstream position data\n"
		"         -h     Show this usage message and abort\n"
		"         -i     Input AC-3 file name (default output.ac3)\n"
		"         -k     Dynamic range compression mode (default 2)\n"
		"                  0 = custom mode, analog dialnorm\n"
		"                  1 = custom mode, digital dialnorm\n"
		"                  2 = line out mode\n"
		"                  3 = RF remod mode\n"
		"         -l     Output lfe channel present (default 1)\n"
		"         -m     Output channel configuration (default 7)\n"
		"                  0 = reserved\n"
		"                  1 = 1/0 (C)\n"
		"                  2 = 2/0 (L, R)\n"
		"                  3 = 3/0 (L, C, R)"
		);
	puts(
		"                  4 = 2/1 (L, R, l)\n"
		"                  5 = 3/1 (L, C, R, l)\n"
		"                  6 = 2/2 (L, R, l, r)\n"
		"                  7 = 3/2 (L, C, R, l, r)\n"
		"         -n     Number of output channels (default 6)\n"
		"         -o     Output PCM file name (default output.pcm)\n"
		"         -p     PCM scale factor (default 1.0)\n"
		"         -r     Region to process in frames (default 0 to end)\n"
		"         -s     Stereo output mode (default 0)\n"
		"                Only effective when -m = 2\n"
		"                  0 = auto detect\n"
		"                  1 = Dolby Surround compatible (Lt/Rt)\n"
		"                  2 = Stereo (Lo/Ro)\n"
		"         -u     Dual mono reproduction mode (default 0)\n"
		"                  0 = Stereo\n"
		"                  1 = Left mono\n"
		"                  2 = Right mono\n"
		"                  3 = Mixed mono\n"
		"         -v     Verbose mode. Display frame number and % done\n"
		"         -x     Dynamic range compression cut scale factor (default 1.0)\n"
		"         -y     Dynamic range compression boost scale factor (default 1.0)\n"
		"         -z     Suppress output file"
		);
	puts(
		"         -0..5  Channel routing information\n"
		"                  Route arbitrary input channels (L, C, R, l, r, s) to\n"
		"                  arbitrary interleaved output channel (0..5).\n"
		"                  Example: -0L -1R routes left bitstream channel to first\n"
		"                  interleaved output channel, and routes right bitstream\n"
		"                  channel to second interleaved output channel.\n"
		"                  Note: use l to designate mono surround in 2/1 or 3/1 modes,\n"
		"                  use L and R to designate independent channels in 1+1 mode.\n"
		"                  Default: -0L -1C -2R -3l -4r -5s\n"
		);
	exit(0);
}


