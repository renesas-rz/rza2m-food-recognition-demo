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
* File Name    : fat_sample_main.c
* Version      : 1.20
* Device(s)    : RZ/A2M
* Tool-Chain   : e2 studio (GCC ARM Embedded)
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M FAT/exFAT File System Sample Program - Main
* Operation    :
* Limitations  : None
******************************************************************************/
/*****************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 16.03.2018 1.00     First Release
*         : 28.12.2018 1.02     Support for OS
*         : 29.05.2019 1.20     Correspond to internal coding rules
*         : 17.09.2019 1.30     Support for SDIO
*         : 27.02.2020 1.50     Change version number
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include "r_typedefs.h"
#include "fat_sample.h"

/******************************************************************************
Typedef definitions
******************************************************************************/


/******************************************************************************
Macro definitions
******************************************************************************/


/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/


/******************************************************************************
Private global variables and functions
******************************************************************************/

/******************************************************************************
 * Function Name: fat_sample_welcome_msg
 * Description  : Function to display initial welcome message
 * Arguments    : IN  pCom - Pointer to the command object data
 *              : IN  clear_screen - clear screen before message  true/false
 * Return Value : None
 *****************************************************************************/
void fat_sample_welcome_msg(FILE *p_out, bool_t clear_screen)
{
    /* Cast to an appropriate type */
    UNUSED_PARAM(clear_screen);

    fprintf(p_out, "\r\nRZ/A2M CPU Board FAT/exFAT Sample Program. Ver.1.50\r\n");
    fprintf(p_out, "Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.\r\n");
    fprintf(p_out, "\r\n");

    fprintf(p_out, "NOTE! Only \"ASCII\" character codes are supported.\r\n");
    fprintf(p_out, "FAT command mode.\r\n");
    fprintf(p_out, "\r\n");
}
/*******************************************************************************
 End of function fat_sample_welcome_msg
 ******************************************************************************/

/* End of File */

