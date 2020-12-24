/******************************************************************************
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
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*******************************************************************************
* Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************
* File Name    : command.c
* Version      : 1.10
* Device(s)    : Renesas
* Tool-Chain   : GNUARM-NONE-EABI v14.02
* OS           : N/A
* H/W Platform : RSK+
* Description  : The entry point of the main command handler and associated
*                commands
*******************************************************************************
* History      : DD.MM.YYYY Version Description
*              : 04.02.2010 1.00    First Release
*              : 10.06.2010 1.01    Updated type definitions.
*              : 09.06.2015 1.10    Updated high-ASCII password for GNU TC
*                                   Added FAT Stub check and warning
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "compiler_settings.h"
#include "driver.h"
#include "version.h"
#include "command.h"

#include "r_os_abstraction_api.h"
#include "r_devlink_wrapper.h"
#include "application_cfg.h"
#include "r_sd_cfg.h"
#include "application_common.h"

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/

/* Define the number of characters printed on a line for the "mem" command */
#define CMD_PRV_MEM_COMMAND_CHAR_COUNT      (16)
#define CMD_LOGIN_AND_PASSWORD_STRING_PRV_   (32)

#define R_REGION_LARGE_CAPACITY_RAM  (78957)

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
extern const st_command_table_t g_fatfs_cmd_tbl_command;
#else /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */
extern const st_command_table_t g_sdhi_cmd_tbl_command;
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/******************************************************************************
 Exported global variables and functions (to be accessed by other files)
 ******************************************************************************/


/******************************************************************************
 Private global variables and functions
 ******************************************************************************/

static int16_t cmd_version(int iArgCount, char **ppszArgument, pst_comset_t pCom);
_Bool cmdCheckUserNameAndPassword(char *pszUserName, char *pszPassword);
int cmdUserName (int iArgCount, char **ppszArgument, pst_comset_t pCom);
int cmdChangePassword (int iArgCount, char **ppszArgument, pst_comset_t pCom);

/* Define the engineering exp symbols for Kilo, Mega, Giga, Tera, Peta, Exa, Zetta and Yotta */
static char * const gsp_eng_mult_chars = " kMGTPEZY";

/*****************************************************************************
Global Variables
******************************************************************************/
char gpszLoginsAndPasswords[CMD_LOGIN_AND_PASSWORD_STRING_PRV_];
_Bool  gbfLoginsLoaded = false;

/******************************************************************************
Public Functions
******************************************************************************/

/* Table that associates command letters, function pointer and a little
   description of what the command does */
static st_cmdfnass_t gs_cmd_command[] =
{
     {
        "ver",
        cmd_version,
        "<CR> - Show the application version",
     }
};

/* Table that points to the above table and contains the number of entries */
const st_command_table_t gs_cmd_tbl_command =
{
    "General Commands",
    (pst_cmdfnass_t) gs_cmd_command,
    ((sizeof(gs_cmd_command)) / sizeof(st_cmdfnass_t))
};

/* Define the command tables required */
pst_command_table_t gppCommands[32] =
{
    /* cast to pst_command_table_t */
/*    (pst_command_table_t) &gs_cmd_tbl_command, */

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
    /* Added space for additional 'sample code' */
    (pst_command_table_t) &g_fatfs_cmd_tbl_command,
#else /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */
    (pst_command_table_t) &g_sdhi_cmd_tbl_command
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */
};

int32_t g_num_commands = (sizeof(gppCommands)) / sizeof(pst_command_table_t);

/* The default user name is admin and the password is password */
/* Password is xor 129 protected */
static const char gpszDefaultUser[] = "admin:\xF1\xE0\xF2\xF2\xF6\xEE\xF3\xE5";
/******************************************************************************
 Function Name: cmd_console
 Description:   Function to process the console commands
 Arguments:     IN  pIn - Pointer to the input file stream
                IN  pOut - Pointer to the file stream for echo of command input
                IN  pszPrompt - Pointer to a command prompt
 Return value:  none
 ******************************************************************************/
void cmd_console(FILE* pIn, FILE *pOut, char *pszPrompt)
{
    pst_comset_t com = R_OS_Malloc(sizeof(st_comset_t), R_MEMORY_REGION_DEFAULT);
    int32_t pos;

    g_num_commands = (sizeof(gppCommands)) / sizeof(pst_command_table_t);

    /* remove unused console command lists */
    for (pos = (g_num_commands - 1); pos > 0; pos--)
    {
        /* check for null entry */
        if (NULL == gppCommands[pos])
        {
            g_num_commands--;
        }
    }

    if (com)
    {
        while (1)
        {
            e_cmderr_t cmd_err = CMD_OK;

            /* Initialise the console for login */
            memset(com, 0, sizeof(st_comset_t));
            com->p_in = pIn;
            com->p_out = pOut;
#if 0
            /* Print a welcome message */
            show_welcome_msg(com->p_out, true);

            fflush(pOut);
#endif
            /* Process commands until the link is lost or the command returns a code greater than CMD_ERROR_IN_IO */
            while (cmd_err < CMD_USER_EXIT)
            {
                /* cast gppCommands to cpst_command_table_t * */
                cmd_err = console(com, (cpst_command_table_t *) gppCommands, g_num_commands, pIn, pOut, pszPrompt);
            }

            /* CMD_ERROR_IN_IO is returned when the client disconnects check for an IP stream */
            if ((CMD_ERROR_IN_IO == cmd_err) && (control(R_DEVLINK_FilePtrDescriptor(com->p_in), CTL_STREAM_TCP, NULL) == 0))
            {
                /* Log out */
                cmd_err = CMD_LOG_OUT;
            }

            /* Check for the "exit<CR>" command return code */
            if (CMD_LOG_OUT == cmd_err)
            {
                fprintf(com->p_out, "\r\nBye!\r\n");
                fflush(pOut);

                /* Do "exit" code */
                R_OS_Free((void*) &com);
                return;
            }
        }
    }
}
/******************************************************************************
 End of function cmd_console
 ******************************************************************************/

/******************************************************************************
Function Name: cmd_version
Description:   Command to show the version
Arguments:     IN  iArgCount - The number of arguments in the argument list
               IN  ppszArgument - The argument list
               IN  pCom - Pointer to the command object
Return value:  CMD_OK for success
******************************************************************************/
static int16_t cmd_version(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    show_welcome_msg(pCom->p_out, false);
    return CMD_OK;
}
/******************************************************************************
End of function cmd_version
******************************************************************************/


/******************************************************************************
 End  Of File
 ******************************************************************************/
