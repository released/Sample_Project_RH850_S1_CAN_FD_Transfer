#ifdef ENABLE_QAC_TEST
    #pragma PRQA_MESSAGES_OFF 0292
#endif
/**********************************************************************************************************************
* Library       : Code Flash Access Library for Renesas RH850 devices, based on the RV40 Flash technology
*
* File Name     : $Source: r_fcl_types.h $
* Lib. Version  : $RH850_FCL_LIB_VERSION_T01: V2.13 $
* Mod. Revision : $Revision: 1.22 $
* Mod. Date     : $Date: 2019/04/18 13:13:49JST $
* Device(s)     : RV40 Flash based RH850 microcontroller
* Description   : User interface type definitions
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

#ifndef R_FCL_TYPES_H
#define R_FCL_TYPES_H

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/
#define R_FCL_NULL      0x00000000u

/* Global compiler definition */
#define R_FCL_COMP_GHS         (1)
#define R_FCL_COMP_IAR         (2)
#define R_FCL_COMP_REC         (3)

#if defined (__IAR_SYSTEMS_ASM__)
    #define R_FCL_COMPILER R_FCL_COMP_IAR
#elif defined (__IAR_SYSTEMS_ICC__)
    #define R_FCL_COMPILER R_FCL_COMP_IAR
#elif defined (__v850e3v5__)
    #define R_FCL_COMPILER R_FCL_COMP_REC
#else /*GHS */
    #define R_FCL_COMPILER R_FCL_COMP_GHS
#endif

/**********************************************************************************************************************
Support device macro definition
**********************************************************************************************************************/

#if ((defined R_FCL_MIRROR_FCU_COPY) && (defined R_FCL_NO_FCU_COPY))
    #error "r_fcl_types.h: Invalid define for support device macro"
#elif ((defined R_FCL_MIRROR_FCU_COPY) && (defined R_FCL_NO_BFA_SWITCH))
    #error "r_fcl_types.h: Invalid define for support device macro"
#elif ((defined R_FCL_NO_FCU_COPY) && (defined R_FCL_NO_BFA_SWITCH))
    #error "r_fcl_types.h: Invalid define for support device macro"
#elif (defined R_FCL_MIRROR_FCU_COPY)
    #define R_FCL_INNER_NO_BFA_SWITCH
#elif (defined R_FCL_NO_FCU_COPY)
    #define R_FCL_INNER_NO_FCU_COPY
#elif (defined R_FCL_NO_BFA_SWITCH)
    #define R_FCL_INNER_NO_BFA_SWITCH
    #define R_FCL_INNER_NO_FCU_COPY
#endif

/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/
/**
   @struct r_fcl_descriptor_t
   FCL descriptor variable definition.
*/
typedef struct R_FCL_DESCRIPTOR_T
{
    uint32_t    id_au32[4];               /**< Authentication ID array               */
    uint32_t    addrRam_u32;              /**< Copy location for FCL routines in RAM */
    uint16_t    frequencyCpuMHz_u16;      /**< Flash hardware frequency in MHz       */
} r_fcl_descriptor_t;

/**
   @enum r_fcl_status_t
   FCL status return values
*/
typedef enum R_FCL_STATUS_T
{
    R_FCL_OK,                             /**< Operation terminated successfully                    */
    R_FCL_BUSY,                           /**< Operation is still ongoing                           */
    R_FCL_SUSPENDED,                      /**< Operation is suspended                               */
    R_FCL_ERR_FLMD0,                      /**< FLMD0-Pin not at high level                          */
    R_FCL_ERR_PARAMETER,                  /**< A parameter of the FCL function call was wrong       */
    R_FCL_ERR_PROTECTION,                 /**< Operation forbidden due to a security feature        */
    R_FCL_ERR_REJECTED,                   /**< Flow error, e.g. another operation is still busy     */
    R_FCL_ERR_FLOW,                       /**< Flow error, e.g. the environment is not yet prepared */
    R_FCL_ERR_WRITE,                      /**< Flash write error                                    */
    R_FCL_ERR_ERASE,                      /**< Flash erase error                                    */
    R_FCL_ERR_COMMAND,                    /**< Unknown command                                      */
    R_FCL_ERR_SUSPEND_FAILED,             /**< Suspend processing failed                            */
    R_FCL_CANCELLED,                      /**< On-going write or erase operation is cancelled       */
    R_FCL_ERR_INTERNAL                    /**< Library internal error                               */
} r_fcl_status_t;

/**
   @enum r_fcl_command_t
   FCL operation commands
*/
typedef enum R_FCL_COMMAND_T
{
    R_FCL_CMD_PREPARE_ENV,                /**< Prepare Flash environment                     */
    R_FCL_CMD_ERASE,                      /**< Flash block erase (Multiple blocks)           */
    R_FCL_CMD_WRITE,                      /**< Flash write (Multiple words)                  */
    R_FCL_CMD_SET_LOCKBIT,                /**< Protect a block against unwanted mofification */
    R_FCL_CMD_GET_LOCKBIT,                /**< Read-out protection setting of a block        */
    R_FCL_CMD_ENABLE_LOCKBITS,            /**< Enable lockbit protection                     */
    R_FCL_CMD_DISABLE_LOCKBITS,           /**< Disable lockbit protection                    */
    R_FCL_CMD_SET_OTP,                    /**< One time programmable                         */
    R_FCL_CMD_GET_OTP,                    /**< Read-out protection setting of a block        */
    R_FCL_CMD_SET_OPB,                    /**< Set option bytes                              */
    R_FCL_CMD_GET_OPB,                    /**< Get option bytes                              */
    R_FCL_CMD_SET_ID,                     /**< Change ID authentification                    */
    R_FCL_CMD_GET_ID,                     /**< Get ID authentification                       */
    R_FCL_CMD_SET_READ_PROTECT_FLAG,      /**< Set Read protection                           */
    R_FCL_CMD_GET_READ_PROTECT_FLAG,      /**< Get Read protection setting                   */
    R_FCL_CMD_SET_WRITE_PROTECT_FLAG,     /**< Set Write protection                          */
    R_FCL_CMD_GET_WRITE_PROTECT_FLAG,     /**< Get Write protection setting                  */
    R_FCL_CMD_SET_ERASE_PROTECT_FLAG,     /**< Set Erase protection                          */
    R_FCL_CMD_GET_ERASE_PROTECT_FLAG,     /**< Get Erase protection setting                  */
    R_FCL_CMD_SET_SERIAL_PROG_DISABLED,   /**< Disable Serial Programmer interface           */
    R_FCL_CMD_GET_SERIAL_PROG_DISABLED,   /**< Get Serial Programmer interface setting       */
    R_FCL_CMD_SET_SERIAL_ID_ENABLED,      /**< Enable Serial Programmer ID authentification  */
    R_FCL_CMD_GET_SERIAL_ID_ENABLED,      /**< Enable Serial Programmer ID authentification  */
    R_FCL_CMD_SET_RESET_VECTOR,           /**< Set variable reset vector                     */
    R_FCL_CMD_GET_RESET_VECTOR,           /**< Get reset vector setting                      */
    R_FCL_CMD_GET_BLOCK_CNT,              /**< Get amount of Flash blocks                    */
    R_FCL_CMD_GET_BLOCK_END_ADDR,         /**< Get end address of a specified Flash block    */
    R_FCL_CMD_GET_DEVICE_NAME             /**< Get name string of device                     */
} r_fcl_command_t;

/**
   @struct r_fcl_request_t
   FCL operations request structure, required for R_FCL_Execute
*/
typedef volatile struct R_FCL_REQUEST_T
{
    r_fcl_command_t     command_enu;      /**< FDL operation, e.g. R_FDL_CMD_ERASE              */
    uint32_t            bufferAdd_u32;    /**< Buffer address for input data or result          */
    uint32_t            idx_u32;          /**< Write operation: \n
                                               Write destination address \n
                                               Erase operation: \n
                                               Block number of the first block to erase \n
                                               Read config area: \n
                                               Start address of area to read                    */
    uint16_t cnt_u16;                     /**< Write operation: \n
                                               Number of units to write (unit = 256Bytes)\n
                                               Erase operation: \n
                                               Number of blocks to erase \n
                                               Read config area: \n
                                               Number of bytes to read                          */
    r_fcl_status_t status_enu;            /**< status return value of the FCL                   */
} r_fcl_request_t;

/**********************************************************************************************************************
Exported global variables
**********************************************************************************************************************/


/**********************************************************************************************************************
Exported global functions (to be accessed by other files)
**********************************************************************************************************************/


#endif /* end of R_FCL_TYPES_H */

