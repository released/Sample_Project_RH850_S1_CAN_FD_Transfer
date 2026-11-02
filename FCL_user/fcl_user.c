#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/**********************************************************************************************************************
* File Name     : $Source: fcl_user.c $
* Mod. Revision : $Revision: 1.8 $
* Mod. Date     : $Date: 2019/04/26 09:54:04JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : Sample application functions to prepare Self-Programming
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


/**********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
**********************************************************************************************************************/
#include "r_typedefs.h"
#include "r_fcl_types.h"
#include "target.h"


/************************************************************************************************************
 * Function name: FCLUser_Open
 ***********************************************************************************************************/
/**
 * Prepare Flash programming hardware for Flash modification operations
 *
 * @param         ---
 * @return        ---
 */
/***********************************************************************************************************/
#if   R_FCL_COMPILER == R_FCL_COMP_GHS
  #pragma ghs section text =".R_FCL_CODE_USR"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
  #pragma location = "R_FCL_CODE_USR"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
#ifdef __PIC
  #pragma section pctext "R_FCL_CODE_USR"
#else
  #pragma section text "R_FCL_CODE_USR"
#endif
#endif
void FCLUser_Open (void)
{
    FCL_INIT_FLASHACCESS
}

/************************************************************************************************************
 * Function name: FCLUser_Close
 ***********************************************************************************************************/
/**
 * Prepare Flash programming hardware for Flash modification operations
 *
 * @param         ---
 * @return        ---
 */
/***********************************************************************************************************/
#if   R_FCL_COMPILER == R_FCL_COMP_GHS
  #pragma ghs section text =".R_FCL_CODE_USR"
#elif R_FCL_COMPILER == R_FCL_COMP_IAR
  #pragma location = "R_FCL_CODE_USR"
#elif R_FCL_COMPILER == R_FCL_COMP_REC
#ifdef __PIC
  #pragma section pctext "R_FCL_CODE_USR"
#else
  #pragma section text "R_FCL_CODE_USR"
#endif
#endif
void FCLUser_Close (void)
{
    FCL_DISABLE_FLASHACCESS
}

