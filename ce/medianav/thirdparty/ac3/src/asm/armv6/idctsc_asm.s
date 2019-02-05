;*********************************************************************
;* Copyright 2003-2009 by VisualOn Software, Inc.
;* All modifications are confidential and proprietary information
;* of VisualOn Software, Inc. ALL RIGHTS RESERVED.
;**********************************************************************

;***************************** Change History**************************
;* 
;*    DD/MMM/YYYY     Code Ver     Description             Author
;*    -----------     --------     -----------             ------
;*    08-12-2009        1.0        File imported from      Huaping Liu
;*                                             
;**********************************************************************  
   

         AREA     |.text|, CODE, READONLY
	 EXPORT   idctsc
	 EXPORT   idctsc2
	 IMPORT   z1mix
	 IMPORT   z2mix
	 IMPORT   bitrevary

;Structure, DEC_PARAM , Size 0x7c bytes, from ./hdr/vo_ac3_var.h
DEC_PARAM_ac3_aub                      EQU    0x8      ;  pointer to AC3_AB_DATA
DEC_PARAM_bswitch                      EQU    0x10     ;  Word16
;End of Structure DEC_PARAM

;Structure, AC3_AB_DATA , Size 0x650 bytes, from ./hdr/vo_ac3_var.h
AC3_AB_DATA_tcbuf                      EQU    0x648    ;  pointer to Word32
;End of Structure AC3_AB_DATA


;void idctsc(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
         ;r0 --- *p_decparam
	 ;r1 --- *p_buff 
idctsc   FUNCTION

         STMFD         r13!, {r4 - r12, r14}
         SUB           r13, r13, #0x20
	 LDR           r2, [r0, #DEC_PARAM_bswitch]      ;p_decparam->bswitch
         LDR           r3, [r0, #DEC_PARAM_ac3_aub]      ;AC3_AB_DATA *p_aublk
	 MOV           r4, r1                            ;p_buff->fftbuf
	 LDR           r5, [r3, #AC3_AB_DATA_tcbuf]      ;p_aublk->tcbuf
	 CMP           r2, #0
	 BEQ           Lable1
	 ;r4---p_buff->fftbuf,  r5---p_aublk->tcbuf
	 MOV           r6, #0xC0                         ;3*N/4
	 MOV           r7, #0x40                         ;N/4
	 MOV           r8, #0x80                         ;N/2
	 MOV           r9, #0xFE                         ;N-2
	 ADD           r10, r4, r6, LSL #2               ;fftiptr
	 ADD           r11, r4, r7, LSL #2               ;fftrptr2
	 ADD           r12, r4, r8, LSL #2               ;fftiptr2
         ADD           r6, r5, r9, LSL #2                ;tcrptr
	 LDR           r7, Table2                        ;mixptr

	 MOV           r0, #0x40                         ;count = N/4
	 ;r0---count, r4---fftrptr, r5---tciptr, r7---mixptr
	 ;r10---fftiptr, r11---fftrptr2, r12---fftiptr2, r6---tcrptr
LOOP1
         LDR           r1, [r7], #4                      ;cr = *mixptr++
	 LDR           r2, [r7], #4                      ;ci = *mixptr++
	 LDR           r3, [r6], #4                      ;ar = *trcptr++
	 LDR           r8, [r5], #4                      ;ai = *triptr++
         SMMUL         r9, r1, r3     
	 SMMUL         r14, r2, r8
         ADD           r9, r9, r9
         SUB           r14, r9, r14, LSL #1
         SMMUL         r9, r2, r3
         SMMUL         r3, r1, r8
         ADD           r9, r9, r9
         ADD           r9, r9, r3, LSL #1
         STR           r14, [r4], #4                     ;*fftrptr++
       
         LDR           r3, [r6], #-4                     ;ar = *tcrptr--
         LDR           r8, [r5], #-4                     ;ai = *tciptr--
	 STR           r9, [r10], #4                     ;*fftiptr++

         SMMUL         r9, r1, r3     
	 SMMUL         r14, r2, r8
         ADD           r9, r9, r9
         SUB           r14, r9, r14, LSL #1
         SMMUL         r9, r2, r3
         SMMUL         r3, r1, r8
         ADD           r9, r9, r9
         ADD           r9, r9, r3, LSL #1
         STR           r14, [r11], #4                    ;*fftrptr2++
       
         SUB           r6, r6, #16
	 ADD           r5, r5, #16
         SUBS          r0, r0, #1
	 STR           r9, [r12], #4                     ;*fftiptr2++
	 BGT           LOOP1
         B             idctsc_end
Lable1
         ;r4---p_buff->fftbuf,  r5---p_aublk->tcbuf
	 MOV           r6, #0x80                         ;N/2
         MOV           r7, #0xFF                         ;N-1
         ADD           r8, r4, r6, LSL #2                ;fftiptr
         ADD           r9, r5, r7, LSL #2                ;tcrptr
         LDR           r10, Table1                       ;mixptr = z1mix

	 MOV           r0, #0x40                         ;count = N/4

LOOP2
         LDR           r1, [r10], #4                     ;cr = *mixptr++
	 LDR           r2, [r10], #4                     ;ci = *mixptr++
	 LDR           r3, [r9], #-8                     ;ar = *tcrptr
	 LDR           r11,[r5], #8                      ;ai = *tciptr
         SMMUL         r12, r1, r3
	 SMMUL         r14, r2, r11
	 ADD           r12, r12, r12
	 SUB           r14, r12, r14, LSL #1
         SMMUL         r12, r1, r11
         SMMUL         r1, r2, r3
         STR           r14, [r4], #4                     ;*fftrptr++
	 ADD           r2, r1, r1
	 ADD           r3, r2, r12, LSL #1
         LDR           r1, [r10], #4                     ;cr = *mixptr++
	 STR           r3, [r8], #4                      ;*fftiptr++

	 LDR           r2, [r10], #4                     ;ci = *mixptr++
	 LDR           r3, [r9], #-8                     ;ar = *tcrptr
	 LDR           r11,[r5], #8                      ;ai = *tciptr
         SMMUL         r12, r1, r3
	 SMMUL         r14, r2, r11
	 ADD           r12, r12, r12
	 SUB           r14, r12, r14, LSL #1
         SMMUL         r12, r1, r11
         SMMUL         r1, r2, r3
         STR           r14, [r4], #4                     ;*fftrptr++
	 ADD           r2, r1, r1
	 ADD           r3, r2, r12, LSL #1
         SUBS          r0, r0, #1
	 STR           r3, [r8], #4                      ;*fftiptr++   	 

	 BGT           LOOP2
idctsc_end
         ADD           r13, r13, #0x20
	 LDMFD         r13!, {r4 - r12, r15}
         ENDFUNC


;void idctsc2(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
         ;r0 --- *p_decparam
	 ;r1 --- *p_buff

idctsc2  FUNCTION
    
         STMFD         r13!, {r4 - r12, r14}
	 SUB           r13, r13, #0x20
	 LDR           r2, [r0, #DEC_PARAM_bswitch]       ;p_decparam->bswitch
         LDR           r3, [r0, #DEC_PARAM_ac3_aub]       ;AC3_AB_DATA *p_aublk
	 MOV           r4, r1                             ;p_buff->fftbuf
	 LDR           r5, [r3, #AC3_AB_DATA_tcbuf]       ;p_aublk->tcbuf
	 ;r4---p_buff->fftbuf,  r5---p_aublk->tcbuf
         CMP           r2, #0
         BEQ           Lable2
	 MOV           r0, #0xC0                          ;3*N/4
	 MOV           r1, #0x40                          ;N/4
	 MOV           r2, #0x80                          ;N/2
	 ;ADD           r6, r4, r0, LSL #2                 ;fftiptr
	 ;ADD           r7, r4, r1, LSL #2                 ;fftrptr2
	 ;ADD           r8, r4, r2, LSL #2                 ;fftiptr2
	 ADD           r9, r5, r0, LSL #2                 ;tciptr
	 ADD           r10, r5, r1, LSL #2                ;tcrptr2
	 ADD           r11, r5, r2, LSL #2                ;tciptr2
	 LDR           r12, Table2                        ;mixptr = z2mix
	 LDR           r3,  Table3                        ;bitrevary[]
	 MOV           r0, #0x0                              ;count = 0

LOOP3
         LDR           r1, [r12], #4                      ;cr = *mixptr++
	 LDR           r2, [r12], #4                      ;ci = *mixptr++
	 ADD           r6, r9, r0, LSL #2                 ;&bitrevary[2 * count]
         LDRSH         r7, [r6]                           ;index
         ADD           r8, r4, r7, LSL #1
         LDR           r7, [r8]	                          ;ar = fftrptr[index]
	 LDR           r6, [r8, #0x300]                   ;ai = fftiptr[index]
	 STR           r8, [sp, #0x4]                     ;push fftrptr[index]
	 SMMUL         r8, r1, r7
	 SMMUL         r14, r2, r6
	 ADD           r8, r8, r8
	 SUB           r8, r8, r14, LSL #1
	 STR           r8, [r5], #4                       ;tcrptr++
	 SMMUL         r8, r1, r6
	 SMMUL         r14, r2, r7
	 ADD           r8, r8, r8
	 ADD           r8, r8, r14, LSL #1
	 LDR           r14, [sp, #0x4]                    ;pull fftrptr[index]
         
	 STR           r8, [r9], #4                       ;tciptr++

	 LDR           r7, [r14, #0x100]                  ;ar = fftrptr2[index]
	 LDR           r6, [r14, #0x200]                  ;ai = fftiptr2[index]
	 SMMUL         r8, r1, r7
	 SMMUL         r14, r2, r6
	 ADD           r8, r8, r8
	 SUB           r8, r8, r14, LSL #1
	 STR           r8, [r10], #4                      ;tcrptr2++
	 SMMUL         r8, r1, r6
	 SMMUL         r14, r2, r7
	 ADD           r8, r8, r8
	 ADD           r8, r8, r14, LSL #1
         ADD           r0, r0, #1
	 STR           r8, [r11], #4                      ;tciptr2++
	 CMP           r0, #0x40
    
         BLT           LOOP3
         B             idctsc2_end	 
Lable2
         MOV           r0, #0x80                           
	 ADD           r6, r4, r0, LSL #2                 ;fftiptr
	 ADD           r7, r5, r0, LSL #2                 ;tciptr 
	 LDR           r8, Table1                         ;mixptr
	 LDR           r9, Table3                         ;bitrevary
         MOV           r0, #0x0                          ;count = 0
LOOP4
	 LDR           r1, [r8], #4                       ;cr = *mixptr++
	 LDRSH         r3, [r9], #2                       ;index
	 LDR           r2, [r8], #4                       ;ci = *mixptr++
         ADD           r10, r4, r3, LSL #2
	 ADD           r11, r6, r3, LSL #2
	 LDR           r3, [r10]                          ;ar = fftrptr[index]
	 LDR           r10, [r11]                         ;ai = fftiptr[index]

	 SMMUL         r11, r1, r3
	 SMMUL         r12, r2, r10
	 ADD           r11, r11, r11
	 SUB           r11, r11, r12, LSL #1

	 SMMUL         r12, r1, r10
	 SMMUL         r1, r2, r3
	 ADD           r12, r12, r12
	 ADD           r12, r12, r1, LSL #1
	 STR           r11, [r5], #4                      ;*tcrptr++
	 ADD           r0, r0, #1
	 STR           r12, [r7], #4                      ;*tciptr++
	 CMP           r0, #0x80
	 BLT           LOOP4

idctsc2_end
         ADD           r13, r13, #0x20
	 LDMFD         r13!, {r4 - r12, r15}
	 ENDFUNC

Table1
         DCD           z1mix

Table2 
         DCD           z2mix

Table3
         DCD           bitrevary

	 END
         























