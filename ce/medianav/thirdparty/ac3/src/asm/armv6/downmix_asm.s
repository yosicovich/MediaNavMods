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
        

           AREA       |.text|, CODE, READONLY
	   EXPORT     downmix
	   IMPORT     chantab

;void downmix(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
           ; r0 --- d_pecparam, 
	   ; r1 --- p_buff

;Structure, DEC_PARAM , Size 0x7c bytes, from ./hdr/vo_ac3_var.h
DEC_PARAM_ac3_si                       EQU    0x4      ;  pointer to AC3_INFO_SI
DEC_PARAM_ac3_aub                      EQU    0x8      ;  pointer to AC3_AB_DATA
DEC_PARAM_channum                      EQU    0xe      ;  Word16
DEC_PARAM_outmod                       EQU    0x1c     ;  Word16
DEC_PARAM_outnchans                    EQU    0x20     ;  Word16
;End of Structure DEC_PARAM

;Structure, BUFF_PARAM , Size 0x48d4 bytes, from ./hdr/vo_ac3_var.h
BUFF_PARAM_dnmix_buf                   EQU    0xc00    ;  array[6] of array[256] of Word32
;End of Structure BUFF_PARAM

;Structure, AC3_INFO_SI , Size 0x68 bytes, from ./hdr/vo_ac3_var.h
AC3_INFO_SI_acmod                      EQU    0xc      ;  Word16
;End of Structure AC3_INFO_SI

;Structure, AC3_AB_DATA , Size 0x650 bytes, from ./hdr/vo_ac3_var.h
AC3_AB_DATA_appgainrng                 EQU    0x216    ;  array[6] of Word16
AC3_AB_DATA_dnmixtab                   EQU    0x3fc    ;  array[6] of array[6] of Word32
AC3_AB_DATA_tcbuf                      EQU    0x648    ;  pointer to Word32
AC3_AB_DATA_dnmixbufinu                EQU    0x3ee    ;  array[6] of Word16
;End of Structure AC3_AB_DATA

downmix    FUNCTION
        
           STMFD       r13!, {r4 - r12, r14}
	   SUB         r13, r13, #0x20
	   LDR         r2, [r0, #DEC_PARAM_ac3_aub]      ;AC3_AB_DATA *p_aublk
	   LDR         r3, [r0, #DEC_PARAM_ac3_si]       ;AC3_INFO_SI *ac3_si

           ; Do downmixing
           LDR         r8, =0x216 
           LDRSH       r4, [r3, #AC3_INFO_SI_acmod]      ;ac3_si->acmod
	   LDRSH       r5, [r0, #DEC_PARAM_channum]      ;p_decparam->channum
	   LDRSH       r6, [r0, #DEC_PARAM_outnchans]    ;p_decparam->outnchans
	   LDRSH       r10, [r0, #DEC_PARAM_outmod]      ;p_decparam->outmod
	   ADD         r14, r2, r8                       ;p_aublk->appgainrng


	   LDR         r8, Table                         ;chantab[]
           MOV         r7, #0x6
	   MUL         r4, r4, r7
	   MUL         r11, r10, r7
	   ADD         r7, r4, r5
	   ADD         r9, r8, r7, LSL #1                ;&chantab[ac3_si->acmod][p_decparam->channum];
           ADD         r7, r8, r11, LSL #1               ;&chantab[p_decparam->outmod][0]
	   ADD         r11, r14, r5, LSL #1              
           LDRSH       r12, [r9]                         ;inchan
	   LDRSH       r5, [r11]                         ;gainrng

	   ADD         r8, r1, #BUFF_PARAM_dnmix_buf     ;p_buff->dnmix_buf[]
	   ADD         r10, r2, #AC3_AB_DATA_dnmixtab    ;p_aublk->dnmixtab[][]

	   ;r7 --- chantabptr, r12 --- inchan, r6 --- p_decparam->outnchans
	   ;r5 --- gainrng, r8 --- p_buff->dnmix_buf[], r2 --- p_aublk, r10 --- p_aublk->dnmixtab[][]

	   MOV         r0, #0x0                          ;chan=0
LOOP
	   LDRSH       r1, [r7], #0x2                    ;outchan = *chantabptr
	   MOV         r11, #0x6
	   MUL         r3, r11, r1
           ADD         r3, r3, r12
	   ADD         r3, r10, r3, LSL #2               ;&p_aublk->dnmixtab[outchan][inchan]
           LDR         r11, [r3]                         ;p_aublk->dnmixtab[outchan][inchan]
           CMP         r5, #0x0
	   MOVEQ       r11, r11, LSL #1                  ;p_aublk->dnmixtab[outchan][inchan]<<1
	   SUBNE       r4, r5, #1
	   MOVNE       r11, r11, LSR r4                  ;p_aublk->dnmixtab[outchan][inchan] >> (gainrng - 1)
	   MOVNE       r11, r11

	   CMP         r11, #0
	   BEQ         Lable1

	   ;r8 --- p_buff->dnmix_buf[], r2 --- p_aublk, r0 --- chan
	   ;r7 --- chantabptr, r10 --- p_aublk->dnmixtab[][], r5 --- gainrng, 
	   ;r6 --- p_decparam->outchans, r12 --- inchan, r14 --- dnmixfac
	   STR         r2, [sp, #0x4]                    ;push p_aublk
  
           LDR         r9, =0x3ee
	   LDR         r1, [r2, #AC3_AB_DATA_tcbuf]      ;tcptr = p_aublk->tcbuf
	   ADD         r3, r8, r0, LSL #10               ;dnmixptr = p_buff->dnmix_buf[chan]
	   ADD         r4, r2, r9                        ;p_aublk->dnmixbufinu
	   MOV         r9, #0x100                        ;count = 256
	   ADD         r14, r4, r0, LSL #1               ;&p_aublk->dnmixbufinu[chan]

	   STR         r14, [sp, #0x8]                   ;push &p_aublk->dnmixbufinu[chan]
	   STR         r6,  [sp, #0xc]                   ;push &p_decparam->outnchans
	   STR         r10, [sp, #0x10]                  ;push p_aublk->dnmixtab[][]

	   LDRSH       r2, [r14]                         ;p_aublk->dnmixbufinu[chan]
	   CMP         r2, #0
	   BNE         LOOP2

	   ;r1 --- tcptr, r3 --- dnmixptr, r11 --- dnmixfac, 
LOOP1
           LDR         r2, [r1], #4
	   LDR         r4, [r1], #4
	   LDR         r6, [r1], #4
	   LDR         r10, [r1], #4
	   SMMUL       r2, r2, r11
	   SMMUL       r4, r4, r11
	   SMMUL       r6, r6, r11
	   SMMUL       r10, r10, r11
	   ADD         r2, r2, r2
	   ADD         r4, r4, r4
	   ADD         r6, r6, r6
	   ADD         r10, r10, r10
	   STR         r2, [r3], #4
	   STR         r4, [r3], #4
	   STR         r6, [r3], #4
	   STR         r10, [r3], #4
	   SUBS        r9, r9, #0x4
	   BGT         LOOP1
	   B           Lable

LOOP2
           LDR         r2, [r1], #4
	   LDR         r4, [r1], #4
	   LDR         r14, [r3]
	   SMMUL       r2, r2, r11
	   SMMUL       r4, r4, r11
	   LDR         r6, [r1], #4
	   LDR         r10, [r1], #4
	   ADD         r14, r14, r2, LSL #1
	   SMMUL       r6, r6, r11
	   SMMUL       r10, r10, r11
	   STR         r14, [r3], #4
	   LDR         r2, [r3]
	   ADD         r2, r2, r4, LSL #1
	   STR         r2, [r3], #4
	   LDR         r2, [r3]
	   ADD         r2, r2, r6, LSL #1
	   STR         r2, [r3], #4
	   LDR         r2, [r3]
	   ADD         r2, r2, r10, LSL #1
	   STR         r2, [r3], #4
	   SUBS        r9, r9, #0x4
	   BGT         LOOP2

Lable
	   LDR         r2, [sp, #0x4]                    ;pull p_aublk
	   LDR         r14, [sp, #0x8]                   ;pull &p_aublk->dnmixbufinu[chan]
	   LDR         r6,  [sp, #0xc]                   ;pull &p_decparam->outnchans
	   LDR         r10, [sp, #0x10]                  ;pull p_aublk->dnmixtab[][] 
           MOV         r4, #0x1
           STRH        r4, [r14]                         ;p_aublk->dnmixbufinu[chan] = 1;

Lable1
	   
           ADD         r0, r0, #0x1
	   CMP         r0, r6
	   BLT         LOOP

downmix_end
           ADD         r13, r13, #0x20
	   LDMFD       r13!, {r4 - r12, r15}

	   ENDFUNC

Table
           DCD         chantab

	   END


