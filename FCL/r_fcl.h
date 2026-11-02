#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/**********************************************************************************************************************
* Library       : Code Flash Access Library for Renesas RH850 devices, based on the RV40 Flash technology
*
* File Name     : $Source: r_fcl.h $
* Lib. Version  : $RH850_FCL_LIB_VERSION_T01: V2.13 $
* Mod. Revision : $Revision: 1.12 $
* Mod. Date     : $Date: 2019/04/18 13:13:36JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : User interface function prototypes
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

#ifndef R_FCL_H
#define R_FCL_H

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Exported global variables
**********************************************************************************************************************/


/**********************************************************************************************************************
Exported global functions (to be accessed by other files)
**********************************************************************************************************************/
/* Copy FCL to RAM */
r_fcl_status_t  R_FCL_CopySections (void);

/* Calculate new address after copy process */
uint32_t        R_FCL_CalcFctAddr (uint32_t addFct_u32);

/* Return of FCL library version string */
const uint8_t * R_FCL_GetVersionString (void);

/* FCL initialization function */
r_fcl_status_t  R_FCL_Init (const r_fcl_descriptor_t * descriptor_pstr);

/* Flash operation initiation */
void            R_FCL_Execute (r_fcl_request_t * request_pstr);

#if R_FCL_COMMAND_EXECUTION_MODE == R_FCL_HANDLER_CALL_USER
    /* Flash operation forwarding and status check */
    void R_FCL_Handler (void);
    
    /* Suspend / Resume handling */
    r_fcl_status_t R_FCL_SuspendRequest (void);
    r_fcl_status_t R_FCL_ResumeRequest  (void);
    
    /* Cancel on-going erase or write operation */
    r_fcl_status_t R_FCL_CancelRequest  (void);
#endif


#endif /* end of R_FCL_H */

