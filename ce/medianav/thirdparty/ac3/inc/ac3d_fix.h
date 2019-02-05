/********************************************************************
* Copyright 2008 by VisualOn Software, Inc.
* All modifications are confidential and proprietary information
* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
*********************************************************************
* File Name: 
*            ac3d_fix.h
*
* Project:
* contents/description:vo fixed version porting, the file will define 
*                      some floating to fixed operate function.
*            
***************************** Change History**************************
* 
*    DD/MMM/YYYY     Code Ver     Description             Author
*    -----------     --------     -----------             ------
*    02-05-2009        1.0        File imported from      Huaping Liu
*                                             
**********************************************************************/
#ifndef  __AC3D_FIX_H__
#define  __AC3D_FIX_H__

/* data type definition */
#ifdef   LINUX
#define      DSPint64      long long
#elif IOS
#define      DSPint64      long long  
#else
#define      DSPint64      __int64
#endif

/* global fix definition Q format */
#define      AC3_Q31_FORMAT         31
#define      AC3_Q15_FORMAT         15
#define      AC3_Q27_FORMAT         27

#define      AC3_Q31_RND            (unsigned int)(1<<(AC3_Q31_FORMAT - 1))
#define      AC3_Q15_RND            (unsigned int)(1<<(AC3_Q15_FORMAT - 1))
#define      AC3_Q27_RND            (unsigned int)(1<<(AC3_Q27_FORMAT - 1))

/* below part used to define unsigned Q31 format */
#define  SQUARE2DIV2             0X2d413cccL
#define  Q31MAX                  0x3fffffffL
#define  Q31MIN                  (int)0xc0000000L
#define  MAX_32                  (int)0x7fffffffL
#define  MIN_32                  (int)0x80000000L

#ifdef LINUX
#define  __voinline  static __inline__
#else
#define  __voinline  static __inline
#endif 

__voinline int Vo_DSPlimit(int a)                  /* DSP fixed fractional limiting */
{
	if(a > (int)0x7fffffffL)
		a = (int)0x7fffffffL;
	else if(a <=(int)0x80000000L)
	    a = (int)0x80000000L;
    return (a);
}

__voinline int  Vo_Multi32_Q(int var1, int var2)
{
	DSPint64  acc;
	acc = (DSPint64)var1 * (DSPint64)var2;
	acc += AC3_Q31_RND;
	acc >>= AC3_Q31_FORMAT;
	return (int)acc;
}

__voinline int  Vo_Multi28_Q(int var1, int var2)
{
	DSPint64  acc;
	acc = (DSPint64)var1 * (DSPint64)var2;
	acc += AC3_Q27_RND;
	acc >>= AC3_Q27_FORMAT;
	return (int)acc;
}

__voinline int Vo_Multi1632(short var1, int var2)
{
	DSPint64  acc;
	acc = (DSPint64)var1 * (DSPint64)var2;
	acc += AC3_Q15_RND;
	acc >>= 15;
	return (int)acc;
}

__voinline int Vo_Multi1632_Scale(short var1, int var2)
{
	DSPint64  acc;
	acc = (DSPint64)var1 * (DSPint64)var2;
	//acc += AC3_Q15_RND;
	acc >>= 15;
	return (int)acc;
}

__voinline int  Vo_Multi32(int var1, int var2)
{
#ifdef  ARMGCC_OPT
        register  int  ra = var1;
	register  int  rb = var2
        register  int  result;
        asm volatile(
			"SMMUL  %[result], %[ra], %[rb] \n"
			"ADD    %[result], %[result], %[result] \n"
                        :[result] "+r" (result):[ra] "r" (ra), [rb] "rb" (rb)
		    );
        return  result;	
#else
	DSPint64  acc;
	acc =  (DSPint64)var1 * (DSPint64)var2;
	//acc += AC3_Q31_RND;          //have 1bit error
	//acc >>= AC3_Q31_FORMAT;
	acc >>= (AC3_Q31_FORMAT + 1);
	acc <<=1;
	return  (int)acc;
#endif
}

__voinline int  Vo_Multi32_28(int var1, int var2)
{
#ifdef  ARMGCC_OPT
        register  int  ra = var1;
	register  int  rb = var2
        register  int  result;
        asm volatile(
			"SMMUL  %[result], %[ra], %[rb] \n"
			"ADD    %[result], %[result], %[result] \n"
                        :[result] "+r" (result):[ra] "r" (ra), [rb] "rb" (rb)
		    );
        return  result;	
#else
	DSPint64  acc;
	acc =  (DSPint64)var1 * (DSPint64)var2;
	acc >>= (AC3_Q31_FORMAT + 1);
	acc <<= 1;
	return  (int)acc;
#endif
}

//32DIV2
__voinline int  Vo_Multi32DIV2(int var1, int var2)
{
	DSPint64  acc;
	acc =  (DSPint64)var1 * (DSPint64)var2;
	acc += AC3_Q31_RND;
	acc >>= (AC3_Q31_FORMAT + 4);
	acc <<=1;
	return  (int)acc;
}

//Q30 * Q31 = Q31
__voinline int  Vo_Multi32_31(int var1, int var2)
{
#ifdef  ARMGCC_OPT
        register  int  ra = var1;
	register  int  rb = var2
        register  int  result;
        asm volatile(
			"MOV    r2, #0x3 \n"
			"SMMUL  %[result], %[ra], %[rb] \n"
			"MOV    %[result], %[result], LSL r2 \n"
                        :[result] "+r" (result):[ra] "r" (ra), [rb] "rb" (rb) : "r2"
		    );
        return  result;	
#else
	DSPint64  acc;
	acc =  (DSPint64)var1 * (DSPint64)var2;
	acc >>= (AC3_Q31_FORMAT + 1);
	acc <<= 3;
	return  (int)acc;
#endif
}

__voinline short Vo_MultiShortQ31(short var1, int var2)
{
#ifdef  ARMGCC_OPT
        register  int  ra = var1;
	register  int  rb = var2
        register  int  result;
        asm volatile(
			"SMMUL  %[result], %[ra], %[rb] \n"
			"ADD    %[result], %[result], %[result] \n"
                        :[result] "+r" (result):[ra] "r" (ra), [rb] "rb" (rb)
		    );
        return  (short)result;	
#else
	DSPint64  acc;
	acc = (DSPint64)var1 * (DSPint64)var2;
	//acc += AC3_Q31_RND;
	acc >>= (AC3_Q31_FORMAT + 1);
	acc <<= 1;
	return (short)acc;
#endif
}


#endif //__AC3D_FIX_H__


