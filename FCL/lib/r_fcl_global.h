#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/*********************************************************************************************************************
* Library       : Code Flash Access Library for Renesas RH850 devices, based on the RV40 Flash technology
*
* File Name     : $Source: r_fcl_global.h $
* Lib. Version  : $RH850_FCL_LIB_VERSION_T01: V2.13 $
* Mod. Revision : $Revision: 1.53 $
* Mod. Date     : $Date: 2019/04/18 13:13:47JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : Flash programming global typedefs, functions and variables
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

#ifndef R_FCL_GLOBAL_H
#define R_FCL_GLOBAL_H

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/
/* Library version string */
#define R_FCL_VERSION_STRING     "SH850T01xxxxxxV213"

/* Static functions definition */
#if !(defined FCL_TEST_PATCH)
#ifndef R_FCL_STATIC
    #define R_FCL_STATIC        static
#endif
#else
    #define R_FCL_STATIC
#endif

/* Reset value of request pointers */
#define R_FCL_REQUEST_POINTER_UNDEFINED         (r_fcl_request_t *)(0x00000000u)
#define R_FCL_DESCRIPTOR_POINTER_UNDEFINED      (r_fcl_descriptor_t *)(0x00000000u)

/* ----- Unique timeout value is 40us ----- */
/* Clear cache register check loop timeout */
/* clocks per timeout loop */
#define R_FCL_TIMEOUT_CC_CLK_P_LOOP             (3u)
/* timeout loop_count: (40us * f_MHz) / (clk per loop)   ---  but rounding up to be considered */
#define R_FCL_TIMEOUT_CC                        (   ( (40u * (uint32_t)(g_fcl_data_str.RTCfg_pstr->frequencyCpuMHz_u16)) \
                                                  + (R_FCL_TIMEOUT_CC_CLK_P_LOOP - 1u) ) / R_FCL_TIMEOUT_CC_CLK_P_LOOP )

/* IO register check loop timeout */
/* clocks per timeout loop */
#define R_FCL_TIMEOUT_CHKREG_CLK_P_LOOP         (19u)
/* timeout loop_count: (40us * f_MHz) / (clk per loop)   ---  but rounding up to be considered */
#define R_FCL_TIMEOUT_CHKREG                    (   ( (40u * (uint32_t)(g_fcl_data_str.RTCfg_pstr->frequencyCpuMHz_u16)) \
                                                  + (R_FCL_TIMEOUT_CHKREG_CLK_P_LOOP - 1u) ) / R_FCL_TIMEOUT_CHKREG_CLK_P_LOOP )
 
/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/
/**
   @enum r_fcl_flag_t
   standard FCL internal flag definition
*/
typedef enum R_FCL_FLAG_T
{
    R_FCL_FALSE                 = 0x00u,                 /**< Flag false */
    R_FCL_TRUE                  = 0x01u                  /**< Flag true  */
} r_fcl_flag_t;

/**
   @enum r_fcl_mode_t
   FCL internal mode for Code Flash access
*/
typedef enum R_FCL_MODE_T
{
    R_FCL_READ_MODE             = 0x00u,                 /**< Code Flash Read Mode */
    R_FCL_WRITE_MODE            = 0x01u,                 /**< Code Flash P/E Mode  */
    R_FCL_CFG_WRITE_MODE        = 0x80u                  /**< Config Area P/E Mode */
} r_fcl_mode_t;

/**
   @enum r_fcl_int_status_t
   definition of the FCL operational status
*/
typedef enum R_FCL_INT_STATUS_T
{
    R_FCL_ISTAT_NOINIT,                                  /**< Pattern for uninitialized library (0x00000000) */
    R_FCL_ISTAT_INIT,                                    /**< Pattern for initialized library                */
    R_FCL_ISTAT_PREPARED,                                /**< Pattern for prepared/idle library              */
    R_FCL_ISTAT_BUSY,                                    /**< Pattern for a busy library                     */
    R_FCL_ISTAT_SUSPENDED                                /**< Pattern for a suspended library                */
} r_fcl_int_status_t;

/**
   @enum r_fcl_op_status_t
   definition of the internal operation status
*/
typedef enum R_FCL_OP_STATUS_T
{
    R_FCL_OP_IDLE_STATE,                                 /**< Pattern for idle - no operation ongoing                */
    R_FCL_OP_PREPARE_STATE,                              /**< Pattern for preparing an operation to start:
                                                              switch mode performed for normal operations and
                                                              for prepare environment the get the device information
                                                              and start the FCURAM copy process                      */
    R_FCL_PREP_COPY_FW_STATE,                            /**< Pattern for prepare env cmd to copy the FCURAM         */
    R_FCL_PREP_RESET_FCU_STATE,                          /**< Pattern for prepare env cmd to clear ECC errors        */
    R_FCL_PREP_FCURAM_CHKSUM_STATE,                      /**< Pattern for prepare env cmd to perfom a checksum
                                                              verification                                           */
    R_FCL_PREP_ID_CHECK_FREQ_SET_STATE,                  /**< Pattern for prepare env cmd to perform ID verification
                                                              and FCU frequency calculation and setting              */
    R_FCL_BUSY_CHECK_PE_MODE_SWITCH_STATE,               /**< Pattern for multi operations to check if mode is
                                                              switched to PE                                         */
    R_FCL_BUSY_START_OP_STATE,                           /**< Pattern for multi operations to start the physical
                                                              operation                                              */
    R_FCL_BUSY_START_WRITE_OP_STATE,                     /**< Pattern for multi operations to start the physical
                                                              write operation                                        */
    R_FCL_BUSY_CHECK_RESULT_STATE,                       /**< Pattern for multi operations to check the operation
                                                              status                                                 */
    R_FCL_BUSY_SWITCH_READ_MODE_STATE,                   /**< Pattern for multi operations to switch to read mode    */
    R_FCL_BUSY_CHECK_READ_MODE_SWITCH_STATE,             /**< Pattern for multi operations to check if the mode is
                                                              switched to read mode                                  */
    R_FCL_OP_FINISHED_STATE,                             /**< Pattern for a finished operation                       */
    R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE,           /**< Pattern for internal error during op. execution
                                                              - switch to read mode                                  */
    R_FCL_OP_INTERNAL_ERROR_CHECK_READ_STATE,            /**< Pattern for internal error during op. execution
                                                              - check switch to read mode                            */
    R_FCL_OP_INTERNAL_ERROR_STATE                        /**< Pattern for internal error during op. execution        */
} r_fcl_op_status_t;

/**
   @struct r_fcl_multiOp_t
   structure containing the additional data required for operations
*/
typedef struct R_FCL_MULTIOP_T
{
    uint32_t            opAddr_u32;                      /**< current source address                  */
    uint32_t            buffAddr_u32;                    /**< buffer address for input data or result */
    uint16_t            opCnt_u16;                       /**< remaining blocks to erase               */
    r_fcl_op_status_t   opStatus_enu;                    /**< current operation status                */
    r_fcl_status_t      opResult_enu;                    /**< current operation result                */
} r_fcl_multiOp_t;

/**
   @struct r_fcl_suspendInfo_t
   structure containing the read out device settings during start-up
*/
typedef struct R_FCL_SUSPENDINFO_T
{
    r_fcl_request_t *   reqSuspend_pstr;                 /**< request structure pointer for current operation */
    r_fcl_multiOp_t     opSuspend_str;                   /**< operation variables                             */
    r_fcl_flag_t        spdSpdRequest_enu;               /**< suspend req. indication to the handler          */
    r_fcl_flag_t        spdResRequest_enu;               /**< resume req. indication to the handler           */
    r_fcl_flag_t        flagSuspended_enu;               /**< resume req. indication to the handler           */
    uint16_t            matSel_u16;                      /**< resume value for CF area selection              */
} r_fcl_suspendInfo_t;

/**
   @struct r_fcl_deviceInfo_t
   structure containing the read out device settings during start-up
*/
typedef struct R_FCL_DEVICEINFO_T
{
    uint32_t            cf1BlkCnt32kB_u32;
    uint32_t            cf2BlkCnt32kB_u32;
    uint32_t            usrBlkCnt32kB_u32;
    uint32_t            secureData_u32;
    uint16_t            fMin_u16;
    uint16_t            fMax_u16;
    uint16_t            fPClk_u16;
    uint16_t            blkCnt_u16;
    uint16_t            matSel_u16;
    uint8_t             deviceName_au08[16];
    uint8_t             cf1BlkCnt8kB_u8;
    uint8_t             fVersion_u8;
    uint8_t             fDivider_u8;
    r_fcl_flag_t        emulationMode_enu;
    r_fcl_flag_t        enableLockBits_enu;
    r_fcl_flag_t        accessExtendedUserArea_enu;
} r_fcl_deviceInfo_t;

/**
    this typedefs describe function pointers used to execute functions from RAM
*/
typedef void (* r_fcl_fctPtr_t)(void);

/**
   @struct r_fcl_data_t
   this structure contains all FCL internal data
*/
typedef struct R_FCL_DATA_T
{
    const r_fcl_descriptor_t *  RTCfg_pstr;              /**< pointer to descriptor structure,
                                                              initialized by R_FCL_ISTAT_INIT                 */
    r_fcl_request_t *           reqInt_pstr;             /**< request structure pointer for current operation */
    r_fcl_multiOp_t             opVar_str;               /**< operation variables                             */
    r_fcl_suspendInfo_t         suspendInfo_str;         /**< storage variable for suspend                    */
    r_fcl_int_status_t          intStatus_enu;           /**< FCL status variable                             */
    r_fcl_mode_t                flashMode_enu;           /**< FCL Flash access mode variable                  */
    r_fcl_deviceInfo_t          deviceInfo_str;          /**< Device information                              */
    r_fcl_fctPtr_t              executionPtr_p;          /**< Pointer to call the function from RAM           */
    r_fcl_flag_t                waitAckModeSwitch_enu;   /**< Used for suspend/resume mode switch mechanism   */
    r_fcl_flag_t                cancelReq_enu;           /**< Cancel request indication for the handler       */
    uint32_t                    fcuramAddr_u32;          /**< FCURAM address for copy function                */
    uint32_t                    firmAddr_u32;            /**< Firmware address for copy function              */
    uint8_t                     intBuf_au08[16];         /**< Internal buffer                                 */
    #if R_FCL_COMMAND_EXECUTION_MODE == R_FCL_HANDLER_CALL_INTERNAL
            r_fcl_fctPtr_t      handlerPtr_p;            /**< Pointer to call the handler from RAM            */
    #endif
} r_fcl_data_t;

/**********************************************************************************************************************
Exported global variables
**********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs startdata
    #pragma ghs section bss = ".R_FCL_DATA"
    #define R_FCL_NOINIT
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma segment = "R_FCL_DATA"
    #pragma location = "R_FCL_DATA"
    #define R_FCL_NOINIT __no_init
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #pragma section r0_disp32 "R_FCL_DATA"
    #define R_FCL_NOINIT
#endif

extern R_FCL_NOINIT r_fcl_data_t   g_fcl_data_str;       /**< fcl global variable structure */

#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section bss = default
    #pragma ghs enddata
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    /* location only for one function, so no default required */
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #pragma section default
#endif

/**********************************************************************************************************************
Exported global functions (to be accessed by other files)
**********************************************************************************************************************/
void            R_FCL_I_write_memory_u08          (const uint32_t addr_u32, const uint8_t val_u08);
void            R_FCL_Fct_CallFromRAM             (void);
void            R_FCL_FCUFct_PrepareEnvironment   (void);
void            R_FCL_FCUFct_HandleMultiOperation (void);
void            R_FCL_FCUFct_GetConfigArea        (void);
void            R_FCL_FCUFct_SetConfigArea        (void);
void            R_FCL_FCUFct_HandleLockBit        (void);
void            R_FCL_FCUFct_LockBitMode          (uint16_t mode_u16);
void            R_FCL_FCUFct_GetOTPBit            (void);
void            R_FCL_FCUFct_SetOTPBit            (void);
void            R_FCL_FCUFct_GetSecurityFlag      (void);
void            R_FCL_FCUFct_GetDeviceName        (void);
void            R_FCL_FCUFct_GetBlockEndAddr      (void);
void            R_FCL_FCUFct_GetBlockCnt          (void);
void            R_FCL_FCUFct_Erase                (void);
void            R_FCL_FCUFct_Write                (void);
void            R_FCL_Fct_CodeRam_CalcRange       (uint32_t * secStart_pu32, uint32_t * secSize_pu32);
void            R_FCL_Fct_CodeRomRam_CalcRange    (uint32_t * secStart_pu32, uint32_t * secSize_pu32);
void            R_FCL_Fct_CodeUsr_CalcRange       (uint32_t * secStart_pu32, uint32_t * secSize_pu32);
void            R_FCL_Fct_CodeUsrInt_CalcRange    (uint32_t * secStart_pu32, uint32_t * secSize_pu32);
void            R_FCL_Fct_CodeExProt_CalcRange    (uint32_t * secStart_pu32, uint32_t * secSize_pu32);
void            R_FCL_FCUFct_Switch_BFlash        (uint8_t mode_u08);
uint32_t        R_FCL_FCUFct_ChkReg               (uint32_t regAdd, uint32_t valMask, uint32_t val, uint32_t to);
uint32_t        R_FCL_FCUFct_Clear_Cache_Asm      (uint32_t add_u32, uint32_t timeout_u32);
uint32_t        R_FCL_Fct_PrgOffset               (void);
uint32_t        R_FCL_Fct_Copy_Code               (uint32_t src_u32, uint32_t dest_u32, uint32_t cnt_u32);
uint32_t        R_FCL_Fct_Get_PID_Asm             (void);
r_fcl_status_t  R_FCL_FCUFct_SwitchMode_Start     (r_fcl_mode_t mode_enu);
r_fcl_status_t  R_FCL_FCUFct_SwitchMode_Check     (void);
r_fcl_flag_t    R_FCL_FCUFct_CheckMode            (void);
#if R_FCL_COMMAND_EXECUTION_MODE == R_FCL_HANDLER_CALL_USER
    void        R_FCL_FCUFct_HandleResume         (void);
#endif

#if (defined FCL_TEST_PATCH)
typedef struct TESTPATCH_T
{
    uint32_t status_patch_active;
    uint32_t status_patch_value;
} TestPatch_t;

extern TestPatch_t TestPatch;

uint32_t test_cb_read_u32 (const uint32_t addr);
uint16_t test_cb_read_u16 (const uint32_t addr);
uint8_t test_cb_read_u8   (const uint32_t addr);

void test_cb_write_u32    (const uint32_t addr, const uint32_t val);
void test_cb_write_u16    (const uint32_t addr, const uint16_t val);
void test_cb_write_u8     (const uint32_t addr, const uint8_t val);
#endif

#endif /* end of R_FCL_GLOBAL_H */

