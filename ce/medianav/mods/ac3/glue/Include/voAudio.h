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

#ifndef __voAudio_H__
#define __voAudio_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "voIndex.h"
#include "voMem.h"

#define	VO_PID_AUDIO_BASE			 0x42000000							/*!< The base param ID for AUDIO codec */
#define	VO_PID_AUDIO_FORMAT			(VO_PID_AUDIO_BASE | 0X0001)		/*!< The format data of audio in track */
#define	VO_PID_AUDIO_SAMPLEREATE	(VO_PID_AUDIO_BASE | 0X0002)		/*!< The samplerate of audio  */
#define	VO_PID_AUDIO_CHANNELS		(VO_PID_AUDIO_BASE | 0X0003)		/*!< The channel of audio */
#define	VO_PID_AUDIO_BITRATE		(VO_PID_AUDIO_BASE | 0X0004)		/*!< The bitrate of audio */
#define VO_PID_AUDIO_CHANNELMODE	(VO_PID_AUDIO_BASE | 0X0005)		/*!< The channel mode of audio */
#define VO_PID_AUDIO_CHANNELCONFIG	(VO_PID_AUDIO_BASE | 0X0006)		/*!< The channel config of audio */

#define VO_PID_AUDIO_EFFECTMODE		(VO_PID_AUDIO_BASE | 0X0100)		/*!< The mode of audio effect */
#define VO_PID_AUDIO_EFFECTCONFIG	(VO_PID_AUDIO_BASE | 0X0200)		/*!< The config of audio effect */

#define	VO_ERR_AUDIO_BASE			0x82000000
#define VO_ERR_AUDIO_UNSCHANNEL		VO_ERR_AUDIO_BASE | 0x0001
#define VO_ERR_AUDIO_UNSSAMPLERATE	VO_ERR_AUDIO_BASE | 0x0002
#define VO_ERR_AUDIO_UNSFEATURE		VO_ERR_AUDIO_BASE | 0x0003


/**
 *Enumeration used to define the possible audio codings.
 */
typedef enum VO_AUDIO_CODINGTYPE {
	VO_AUDIO_CodingUnused = 0,  /**< Placeholder value when coding is N/A  */
	VO_AUDIO_CodingPCM,         /**< Any variant of PCM coding */
	VO_AUDIO_CodingADPCM,       /**< Any variant of ADPCM encoded data */
	VO_AUDIO_CodingAMRNB,       /**< Any variant of AMR encoded data */
	VO_AUDIO_CodingAMRWB,       /**< Any variant of AMR encoded data */
	VO_AUDIO_CodingAMRWBP,      /**< Any variant of AMR encoded data */
	VO_AUDIO_CodingQCELP13,     /**< Any variant of QCELP 13kbps encoded data */
	VO_AUDIO_CodingEVRC,        /**< Any variant of EVRC encoded data */
	VO_AUDIO_CodingAAC,         /**< Any variant of AAC encoded data, 0xA106 - ISO/MPEG-4 AAC, 0xFF - AAC */
	VO_AUDIO_CodingAC3,         /**< Any variant of AC3 encoded data */
	VO_AUDIO_CodingFLAC,        /**< Any variant of FLAC encoded data */
	VO_AUDIO_CodingMP1,			/**< Any variant of MP1 encoded data */
	VO_AUDIO_CodingMP3,         /**< Any variant of MP3 encoded data */
	VO_AUDIO_CodingOGG,         /**< Any variant of OGG encoded data */
	VO_AUDIO_CodingWMA,         /**< Any variant of WMA encoded data */
	VO_AUDIO_CodingRA,          /**< Any variant of RA encoded data */
	VO_AUDIO_CodingMIDI,        /**< Any variant of MIDI encoded data */
	VO_AUDIO_CodingDRA,         /**< Any variant of dra encoded data */
	VO_AUDIO_CodingG729,        /**< Any variant of dra encoded data */
	VO_AUDIO_CodingEAC3,		/**< Any variant of Enhanced AC3 encoded data */
	VO_AUDIO_CodingAPE,			/**< Any variant of APE encoded data */
	VO_AUDIO_CodingALAC,		/**< Any variant of ALAC encoded data */
	VO_AUDIO_CodingDTS,			/**< Any variant of DTS encoded data */
	VO_AUDIO_CodingSBC,         /**< Any variant of SBC encoded data */                
	VO_AUDIO_CodingG722,        /**< Any variant of G722 encoded data */
	VO_AUDIO_CodingG723,        /**< Any variant of G723 encoded data */
	VO_AUDIO_CodingG726,        /**< Any variant of G726 encoded data */
	VO_AUDIO_CodingG711,        /**< Any variant of G711 codec data */
	VO_AUDIO_AudioSpeed,        /**< Mutli-Speed ID > */
	VO_AUDIO_CodingGSM610,		/**< Any variant of GSM6.10 codec data */
	VO_AUDIO_Coding_MAX		= VO_MAX_ENUM_VALUE
} VO_AUDIO_CODINGTYPE;

/**
 *Enumeration used to define the possible audio effect type.
 */
typedef enum VO_AUDIO_EFFECTTYPE {
	VO_AUDIO_EffectUnused = 1000,  /**< Placeholder value when coding is N/A  */
	VO_AUDIO_Equalzier,        /**< Any variant of audio equalizer effect */
	VO_AUDIO_DolbyMoblie,       /**< Any variant of dolby mobile audio effect */
	VO_AUDIO_EFFECT_MAX		= VO_MAX_ENUM_VALUE
} VO_AUDIO_EFFECTTYPE;

/*!
* the channel type value
*/
typedef enum {
	VO_CHANNEL_CENTER				= 1,	/*!<center channel*/
	VO_CHANNEL_FRONT_LEFT			= 1<<1,	/*!<front left channel*/
	VO_CHANNEL_FRONT_RIGHT			= 1<<2,	/*!<front right channel*/
	VO_CHANNEL_SIDE_LEFT  			= 1<<3, /*!<side left channel*/
	VO_CHANNEL_SIDE_RIGHT			= 1<<4, /*!<side right channel*/
	VO_CHANNEL_BACK_LEFT			= 1<<5,	/*!<back left channel*/
	VO_CHANNEL_BACK_RIGHT			= 1<<6,	/*!<back right channel*/
	VO_CHANNEL_BACK_CENTER			= 1<<7,	/*!<back center channel*/
	VO_CHANNEL_LFE_BASS				= 1<<8,	/*!<low-frequency effects bass channel*/
	VO_CHANNEL_ALL					= 0xffff,/*!<[default] decode all channels */
	VO_CHANNEL_MAX					= VO_MAX_ENUM_VALUE
} VO_AUDIO_CHANNELTYPE;


/**
 * General channel mode
 */
typedef enum
{
	VO_AUDIO_CHANNEL_STEREO			= 0x0000,		/**< stereo channel */
	VO_AUDIO_CHANNEL_JOINTSTEREO	= 0x0001,		/**< joint stereo channel */
	VO_AUDIO_CHANNEL_DUALMONO		= 0x0002,		/**< dual mono(bilingual)channel  */
	VO_AUDIO_CHANNEL_MONO			= 0x0003,		/**< Mono channel */
	VO_AUDIO_CHANNEL_MULTICH		= 0x7F000001,	/**< multichannel mode, channel number gt 2 */
	VO_AUDIO_CHANNEL_MODE_MAX		= VO_MAX_ENUM_VALUE
}VO_AUDIO_CHANNELMODE;


/**
 * General channel config
 */
typedef enum
{
	VO_AUDIO_CHAN_NULL				= 0x0000,		/**< no channel */
	VO_AUDIO_CHAN_MONO				= 0x0001,		/**< Mono channel */
	VO_AUDIO_CHAN_DUALONE			= 0x0002,		/**< double mono channel */
	VO_AUDIO_CHAN_DUALMONO			= 0x0010,		/**< dual mono(bilingual)channel  */
	VO_AUDIO_CHAN_DUALLEFT			= 0x0011,		/**< dual left channel */
	VO_AUDIO_CHAN_DUALRIGHT			= 0x0012,		/**< dual right channel */
	VO_AUDIO_CHAN_STEREO			= 0x0020,		/**< stereo channel */
	VO_AUDIO_CHAN_STE2MONO			= 0x0021,		/**< stereo channel to Mono */
	VO_AUDIO_CHAN_MULTI				= 0x0030,		/**< multichannel, channel number gt 2 */
	VO_AUDIO_CHAN_MULDOWNMIX2		= 0x0031,		/**< multichannel,downmix to 2 channel */
	VO_AUDIO_CHAN_CONFIG_MAX		= VO_MAX_ENUM_VALUE
}VO_AUDIO_CHANNELCONFIG;


/**
 * General wave format info
 */
#define VO_WAVEFORMAT_STRUCTLEN		16
typedef struct _VO_WAVEFORMAT
{
	VO_U16		wFormatTag;         /**< format type */
	VO_U16		nChannels;          /**< number of channels (i.e. mono, stereo...) */
	VO_U32		nSamplesPerSec;     /**< sample rate */
	VO_U32		nAvgBytesPerSec;    /**< for buffer estimation */
	VO_U16		nBlockAlign;        /**< block size of data */
	VO_U16		wBitsPerSample;     /**< number of bits per sample of mono data */
} VO_WAVEFORMAT;

#define VO_WAVEFORMATEX_STRUCTLEN		18
typedef struct _VO_WAVEFORMATEX
{
	VO_U16		wFormatTag;         /**< format type */
	VO_U16		nChannels;          /**< number of channels (i.e. mono, stereo...) */
	VO_U32		nSamplesPerSec;     /**< sample rate */
	VO_U32		nAvgBytesPerSec;    /**< for buffer estimation */
	VO_U16		nBlockAlign;        /**< block size of data */
	VO_U16		wBitsPerSample;     /**< number of bits per sample of mono data */
	VO_U16		cbSize;             /**< the count in bytes of the size of */
									/**< extra information (after cbSize) */
} VO_WAVEFORMATEX;

typedef struct _VO_WAVEFORMATEXTENSIBLE
{
	VO_U16		wFormatTag;         /**< format type */
	VO_U16		nChannels;          /**< number of channels (i.e. mono, stereo...) */
	VO_U32		nSamplesPerSec;     /**< sample rate */
	VO_U32		nAvgBytesPerSec;    /**< for buffer estimation */
	VO_U16		nBlockAlign;        /**< block size of data */
	VO_U16		wBitsPerSample;     /**< number of bits per sample of mono data */
	VO_U16		cbSize;             /**< the count in bytes of the size of */
	union {
		VO_U16  wValidBitsPerSample;
		VO_U16  wSamplesPerBlock;
		VO_U16  wReserved;
	} Samples;

	VO_U32   			dwChannelMask;
	VO_GUID  			SubFormat;
} VO_WAVEFORMATEXTENSIBLE;

/**
 * General audio format info
 */
typedef struct
{
	VO_S32	SampleRate;  /*!< Sample rate */
	VO_S32	Channels;    /*!< Channel count */
	VO_S32	SampleBits;  /*!< Bits per sample */
} VO_AUDIO_FORMAT;

/**
 * General audio output info
 */
typedef struct
{
	VO_AUDIO_FORMAT	Format;			/*!< Sample rate */
	VO_U32			InputUsed;		/*!< Channel count */
	VO_U32			Resever;		/*!< Resevered */
} VO_AUDIO_OUTPUTINFO;

/**
 * General audio codec function set
 */
typedef struct
{
	/**
	 * Init the audio codec module and return codec handle
	 * \param phCodec [OUT] Return the video codec handle
	 * \param vType	[IN] The codec type if the module support multi codec.
	 * \param pUserData	[IN] The init param. It is memory operator or alloced memory
	 * \retval VO_ERR_NONE Succeeded.
	 */
	VO_U32 (VO_API * Init) (VO_HANDLE * phCodec,VO_AUDIO_CODINGTYPE vType, VO_CODEC_INIT_USERDATA * pUserData );

	/**
	 * Set input audio data.
	 * \param hCodec [IN]] The Codec Handle which was created by Init function.
	 * \param pInput [IN] The input buffer param.
	 * \param pOutBuffer [OUT] The output buffer info.
	 * \retval VO_ERR_NONE Succeeded.
	 */
	VO_U32 (VO_API * SetInputData) (VO_HANDLE hCodec, VO_CODECBUFFER * pInput);

	/**
	 * Get the outut audio data
	 * \param hCodec [IN]] The Codec Handle which was created by Init function.
	 * \param pOutBuffer [OUT] The output audio data
	 * \param pOutInfo [OUT] The dec module filled audio format and used the input size.
	 *						 pOutInfo->InputUsed is total used the input size.
	 * \retval  VO_ERR_NONE Succeeded.
	 *			VO_ERR_INPUT_BUFFER_SMALL. The input was finished or the input data was not enought.
	 */
	VO_U32 (VO_API * GetOutputData) (VO_HANDLE hCodec, VO_CODECBUFFER * pOutBuffer, VO_AUDIO_OUTPUTINFO * pOutInfo);

	/**
	 * Set the param for special target.
	 * \param hCodec [IN]] The Codec Handle which was created by Init function.
	 * \param uParamID [IN] The param ID.
	 * \param pData [IN] The param value depend on the ID>
	 * \retval VO_ERR_NONE Succeeded.
	 */
	VO_U32 (VO_API * SetParam) (VO_HANDLE hCodec, VO_S32 uParamID, VO_PTR pData);

	/**
	 * Get the param for special target.
	 * \param hCodec [IN]] The Codec Handle which was created by Init function.
	 * \param uParamID [IN] The param ID.
	 * \param pData [IN] The param value depend on the ID>
	 * \retval VO_ERR_NONE Succeeded.
	 */
	VO_U32 (VO_API * GetParam) (VO_HANDLE hCodec, VO_S32 uParamID, VO_PTR pData);

	/**
	 * Uninit the Codec.
	 * \param hCodec [IN]] The Codec Handle which was created by Init function.
	 * \retval VO_ERR_NONE Succeeded.
	 */
	VO_U32 (VO_API * Uninit) (VO_HANDLE hCodec);
} VO_AUDIO_CODECAPI;

/**
 * audio render call back function. It will be called before audio render.
 * \param pUserData [IN]] The user data pointer which was setted by caller.
 * \param pAudioBuffer [IN] The audio buffer info.
 * \param pAudioFormat [IN] The audio format
 * \param nStart [IN] The audio time.
 * \retval VO_ERR_NONE Succeeded.
 *		   VO_ERR_FINISH, the render will not render audio..
 */
typedef VO_S32 (VO_API * VOAUDIOCALLBACKPROC) (VO_PTR pUserData, VO_CODECBUFFER * pAudioBuffer, VO_AUDIO_FORMAT * pAudioFormat, VO_S32 nStart);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __voAudio_H__
