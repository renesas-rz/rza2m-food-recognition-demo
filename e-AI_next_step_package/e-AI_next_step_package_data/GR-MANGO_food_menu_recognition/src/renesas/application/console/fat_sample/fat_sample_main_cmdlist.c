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
* File Name    : fat_sample_main_cmdlist.c
* Version      : 1.20
* Device(s)    : RZ/A2M
* Tool-Chain   : e2 studio (GCC ARM Embedded)
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M FAT/exFAT File System Sample Program - Command list
* Operation    :
* Limitations  : None
******************************************************************************/
/*****************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 16.03.2018 1.00     First Release
*         : 28.12.2018 1.02     Support for OS
*         : 29.05.2019 1.20     Correspond to internal coding rules
******************************************************************************/


/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include "r_typedefs.h"
#include "command.h"
#include "fat_sample.h"

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
static st_cmdfnass_t s_fat_sample_main_cmd_list[] =
{
    /* Cast to an appropriate type */
    {"HELP",    FAT_Sample_Help,        NULL},      /* Display command help */

    /* Cast to an appropriate type */
    /*    {"ATT",     FAT_Sample_Att,         NULL}, *//* no used */

    /* Cast to an appropriate type */
    /*    {"DET",     FAT_Sample_Det,         NULL}, *//* no used */

    /* Cast to an appropriate type */
    {"DIR",     FAT_Sample_Dir,         NULL},

    /* Cast to an appropriate type */
    {"TYPE",    FAT_Sample_Type,        NULL},

    /* Cast to an appropriate type */
    {"WRITE",   FAT_Sample_Write,       NULL},

    /* Cast to an appropriate type */
    {"READ",    FAT_Sample_Read,        NULL},

    /* Cast to an appropriate type */
    {"CREATE",  FAT_Sample_Create,      NULL},

    /* Cast to an appropriate type */
    {"DEL",     FAT_Sample_Del,         NULL},

    /* Cast to an appropriate type */
    {"MKDIR",   FAT_Sample_Mkdir,       NULL},

    /* Cast to an appropriate type */
    {"RMDIR",   FAT_Sample_Rmdir,       NULL},

    /* Cast to an appropriate type */
    /*    {"EXIT",    FAT_Sample_Exit,        NULL}, *//* no used */
};

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
/* Table that associates command letters, function pointer and a little
   description of what the command does */

/* Table that points to the above table and contains the number of entries */
const st_command_table_t g_fatfs_cmd_tbl_command =
{
    "sdhi Commands",

    /* Cast to an appropriate type */
    (pst_cmdfnass_t) s_fat_sample_main_cmd_list,
    ((sizeof(s_fat_sample_main_cmd_list)) / sizeof(st_cmdfnass_t))
};

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Help
* Description  : Outputs command descriptions to the terminal.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Help(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    /* Cast to an appropriate type */
    UNUSED_PARAM(ppszArgument);

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    /* ==== Command help ==== */
    if (1 == iArgCount)
    {
        /* Processing may be added here by using the terminal output   */
        /* to display descriptions such as what commands are embedded. */
        printf("\r\n");

        /*
        printf
            ("  ATT drive_number                  : Connects a drive.                                           \r\n");
        printf
            ("  DET drive_number                  : Disconnects a drive.                                        \r\n");
        *//* no used */

        printf
            ("  DIR path_name(full path)          : Displays a list of files and subdirectories in a directory. \r\n");
        printf
            ("  TYPE file_name(full path)         : Displays the contents of a file.                            \r\n");
        printf
            ("  WRITE file_name(full path)        : Write a sample string.                                      \r\n");
        printf
            ("  CREATE new_file_name(full path)   : Create a new file.                                          \r\n");
        printf
            ("  DEL delete_file_name(full path)   : Delete a file.                                              \r\n");
        printf
            ("  MKDIR new_dir_name(full path)     : Create a new directory.                                     \r\n");
        printf
            ("  RMDIR delete_dir_name(full path)  : Delete a directory.                                         \r\n");

        /*
        printf
            ("  EXIT                              : Exit from FAT Command mode.                                 \r\n");
        *//* no used */

        printf("\r\n");
    }
    /* ==== Help by command ==== */
    else if (2 == iArgCount)
    {
        /* Processing may be added here by using the terminal output */
        /* to display descriptions for the specified command format. */

        /* DO NOTHING */
        ;
    }
    else
    {
        printf("error: Command failure.\r\n");
        printf("\r\n");
        return CMD_OK;
    }

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Help
 ******************************************************************************/

/* End of File */

