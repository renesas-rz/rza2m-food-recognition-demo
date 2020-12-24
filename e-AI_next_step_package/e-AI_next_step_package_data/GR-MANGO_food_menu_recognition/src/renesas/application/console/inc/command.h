/*******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software
 * and to discontinue the availability of this software. By using this
 * software, you agree to the additional terms and conditions found by
 * accessing the following link:
 * http://www.renesas.com/disclaimer
*******************************************************************************
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
 *****************************************************************************/
/******************************************************************************
 * @headerfile     command.h
 * @brief          The entry point of the main command handler and associated
 *                 commands
 * @version        1.00
 * @date           27.06.2018
 * H/W Platform    RZ/A1LU
 *****************************************************************************/
 /*****************************************************************************
 * History      : DD.MM.YYYY Ver. Description
 *              : 30.06.2018 1.00 First Release
 *****************************************************************************/

/* Multiple inclusion prevention macro */
#ifndef COMMAND_H_
#define COMMAND_H_

/**************************************************************************//**
 * @ingroup RENESAS_APPLICATION_SOFTWARE_PACKAGE Software Package
 * @defgroup R_SW_PKG_93_CONSOLE Console
 * @brief Console implementation for the RZA1LU Camera-SDK Application.
 * @anchor R_SW_PKG_93_CONSOLE_SUMMARY
 * @par Summary
 * @brief Console implementation for the RZA1LU Camera-SDK Application.<br>
 *
 * @anchor R_SW_PKG_93_CONSOLE_INSTANCES
 * @par Known Implementations:
 * This driver is used in the RZA1LU Software Package.
 * @see RENESAS_APPLICATION_SOFTWARE_PACKAGE
 * @{
 *****************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 *****************************************************************************/
#include "console.h"

/******************************************************************************
 Macro definitions
 *****************************************************************************/

/******************************************************************************
 Typedef definitions
 *****************************************************************************/

/******************************************************************************
 Variable External definitions and Function External definitions
 *****************************************************************************/

/******************************************************************************
 Exported global functions (to be accessed by other files)
 *****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _iostr
{
    FILE *p_in;
    FILE *p_out;
} st_iostr_t, * pst_iostr_t;


extern pst_command_table_t gppCommands[];
extern int32_t g_num_commands;
int_t g_rtc_handle;

/**
 * @brief Function to process the console commands
 * @param pIn - Pointer to the input file stream
 * @param pOut - Pointer to the file stream for echo of command input
 * @param pszPrompt - Pointer to a command prompt
 */
void cmd_console(FILE* pIn, FILE *pOut, char_t *pszPrompt);

/**
 * @brief Function to show memory in format <br>
          XX XX XX XX XX XX XX XX : AAAAAAAAAAAAAAAAAA <br>
          where XX is a H rep & AA is ASCII rep of byte <br>
 * @param pbyView - pointer to the memory to display
 * @param bLength - The number of bytes to display
 * @param pOut - Pointer to the file stream to print to
 * @return address of last printed byte
 */
uint8_t * cmd_view_memory(uint8_t *pbyView, uint8_t bLength, FILE *pOut);

/**
 * @brief Function to print the data transfer rate (in engineering units)
 * @param pFile - Pointer to the file to print to
 * @param fTransferTime - The transfer time in seconds
 * @param stLength - The length of data transferred
 */
void cmd_show_data_rate (FILE *pFile, float fTransferTime, size_t stLength);

/**
 * @brief Function to print a memory size in engineering units
 * @param pFile - Pointer to the file to print to
 * @param stLength - The length of the memory
 */
void cmd_show_memory_size (FILE *pFile, size_t stLength);

/**
 * @brief Command to show memory in the format <br>
          XX XX XX XX XX XX XX XX : AAAAAAAAAAAAAAAAAA <br>
          where XX is a H rep & AA is ASCII rep of byte <br>
 * @param pbyView - pointer to the memory to display
 * @param iLength - The number of bytes to display
 * @param stOffset - The file offset
 * @param pOut - Pointer to the file stream to print to
 * @return address of last printed byte
 */
uint8_t * cmd_show_bin(uint8_t *pbyView, int_t iLength, size_t stOffset, FILE *pOut);

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_H_ */
/**************************************************************************//**
 * @} (end addtogroup)
 *****************************************************************************/
/******************************************************************************
 End  Of File
 *****************************************************************************/
