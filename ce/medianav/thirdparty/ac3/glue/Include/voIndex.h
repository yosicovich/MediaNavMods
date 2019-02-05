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

#ifndef __voIndex_H__
#define __voIndex_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "voType.h"

/* Define the module ID */
#define _MAKE_SOURCE_ID(id, name) \
VO_INDEX_SRC_##name = _VO_INDEX_SOURCE | id,

#define _MAKE_CODEC_ID(id, name) \
VO_INDEX_DEC_##name = _VO_INDEX_DEC | id, \
VO_INDEX_ENC_##name = _VO_INDEX_ENC | id,

#define _MAKE_EFFECT_ID(id, name) \
VO_INDEX_EFT_##name = _VO_INDEX_EFFECT | id,

#define _MAKE_SINK_ID(id, name) \
VO_INDEX_SNK_##name = _VO_INDEX_SINK | id,

#define _MAKE_FILTER_ID(id, name) \
VO_INDEX_FLT_##name = _VO_INDEX_FILTER | id,

#define _MAKE_OMX_ID(id, name) \
VO_INDEX_OMX_##name = _VO_INDEX_OMX | id,

#define _MAKE_MFW_ID(id, name) \
VO_INDEX_MFW_##name = _VO_INDEX_MFW | id,

#define _MAKE_OSMP_ID(id, name) \
VO_INDEX_OSMP_##name = _VO_INDEX_OSMP | id,

#define _MAKE_DRM_ID(id, name) \
VO_INDEX_DRM_##name = _VO_INDEX_DRM | id,
    
#define _MAKE_PLUGIN_ID(id, name) \
VO_INDEX_PLUGIN_##name = _VO_INDEX_PLUGIN | id,
    
#define _MAKE_UTILITY_ID(id, name) \
VO_INDEX_UTILITY_##name = _VO_INDEX_UTILITY | id,

#define _MAKE_CUSTOM_ID(id, name) \
	VO_INDEX_CUSTOM_##name = _VO_INDEX_CUSTOM | id,

enum
{
	_VO_INDEX_SOURCE		= 0x01000000,
	_VO_INDEX_DEC			= 0x02000000,
	_VO_INDEX_ENC			= 0x03000000,
	_VO_INDEX_EFFECT		= 0x04000000,
	_VO_INDEX_SINK			= 0x05000000,
	_VO_INDEX_FILTER		= 0x06000000,
	_VO_INDEX_OMX			= 0x07000000,
	_VO_INDEX_MFW			= 0x08000000,
    _VO_INDEX_OSMP			= 0x09000000,
	_VO_INDEX_DRM			= 0x0A000000,
    _VO_INDEX_PLUGIN		= 0x0B000000,
    _VO_INDEX_UTILITY		= 0x0C000000,
	_VO_INDEX_CUSTOM		= 0x0D000000,
    
	// define file parser modules
	_MAKE_SOURCE_ID (0x010000, MP4)
	_MAKE_SOURCE_ID (0x020000, AVI)
	_MAKE_SOURCE_ID (0x030000, ASF)
	_MAKE_SOURCE_ID (0x040000, REAL)
	_MAKE_SOURCE_ID (0x050000, AUDIO)
	_MAKE_SOURCE_ID (0x060000, FLASH)
	_MAKE_SOURCE_ID (0x070000, OGG)
	_MAKE_SOURCE_ID (0x080000, MKV)
	_MAKE_SOURCE_ID (0x090000, MPEG)
	_MAKE_SOURCE_ID (0x0A0000, RAWDATA)
	_MAKE_SOURCE_ID (0x0B0000, SSP)     // refer to the SSP smooth streaming parser
	_MAKE_SOURCE_ID (0x0C0000, TSP)     // refer to TS parser
	_MAKE_SOURCE_ID (0x0D0000, VMAP)     // refer to VMAP parser
	_MAKE_SOURCE_ID (0x0E0000, SMIL)		//ADDED BY AIVEN
	_MAKE_SOURCE_ID (0x0F0000, XML) 		// ADDED BY AIVEN
	_MAKE_SOURCE_ID (0x001000, DOWNLOADER)	 // ADDED BY AIVEN

	
	// define network source modules
	_MAKE_SOURCE_ID (0x110000, RTSP)
	_MAKE_SOURCE_ID (0x120000, HTTP)
	
	_MAKE_SOURCE_ID (0x121000, HLS)
	_MAKE_SOURCE_ID (0x122000, ISS)
	_MAKE_SOURCE_ID (0x123000, DASH)
	_MAKE_SOURCE_ID (0x124000, MSHTTP)
	_MAKE_SOURCE_ID (0x125000, PD)
	_MAKE_SOURCE_ID (0x126000, PREHTTP)
	_MAKE_SOURCE_ID (0x127000, SOURCEIO)
	_MAKE_SOURCE_ID (0x150000, CONTROLLER)
	_MAKE_SOURCE_ID (0x160000, ADSMANAGER)
	_MAKE_SOURCE_ID (0x170000, PUSHPDMANAGER)
	
	// define CMMB source modules
	_MAKE_SOURCE_ID (0x200000, CMMB)
	_MAKE_SOURCE_ID (0x210000, CMMB_INNO)
	_MAKE_SOURCE_ID (0x220000, CMMB_TELE)
	_MAKE_SOURCE_ID (0x230000, CMMB_SIANO)

	// define DVBT source modules
	_MAKE_SOURCE_ID (0x300000, DVBT)
	_MAKE_SOURCE_ID (0x310000, DVBT_DIBCOM)

	// define other source modules
	_MAKE_SOURCE_ID (0x400000, ID3)

	// define Caption source modules
	_MAKE_SOURCE_ID (0x500000, CLOSEDCAPTION)
	_MAKE_SOURCE_ID (0x510000, EXTSUBTITLE)
	_MAKE_SOURCE_ID (0x520000, DVBSUBTITLE)

	
	// define video codec modules
	_MAKE_CODEC_ID (0x010000, H264)
	_MAKE_CODEC_ID (0x020000, MPEG4)
	_MAKE_CODEC_ID (0x030000, H263)
	_MAKE_CODEC_ID (0x040000, S263)
	_MAKE_CODEC_ID (0x050000, RV)
	_MAKE_CODEC_ID (0x060000, WMV)
	_MAKE_CODEC_ID (0x070000, DIVX3)
	_MAKE_CODEC_ID (0x080000, MJPEG)
	_MAKE_CODEC_ID (0x090000, MPEG2)
	_MAKE_CODEC_ID (0x0A0000, VP6)
	_MAKE_CODEC_ID (0x0B0000, VP8)
	_MAKE_CODEC_ID (0x0C0000, VC1)
	_MAKE_CODEC_ID (0x0D0000, VIDEOPASER)
	_MAKE_CODEC_ID (0x0E0000, H265)
	_MAKE_CODEC_ID (0x100000, HDEC)                    // refer to the Hardware decoder
	_MAKE_CODEC_ID (0x110000, MCDEC)                   // refer to the MediaCodec Decoder


	// define audio codec modules
	_MAKE_CODEC_ID (0x210000, AAC)
	_MAKE_CODEC_ID (0x220000, MP3)
	_MAKE_CODEC_ID (0x230000, WMA)
	_MAKE_CODEC_ID (0x240000, RA)
	_MAKE_CODEC_ID (0x250000, AMRNB)
	_MAKE_CODEC_ID (0x260000, AMRWB)
	_MAKE_CODEC_ID (0x270000, AMRWBP)
	_MAKE_CODEC_ID (0x280000, QCELP)
	_MAKE_CODEC_ID (0x290000, EVRC)
	_MAKE_CODEC_ID (0x2A0000, ADPCM)
	_MAKE_CODEC_ID (0x2B0000, MIDI)
	_MAKE_CODEC_ID (0x2C0000, AC3)
	_MAKE_CODEC_ID (0x2D0000, FLAC)
	_MAKE_CODEC_ID (0x2E0000, DRA)
	_MAKE_CODEC_ID (0x2F0000, OGG)
	_MAKE_CODEC_ID (0x300000, G729)
	_MAKE_CODEC_ID (0x310000, APE)
	_MAKE_CODEC_ID (0x320000, ALAC)
	_MAKE_CODEC_ID (0x330000, EAC3)
	_MAKE_CODEC_ID (0x340000, DTS)
	_MAKE_CODEC_ID (0x350000, SBC)
	_MAKE_CODEC_ID (0x360000, G722)
	_MAKE_CODEC_ID (0x370000, G723)
	_MAKE_CODEC_ID (0x380000, G726)
	_MAKE_CODEC_ID (0x390000, G711)
	_MAKE_CODEC_ID (0x390000, AMC)
	_MAKE_CODEC_ID (0x3A0000, GSM610)
	_MAKE_CODEC_ID (0x3B0000, DSAPLUS)
	// define image codec modules
	_MAKE_CODEC_ID (0x410000, JPEG)
	_MAKE_CODEC_ID (0x420000, GIF)
	_MAKE_CODEC_ID (0x430000, PNG)
	_MAKE_CODEC_ID (0x440000, TIF)

	// define effect modules
	_MAKE_EFFECT_ID (0x010000, EQ)
	_MAKE_EFFECT_ID (0x020000, DOLBY)
	_MAKE_EFFECT_ID (0x030000, AudioSpeed)
	_MAKE_EFFECT_ID (0x040000, Resample)

	// define sink modules
	_MAKE_SINK_ID (0x010000, VIDEO)
	_MAKE_SINK_ID (0x020000, AUDIO)
	_MAKE_SINK_ID (0x030000, CCRRR)
	_MAKE_SINK_ID (0x040000, CCRRV)
	_MAKE_SINK_ID (0x050000, YUVR)						// refer to YUV renderer

	_MAKE_SINK_ID (0x110000, MP4)
	_MAKE_SINK_ID (0x120000, AVI)
	_MAKE_SINK_ID (0x130000, AFW)
	_MAKE_SINK_ID (0x140000, TS)

	// define media frame module ID
	_MAKE_MFW_ID (0x010000, VOMMPLAY)
	_MAKE_MFW_ID (0x020000, VOMMREC)
	_MAKE_MFW_ID (0x030000, VOME)
	_MAKE_MFW_ID (0x040000, VOMP)
	_MAKE_MFW_ID (0x050000, VOEDT)
	_MAKE_MFW_ID (0x060000, VOALS)
	_MAKE_MFW_ID (0x070000, VOLCS)
    _MAKE_MFW_ID (0x080000, VOAVPLAYER)
    
    // define OSMP module ID
	_MAKE_OSMP_ID (0x010000, MEDIAPLAYER)
    _MAKE_OSMP_ID (0x020000, SOURCE)
    _MAKE_OSMP_ID (0x030000, ENGINE)
    _MAKE_OSMP_ID (0x040000, DOWNLOADER)
    _MAKE_OSMP_ID (0x050000, ADMANAGERJNI)
    _MAKE_OSMP_ID (0x060000, DRMCOMMONWRAPJNI)
    _MAKE_OSMP_ID (0x070000, DRMCONAXJNI)
    _MAKE_OSMP_ID (0x080000, MODULEVERSION)
    _MAKE_OSMP_ID (0x090000, DOWNLOADERJNI)
    _MAKE_OSMP_ID (0x0A0000, ANALYTICSJNI)


	// define DRM modules
	_MAKE_DRM_ID(0x010000, VO_AES)
	_MAKE_DRM_ID(0x020000, SHOWTIME_PRWM)
	_MAKE_DRM_ID(0x030000, VERIMATRIX_PR)
	_MAKE_DRM_ID(0x040000, DISCRETIX_PR)
	_MAKE_DRM_ID(0x050000, CABLEVISION_PR)
	_MAKE_DRM_ID(0x060000, COMCAST_PR)
	_MAKE_DRM_ID(0x070000, NEXTSCAPE_PR)
	_MAKE_DRM_ID(0x080000, MOTO_PR)
	_MAKE_DRM_ID(0x090000, MEDIACRYPTO)
	_MAKE_DRM_ID(0x0A0000, TIVO_AES)

	_MAKE_DRM_ID(0x300000, ADAPTER)
	_MAKE_DRM_ID(0x310000, COMMONAES)
	_MAKE_DRM_ID(0x320000, CONAX_PR)

	_MAKE_DRM_ID(0x610000, DIVX)
	_MAKE_DRM_ID(0x620000, DIVXJNI)

	_MAKE_DRM_ID(0x630000, OPENSSL)
    
    // define Plugin modules
    _MAKE_PLUGIN_ID(0x010000, WINDOWEDIE)
    _MAKE_PLUGIN_ID(0x020000, WINDOWLESSIE)
    _MAKE_PLUGIN_ID(0x030000, FFCHROME)
    _MAKE_PLUGIN_ID(0x040000, UI)
    
	// define utility modules
	_MAKE_UTILITY_ID(0x010000, ANALYTICS)

	// define custom modules
	_MAKE_CUSTOM_ID(0x010000, OSCENGINE)  //refer to the voOSCEngine, Amazon AIV Customized Engine
};


/* define the error ID */
#define VO_ERR_NONE						0x00000000
#define VO_ERR_FINISH					0x00000001
#define VO_ERR_RETRY					0x00000002
#define VO_ERR_DROPPEDFRAME				0x00000003
#define VO_ERR_BASE						0X80000000
#define VO_ERR_FAILED					0x80000001
#define VO_ERR_OUTOF_MEMORY				0x80000002
#define VO_ERR_NOT_IMPLEMENT			0x80000003
#define VO_ERR_INVALID_ARG				0x80000004
#define VO_ERR_INPUT_BUFFER_SMALL		0x80000005
#define VO_ERR_OUTPUT_BUFFER_SMALL		0x80000006
#define VO_ERR_WRONG_STATUS				0x80000007
#define VO_ERR_WRONG_PARAM_ID			0x80000008
#define VO_ERR_CODEC_UNSUPPORTED        0x80000009
	
#define VO_ERR_LICENSE_ERROR		   (VO_INDEX_MFW_VOLCS|VO_ERR_BASE)

/* xxx is the module ID
#define VO_ERR_FAILED					0x8xxx0001
#define VO_ERR_OUTOF_MEMORY				0x8xxx0002
#define VO_ERR_NOT_IMPLEMENT			0x8xxx0003
#define VO_ERR_INVALID_ARG				0x8xxx0004
#define VO_ERR_INPUT_BUFFER_SMALL		0x8xxx0005
#define VO_ERR_OUTPUT_BUFFER_SMALL		0x8xxx0006
#define VO_ERR_WRONG_STATUS				0x8xxx0007
#define VO_ERR_WRONG_PARAM_ID			0x8xxx0008
#define VO_ERR_LICENSE_ERROR			0x8xxx0009
// Module own error ID
#define VO_ERR_Module					0x8xxx0X00
*/
 
#define	VO_PID_COMMON_BASE				 0x40000000						/*!< The base param ID for common */
#define	VO_PID_COMMON_QUERYMEM			(VO_PID_COMMON_BASE | 0X0001)	/*!< Query the memory needed Reserved. */
#define	VO_PID_COMMON_INPUTTYPE			(VO_PID_COMMON_BASE | 0X0002)	/*!< Set or Get the input buffer type. VO_INPUT_TYPE */
#define	VO_PID_COMMON_HASRESOURCE		(VO_PID_COMMON_BASE | 0X0003)	/*!< Query it has resource to use. VO_U32 *, 1 have, 0 No */
#define	VO_PID_COMMON_HEADDATA			(VO_PID_COMMON_BASE | 0X0004)	/*!< The head data of decoder in track. VO_CODECBUFFER * */
#define	VO_PID_COMMON_FLUSH				(VO_PID_COMMON_BASE | 0X0005)	/*!< Flush the codec buffer.VO_U32 *, 1 Flush, 0 No * */
#define	VO_PID_COMMON_START				(VO_PID_COMMON_BASE | 0X0006)	/*!< Start. 0 */
#define	VO_PID_COMMON_PAUSE				(VO_PID_COMMON_BASE | 0X0007)	/*!< Pause  */
#define	VO_PID_COMMON_STOP				(VO_PID_COMMON_BASE | 0X0008)	/*!< Stop  * */
#define	VO_PID_COMMON_CoreFile			(VO_PID_COMMON_BASE | 0X0009)	/*!< Set OMX Core file name. char * */
#define	VO_PID_COMMON_CompName			(VO_PID_COMMON_BASE | 0X000A)	/*!< Set OMX component name. char * */
#define	VO_PID_COMMON_HeadInfo			(VO_PID_COMMON_BASE | 0X000B)	/*!< Get the head info desciption. VO_HEAD_INFO * */
#define	VO_PID_COMMON_LIBOP				(VO_PID_COMMON_BASE | 0X010C)	/*!< Set library operator pointer. VO_LIB_OPERATOR * */
#define	VO_PID_COMMON_THREAD			(VO_PID_COMMON_BASE | 0X0200)	/*!< Set thread create function pointer. VOThreadCreate */
#define	VO_PID_COMMON_CPUNUM			(VO_PID_COMMON_BASE | 0X0201)	/*!< Set the cpu number. int * */
#define	VO_PID_COMMON_CPUVERSION		(VO_PID_COMMON_BASE | 0X0202)	/*!< Set the cpu version. int * */
#define	VO_PID_COMMON_LOGFUNC			(VO_PID_COMMON_BASE | 0X0203)	/*!< Set the volog function callback. int * */
#define	VO_PID_COMMON_FRAME_BUF_EX		(VO_PID_COMMON_BASE | 0X0204)	/*!< Set the additional frame buffer num. VO_U32 * */
#define	VO_PID_COMMON_LIVE_STRAT_OPT    (VO_PID_COMMON_BASE | 0X0205)	/*!< Set the live stream start position option. VO_U32 * */
#define VO_PID_COMMON_FRAME_BUF_BACK    (VO_PID_COMMON_BASE | 0X0206) /*!< Set the frame buffer back to decoder. VO_VIDEO_BUFFER * */
#define	VO_PID_COMMON_WORKPATH			(VO_PID_COMMON_BASE | 0X0207)	/*!< Set the working path. VO_TCHAR*     * */
#define	VO_PID_COMMON_EOF_FLUSH			(VO_PID_COMMON_BASE | 0X0208)	/*!< Flush the codec buffer when EOF.VO_U32 *, 1 Flush, 0 No * */
#define	VO_PID_COMMON_MAX_BA_SIZE		(VO_PID_COMMON_BASE | 0X0209)	/*!< Set the MAX resolution for BA. VO_VIDEO_FORMAT * */

/*
// Module Param ID
#define VO_ID_Mdoule					0x0xxx1000
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __voIndex_H__
