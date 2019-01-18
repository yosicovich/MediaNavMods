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
      
        AREA    |.text|, CODE, READONLY
	EXPORT  crc_calc
	IMPORT  crctab


;Structure, CRC_CALC_PL , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
CRC_CALC_PL_iptr                       EQU    0x4      ;  pointer to Word16
CRC_CALC_PL_count                      EQU    0xe      ;  Word16
;End of Structure CRC_CALC_PL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;DOLBY_SIP crc_calc(DOLBY_SIP input_sip)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; DOLBY_SIP Structure
; typedef struct {
;       Word16  funcnum;          --- r0
;       Word16  status;           --- r1
;       void    *param_ptr;       --- r2
; }DOLBY_SIP


crc_calc FUNCTION

        STMFD    r13!, {r4 - r12, r14}

        LDR      r7, [r2, #CRC_CALC_PL_iptr]            ;r7 = iptr
        UXTH     r1, r1, ROR #16                        ;r1 = status --- syndrom
        LDRSH    r8, [r2, #CRC_CALC_PL_count]           ;r8 = buflen
        LDR      r11,Table1 
        LDR      r9, Table2                             ; Load SIP_REV    
        LDRH     r3, [r7], #2                           ; bufptr[i]                         
        SUBS     r8, r8,#1
        BLT      crc_calc_end 

; for (i = buflen; i > 0; i--)
Lable1        

        ORR     r1, r3, r1, LSL #16
        UXTB    r6, r1, ROR #24
        MOV     r6, r6, LSL #1
        LDRH    r5, [r11, r6]                            ;load crctab[(syndrome >> 8) & 0xff]
        EOR     r1, r1, r5, LSL #8                                           

        UXTB    r6, r1, ROR #16
        MOV     r6, r6, LSL #1
        LDRH    r3, [r7], #2   
        LDRH    r5, [r11, r6]                            ;load crctab[(syndrome >> 8) & 0xff]                                               
        SUBS    r8, r8, #1
        EOR     r1, r1, r5        
        
        BGE     Lable1

crc_calc_end 
 
        ORR     r9, r9, r1, LSL #16
        MOV     r10, #0                                  ;ret_sip.param_ptr = NULLPTR
        STR     r9, [r0, #0]
        STR     r10, [r0, #4]                            ;ret_sip.funcnum = SIP_REV
        LDMFD   r13!, {r4 - r12, r15}

        ENDFUNC

Table1
        DCD      crctab
Table2
        DCD      0x00000701                              ;((7<<8) + 1)
  
	END
