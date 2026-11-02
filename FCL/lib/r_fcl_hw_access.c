#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/**********************************************************************************************************************
* Library       : Code Flash Access Library for Renesas RH850 devices, based on the RV40 Flash technology
*
* File Name     : $Source: r_fcl_hw_access.c $
* Lib. Version  : $RH850_FCL_LIB_VERSION_T01: V2.13 $
* Mod. Revision : $Revision: 1.93 $
* Mod. Date     : $Date: 2019/04/19 18:52:29JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : FCL hardware interface functions (c-code)
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

/* Moritz tool config */
/* @MRTZ_commentModel CommentBefore */
/* @MRTZ_beginBlock */

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

/*********************************************************************************************************************
 * MISRA Rule:   MISRA-C 2004 rule 8.10 (QAC message 1503)
 * Reason:       This function is not used within the library, it is only used by the user
 *               application.
 * Verification: This function is part of the library API and therefore essential on the one hand.
 *               On the other hand the QAC warning is correct, because this function is never
 *               called within the library. To solve this dillemma it is neccessary and suitable
 *               to disable the message.
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * MISRA Rule:   MISRA-C 2004 rule 11.3 (QAC message 0303)
 * Reason:       For effective embedded programming, integer to pointer conversions are used
 * Verification: The converted addresses are essential for complete code execution. Incorrect
 *               conversion would result in test fails.
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * MISRA Rule:   MISRA-C 2004 rule 11.3 (QAC message 0306)
 * Reason:       For effective embedded programming, integer to pointer conversions are used
 * Verification: The converted addresses are essential for complete code execution. Incorrect
 *               conversion would result in test fails.
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * MISRA Rule:   QAC message 2814
 * Reason:       Internal library functions used with pointers passed as arguments.
 * Verification: Code review, ensure that before each function call the pointers are initialized.
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * MISRA Rule:   MISRA-C 2004 rule 13.7 (QAC message 2991)
 * Reason:       If a particular macro switch(R_FCL_INNER_NO_BFA_SWITCH) is specified,
 *               the value of this controlling expression must always be set to "true".
 * Verification: By specifying the other macro switch, the value of the controlling expression
 *               does not become 'true'. But, on the other hand, the QAC warning is correct.
 *               To solve this dilemma it is necessary and suitable to disable the message.
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * MISRA Rule:   MISRA-C 2004 rule 13.7 (QAC message 2995)
 * Reason:       If a particular macro switch(R_FCL_INNER_NO_BFA_SWITCH) is specified,
 *               the result of this logical operation must always be set to "true"
 * Verification: By specifying the other macro switch, the result of this logical operation
 *               does not become 'true'. But, on the other hand, the QAC warning is correct.
 *               To solve this dilemma it is necessary and suitable to disable the message.
 *********************************************************************************************************************/

/**********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
**********************************************************************************************************************/
#include "r_typedefs.h"
#include "fcl_cfg.h"
#include "r_fcl_types.h"
#include "r_fcl_global.h"
#include "r_fcl_env.h"

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Exported global variables (to be accessed by other files)
**********************************************************************************************************************/
#if (defined FCL_TEST_PATCH)
    TestPatch_t TestPatch;
#endif


/**********************************************************************************************************************
Private global variables and functions
**********************************************************************************************************************/
R_FCL_STATIC uint8_t        R_FCL_I_read_memory_u08            (const uint32_t addr_u32);
R_FCL_STATIC uint16_t       R_FCL_I_read_memory_u16            (const uint32_t addr_u32);
R_FCL_STATIC uint32_t       R_FCL_I_read_memory_u32            (const uint32_t addr_u32);
R_FCL_STATIC void           R_FCL_I_write_memory_u16           (const uint32_t addr_u32, const uint16_t val_u16);
R_FCL_STATIC void           R_FCL_I_write_memory_u32           (const uint32_t addr_u32, const uint32_t val_u32);
R_FCL_STATIC void           R_FCL_I_write_unaligned_memory_u32 (const uint32_t addr_u32, const uint32_t val_u32);
R_FCL_STATIC uint32_t       R_FCL_I_GetCPUCorePid              (void);
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_Clear_Cache           (void);
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_Chk_DBFull            (void);
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_ForcedStop            (void);
R_FCL_STATIC void           R_FCL_FCUFct_RestoreLastState      (void);
R_FCL_STATIC void           R_FCL_FCUFct_ClearStatus           (void);
R_FCL_STATIC void           R_FCL_FCUFct_GetDeviceInfo         (void);
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_VerifyID              (void);
#ifdef R_FCL_INNER_NO_FCU_COPY
#else
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_VerifyChecksum        (void);
#endif
R_FCL_STATIC void           R_FCL_FCUFct_SetFrequency          (void);
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_GetResult             (r_fcl_flag_t cmdSpdExec_enu);
R_FCL_STATIC r_fcl_flag_t   R_FCL_FCUFct_ChkSuspended          (void);
R_FCL_STATIC void           R_FCL_FCUFct_HandleMultiOperation_IntErr (r_fcl_op_status_t opState);
R_FCL_STATIC void           R_FCL_FCUFct_PrepareEnvironment_IntErr (r_fcl_op_status_t opState);

/**********************************************************************************************************************
 * Function name: R_FCL_I_read_memory_u32
 *********************************************************************************************************************/
/**
 * Function to read a 32-bit IO register or memory
 *
 * @param[in]     addr_u32       source address
 * @return        32-bit register contents
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC uint32_t R_FCL_I_read_memory_u32 (const uint32_t addr_u32)
{
    #if (defined FCL_TEST_PATCH)
        return (test_cb_read_u32 (addr_u32));
    #else
        return ( *( (volatile uint32_t *)(addr_u32) ) );                                          /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_read_memory_u32
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_read_memory_u16
 *********************************************************************************************************************/
/**
 * Function to read a 16-bit IO register or memory
 *
 * @param[in]     addr_u32       source address
 * @return        16-bit register contents
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC uint16_t R_FCL_I_read_memory_u16 (const uint32_t addr_u32)
{
    #if (defined FCL_TEST_PATCH)
        return (test_cb_read_u16 (addr_u32));
    #else
        return ( *( (volatile uint16_t *)(addr_u32) ) );                                          /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_read_memory_u16
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_read_memory_u08
 *********************************************************************************************************************/
/**
 * Function to read a 8-bit IO register or memory
 *
 * @param[in]     addr_u32       source address
 * @return        8-bit register contents
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC uint8_t R_FCL_I_read_memory_u08 (const uint32_t addr_u32)
{
    #if (defined FCL_TEST_PATCH)
        return (test_cb_read_u8 (addr_u32));
    #else
        return ( *( (volatile uint8_t *)(addr_u32) ) );                                           /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_read_memory_u08
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_write_unaligned_memory_u32
 *********************************************************************************************************************/
/**
 * Function to write a 32-bit data to memory
 *
 * @param[in]     addr_u32       unaligned write destination address
 * @param[in]     val_u32        32-bit write data
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_I_write_unaligned_memory_u32 (const uint32_t addr_u32, const uint32_t val_u32)
{
    R_FCL_I_write_memory_u08 (addr_u32,       (uint8_t)val_u32);
    R_FCL_I_write_memory_u08 ((addr_u32 + 1u), (uint8_t)(val_u32 >> 8));
    R_FCL_I_write_memory_u08 ((addr_u32 + 2u), (uint8_t)(val_u32 >> 16));
    R_FCL_I_write_memory_u08 ((addr_u32 + 3u), (uint8_t)(val_u32 >> 24));
}
/**********************************************************************************************************************
 End of function R_FCL_I_write_unaligned_memory_u32
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_write_memory_u32
 *********************************************************************************************************************/
/**
 * Function to write a 32-bit IO register or memory
 *
 * @param[in]     addr_u32       write destination address
 * @param[in]     val_u32        32-bit write data
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_I_write_memory_u32 (const uint32_t addr_u32, const uint32_t val_u32)
{
    #if (defined FCL_TEST_PATCH)
        test_cb_write_u32 (addr_u32, val_u32);
    #else
        ( *( (volatile uint32_t *)(addr_u32) ) ) = val_u32;                                       /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_write_memory_u32
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_write_memory_u16
 *********************************************************************************************************************/
/**
 * Function to write a 16-bit IO register or memory
 *
 * @param[in]     addr_u32       write destination address
 * @param[in]     val_u16        16-bit write data
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_I_write_memory_u16 (const uint32_t addr_u32, const uint16_t val_u16)
{
    #if (defined FCL_TEST_PATCH)
        test_cb_write_u16 (addr_u32, val_u16);
    #else
        ( *( (volatile uint16_t *)(addr_u32) ) ) = val_u16;                                       /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_write_memory_u16
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_write_memory_u08
 *********************************************************************************************************************/
/**
 * Function to write a 8-bit IO register or memory
 *
 * @param[in]     addr_u32       write destination address
 * @param[in]     val_u08        8-bit write data
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
void R_FCL_I_write_memory_u08 (const uint32_t addr_u32, const uint8_t val_u08)
{
    #if (defined FCL_TEST_PATCH)
        test_cb_write_u8 (addr_u32, val_u08);
    #else
        ( *( (volatile uint8_t *)(addr_u32) ) ) = val_u08;                                        /* PRQA S 0303,2814 */
    #endif
}
/**********************************************************************************************************************
 End of function R_FCL_I_write_memory_u08
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_I_GetCPUCorePid
 *********************************************************************************************************************/
/**
 * Function read CPU core PID
 *
 * @param       -
 * @return      Bits 5 - 7 of PID Sysyem register
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC uint32_t R_FCL_I_GetCPUCorePid (void)
{
    uint32_t pid;

    /* regID 6, selID 1 */
    pid = R_FCL_Fct_Get_PID_Asm();

    pid &= R_PID_CORE_MASK;    /* Extract bits 5~7 */

    return (pid);
}
/**********************************************************************************************************************
 End of function R_FCL_I_GetCPUCorePid
 *********************************************************************************************************************/

 
/**********************************************************************************************************************
 * Function name: R_FCL_Fct_CallFromRAM
 *********************************************************************************************************************/
/**
 * This function calls the function to be executed from RAM
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_Fct_CallFromRAM (void)                                                                  /* PRQA S 1505 */
{
    if (R_FCL_NULL != g_fcl_data_str.executionPtr_p)
    {
        (*g_fcl_data_str.executionPtr_p)();
    }
    else
    {
        R_FCL_FCUFct_GetConfigArea ();
    }
}
/**********************************************************************************************************************
 End of function R_FCL_Fct_CallFromRAM
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_VerifyID
 *********************************************************************************************************************/
/**
 * Function to verify the given ID against the stored reference ID \n
 * The reference ID is tranferred to the check unit and the comparison result is returned to the
 * calling function. Depending on the comparison result, all further Flash operations will be
 * accepted or rejected by the hardware.
 *
 * @param[in]     -
 * @return
 * @li            R_FCL_OK:              ID check passed successful
 * @li            R_FCL_ERR_PROTECTION:  ID mismatch
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_VerifyID ( void )
{
    uint32_t       result;
    r_fcl_status_t ret;

    ret = R_FCL_OK;

    R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_AUTH_ID0_U32, (g_fcl_data_str.RTCfg_pstr->id_au32[0]));  /* PRQA S 2814 */
    R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_AUTH_ID1_U32, (g_fcl_data_str.RTCfg_pstr->id_au32[1]));
    R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_AUTH_ID2_U32, (g_fcl_data_str.RTCfg_pstr->id_au32[2]));
    R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_AUTH_ID3_U32, (g_fcl_data_str.RTCfg_pstr->id_au32[3]));

    /* Dummy read because some devices have some cycles delay before verifying the ID */
    (void) R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_AUTH_ID3_U32);

    /* Read verification result */
    result = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_AUTH_STATUS_U32);
    if (R_FPSYS_REGBIT_AUTH_STATUS_LOCKED == result)
    {
        ret = R_FCL_ERR_PROTECTION;
    }

    return (ret);
} /* R_FCL_FCUFct_VerifyID */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_VerifyID
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * Function name:  R_FCL_FCUFct_Clear_Cache
 *********************************************************************************************************************/
/**
 * Use intrinsic functions to evaluate BWC address from PID register
 *
 * @param      -
 * @return     R_FCL_OK             Normal case
 *             R_FCL_ERR_INTERNAL   Timeout during Clear Cache
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_Clear_Cache (void)
{
    uint32_t bwc;
    uint32_t pid;
    uint32_t res;
    r_fcl_status_t ret;

    pid = R_FCL_I_GetCPUCorePid ();

    /* G3K core */
    if (R_PID_CORE_G3K == pid)
    {
        bwc = R_BWCBUF_G3K_ADD;
    }

    /* G3KH core */
    else if (R_PID_CORE_G3KH == pid)
    {
        bwc = R_BWCBUF_G3KH_ADD;
    }

    /* No G3K code --> no BWC buffer */
    else
    {
        bwc = 0x00000000u;
    }

    ret = R_FCL_OK;
    res = R_FCL_FCUFct_Clear_Cache_Asm (bwc, R_FCL_TIMEOUT_CC);
    if (0x00000000u != res)
    {
        ret = R_FCL_ERR_INTERNAL;
    }

    return (ret);
}
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_Clear_Cache
 *********************************************************************************************************************/

/*********************************************************************************************************************
 * Function name:  R_FCL_FCUFct_Chk_DBFull
 *********************************************************************************************************************/
/**
 * Check for DBFull with timeout supervision
 *
 * @param      -
 * @return     R_FCL_OK             Normal case
 *             R_FCL_ERR_INTERNAL   Timeout during Clear Cache
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_Chk_DBFull (void)
{
    r_fcl_status_t ret;
    uint32_t res;
    
    /* Register check loop */
    res = R_FCL_FCUFct_ChkReg (R_FPSYS_REGADD_FSTATR_U32,
                               R_FPSYS_REGBIT_FSTATR_DBFULL,
                               0x00000000u,                /* Wait until the DBFULL bit is cleared */
                               R_FCL_TIMEOUT_CHKREG);


    /* Patch the check loop result: 0 --> check passed */
    if (0x00000000u == res)
    {
        ret = R_FCL_OK;
    }
    
    /* != 0 --> timeout in check loop */
    else
    {
        ret = R_FCL_ERR_INTERNAL;
    }

    return (ret);
}
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_Chk_DBFull
 *********************************************************************************************************************/
 

#ifdef R_FCL_INNER_NO_FCU_COPY
#else
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_VerifyChecksum
 *********************************************************************************************************************/
/**
 * Function to verify the FCURAM checksum \n
 *
 * @param[in,out] -
 * @return        R_FCL_OK: Checksum is OK
 *                R_FCL_ERR_INTERNAL: Checksum is Wrong or timeout during cache clear
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_VerifyChecksum  (void)
{
    uint32_t startAddr;
    uint32_t endAddr;
    uint32_t chkSum;
    uint32_t chkSumCmp;
    r_fcl_status_t ret;
    r_fcl_status_t ret1;

    /* Activate FCU RAM access again */
    R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FCURAME_U16, R_FPSYS_REGBIT_FCURAME_FCRME +
                              R_FPSYS_REGBIT_FCURAME_KEY);

    /* Activate BFlash */
    #ifdef R_FCL_INNER_NO_BFA_SWITCH
        ret = R_FCL_OK;
    #else
        R_FCL_FCUFct_Switch_BFlash (0x01u);
        ret = R_FCL_FCUFct_Clear_Cache ();
    #endif

    endAddr   = R_FCL_I_read_memory_u32 (R_FPSYS_FCURAM_AREA_CHKSUM_END);
    chkSumCmp = R_FCL_I_read_memory_u32 (R_FPSYS_FCURAM_AREA_CHKSUM);
    
    /* Deactivate BFlash */
    #ifdef R_FCL_INNER_NO_BFA_SWITCH
        ret1 = R_FCL_OK;
    #else
        R_FCL_FCUFct_Switch_BFlash (0x00u);
        ret1 = R_FCL_FCUFct_Clear_Cache ();
    #endif
    
    /* Return possible error of both times CC */
    if (R_FCL_OK == ret)                                                                         /* PRQA S 2991,2995 */
    {
        ret = ret1;
    }
    
    /* No timeout in Clear Cache */
    if (R_FCL_OK == ret)                                                                         /* PRQA S 2991,2995 */
    {
        /* Calculate and compare FCU RAM checksum */
        endAddr <<= 2;
        endAddr += R_FPSYS_FCURAM_AREA_BASE;
        chkSum  = 0x00000000u;
        for (startAddr = R_FPSYS_FCURAM_AREA_BASE; startAddr < endAddr; startAddr +=2u)
        {
            chkSum += (uint32_t)(R_FCL_I_read_memory_u16 (startAddr));
        }

        /* Deactivate FCU RAM access */
        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FCURAME_U16,
                                  R_FPSYS_REGBIT_FCURAME_RESET + R_FPSYS_REGBIT_FCURAME_KEY);

        /* Calculated checksum matches stored checksum */
        if (chkSum != chkSumCmp)
        {
            ret = R_FCL_ERR_INTERNAL;
        }
    } /* if (R_FCL_OK == ret) */
    
    return (ret);
} /* R_FCL_FCUFct_VerifyChecksum */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_VerifyChecksum
 *********************************************************************************************************************/
#endif

/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_SetFrequency
 *********************************************************************************************************************/
/**
 * Function to calculate and set the FACI frequency
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_SetFrequency  (void)
{
    uint16_t       ffcu;

    /* Firmware version >1 (version read value != 0xFF) */
    if (g_fcl_data_str.deviceInfo_str.fVersion_u8 != 0xffu)
    {
        ffcu = g_fcl_data_str.RTCfg_pstr->frequencyCpuMHz_u16;                             /* PRQA S 2814 */
       
        /* If you want to operate with low frequency(ffcu) less than fMin_u16, */
        /* please remove the lower limit frequency check from the following if statement */
        /* Remove the condition (g_fcl_data_str.deviceInfo_str.fMin_u16 > ffcu) */
        /* Note: If the frequency is too low, FCU may not work */
        /*       Please refer to the hardware manual for the target device */
        if ((g_fcl_data_str.deviceInfo_str.fMin_u16 > ffcu) ||
            (g_fcl_data_str.deviceInfo_str.fMax_u16 < ffcu))
        {
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_ERR_PARAMETER;
        }
        else
        {
        
            /* No FCU frequency calculation divider set */
            if (g_fcl_data_str.deviceInfo_str.fDivider_u8 == 0xFFu)
            {
                ffcu = g_fcl_data_str.deviceInfo_str.fPClk_u16;
            }
            else
            {
                if (g_fcl_data_str.deviceInfo_str.fDivider_u8 > 0u)
                {
                    /********************************************************************
                     * SAMPLE: Changing CKDIVMD
                     ********************************************************************/
                    /* If a division ratio for FCU clock is changed by Option Bytes, */
                    /* you should adjust the frequency for FCU. */
                    /* [Target Devices] */
                    /* - F1KM-S4 */
                    /* - F1KH-D8 */
                    /* When CKDIVMD (OPBT0 - Option Byte 0) is changed from 1 to 0, */
                    /* you should swap following formulas of 'ffcu'. */
                    #if 0
                    ffcu = ( (ffcu + (g_fcl_data_str.deviceInfo_str.fDivider_u8 / 2u)) - 1u) /
                            (g_fcl_data_str.deviceInfo_str.fDivider_u8 / 2u);
                    #else
                    ffcu = ( (ffcu + g_fcl_data_str.deviceInfo_str.fDivider_u8) - 1u) /
                            g_fcl_data_str.deviceInfo_str.fDivider_u8;
                    #endif
                    /********************************************************************/
                }
            }

            R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_PCKAR_U16, R_FPSYS_REGBIT_PCKAR_KEY + (uint16_t)ffcu);
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_OK;
        }
    }
    
    /* Firmware version 1 required no frequency configuration */
    else
    {
        g_fcl_data_str.opVar_str.opResult_enu = R_FCL_OK;
    }
} /* R_FCL_FCUFct_SetFrequency */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_SetFrequency
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_SwitchMode_Start
 *********************************************************************************************************************/
/**
 * This function changes the Flash sequencer operation mode.
 *
 * @param[in]     mode_enu:
 * @li              R_FCL_READ_MODE
 * @li              R_FCL_WRITE_MODE
 * @li              R_FCL_CFG_WRITE_MODE
 * @return
 * @li            R_FCL_OK:             Mode switched started successfully
 * @li            R_FCL_ERR_REJECTED:   Data Flash operation on-going or the mode is already switched
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
r_fcl_status_t R_FCL_FCUFct_SwitchMode_Start (r_fcl_mode_t mode_enu)                               /* PRQA S 1503 */
{
    r_fcl_status_t ret;
    uint32_t       regFSTATR;
    uint16_t       regFENTRYR;

    ret = R_FCL_OK;

    if (R_FCL_READ_MODE == mode_enu)
    {
        regFSTATR = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);

        /* erase, write or illegal error must be reset before mode switch */
        if (R_FPSYS_REGBIT_FSTATR_RESET != ( regFSTATR & (R_FPSYS_REGBIT_FSTATR_ERSERR +
                                                          R_FPSYS_REGBIT_FSTATR_PRGERR +
                                                          R_FPSYS_REGBIT_FSTATR_ILGERR)))
        {
            R_FCL_FCUFct_ClearStatus ();
        }
    }
    else
    {
        regFENTRYR = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FENTRYR_U16);
        /* If Data Flash PE mode is activated, don't continue */
        if ((regFENTRYR & (uint16_t)R_FPSYS_MODE_DPE) == (uint16_t)R_FPSYS_MODE_DPE)
        {
            ret = R_FCL_ERR_REJECTED;
        }
        
        /* If the target mode is activated, don't continue */
        else if ((regFENTRYR & (uint16_t)0x00FFu) == (uint16_t)mode_enu)
        {
            ret = R_FCL_ERR_REJECTED;
        }
        else
        {
            /* Reset FACI address registers */
            R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_FSADR_U32, 0x00000000u);
            R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_FEADR_U32, 0x00000000u);
        }
    }
    
    if (R_FCL_OK == ret)
    {
        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FENTRYR_U16, (uint16_t)(mode_enu) + R_FPSYS_REGBIT_FENTRY_KEY);
        g_fcl_data_str.flashMode_enu = mode_enu;
    }
    
    return (ret);
} /* R_FCL_FCUFct_SwitchMode_Start */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_SwitchMode_Start
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_SwitchMode_Check
 *********************************************************************************************************************/
/**
 * This function checks if the Flash sequencer operation mode switch is performed correctly
 *
 * @param[in]     -
 * @return
 * @li            R_FCL_OK:              mode switched successful
 * @li            R_FCL_BUSY:            switching not performed yet
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
r_fcl_status_t R_FCL_FCUFct_SwitchMode_Check ( void )
{
    r_fcl_status_t ret;
    uint16_t       regFENTRYR;

    ret = R_FCL_BUSY;

    regFENTRYR = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FENTRYR_U16);
    regFENTRYR &= (uint16_t)(R_FPSYS_MODE_DPE + R_FPSYS_MODE_CPE);
    if ((uint16_t)g_fcl_data_str.flashMode_enu == regFENTRYR)
    {
        ret = R_FCL_OK;
    }

    return (ret);
} /* R_FCL_FCUFct_SwitchMode_Check */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_SwitchMode_Check
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_ForcedStop
 *********************************************************************************************************************/
/**
 * This function resets the FCU and FACI registers
 *
 * @param[in,out] -
 * @return
 * @li            R_FCL_OK:             function passed
 * @li            R_FCL_ERR_INTERNAL:   timeout error
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_ForcedStop (void)                                          /* PRQA S 1503 */
{
    r_fcl_status_t ret;
    uint32_t res;

    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_FORCED_STOP);


    /* Register check loop */
    res = R_FCL_FCUFct_ChkReg (R_FPSYS_REGADD_FSTATR_U32,
                               R_FPSYS_REGBIT_FSTATR_FRDY,
                               R_FPSYS_REGBIT_FSTATR_FRDY,  /* Wait until the ready flag is set */
                               R_FCL_TIMEOUT_CHKREG);


    /* Patch the check loop result: 0 --> check passed */
    if (0x00000000u == res)
    {
        ret = R_FCL_OK;
    }
    
    /* != 0 --> timeout in check loop */
    else
    {
        ret = R_FCL_ERR_INTERNAL;
    }
    
    return (ret);

} /* R_FCL_FCUFct_ForcedStop */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_ForcedStop
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_ChkSuspended
 *********************************************************************************************************************/
/**
 * This function checks if an erase or write operation is suspended by checking the corresponding bits in FSTATR
 *
 * @param[in,out] -
 * @return
 * @li            R_FCL_TRUE:   either one of the following bits is set PRGSPD or ERSSPD
 * @li            R_FCL_FALSE:  no operation is suspended
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_flag_t R_FCL_FCUFct_ChkSuspended (void)
{
    uint32_t        regFSTATR;
    r_fcl_flag_t    ret;

    ret = R_FCL_FALSE;

    regFSTATR = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);
    if ((R_FPSYS_REGBIT_FSTATR_PRGSPD == (regFSTATR & R_FPSYS_REGBIT_FSTATR_PRGSPD)) ||
        (R_FPSYS_REGBIT_FSTATR_ERSSPD == (regFSTATR & R_FPSYS_REGBIT_FSTATR_ERSSPD)))
    {
        ret = R_FCL_TRUE;
    }
    
    return ret;
} /* R_FCL_FCUFct_ChkSuspended */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_ChkSuspended
 *********************************************************************************************************************/


/*********************************************************************************************************************
* Function name:  R_FCL_FCUFct_ClearStatus
*********************************************************************************************************************/
/**
 * Clear (error) status of the sequencer by the clear status FCU command
 *
 * @param      -
 * @return     ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_ClearStatus (void)
{
    uint32_t res32;
    uint8_t  res8;

    res32 = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);

    /* In case of command lock due to illegal error, reset the command lock bit */
    if (R_FPSYS_REGBIT_FSTATR_ILGERR == (R_FPSYS_REGBIT_FSTATR_ILGERR & res32))
    {
        res8 = R_FCL_I_read_memory_u08 (R_FPSYS_REGADD_FASTAT_U8);
        /* Only CLDLK bit may be set, others have to be cleared */
        if (R_FPSYS_REGBIT_FASTAT_CMDLK != res8)
        {
            R_FCL_I_write_memory_u08 (R_FPSYS_REGADD_FASTAT_U8, R_FPSYS_REGBIT_FASTAT_CMDLK);
        }
    }

    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_CLEARSTAT);
} /* R_FCL_FCUFct_ClearStatus */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_ClearStatus
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_RestoreLastState
 *********************************************************************************************************************/
/**
 * This function restores the last library state after execution of a command. Depending on the previous executed
 * command the library state is:
 * @li            prepared (no command suspended)
 * @li            suspended (the previous command need to be finished)
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_RestoreLastState (void)
{
    if (g_fcl_data_str.suspendInfo_str.flagSuspended_enu == R_FCL_FALSE)
    {
        g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_PREPARED;
    }
    else
    {
        g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_SUSPENDED;
    }
} /* R_FCL_FCUFct_RestoreLastState */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_RestoreLastState
 *********************************************************************************************************************/


/**********************************************************************************************************************
* Function name: R_FCL_FCUFct_HandleResume
*********************************************************************************************************************/
/**
 * This function performs all operations necessary for resuming a suspended erase or write command
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMMAND_EXECUTION_MODE == R_FCL_HANDLER_CALL_USER

#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_HandleResume (void)
{
    uint32_t regFSTATR;

    /* check if, before suspend, the operation was waiting on switch mode confirmation */
    if (R_FCL_TRUE == g_fcl_data_str.waitAckModeSwitch_enu)
    {
    
        /* After mode switch is completed we can continue */
        if (R_FCL_OK == R_FCL_FCUFct_SwitchMode_Check())
        {
            R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                      g_fcl_data_str.suspendInfo_str.matSel_u16 + R_FPSYS_REGBIT_FMATSELC_KEY);

            /* Restore variables for resume */
            g_fcl_data_str.reqInt_pstr            = g_fcl_data_str.suspendInfo_str.reqSuspend_pstr;
            g_fcl_data_str.opVar_str.opAddr_u32   = g_fcl_data_str.suspendInfo_str.opSuspend_str.opAddr_u32;
            g_fcl_data_str.opVar_str.opCnt_u16    = g_fcl_data_str.suspendInfo_str.opSuspend_str.opCnt_u16;
            g_fcl_data_str.opVar_str.opStatus_enu = g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu;
            g_fcl_data_str.opVar_str.opResult_enu = g_fcl_data_str.suspendInfo_str.opSuspend_str.opResult_enu;
        
            g_fcl_data_str.suspendInfo_str.reqSuspend_pstr            = R_FCL_REQUEST_POINTER_UNDEFINED;
            g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_IDLE_STATE;
            g_fcl_data_str.suspendInfo_str.spdResRequest_enu          = R_FCL_FALSE;
            g_fcl_data_str.suspendInfo_str.flagSuspended_enu          = R_FCL_FALSE;
            
            g_fcl_data_str.deviceInfo_str.matSel_u16 = g_fcl_data_str.suspendInfo_str.matSel_u16;

            regFSTATR = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);
            if ((R_FPSYS_REGBIT_FSTATR_PRGSPD == (regFSTATR & R_FPSYS_REGBIT_FSTATR_PRGSPD)) ||
                (R_FPSYS_REGBIT_FSTATR_ERSSPD == (regFSTATR & R_FPSYS_REGBIT_FSTATR_ERSSPD)))
            {
                R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_RESUME);
            }
            g_fcl_data_str.intStatus_enu           = R_FCL_ISTAT_BUSY;
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_BUSY;
            g_fcl_data_str.waitAckModeSwitch_enu   = R_FCL_FALSE;
        }
    }
    else
    {

        /* restore the FMATSELC content in case the FACI operation was not finished before suspend */
        if ( g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu != R_FCL_OP_FINISHED_STATE)
        {
            R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                      g_fcl_data_str.suspendInfo_str.matSel_u16 + R_FPSYS_REGBIT_FMATSELC_KEY);
        }

        /* Restore variables for resume */
        g_fcl_data_str.reqInt_pstr             = g_fcl_data_str.suspendInfo_str.reqSuspend_pstr;
        g_fcl_data_str.opVar_str.opAddr_u32    = g_fcl_data_str.suspendInfo_str.opSuspend_str.opAddr_u32;
        g_fcl_data_str.opVar_str.opCnt_u16     = g_fcl_data_str.suspendInfo_str.opSuspend_str.opCnt_u16;
        g_fcl_data_str.opVar_str.opStatus_enu  = g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu;
        g_fcl_data_str.opVar_str.opResult_enu  = g_fcl_data_str.suspendInfo_str.opSuspend_str.opResult_enu;
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_BUSY;
        
        g_fcl_data_str.deviceInfo_str.matSel_u16 = g_fcl_data_str.suspendInfo_str.matSel_u16;
        
        g_fcl_data_str.suspendInfo_str.reqSuspend_pstr            = R_FCL_REQUEST_POINTER_UNDEFINED;
        g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_IDLE_STATE;
        g_fcl_data_str.suspendInfo_str.spdResRequest_enu          = R_FCL_FALSE;
        g_fcl_data_str.suspendInfo_str.flagSuspended_enu          = R_FCL_FALSE;
        g_fcl_data_str.intStatus_enu                              = R_FCL_ISTAT_BUSY;
    }
} /* R_FCL_FCUFct_HandleResume */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_HandleResume
 *********************************************************************************************************************/
#endif


/**********************************************************************************************************************
* Function name: R_FCL_FCUFct_CheckMode
*********************************************************************************************************************/
/**
 * This function checks whether the FLMD0 pin is set according to its needs.
 *
 * @param[in,out] -
 * @return
 * @li            R_FCL_TRUE:            FLMD0 pin set correct
 * @li            R_FCL_FALSE:           FLMD0 pin setting invalid
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
r_fcl_flag_t R_FCL_FCUFct_CheckMode (void)
{
    r_fcl_flag_t ret;
    uint8_t      regVal;

    ret = R_FCL_FALSE;

    regVal = R_FCL_I_read_memory_u08 (R_FPSYS_REGADD_FPMON_U8);
    if (R_FPSYS_REGBIT_SFWE == (regVal & R_FPSYS_REGBIT_SFWE))
    {
        ret = R_FCL_TRUE;
    }

    return (ret);
} /* R_FCL_FCUFct_CheckMode */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_CheckMode
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetResult
 *********************************************************************************************************************/
/**
 * This function reads the current status of the FACI.
 *
 * @param[in]     cmdSpdExec_enu
 * @li                                  R_FCL_TRUE:     last executed FCU command is Suspend
 * @li                                  R_FCL_FALSE:    last executed FCU command is different than Suspend
 *
 * @return
 * @li            R_FCL_BUSY:           a FACI operation is ongoing
 * @li            R_FCL_OK:             last FACI command finished successfully
 * @li            R_FCL_ERR_PROTECTION: command execution impossible due to set OTP flag
 * @li            R_FCL_ERR_ERASE:      an erase error occured
 * @li            R_FCL_ERR_WRITE:      a write error occured
 * @li            R_FCL_ERR_INTERNAL:   any other error occured
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC r_fcl_status_t R_FCL_FCUFct_GetResult (r_fcl_flag_t cmdSpdExec_enu)
{
    r_fcl_status_t ret;
    uint32_t       regFSTATR;
    uint16_t       regFPESTAT;

    regFSTATR = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);

    /* For the result check, the FACI must be ready */
    if (R_FPSYS_REGBIT_FSTATR_FRDY == (regFSTATR & R_FPSYS_REGBIT_FSTATR_FRDY))
    {
        #if (defined FCL_TEST_PATCH)
            if (0 != TestPatch.status_patch_active)
            {
                regFSTATR = TestPatch.status_patch_value;
                if (UINT32_MAX != TestPatch.status_patch_active)
                {
                    TestPatch.status_patch_active--;
                }
            }
        #endif
        
        /* Check fatal error bits */
        if ((R_FPSYS_REGBIT_FSTATR_FCURAMERR == (regFSTATR & R_FPSYS_REGBIT_FSTATR_FCURAMERR)) ||
            (R_FPSYS_REGBIT_FSTATR_TBLDERR   == (regFSTATR & R_FPSYS_REGBIT_FSTATR_TBLDERR))   ||
            (R_FPSYS_REGBIT_FSTATR_CONFDERR  == (regFSTATR & R_FPSYS_REGBIT_FSTATR_CONFDERR))  ||
            (R_FPSYS_REGBIT_FSTATR_FCUERR    == (regFSTATR & R_FPSYS_REGBIT_FSTATR_FCUERR))    ||
            (R_FPSYS_REGBIT_FSTATR_OTPDERR   == (regFSTATR & R_FPSYS_REGBIT_FSTATR_OTPDERR)))
        {
            ret = R_FCL_ERR_INTERNAL;
        }
        
        /* ILG error is only fatal if we did not want to suspend an operation */
        else if (R_FPSYS_REGBIT_FSTATR_ILGERR == (regFSTATR & R_FPSYS_REGBIT_FSTATR_ILGERR))
        {
            if (R_FCL_FALSE == cmdSpdExec_enu)
            {
                ret = R_FCL_ERR_INTERNAL;
            }
            else
            {
                ret = R_FCL_ERR_SUSPEND_FAILED;
            }
        }
        
        /* Check FHVE error (protection error) */
        else if (R_FPSYS_REGBIT_FSTATR_FHVEERR == (regFSTATR & R_FPSYS_REGBIT_FSTATR_FHVEERR))
        {
            ret = R_FCL_ERR_PROTECTION;
        }
        
        /* Erase error bit may result in erase or protection error, depending on the lock bit protection */
        else if (R_FPSYS_REGBIT_FSTATR_ERSERR == (regFSTATR & R_FPSYS_REGBIT_FSTATR_ERSERR))
        {
            regFPESTAT = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FPESTAT_U16);
            if (R_FPSYS_REGBIT_FPESTAT_ERASELOCK != (regFPESTAT & R_FPSYS_REGBIT_FPESTAT_ERASELOCK))
            {
                ret = R_FCL_ERR_ERASE;
            }
            else
            {
                ret = R_FCL_ERR_PROTECTION;
            }
            
        }
        
        /* Program error bit may result in program or protection error, depending on the lock bit protection */
        else if (R_FPSYS_REGBIT_FSTATR_PRGERR == (regFSTATR & R_FPSYS_REGBIT_FSTATR_PRGERR))
        {
            regFPESTAT = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FPESTAT_U16);
            if (R_FPSYS_REGBIT_FPESTAT_WRITELOCK != (regFPESTAT & R_FPSYS_REGBIT_FPESTAT_WRITELOCK))
            {
                ret = R_FCL_ERR_WRITE;
            }
            else
            {
                ret = R_FCL_ERR_PROTECTION;
            }
        }
        
        /* No error */
        else
        {
            ret = R_FCL_OK;
        }
    }
    
    /* Not finished yet */
    else
    {
        ret = R_FCL_BUSY;
    }


    return (ret);
} /* R_FCL_FCUFct_GetResult */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetResult
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetConfigArea
 *********************************************************************************************************************/
/**
 * This function reads the Extra Area 2
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
void R_FCL_FCUFct_GetConfigArea (void)                                                                /* PRQA S 1505 */
{
    uint16_t data;
    uint16_t i;
    uint32_t bufAddr;
    uint32_t readAddr;
    uint32_t cnt;
    r_fcl_status_t ret;
    r_fcl_status_t ret1;

    R_FCL_FCUFct_Switch_BFlash (0x01u);
    ret = R_FCL_FCUFct_Clear_Cache ();

    /* No timeout error in clear cache */
    if (R_FCL_OK == ret)
    {
        /* Select byte count and read address acc. to the command */
        if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_OPB)                             /* PRQA S 2814 */
        {
            readAddr = R_FPSYS_CONFIG_AREA_OPTIONBYTES;
            cnt = R_PRG_CFG_TRANSFERSIZE_OPB;
        }
        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_ID)
        {
            readAddr = R_FPSYS_CONFIG_AREA_OCD;
            cnt = R_PRG_CFG_TRANSFERSIZE_OCD;
        }
        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_OTP)
        {
            readAddr = g_fcl_data_str.opVar_str.opAddr_u32;
            cnt = R_PRG_CFG_TRANSFERSIZE_OTP;
        }
        
        /* Get reset vector command*/
        else
        {
            readAddr = R_FPSYS_CONFIG_AREA_RESET;
            cnt = R_PRG_CFG_TRANSFERSIZE_RESET;
        }

        bufAddr = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;

        /* Read the  data */
        for (i = 0u; i < (2u * cnt); i++)
        {
            data = R_FCL_I_read_memory_u08 (readAddr);
            R_FCL_I_write_memory_u08 (bufAddr, (uint8_t)data);
            bufAddr++;
            readAddr++;
        }
    } /* if (R_FCL_OK == ret) */
    
    R_FCL_FCUFct_Switch_BFlash (0x00u);
    ret1 = R_FCL_FCUFct_Clear_Cache ();

    /* return error of 1st and of 2nd clear cache */
    if (R_FCL_OK == ret)
    {
        ret = ret1;
    }

    /* Internal error results in not initialized library to avoid further command execution */
    if (R_FCL_OK != ret)
    {
        g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_NOINIT;
    }

    g_fcl_data_str.reqInt_pstr->status_enu = ret;
} /* R_FCL_FCUFct_GetConfigArea */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetConfigArea
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_SetConfigArea
 *********************************************************************************************************************/
/**
 * This function prepares internal variables to write the Extra Area 2
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_SetConfigArea (void)
{
    uint32_t flag;
    uint8_t  i;

    if (R_FCL_FCUFct_CheckMode () == R_FCL_FALSE)
    {
        /* FLMD0 pin not set */
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_FLMD0;                                  /* PRQA S 2814 */
    }
    else
    {
        /* Set config area write data, write count and write address depending on the executed command */
        if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_OPB)
        {
            g_fcl_data_str.opVar_str.buffAddr_u32 = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;
            g_fcl_data_str.opVar_str.opAddr_u32   = R_FPSYS_CONFIG_AREA_OPTIONBYTES;
            g_fcl_data_str.opVar_str.opCnt_u16    = R_PRG_CFG_TRANSFERSIZE_OPB;
        }
        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_OTP)
        {
            g_fcl_data_str.opVar_str.opCnt_u16 = R_PRG_CFG_TRANSFERSIZE_OTP;
        }
        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_ID)
        {
            g_fcl_data_str.opVar_str.buffAddr_u32 = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;
            g_fcl_data_str.opVar_str.opAddr_u32   = R_FPSYS_CONFIG_AREA_OCD;
            g_fcl_data_str.opVar_str.opCnt_u16    = R_PRG_CFG_TRANSFERSIZE_OCD;
        }
        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_RESET_VECTOR)
        {
            g_fcl_data_str.opVar_str.buffAddr_u32 = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;
            g_fcl_data_str.opVar_str.opAddr_u32   = R_FPSYS_CONFIG_AREA_RESET;
            g_fcl_data_str.opVar_str.opCnt_u16    = R_PRG_CFG_TRANSFERSIZE_RESET;
        }

        /* We have a protection flag set command */
        else
        {
            flag = g_fcl_data_str.deviceInfo_str.secureData_u32;
            g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_AREA_SEC_FLAGS;
            g_fcl_data_str.opVar_str.opCnt_u16  = R_PRG_CFG_TRANSFERSIZE;

            if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_READ_PROTECT_FLAG)
            {
                flag = flag & ~(1u << R_RDPR_FLAG_SHIFT);
            }
            else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_WRITE_PROTECT_FLAG)
            {
                flag = flag & ~(1u << R_WRPR_FLAG_SHIFT);
            }
            else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_ERASE_PROTECT_FLAG)
            {
                flag = flag & ~(1u << R_SEPR_FLAG_SHIFT);
            }
            else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_SERIAL_PROG_DISABLED)
            {
                flag = flag & ~(1u << R_SPD_FLAG_SHIFT);
            }
            else
            {
                flag = flag & ~(1u << R_RDPR_FLAG_SHIFT);
                flag = flag & ~(1u << R_SEPR_FLAG_SHIFT);
                flag = flag & ~(1u << R_WRPR_FLAG_SHIFT);
                flag = flag & ~(1u << R_SPIE_FLAG_SHIFT);
            }
            flag = flag & 0xfffffff0u;

            /* Prepare write data  */
            g_fcl_data_str.intBuf_au08[0] = (uint8_t)flag;
            g_fcl_data_str.intBuf_au08[1] = (uint8_t)(flag >> 8u);
            g_fcl_data_str.intBuf_au08[2] = (uint8_t)(flag >> 16u);
            g_fcl_data_str.intBuf_au08[3] = (uint8_t)(flag >> 24u);
            for (i = 4u; i < (2u*R_PRG_CFG_TRANSFERSIZE); i++)
            {
                g_fcl_data_str.intBuf_au08[i] = (uint8_t)0xffu;
            }
            g_fcl_data_str.opVar_str.buffAddr_u32 = (uint32_t)(&g_fcl_data_str.intBuf_au08[0]);    /* PRQA S 0306 */
        }
        g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_BUSY;
        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
    }
} /* R_FCL_FCUFct_SetConfigArea */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_SetConfigArea
 *********************************************************************************************************************/


#ifdef R_FCL_SUPPORT_SECURITYFLAGS
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetSecurityFlag
 *********************************************************************************************************************/
/**
 * This function reads the security settings from a internal stored variable
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_GetSecurityFlag (void)                                                           /* PRQA S 1503 */
{
    uint32_t data;

    /* Decode the protection flag from read data acc. to the executed command */
    if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_READ_PROTECT_FLAG)                /* PRQA S 2814 */
    {
        data = (((~g_fcl_data_str.deviceInfo_str.secureData_u32) & R_RDPR_FLAG_MASK) >> R_RDPR_FLAG_SHIFT);
    }
    else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_WRITE_PROTECT_FLAG)
    {
        data = (((~g_fcl_data_str.deviceInfo_str.secureData_u32) & R_WRPR_FLAG_MASK) >> R_WRPR_FLAG_SHIFT);
    }
    else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_ERASE_PROTECT_FLAG)
    {
        data = (((~g_fcl_data_str.deviceInfo_str.secureData_u32) & R_SEPR_FLAG_MASK) >> R_SEPR_FLAG_SHIFT);
    }
    else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_SERIAL_PROG_DISABLED)
    {
        data = (((~g_fcl_data_str.deviceInfo_str.secureData_u32) & R_SPD_FLAG_MASK) >> R_SPD_FLAG_SHIFT);
    }
    else
    {
        data = (((~g_fcl_data_str.deviceInfo_str.secureData_u32) & R_SPIE_FLAG_MASK) >> R_SPIE_FLAG_SHIFT);
    }

    R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, data);

    g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
} /* R_FCL_FCUFct_GetSecurityFlag */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetSecurityFlag
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_SECURITYFLAGS */


#ifdef R_FCL_SUPPORT_BLOCKCNT
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetBlockCnt
 *********************************************************************************************************************/
/**
 * This function reads the amount of available blocks from a internal stored variable
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_GetBlockCnt (void)                                                               /* PRQA S 1503 */
{
    R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32,
                                        (uint32_t)g_fcl_data_str.deviceInfo_str.blkCnt_u16);

    g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
}
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetBlockCnt
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_BLOCKCNT */


#ifdef R_FCL_SUPPORT_DEVICENAME
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetDeviceName
 *********************************************************************************************************************/
/**
 * This function reads the device name from a internal stored variable
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_GetDeviceName (void)                                                             /* PRQA S 1503 */
{
    uint32_t bufAddr;
    uint8_t  i;

    bufAddr = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;                                           /* PRQA S 2814 */

    for (i = 0u; i < 16u; i++)
    {
        R_FCL_I_write_memory_u08 (bufAddr, g_fcl_data_str.deviceInfo_str.deviceName_au08[i]);
        bufAddr += 1u;
    }
    g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
}
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetDeviceName
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_DEVICENAME */


#ifdef R_FCL_SUPPORT_BLOCKENDADDR
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetBlockEndAddr
 *********************************************************************************************************************/
/**
 * This function calculates the block end address depending on the given block
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_GetBlockEndAddr (void)                                                           /* PRQA S 1503 */
{
    uint32_t blk;
    uint32_t data;

    /* User boot area is addressed */                                                              /* PRQA S 2814 1 */
    if (R_FCL_USERBOOT_AREA_SELECT == (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FCL_USERBOOT_AREA_SELECT))
    {
        /* Parameter check passes */
        if ( ((g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK) <
               g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32)
           && (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE) )
        {
            blk = ((g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK) + 1u);
            data = (blk * R_FLI_BLOCK_SIZE) - 1u;
            data = data + R_FLI_USERBOOT_AREA_START;
            R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, data);
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
        }
        else
        {
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
        }
    }
    
    /* User area is addressed */
    else
    {
        /* Parameter check passes */
        if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.blkCnt_u16)
        {
            /* Code Flash 1 (8kb blocks) */
            if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8)
            {
                blk = (g_fcl_data_str.reqInt_pstr->idx_u32 + 1u);
                data = (blk * R_FLI_SMALL_BLOCK_SIZE) - 1u;
                R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, data);
            }
            
            /* Code Flash 1 (32kb blocks) */
            else
            {
                /* Code Flash 1 */
                if (g_fcl_data_str.reqInt_pstr->idx_u32 <
                   (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                    g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32))
                {
                    blk  = (g_fcl_data_str.reqInt_pstr->idx_u32 + 1u) -
                             g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8;
                    data = (((uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8) * R_FLI_SMALL_BLOCK_SIZE) +
                            (blk * R_FLI_BLOCK_SIZE)) - 1u;
                    R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, data);
                }
                else
                {
                    /* Code Flash 2 (32kb blocks) */
                    blk  = (g_fcl_data_str.reqInt_pstr->idx_u32 + 1u) -
                            (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                             g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32);
                    data = (R_FLI_CFLASH2_START + (blk * R_FLI_BLOCK_SIZE)) - 1u;
                    R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, data);
                }
            }
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
        }
        else
        {
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
        }
    }
} /* R_FCL_FCUFct_GetBlockEndAddr */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetBlockEndAddr
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_BLOCKENDADDR */


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_PrepareEnvironment_IntErr
 *********************************************************************************************************************/
/**
 * Internal error handling preparation for PrepareEnvironment
 *
 * @param[in]     opState: internal error handling state to be entered
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_PrepareEnvironment_IntErr (r_fcl_op_status_t opState)
{
    /* Enable FAEINT again in error case */
    R_FCL_I_write_memory_u08 (R_FPSYS_REGADD_FAEINT_U8, R_FPSYS_REGVAL_FAEINT_ENABLE);

    g_fcl_data_str.opVar_str.opStatus_enu = opState;
}


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_PrepareEnvironment
 *********************************************************************************************************************/
/**
 * This function prepares the environment for further Flash operations:
 * @li      copy the firmware to the FCURam
 * @li      read out device internal settings (block, count, security settings, ...)
 * @li      set device frequency
 * @li      resets the FACI
 *
 * @param[in,out]   -
 * @return          ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
void R_FCL_FCUFct_PrepareEnvironment (void)
{
    r_fcl_status_t ret;
    uint16_t regFENTRYR;

    /* Prepare operation state 1: R_FCL_OP_PREPARE_STATE */
    if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_OP_PREPARE_STATE)
    {
        /* Only start if FACI is in Read mode */
        regFENTRYR = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FENTRYR_U16);
        if (R_FPSYS_REGBIT_FENTRY_OFF == regFENTRYR)
        {
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;

            /* Disable FCU error interrupt which will occur always during FCU initialization from power up */
            R_FCL_I_write_memory_u08 (R_FPSYS_REGADD_FAEINT_U8, R_FPSYS_REGVAL_FAEINT_DISABLE);

            /* Activate FCU RAM access */
            R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FCURAME_U16, R_FPSYS_REGBIT_FCURAME_FCRME +
                                      R_FPSYS_REGBIT_FCURAME_FRAMTRAN + R_FPSYS_REGBIT_FCURAME_KEY);

            #ifdef R_FCL_INNER_NO_BFA_SWITCH
                /* Get device information - must be read outside BFA switch for F1K*/
                R_FCL_FCUFct_GetDeviceInfo ();
            #endif

            /* Activate BFlash
               Note: required even for R_FCL_INNER_NO_BFA_SWITCH configuration. See function header of R_FCL_FCUFct_GetDeviceInfo */
            R_FCL_FCUFct_Switch_BFlash (0x01u);
            ret = R_FCL_FCUFct_Clear_Cache ();

            #ifdef R_FCL_INNER_NO_BFA_SWITCH
            #else
                /* Get device information */
                R_FCL_FCUFct_GetDeviceInfo ();
            #endif

            /* timeout in cache clear */
            if (R_FCL_OK != ret)
            {
                R_FCL_FCUFct_Switch_BFlash (0x00u);
                (void)R_FCL_FCUFct_Clear_Cache ();     /* Try to clear the cache even though the previous CC failed */

                R_FCL_FCUFct_PrepareEnvironment_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
            }
            else
            {
                /* Get security data
                   Note: requires BFA switching */
                g_fcl_data_str.deviceInfo_str.secureData_u32 = R_FCL_I_read_memory_u32 (R_FPSYS_CONFIG_AREA_SEC_FLAGS);

                #ifdef R_FCL_INNER_NO_FCU_COPY
                    /* In case we don't need to copy the FW, we set the copy address to copy end address.
                       So, copying is avoided, but BFA switch mechanism is executed as required also for devices with
                       ROM based FCU code */
                    g_fcl_data_str.fcuramAddr_u32 = (R_FPSYS_FCURAM_AREA_BASE + R_FPSYS_FCURAM_AREA_SIZE);
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_PREP_COPY_FW_STATE;

                #else
                    ret = R_FCL_OK;                                                                /* PRQA S 2982 */
                    #ifdef R_FCL_INNER_NO_BFA_SWITCH
                        /* Deactivate BFlash */
                        R_FCL_FCUFct_Switch_BFlash (0x00u);
                        ret = R_FCL_FCUFct_Clear_Cache ();

                        /* Timeout in clear cache */
                        if (R_FCL_OK != ret)
                        {
                            R_FCL_FCUFct_PrepareEnvironment_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
                        }
                    #endif
                    if (R_FCL_OK == ret)                                                           /* PRQA S 2991,2995 */
                    {
                        g_fcl_data_str.fcuramAddr_u32 = R_FPSYS_FCURAM_AREA_BASE;
                        g_fcl_data_str.firmAddr_u32 = R_FPSYS_FCURAM_AREA_RAM_ADD;

                        /* Start copying firmware to FCU RAM */
                        (void)R_FCL_Fct_Copy_Code (g_fcl_data_str.firmAddr_u32, g_fcl_data_str.fcuramAddr_u32,
                                                   R_FPSYS_FCURAM_AREA_COPY_SIZE);
                        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_PREP_COPY_FW_STATE;
                    }

                #endif

            }
        }
        else
        {
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_ERR_REJECTED;
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
        }
    }

    /* Prepare operation state 2: R_FCL_PREP_COPY_FW_STATE */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_PREP_COPY_FW_STATE)
    {
        #ifdef R_FCL_INNER_NO_FCU_COPY
        #else
            g_fcl_data_str.fcuramAddr_u32 = g_fcl_data_str.fcuramAddr_u32 + R_FPSYS_FCURAM_AREA_COPY_SIZE;
            g_fcl_data_str.firmAddr_u32   = g_fcl_data_str.firmAddr_u32 + R_FPSYS_FCURAM_AREA_COPY_SIZE;
        #endif

        /* Firmware copy process is finished? */
        if (g_fcl_data_str.fcuramAddr_u32 >= (R_FPSYS_FCURAM_AREA_BASE + R_FPSYS_FCURAM_AREA_SIZE))
        {
            /* Deactivate BFlash */
            R_FCL_FCUFct_Switch_BFlash (0x00u);
            ret = R_FCL_FCUFct_Clear_Cache ();

            /* Timeout in clear cache */
            if (R_FCL_OK != ret)
            {
                R_FCL_FCUFct_PrepareEnvironment_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
            }

            /* Cache clear worked */
            else
            {
                /* Deactivate FCU RAM access */
                R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FCURAME_U16,
                                          R_FPSYS_REGBIT_FCURAME_RESET + R_FPSYS_REGBIT_FCURAME_KEY);

                ret = R_FCL_FCUFct_SwitchMode_Start (R_FCL_CFG_WRITE_MODE);
                /* Switch mode error check */

                if (R_FCL_OK == ret)
                {
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_PREP_RESET_FCU_STATE;
                }
                else
                {
                    /* Enable FAEINT again in error case */
                    R_FCL_I_write_memory_u08 (R_FPSYS_REGADD_FAEINT_U8, R_FPSYS_REGVAL_FAEINT_ENABLE);

                    g_fcl_data_str.opVar_str.opResult_enu = ret;
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
                }
            } /* else - if (R_FCL_OK != ret) */
        }
        else
        {
            (void)R_FCL_Fct_Copy_Code (g_fcl_data_str.firmAddr_u32, g_fcl_data_str.fcuramAddr_u32,
                                       R_FPSYS_FCURAM_AREA_COPY_SIZE);
        }
    }

    /* Prepare operation state 3: R_FCL_PREP_RESET_FCU_STATE */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_PREP_RESET_FCU_STATE)
    {
        /* Wait in the same state till mode is really changed */
        ret = R_FCL_FCUFct_SwitchMode_Check ();
        if (R_FCL_OK == ret)
        {
            ret = R_FCL_FCUFct_ForcedStop ();
            
            /* timeout in forced stop processing */
            if (R_FCL_OK != ret)
            {
                R_FCL_FCUFct_PrepareEnvironment_IntErr (R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE);
            }
            else
            {
                R_FCL_FCUFct_ClearStatus ();

                (void)R_FCL_FCUFct_SwitchMode_Start (R_FCL_READ_MODE);

                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_PREP_FCURAM_CHKSUM_STATE;
            }

            /* Enable FCU error interrupt again, now we expect no more ECC error */
            R_FCL_I_write_memory_u08 (R_FPSYS_REGADD_FAEINT_U8, R_FPSYS_REGVAL_FAEINT_ENABLE);
        }
    }
    
    /* Prepare operation state 4: R_FCL_PREP_FCURAM_CHKSUM_STATE */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_PREP_FCURAM_CHKSUM_STATE)
    {
        /* Wait in the same state till mode is really changed */
        ret = R_FCL_FCUFct_SwitchMode_Check ();
        if (R_FCL_OK == ret)
        {
            /* Default next state */
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_PREP_ID_CHECK_FREQ_SET_STATE;

            #ifdef R_FCL_INNER_NO_FCU_COPY
            #else
                ret = R_FCL_FCUFct_VerifyChecksum ();
                
                /* Either Checksum not OK or timeout */
                if (R_FCL_OK != ret)
                {
                    R_FCL_FCUFct_PrepareEnvironment_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
                }
            #endif
        }
    }
    
    /* Prepare operation state 5: R_FCL_PREP_ID_CHECK_FREQ_SET_STATE */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_PREP_ID_CHECK_FREQ_SET_STATE)
    {
        /* ID verification */
        ret = R_FCL_FCUFct_VerifyID ();
        if (R_FCL_OK == ret)
        {
            R_FCL_FCUFct_SetFrequency ();
        }
        else
        {
            g_fcl_data_str.opVar_str.opResult_enu = ret;
        }
        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
    }
    
    /* Prepare operation finish state */
    else /* (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_OP_FINISHED_STATE) */
    {
        /* operation finished state - set the library status acc. to the operation result */
        g_fcl_data_str.reqInt_pstr->status_enu = g_fcl_data_str.opVar_str.opResult_enu;            /* PRQA S 2814 */
        if (g_fcl_data_str.reqInt_pstr->status_enu == R_FCL_OK)
        {
            g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_PREPARED;
        }
        else
        {
            g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_INIT;
        }
        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_IDLE_STATE;

        g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_IDLE_STATE;
        g_fcl_data_str.suspendInfo_str.reqSuspend_pstr = R_FCL_REQUEST_POINTER_UNDEFINED;
        g_fcl_data_str.suspendInfo_str.flagSuspended_enu = R_FCL_FALSE;
        g_fcl_data_str.cancelReq_enu = R_FCL_FALSE;
    }
} /* R_FCL_FCUFct_PrepareEnvironment */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_PrepareEnvironment
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetDeviceInfo
 *********************************************************************************************************************/
/**
 * This function reads out device settings from the Extra Area:
 *          - firmware version
 *          - frequency settings
 *          - amount of Code Flash blocks for all Flash macros
 *          - device name
 *          - security settings
 * Note: This function requires BFA switching as the security flag information is stored in the config area. To read it,
 *       BFA switching is always required even for e.g. F1K.
 *       For code simplification, the BFA switch mechanism which also requires clear cache error handling is implemented
 *       only once (for devices with and without R_FCL_INNER_NO_BFA_SWITCH set) in prepare environment. It is not
 *       separately handled for R_FCL_INNER_NO_BFA_SWITCH set
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_GetDeviceInfo (void)                                                /* PRQA S 1503 */
{
    uint32_t data;
    uint32_t dataAddr;

    /* Allow Extended User Area Programming and Erase */
    g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu = R_FCL_TRUE;
    
    /* Get firmware version */
    g_fcl_data_str.deviceInfo_str.fVersion_u8 = R_FCL_I_read_memory_u08 (R_FPSYS_EXTRA3_AREA_FPVER_U08);

    /* Get block information */
    dataAddr = R_FCL_I_read_memory_u32 (R_FPSYS_EXTRA3_AREA_SCDS_U32);

    /* Code Flash 1 */
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDSEL1);
    g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32 = ((data & 0xffff0000u) >> 16) / 32u;
    g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 = (uint8_t)((data & 0x0000ffffu) / 8u);

    /* Code Flash 2 */
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDSEL2);
    g_fcl_data_str.deviceInfo_str.cf2BlkCnt32kB_u32 = (data & 0x0000ffffu) / 32u;

    /* user boot area */
    data = ((R_FLI_USERBOOT_AREA_END + 1u) - R_FLI_USERBOOT_AREA_START);
    g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32 = data / R_FLI_BLOCK_SIZE;


    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDSEL4);

    /* Check emulation mode */
    if ((data & 0x000000ffu) == 0x55u)                                                             /* CCOV 0001 */
    {
        g_fcl_data_str.deviceInfo_str.emulationMode_enu = R_FCL_TRUE;
    }
    else
    {
        g_fcl_data_str.deviceInfo_str.emulationMode_enu = R_FCL_FALSE;
    }

    /* Get device name */
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDNAME1);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[0]  = (uint8_t)(data & 0x000000ffu);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[1]  = (uint8_t)((data & 0x0000ff00u) >> 8);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[2]  = (uint8_t)((data & 0x00ff0000u) >> 16);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[3]  = (uint8_t)((data & 0xff000000u) >> 24);
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDNAME2);
    
    /* get clock settings */
    if (g_fcl_data_str.deviceInfo_str.fVersion_u8 != 0xffu)
    {
        /* Read out PClk related data */
        if (g_fcl_data_str.deviceInfo_str.fVersion_u8 < R_FPSYS_FPVER_04)
        {
            g_fcl_data_str.deviceInfo_str.fDivider_u8 =
                R_FCL_I_read_memory_u08 (R_FPSYS_EXTRA3_AREA_PCLKDIV1_FPVER03_U32);
            g_fcl_data_str.deviceInfo_str.fPClk_u16 =
                (uint16_t)(R_FCL_I_read_memory_u32 (R_FPSYS_EXTRA3_AREA_PCLK_FPVER03_U32) / 1000000u);
            
            /* Only for E1x devices, user boot area access is not allowed */
            if (  ((data & 0x00FFFFFFu) == R_FPSYS_PRDNAME_E1x_012x)
               || ((data & 0x00FFFFFFu) == R_FPSYS_PRDNAME_E1x_01Zx) )                             /* CCOV 0002 */
            {
                g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu = R_FCL_FALSE;
            }
        }
        else
        {
            /* Firmware version 4 or higher */
            g_fcl_data_str.deviceInfo_str.fDivider_u8 =
                R_FCL_I_read_memory_u08 (R_FPSYS_EXTRA3_AREA_PCLKDIV1_FPVER04_U32);
            g_fcl_data_str.deviceInfo_str.fPClk_u16 =
                (uint16_t)(R_FCL_I_read_memory_u32 (R_FPSYS_EXTRA3_AREA_PCLK_FPVER04_U32) / 1000000u);

            /* Only for E1x devices, user boot area access is not allowed */
            if (0u == R_FCL_I_read_memory_u08 (R_FPSYS_EXTRA3_AREA_UB_FPVER04_U08))                /* CCOV 0003 */
            {
                g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu = R_FCL_FALSE;
            }
        }

        /* On F1K devices (identified by G3KH core) UBA usage is allowed */
        if (R_PID_CORE_G3KH == R_FCL_I_GetCPUCorePid ())
        {
            g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu = R_FCL_TRUE;
        }

        /* Read out min and max CPU frequency in MHz */
        g_fcl_data_str.deviceInfo_str.fMax_u16 =
            (uint16_t)(R_FCL_I_read_memory_u32 (R_FPSYS_EXTRA3_AREA_MAXHCLK_U32) / 1000000u);
        g_fcl_data_str.deviceInfo_str.fMin_u16 =
            (uint16_t)(R_FCL_I_read_memory_u32 (R_FPSYS_EXTRA3_AREA_MINHCLK_U32) / 1000000u);
    }

    g_fcl_data_str.deviceInfo_str.deviceName_au08[4]  = (uint8_t)(data & 0x000000ffu);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[5]  = (uint8_t)((data & 0x0000ff00u) >> 8);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[6]  = (uint8_t)((data & 0x00ff0000u) >> 16);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[7]  = (uint8_t)((data & 0xff000000u) >> 24);
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDNAME3);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[8]  = (uint8_t)(data & 0x000000ffu);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[9]  = (uint8_t)((data & 0x0000ff00u) >> 8);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[10] = (uint8_t)((data & 0x00ff0000u) >> 16);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[11] = (uint8_t)((data & 0xff000000u) >> 24);
    data = R_FCL_I_read_memory_u32 (dataAddr + R_FPSYS_EXTRA3_AREA_PRDNAME4);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[12] = (uint8_t)(data & 0x000000ffu);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[13] = (uint8_t)((data & 0x0000ff00u) >> 8);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[14] = (uint8_t)((data & 0x00ff0000u) >> 16);
    g_fcl_data_str.deviceInfo_str.deviceName_au08[15] = (uint8_t)((data & 0xff000000u) >> 24);

    /* Calculate block count */
    g_fcl_data_str.deviceInfo_str.blkCnt_u16 = (uint16_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                                                              g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32 +
                                                              g_fcl_data_str.deviceInfo_str.cf2BlkCnt32kB_u32);

} /* R_FCL_FCUFct_GetDeviceInfo */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetDeviceInfo
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_Erase
 *********************************************************************************************************************/
/**
 * This function prepares internal variables to erase a range of Flash blocks
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_Erase (void)
{
    uint32_t blockCnt;

    /* Check FLMD0 pin setting */
    if (R_FCL_FCUFct_CheckMode () == R_FCL_FALSE)
    {
        
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_FLMD0;                                  /* PRQA S 2814 */
    }
    else
    {
    
        /* Operation on the user boot area? */
        if (R_FCL_USERBOOT_AREA_SELECT == (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FCL_USERBOOT_AREA_SELECT))
        {
            blockCnt = (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK);

            /* extended user area selected */
            if (  ((blockCnt + g_fcl_data_str.reqInt_pstr->cnt_u16) <=
                    g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32)
               && (g_fcl_data_str.reqInt_pstr->cnt_u16 > 0u)
               && (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE))
            {
                /* setup internal variables for multi operation */
                g_fcl_data_str.opVar_str.opCnt_u16 = g_fcl_data_str.reqInt_pstr->cnt_u16;
                g_fcl_data_str.opVar_str.opAddr_u32 = R_FLI_USERBOOT_AREA_START +
                                                        ((g_fcl_data_str.reqInt_pstr->idx_u32 &
                                                          R_FLI_USERBOOT_AREA_MASK) * R_FLI_BLOCK_SIZE);
                g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_BUSY;
                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
            }
            
            /* parameter error */
            else
            {
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }
        
        /* Operation on the user area */
        else
        {
            /* Use local variable to remove IAR warning */
            blockCnt = g_fcl_data_str.reqInt_pstr->idx_u32;

            /* Are the parameters OK? */
            if (  ((blockCnt + g_fcl_data_str.reqInt_pstr->cnt_u16) <= g_fcl_data_str.deviceInfo_str.blkCnt_u16)
               && (g_fcl_data_str.reqInt_pstr->cnt_u16 > 0u))
            {
                /* setup internal variables for multi operation */
                g_fcl_data_str.opVar_str.opCnt_u16 = g_fcl_data_str.reqInt_pstr->cnt_u16;
                
                /* Code Flash 1 (8kb blocks) */
                if (g_fcl_data_str.reqInt_pstr->idx_u32  < g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8)
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 =
                        g_fcl_data_str.reqInt_pstr->idx_u32 * R_FLI_SMALL_BLOCK_SIZE;
                }
                
                /* Code Flash 1 (32kb blocks) */
                else if ( g_fcl_data_str.reqInt_pstr->idx_u32 <
                          (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8  +
                           g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32)
                        )
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 =
                       ((uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8) * R_FLI_SMALL_BLOCK_SIZE) +
                       ((g_fcl_data_str.reqInt_pstr->idx_u32 - g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8)
                        * R_FLI_BLOCK_SIZE);
                }
                
                /* Code Flash 2 */
                else
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 = R_FLI_CFLASH2_START +
                        ((g_fcl_data_str.reqInt_pstr->idx_u32 - g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8
                          - g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32) * R_FLI_BLOCK_SIZE);
                }
                g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_BUSY;
                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
            }
            
            /* parameter error */
            else
            {
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }
    }
} /* R_FCL_FCUFct_Erase */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_Erase
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_Write
 *********************************************************************************************************************/
/**
 * This function prepares internal variables to write a range of Flash addresses
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_Write (void)
{
    uint32_t startAddr;

    /* Check FLMD0 pin setting */
    if (R_FCL_FCUFct_CheckMode () == R_FCL_FALSE)
    {
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_FLMD0;                                  /* PRQA S 2814 */
    }
    else
    {
        /* Use local variable to remove IAR warning */
        startAddr = g_fcl_data_str.reqInt_pstr->idx_u32;

        /* input parameters check against user area range --> user area and data is aligned */
        if ((((startAddr + ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) * (2u * R_FLI_BLOCK_TRANSFERSIZE))) <=
             (((uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8) * R_FLI_SMALL_BLOCK_SIZE) +
              (g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32 * R_FLI_BLOCK_SIZE))) ||
            (((startAddr + ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) * (2u * R_FLI_BLOCK_TRANSFERSIZE))) <=
              (R_FLI_CFLASH2_START + (g_fcl_data_str.deviceInfo_str.cf2BlkCnt32kB_u32 * R_FLI_BLOCK_SIZE))) &&
              (g_fcl_data_str.reqInt_pstr->idx_u32 >= R_FLI_CFLASH2_START))) &&
             ((g_fcl_data_str.reqInt_pstr->idx_u32 % (2u * R_FLI_BLOCK_TRANSFERSIZE)) == 0u) &&
              (g_fcl_data_str.reqInt_pstr->cnt_u16 > 0u))
        {
            g_fcl_data_str.opVar_str.buffAddr_u32 = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;
            g_fcl_data_str.opVar_str.opCnt_u16    = g_fcl_data_str.reqInt_pstr->cnt_u16;
            g_fcl_data_str.opVar_str.opAddr_u32   = g_fcl_data_str.reqInt_pstr->idx_u32;
            g_fcl_data_str.intStatus_enu          = R_FCL_ISTAT_BUSY;
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
        }
        
        /* input parameters check against extended user area range --> area selected and data is aligned */
        else if ((((startAddr + ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) * (2u * R_FLI_BLOCK_TRANSFERSIZE))) <=
                   (R_FLI_USERBOOT_AREA_START +
                   (g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32 * R_FLI_BLOCK_SIZE))) &&
                  ((g_fcl_data_str.reqInt_pstr->idx_u32 % (2u * R_FLI_BLOCK_TRANSFERSIZE)) == 0u)) &&
                   (g_fcl_data_str.reqInt_pstr->cnt_u16 > 0u) &&
                   (g_fcl_data_str.reqInt_pstr->idx_u32 >= R_FLI_USERBOOT_AREA_START) &&
                   (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE))
        {
            g_fcl_data_str.opVar_str.buffAddr_u32 = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;
            g_fcl_data_str.opVar_str.opCnt_u16    = g_fcl_data_str.reqInt_pstr->cnt_u16;
            g_fcl_data_str.opVar_str.opAddr_u32   = g_fcl_data_str.reqInt_pstr->idx_u32;
            g_fcl_data_str.intStatus_enu          = R_FCL_ISTAT_BUSY;
            g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
        }
        
        /* parameter error */
        else
        {
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
        }
    }
} /* R_FCL_FCUFct_Write */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_Write
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_HandleMultiOperation_IntErr
 *********************************************************************************************************************/
/**
 * Internal error handling preparation for MultiOperation
 *
 * @param[in]     opState: internal error handling state to be entered
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
R_FCL_STATIC void R_FCL_FCUFct_HandleMultiOperation_IntErr (r_fcl_op_status_t opState)
{
    /* Reset FMATSEL */
    R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                              g_fcl_data_str.deviceInfo_str.matSel_u16 + R_FPSYS_REGBIT_FMATSELC_KEY);

    g_fcl_data_str.opVar_str.opStatus_enu = opState;
}

/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_HandleMultiOperation
 *********************************************************************************************************************/
/**
 * This function handles all operations which need several handler calls (e.g. erase and write)
 * Depending of the executed command, the FACI is configured. After that, the result is checked until the operation
 * is finished. At the end the result variable of the request structure is updated.
 *
 * @param[in,out] -
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_RAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_RAM"
    #else
        #pragma section text "R_FCL_CODE_RAM"
    #endif
#endif
void R_FCL_FCUFct_HandleMultiOperation (void)
{
    r_fcl_status_t ret;
    r_fcl_status_t res;
    uint32_t       regFSTATR;
    uint16_t       data;
    uint32_t       i;
    uint32_t       readData;
    r_fcl_flag_t   skipCode;
    r_fcl_flag_t   isCmdSpdExec;

    /* Multi operation state 1: start switch mode */
    if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_OP_PREPARE_STATE)
    {
        g_fcl_data_str.deviceInfo_str.matSel_u16  = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16);

        /* Check if suspend is requested */
        if (R_FCL_FALSE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu)
        {

            /* Switch dto COde or Data Flash mode according to the command */
            if ((g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_LOCKBIT) ||              /* PRQA S 2814 */
                (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_LOCKBIT) ||
                (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_ERASE) ||
                (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_WRITE))
            {
                ret = R_FCL_FCUFct_SwitchMode_Start (R_FCL_WRITE_MODE);
            }
            
            /* Extra area operations */
            else
            {
                ret = R_FCL_FCUFct_SwitchMode_Start (R_FCL_CFG_WRITE_MODE);
            }

            /* Switch mode error check */
            if (ret == R_FCL_OK)
            {
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_PE_MODE_SWITCH_STATE;
                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
            }
            else
            {
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
                g_fcl_data_str.opVar_str.opResult_enu = ret;
            }
        }
        else
        {
            g_fcl_data_str.suspendInfo_str.matSel_u16 = g_fcl_data_str.deviceInfo_str.matSel_u16;
            g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = g_fcl_data_str.opVar_str.opStatus_enu;
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
        }
    }
    
    /* Multi operation state 2: wait for switch mode confirmation */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_CHECK_PE_MODE_SWITCH_STATE)
    {
        /* Check if the mode is switched */
        ret = R_FCL_FCUFct_SwitchMode_Check();
        if (R_FCL_OK == ret)
        {
            /* Treatment of cancel request? */
            if (R_FCL_FALSE == g_fcl_data_str.cancelReq_enu)
            {

                /* No Treatment of suspend request? */
                if (R_FCL_FALSE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu)
                {
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_START_OP_STATE;
                }
                else
                {
                    g_fcl_data_str.suspendInfo_str.matSel_u16 = g_fcl_data_str.deviceInfo_str.matSel_u16;
                    g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                }
            }
            
            /* Cancel request issued */
            else
            {
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                
                ret = R_FCL_FCUFct_ForcedStop ();
            
                /* timeout in forced stop processing */
                if (R_FCL_OK != ret)
                {
                    R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE);
                }
            }
        }
    }
    
    /* Multi operation state 3: prepare the operation */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_START_OP_STATE)
    {

        /* No cancel request to be handled */
        if (R_FCL_FALSE == g_fcl_data_str.cancelReq_enu)
        {

            /* Suspend operation to be handled? */
            if (R_FCL_FALSE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu)
            {

                /* start a new Code Flash operation */
                if ((g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_LOCKBIT) ||
                    (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_LOCKBIT) ||
                    (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_ERASE) ||
                    (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_WRITE))
                {

                    /* Enable / Disable lock bit protection */
                    if (g_fcl_data_str.deviceInfo_str.enableLockBits_enu == R_FCL_TRUE)
                    {
                        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FPROTR_U16,
                                                  R_FPSYS_REGBIT_FPROTR_ON + R_FPSYS_REGBIT_FPROTR_KEY);
                    }
                    else
                    {
                        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FPROTR_U16,
                                                  R_FPSYS_REGBIT_FPROTR_OFF + R_FPSYS_REGBIT_FPROTR_KEY);
                    }

                    /* select proper area: user area or extended user area */
                    if (g_fcl_data_str.opVar_str.opAddr_u32 < R_FLI_USERBOOT_AREA_START)
                    {
                        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                                  R_FPSYS_REGBIT_FMATSELC_USS + R_FPSYS_REGBIT_FMATSELC_KEY);
                    }
                    else
                    {
                        R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                                  R_FPSYS_REGBIT_FMATSELC_EX2S + R_FPSYS_REGBIT_FMATSELC_KEY);
                    }
                }
                
                R_FCL_I_write_memory_u32 (R_FPSYS_REGADD_FSADR_U32, g_fcl_data_str.opVar_str.opAddr_u32);
                
                /* Additional configurations for the different operations */
                if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_ERASE)
                {
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_ERASE);
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_EXE);
                    g_fcl_data_str.opVar_str.opCnt_u16 -= 1u;
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
                }
                
                else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_WRITE)
                {
                    /* setup sequencer to write */
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_WRITE);
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FLI_BLOCK_TRANSFERSIZE);

                    /* Default next state */
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_START_WRITE_OP_STATE;

                    /* copy first half of the data to write */
                    ret = R_FCL_OK;
                    for (i = 0u; ( (i < R_FLI_BLOCK_TRANSFERSIZE_PART) && (R_FCL_OK == ret) ); i++)
                    {
                        /* assume reading of unaligned source buffer */
                        data = (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32 + 1u);
                        data <<= 8;
                        data += (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32);
                        g_fcl_data_str.opVar_str.buffAddr_u32 += 2u;
                        R_FCL_I_write_memory_u16 (R_FPSYS_FLASH_ACCESS_AREA_BASE, data);
                        
                        ret = R_FCL_FCUFct_Chk_DBFull ();

                        /* timeout in DBFull check */
                        if (R_FCL_OK != ret)
                        {
                            /* On DBFULL timeout error, FACI cannot be switched to read mode if the sequence of write command is
                               not completed. In this case, we can only return internal error, but CF remains switched off */
                            R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
                        }
                    }
                }
                else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_LOCKBIT)
                {
                    /* setup sequencer to read lockbits */
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_READ_LOCKBIT);
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_EXE);
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
                }
                else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_LOCKBIT)
                {
                    /* setup sequencer to read lockbits */
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_WRITE_LOCKBIT);
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_EXE);
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
                }
                
                /* OPT or config area write */
                else
                {
                    if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_OTP)
                    {
                        R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_PRG_CFG_OTP);
                    }
                    else
                    {
                        R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_PRG_CFG);
                    }
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_PRG_CFG_TRANSFERSIZE);

                    /* Transfer the write data */
                    for (i = 0u; i < R_PRG_CFG_TRANSFERSIZE; i++)
                    {
                        /* assume reading of unaligned source buffer */
                        data = (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32 + 1u);
                        data <<= 8;
                        data += (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32);
                        g_fcl_data_str.opVar_str.buffAddr_u32 += 2u;
                        R_FCL_I_write_memory_u16 (R_FPSYS_FLASH_ACCESS_AREA_BASE, data);
                    }
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_EXE);
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
                }
            }
            else
            {
                g_fcl_data_str.suspendInfo_str.matSel_u16 = g_fcl_data_str.deviceInfo_str.matSel_u16;
                g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
            }
        }
        
        /* Cancel request to be handled */
        else
        {
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;

            ret = R_FCL_FCUFct_ForcedStop();
            
            /* timeout in forced stop processing */
            if (R_FCL_OK != ret)
            {
                R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE);
            }
        }
    }

    /* Multi operation state 4: specific for the write command: transfer the 2nd half of write data */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_START_WRITE_OP_STATE)
    {
        /* copy second half of the data to write */
        ret = R_FCL_OK;
        for (i = R_FLI_BLOCK_TRANSFERSIZE_PART; ( (i < R_FLI_BLOCK_TRANSFERSIZE) &&
                                                  (R_FCL_OK == ret) ); i++)
        {
            /* assume reading of unaligned source buffer */
            data = (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32 + 1u);
            data <<= 8;
            data += (uint16_t)R_FCL_I_read_memory_u08 (g_fcl_data_str.opVar_str.buffAddr_u32);
            g_fcl_data_str.opVar_str.buffAddr_u32 += 2u;
            R_FCL_I_write_memory_u16 (R_FPSYS_FLASH_ACCESS_AREA_BASE, data);

            ret = R_FCL_FCUFct_Chk_DBFull ();
        }
        
        /* start Flash operation if the data copy worked */
        if (R_FCL_OK == ret)
        {
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
            R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_EXE);
        }
        
        /* Timeout in DBFull check */
        else
        {
            /* On DBFULL timeout error, FACI cannot be switched to read mode if the sequence of write command is
               not completed. In this case, we can only return internal error, but CF remains switched off */
 
            R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
        }
    }

    /* Multi operation state 5: Result check */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_CHECK_RESULT_STATE)
    {

        /* No cancel request to handle */
        if (R_FCL_FALSE == g_fcl_data_str.cancelReq_enu)
        {
            isCmdSpdExec = R_FCL_FALSE;

            /* suspend request to handle */
            if ( (R_FCL_TRUE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu) &&
                 (g_fcl_data_str.suspendInfo_str.reqSuspend_pstr == R_FCL_REQUEST_POINTER_UNDEFINED) )
            {
                g_fcl_data_str.suspendInfo_str.matSel_u16 = R_FCL_I_read_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16);
                regFSTATR = R_FCL_I_read_memory_u32 (R_FPSYS_REGADD_FSTATR_U32);

                /* physical operation ready to suspend? */
                if (R_FPSYS_REGBIT_FSTATR_SUSRDY == (regFSTATR & R_FPSYS_REGBIT_FSTATR_SUSRDY))
                {
                    R_FCL_I_write_memory_u08 (R_FPSYS_FLASH_ACCESS_AREA_BASE, R_FPSYS_FLASH_ACCESS_CMD_SUSPEND);
                    g_fcl_data_str.suspendInfo_str.reqSuspend_pstr = g_fcl_data_str.reqInt_pstr;
                    isCmdSpdExec = R_FCL_TRUE;
                }
            }
        
            ret = R_FCL_FCUFct_GetResult (isCmdSpdExec);
            
            /* check if sequencer is still busy */
            if (ret != R_FCL_BUSY)
            {

                /* was the previous sequencer command a suspend? */
                if ((g_fcl_data_str.suspendInfo_str.reqSuspend_pstr != R_FCL_REQUEST_POINTER_UNDEFINED) &&
                    (R_FCL_TRUE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu))
                {

                    /* operation result check */
                    if (ret != R_FCL_OK)
                    {
                        g_fcl_data_str.opVar_str.opResult_enu = ret;
                        g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu = R_FCL_FALSE;
                    }
                    else
                    {
                        g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_BUSY_CHECK_RESULT_STATE;
                    }
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                }
                
                /* normal/error operation result end */
                else
                {
                    g_fcl_data_str.opVar_str.opResult_enu = ret;
                    
                    /* Result not OK */
                    if (g_fcl_data_str.opVar_str.opResult_enu != R_FCL_OK)
                    {
                        /* an error occured -> stop operation */
                        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                    }
                    else
                    {

                        /* Write operation to be continued (Multi Write)? */
                        if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_WRITE)
                        {
                            /* Use local variable to remove IAR warning */
                            readData = g_fcl_data_str.reqInt_pstr->idx_u32;
    
                            /* still some data to write? */
                            if ((g_fcl_data_str.opVar_str.opAddr_u32 + R_FLI_MIN_WRITESIZE) <
                                (readData + ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) * R_FLI_MIN_WRITESIZE)))
                            {
                                g_fcl_data_str.opVar_str.opAddr_u32 += R_FLI_MIN_WRITESIZE;
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_START_OP_STATE;
                                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
                            }
                            
                            /* operation finished */
                            else
                            {
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                            }
                        }
                        
                        /* Erase operation to be continued (Multi Erase)? */
                        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_ERASE)
                        {
                            /* still some blocks to erase? */
                            if (g_fcl_data_str.opVar_str.opCnt_u16 > 0u)
                            {
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_START_OP_STATE;
                                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
                                /* Use local variable to remove IAR warning */
                                readData = (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK);

                                /* distinguish what code flash block was used: 8kb, 32KB? */
                                if ((readData +
                                    ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) - (uint32_t)(g_fcl_data_str.opVar_str.opCnt_u16)))
                                     <= (uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8))
                                {
                                    /* last block was a 8kb block */
                                    g_fcl_data_str.opVar_str.opAddr_u32 += R_FLI_SMALL_BLOCK_SIZE;
                                }
                             
                                /* crossing Code Flash banks */
                                else if ((readData +
                                     ((uint32_t)(g_fcl_data_str.reqInt_pstr->cnt_u16) - (uint32_t)(g_fcl_data_str.opVar_str.opCnt_u16)))
                                     == ((uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8) +
                                         g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32))
                                {
                                    
                                    g_fcl_data_str.opVar_str.opAddr_u32 = R_FLI_CFLASH2_START;
                                }
                                
                                /* last block was a 32kb block */
                                else
                                {
                                    g_fcl_data_str.opVar_str.opAddr_u32 += R_FLI_BLOCK_SIZE;
                                }
                            }
                            
                            /* operation finished */
                            else
                            {
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                            }
                        }
                        
                        /* Get Lock Bit command result return */
                        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_GET_LOCKBIT)
                        {
                            readData = (uint32_t)R_FCL_I_read_memory_u08 (R_FPSYS_REGADD_FLKSTAT_U8);
                            readData = ((~readData) & (uint32_t)0x01u);
                            R_FCL_I_write_unaligned_memory_u32(g_fcl_data_str.reqInt_pstr->bufferAdd_u32, readData);
                            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                        }
                        
                        /* Set Lock Bit command */
                        else if (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_LOCKBIT)
                        {
                            /* operation finished */
                            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                        }

                        /* Security flag write commands */
                        else if ( (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_READ_PROTECT_FLAG) ||
                                  (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_WRITE_PROTECT_FLAG) ||
                                  (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_ERASE_PROTECT_FLAG) ||
                                  (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_SERIAL_PROG_DISABLED) ||
                                  (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_SET_SERIAL_ID_ENABLED) )
                        {
                            /* Update the security data after successful set operation */
                            g_fcl_data_str.deviceInfo_str.secureData_u32 =
                                    (uint32_t)g_fcl_data_str.intBuf_au08[0] +
                                ( ( (uint32_t)g_fcl_data_str.intBuf_au08[1] ) << 8 ) +
                                ( ( (uint32_t)g_fcl_data_str.intBuf_au08[2] ) << 16 ) +
                                ( ( (uint32_t)g_fcl_data_str.intBuf_au08[3] ) << 24 );
                            
                            /* operation finished */
                            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                        }
                        
                        /* All remaining Data Flash write operations */
                        else
                        {
                            g_fcl_data_str.opVar_str.opCnt_u16 -= R_PRG_CFG_TRANSFERSIZE;

                            /* still some settings to write? */
                            if (g_fcl_data_str.opVar_str.opCnt_u16 > 0u)
                            {
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_START_OP_STATE;
                                g_fcl_data_str.opVar_str.opResult_enu = R_FCL_BUSY;
                                g_fcl_data_str.opVar_str.opAddr_u32 += (2u * R_PRG_CFG_TRANSFERSIZE);
                            }
                            
                            /* operation finished */
                            else
                            {
                                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;
                            }
                        }
                    }
                }
            }
        }

        /* Cancel request to handle */
        else
        {
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_SWITCH_READ_MODE_STATE;

            ret = R_FCL_FCUFct_ForcedStop ();
        
            /* timeout in forced stop processing */
            if (R_FCL_OK != ret)
            {
                R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE);
            }
        }
        
        /* Operation finished? */
        if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_SWITCH_READ_MODE_STATE)
        {
            R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                      g_fcl_data_str.deviceInfo_str.matSel_u16 + R_FPSYS_REGBIT_FMATSELC_KEY);
            R_FCL_FCUFct_ClearStatus ();
        }
    }
    
    /* Multi operation state 6: Start switch to read mode */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_SWITCH_READ_MODE_STATE)
    {
        (void)R_FCL_FCUFct_SwitchMode_Start (R_FCL_READ_MODE);
        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_READ_MODE_SWITCH_STATE;
    }
    
    /* Multi operation state 7: Switch mode check */
    else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_BUSY_CHECK_READ_MODE_SWITCH_STATE)
    {
        ret = R_FCL_FCUFct_SwitchMode_Check ();
        if (R_FCL_OK == ret)
        {
            g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
        }
    }
    
    /* Multi operation state 8: Finish state */
    else /* (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_OP_FINISHED_STATE) */
    {

        /* No cancel request to handle */
        if (R_FCL_FALSE == g_fcl_data_str.cancelReq_enu)
        {

            /* No suspend request to handle */
            if (R_FCL_FALSE == g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu)
            {
                ret = R_FCL_OK;
                if ((g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_ERASE) ||
                    (g_fcl_data_str.reqInt_pstr->command_enu == R_FCL_CMD_WRITE)  )
                {
                    ret = R_FCL_FCUFct_Clear_Cache ();
                    
                    /* timeout in cache clearing */
                    if (R_FCL_OK != ret)
                    {
                        /* This is also for loop break condition */
                        R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
                    }
                }
                
                if (R_FCL_OK == ret)
                {
                    /* finish operation */
                    g_fcl_data_str.reqInt_pstr->status_enu = g_fcl_data_str.opVar_str.opResult_enu;
                    R_FCL_FCUFct_RestoreLastState ();
                    g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_IDLE_STATE;
                }
            }
            
            /* Suspend request to handle */
            else
            {
                R_FCL_I_write_memory_u16 (R_FPSYS_REGADD_FMATSELC_U16,
                                          g_fcl_data_str.deviceInfo_str.matSel_u16 + R_FPSYS_REGBIT_FMATSELC_KEY);

                /* save proper state for an operation that was suspended before it physically started */
                if ((g_fcl_data_str.suspendInfo_str.reqSuspend_pstr == R_FCL_REQUEST_POINTER_UNDEFINED) &&
                    (g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu == R_FCL_OP_IDLE_STATE))
                {
                    g_fcl_data_str.suspendInfo_str.opSuspend_str.opStatus_enu = R_FCL_OP_FINISHED_STATE;
                }
                g_fcl_data_str.suspendInfo_str.reqSuspend_pstr            = g_fcl_data_str.reqInt_pstr;
                g_fcl_data_str.suspendInfo_str.opSuspend_str.opAddr_u32   = g_fcl_data_str.opVar_str.opAddr_u32;
                g_fcl_data_str.suspendInfo_str.opSuspend_str.opCnt_u16    = g_fcl_data_str.opVar_str.opCnt_u16;
                g_fcl_data_str.suspendInfo_str.opSuspend_str.opResult_enu = g_fcl_data_str.opVar_str.opResult_enu;
                
                g_fcl_data_str.intStatus_enu                     = R_FCL_ISTAT_SUSPENDED;
                g_fcl_data_str.reqInt_pstr->status_enu           = R_FCL_SUSPENDED;
                g_fcl_data_str.suspendInfo_str.flagSuspended_enu = R_FCL_TRUE;
                g_fcl_data_str.suspendInfo_str.spdSpdRequest_enu = R_FCL_FALSE;
            }
        }
        
        /* Cancel request to handle */
        else
        {
            skipCode = R_FCL_FALSE;
            ret      = R_FCL_CANCELLED;

            /* Check if a suspended command exists that need to be cancelled too */
            if (R_FCL_REQUEST_POINTER_UNDEFINED != g_fcl_data_str.suspendInfo_str.reqSuspend_pstr)
            {
 
                /* Check if there is a first command that is suspended */
                if (R_FCL_FALSE != R_FCL_FCUFct_ChkSuspended())
                {
                    res = R_FCL_FCUFct_SwitchMode_Start (R_FCL_WRITE_MODE);
                    if (R_FCL_OK == res)
                    {
                        g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_BUSY_CHECK_PE_MODE_SWITCH_STATE;
                    }
                    else
                    {
                        R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_SWITCH_READ_STATE);
                    }
                    skipCode = R_FCL_TRUE;
                }
            }
        
            /* the below code block is skipped in case of cancelling an on-going suspend request */
            if (R_FCL_FALSE == skipCode)
            {
                res = R_FCL_FCUFct_Clear_Cache ();
                
                /* timeout in cache clearing */
                if (R_FCL_OK != res)
                {
                    R_FCL_FCUFct_HandleMultiOperation_IntErr (R_FCL_OP_INTERNAL_ERROR_STATE);
                }
                else
                {
                    g_fcl_data_str.intStatus_enu           = R_FCL_ISTAT_PREPARED;
                    g_fcl_data_str.reqInt_pstr->status_enu = ret;
                    g_fcl_data_str.opVar_str.opStatus_enu  = R_FCL_OP_IDLE_STATE;
                    g_fcl_data_str.cancelReq_enu           = R_FCL_FALSE;
                    if (R_FCL_REQUEST_POINTER_UNDEFINED != g_fcl_data_str.suspendInfo_str.reqSuspend_pstr)
                    {
                        g_fcl_data_str.suspendInfo_str.reqSuspend_pstr->status_enu = ret;
                        g_fcl_data_str.suspendInfo_str.reqSuspend_pstr = R_FCL_REQUEST_POINTER_UNDEFINED;
                        g_fcl_data_str.suspendInfo_str.flagSuspended_enu = R_FCL_FALSE;
                    }
                } /* else - if (R_FCL_OK != res) */
            } /* if (R_FCL_FALSE == skipCode) */
        } /* if (R_FCL_FALSE == g_fcl_data_str.cancelReq_enu) */
    } /* else if (g_fcl_data_str.opVar_str.opStatus_enu == R_FCL_OP_FINISHED_STATE) */


} /* R_FCL_FCUFct_HandleMultiOperation */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_HandleMultiOperation
 *********************************************************************************************************************/


#ifdef R_FCL_SUPPORT_OTP
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_GetOTPBit
 *********************************************************************************************************************/
/**
 * This function reads out the OTP bit of a given block
 *
 * @param[in,out] ---
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_GetOTPBit (void)                                                                 /* PRQA S 1503 */
{
    uint8_t  readData[16];
    uint8_t  data;
    uint32_t buffAddr;

    buffAddr = g_fcl_data_str.reqInt_pstr->bufferAdd_u32;

    /* User boot area to be handled */
    if (R_FCL_USERBOOT_AREA_SELECT == (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FCL_USERBOOT_AREA_SELECT))
    {

        /* User boot area parameter check */
        if (  ((g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK) <
                g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32)
           && (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE) )
        {
            g_fcl_data_str.reqInt_pstr->bufferAdd_u32 = (uint32_t)&readData[0];                    /* PRQA S 0306 */
            g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_UB_OFFSET;
            R_FCL_Fct_CallFromRAM ();
            data = (~(readData[0]));                                                               /* PRQA S 2961 */
            g_fcl_data_str.reqInt_pstr->bufferAdd_u32 = buffAddr;
            R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, (uint32_t)data);
        }
        else
        {
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
        }
    }
    
    /* User area to be handled */
    else
    {

        /* User area parameter check */
        if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.blkCnt_u16 )
        {
            g_fcl_data_str.reqInt_pstr->bufferAdd_u32 = (uint32_t)&readData[0];                    /* PRQA S 0306 */

            /* Code Flash bank A or B to be handled? */
            if (g_fcl_data_str.reqInt_pstr->idx_u32 >
               ((uint32_t)g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 + g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32))
            {
                g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_CF2_OFFSET +
                    (0x10u * ((g_fcl_data_str.reqInt_pstr->idx_u32 -
                    ((uint32_t)g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                     g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32)) / 128u));
            }
            else
            {
                g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_CF1_OFFSET +
                    (0x10u * (g_fcl_data_str.reqInt_pstr->idx_u32 / 128u));
            }
            R_FCL_Fct_CallFromRAM ();
            
            /* We have 8k Flash blocks */
            if (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 > 0u)
            {
                data = readData[(g_fcl_data_str.reqInt_pstr->idx_u32 / 8u) % 16u];
                data = (((uint8_t)(~data)) >> (g_fcl_data_str.reqInt_pstr->idx_u32 % 8u)) & 0x01u;
            }
            
            /* We have no 8k Flash blocks */
            else
            {
                if (g_fcl_data_str.reqInt_pstr->idx_u32 == 0u)
                {
                    data = ((uint8_t)(~(readData[0]))) & 0x01u;
                }
                else if (g_fcl_data_str.reqInt_pstr->idx_u32 == 1u)
                {
                    data = (((uint8_t)(~(readData[0]))) >> 4) & 0x01u;
                }
                else
                {
                    data = readData[((g_fcl_data_str.reqInt_pstr->idx_u32 + 6u) / 8u) % 16u];
                
                    data = (((uint8_t)(~data)) >> ((g_fcl_data_str.reqInt_pstr->idx_u32 + 6u) % 8u)) & 0x01u;
                }
            }
            g_fcl_data_str.reqInt_pstr->bufferAdd_u32 = buffAddr;
            R_FCL_I_write_unaligned_memory_u32 (g_fcl_data_str.reqInt_pstr->bufferAdd_u32, (uint32_t)data);
        }
        else
        {
            g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
        }
    }
} /* R_FCL_FCUFct_GetOTPBit */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_GetOTPBit
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_SetOTPBit
 *********************************************************************************************************************/
/**
 * This function writes the OTP bit of a given block
 *
 * @param[in,out] ---
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_SetOTPBit (void)                                                                 /* PRQA S 1503 */
{
    uint8_t     i;
    uint32_t    blockNo;

    /* Emulation mode */
    if (g_fcl_data_str.deviceInfo_str.emulationMode_enu == R_FCL_TRUE)                             /* CCOV 0006 */
    {
        /* do not set OTP bits in emulation mode, but return R_FCL_OK */
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;
    }
    
    /* No normal operation mode */
    else
    {
        for (i = 0u; i < 16u; i++)
        {
            g_fcl_data_str.intBuf_au08[i] = 0xffu;
        }
        
        g_fcl_data_str.opVar_str.buffAddr_u32 = (uint32_t)(&g_fcl_data_str.intBuf_au08[0]);        /* PRQA S 0306 */

        /* User boot area to be handled */
        if (R_FCL_USERBOOT_AREA_SELECT == (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FCL_USERBOOT_AREA_SELECT))
        {

            /* Parameter check on the user boot area */
            if (  ((g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK) <
                    g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32)
               && (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE) )
            {
                g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_UB_OFFSET;
                g_fcl_data_str.intBuf_au08[0] = (uint8_t)(~0x01u);
                R_FCL_FCUFct_SetConfigArea ();
            }
            else
            {
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }
        
        /* User area to be handled */
        else
        {
        
            /* No parameter error */
            if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.blkCnt_u16)
            {
                /* Use local variable to remove IAR warning */
                blockNo = g_fcl_data_str.reqInt_pstr->idx_u32;

                if (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 > 0u)
                {
                    g_fcl_data_str.intBuf_au08[(blockNo / 8u) % 16u] = ~(0x01u << (blockNo % 8u));
                }
                else
                {

                    /* clear the bit in the write data acc. to the addressed Flash block */
                    if (blockNo  == 0u)
                    {
                        g_fcl_data_str.intBuf_au08[0] = (uint8_t)(~0x01u);
                    }
                    else if (blockNo == 1u)
                    {
                        g_fcl_data_str.intBuf_au08[0] = (uint8_t)(~0x10u);
                    }
                    else
                    {
                        g_fcl_data_str.intBuf_au08[((blockNo + 6u) / 8u) % 16u] = (~(0x01u << ((blockNo + 6u) % 8u)));
                    }
                }

                /* is it a 32kb code flash 1 block? */
                if (g_fcl_data_str.reqInt_pstr->idx_u32 >
                    ((uint32_t)g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                     g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32))
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_CF2_OFFSET +
                        (0x10u * ((g_fcl_data_str.reqInt_pstr->idx_u32 -
                        ((uint32_t)g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                         g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32)) / 128u));
                }
                else
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 = R_FPSYS_CONFIG_OTP_CF1_OFFSET +
                        (0x10u * (g_fcl_data_str.reqInt_pstr->idx_u32 / 128u));
                }
                R_FCL_FCUFct_SetConfigArea ();
            }
            else
            {
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }
    }
} /* R_FCL_FCUFct_SetOTPBit */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_SetOTPBit
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_OTP */


#ifdef R_FCL_SUPPORT_LOCKBIT
/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_HandleLockBit
 *********************************************************************************************************************/
/**
 * This function prepares internal variables to read out the lock bit of a given block
 *
 * @param[in,out] ---
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_HandleLockBit (void)                                                             /* PRQA S 1503 */
{
    /* FLMD0 pin is not set */
    if (R_FCL_FCUFct_CheckMode () == R_FCL_FALSE)
    {
        g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_FLMD0;                                  /* PRQA S 2814 */
    }
    else
    {

        /* User boot area addressed */
        if (R_FCL_USERBOOT_AREA_SELECT == (g_fcl_data_str.reqInt_pstr->idx_u32 & R_FCL_USERBOOT_AREA_SELECT))
        {
 
            /* User boot area parameter check */
            if (  ((g_fcl_data_str.reqInt_pstr->idx_u32 & R_FLI_USERBOOT_AREA_MASK) <
                    g_fcl_data_str.deviceInfo_str.usrBlkCnt32kB_u32)
               && (g_fcl_data_str.deviceInfo_str.accessExtendedUserArea_enu != R_FCL_FALSE) )
            {
                /* setup internal variables for multi operation */
                g_fcl_data_str.opVar_str.opAddr_u32 = R_FLI_USERBOOT_AREA_START +
                                                        ((g_fcl_data_str.reqInt_pstr->idx_u32 &
                                                          R_FLI_USERBOOT_AREA_MASK) * R_FLI_BLOCK_SIZE);
                g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_BUSY;
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
            }
            else
            {
                /* parameter error */
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }

        /* User area addressed */
        else
        {

            /* No user area parameter error */
            if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.blkCnt_u16 )
            {

                /* setup internal variables for multi operation */
                if (g_fcl_data_str.reqInt_pstr->idx_u32 < g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8)
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 =
                        g_fcl_data_str.reqInt_pstr->idx_u32 * R_FLI_SMALL_BLOCK_SIZE;
                }

                /* is it a 32kb code flash 1 block? */
                else if ( g_fcl_data_str.reqInt_pstr->idx_u32 <
                          (g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 +
                           g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32))
                {
                    g_fcl_data_str.opVar_str.opAddr_u32 =
                        ((uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8) * R_FLI_SMALL_BLOCK_SIZE) +
                        ( (g_fcl_data_str.reqInt_pstr->idx_u32 - (uint32_t)(g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8))
                          * R_FLI_BLOCK_SIZE);
                }
                else
                {
                    /* Bank B available */
                    g_fcl_data_str.opVar_str.opAddr_u32 = R_FLI_CFLASH2_START +
                       ((g_fcl_data_str.reqInt_pstr->idx_u32 - g_fcl_data_str.deviceInfo_str.cf1BlkCnt8kB_u8 -
                         g_fcl_data_str.deviceInfo_str.cf1BlkCnt32kB_u32) * R_FLI_BLOCK_SIZE);
                }
                g_fcl_data_str.intStatus_enu = R_FCL_ISTAT_BUSY;
                g_fcl_data_str.opVar_str.opStatus_enu = R_FCL_OP_PREPARE_STATE;
            }
            else
            {
                g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_ERR_PARAMETER;
            }
        }
    }
} /* R_FCL_FCUFct_HandleLockBit */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_HandleLockBit
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function name: R_FCL_FCUFct_LockBitMode
 *********************************************************************************************************************/
/**
 * This function enables or disables the lockbit protection mechanism
 *
 * @param[in]     R_FPSYS_REGBIT_FPROTR_OFF     disables lockbit protection
 *                R_FPSYS_REGBIT_FPROTR_ON      enables lockbit protection
 * @return        ---
 */
/*********************************************************************************************************************/
#if R_FCL_COMPILER == R_FCL_COMP_GHS
    #pragma ghs section text = ".R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
    #pragma location = "R_FCL_CODE_ROMRAM"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
    #ifdef __PIC
        #pragma section pctext "R_FCL_CODE_ROMRAM"
    #else
        #pragma section text "R_FCL_CODE_ROMRAM"
    #endif
#endif
void R_FCL_FCUFct_LockBitMode (uint16_t mode_u16)                                                  /* PRQA S 1503 */
{
    if (mode_u16 != R_FPSYS_REGBIT_FPROTR_OFF)
    {
        g_fcl_data_str.deviceInfo_str.enableLockBits_enu = R_FCL_TRUE;
    }
    else
    {
        g_fcl_data_str.deviceInfo_str.enableLockBits_enu = R_FCL_FALSE;
    }
    g_fcl_data_str.reqInt_pstr->status_enu = R_FCL_OK;                                             /* PRQA S 2814 */
} /* R_FCL_FCUFct_LockBitMode */
/**********************************************************************************************************************
 End of function R_FCL_FCUFct_LockBitMode
 *********************************************************************************************************************/
#endif /* ifdef R_FCL_SUPPORT_LOCKBIT */

/* @MRTZ_endBlock */
