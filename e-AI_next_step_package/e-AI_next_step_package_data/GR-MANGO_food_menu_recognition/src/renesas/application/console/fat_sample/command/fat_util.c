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
* File Name    : fat_util.c
* Version      : 1.20
* Device(s)    : RZ/A2M
* Tool-Chain   : e2 studio (GCC ARM Embedded)
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M FAT/exFAT File System Sample Program
* Operation    :
* Limitations  : None
******************************************************************************/
/*****************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 16.03.2018 1.00     First Release
*         : 29.05.2019 1.20     Correspond to internal coding rules
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
* Function Name: char_to_tchar
* Description  : Convert char string to TCHAR string
* Arguments    : char_t *strIn  : input string
*              : TCHAR  *strOut : output string
* Return Value : none
******************************************************************************/
void char_to_tchar(char_t *strIn, TCHAR *strOut)
{
    uint32_t cntin;
    uint32_t cntout;

    cntin  = 0;
    cntout = 0;

    while (0 != strIn[cntin])
    {
        /* strIn is ASCII only. */
        strOut[cntout] = strIn[cntin];
        cntin++;
        cntout++;
    }
    strOut[cntout] = 0;
}
/*******************************************************************************
 End of function char_to_tchar
 ******************************************************************************/

/******************************************************************************
* Function Name: tchar_to_char
* Description  : Convert TCHAR string to char string
* Arguments    : TCHAR *strIn   : input string
*              : char_t *strOut : output string
* Return Value : none
******************************************************************************/
void tchar_to_char(TCHAR *strIn, char_t *strOut)
{
    uint32_t cntin;
    uint32_t cntout;

    cntin  = 0;
    cntout = 0;

    while (0 != strIn[cntin])
    {
        /* strIn is ASCII only. */
        strOut[cntout] = strIn[cntin];
        cntin++;
        cntout++;
    }
    strOut[cntout] = 0;
}
/*******************************************************************************
 End of function tchar_to_char
 ******************************************************************************/

/******************************************************************************
* Function Name: get_side
* Description  : Get the drive number from the path name.
* Arguments    : uint8_t *name     : path name
* Return Value : drive number      : Success
*              : -1                : Error
******************************************************************************/
int32_t get_side(uint8_t * name)
{
    int32_t side;

    if (':' == name[1])
    {
        if ((name[0] >= '0') && (name[0] <= '9'))
        {
            side = name[0] - '0';
        }
        else
        {
            return -1;
        }
    }
    else
    {
        side = 0;   /* Default drive is 0. */
    }

    return side;
}
/*******************************************************************************
 End of function get_side
 ******************************************************************************/

/******************************************************************************
* Function Name: make_file_size_str
* Description  : Create file size string.
*              : Example: 1234567 -> 1,234,567
* Arguments    : uint8_t *new_str : output
*              : uint8_t *str     : input
* Return Value : none
******************************************************************************/
void make_file_size_str(uint8_t * new_str, uint8_t * str)
{
    int32_t new_cnt;
    int32_t cnt;

    new_cnt = 0;
    cnt = 0;

    while (0 != str[cnt])
    {
        new_str[new_cnt] = str[cnt];

        if ((((MAX_FILE_SIZE_STRING_LENGTH % 3) - 1) == (cnt % 3)) && ((cnt + 3) < MAX_FILE_SIZE_STRING_LENGTH))
        {
            new_cnt++;
            if (' ' == str[cnt])
            {
                new_str[new_cnt] = ' ';
            }
            else
            {
                new_str[new_cnt] = ',';
            }
        }
        new_cnt++;
        cnt++;
    }
    new_str[new_cnt] = 0;
}
/*******************************************************************************
 End of function make_file_size_str
 ******************************************************************************/

/******************************************************************************
* Function Name: make_file_create_date_str
* Description  : Create file create date string
* Arguments    : uint8_t *new_str : output
*              : uint16_t date    : input date
* Return Value : none
******************************************************************************/
void make_file_create_date_str(uint8_t * new_str, uint16_t date)
{
    /* Cast to an appropriate type */
    sprintf((char *)new_str, "%4d/%02d/%02d", 1980 + (date >> (4 + 5)), (date >> 5) & 0x000F, date & 0x001F);
}
/*******************************************************************************
 End of function make_file_create_date_str
 ******************************************************************************/

/******************************************************************************
* Function Name: make_file_create_time_str
* Description  : Create file create time string
* Arguments    : uint8_t *new_str : output
*              : uint16_t time    : input time
* Return Value : none
******************************************************************************/
void make_file_create_time_str(uint8_t * new_str, uint16_t time)
{
    /* Cast to an appropriate type */
    sprintf((char *)new_str, "%02d:%02d", time >> (6 + 5), (time >> 5) & 0x003F);
}
/*******************************************************************************
 End of function make_file_create_time_str
 ******************************************************************************/
