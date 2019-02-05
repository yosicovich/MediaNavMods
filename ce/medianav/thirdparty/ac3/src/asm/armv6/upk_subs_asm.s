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
    

        AREA           |.text|, CODE, READONLY
        EXPORT         bitunp_rj
        EXPORT         bitunp_lj
        EXPORT         bitskip
        
;Structure, AC3_BSPK , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
AC3_BSPK_pkptr                         EQU    0x4      ;  pointer to Word16
AC3_BSPK_pkbitptr                      EQU    0x8      ;  Word16
AC3_BSPK_pkdata                        EQU    0xa      ;  Word16
;End of Structure AC3_BSPK

;*******************************************************************************
;void bitunp_rj(DSPshort *dataptr, DSPshort numbits, AC3_BSPK *p_bstrm)
;*******************************************************************************
bitunp_rj            FUNCTION

        STMFD    sp!, {r4 - r10, r14}                     
        LDR      r4,[r2,#AC3_BSPK_pkptr]                      ;r5 = pkptr
        LDRH     r5,[r2,#AC3_BSPK_pkbitptr]                   ;r5 = pkptr     
        LDRH     r9,[r4]
        LDRH     r10,[r4,#2]     
        ORR      r6,r10,r9,LSL #0x10
        
        RSB      r7,r1,#0x20                                    ;r7  = 32-r1
        MOV      r6,r6,LSL r5
        MOV      r6,r6,LSR r7
              
        ADD      r8,r5,r1
        AND      r5,r8,#0xF                                      
        SUBS     r8,r8,r5
                  
        LDRNEH   r9,[r4,#2]!
        STRH     r6,[r0]

        STR      r4,[r2,#AC3_BSPK_pkptr]                      ;r4 = pkptr
        STRH     r5,[r2,#AC3_BSPK_pkbitptr]
        STRNEH   r9,[r2,#AC3_BSPK_pkdata]

        LDMFD    sp!, {r4-r10,pc}
        ENDFUNC               

;*******************************************************************************
;void bitunp_lj(DSPshort *dataptr, DSPshort numbits,AC3_BSPK *p_bstrm)
;*******************************************************************************

bitunp_lj            FUNCTION

        STMFD    sp!, {r4-r10, r14}                    
        LDR      r4,[r2,#AC3_BSPK_pkptr]                      ;r5 = pkptr
        LDRH     r5,[r2,#AC3_BSPK_pkbitptr]                   ;r5 = pkptr
        LDRH     r9,[r4]
        LDRH     r10,[r4,#2]     
        ORR       r6,r10,r9,LSL #0x10
     
        RSB      r7,r1,#0x20                                    ;r7  = 32-r1
        SUB      r8,r7,r5                                       ;r8  = 32-r10-r2
        MOV      r6,r6,LSR r8
        MOV      r6,r6,LSL r7
        MOV      r6,r6,LSR #16
        
        ADD      r8,r5,r1
        AND      r5,r8,#0xF                                      
        SUBS     r8,r8,r5
                 
        LDRNEH   r9,[r4,#2]!
        STRH     r6,[r0]

        STR      r4,[r2,#AC3_BSPK_pkptr]                      ;r4 = pkptr
        STRH     r5,[r2,#AC3_BSPK_pkbitptr]
        STRNEH   r9,[r2,#AC3_BSPK_pkdata]

        LDMFD    sp!, {r4-r10,pc}
        ENDFUNC               
        
        
;*******************************************************************************
;void bitskip(DSPshort numbits, AC3_BSPK *p_bstrm)
;*******************************************************************************
; bitskip            FUNCTION

        ; STMFD    sp!, {r4-r12,r14}                      

        ; LDR      r4,[r1,#AC3_BSPK_pkptr]                      ;r5 = pkptr
        ; LDRH     r5,[r1,#AC3_BSPK_pkbitptr]                   ;r5 = pkptr

        ; ADD      r6,r5,r0
        ; AND      r5,r6,#0xF
        ; MOV      r6,r6,LSR #4                                    

        ; MOV      r6,r6,LSL #1      
        ; LDRH     r7,[r4,r6]!

        ; STR      r4,[r1,#AC3_BSPK_pkptr]                      ;r4 = pkptr
        ; STRH     r5,[r1,#AC3_BSPK_pkbitptr]
        ; STRH     r7,[r1,#AC3_BSPK_pkdata]

        ; LDMFD    sp!, {r4-r12,pc}
        ; ENDFUNC          
        
        ;ENDIF
        END
	        
	        
