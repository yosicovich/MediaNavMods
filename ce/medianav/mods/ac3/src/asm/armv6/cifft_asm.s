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
	   EXPORT   cifft
	   IMPORT   brxmix


;Structure, DEC_PARAM , Size 0x7c bytes, from ./hdr/vo_ac3_var.h
DEC_PARAM_bswitch                      EQU    0x10     ;  Word16
;End of Structure DEC_PARAM

;void cifft(DEC_PARAM  *p_decparam, BUFF_PARAM *p_buff)
         ;r0 --- *p_decparam
	 ;r1 --- *p_buff

cifft    FUNCTION

         STMFD         r13!, {r4 - r12, r14}
	 SUB           r13, r13, #0x20
	 LDR           r2, [r0, #DEC_PARAM_bswitch]      ;p_decparam->bswitch
	 MOV           r4, r1                            ;p_buff->fftbuf
	 CMP           r2, #0
	 BEQ           Lable1
	 ; r4 --- p_buff->fftbuf
	 MOV           r0, #0x40                         ;fftn = N/4
	 MOV           r1, #0x3                          ;fftnlg2m3 = FFTNLG2M3 - 1
	 B             Lable2
         	   
Lable1
	 ADD           r3, r4, #0x200                    ;fftiptr = p_buff->fftbuf + N/2
	 STR           r4, [sp, #0x8]                    ;push fftrptr      
	 STR           r3, [sp, #0x4]                    ;push fftiptr
 
	 ;r0 --- fftn, r1 --- fftnlg2m3, r2 --- nstep, r3 --- fftiptr, r4 --- fftrptr
;for(m = nstep; m > 0; m--) -- the branch one time
         ; Do first Radix-4 Pass
;for(i = fftn/4, i > 0; i--)
         MOV           r6, #0x20                         ;i = fftn/4
LOOP1	 
         LDR           r2, [r4]                          ;ar = *bfyrptr1
	 LDR           r5, [r3]                          ;ai = *bfyiptr1
	 LDR           r7, [r4, #0x80]                   ;br = *bfyrptr2
	 LDR           r8, [r3, #0x80]                   ;bi = *bfyiptr2
	 LDR           r9, [r4, #0x100]                  ;cr = *bfyrptr3
	 LDR           r10, [r3, #0x100]                 ;ci = *bfyiptr3
	 LDR           r11, [r4, #0x180]                 ;dr = *bfyrptr4
	 LDR           r12, [r3, #0x180]                 ;di = *bfyiptr4

	 ADD           r0, r2, r9                        ;arcr = ar + cr
	 ADD           r1, r7, r11                       ;brdr = br + dr
	 ADD           r14, r0, r1
	 SUB           r0, r0, r1

	 STR           r14, [r4]
	 STR           r0, [r4, #0x80]
	 ADD           r0, r5, r10                       ;aici = ai + ci
	 ADD           r1, r8, r12                       ;bidi = bi + di
	 ADD           r14, r0, r1
	 SUB           r0, r0, r1
	 STR           r14, [r3]
	 STR           r0, [r3, #0x80]

	 SUB           r0, r5, r10                       ;aici = ai - ci
	 SUB           r1, r7, r11                       ;brdr = br - dr
         ADD           r14, r0, r1
	 SUB           r0, r0, r1
	 STR           r14, [r3, #0x100]
	 STR           r0, [r3, #0x180]

         SUB           r0, r2, r9                        ;arcr = ar - cr
	 SUB           r1, r8, r12                       ;bidi = bi - di
	 SUB           r14, r0, r1
	 ADD           r0, r0, r1
	 STR           r14, [r4, #0x100]
	 STR           r0, [r4, #0x180]

	 ADD           r3, r3, #4                        ;*bfyiptr++
	 ADD           r4, r4, #4                        ;*byfrptr++
	 SUBS          r6, r6, #1
	 BGT           LOOP1

	 ;Do all Radix-2 passes except first two and last      
         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x4                          ;j = gp	 
LOOP3
         MOV           r8, #0x10                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

LOOP4
	 LDR           r11, [r4, #0x40]                  ;br
	 LDR           r12, [r3, #0x40]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x40]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x40]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           LOOP4
	 ADD           r3, r3, #0x40                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x40                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           LOOP3

         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x8                          ;j = gp	 
LOOP31
         MOV           r8, #0x8                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

LOOP41
	 LDR           r11, [r4, #0x20]                  ;br
	 LDR           r12, [r3, #0x20]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x20]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x20]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           LOOP41
	 ADD           r3, r3, #0x20                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x20                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           LOOP31

         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x10                          ;j = gp	 
LOOP32
         MOV           r8, #0x4                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

LOOP42
	 LDR           r11, [r4, #0x10]                  ;br
	 LDR           r12, [r3, #0x10]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x10]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x10]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           LOOP42
	 ADD           r3, r3, #0x10                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x10                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           LOOP32
	 
         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x20                          ;j = gp	 
LOOP33
         MOV           r8, #0x2                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

LOOP43
	 LDR           r11, [r4, #0x8]                  ;br
	 LDR           r12, [r3, #0x8]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x8]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x8]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           LOOP43
	 ADD           r3, r3, #0x8                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x8                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           LOOP33 

;Do last Radix-2 pass
         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
         MOV           r0, #0x40                         ;i = fftn/2

LOOP5
         LDR           r6, [r4], #4                      ;ar = *bfyrptr1++
	 LDR           r7, [r3], #4                      ;ai = *bfyiptr1++
	 LDR           r8, [r4], #-4                     ;br = *bfyrptr1--
	 LDR           r9, [r3], #-4                     ;bi = *bfyiptr1--
	 LDR           r10, [r5], #4                     ;cr_t = *brxmixptr++
	 LDR           r11, [r5], #4                     ;ci_t = *brxmixptr++

	 SMMUL         r12, r10, r8
	 SMMUL         r14, r11, r9
	 ADD           r12, r12, r12
	 SUB           r12, r12, r14, LSL #1             ;rtemp

	 SMMUL         r14, r11, r8
	 SMMUL         r8, r10, r9
	 ADD           r14, r14, r14
	 ADD           r14, r14, r8, LSL #1              ;itemp

	 SUB           r8, r6, r12
	 SUB           r9, r7, r14
	 ADD           r10, r6, r12
	 ADD           r11, r7, r14
	 STR           r8, [r4], #4
	 STR           r9, [r3], #4
	 STR           r10, [r4], #4
	 STR           r11, [r3], #4
	 SUBS          r0, r0, #0x1
	 BGT           LOOP5
	 B             cifft_end

Lable2
	 ADD           r3, r4, #0x300                    ;fftiptr = p_buff->fftbuf + N/2
	 STR           r4, [sp, #0x8]                    ;push fftrptr      
	 STR           r3, [sp, #0x4]                    ;push fftiptr

	 MOV           r0, #2
	 STR           r0, [sp, #0x10]                   ; nstep

;for(m = nstep; m > 0; m--) -- the branch one time
SW_LOOP
         ; Do first Radix-4 Pass
;for(i = fftn/4, i > 0; i--)
         MOV           r6, #0x10                         ;i = fftn/4
	 LDR           r4, [sp, #0x8]                    ;push fftrptr      
	 LDR           r3, [sp, #0x4]                    ;push fftiptr
SW_LOOP1	 
         LDR           r2, [r4]                          ;ar = *bfyrptr1
	 LDR           r5, [r3]                          ;ai = *bfyiptr1
	 LDR           r7, [r4, #0x40]                   ;br = *bfyrptr2
	 LDR           r8, [r3, #0x40]                   ;bi = *bfyiptr2
	 LDR           r9, [r4, #0x80]                   ;cr = *bfyrptr3
	 LDR           r10, [r3, #0x80]                  ;ci = *bfyiptr3
	 LDR           r11, [r4, #0xC0]                  ;dr = *bfyrptr4
	 LDR           r12, [r3, #0xC0]                  ;di = *bfyiptr4

	 ADD           r0, r2, r9                        ;arcr = ar + cr
	 ADD           r1, r7, r11                       ;brdr = br + dr
	 ADD           r14, r0, r1
	 SUB           r0, r0, r1

	 STR           r14, [r4]
	 STR           r0, [r4, #0x40]
	 ADD           r0, r5, r10                       ;aici = ai + ci
	 ADD           r1, r8, r12                       ;bidi = bi + di
	 ADD           r14, r0, r1
	 SUB           r0, r0, r1
	 STR           r14, [r3]
	 STR           r0, [r3, #0x40]

	 SUB           r0, r5, r10                       ;aici = ai - ci
	 SUB           r1, r7, r11                       ;brdr = br - dr
         ADD           r14, r0, r1
	 SUB           r0, r0, r1
	 STR           r14, [r3, #0x80]
	 STR           r0, [r3, #0xC0]

         SUB           r0, r2, r9                        ;arcr = ar - cr
	 SUB           r1, r8, r12                       ;bidi = bi - di
	 SUB           r14, r0, r1
	 ADD           r0, r0, r1
	 STR           r14, [r4, #0x80]
	 STR           r0, [r4, #0xC0]

	 ADD           r3, r3, #4                        ;*bfyiptr++
	 ADD           r4, r4, #4                        ;*byfrptr++
	 SUBS          r6, r6, #1
	 BGT           SW_LOOP1

	 ;Do all Radix-2 passes except first two and last      
         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x4                          ;j = gp	 
SW_LOOP3
         MOV           r8, #0x8                          ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

SW_LOOP4
	 LDR           r11, [r4, #0x20]                  ;br
	 LDR           r12, [r3, #0x20]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x20]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x20]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           SW_LOOP4
	 ADD           r3, r3, #0x20                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x20                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           SW_LOOP3

         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x8                          ;j SW_LOOP5= gp

SW_LOOP31
         MOV           r8, #0x4                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;cr_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

SW_LOOP41
	 LDR           r11, [r4, #0x10]                  ;br
	 LDR           r12, [r3, #0x10]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x10]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x10]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bSW_LOOP5fyiptr1++

	 BGT           SW_LOOP41
	 ADD           r3, r3, #0x10                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x10                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           SW_LOOP31

         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;ffSW_LOOP5trptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
 
         MOV           r7, #0x10                          ;j = gp

SW_LOOP32
         MOV           r8, #0x2                         ;i = bg = fftn/8
         LDR           r0, [r5], #4                      ;crSW_LOOP5_t = *brxmixptr++
	 LDR           r1, [r5], #4                      ;ci_t = *brxmixptr++

SW_LOOP42
	 LDR           r11, [r4, #0x8]                  ;br
	 LDR           r12, [r3, #0x8]                  ;bi
         LDR           r9, [r4]                          ;ar
	 LDR           r10, [r3]                         ;ai
	 SMMUL         r2, r0, r11
	 SMMUL         r14, r1, r12
	 ADD           r2, r2, r2
	 SUB           r2, r2, r14, LSL #1               ;rtemp
	 SMMUL         r14, r1, r11
	 SMMUL         r11, r0, r12
	 ADD           r14, r14, r14
	 ADD           r14, r14, r11, LSL #1             ;itemp
	 ADD           r11, r9, r2                       ;ar + rtemp
	 ADD           r12, r10, r14                     ;ai + itemp
	 STR           r11, [r4, #0x8]                  ;*bfyrptr2++
	 STR           r12, [r3, #0x8]                  ;*bfyiptr2++
	 SUB           r11, r9, r2                       ;ar - rtemp
	 SUB           r12, r10, r14                     ;ai - itemp
	 SUBS          r8, r8, #1
	 STR           r11, [r4], #4                     ;*bfyrptr1++
	 STR           r12, [r3], #4                     ;*bfyiptr1++

	 BGT           SW_LOOP42
	 ADD           r3, r3, #0x8                     ;bfyiptr1 += bg
	 ADD           r4, r4, #0x8                     ;bfyrptr1 += bg
	 SUBS          r7, r7, #0x1
	 BGT           SW_LOOP32
	 
;Do last Radix-2 pass
         LDR           r3, [sp, #0x4]                    ;fftiptr
	 LDR           r4, [sp, #0x8]                    ;fftrptr
	 LDR           r5, Table                         ;brxmixptr = brxmix
         MOV           r0, #0x20                         ;i = fftn/2

SW_LOOP5
         LDR           r6, [r4], #4                      ;ar = *bfyrptr1++
	 LDR           r7, [r3], #4                      ;ai = *bfyiptr1++
	 LDR           r8, [r4], #-4                     ;br = *bfyrptr1--
	 LDR           r9, [r3], #-4                     ;bi = *bfyiptr1--
	 LDR           r10, [r5], #4                     ;cr_t = *brxmixptr++
	 LDR           r11, [r5], #4                     ;ci_t = *brxmixptr++

	 SMMUL         r12, r10, r8
	 SMMUL         r14, r11, r9
	 ADD           r12, r12, r12
	 SUB           r12, r12, r14, LSL #1             ;rtemp

	 SMMUL         r14, r11, r8
	 SMMUL         r8, r10, r9
	 ADD           r14, r14, r14
	 ADD           r14, r14, r8, LSL #1              ;itemp

	 SUB           r8, r6, r12
	 SUB           r9, r7, r14
	 ADD           r10, r6, r12
	 ADD           r11, r7, r14
	 STR           r8, [r4], #4
	 STR           r9, [r3], #4
	 STR           r10, [r4], #4
	 STR           r11, [r3], #4
	 SUBS          r0, r0, #0x1
	 BGT           SW_LOOP5

	 LDR           r0, [sp, #0x10]
	 LDR           r4, [sp, #0x8]                    ;p_buff->fftbuf
	 SUBS          r0, r0, #1
	 ADD           r6, r4, #0x100                    ;p_buff->fftbuf + N/4
	 ADD           r7, r4, #0x200                    ;p_buff->fftbuf + N/2
	 STR           r6, [sp, #0x8]
	 STR           r7, [sp, #0x4]
	 STR           r0, [sp, #0x10]
	 BGT           SW_LOOP


cifft_end
         ADD           r13, r13, #0x20
	 LDMFD         r13!, {r4 - r12, r15}
	 ENDFUNC

Table
         DCD           brxmix

	 END



