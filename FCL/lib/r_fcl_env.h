#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/**********************************************************************************************************************
* Library       : Code Flash Access Library for Renesas RH850 devices, based on the RV40 Flash technology
*
* File Name     : $Source: r_fcl_env.h $
* Lib. Version  : $RH850_FCL_LIB_VERSION_T01: V2.13 $
* Mod. Revision : $Revision: 1.30 $
* Mod. Date     : $Date: 2019/04/18 13:13:42JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : Flash programming hardware related definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only  intended for use with
* Renesas products. No other uses are authorized. This software is owned by Renesas Electronics
* Corporation and is protected under all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR
* ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the
* availability of this software. By using this software, you agree to the additional terms and conditions
* found by accessing the  following link:
* www.renesas.com/disclaimer
*
* Copyright (C) 2015-2019 Renesas Electronics Corporation. All rights reserved.
**********************************************************************************************************************/

#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_ON 0292
#endif

/**********************************************************************************************************************
* MISRA Rule:   MISRA-C 2004 rule 3.1 (QAC message 0292)
* Reason:       To support automatic insertion of revision, module name etc. by the source
*               revision control system it is necessary to violate the rule, because the
*               system uses non basic characters as placeholders.
* Verification: The placeholders are used in commentars only. Therefore rule violation cannot
*               influency code compilation.
**********************************************************************************************************************/

#ifndef R_FCL_ENV_H
#define R_FCL_ENV_H

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/
#ifdef R_FCL_CPE_SUPPORT
    #include "r_fcl_env_cpe.h"
#else
    #define R_FPSYS_REG_AREA_BASE                       (0xFFA10000u)
    #define R_FPSYS_FCURAM_AREA_BASE                    (0xFFA12000u)
    #define R_FPSYS_FLASH_ACCESS_AREA_BASE              (0xFFA20000u)
    #define R_BWCBUF_G3K_ADD                            (0xFFBC0700u)
    #define R_BWCBUF_G3KH_ADD                           (0xFFC5B000u)
#endif

#define R_FPSYS_REGADD_AUTH_BASE                        (0xFFA08000u)
#define R_FPSYS_REGADD_AUTH_ID0_U32                     (R_FPSYS_REGADD_AUTH_BASE + 0x000u)
#define R_FPSYS_REGADD_AUTH_ID1_U32                     (R_FPSYS_REGADD_AUTH_BASE + 0x004u)
#define R_FPSYS_REGADD_AUTH_ID2_U32                     (R_FPSYS_REGADD_AUTH_BASE + 0x008u)
#define R_FPSYS_REGADD_AUTH_ID3_U32                     (R_FPSYS_REGADD_AUTH_BASE + 0x00cu)
#define R_FPSYS_REGADD_AUTH_STATUS_U32                  (R_FPSYS_REGADD_AUTH_BASE + 0x010u)
#define R_FPSYS_REGBIT_AUTH_STATUS_LOCKED               (0x00000001u)

/* Register area */
#define R_FPSYS_REGADD_FPMON_U8                         (R_FPSYS_REG_AREA_BASE)
#define R_FPSYS_REGBIT_SFWE                             (0x80u)

#define R_FPSYS_REGADD_FCURAME_U16                      (R_FPSYS_REG_AREA_BASE + 0x054u)
#define R_FPSYS_REGBIT_FCURAME_KEY                      (0xC400u)
#define R_FPSYS_REGBIT_FCURAME_FCRME                    (0x0001u)          /* 1: on, 0: off        */
#define R_FPSYS_REGBIT_FCURAME_FRAMTRAN                 (0x0002u)          /* 0: RW, 1: High speed */
#define R_FPSYS_REGBIT_FCURAME_RESET                    (0x0000u)

#define R_FPSYS_REGADD_FSADR_U32                        (R_FPSYS_REG_AREA_BASE + 0x030u)
#define R_FPSYS_REGADD_FEADR_U32                        (R_FPSYS_REG_AREA_BASE + 0x034u)

#define R_FPSYS_REGADD_FASTAT_U8                        (R_FPSYS_REG_AREA_BASE + 0x010u)
#define R_FPSYS_REGBIT_FASTAT_CMDLK                     (0x10u)

#define R_FPSYS_REGADD_FAEINT_U8                        (R_FPSYS_REG_AREA_BASE + 0x014u)
#define R_FPSYS_REGVAL_FAEINT_DISABLE                   (0x88u)
#define R_FPSYS_REGVAL_FAEINT_ENABLE                    (0x99u)

#define R_FPSYS_REGADD_PCKAR_U16                        (R_FPSYS_REG_AREA_BASE + 0x0E4u)
#define R_FPSYS_REGBIT_PCKAR_KEY                        (0x1E00u)

#define R_FPSYS_REGADD_FMATSELC_U16                     (R_FPSYS_REG_AREA_BASE + 0x020u)
#define R_FPSYS_REGBIT_FMATSELC_KEY                     (0x3B00u)
#define R_FPSYS_REGBIT_FMATSELC_USS                     (0x0000u)
#define R_FPSYS_REGBIT_FMATSELC_EX1S                    (0x0001u)
#define R_FPSYS_REGBIT_FMATSELC_EX2S                    (0x0002u)

#define R_FPSYS_REGADD_FSTATR_U32                       (R_FPSYS_REG_AREA_BASE + 0x080u)
#define R_FPSYS_REGBIT_FSTATR_RESET                     (0x00000000u)
#define R_FPSYS_REGBIT_FSTATR_FCURAMERR                 (0x00000002u)
#define R_FPSYS_REGBIT_FSTATR_TBLDERR                   (0x00000008u)
#define R_FPSYS_REGBIT_FSTATR_CONFDERR                  (0x00000020u)
#define R_FPSYS_REGBIT_FSTATR_FHVEERR                   (0x00000040u)
#define R_FPSYS_REGBIT_FSTATR_FCUERR                    (0x00000080u)
#define R_FPSYS_REGBIT_FSTATR_PRGSPD                    (0x00000100u)
#define R_FPSYS_REGBIT_FSTATR_ERSSPD                    (0x00000200u)
#define R_FPSYS_REGBIT_FSTATR_DBFULL                    (0x00000400u)
#define R_FPSYS_REGBIT_FSTATR_SUSRDY                    (0x00000800u)
#define R_FPSYS_REGBIT_FSTATR_PRGERR                    (0x00001000u)
#define R_FPSYS_REGBIT_FSTATR_ERSERR                    (0x00002000u)
#define R_FPSYS_REGBIT_FSTATR_ILGERR                    (0x00004000u)
#define R_FPSYS_REGBIT_FSTATR_FRDY                      (0x00008000u)
#define R_FPSYS_REGBIT_FSTATR_OTPDERR                   (0x00020000u)

#define R_FPSYS_REGADD_FPESTAT_U16                      (R_FPSYS_REG_AREA_BASE + 0x0C0u)
#define R_FPSYS_REGBIT_FPESTAT_WRITELOCK                (0x0001u)
#define R_FPSYS_REGBIT_FPESTAT_ERASELOCK                (0x0011u)

#define R_FPSYS_REGADD_FPROTR_U16                       (R_FPSYS_REG_AREA_BASE + 0x088u)
#define R_FPSYS_REGBIT_FPROTR_RESET                     (0x0000u)
#define R_FPSYS_REGBIT_FPROTR_KEY                       (0x5500u)
#define R_FPSYS_REGBIT_FPROTR_OFF                       (0x0001u)
#define R_FPSYS_REGBIT_FPROTR_ON                        (0x0000u)

#define R_FPSYS_REGADD_FLKSTAT_U8                       (R_FPSYS_REG_AREA_BASE + 0x090u)

#define R_FPSYS_REGADD_FENTRYR_U16                      (R_FPSYS_REG_AREA_BASE + 0x084u)
#define R_FPSYS_REGBIT_FENTRY_FENTRYD                   (0x0080u)
#define R_FPSYS_REGBIT_FENTRY_FENTRYC                   (0x0001u)
#define R_FPSYS_REGBIT_FENTRY_KEY                       (0xAA00u)
#define R_FPSYS_REGBIT_FENTRY_OFF                       (0x0000u)
#define R_FPSYS_REGBIT_FENTRY_KEYMASK                   (0x00FFu)
#define R_FPSYS_MODE_DPE                                (R_FPSYS_REGBIT_FENTRY_FENTRYD)
#define R_FPSYS_MODE_CPE                                (R_FPSYS_REGBIT_FENTRY_FENTRYC)
#define R_FPSYS_MODE_USER                               (R_FPSYS_REGBIT_FENTRY_OFF)

/* FCURAM area */
#define R_FPSYS_FCURAM_AREA_SIZE                        (0x00001000u)

#ifdef R_FCL_INNER_NO_BFA_SWITCH
    #define R_FPSYS_FCURAM_AREA_RAM_ADD                 (0x01037000u)
    #define R_FPSYS_EXTRA3_AREA_BASE                    (0x01030000u)
#else
    #define R_FPSYS_FCURAM_AREA_RAM_ADD                 (0x00017000u)
    #define R_FPSYS_EXTRA3_AREA_BASE                    (0x00010000u)
#endif

#define R_FPSYS_FCURAM_AREA_COPY_SIZE                   (R_FPSYS_FCURAM_AREA_SIZE / 8u)

#define R_FPSYS_FCURAM_AREA_CHKSUM_END                  (R_FPSYS_FCURAM_AREA_RAM_ADD + 0x0FF8u)
#define R_FPSYS_FCURAM_AREA_CHKSUM                      (R_FPSYS_FCURAM_AREA_RAM_ADD + 0x0FFCu)

/* Flash access area */
#define R_FPSYS_FLASH_ACCESS_CMD_ERASE                  (0x20u)            /* FCU erase command              */
#define R_FPSYS_FLASH_ACCESS_CMD_CLEARSTAT              (0x50u)            /* FCU clear status reg. command  */
#define R_FPSYS_FLASH_ACCESS_CMD_FORCED_STOP            (0xB3u)            /* Stop and reset FCU             */
#define R_FPSYS_FLASH_ACCESS_CMD_WRITE                  (0xE8u)            /* FCU write command              */
#define R_FPSYS_FLASH_ACCESS_CMD_PRG_CFG                (0x40u)            /* FCU program config command     */
#define R_FPSYS_FLASH_ACCESS_CMD_PRG_CFG_OTP            (0x45u)            /* FCU program config command     */
#define R_FPSYS_FLASH_ACCESS_CMD_READ_LOCKBIT           (0x71u)            /* FCU read lock bit setting      */
#define R_FPSYS_FLASH_ACCESS_CMD_WRITE_LOCKBIT          (0x77u)            /* FCU write lock bit setting     */
#define R_FPSYS_FLASH_ACCESS_CMD_SUSPEND                (0xB0u)            /* Suspended a FCU command        */
#define R_FPSYS_FLASH_ACCESS_CMD_RESUME                 (0xD0u)            /* FCU resume a suspended command */
#define R_FPSYS_FLASH_ACCESS_CMD_EXE                    (0xD0u)            /* FCU start execution            */

#define R_FPSYS_CONFIG_AREA_BASE                        (0xFF300000u)
#define R_FPSYS_CONFIG_AREA_SEC_FLAGS                   (R_FPSYS_CONFIG_AREA_BASE + 0x40u)    /* Security flags */
#define R_FPSYS_CONFIG_AREA_OCD                         (R_FPSYS_CONFIG_AREA_BASE + 0x50u)    /* OCD ID         */
#define R_FPSYS_CONFIG_AREA_RESET                       (R_FPSYS_CONFIG_AREA_BASE + 0x60u)    /* Reset vector   */
#define R_FPSYS_CONFIG_AREA_OPTIONBYTES                 (R_FPSYS_CONFIG_AREA_BASE + 0x70u)    /* Option bytes   */

#define R_FPSYS_CONFIG_OTP                              (0xFF380040u)
#define R_FPSYS_CONFIG_OTP_CF1_OFFSET                   (R_FPSYS_CONFIG_OTP + 0x00u)
#define R_FPSYS_CONFIG_OTP_CF2_OFFSET                   (R_FPSYS_CONFIG_OTP + 0x20u)
#define R_FPSYS_CONFIG_OTP_UB_OFFSET                    (R_FPSYS_CONFIG_OTP + 0x50u)

#define R_FPSYS_EXTRA3_AREA_SCDS_U32                    (R_FPSYS_EXTRA3_AREA_BASE + 0x270u)
#define R_FPSYS_EXTRA3_AREA_PRDSEL1                     (0xC0u)
#define R_FPSYS_EXTRA3_AREA_PRDSEL2                     (0xC4u)
#define R_FPSYS_EXTRA3_AREA_PRDSEL4                     (0xCCu)
#define R_FPSYS_EXTRA3_AREA_PRDNAME1                    (0xD0u)
#define R_FPSYS_EXTRA3_AREA_PRDNAME2                    (0xD4u)
#define R_FPSYS_EXTRA3_AREA_PRDNAME3                    (0xD8u)
#define R_FPSYS_EXTRA3_AREA_PRDNAME4                    (0xDCu)
#define R_FPSYS_EXTRA3_AREA_FPVER_U08                   (R_FPSYS_EXTRA3_AREA_BASE + 0x219u)
#define R_FPSYS_FPVER_03                                (0x03u)
#define R_FPSYS_FPVER_04                                (0x04u)

#define R_FPSYS_EXTRA3_AREA_UB_FPVER04_U08              (R_FPSYS_EXTRA3_AREA_BASE + 0x21Eu)
#define R_FPSYS_EXTRA3_AREA_MAXHCLK_U32                 (R_FPSYS_EXTRA3_AREA_BASE + 0x234u)
#define R_FPSYS_EXTRA3_AREA_MINHCLK_U32                 (R_FPSYS_EXTRA3_AREA_BASE + 0x238u)
#define R_FPSYS_EXTRA3_AREA_PCLKDIV1_FPVER03_U32        (R_FPSYS_EXTRA3_AREA_BASE + 0x264u)
#define R_FPSYS_EXTRA3_AREA_PCLK_FPVER03_U32            (R_FPSYS_EXTRA3_AREA_BASE + 0x274u)
#define R_FPSYS_EXTRA3_AREA_PCLKDIV1_FPVER04_U32        (R_FPSYS_EXTRA3_AREA_BASE + 0x265u)
#define R_FPSYS_EXTRA3_AREA_PCLK_FPVER04_U32            (R_FPSYS_EXTRA3_AREA_BASE + 0x278u)

#define R_FPSYS_PRDNAME_E1x_012x                        (0x00323130u)
#define R_FPSYS_PRDNAME_E1x_01Zx                        (0x005A3130u)


#define R_RDPR_FLAG_MASK                                (0x80000000u)
#define R_WRPR_FLAG_MASK                                (0x40000000u)
#define R_SEPR_FLAG_MASK                                (0x20000000u)
#define R_SPD_FLAG_MASK                                 (0x08000000u)
#define R_SPIE_FLAG_MASK                                (0x01000000u)
#define R_RDPR_FLAG_SHIFT                               (0x1Fu)
#define R_WRPR_FLAG_SHIFT                               (0x1Eu)
#define R_SEPR_FLAG_SHIFT                               (0x1Du)
#define R_SPD_FLAG_SHIFT                                (0x1Bu)
#define R_SPIE_FLAG_SHIFT                               (0x18u)

#define R_FLI_BLOCK_SIZE                                (0x8000u)   /* FLI Flash block size 32kByte                   */
#define R_FLI_SMALL_BLOCK_SIZE                          (0x2000u)   /* FLI Flash block size 8kByte                    */
#define R_FLI_BLOCK_TRANSFERSIZE                        (0x80u)     /* FLI Flash transfersize for programming         */
#define R_FLI_BLOCK_TRANSFERSIZE_PART                   (0x40u)     /* FLI Flash transfersize for programming \n
                                                                       (first half)                                   */
#define R_PRG_CFG_TRANSFERSIZE                          (0x08u)     /* Transfersize for programming new configuration */
#define R_PRG_CFG_TRANSFERSIZE_OTP                      (R_PRG_CFG_TRANSFERSIZE)
#define R_PRG_CFG_TRANSFERSIZE_OPB                      (2u * R_PRG_CFG_TRANSFERSIZE)
#define R_PRG_CFG_TRANSFERSIZE_OCD                      (R_PRG_CFG_TRANSFERSIZE)
#define R_PRG_CFG_TRANSFERSIZE_RESET                    (R_PRG_CFG_TRANSFERSIZE)

#define R_FLI_USERBOOT_AREA_START                       (0x01000000u)
#define R_FLI_USERBOOT_AREA_END                         (0x01007FFFu)
#define R_FCL_USERBOOT_AREA_SELECT                      (0x80000000u)
#define R_FLI_USERBOOT_AREA_MASK                        (0x7FFFFFFFu)
#define R_FLI_CFLASH1_START                             (0x00000000u)
#if ((defined R_FCL_CFL2_START_ADDR_2048K) && (defined R_FCL_CFL2_START_ADDR_4096K))
    #error "r_fcl_env.h: Invalid define for R_FCL_CFL2_START_ADDR_*"
#elif (defined R_FCL_CFL2_START_ADDR_2048K)
    #define R_FLI_CFLASH2_START                             (0x00200000u) /* 2nd Code Flash Bank begins at 0x00200000 */
#elif (defined R_FCL_CFL2_START_ADDR_4096K)
    #define R_FLI_CFLASH2_START                             (0x00400000u) /* 2nd Code Flash Bank begins at 0x00400000 */
#else
    #define R_FLI_CFLASH2_START                             (0x00800000u) /* 2nd Code Flash Bank begins at 0x00800000 */
#endif

#define R_FCL_FCU_FMIN                                  (0x0004u)
#define R_FCL_FCU_FMAX                                  (0x0064u)

#define R_FLI_MIN_WRITESIZE                             (2u * R_FLI_BLOCK_TRANSFERSIZE)

#define R_PID_CORE_MASK                                 (0x000000E0u)
#define R_PID_CORE_G3K                                  (0x00000020u)
#define R_PID_CORE_G3KH                                 (0x000000a0u)


/**********************************************************************************************************************
Exported global variables
**********************************************************************************************************************/


/**********************************************************************************************************************
Exported global functions (to be accessed by other files)
**********************************************************************************************************************/


#endif /* end of R_FCL_ENV_H */

