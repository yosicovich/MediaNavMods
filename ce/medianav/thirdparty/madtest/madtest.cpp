#ifdef _WIN32_WCE
#include 	<windows.h>
#include 	<objbase.h>
#include 	<Winbase.h>
#endif // _WIN32_WCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32_WCE
#include <sys/timeb.h>
#endif
#include <math.h>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif 
#include <msvc++/config.h>

#include <stream.h>
#include <frame.h>
#include <synth.h>

#ifdef __cplusplus
}
#endif

struct BuffersState
{
	const unsigned char* inData;
	size_t  inDataSize;
};

static unsigned char wav_header[] = {
	'R', 'I', 'F', 'F', 0xfc, 0xff, 0xff, 0xff, 'W', 'A', 'V', 'E',
	'f', 'm', 't', ' ', 16, 0, 0, 0,
	1, 0, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1, 4, 0, 16, 0,
	'd', 'a', 't', 'a', 0xd8, 0xff, 0xff, 0xff
};

static void print_fps(int final);

class Decoder
{
public:
	Decoder(FILE* output)
		: m_output(output)
		, m_headerWritten(false)
		, m_wavSize(0)
	{
	}
	virtual ~Decoder()
	{
	}
	virtual size_t decodeOne(BuffersState& bufferState) = 0;
	virtual size_t getBufferSize() const = 0;

	virtual void decodeFinish()
	{
		if (m_headerWritten)
		{
			unsigned char* header = m_wavHeader.get();
			*((unsigned int *)&header[4]) = m_wavSize;
			*((unsigned int *)&header[40]) = m_wavSize + 36;
			fseek(m_output, 0, SEEK_SET);
			fwrite(header, sizeof(wav_header), 1, m_output);
		}
	}

protected:

	virtual short convert(int value) = 0;
	void setupOutput(unsigned short channels, unsigned int sampleRate)
	{
		m_wavHeader.reset(new unsigned char[sizeof(wav_header)]);
		unsigned char* header = m_wavHeader.get();
		memcpy(header, wav_header, sizeof(wav_header));
		*((unsigned short *)&header[22]) = channels;
		*((unsigned int *)&header[24]) = sampleRate;
		*((unsigned int *)&header[28]) = sampleRate * channels * 2;
		fwrite(header, sizeof(wav_header), 1, m_output);
		m_wavSize = 0;
	}

	void outputSamples(const mad_pcm& pcm)
	{
		if (!m_headerWritten)
		{
			m_headerWritten = true;
			setupOutput(pcm.channels, pcm.samplerate);
		}
		static short buffer[2 * 1152]; // 2 channels * max samples per channel
		int bufPos = 0;
		for (int i = 0; i < pcm.length; ++i)
		{
			buffer[bufPos++] = convert(pcm.samples[0][i]);
			if(pcm.channels > 1)
				buffer[bufPos++] = convert(pcm.samples[1][i]);
		}

		size_t dataSize = pcm.length * 2 * pcm.channels;
		//fwrite(buffer, dataSize, 1, m_output);
		m_wavSize += dataSize;
	}
	FILE* m_output;
	bool m_headerWritten;
private:
	std::auto_ptr<unsigned char> m_wavHeader;
	size_t m_wavSize;

};


class MP3Decoder : public Decoder
{
public:
	MP3Decoder(FILE* output)
		:Decoder(output)
	{
		mad_stream_init(&stream);
		mad_frame_init(&frame);
		mad_synth_init(&synth);

		//mad_stream_options(&stream, opt)
	}

	virtual ~MP3Decoder()
	{
		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);
	}

	virtual size_t decodeOne(BuffersState& bufferState)
	{
		mad_stream_buffer(&stream, bufferState.inData, bufferState.inDataSize);
		/*if (mad_header_decode(&frame.header, &stream) == -1) 
		{
			if (!MAD_RECOVERABLE(stream.error))
				return false;
			return true;
		}*/

		do
		{
			if (mad_frame_decode(&frame, &stream) == 0 || stream.error == MAD_ERROR_BADCRC)
			{
				mad_synth_frame(&synth, &frame);
				outputSamples(synth.pcm);
				print_fps(0);
				continue;
			}
		} while (MAD_RECOVERABLE(stream.error));

		if (stream.error != MAD_ERROR_BUFLEN)
			return -1;

		const unsigned char* framePtr;
		if (stream.next_frame)
			framePtr = stream.next_frame;
		else
			framePtr = stream.this_frame;
		bufferState.inDataSize -= framePtr - bufferState.inData;
		bufferState.inData = framePtr;
		return 0;
	}

	virtual size_t getBufferSize() const
	{
		return 40000;
	}
protected:
	virtual short convert(int i)
	{
		// input 28bit float + 3bit whole + 1bit sing
		// output 15bit data + 1bit sign
		// MAD gives output in range wider than from -1.0 to 1.0 so We take 1 whole as well.
		// So we have to shrink 29bit(28 + 1) input to 15bit output.-> 29 - 15 = 14
		i = i >> 14;
		if (i > 32767)
			return 32767;
		else if (i < -32767)
			return -32768;
		else
			return i;
	}

private:
	mad_stream stream;
	mad_frame frame;
	mad_synth synth;
};

static FILE * in_file = NULL;
static FILE * out_file = NULL;


#ifdef HAVE_GETTIMEOFDAY

static RETSIGTYPE signal_handler(int sig)
{
	print_fps(1);
	signal(sig, SIG_DFL);
	raise(sig);
}

static void print_fps(int final)
{
	static unsigned int frame_counter = 0;
	static struct timeval tv_beg, tv_start;
	static int total_elapsed;
	static int last_count = 0;
	struct timeval tv_end;
	float fps, tfps;
	int frames, elapsed;

	gettimeofday(&tv_end, NULL);

	if (!frame_counter) {
		tv_start = tv_beg = tv_end;
		signal(SIGINT, signal_handler);
	}

	elapsed = (tv_end.tv_sec - tv_beg.tv_sec) * 100 +
		(tv_end.tv_usec - tv_beg.tv_usec) / 10000;
	total_elapsed = (tv_end.tv_sec - tv_start.tv_sec) * 100 +
		(tv_end.tv_usec - tv_start.tv_usec) / 10000;

	if (final) {
		if (total_elapsed)
			tfps = frame_counter * 100.0 / total_elapsed;
		else
			tfps = 0;

		fprintf(stderr, "\n%d frames decoded in %.2f seconds (%.2f fps)\n",
			frame_counter, total_elapsed / 100.0, tfps);

		return;
	}

	frame_counter++;

	if (elapsed < 50)	// only display every 0.50 seconds
		return;

	tv_beg = tv_end;
	frames = frame_counter - last_count;

	fps = frames * 100.0 / elapsed;
	tfps = frame_counter * 100.0 / total_elapsed;

	fprintf(stderr, "%d frames in %.2f sec (%.2f fps), "
		"%d last %.2f sec (%.2f fps)\033[K\r", frame_counter,
		total_elapsed / 100.0, tfps, frames, elapsed / 100.0, fps);

	last_count = frame_counter;
}

#else // !HAVE_GETTIMEOFDAY

long getMeasureStamp()
{
#ifdef _WIN32_WCE
	return GetTickCount() / 10;
#else
	struct timeb tm;
	ftime(&tm);
	return (long)tm.time * 100 + tm.millitm / 10;
#endif
}
static void print_fps(int final)
{
	static unsigned int frame_counter = 0;
	static long tbeg, tstart;
	static int total_elapsed = 0;
	static int last_count = 0;
	long tend;
	double fps, tfps;
	int frames, elapsed;

	tend = getMeasureStamp();

	if (!frame_counter) {
		tstart = tbeg = tend;
		//signal (SIGINT, signal_handler);
	}

	elapsed = tend - tbeg;
	total_elapsed = tend - tstart;

	if (final) {
		if (total_elapsed)
			tfps = frame_counter * 100.0 / total_elapsed;
		else
			tfps = 0;

		fprintf(stderr, "\n%d frames decoded in %.2f seconds (%.2f fps)\n",
			frame_counter, total_elapsed / 100.0, tfps);

		return;
	}

	frame_counter++;

	if (elapsed < 50)	// only display every 0.50 seconds
		return;

	tbeg = tend;
	frames = frame_counter - last_count;

	fps = frames * 100.0 / elapsed;
	tfps = frame_counter * 100.0 / total_elapsed;

	fprintf(stderr, "%d frames in %.2f sec (%.2f fps), "
		"%d last %.2f sec (%.2f fps)\033[K\r", frame_counter,
		total_elapsed / 100.0, tfps, frames, elapsed / 100.0, fps);

	last_count = frame_counter;
}

#endif

#ifndef _WIN32_WCE
static void print_usage(char ** argv)
{
    fprintf(stderr, "usage: "
        "%s <in file> [out file]\r\n" , argv[0]);

    exit(1);
}

static void handle_args(int argc, char ** argv)
{
	if (argc < 2)
	{
		print_usage(argv);
		exit(1);
	}

	in_file = fopen(argv[1], "rb");
	if (!in_file) {
		fprintf(stderr, "Could not open file %s\n", argv[1]);
		exit(1);
	}

	char *outName = "out.wav";
	if (argc > 2)
	{
		outName = argv[2];
	}

	out_file = fopen(outName, "wb");
	if (!out_file)
	{
		fclose(in_file);
		exit(2);
	}
}
#else
static void print_usage(wchar_t ** argv)
{
    fprintf(stderr, "usage: "
        "%s <in file> [out file]\r\n" , argv[0]);

    exit(1);
}

static void handle_args(int argc, wchar_t ** argv)
{
    if (argc < 2)
    {
        print_usage(argv);
        exit(1);
    }

    in_file = _wfopen(argv[1], L"rb");
    if (!in_file) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        exit(1);
    }

    wchar_t *outName = L"out.wav";
    if (argc > 2)
    {
        outName = argv[2];
    }

    out_file = _wfopen(outName, L"wb");
    if (!out_file)
    {
        fclose(in_file);
        exit(2);
    }
}
#endif

#ifdef _WIN32_WCE
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char ** argv)
#endif
{
	handle_args(argc, argv);

	MP3Decoder decoder(out_file);

	size_t chunkSize = decoder.getBufferSize();
	size_t size;

	unsigned char* buffer = new unsigned char[chunkSize];
	size_t bufPos = 0;
	do {
		size = fread(&buffer[bufPos], 1, chunkSize - bufPos, in_file);

        size = bufPos + size;
		BuffersState bufferState;
		bufferState.inData = buffer;
		bufferState.inDataSize = size;

		size_t state;
		while (bufferState.inDataSize && (state = decoder.decodeOne(bufferState)) > 0)
			;

		if (state == -1)
		{
			fprintf(stderr, "decodeOne() error\r\n");
			break;
		}
		else //if (state == 0)
		{
			// underflow
			if (bufferState.inDataSize)
			{
				bufPos = bufferState.inDataSize;
				memcpy(buffer, bufferState.inData, bufPos);
			}
			else
			{
				bufPos = 0;
			}
		}

	} while (size == chunkSize);
	
	decoder.decodeFinish();
	
	print_fps(1);
	delete buffer;
	fclose(in_file);
	fclose(out_file);
	return 0;
}
