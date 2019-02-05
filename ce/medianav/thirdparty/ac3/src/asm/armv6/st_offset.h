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

;Structure, DECEXEC , Size 0x5c bytes, from ./hdr/vo_ac3_var.h
DECEXEC_decparam                       EQU    0        ;  pointer to DEC_PARAM
DECEXEC_inparam                        EQU    0x4      ;  pointer to AC3D_CONF_PARAM
DECEXEC_ac3_buff                       EQU    0x8      ;  pointer to BUFF_PARAM
DECEXEC_ac3_info_list                  EQU    0xc      ;  pointer to AC3_INFO_PL
DECEXEC_ac3_info_rptr                  EQU    0x10     ;  pointer to AC3_INFO_RL
DECEXEC_crc_calc_list                  EQU    0x14     ;  pointer to CRC_CALC_PL
DECEXEC_ac3_dec_list                   EQU    0x18     ;  pointer to AC3_DEC_PL
DECEXEC_stream                         EQU    0x1c     ;  pointer to FrameStream
DECEXEC_pvoMemop                       EQU    0x20     ;  pointer to VO_MEM_OPERATOR
DECEXEC_voMemoprator                   EQU    0x24     ;  VO_MEM_OPERATOR
DECEXEC_voMemoprator_Alloc             EQU    0x24     ;  pointer to function 
DECEXEC_voMemoprator_Free              EQU    0x28     ;  pointer to function 
DECEXEC_voMemoprator_Set               EQU    0x2c     ;  pointer to function 
DECEXEC_voMemoprator_Copy              EQU    0x30     ;  pointer to function 
DECEXEC_voMemoprator_Check             EQU    0x34     ;  pointer to function 
DECEXEC_voMemoprator_Compare           EQU    0x38     ;  pointer to function 
DECEXEC_voMemoprator_Move              EQU    0x3c     ;  pointer to function 
DECEXEC_pk_buf                         EQU    0x40     ;  pointer to unsigned char
DECEXEC_input_length                   EQU    0x44     ;  Word32
DECEXEC_output_length                  EQU    0x48     ;  Word16
;Padding 0x2 bytes
DECEXEC_pContext                       EQU    0x4c     ;  pointer to void
DECEXEC_pcm_buf                        EQU    0x50     ;  pointer to Word32
DECEXEC_output_buf                     EQU    0x54     ;  pointer to Word32
DECEXEC_hCheck                         EQU    0x58     ;  pointer to void
;End of Structure DECEXEC


;Structure, AC3D_CONF_PARAM , Size 0x90 bytes, from ./hdr/vo_ac3_ghdr.h
AC3D_CONF_PARAM_pcmptr                 EQU    0        ;  array[6] of pointer to Word32
AC3D_CONF_PARAM_pcmoff                 EQU    0x18     ;  array[6] of Word16
AC3D_CONF_PARAM_pcmmod                 EQU    0x24     ;  array[6] of Word16
AC3D_CONF_PARAM_chanptr                EQU    0x30     ;  array[6] of Word32
AC3D_CONF_PARAM_numchans               EQU    0x48     ;  Word32
AC3D_CONF_PARAM_wordsize               EQU    0x4c     ;  Word32
AC3D_CONF_PARAM_dynrngscalelow         EQU    0x50     ;  Word32
AC3D_CONF_PARAM_dynrngscalehi          EQU    0x54     ;  Word32
AC3D_CONF_PARAM_pcmscalefac            EQU    0x58     ;  Word32
AC3D_CONF_PARAM_compmode               EQU    0x5c     ;  Word32
AC3D_CONF_PARAM_stereomode             EQU    0x60     ;  Word32
AC3D_CONF_PARAM_dualmonomode           EQU    0x64     ;  Word32
AC3D_CONF_PARAM_outputmode             EQU    0x68     ;  Word32
AC3D_CONF_PARAM_outlfeon               EQU    0x6c     ;  Word32
AC3D_CONF_PARAM_outputflg              EQU    0x70     ;  Word32
AC3D_CONF_PARAM_framecount             EQU    0x74     ;  long
AC3D_CONF_PARAM_blockcount             EQU    0x78     ;  Word32
AC3D_CONF_PARAM_framestart             EQU    0x7c     ;  Word32
AC3D_CONF_PARAM_frameend               EQU    0x80     ;  Word32
AC3D_CONF_PARAM_useverbose             EQU    0x84     ;  Word32
AC3D_CONF_PARAM_debug_arg              EQU    0x88     ;  Word32
AC3D_CONF_PARAM_kcapablemode           EQU    0x8c     ;  Word32
;End of Structure AC3D_CONF_PARAM

;Structure, FrameStream , Size 0x1c bytes, from ./hdr/stream.h
FrameStream_buffer                    EQU    0        ;  pointer to unsigned char
FrameStream_buffer_bk                 EQU    0x4      ;  pointer to unsigned char
FrameStream_bufend                    EQU    0x8      ;  pointer to unsigned char
FrameStream_set_ptr                   EQU    0xc      ;  pointer to unsigned char
FrameStream_set_len                   EQU    0x10     ;  int
FrameStream_maxframesize              EQU    0x14     ;  int
FrameStream_used_len                  EQU    0x18     ;  int
;End of Structure FrameStream


;Structure, PKEXPS , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
PKEXPS_grpsz                          EQU    0        ;  Word16
PKEXPS_npkgrps                        EQU    0x2      ;  Word16
PKEXPS_absexp                         EQU    0x4      ;  Word16
;Padding 0x2 bytes
PKEXPS_pkptr                          EQU    0x8      ;  pointer to Word16
PKEXPS_pkbitptr                       EQU    0xc      ;  Word16
PKEXPS_pkdata                         EQU    0xe      ;  Word16
;End of Structure PKEXPS

;Structure, AC3_INFO_PL , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
AC3_INFO_PL_size                       EQU    0        ;  Word16
;Padding 0x2 bytes
AC3_INFO_PL_iptr                       EQU    0x4      ;  pointer to Word16
AC3_INFO_PL_ioff                       EQU    0x8      ;  Word16
AC3_INFO_PL_imod                       EQU    0xa      ;  Word16
AC3_INFO_PL_icfg                       EQU    0xc      ;  Word16
;End of Structure AC3_INFO_PL

;Structure, AC3_INFO_RL , Size 0x14 bytes, from ./hdr/vo_ac3_var.h
AC3_INFO_RL_size                       EQU    0        ;  Word16
AC3_INFO_RL_bscfg                      EQU    0x2      ;  Word16
AC3_INFO_RL_frmsize                    EQU    0x4      ;  Word16
AC3_INFO_RL_crcsize                    EQU    0x6      ;  Word16
AC3_INFO_RL_bsinfo                     EQU    0x8      ;  Word16
AC3_INFO_RL_dialnorm                   EQU    0xa      ;  Word16
AC3_INFO_RL_langcod                    EQU    0xc      ;  Word16
AC3_INFO_RL_audprod                    EQU    0xe      ;  Word16
AC3_INFO_RL_timecod1                   EQU    0x10     ;  Word16
AC3_INFO_RL_timecod2                   EQU    0x12     ;  Word16
;End of Structure AC3_INFO_RL

;Structure, AC3_DEC_PL , Size 0x38 bytes, from ./hdr/vo_ac3_var.h
AC3_DEC_PL_size                        EQU    0        ;  Word16
;Padding 0x2 bytes
AC3_DEC_PL_iptr                        EQU    0x4      ;  pointer to Word16
AC3_DEC_PL_ioff                        EQU    0x8      ;  Word16
AC3_DEC_PL_imod                        EQU    0xa      ;  Word16
AC3_DEC_PL_icfg                        EQU    0xc      ;  Word16
;Padding 0x2 bytes
AC3_DEC_PL_ooff                        EQU    0x10     ;  pointer to Word16
AC3_DEC_PL_omod                        EQU    0x14     ;  pointer to Word16
AC3_DEC_PL_ocfg                        EQU    0x18     ;  Word16
AC3_DEC_PL_blknum                      EQU    0x1a     ;  Word16
AC3_DEC_PL_rptmax                      EQU    0x1c     ;  Word16
AC3_DEC_PL_debug                       EQU    0x1e     ;  Word16
AC3_DEC_PL_optr                        EQU    0x20     ;  pointer to pointer to Word32
AC3_DEC_PL_dynsclh                     EQU    0x24     ;  Word32
AC3_DEC_PL_dynscll                     EQU    0x28     ;  Word32
AC3_DEC_PL_pcmscl                      EQU    0x2c     ;  Word32
AC3_DEC_PL_dnmxptr                     EQU    0x30     ;  pointer to Word32
AC3_DEC_PL_krkptr                      EQU    0x34     ;  pointer to Word32
;End of Structure AC3_DEC_PL


;Structure, CRC_CALC_PL , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
CRC_CALC_PL_size                       EQU    0        ;  Word16
;Padding 0x2 bytes
CRC_CALC_PL_iptr                       EQU    0x4      ;  pointer to Word16
CRC_CALC_PL_ioff                       EQU    0x8      ;  Word16
CRC_CALC_PL_imod                       EQU    0xa      ;  Word16
CRC_CALC_PL_icfg                       EQU    0xc      ;  Word16
CRC_CALC_PL_count                      EQU    0xe      ;  Word16
;End of Structure CRC_CALC_PL


;Structure, AC3_INFO_SI , Size 0x68 bytes, from ./hdr/vo_ac3_var.h
AC3_INFO_SI_syncword                   EQU    0        ;  Word16
AC3_INFO_SI_crc1                       EQU    0x2      ;  Word16
AC3_INFO_SI_fscod                      EQU    0x4      ;  Word16
AC3_INFO_SI_frmsizecod                 EQU    0x6      ;  Word16
AC3_INFO_SI_bsid                       EQU    0x8      ;  Word16
AC3_INFO_SI_bsmod                      EQU    0xa      ;  Word16
AC3_INFO_SI_acmod                      EQU    0xc      ;  Word16
AC3_INFO_SI_cmixlev                    EQU    0xe      ;  Word16
AC3_INFO_SI_surmixlev                  EQU    0x10     ;  Word16
AC3_INFO_SI_dsurmod                    EQU    0x12     ;  Word16
AC3_INFO_SI_lfeon                      EQU    0x14     ;  Word16
AC3_INFO_SI_dialnorm                   EQU    0x16     ;  Word16
AC3_INFO_SI_compre                     EQU    0x18     ;  Word16
AC3_INFO_SI_compr                      EQU    0x1a     ;  Word16
AC3_INFO_SI_langcode                   EQU    0x1c     ;  Word16
AC3_INFO_SI_langcod                    EQU    0x1e     ;  Word16
AC3_INFO_SI_audprodie                  EQU    0x20     ;  Word16
AC3_INFO_SI_mixlevel                   EQU    0x22     ;  Word16
AC3_INFO_SI_roomtyp                    EQU    0x24     ;  Word16
AC3_INFO_SI_dialnorm2                  EQU    0x26     ;  Word16
AC3_INFO_SI_compr2e                    EQU    0x28     ;  Word16
AC3_INFO_SI_compr2                     EQU    0x2a     ;  Word16
AC3_INFO_SI_langcod2e                  EQU    0x2c     ;  Word16
AC3_INFO_SI_langcod2                   EQU    0x2e     ;  Word16
AC3_INFO_SI_audprodi2e                 EQU    0x30     ;  Word16
AC3_INFO_SI_mixlevel2                  EQU    0x32     ;  Word16
AC3_INFO_SI_roomtyp2                   EQU    0x34     ;  Word16
AC3_INFO_SI_copyrightb                 EQU    0x36     ;  Word16
AC3_INFO_SI_origbs                     EQU    0x38     ;  Word16
AC3_INFO_SI_timecod1e                  EQU    0x3a     ;  Word16
AC3_INFO_SI_timecod1                   EQU    0x3c     ;  Word16
AC3_INFO_SI_timecod2e                  EQU    0x3e     ;  Word16
AC3_INFO_SI_timecod2                   EQU    0x40     ;  Word16
AC3_INFO_SI_addbsie                    EQU    0x42     ;  Word16
AC3_INFO_SI_addbsil                    EQU    0x44     ;  Word16
AC3_INFO_SI_nfchans                    EQU    0x46     ;  Word16
AC3_INFO_SI_nchans                     EQU    0x48     ;  Word16
AC3_INFO_SI_xbsi1e                     EQU    0x4a     ;  Word16
AC3_INFO_SI_dmixmod                    EQU    0x4c     ;  Word16
AC3_INFO_SI_ltrtcmixlev                EQU    0x4e     ;  Word16
AC3_INFO_SI_ltrtsurmixlev              EQU    0x50     ;  Word16
AC3_INFO_SI_lorocmixlev                EQU    0x52     ;  Word16
AC3_INFO_SI_lorosurmixlev              EQU    0x54     ;  Word16
AC3_INFO_SI_xbsi2e                     EQU    0x56     ;  Word16
AC3_INFO_SI_dsurexmod                  EQU    0x58     ;  Word16
AC3_INFO_SI_dheadphonmod               EQU    0x5a     ;  Word16
AC3_INFO_SI_adconvtyp                  EQU    0x5c     ;  Word16
AC3_INFO_SI_xbsi2                      EQU    0x5e     ;  Word16
AC3_INFO_SI_encinfo                    EQU    0x60     ;  Word16
AC3_INFO_SI_dmixmodd                   EQU    0x62     ;  Word16
AC3_INFO_SI_dsurexmodd                 EQU    0x64     ;  Word16
AC3_INFO_SI_dheadphonmodd              EQU    0x66     ;  Word16
;End of Structure AC3_INFO_SI

;Structure, AC3_BSPK , Size 0x10 bytes, from ./hdr/vo_ac3_var.h
AC3_BSPK_pkbufptr                      EQU    0        ;  pointer to Word16
AC3_BSPK_pkptr                         EQU    0x4      ;  pointer to Word16
AC3_BSPK_pkbitptr                      EQU    0x8      ;  Word16
AC3_BSPK_pkdata                        EQU    0xa      ;  Word16
AC3_BSPK_pkmod                         EQU    0xc      ;  Word16
AC3_BSPK_pkwrdsz                       EQU    0xe      ;  Word16
;End of Structure AC3_BSPK

;Structure, BUFF_PARAM , Size 0x48d4 bytes, from ./hdr/vo_ac3_var.h
BUFF_PARAM_fftbuf                      EQU    0        ;  array[256] of Word32
BUFF_PARAM_tc1                         EQU    0x400    ;  array[256] of Word32
BUFF_PARAM_tc2                         EQU    0x800    ;  array[256] of Word32
BUFF_PARAM_dnmix_buf                   EQU    0xc00    ;  array[6] of array[256] of Word32
BUFF_PARAM_delay_buf                   EQU    0x2400   ;  array[6] of array[128] of Word32
BUFF_PARAM_mantbuf                     EQU    0x3000   ;  array[24] of Word32
BUFF_PARAM_psdbuf                      EQU    0x3060   ;  array[24] of Word16
BUFF_PARAM_expbuf                      EQU    0x3090   ;  pointer to Word16
BUFF_PARAM_cplexps                     EQU    0x3094   ;  array[253] of Word16
BUFF_PARAM_exps                        EQU    0x328e   ;  array[5] of array[253] of Word16
BUFF_PARAM_lfeexps                     EQU    0x3c70   ;  array[7] of Word16
;Padding 0x2 bytes
BUFF_PARAM_bapbuf                      EQU    0x3c80   ;  pointer to Word16
BUFF_PARAM_newbitalloc                 EQU    0x3c84   ;  Word16
BUFF_PARAM_cplbap                      EQU    0x3c86   ;  array[253] of Word16
BUFF_PARAM_bap                         EQU    0x3e80   ;  array[5] of array[253] of Word16
BUFF_PARAM_lfebap                      EQU    0x4862   ;  array[7] of Word16
BUFF_PARAM_deltary                     EQU    0x4870   ;  array[50] of Word16
; End of Structure BUFF_PARAM


;Structure, DEC_PARAM , Size 0x7c bytes, from ./hdr/vo_ac3_var.h
DEC_PARAM_ac3_str                      EQU    0        ;  pointer to AC3_BSPK
DEC_PARAM_ac3_si                       EQU    0x4      ;  pointer to AC3_INFO_SI
DEC_PARAM_ac3_aub                      EQU    0x8      ;  pointer to AC3_AB_DATA
DEC_PARAM_blknum                       EQU    0xc      ;  Word16
DEC_PARAM_channum                      EQU    0xe      ;  Word16
DEC_PARAM_bswitch                      EQU    0x10     ;  Word16
DEC_PARAM_rptcnt                       EQU    0x12     ;  Word16
DEC_PARAM_dithreg                      EQU    0x14     ;  Word16
DEC_PARAM_in_stat                      EQU    0x16     ;  Word16
DEC_PARAM_out_stat                     EQU    0x18     ;  Word16
DEC_PARAM_ac3raminit                   EQU    0x1a     ;  Word16
DEC_PARAM_outmod                       EQU    0x1c     ;  Word16
DEC_PARAM_outlfe                       EQU    0x1e     ;  Word16
DEC_PARAM_outnchans                    EQU    0x20     ;  Word16
DEC_PARAM_lastoutmod                   EQU    0x22     ;  Word16
DEC_PARAM_lastoutlfe                   EQU    0x24     ;  Word16
DEC_PARAM_pcmbufoff                    EQU    0x26     ;  array[6] of Word16
DEC_PARAM_pcmbufmod                    EQU    0x32     ;  array[6] of Word16
DEC_PARAM_compmod                      EQU    0x3e     ;  Word16
DEC_PARAM_digdialnorm                  EQU    0x40     ;  Word16
DEC_PARAM_dualmod                      EQU    0x42     ;  Word16
DEC_PARAM_stereomod                    EQU    0x44     ;  Word16
DEC_PARAM_halfratecod                  EQU    0x46     ;  Word16
DEC_PARAM_useverbose                   EQU    0x48     ;  Word32
DEC_PARAM_karaokeflag                  EQU    0x4c     ;  Word16
;Padding 0x2 bytes
DEC_PARAM_kcapableptr                  EQU    0x50     ;  pointer to Word32
DEC_PARAM_dynscalehigh                 EQU    0x54     ;  Word32
DEC_PARAM_dynscalelow                  EQU    0x58     ;  Word32
DEC_PARAM_pcmscale                     EQU    0x5c     ;  Word32
DEC_PARAM_ext_dnmixtab                 EQU    0x60     ;  pointer to Word32
DEC_PARAM_pcmbufptr                    EQU    0x64     ;  array[6] of pointer to Word32
;End of Structure DEC_PARAM

;Structure, AC3_AB_DATA , Size 0x650 bytes, from ./hdr/vo_ac3_var.h
AC3_AB_DATA_blksw                      EQU    0        ;  array[5] of Word16
AC3_AB_DATA_dithflag                   EQU    0xa      ;  array[5] of Word16
AC3_AB_DATA_dynrnge                    EQU    0x14     ;  Word16
AC3_AB_DATA_dynrng                     EQU    0x16     ;  Word16
AC3_AB_DATA_dynrng2e                   EQU    0x18     ;  Word16
AC3_AB_DATA_dynrng2                    EQU    0x1a     ;  Word16
AC3_AB_DATA_gainexp                    EQU    0x1c     ;  Word16
AC3_AB_DATA_cplstre                    EQU    0x1e     ;  Word16
AC3_AB_DATA_cplinu                     EQU    0x20     ;  Word16
AC3_AB_DATA_chincpl                    EQU    0x22     ;  array[5] of Word16
AC3_AB_DATA_phsflginu                  EQU    0x2c     ;  Word16
AC3_AB_DATA_cplbegf                    EQU    0x2e     ;  Word16
AC3_AB_DATA_cplendf                    EQU    0x30     ;  Word16
AC3_AB_DATA_cplbndstrc                 EQU    0x32     ;  array[18] of Word16
AC3_AB_DATA_cplchan                    EQU    0x56     ;  Word16
AC3_AB_DATA_cplstrtmant                EQU    0x58     ;  Word16
AC3_AB_DATA_cplendmant                 EQU    0x5a     ;  Word16
AC3_AB_DATA_cplcoe                     EQU    0x5c     ;  array[5] of Word16
AC3_AB_DATA_mstrcplco                  EQU    0x66     ;  Word16
AC3_AB_DATA_cplcoexp                   EQU    0x68     ;  array[5] of array[18] of Word16
AC3_AB_DATA_phsflg                     EQU    0x11c    ;  Word16
AC3_AB_DATA_phscore                    EQU    0x11e    ;  Word16
AC3_AB_DATA_phsoutmod                  EQU    0x120    ;  Word16
AC3_AB_DATA_phscorstr                  EQU    0x122    ;  Word16
AC3_AB_DATA_phscor                     EQU    0x124    ;  array[18] of Word16
AC3_AB_DATA_sclcplcoexp                EQU    0x148    ;  array[18] of Word16
AC3_AB_DATA_nrematbnds                 EQU    0x16c    ;  Word16
AC3_AB_DATA_rematstr                   EQU    0x16e    ;  Word16
AC3_AB_DATA_rematflg                   EQU    0x170    ;  array[4] of Word16
AC3_AB_DATA_rematinu                   EQU    0x178    ;  Word16
AC3_AB_DATA_cplexpstr                  EQU    0x17a    ;  Word16
AC3_AB_DATA_chexpstr                   EQU    0x17c    ;  array[5] of Word16
AC3_AB_DATA_lfeexpstr                  EQU    0x186    ;  Word16
AC3_AB_DATA_chbwcod                    EQU    0x188    ;  array[5] of Word16
AC3_AB_DATA_endmant                    EQU    0x192    ;  array[5] of Word16
AC3_AB_DATA_cplpkexps                  EQU    0x19c    ;  PKEXPS
AC3_AB_DATA_cplpkexps_grpsz            EQU    0x19c    ;  Word16
AC3_AB_DATA_cplpkexps_npkgrps          EQU    0x19e    ;  Word16
AC3_AB_DATA_cplpkexps_absexp           EQU    0x1a0    ;  Word16
;Padding 0x2 bytes
AC3_AB_DATA_cplpkexps_pkptr            EQU    0x1a4    ;  pointer to Word16
AC3_AB_DATA_cplpkexps_pkbitptr         EQU    0x1a8    ;  Word16
AC3_AB_DATA_cplpkexps_pkdata           EQU    0x1aa    ;  Word16
AC3_AB_DATA_pkexps                     EQU    0x1ac    ;  array[5] of PKEXPS
AC3_AB_DATA_lfepkexps                  EQU    0x1fc    ;  PKEXPS
AC3_AB_DATA_lfepkexps_grpsz            EQU    0x1fc    ;  Word16
AC3_AB_DATA_lfepkexps_npkgrps          EQU    0x1fe    ;  Word16
AC3_AB_DATA_lfepkexps_absexp           EQU    0x200    ;  Word16
;Padding 0x2 bytes
AC3_AB_DATA_lfepkexps_pkptr            EQU    0x204    ;  pointer to Word16
AC3_AB_DATA_lfepkexps_pkbitptr         EQU    0x208    ;  Word16
AC3_AB_DATA_lfepkexps_pkdata           EQU    0x20a    ;  Word16
AC3_AB_DATA_gainrng                    EQU    0x20c    ;  array[5] of Word16
AC3_AB_DATA_appgainrng                 EQU    0x216    ;  array[6] of Word16
AC3_AB_DATA_baie                       EQU    0x222    ;  Word16
AC3_AB_DATA_sdcycod                    EQU    0x224    ;  Word16
AC3_AB_DATA_fdcycod                    EQU    0x226    ;  Word16
AC3_AB_DATA_sgaincod                   EQU    0x228    ;  Word16
AC3_AB_DATA_dbpbcod                    EQU    0x22a    ;  Word16
AC3_AB_DATA_floorcod                   EQU    0x22c    ;  Word16
AC3_AB_DATA_snroffste                  EQU    0x22e    ;  Word16
AC3_AB_DATA_csnroffst                  EQU    0x230    ;  Word16
AC3_AB_DATA_cplfsnroffst               EQU    0x232    ;  Word16
AC3_AB_DATA_fsnroffst                  EQU    0x234    ;  array[5] of Word16
AC3_AB_DATA_lfefsnroffst               EQU    0x23e    ;  Word16
AC3_AB_DATA_cplfgaincod                EQU    0x240    ;  Word16
AC3_AB_DATA_fgaincod                   EQU    0x242    ;  array[5] of Word16
AC3_AB_DATA_lfefgaincod                EQU    0x24c    ;  Word16
AC3_AB_DATA_cplleake                   EQU    0x24e    ;  Word16
AC3_AB_DATA_cplfleak                   EQU    0x250    ;  Word16
AC3_AB_DATA_cplsleak                   EQU    0x252    ;  Word16
AC3_AB_DATA_deltbaie                   EQU    0x254    ;  Word16
AC3_AB_DATA_cpldeltbae                 EQU    0x256    ;  Word16
AC3_AB_DATA_deltbae                    EQU    0x258    ;  array[5] of Word16
AC3_AB_DATA_cpldeltnseg                EQU    0x262    ;  Word16
AC3_AB_DATA_cpldeltoffst               EQU    0x264    ;  array[8] of Word16
AC3_AB_DATA_cpldeltlen                 EQU    0x274    ;  array[8] of Word16
AC3_AB_DATA_cpldeltba                  EQU    0x284    ;  array[8] of Word16
AC3_AB_DATA_chdeltnseg                 EQU    0x294    ;  array[5] of Word16
AC3_AB_DATA_chdeltoffst                EQU    0x29e    ;  array[5] of array[8] of Word16
AC3_AB_DATA_chdeltlen                  EQU    0x2ee    ;  array[5] of array[8] of Word16
AC3_AB_DATA_chdeltba                   EQU    0x33e    ;  array[5] of array[8] of Word16
AC3_AB_DATA_skiple                     EQU    0x38e    ;  Word16
AC3_AB_DATA_skipl                      EQU    0x390    ;  Word16
AC3_AB_DATA_count3                     EQU    0x392    ;  Word16
AC3_AB_DATA_count5                     EQU    0x394    ;  Word16
AC3_AB_DATA_count11                    EQU    0x396    ;  Word16
AC3_AB_DATA_mant3                      EQU    0x398    ;  UWord16
AC3_AB_DATA_mant5                      EQU    0x39a    ;  UWord16
AC3_AB_DATA_mant11                     EQU    0x39c    ;  UWord16
AC3_AB_DATA_chcount3                   EQU    0x39e    ;  Word16
AC3_AB_DATA_chcount5                   EQU    0x3a0    ;  Word16
AC3_AB_DATA_chcount11                  EQU    0x3a2    ;  Word16
AC3_AB_DATA_chmant3                    EQU    0x3a4    ;  UWord16
AC3_AB_DATA_chmant5                    EQU    0x3a6    ;  UWord16
AC3_AB_DATA_chmant11                   EQU    0x3a8    ;  UWord16
;Padding 0x2 bytes
AC3_AB_DATA_chpkptr                    EQU    0x3ac    ;  pointer to Word16
AC3_AB_DATA_chpkbitptr                 EQU    0x3b0    ;  Word16
AC3_AB_DATA_chpkdata                   EQU    0x3b2    ;  Word16
AC3_AB_DATA_cplcount3                  EQU    0x3b4    ;  Word16
AC3_AB_DATA_cplcount5                  EQU    0x3b6    ;  Word16
AC3_AB_DATA_cplcount11                 EQU    0x3b8    ;  Word16
AC3_AB_DATA_cplmant3                   EQU    0x3ba    ;  UWord16
AC3_AB_DATA_cplmant5                   EQU    0x3bc    ;  UWord16
AC3_AB_DATA_cplmant11                  EQU    0x3be    ;  UWord16
AC3_AB_DATA_cplpkptr                   EQU    0x3c0    ;  pointer to Word16
AC3_AB_DATA_cplpkbitptr                EQU    0x3c4    ;  Word16
AC3_AB_DATA_cplpkdata                  EQU    0x3c6    ;  Word16
AC3_AB_DATA_sdecay                     EQU    0x3c8    ;  Word16
AC3_AB_DATA_fdecay                     EQU    0x3ca    ;  Word16
AC3_AB_DATA_sgain                      EQU    0x3cc    ;  Word16
AC3_AB_DATA_fgain                      EQU    0x3ce    ;  Word16
AC3_AB_DATA_dbknee                     EQU    0x3d0    ;  Word16
AC3_AB_DATA_floorval                   EQU    0x3d2    ;  Word16
AC3_AB_DATA_snroff                     EQU    0x3d4    ;  Word16
AC3_AB_DATA_minsnrflg                  EQU    0x3d6    ;  Word16
AC3_AB_DATA_bandindex                  EQU    0x3d8    ;  Word16
AC3_AB_DATA_mantindex                  EQU    0x3da    ;  Word16
AC3_AB_DATA_lastmant                   EQU    0x3dc    ;  Word16
AC3_AB_DATA_dith                       EQU    0x3de    ;  Word16
AC3_AB_DATA_cplflag                    EQU    0x3e0    ;  Word16
;Padding 0x2 bytes
AC3_AB_DATA_cplcoexpptr                EQU    0x3e4    ;  pointer to Word16
AC3_AB_DATA_cplcnt                     EQU    0x3e8    ;  Word16
AC3_AB_DATA_fleak                      EQU    0x3ea    ;  Word16
AC3_AB_DATA_sleak                      EQU    0x3ec    ;  Word16
AC3_AB_DATA_dnmixbufinu                EQU    0x3ee    ;  array[6] of Word16
;Padding 0x2 bytes
AC3_AB_DATA_dnmixtab                   EQU    0x3fc    ;  array[6] of array[6] of Word32
AC3_AB_DATA_compfact                   EQU    0x48c    ;  Word32
AC3_AB_DATA_compfact2                  EQU    0x490    ;  Word32
AC3_AB_DATA_cplco                      EQU    0x494    ;  array[5] of array[18] of Word32
AC3_AB_DATA_gainmant                   EQU    0x5fc    ;  Word32
AC3_AB_DATA_sclcplco                   EQU    0x600    ;  array[18] of Word32
AC3_AB_DATA_tcbuf                      EQU    0x648    ;  pointer to Word32
AC3_AB_DATA_cplcoptr                   EQU    0x64c    ;  pointer to Word32
;End of Structure AC3_AB_DATA


;Structure, DOLBY_SIP , Size 0x8 bytes, from ./hdr/vo_ac3_var.h
DOLBY_SIP_funcnum                      EQU    0        ;  Word16
DOLBY_SIP_status                       EQU    0x2      ;  Word16
DOLBY_SIP_param_ptr                    EQU    0x4      ;  pointer to void
;End of Structure DOLBY_SIP
