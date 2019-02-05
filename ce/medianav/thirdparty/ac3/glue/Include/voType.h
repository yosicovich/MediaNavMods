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
#ifndef __voType_H__
#define __voType_H__

#ifdef _MAC_OS
#include <stdlib.h>
#endif

#ifdef _WIN32
#include "tchar.h"
#endif // _WIN32

#include "voYYDef_Type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _WIN32
#	define VO_API __cdecl
#	define VO_CBI __stdcall
#else
#	define VO_API
#	define VO_CBI
#endif //_WIN32

/** VO_IN is used to identify inputs to an VO function.  This designation
    will also be used in the case of a pointer that points to a parameter
    that is used as an output. */
#ifndef VO_IN
#define VO_IN
#endif

/** VO_OUT is used to identify outputs from an VO function.  This
    designation will also be used in the case of a pointer that points
    to a parameter that is used as an input. */
#ifndef VO_OUT
#define VO_OUT
#endif

/** VO_INOUT is used to identify parameters that may be either inputs or
    outputs from an VO function at the same time.  This designation will
    also be used in the case of a pointer that  points to a parameter that
    is used both as an input and an output. */
#ifndef VO_INOUT
#define VO_INOUT
#endif

#ifdef __SYMBIAN32__
#define	VO_EXPORT_FUNC	EXPORT_C
#elif defined LINUX || defined _LINUX || defined _IOS
#if defined EXPORT
#undef EXPORT
#endif
#define EXPORT __attribute__((visibility("default")))
#define	VO_EXPORT_FUNC
#else
#define	VO_EXPORT_FUNC
#define	EXPORT
#endif // __SYMBIAN32__

#define VO_MAX_ENUM_VALUE	0X7FFFFFFF

/** VO_VOID */
typedef void VO_VOID;

/** VO_U8 is an 8 bit unsigned quantity that is byte aligned */
typedef unsigned char VO_U8;

/** VO_BYTE is an 8 bit unsigned quantity that is byte aligned */
typedef unsigned char VO_BYTE;

/** VO_S8 is an 8 bit signed quantity that is byte aligned */
typedef signed char VO_S8;

/** VO_CHAR is an 8 bit signed quantity that is byte aligned */
typedef char VO_CHAR;

/** VO_U16 is a 16 bit unsigned quantity that is 16 bit word aligned */
typedef unsigned short VO_U16;

/** VO_WCHAR is a 16 bit unsigned quantity that is 16 bit word aligned */
#if defined _WIN32
typedef unsigned short VO_WCHAR;
typedef unsigned short* VO_PWCHAR;
#elif defined LINUX
typedef unsigned char VO_WCHAR;
typedef unsigned char* VO_PWCHAR;
#endif

#ifdef _WIN32
#define VO_TCHAR		TCHAR
#define VO_PTCHAR		TCHAR*
#else
#define VO_TCHAR		char
#define VO_PTCHAR		char*
#endif // _WIN32

/** VO_S16 is a 16 bit signed quantity that is 16 bit word aligned */
typedef signed short VO_S16;

/** VO_U32 is a 32 bit unsigned quantity that is 32 bit word aligned */
#ifdef _MAC_OS
	typedef u_int32_t VO_U32;
#else
	typedef unsigned long VO_U32;	
#endif // end _MAC_OS
	
/** VO_S32 is a 32 bit signed quantity that is 32 bit word aligned */
#ifdef _MAC_OS
	typedef int32_t VO_S32;
#else
	typedef signed long VO_S32;
#endif // end _MAC_OS

/* Users with compilers that cannot accept the "long long" designation should
   define the VO_SKIP64BIT macro.  It should be noted that this may cause
   some components to fail to compile if the component was written to require
   64 bit integral types.  However, these components would NOT compile anyway
   since the compiler does not support the way the component was written.
*/
#ifndef VO_SKIP64BIT
#ifdef _WIN32
/** VO_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned __int64  VO_U64;
/** VO_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed   __int64  VO_S64;
#else // WIN32
/** VO_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long VO_U64;
/** VO_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long VO_S64;
#endif // WIN32
#endif // VO_SKIP64BIT

/** The VO_BOOL type is intended to be used to represent a true or a false
    value when passing parameters to and from the VO core and components.  The
    VO_BOOL is a 32 bit quantity and is aligned on a 32 bit word boundary.
 */
typedef enum VO_BOOL {
    VO_FALSE = 0,
    VO_TRUE = !VO_FALSE,
	VO_BOOL_MAX = VO_MAX_ENUM_VALUE
} VO_BOOL;

/** The VO_PTR type is intended to be used to pass pointers between the VO
    applications and the VO Core and components.  This is a 32 bit pointer and
    is aligned on a 32 bit boundary.
 */
typedef void* VO_PTR;
typedef const void* VO_CPTR;

/** The VO_HANDLE type is intended to be used to pass pointers between the VO
    applications and the VO Core and components.  This is a 32 bit pointer and
    is aligned on a 32 bit boundary.
 */
typedef void* VO_HANDLE;

/** The VO_STRING type is intended to be used to pass "C" type strings between
    the application and the core and component.  The VO_STRING type is a 32
    bit pointer to a zero terminated string.  The  pointer is word aligned and
    the string is byte aligned.
 */
typedef char* VO_PCHAR;

/** The VO_PBYTE type is intended to be used to pass arrays of bytes such as
    buffers between the application and the component and core.  The VO_PBYTE
    type is a 32 bit pointer to a zero terminated string.  The  pointer is word
    aligned and the string is byte aligned.
 */
typedef unsigned char* VO_PBYTE;

/** The VO_PTCHAR type is intended to be used to pass arrays of wchar such as
    unicode char between the application and the component and core.  The VO_PTCHAR
    type is a 32 bit pointer to a zero terminated string.  The  pointer is word
    aligned and the string is byte aligned.
 */

#ifdef _UNICODE
typedef unsigned short* VO_PTTCHAR;
typedef unsigned short VO_TTCHAR;
#else
typedef char* VO_PTTCHAR;
typedef char VO_TTCHAR;
#endif


#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/**
 * Input stream format, Frame or Stream..
 */
typedef enum {
    VO_INPUT_FRAME	= 1,	/*!< Input contains completely frame(s) data. */
    VO_INPUT_STREAM,		/*!< Input is stream data. */
	VO_INPUT_STREAM_MAX = VO_MAX_ENUM_VALUE
} VO_INPUT_TYPE;

/**
 * General data buffer, used as input or output.
 */
typedef struct {
	VO_S32		nFlag;		/*!< add flag    */
	VO_PBYTE	Buffer;		/*!< Buffer pointer */
	VO_U32		Length;		/*!< Buffer size in byte */
	VO_S64		Time;		/*!< The time of the buffer */
	VO_U32		nReserved;  /*!< The reserved data for later use.*/
	VO_PTR		pReserved;  /*!< The reserved data for later use.*/
} VO_USERBUFFER;

/**
 * General data buffer, used as input or output.
 */
typedef struct {
	VO_PBYTE	Buffer;		/*!< Buffer pointer */
	VO_U32		Length;		/*!< Buffer size in byte */
	VO_S64		Time;		/*!< The time of the buffer */
	VO_PTR		UserData;   /*!< The user data for later use.*/
} VO_CODECBUFFER;


/**
 * GUID structure...
 */
typedef struct _VO_GUID {
	VO_U32	Data1;		/*!< Data1 value */
	VO_U16	Data2;		/*!< Data2 value */
	VO_U16	Data3;		/*!< Data3 value */
	VO_U8	Data4[8];	/*!< Data4 value */
} VO_GUID;


/**
 * Head Info description
 */
typedef struct _VO_HEAD_INFO {
	VO_PBYTE	Buffer;				/*!< [In] Head Buffer pointer */
	VO_U32		Length;				/*!< [In] Head Buffer size in byte */
	VO_PCHAR	Description;		/*!< [In/Out] Allocated by Caller. The char buffer of description  */
	VO_U32		Size;				/*!< [In] The size of description  */
} VO_HEAD_INFO;


/**
 * The init memdata flag.
 */
typedef enum{
	VO_IMF_USERMEMOPERATOR		=0,	/*!< memData is  the pointer of memoperator function*/
	VO_IMF_PREALLOCATEDBUFFER	=1,	/*!< memData is  preallocated memory*/
	VO_IMF_MAX = VO_MAX_ENUM_VALUE
}VO_INIT_MEM_FlAG;


typedef struct
{
	VO_PTR	pUserData;
	VO_PTR	(VO_API * LoadLib) (VO_PTR pUserData, VO_PCHAR pLibName, VO_S32 nFlag);
	VO_PTR	(VO_API * GetAddress) (VO_PTR pUserData, VO_PTR hLib, VO_PCHAR pFuncName, VO_S32 nFlag);
	VO_S32	(VO_API * FreeLib) (VO_PTR pUserData, VO_PTR hLib, VO_S32 nFlag);
} VO_LIB_OPERATOR;

/**
 * The init memory structure..
 */
typedef struct{
	VO_U32						memflag;		/*!<memory and other param flag  0X000X for memData type, 0X0010 for lib op, 0X0100 for Thumbnail or Video */
	VO_PTR						memData;		/*!<a pointer to VO_MEM_OPERATOR or a preallocated buffer  */
	VO_LIB_OPERATOR *			libOperator;	/*!<Library operator function pointer. If memflag is 0X1x, the param is available  */
	VO_U32						reserved1;		/*!<reserved  */
	VO_U32						reserved2;		/*!<reserved */
}VO_CODEC_INIT_USERDATA;

/**
 * Thread create function
 * \param pHandle [in/out] the handle of thread
 * \param nID [in] the id of thread
 * \param fProc [in] the function entry pointer
 * \param pParam [in] the parameter in call function.
 */
typedef int (* VOThreadCreate) (void ** pHandle, int * pID, void * fProc, void * pParam);

/**
 * Input stream format, Frame or Stream..
 */
typedef enum {
    VO_CPU_ARMV4		= 1,	/*!< The CPU is ARMV4. */
    VO_CPU_ARMV5,				/*!< The CPU is ARMV5. */
    VO_CPU_ARMV5x,				/*!< The CPU is ARMV5+. */
    VO_CPU_ARMV6,				/*!< The CPU is ARMV6. */
    VO_CPU_ARMV7,				/*!< The CPU is ARMV7. */
    VO_CPU_ARM_NEON,			/*!< The CPU is ARM neon. */
    VO_CPU_WMMX			=100,	/*!< The CPU is WMMX. */
	VO_CPU_MAX = VO_MAX_ENUM_VALUE
} VO_CPU_VERSION;


typedef int (* VOLOG_PRINT) (void * pUserData, int nLevel, VO_TCHAR * pLogText);

/**
 * The log print call back function
 */
typedef struct{
	void *					pUserData;		/*! the user data for call back*/
	VOLOG_PRINT				fCallBack;		/*! call back function pointer */
}VO_LOG_PRINT_CB;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __voType_H__
