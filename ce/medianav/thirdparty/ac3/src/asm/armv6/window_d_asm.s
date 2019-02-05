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
    

       AREA   |.text|, CODE, READONLY
       EXPORT window_d
       IMPORT chantab
       IMPORT Lwindow_Interleave

;Structure, DECEXEC , Size 0x5c bytes, from ./hdr/vo_ac3_var.h
DECEXEC_ac3_buff                       EQU    0x8      ;  pointer to BUFF_PARAM
;End of Structure DECEXEC

;Structure, BUFF_PARAM , Size 0x48d4 bytes, from ./hdr/vo_ac3_var.h
BUFF_PARAM_dnmix_buf                   EQU    0xc00    ;  array[6] of array[256] of Word32
BUFF_PARAM_delay_buf                   EQU    0x2400   ;  array[6] of array[128] of Word32
; End of Structure BUFF_PARAM

;Structure, DEC_PARAM , Size 0x7c bytes, from ./hdr/vo_ac3_var.h
DEC_PARAM_outmod                       EQU    0x1c     ;  Word16
DEC_PARAM_pcmbufptr                    EQU    0x64     ;  array[6] of pointer to Word32
;End of Structure DEC_PARAM
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;void window_d(DSPshort chan, DECEXEC *p_decexec)
;
; r0 --- chan
; r1 --- p_decexec, p_decparam

window_d  FUNCTION
            
           STMFD           r13!, {r4 - r12, r14}
	   SUB             r13, r13, #0x20

	   MOV             r7, r0                               ;chan
	   STR             r0, [sp, #0x4]                      ;push chan
	   LDR             r6, [r1, #DECEXEC_ac3_buff]          ;p_buff
	   LDR             r3, [r1]                             ;p_decparam
	   ;r6 ----pcmptr1 = p_buff->fftbuf, p_buff
	   MOV             r5, #0xFF
	   ADD             r4, r6, r5, LSL #2                   ;pcmptr2 = p_buff->fftbuf + N -1

	   STR             r3,  [sp, #0x8]                      ;push p_decparam
           STR             r6,  [sp, #0xc]                      ;push p_buff	   
	   ADD             r8, r6, #BUFF_PARAM_dnmix_buf        ;p_buff->dnmix_buf[] address/*	Copy PCM samples to output buffer */
	   ADD             r9, r8, r7, LSL #10                   ;p_buff->dnmix_buf[chan] address

	   ADD             r10, r6, #BUFF_PARAM_delay_buf       ;p_buff->delay_buf[] address
	   ADD             r11, r10, r7, LSL #9                 ;p_buff->delay_buf[chan] address

	   STR             r9,  [sp, #0x10]                     ;push p_buff->dnmix_buf[chan]
	   STR             r11, [sp, #0x14]                      ;push p_buff->delay_buf[chan]

	   ;r6--- pcmptr1, r4 --- pcmptr2, r9 --- p_buff->dnmix_buf[chan], r11 --- p_buff->delay_buf[chan]

           LDR             r8, Table1                          ;winptr1
	   MOV             r0, #0xc0
	   ADD             r10, r9, r0, LSL #2                   ;p_buff->dnmix_buf[chan] + 3*N/4
           MOV             r0, #0x3F               
	   ADD             r9, r9, r0, LSL #2                    ;p_buff->dnmix_buf[chan] + N/4-1
	   MOV             r0, #0x7F
	   ADD             r2, r11, r0, LSL #2                   ;p_buff->delay_buf[chan] + N/2-1

	   ;r6, r4, r8, r2, r11, r9, r10

	   MOV             r0, #64

Windows_Loop

           ;samp = (- Vo_Multi32((*windptr2), (*delayptr1)) - Vo_Multi32((*windptr1), (*dnmixptr1)))<<1;
           LDR             r1, [r8], #4                          ;*winptr1 
	   LDR             r3, [r8], #4                          ;*winptr2
	   LDR             r5, [r10], #4                         ;*dnmixptr1
	   LDR             r7, [r11], #4                         ;*delayptr1
	   SMMUL           r12, r1, r5                           ;Vo_Multi32((*windptr1), (*dnmixptr1))
           SMMUL           r14, r3, r7                           ;Vo_Multi32((*windptr2), (*delayptr1))
           ADD             r12, r12, r12
	   ADD             r12, r12, r14, LSL #1               
	   SMMUL           r1, r1, r7                            ;Vo_Multi32((*windptr1), (*delayptr1++))
	   SMMUL           r3, r3, r5                            ;Vo_Multi32((*windptr2), (*dnmixptr1++))
	   MOV             r14, #0
	   ADD             r3, r3, r3
	   RSB             r12, r12, r14
	   SUB             r14, r3, r1, LSL #1
	   ADD             r12, r12, r12
	   ADD             r14, r14, r14
	   STR             r12, [r6], #4                         ;*pcmptr1++ = samp
	   STR             r14, [r4], #-4                        ;*pcmptr2-- = samp

           LDR             r1, [r8], #4                          ;*winptr1
           LDR             r3, [r8], #4                          ;*winptr2
           LDR             r5, [r9], #-4                          ;*dnmixptr2
           LDR             r7, [r2], #-4                          ;*delayptr2

           SMMUL           r12, r1, r5                           ;Vo_Multi32((*windptr1), (*dnmixptr2))
           SMMUL           r14, r3, r7                           ;Vo_Multi32((*windptr2), (*delayptr2))	   
           	   
           ADD             r12, r12, r12
	   ADD             r12, r12, r14, LSL #1
	   SMMUL           r1, r1, r7                            ;Vo_Multi32((*windptr1), (*delayptr2--))
	   SMMUL           r3, r3, r5                            ;Vo_Multi32((*windptr2), (*dnmixptr2--))
	   ADD             r12, r12, r12
	   ADD             r1, r1, r1
	   ADD             r3, r3, r3
	   STR             r12, [r6], #4                         ;*pcmptr1++ = samp
	   SUB             r14, r1, r3
	   SUBS            r0, r0, #1                            ;count--
	   ADD             r14, r14, r14
	   STR             r14, [r4], #-4                        ;*pcmptr2-- = samp

	   BGT             Windows_Loop

	   ;Update delay buffers
  
	   LDR             r9, [sp, #0x10]                        ;pull p_buff->dnmix_buf[chan]  
	   MOV             r1, #0x40	
           MOV             r0, #32                               ;N/8
	   ADD             r9, r9, r1, LSL #2                    ;p_buff->dnmix_buf[chan] + (N/4)
           LDR             r11,[sp, #0x14]                        ;pull p_buff->delay_buf[chan]
Up_Loop
           LDR             r3,  [r9], #4
	   LDR             r5,  [r9], #4
	   LDR             r7,  [r9], #4
	   LDR             r12, [r9], #4
	   STR             r3,  [r11], #4
	   STR             r5,  [r11], #4
	   STR             r7,  [r11], #4
	   STR             r12, [r11], #4
	   SUBS            r0, r0, #1
	   BGT             Up_Loop

	   ;Copy PCM samples to output buffer
	   MOV             r0, #0x6
	   LDR             r3, [sp, #0x8]                        ;pull p_decparam
	   LDR             r5, [sp, #0x4]                       ;pull chan
	   LDR             r9, [sp, #0xc]                        ;p_buff->fftbuf
           LDR             r7, Table2                            ;load chantab
           LDRSH           r12, [r3, #DEC_PARAM_outmod]          ;p_decparam->outmod
	   ADD             r14, r3, #DEC_PARAM_pcmbufptr         ;p_decparam->pcmbufptr[]

	   MUL             r12, r12, r0
	   ADD             r5, r5, r12      
	   ADD             r11, r7, r5, LSL #1                   ;chantab[p_decparam->outmod][chan];
	   MOV             r0, #64
	   LDRSH           r11, [r11]                            ;outchan
	   MOV             r10, r11, LSL #2
	   LDR             r11, [r14, r10]                         ;p_decpara->pcmbufptr[outchan]

Copy_Loop
           LDR             r2, [r9], #4
	   LDR             r3, [r9], #4
	   LDR             r4, [r9], #4
	   LDR             r5, [r9], #4
	   STR             r2, [r11], #0x18
           STR             r3, [r11], #0x18
	   STR             r4, [r11], #0x18
	   STR             r5, [r11], #0x18
	   SUBS            r0, r0, #1
	   BGT             Copy_Loop

window_d_end

           ADD             r13, r13, #0x20         
           LDMFD           r13!, {r4 - r12, r15}

Table1
           DCD             Lwindow_Interleave

Table2
           DCD             chantab

	   ENDFUNC
	   END 
  










