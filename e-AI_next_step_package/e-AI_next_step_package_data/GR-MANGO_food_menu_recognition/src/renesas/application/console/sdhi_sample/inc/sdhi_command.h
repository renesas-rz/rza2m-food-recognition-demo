/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/*******************************************************************************
* File Name    : sdhi_command.h
* Description  : SD Driver Sample Program
******************************************************************************/
/*****************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 15.04.2019 1.03     Support for SDIO
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "r_typedefs.h"
#include "r_sdif.h"

#ifndef SDHI_COMMAND_H
#define SDHI_COMMAND_H

/******************************************************************************
Macro definitions
******************************************************************************/
#define SD_TEST_SECTOR_NUM          (512)
#define SD_RW_BUFF_SIZE             (1 * 1024)
#define SDIO_CIS_BUFF_SIZE          (256)

/******************************************************************************
Variable Externs
******************************************************************************/
extern uint8_t  g_sdio_cis[SDIO_CIS_BUFF_SIZE];

/******************************************************************************
Functions Prototypes
******************************************************************************/

/* Function Name: sd_int_callback */
/**************************************************************************//**
 * @fn            int32_t sd_int_callback(int32_t sd_port, int32_t cd)
 * @brief         Interrupt callback function.
 * @warning       .
 * @param [in]    sd_port - channel no (0 or 1)
 * @param [in]    cd - SD card insert or extract
 * @retval        CMD_OK for success
 *****************************************************************************/
int32_t sd_int_callback(int32_t sd_port, int32_t cd);

/* Function Name: sdio_int_callback */
/**************************************************************************//**
 * @fn            int32_t sdio_int_callback(int32_t sd_port)
 * @brief         .
 * @warning       SDIO Interrupt callback function.
 * @param [in]    sd_port - channel no (0 or 1)
 * @retval        CMD_OK for success
 *****************************************************************************/
int32_t sdio_int_callback(int32_t sd_port);

/* Function Name: sdio_check_sdio_interrupt_flag */
/**************************************************************************//**
 * @fn            sdio_check_sdio_interrupt_flag(void)
 * @brief         Check if the SDIO interrupt signal is set.
 * @warning       .
 * @param [in]    none
 * @retval        none
 *****************************************************************************/
void    sdio_check_sdio_interrupt_flag(void);

#endif /* SDHI_COMMAND_H */
