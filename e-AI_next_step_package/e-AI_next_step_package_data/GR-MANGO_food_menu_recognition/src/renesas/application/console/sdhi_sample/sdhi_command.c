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
* File Name    : sdhi_command.c
* Device(s)    : RZ/A2M
* Tool-Chain   : e2 studio (GCC ARM Embedded)
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M SDHI Sample Program - Command list and process
* Operation    :
* Limitations  : None
******************************************************************************/
/*****************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 15.04.2019 1.03     Support for SDIO
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "r_typedefs.h"
#include "command.h"
#include "r_os_abstraction_api.h"
#include "r_sdif.h"
#include "r_sd_cfg.h"
#include "sd.h"
#include "sdhi_command.h"

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Typedef definitions
******************************************************************************/
/* ==== SD card type typedefine  ==== */
typedef struct __sdtype
{
    int32_t        code;
    uint8_t        type[28];
} st_sdtype_t;

/* ==== SD WP type typedefine  ==== */
typedef struct __sdwp
{
    int32_t        code;
    uint8_t        wp[28];
} st_sdwp_t;

typedef struct
{
    char_t         *p_msg;
    int32_t        errorno;
} st_sderr_t;

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/


/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
#ifdef __CC_ARM
#pragma arm section zidata = "UNCACHED_BSS"
static uint32_t
s_sd_work[2][ SD_SIZE_OF_INIT / sizeof(uint32_t) ];
static uint32_t
s_sd_rw_buff[2][ SD_RW_BUFF_SIZE / sizeof(uint32_t) ];
static uint32_t
s_sd_rw_buff2[2][ (SD_TEST_SECTOR_NUM * 512) / sizeof(uint32_t) ];
uint8_t g_sdio_cis[SDIO_CIS_BUFF_SIZE];
#pragma arm section zidata
#elif defined(__GNUC__)
static uint32_t
s_sd_work[2][ SD_SIZE_OF_INIT / sizeof(uint32_t) ] __attribute__ ((section ("UNCACHED_BSS"), aligned(8)));
static uint32_t
s_sd_rw_buff[2][ SD_RW_BUFF_SIZE / sizeof(uint32_t) ] __attribute__ ((section ("UNCACHED_BSS"), aligned(8)));
static uint32_t s_sd_rw_buff2[2][ (SD_TEST_SECTOR_NUM * 512) / sizeof(uint32_t) ]
__attribute__ ((section ("UNCACHED_BSS"), aligned(8)));
uint8_t g_sdio_cis[SDIO_CIS_BUFF_SIZE] __attribute__ ((section ("UNCACHED_BSS"), aligned(8)));
#elif defined(__ICCARM__)
static uint32_t
s_sd_work[2][ SD_SIZE_OF_INIT / sizeof(uint32_t) ] @ "UNCACHED_BSS";
static uint32_t
s_sd_rw_buff[2][ SD_RW_BUFF_SIZE / sizeof(uint32_t) ] @ "UNCACHED_BSS";
static uint32_t
s_sd_rw_buff2[2][ (SD_TEST_SECTOR_NUM * 512) / sizeof(uint32_t) ] @ "UNCACHED_BSS";
uint8_t g_sdio_cis[SDIO_CIS_BUFF_SIZE] @ "UNCACHED_BSS";
#else
error!!

#endif

/******************************************************************************
Private global variables and functions
******************************************************************************/
static int_t s_sdio_interrupt_flag[2];

/* ==== command function prototype ==== */
static int16_t cmd_sd_init(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_int(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_intmask(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_enable(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_reset(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_gint(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_grdy(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_final(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_ioatt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_iodet(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_format(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_gettype(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_getwp(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_getreg(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_get_cia(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_get_ocr(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_get_info(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_get_rca(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_get_sdstatus(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_get_speed(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_setsec(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_setblkcnt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_getsec(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_getblkcnt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_getipv(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_getsize(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_lock_unlock(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_read(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sd_write(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_read_direct(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_write_direct(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_read(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_write(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdhi_help(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_setblk(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);
static int16_t cmd_sdio_getblk(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom);

static void sd_print_put_dmp(uint8_t *Ptr, uint16_t Size, uint16_t Spc, uint16_t Wid);
static void sd_print_put_dmp2(uint8_t *Ptr, uint32_t Size, uint16_t Spc, uint16_t Wid);
static void sd_make_buffer_data(uint8_t *Ptr, uint32_t Size, uint16_t Wid, uint32_t start_value);

static int16_t sdhi_show_error_msg(int32_t errorno);
static int16_t sdio_clear_sdio_interrupt(int32_t sd_port);

static e_cmderr_t con_parse_command_sdhi(pst_comset_t pCom);
static e_cmderr_t terminate_command_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand);
static e_cmderr_t process_ordinary_char_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand);
static e_cmderr_t con_process_char_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand);
static int16_t    sd_read_char(pst_comset_t pCom);
static int16_t    sd_check_port(pst_comset_t pCom, int32_t *p_port);
static int16_t    sd_get_num(pst_comset_t pCom, int32_t *p_num);
static int16_t    sd_get_char(pst_comset_t pCom, char_t *p_char, uint32_t max_len);

/* ==== command table  ==== */
static const st_cmdfnass_t s_sdhi_main_cmd[] =
{
    /* Cast to an appropriate type */
    {"INIT",        cmd_sd_init,            NULL},

    /* Cast to an appropriate type */
    {"SDIOINT",     cmd_sdio_int,           NULL},

    /* Cast to an appropriate type */
    {"SDIOMASK",    cmd_sdio_intmask,       NULL},

    /* Cast to an appropriate type */
    {"SDIOENB",     cmd_sdio_enable,        NULL},

    /* Cast to an appropriate type */
    {"SDIORESET",   cmd_sdio_reset,         NULL},

    /* Cast to an appropriate type */
    {"SDIOGINT",    cmd_sdio_gint,          NULL},

    /* Cast to an appropriate type */
    {"SDIOGRDY",    cmd_sdio_grdy,          NULL},

    /* Cast to an appropriate type */
    {"SDIOCIA",     cmd_sdio_get_cia,       NULL},

    /* Cast to an appropriate type */
    {"SDIOOCR",     cmd_sdio_get_ocr,       NULL},

    /* Cast to an appropriate type */
    {"SDIOINFO",    cmd_sdio_get_info,      NULL},

    /* Cast to an appropriate type */
    {"SDIOSETBLK",  cmd_sdio_setblk,        NULL},

    /* Cast to an appropriate type */
    {"SDIOGETBLK",  cmd_sdio_getblk,        NULL},

    /* Cast to an appropriate type */
    {"FINAL",       cmd_sd_final,           NULL},

    /* Cast to an appropriate type */
    {"IOATT",       cmd_sd_ioatt,           NULL},

    /* Cast to an appropriate type */
    {"IODET",       cmd_sd_iodet,           NULL},

    /* Cast to an appropriate type */
    {"FORMAT",      cmd_sd_format,          NULL},

    /* Cast to an appropriate type */
    /*    {"TYPE",        cmd_sd_gettype,         NULL}, */

    /* Cast to an appropriate type */
    {"SDTYPE",      cmd_sd_gettype,         NULL},

    /* Cast to an appropriate type */
    {"WP",          cmd_sd_getwp,           NULL},

    /* Cast to an appropriate type */
    {"REG",         cmd_sd_getreg,          NULL},

    /* Cast to an appropriate type */
    {"RCA",         cmd_sd_get_rca,         NULL},

    /* Cast to an appropriate type */
    {"SDSTATUS",    cmd_sd_get_sdstatus,    NULL},

    /* Cast to an appropriate type */
    {"SPEED",       cmd_sd_get_speed,       NULL},

    /* Cast to an appropriate type */
    {"SSEC",        cmd_sd_setsec,          NULL},

    /* Cast to an appropriate type */
    {"GSEC",        cmd_sd_getsec,          NULL},

    /* Cast to an appropriate type */
    {"SBLK",        cmd_sdio_setblkcnt,     NULL},

    /* Cast to an appropriate type */
    {"GBLK",        cmd_sdio_getblkcnt,     NULL},

    /* Cast to an appropriate type */
    {"SIZE",        cmd_sd_getsize,         NULL},

    /* Cast to an appropriate type */
    {"IPVER",       cmd_sd_getipv,          NULL},

    /* Cast to an appropriate type */
    {"LOCK",        cmd_sd_lock_unlock,     NULL},

    /* Cast to an appropriate type */
    {"UNLOCK",      cmd_sd_lock_unlock,     NULL},

    /* Cast to an appropriate type */
    {"READ",        cmd_sd_read,            NULL},

    /* Cast to an appropriate type */
    {"WRITE",       cmd_sd_write,           NULL},

    /* Cast to an appropriate type */
    {"IOREAD_D",    cmd_sdio_read_direct,   NULL},

    /* Cast to an appropriate type */
    {"IOWRITE_D",   cmd_sdio_write_direct,  NULL},

    /* Cast to an appropriate type */
    {"IOREAD",      cmd_sdio_read,          NULL},

    /* Cast to an appropriate type */
    {"IOWRITE",     cmd_sdio_write,         NULL},

    /* Cast to an appropriate type */
    {"SDHELP",      cmd_sdhi_help,          NULL},

    /* Cast to an appropriate type */
    {NULL,          NULL,                   NULL}
};

/* ==== SD card type  ==== */
static const st_sdtype_t s_sdtype[5] =
{
    {SD_MEDIA_MMC,      "MMC Card"},
    {SD_MEDIA_SD,       "SD Memory Card"},
    {SD_MEDIA_IO,       "SDIO Card"},
    {SD_MEDIA_COMBO,    "Combo Card"},
    {SD_MEDIA_UNKNOWN,  "unknown media"},
};

/* ==== SD WP type  ==== */
static const st_sdwp_t s_sdwp[5] =
{
    {SD_WP_OFF,         "Not WP"},
    {SD_WP_HW,          "H/W WP"},
    {SD_WP_TEMP,        "Temp WP"},
    {SD_WP_PERM,        "Perm WP"},
    {SD_WP_ROM,         "ROM Card"},
};

static const st_sderr_t s_sd_error_msg[] =
{
    {"SD_OK",                       0},        /* OK */
    {"SD_ERR",                     -1},        /* general error */
    {"SD_ERR_WP",                  -2},        /* write protect error */
    {"SD_ERR_RO",                  -3},        /* read only error */
    {"SD_ERR_RES_TOE",             -4},        /* response time out error */
    {"SD_ERR_CARD_TOE",            -5},        /* card time out error */
    {"SD_ERR_END_BIT",             -6},        /* end bit error */
    {"SD_ERR_CRC",                 -7},        /* CRC error */
    {"SD_ERR_ILL_ACCESS",          -8},        /* illegal access error */
    {"SD_ERR_HOST_TOE",            -9},        /* host time out error */
    {"SD_ERR_CARD_ERASE",         -10},        /* card erase error */
    {"SD_ERR_CARD_LOCK",          -11},        /* card lock error */
    {"SD_ERR_CARD_UNLOCK",        -12},        /* card unlock error */
    {"SD_ERR_HOST_CRC",           -13},        /* host CRC error */
    {"SD_ERR_CARD_ECC",           -14},        /* card internal ECC error */
    {"SD_ERR_CARD_CC",            -15},        /* card internal error */
    {"SD_ERR_CARD_ERROR",         -16},        /* unknown card error */
    {"SD_ERR_CARD_TYPE",          -17},        /* non support card type */
    {"SD_ERR_NO_CARD",            -18},        /* no card */
    {"SD_ERR_ILL_READ",           -19},        /* illegal buffer read */
    {"SD_ERR_ILL_WRITE",          -20},        /* illegal buffer write */
    {"SD_ERR_AKE_SEQ",            -21},        /* the sequence of authentication process */
    {"SD_ERR_OVERWRITE",          -22},        /* CID/CSD overwrite error */

    /* 23-29 */
    {"SD_ERR_CPU_IF",             -30},        /* target CPU interface function error  */
    {"SD_ERR_STOP",               -31},        /* user stop */

    /* 32-49 */
    {"SD_ERR_CSD_VER",            -50},        /* CSD register version error */
    {"SD_ERR_SCR_VER",            -51},        /* SCR register version error */
    {"SD_ERR_FILE_FORMAT",        -52},        /* CSD register file format error  */
    {"SD_ERR_NOTSUP_CMD",         -53},        /* not supported command  */

    /* 54-59 */
    {"SD_ERR_ILL_FUNC",           -60},        /* invalid function request error */
    {"SD_ERR_IO_VERIFY",          -61},        /* direct write verify error */
    {"SD_ERR_IO_CAPAB",           -62},        /* IO capability error */

    /* 63-69 */
    {"SD_ERR_IFCOND_VER",         -70},        /* Interface condition version error */
    {"SD_ERR_IFCOND_VOLT",        -71},        /* Interface condition voltage error */
    {"SD_ERR_IFCOND_ECHO",        -72},        /* Interface condition echo back pattern error */

    /* 73-79 */
    {"SD_ERR_OUT_OF_RANGE",       -80},        /* the argument was out of range */
    {"SD_ERR_ADDRESS_ERROR",      -81},
    {"SD_ERR_BLOCK_LEN_ERROR",    -82},
    {"SD_ERR_ILLEGAL_COMMAND",    -83},
    {"SD_ERR_RESERVED_ERROR18",   -84},
    {"SD_ERR_RESERVED_ERROR17",   -85},
    {"SD_ERR_CMD_ERROR",          -86},
    {"SD_ERR_CBSY_ERROR",         -87},
    {"SD_ERR_NO_RESP_ERROR",      -88},
    {"SD_ERR_NOT_SWITCH_VOLTAGE", -89},

    /* 90-95 */
    {"SD_ERR_ERROR",              -96},
    {"SD_ERR_FUNCTION_NUMBER",    -97},
    {"SD_ERR_COM_CRC_ERROR",      -98},
    {"SD_ERR_INTERNAL",           -99},        /* driver software internal error */
    {0,                             0}
};

/* Table that associates command letters, function pointer and a little
   description of what the command does */

/* Table that points to the above table and contains the number of entries */
const st_command_table_t g_sdhi_cmd_tbl_command =
{
    "sdhi Commands",

    /* Cast to an appropriate type */
    (pst_cmdfnass_t) s_sdhi_main_cmd,
    ((sizeof(s_sdhi_main_cmd)) / sizeof(st_cmdfnass_t))
};

/******************************************************************************
 * Function Name: con_parse_command_sdhi
 * Description  : Function to pars the command and call the handling function
 * Arguments    : IN  pCom - pointer to command table
 * Return Value : 0 for success otherwise error code
 ******************************************************************************/
static e_cmderr_t con_parse_command_sdhi(pst_comset_t pCom)
{
    uint32_t eat_space_count = 0U;
    uint32_t length;

    /*    _Bool valid_command = false; */
    /*    int16_t count = 0; */
    e_cmderr_t error_code = CMD_OK;

    /* Eat any white space before the command */
    while ((' ' == pCom->va.buffer[eat_space_count]) || ('\t' == pCom->va.buffer[eat_space_count])
            || ('@' == pCom->va.buffer[eat_space_count]))
    {
        eat_space_count++;
    }

    length = pCom->va.buffer_index - eat_space_count;
    pCom->va.buffer_index = 0;

    /* Test for valid command or just CR */
    if (length >= 1)
    {
        /* If it is not an escape sequence */
        if (ESC_NO_ESCAPE == pCom->va.escape_sequence)
        {
            /* Copy to last command buffer */
            strcpy(pCom->va.command, pCom->va.buffer);
        }
        else
        {
            /* Cancel escape sequence */
            pCom->va.escape_sequence = ESC_NO_ESCAPE;
        }

        /* Execute the command function *//* no used */
        /*        while (count < pCom->num_tables) */
        /*        { */
        /*            error_code = con_execute(pCom, pCom->p_function[count], &valid_command); */
        /* */
        /*            if (!valid_command) */
        /*            { */
        /*                count++; */
        /*            } */
        /*            else */
        /*            { */
        /*                break; */
        /*            } */
        /*        } */
        /* */
        /* Test to see if command is valid */
        /*        if (!valid_command) */
        /*        { */
        /*            fprintf(pCom->p_out, "\r\n\"%s\" Unknown command\r\n", pCom->va.buffer); */
        /*        } */
    }

    return error_code;
}
/******************************************************************************
End of function con_parse_command_sdhi
******************************************************************************/

/******************************************************************************
 * Function Name: terminate_command_sdhi
 * Description  : Function to process an ordinary character
 * Arguments    : IN  pCom - pointer to the command object
 *              : IN  chChar - The character to process
 *              : IN  pbfCommand - Pointer to a flag that is set when a
 *              :                  command is received
 * Return value : 0 for success otherwise error code
 ******************************************************************************/
static e_cmderr_t terminate_command_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand)
{
    /* 2nd parameter is not used. */
    (void)chChar;

    if ((ESC_NO_ESCAPE == pCom->va.escape_sequence) && ('@' != pCom->va.buffer[0]))
    {
#ifdef SERIAL
        fprintf(pCom->p_out, "\r\n");
#endif
    }

    /* Terminate the string */
    pCom->va.buffer[pCom->va.buffer_index] = (uint8_t) '\0';
    *pbfCommand = true;

    /* Parse the command line */
    return con_parse_command_sdhi(pCom);
}
/******************************************************************************
End of function terminate_command_sdhi
******************************************************************************/

/******************************************************************************
 * Function Name: process_ordinary_char_sdhi
 * Description  : Function to process an ordinary character
 * Arguments    : IN  pCom - pointer to the command object
 *              : IN  chChar - The character to process
 *              : IN  pbfCommand - Pointer to a flag that is set when a
 *              :                  command is received
 * Return value : 0 for success otherwise error code
******************************************************************************/
static e_cmderr_t process_ordinary_char_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand)
{
    if (pCom->va.buffer_index < (CMD_READER_LINE_SIZE - 1))
    {
        /* buffer */
        pCom->va.buffer[pCom->va.buffer_index++] = chChar;

        /* Two character escape sequence termination tests - F Keys */
        if ((ESC_ESCAPE_SEQUENCE == pCom->va.escape_sequence)
            && (2 == pCom->va.buffer_index) && ('O' == (*pCom->va.buffer)))
        {
            pCom->va.buffer[pCom->va.buffer_index] = '\0';
            *pbfCommand = true;

            /* Parse the command line */
            return con_parse_command_sdhi(pCom);
        }
        /* Arrow keys */
        else if ((ESC_ESCAPE_SEQUENCE == pCom->va.escape_sequence)
            && (2 == pCom->va.buffer_index) && ('[' == (*pCom->va.buffer)) && ('1' != chChar))
        {
            pCom->va.buffer[pCom->va.buffer_index] = '\0';
            *pbfCommand = 1;
            return con_parse_command_sdhi(pCom);
        }
        else
        {
            /* do nothing */
            __asm ("nop");
        }
    }
    else
    {
        return CMD_LINE_TOO_LONG;
    }

#ifdef SERIAL
    /* echo to console */
    if ((ESC_NO_ESCAPE == pCom->va.escape_sequence) && ('@' != pCom->va.buffer[0]))
    {
        if (pCom->secrecy)
        {
            fputc('*', pCom->p_out);
        }
        else
        {
            fputc(chChar, pCom->p_out);
        }
    }
#endif

    return CMD_OK;
}
/******************************************************************************
End of function process_ordinary_char_sdhi
******************************************************************************/

/******************************************************************************
 * Function Name: con_process_char_sdhi
 * Description  : Function to process a character
 * Arguments    : IN  pCom - pointer to the command object
 *              : IN  chChar - The character to process
 *              : IN  pbfCommand - Pointer to a flag that is set when a
 *              :                  command is received
 * Return value : 0 for success otherwise error code
 ******************************************************************************/
static e_cmderr_t con_process_char_sdhi(pst_comset_t pCom, char chChar, _Bool *pbfCommand)
{
    *pbfCommand = false;

    switch (chChar)
    {
        /* Ignore escape sequence start */
        case CMD_ESCAPE_CHARACTER:
        {
            /* suppress echo */
            pCom->va.escape_sequence = ESC_ESCAPE_SEQUENCE;
            pCom->va.buffer_index = 0;
        }
        break;

        case '\b':                            /* White out on back space */
        case  0x7F:                            /* PuTTY sends non-ASCII back space */
        {
            if (0 != pCom->va.buffer_index)
            {
                #ifdef SERIAL
                fprintf(pCom->p_out, "\b \b");
                #endif
                pCom->va.buffer_index--;
            }
        }
        break;

        #ifdef SERIAL
        case '\n':                            /* New line - ignore */
        {
            __asm ("nop");
        }
        break;
        #endif

        /* End of escape sequence */
        case ';':
        {
            if (ESC_NO_ESCAPE == pCom->va.escape_sequence)
            {
                return process_ordinary_char_sdhi(pCom, chChar, pbfCommand);
            }

            return terminate_command_sdhi(pCom, chChar, pbfCommand);
        }

        case '~':
        {
            if (ESC_NO_ESCAPE == pCom->va.escape_sequence)
            {
                return process_ordinary_char_sdhi(pCom, chChar, pbfCommand);
            }

            return terminate_command_sdhi(pCom, chChar, pbfCommand);
        }

        case 0:
        {
            if (ESC_NO_ESCAPE == pCom->va.escape_sequence)
            {
                break;
            }

            return terminate_command_sdhi(pCom, chChar, pbfCommand);
        }

        #ifdef SERIAL
        case '\r':                            /* return - do function */
            #else
        case '\n':
            #endif
            {
                return terminate_command_sdhi(pCom, chChar, pbfCommand);
            }

        /* All other characters */
        default:
        {
            return process_ordinary_char_sdhi(pCom, chChar, pbfCommand);
        }
    }

    return CMD_OK;
}
/******************************************************************************
End of function con_process_char_sdhi
******************************************************************************/

/******************************************************************************
 * Function Name: sd_read_char
 * Description  : Function to get a character
 * Arguments    : IN  pCom - pointer to the command object
 * Return value : CMD_OK for success otherwise error code
 ******************************************************************************/
static int16_t sd_read_char(pst_comset_t pCom)
{
    _Bool       bf_command = false;
    e_cmderr_t  error_code = CMD_OK;
    int_t       in_data;

    while (true)
    {
        /* Read a character from the input stream */
        in_data = fgetc(pCom->p_in);
        if (EOF == in_data)
        {
            /* cast to void */
            clearerr(pCom->p_in);
            error_code = CMD_ERROR_IN_IO;
            break;
        }
        else
        {
            /* cast to int16_t */
            pCom->va.data = (int16_t)in_data;

            /* Bump the read count */
            pCom->va.read_count++;

            /* Process the character */
            fflush(pCom->p_out);

            /* cast integer to char */
            error_code = con_process_char_sdhi(pCom, (char) pCom->va.data, &bf_command);

            /* If an error occurs then return it */
            if (error_code > CMD_UNKNOWN)
            {
                break;
            }
            if (true == bf_command)
            {
                break;
            }
        }
    }
    return error_code;
}
/******************************************************************************
End of function sd_read_char
******************************************************************************/

/******************************************************************************
 * Function Name: sd_check_port
 * Description  : Function to check a channel no
 * Arguments    : IN  pCom - pointer to the command object
 *              : IN  p_port - pointer to the channel no
 * Return value : CMD_OK for success otherwise error code
 ******************************************************************************/
static int16_t sd_check_port(pst_comset_t pCom, int32_t *p_port)
{
    int16_t     cmd_err;
    int_t       chk;

    cmd_err = sd_read_char(pCom);
    if (CMD_OK == cmd_err)
    {
        chk = strcmp(pCom->va.command, "0");
        if (0 != chk)
        {
            chk = strcmp(pCom->va.command, "1");
        }
        if (0 == chk)
        {
            /* Cast to an appropriate type */
            *p_port = strtol(pCom->va.command, NULL, 0);

            /* printf("port = %d\r\n", *p_port); */
        }
        else
        {
            cmd_err = -1;
        }
    }
    return cmd_err;
}
/******************************************************************************
End of function sd_check_port
******************************************************************************/

/******************************************************************************
 Function Name: sd_get_num
 Description  : Function to get a number
 Arguments    : IN  pCom - pointer to the command object
              : IN  p_num - pointer to the number
 Return value : CMD_OK for success otherwise error code
 ******************************************************************************/
static int16_t sd_get_num(pst_comset_t pCom, int32_t *p_num)
{
    int16_t     cmd_err;

    cmd_err = sd_read_char(pCom);
    if (CMD_OK == cmd_err)
    {
        /* When input value is invalid, input value is treated as 0. */
        *p_num = strtol(pCom->va.command, NULL, 0);
    }
    return cmd_err;
}
/******************************************************************************
End of function sd_get_num
******************************************************************************/

/******************************************************************************
 * Function Name: sd_get_char
 * Description  : Function to get a character
 * Arguments    : IN  pCom - pointer to the command object
 *                IN  p_char - pointer to the character
 *                IN  max_len - max length of string to get
 * Return value : CMD_OK for success otherwise error code
 ******************************************************************************/
static int16_t sd_get_char(pst_comset_t pCom, char_t *p_char, uint32_t max_len)
{
    int16_t     cmd_err;
    uint32_t    len;

    cmd_err = sd_read_char(pCom);
    if (CMD_OK == cmd_err)
    {
        len = strlen(pCom->va.command);

        /* printf("max_len = %d\r\n", max_len); */
        /* printf("string lentgh = %d\r\n", len); */
        if (max_len > len)
        {
            /* Do not check the return value. */
            (void)strcpy(p_char, pCom->va.command);
        }
        else
        {
            cmd_err = -1;
        }
    }
    return cmd_err;
}
/******************************************************************************
End of function sd_get_char
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_init
 * Description  : "INIT" command
 *              : Initialize SD driver.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_init(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint32_t       base;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        base = SD_CFG_IP0_BASE;
        printf("set card port 0\r\n");
    }
    else
    {
        base = SD_CFG_IP1_BASE;
        printf("set card port 1\r\n");
    }

    printf("sd_init\r\n");
    err = sd_init(sd_port, base, &s_sd_work[sd_port][0], SD_CD_SOCKET);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

#ifdef SD_CFG_CD_INT
    printf("set card detect by interrupt\r\n");

    err = sd_cd_int(sd_port, SD_CD_INT_ENABLE, sd_int_callback);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
    }
#else
    printf("set card detect by polling\r\n");

    err = sd_cd_int(sd_port, SD_CD_INT_DISABLE, 0);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
    }
#endif
    err = sd_set_intcallback(sd_port, &SD_status_callback_function);
    if (SD_OK != err)
    {
        return CMD_OK;
    }
    err = sd_set_dma_intcallback(sd_port, &SD_dma_end_callback_function);
    if (SD_OK != err)
    {
        return CMD_OK;
    }

    printf("sd_set_buffer\r\n");
    sd_set_buffer(sd_port, &s_sd_rw_buff[sd_port][0], SD_RW_BUFF_SIZE);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_init
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_int
 * Description  : "SDIOINT" command
 *              : Initialize SDIO driver.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_int(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    char_t         temp[16];
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint8_t        func_bit;
    int_t          enab;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("set SDIO Interrupt for card port 0\r\n");
    }
    else
    {
        printf("set SDIO Interrupt for card port 1\r\n");
    }

    printf("SDIO Interrupt Enable?:");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK != cmd_err)
    {
        printf("SDIO Interrupt Enable is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' == temp[0]) || ('Y' == temp[0]) )
    {
        printf("set enable\r\n");
        enab = 1;
    }
    else
    {
        printf("set disable\r\n");
        enab = 0;
    }

    printf("All Fucntion?:");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK != cmd_err)
    {
        printf("All Fucntion is invalid.\r\n");
        return CMD_OK;
    }

    if ( ('y' == temp[0]) || ('Y' == temp[0]) )
    {
        func_bit = 0xfe;
        if (1 == enab)
        {
            printf("all function set enable\r\n");
        }
        else
        {
            printf("all function set diable\r\n");
        }
    }
    else
    {
        printf("Fucntion 1?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 1 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit = 0x02;
            if (1 == enab)
            {
                printf("function 1 set enable\r\n");
            }
            else
            {
                printf("function 1 set diable\r\n");
            }
        }
        else
        {
            func_bit = 0x00;
        }

        printf("Fucntion 2?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 2 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x04;
            if (1 == enab)
            {
                printf("function 2 set enable\r\n");
            }
            else
            {
                printf("function 2 set diable\r\n");
            }
        }

        printf("Fucntion 3?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 3 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x08;
            if (1 == enab)
            {
                printf("function 3 set enable\r\n");
            }
            else
            {
                printf("function 3 set diable\r\n");
            }
        }

        printf("Fucntion 4?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 4 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x10;
            if (1 == enab)
            {
                printf("function 4 set enable\r\n");
            }
            else
            {
                printf("function 4 set diable\r\n");
            }
        }

        printf("Fucntion 5?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 5 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x20;
            if (1 == enab)
            {
                printf("function 5 set enable\r\n");
            }
            else
            {
                printf("function 5 set diable\r\n");
            }
        }

        printf("Fucntion 6?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 6 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x40;
            if (1 == enab)
            {
                printf("function 6 set enable\r\n");
            }
            else
            {
                printf("function 6 set diable\r\n");
            }
        }

        printf("Fucntion 7?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 7 is invalid.\r\n");
            return CMD_OK;
        }

        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x80;
            if (1 == enab)
            {
                printf("function 7 set enable\r\n");
            }
            else
            {
                printf("function 7 set diable\r\n");
            }
        }
    }

    if (1 == enab)
    {
        err = sdio_enable_int(sd_port);
        if (SD_OK != err)
        {
            err2 = sd_get_error(sd_port);
            if (SD_OK != err2)
            {
                err = err2;
            }
            sdhi_show_error_msg(err);
            return CMD_OK;
        }
        err = sdio_set_intcallback(sd_port, sdio_int_callback);
        if (SD_OK != err)
        {
            err2 = sd_get_error(sd_port);
            if (SD_OK != err2)
            {
                err = err2;
            }
            sdhi_show_error_msg(err);
            return CMD_OK;
        }
    }
    else
    {
        err = sdio_disable_int(sd_port);
        if (SD_OK != err)
        {
            err2 = sd_get_error(sd_port);
            if (SD_OK != err2)
            {
                err = err2;
            }
            sdhi_show_error_msg(err);
            return CMD_OK;
        }
    }

    err = sdio_set_int(sd_port, func_bit, enab);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_int
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_intmask
 * Description  : "SDIOMASK" command
 *              : Set SDIO interrupt mask.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_intmask(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    char_t         temp[16];
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    int_t          enab;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("set SDIO Interrupt Mask for card port 0\r\n");
    }
    else
    {
        printf("set SDIO Interrupt Mask for card port 1\r\n");
    }

    printf("SDIO Interrupt Enable?:");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK != cmd_err)
    {
        printf("SDIO Interrupt Enable is invalid.\r\n");
        return CMD_OK;
    }

    if ( ('y' == temp[0]) || ('Y' == temp[0]) )
    {
        printf("set enable\r\n");
        enab = 1;
    }
    else
    {
        printf("set disable\r\n");
        enab = 0;
    }

    if (1 == enab)
    {
        err = sdio_enable_int(sd_port);
        if (SD_OK != err)
        {
            err2 = sd_get_error(sd_port);
            if (SD_OK != err2)
            {
                err = err2;
            }
            sdhi_show_error_msg(err);
            return CMD_OK;
        }
    }
    else
    {
        err = sdio_disable_int(sd_port);
        if (SD_OK != err)
        {
            err2 = sd_get_error(sd_port);
            if (SD_OK != err2)
            {
                err = err2;
            }
            sdhi_show_error_msg(err);
            return CMD_OK;
        }
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_intmask
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_setblk
 * Description  : "SDIOSETBLK" command
 *              : Set SDIO block size.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_setblk(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        temp;
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint32_t       func_num;
    uint16_t       blocklen;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("set SDIO Block Size for card port 0\r\n");
    }
    else
    {
        printf("set SDIO Block Size for card port 1\r\n");
    }

    printf("Please select SDIO Function Number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDIO Function Number is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    func_num = (uint32_t)temp;

    printf("Please input size of function block:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input size of function block is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    blocklen = (uint16_t)temp;

    err = sdio_set_blocklen(sd_port, blocklen, func_num);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_setblk
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_getblk
 * Description  : "SDIOGETBLK" command
 *              : Get SDIO block size.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_getblk(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        temp;
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint32_t       func_num;
    uint16_t       blocklen;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("get SDIO Block Size for card port 0\r\n");
    }
    else
    {
        printf("get SDIO Block Size for card port 1\r\n");
    }

    printf("Please select SDIO Function Number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDIO Function Number is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    func_num = (uint32_t)temp;

    err = sdio_get_blocklen(sd_port, &blocklen, func_num);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    printf("SDIO Block Size is %d\r\n", blocklen);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_getblk
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_enable
 * Description  : "SDIOENB" command
 *              : Set io function ready.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_enable(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    char_t         temp[16];
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint8_t        func_bit;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("set SDIO Enable for card port 0\r\n");
    }
    else
    {
        printf("set SDIO Enable for card port 1\r\n");
    }

    printf("All Fucntion?:");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK != cmd_err)
    {
        printf("All Fucntion is invalid.\r\n");
        return CMD_OK;
    }

    if ( ('y' == temp[0]) || ('Y' == temp[0]) )
    {
        func_bit = 0xfe;
        printf("all function set enable\r\n");
    }
    else
    {
        printf("Fucntion 1?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 1 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit = 0x02;
            printf("function 1 set enable\r\n");
        }
        else
        {
            func_bit = 0x00;
        }

        printf("Fucntion 2?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 2 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x04;
            printf("function 2 set enable\r\n");
        }

        printf("Fucntion 3?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 3 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x08;
            printf("function 3 set enable\r\n");
        }

        printf("Fucntion 4?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 4 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x10;
            printf("function 4 set enable\r\n");
        }

        printf("Fucntion 5?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 5 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x20;
            printf("function 5 set enable\r\n");
        }

        printf("Fucntion 6?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 6 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x40;
            printf("function 6 set enable\r\n");
        }

        printf("Fucntion 7?:");
        cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
        if (CMD_OK != cmd_err)
        {
            printf("Fucntion 7 is invalid.\r\n");
            return CMD_OK;
        }
        if ( ('y' == temp[0]) || ('Y' == temp[0]) )
        {
            func_bit |= 0x80;
            printf("function 7 set enable\r\n");
        }
    }

    err = sdio_set_enable(sd_port, func_bit);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_enable
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_reset
 * Description  : "SDIORESET" command
 *              : Reset SDIO function.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_reset(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("Reset SDIO Enable for card port 0\r\n");
    }
    else
    {
        printf("Reset SDIO Enable for card port 1\r\n");
    }

    err = sdio_reset(sd_port);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_reset
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_gint
 * Description  : "SDIOGINT" command
 *              : Get interrupt from io function.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_gint(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint8_t        func_bit;
    int32_t        enab;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("get SDIO Interrupt for card port 0\r\n");
    }
    else
    {
        printf("get SDIO Interrupt for card port 1\r\n");
    }

    err = sdio_get_int(sd_port, &func_bit, &enab);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    if (1 == enab)
    {
        printf("SDIO Interrupt enable %x\r\n",  func_bit);
    }
    else
    {
        printf("SDIO Interrupt disable %x\r\n", func_bit);
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_gint
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_grdy
 * Description  : "SDIOGRDY" command
 *              : Get ready io function.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_grdy(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        sd_port;
    int32_t        err;
    int32_t        err2;
    uint8_t        func_bit;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (0 == sd_port)
    {
        printf("get SDIO Ready for card port 0\r\n");
    }
    else
    {
        printf("get SDIO Ready for card port 1\r\n");
    }

    err = sdio_get_ready(sd_port, &func_bit);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    printf("SDIO Ready is %x\r\n", func_bit);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_grdy
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_final
 * Description  : "FINAL" command
 *              : Do finish operation of SD driver.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_final(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("sd_finalize\r\n");
    if ( sd_finalize(sd_port) != SD_OK)
    {
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_final
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_ioatt
 * Description  : "IOATT" command
 *              : Mount card.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_ioatt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    uint32_t       mode;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    mode = SD_CFG_DRIVER_MODE;

    if (1 == sd_port)
    {
        if (mode & (SD_MODE_SDR104 | SD_MODE_SDR50 | SD_MODE_SDR25 | SD_MODE_SDR12))
        {
            mode &= (~(SD_MODE_SDR104 | SD_MODE_SDR50 | SD_MODE_SDR25 | SD_MODE_SDR12));
            mode |= SD_MODE_HS;
        }
    }

    if ( mode & SD_MODE_HWINT )
    {
        printf(" Interrupt Mode\r\n");
    }
    else
    {
        printf(" Polling Mode\r\n");
    }

    if ( mode & SD_MODE_SDR104)
    {
        printf(" SDR104 Speed Mode\r\n");
    }
    else if ( mode & SD_MODE_SDR50)
    {
        printf(" SDR50 Speed Mode\r\n");
    }
    else if ( mode & SD_MODE_SDR25)
    {
        printf(" SDR25 Speed Mode\r\n");
    }
    else if ( mode & SD_MODE_SDR12)
    {
        printf(" SDR12 Speed Mode\r\n");
    }
    else if ( mode & SD_MODE_HS )
    {
        printf(" High-Speed Mode\r\n");
    }
    else
    {
        printf(" Default-Speed Mode\r\n");
    }

    if ( mode & SD_MODE_DMA )
    {
        printf(" DMA Mode\r\n");
    }
    else
    {
        printf(" PIO Mode\r\n");
    }

    err = sd_mount(sd_port, mode, SD_VOLT_3_3);
    if (SD_OK == err)
    {
        printf("mount sd pass.\r\n");
    }
    else if (SD_OK_LOCKED_CARD == err)
    {
        printf("mount sd pass. card is locked status\r\n");
    }
    else
    {
        printf("mount sd error.\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
    }


    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_ioatt
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_iodet
 * Description  : "IODET" command
 *              : Unmount card.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_iodet(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("sd_unmount\r\n");
    if (sd_unmount(sd_port) != SD_OK)
    {
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_iodet
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_format
 * Description  : "FORMAT" command
 *              : Format SD memory card.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_format(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
#if 0
    char_t         temp[16];
    int32_t        err;
    int32_t        mode;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please input format mode[QUICK(q)/FULL(f)]:");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK == cmd_err)
    {
        if ( ('q' == temp[0]) || ('Q' == temp[0]) )
        {
            printf("set mode quick format\r\n");
            mode = SD_FORMAT_QUICK;
        }
        else if ( ('f' == temp[0]) || ('F' == temp[0]) )
        {
            printf("set mode full format\r\n");
            mode = SD_FORMAT_FULL;
        }
        else
        {
            printf("set mode quick format\r\n");
            mode = SD_FORMAT_QUICK;
        }
    }
    else
    {
        printf("set mode quick format\r\n");
        mode = SD_FORMAT_QUICK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    if (SD_OK != (err = sd_format(sd_port, mode, (int32_t)0)))
    {
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
    }
    
#endif
    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_format
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_gettype
 * Description  : "SDTYPE" command
 *              : Get card type.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_gettype(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        i;
    uint16_t       type;
    uint16_t       speed;
    uint8_t        capa;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (sd_get_type(sd_port, &type, &speed, &capa))
    {
        printf("get type error\r\n");
        return CMD_OK;
    }

    if (type == s_sdtype[4].code)
    {
        printf("This card is %s\r\n", s_sdtype[4].type);
        return CMD_OK;
    }
    for (i = 0; i < 4; i++)
    {
        if ((type & 0x00FFu) == s_sdtype[i].code)
        {
            printf("This card is %s\r\n", s_sdtype[i].type);


            break;
        }
    }

    if (speed & SD_SUP_HIGH_SPEED)
    {
        printf("Support High SPEED\r\n");
    }
    else
    {
        printf("Support Default SPEED\r\n");
    }


    if (speed & SD_CUR_HIGH_SPEED)
    {
        printf("Current High SPEED\r\n");
    }
    else
    {
        printf("Currently Default SPEED\r\n");
    }

    if (2 != i)
    {
        /* memory presents */
        if (1 == capa)
        {
            printf("High-Capacity card mount\r\n");
        }
        else
        {
            printf("Standard-Capacity card mount\r\n");
        }
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_gettype
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_getwp
 * Description  : "WP" command
 *              : Check hardware write protect.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_getwp(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        ret;
    int32_t        i;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    ret = sd_iswp(sd_port);

    if (ret < 0)
    {
        printf("getwp error\r\n");
        return CMD_OK;
    }

    for (i = 0; i < 5; i++)
    {
        if (ret == s_sdwp[i].code)
        {
            printf("This card is %s\r\n", s_sdwp[i].wp);
            break;
        }
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_getwp
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_getreg
 * Description  : "REG" command
 *              : Get card register.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_getreg(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        i;
    uint8_t        ocr[4];
    uint8_t        cid[16];
    uint8_t        csd[16];
    uint8_t        dsr[2];
    uint8_t        scr[8];
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (sd_get_reg(sd_port, ocr, cid, csd, dsr, scr))
    {
        printf("get reg error\r\n");
        return CMD_OK;
    }

    printf("OCR is ");
    for (i = 0; i < (sizeof(ocr)); i++)
    {
        printf("%02X ", ocr[i]);
    }
    printf("\r\n");

    printf("CID is ");
    for (i = 0; i < (sizeof(cid)); i++)
    {
        printf("%02X ", cid[i]);
    }
    printf("\r\n");

    printf("CSD is ");
    for (i = 0; i < (sizeof(csd)); i++)
    {
        printf("%02X ", csd[i]);
    }
    printf("\r\n");

    printf("DSR is ");
    for (i = 0; i < (sizeof(dsr)); i++)
    {
        printf("%02X ", dsr[i]);
    }
    printf("\r\n");

    printf("SCR is ");
    for (i = 0; i < (sizeof(scr)); i++)
    {
        printf("%02X ", scr[i]);
    }
    printf("\r\n");

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_getreg
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_get_cia
 * Description  : "SDIOCIA" command
 *              : Get SDIO cia.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_get_cia(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    uint8_t        reg[0x20];
    int32_t        sd_port;
    int32_t        temp;
    uint32_t       func_num;
    int32_t        size;
    uint32_t       cis_adrs;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please select SDIO Function Number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDIO Function Number is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    func_num = (uint32_t)temp;

    printf("Please input CIS size to read:");
    cmd_err = sd_get_num(pCom, &size);
    if (CMD_OK != cmd_err)
    {
        printf("Input CIS size to read is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");
    if ( size > SDIO_CIS_BUFF_SIZE )
    {
        printf("CIS Size Over!!\r\n");
        return CMD_OK;
    }

    err = sdio_get_cia(sd_port, &reg[0], &g_sdio_cis[0], func_num, size);
    if (SD_OK != err)
    {
        printf("sdio_get_cia error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    cis_adrs = (uint32_t)( ((uint32_t)reg[0x0b] << 16) | ((uint32_t)reg[0x0a] <<  8) | reg[0x09] );

    if (0 == func_num)
    {
        printf("CCCR is \r\n");
    }
    else
    {
        printf("FBR is \r\n");
    }
    sd_print_put_dmp2(&reg[0], 0x20, 4, 16);
    printf("\r\n");

    printf("CIS[%08x] is \r\n", cis_adrs);
    sd_print_put_dmp2(&g_sdio_cis[0], size, 4, 16);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_get_cia
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_get_ocr
 * Description  : "SDIOOCR" command
 *              :
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_get_ocr(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t     err;
    int32_t     sd_port;
    uint32_t    ocr_reg;
    int16_t     cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    err = sdio_get_ioocr(sd_port, &ocr_reg);
    if (SD_OK != err)
    {
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    printf("IO_OCR is %08x\r\n", ocr_reg);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_get_ocr
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_get_info
 * Description  : "SDIOINFO" command
 *              : Get sdio info.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_get_info(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        sd_port;
    uint8_t        io_info;
    uint8_t        funcnum;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    err = sdio_get_ioinfo(sd_port, &io_info);
    if (SD_OK != err)
    {
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    printf("IO_INFO is %02x\r\n", io_info);
    if ( io_info & 0x80 )
    {
        printf("  C is 1\r\n");
    }
    else
    {
        printf("  C is 0\r\n");
    }

    funcnum   = io_info;
    funcnum >>= 4;
    funcnum  &= 0x07;
    printf("  number of function is %d\r\n", funcnum);

    if ( io_info & 0x08 )
    {
        printf("  Memory is present\r\n");
    }
    else
    {
        printf("  Memory is not present\r\n");
    }

    if ( io_info & 0x01 )
    {
        printf("  switching 1.8V support\r\n");
    }
    else
    {
        printf("  switching 1.8V not support\r\n");
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_get_info
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_get_rca
 * Description  : "RCA" command
 *              : Get sd rca.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_get_rca(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint32_t       i;
    int32_t        err;
    uint8_t        rca[2];
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    err = sd_get_rca(sd_port, rca);
    if (SD_OK != err)
    {
        printf("sd_get_rca error\r\n");
        return CMD_OK;
    }

    printf("RCA is ");
    for (i = 0; i < (sizeof(rca)); i++)
    {
        printf("%02X ", rca[i]);
    }
    printf("\r\n");

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_get_rca
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_get_sdstatus
 * Description  : "SDSTATUS" command
 *              : Get sd status.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_get_sdstatus(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint32_t       i;
    int32_t        err;
    uint8_t        sdstatus[16];
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    err = sd_get_sdstatus(sd_port, sdstatus);
    if (SD_OK != err)
    {
        printf("ERROR : sd_get_sdstatus\r\n");
        return CMD_OK;
    }

    printf(" sdstatus is \n\t");
    for (i = 0; i < (sizeof(sdstatus)); i++)
    {
        printf("%02X ", sdstatus[i]);
    }
    printf("\r\n");

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_get_sdstatus
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_get_speed
 * Description  : "SPEED" command
 *              : Get speed.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_get_speed(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint8_t       clas;
    uint8_t       move;
    int32_t       sd_port;
    int16_t       cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (sd_get_speed(sd_port, &clas, &move))
    {
        printf("card status error\r\n");
        return CMD_OK;
    }

    if (0x01 == clas)
    {
        printf("Speed Class 2\r\n");
    }
    else if (0x02 == clas)
    {
        printf("Speed Class 4\r\n");
    }
    else if (0x03 == clas)
    {
        printf("Speed Class 6\r\n");
    }
    else if (0x04 == clas)
    {
        printf("Speed Class 10\r\n");
    }
    else
    {
        printf("Speed Class 0\r\n");
    }

    if (0 != move)
    {
        printf("Performance move is %d [MB/sec]\r\n", move);
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_get_speed
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_setsec
 * Description  : "SSEC" command
 *              : Set sector count.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_setsec(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        sec;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please input seccnt:");
    cmd_err = sd_get_num(pCom, &sec);
    if (CMD_OK != cmd_err)
    {
        printf("Input seccnt is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    if (sd_set_seccnt(sd_port, (int16_t)sec))
    {
        printf("\tERROR : set seccount\r\n");
        return CMD_OK;
    }

    printf("\tseccount is %d\r\n", sec);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_setsec
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_setblkcnt
 * Description  : "SBLK" command
 *              : Set block count.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_setblkcnt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        blk;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please input blkcnt:");
    cmd_err = sd_get_num(pCom, &blk);
    if (CMD_OK != cmd_err)
    {
        printf("Input blkcnt is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    if (sdio_set_blkcnt(sd_port, (int16_t)blk))
    {
        printf("\tERROR : set seccount\r\n");
        return CMD_OK;
    }

    printf("\tblkccount is %d\r\n", blk);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_setblkcnt
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_getsec
 * Description  : "GSEC" command
 *              : Get sector count.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_getsec(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        ret;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    ret = sd_get_seccnt(sd_port);

    if (ret < 0)
    {
        printf("ERROR : get seccount\r\n");
        return CMD_OK;
    }

    printf("\tseccount is %d\r\n", ret);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_getsec
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_getblkcnt
 * Description  : "GBLK" command
 *              : Get block count.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_getblkcnt(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        ret;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    ret = sdio_get_blkcnt(sd_port);

    if (ret < 0)
    {
        printf("ERROR : get seccount\r\n");
        return CMD_OK;
    }

    printf("\tblkcount is %d\r\n", ret);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_getblkcnt
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_getipv
 * Description  : "IPVER" command
 *              : Get sdip version.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_getipv(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint16_t      sdhi_ver;
    char_t        sddrv_ver[32];
    int32_t       sd_port;
    int16_t       cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (sd_get_ver(sd_port, &sdhi_ver, sddrv_ver) == SD_ERR)
    {
        printf("Unpredictable error\r\n");
        return CMD_OK;
    }

    printf("SDIP version is 0x%04x\r\n", sdhi_ver);
    printf("%s\r\n", sddrv_ver);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_getipv
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_getsize
 * Description  : "SIZE" command
 *              : Get card size.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_getsize(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    uint32_t    user;
    uint32_t    prot;
    int32_t     sd_port;
    int16_t     cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    if (sd_get_size(sd_port, &user, &prot) != SD_OK)
    {
        printf("error\r\n");
        return CMD_OK;
    }

    printf("SD User SIZE = %d sectors\r\n", user);
    printf("SD Prot SIZE = %d sectors\r\n", prot);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_getsize
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_lock_unlock
 * Description  : LOCK/UNLOCK command
 *              : sd lock or unlock.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_lock_unlock(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        mode;
    int32_t        len;
    uint8_t        op_code;
    char_t         temp[16];
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("\r\n");
    printf("OPCODE:\r\n");
    printf("    0 : UNLOCK_CARD (0x%02x)\r\n", (0));
    printf("    1 : LOCK_CARD (0x%02x)\r\n", (SD_LOCK_CARD));
    printf("    2 : LOCK_CARD & SET_PWD(0x%02x)\r\n", (SD_LOCK_CARD | SD_SET_PWD));
    printf("    3 : UNLOCK_CARD & CLR_PWD(0x%02x)\r\n", (SD_UNLOCK_CARD | SD_CLR_PWD));
    printf("    4 : UNLOCK_CARD & SET_PWD(0x%02x)\r\n", (SD_UNLOCK_CARD | SD_SET_PWD));
    printf("    5 : FORCE_ERASE(0x%02x)\r\n", (SD_FORCE_ERASE));
    printf("\r\n");

    printf("Please input OPCODE:");
    cmd_err = sd_get_num(pCom, &mode);
    if (CMD_OK != cmd_err)
    {
        printf("Input OPECODE is invalid.\r\n");
        return CMD_OK;
    }

    printf("\r\n");

    switch (mode)
    {
        case 0:
            op_code = 0;
            printf("UNLOCK_CARD (0x%02x)\r\n", op_code);
            break;
        case 1:
            op_code = SD_LOCK_CARD ;
            printf("LOCK_CARD (0x%02x)\r\n", op_code);
            break;
        case 2:
            op_code = SD_LOCK_CARD | SD_SET_PWD;
            printf("LOCK_CARD & SET_PWD(0x%02x)\r\n", op_code);
            break;
        case 3:
            op_code = SD_UNLOCK_CARD | SD_CLR_PWD;
            printf("UNLOCK_CARD & CLR_PWD(0x%02x)\r\n", op_code);
            break;
        case 4:
            op_code = SD_UNLOCK_CARD | SD_SET_PWD;
            printf("UNLOCK_CARD & SET_PWD(0x%02x)\r\n", op_code);
            break;
        case 5:
            op_code = SD_FORCE_ERASE;
            printf("FORCE_ERASE(0x%02x)\r\n", op_code);
            break;
        default:
            goto lock_unlock_usage;
            break;
    }

    printf("Please input password :");
    cmd_err = sd_get_char(pCom, &temp[0], sizeof(temp));
    if (CMD_OK != cmd_err)
    {
        printf("Input password is invalid.\r\n");
        return CMD_OK;
    }
    printf("\npassward is %s\r\n", temp);

    printf("Please input password length:");
    cmd_err = sd_get_num(pCom, &len);
    if (CMD_OK != cmd_err)
    {
        printf("Input password length is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");
    printf("\npassward length is %d\r\n", len);

    /* Cast to an appropriate type */
    if (0 != (err = sd_lock_unlock(sd_port, op_code, (uint8_t*)temp, (uint8_t)len)))
    {
        printf("error: sd_lock_unlock\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }
    else
    {
        printf("pass: sd_lock_unlock\r\n");
    }

    return CMD_OK;

lock_unlock_usage:
    {
        printf("usage: %s <CR>\r\n", ppszArgument[0]);
    }
    printf("code = 0 : unlock card\r\n");
    printf("code = 1 : lock card\r\n");
    printf("code = 2 : set pwd and lock card\r\n");
    printf("code = 3 : clear pwd\r\n");
    printf("code = 4 : set pwd and unlock card\r\n");
    printf("code = 5 : forcing erase\r\n");

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_lock_unlock
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_read
 * Description  : "READ" command
 *              : sd read.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_read(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        temp;
    uint32_t       psn;
    int32_t        cnt;
    uint32_t       bytes;
    uint8_t        *p_rw_buff;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    p_rw_buff = (uint8_t *)&s_sd_rw_buff2[sd_port][0];

    printf("Please input sector number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input sector number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    psn = (uint32_t)temp;
    printf("\r\n");

    printf("Please input sector count:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input sector count is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    cnt = (int32_t)temp;
    printf("\r\n");
    if ( cnt > SD_TEST_SECTOR_NUM )
    {
        printf("error: sector count is over than buffer size!!\r\n");
        return CMD_OK;
    }

    err = sd_read_sect(sd_port, p_rw_buff, psn, cnt);
    if (SD_OK != err)
    {
        printf("error:read error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    bytes = (cnt * 512);
    sd_print_put_dmp2(&p_rw_buff[0], bytes, 4, 16);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_read
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sd_write
 * Description  : "WRITE" command
 *              : sd write.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sd_write(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        loop;
    int32_t        start_value;
    int32_t        err;
    int32_t        temp;
    uint32_t       psn;
    int32_t        cnt;
    uint32_t       bytes;
    uint32_t       bytes_temp;
    uint8_t        *p_rw_buff;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    p_rw_buff = (uint8_t *)&s_sd_rw_buff2[sd_port][0];

    printf("Please input sector number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input sector number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    psn = (uint32_t)temp;
    printf("\r\n");

    printf("Please input sector count:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input sector count is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    cnt = (int32_t)temp;
    printf("\r\n");
    if ( cnt > SD_TEST_SECTOR_NUM )
    {
        printf("error: sector count is over than buffer size!!\r\n");
        return CMD_OK;
    }

    printf("Please input Start Value to write sector:");
    cmd_err = sd_get_num(pCom, &start_value);
    if (CMD_OK != cmd_err)
    {
        printf("Input Start Value is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    bytes = (cnt * 512);

    loop        = 0;
    bytes_temp  = bytes;
    do
    {
        sd_make_buffer_data(&p_rw_buff[512 * loop], 512, 8, start_value);
        bytes_temp -= 512;
        if ( bytes_temp <= 0 )
        {
            break;
        }
        start_value++;
        loop++;

    }
    while (1);

    err = sd_write_sect(sd_port, p_rw_buff, psn, cnt, SD_WRITE_OVERWRITE);
    if (SD_OK != err)
    {
        printf("error:write error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sd_write
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_read_direct
 * Description  : "IOREAD_D" command
 *              : SDIO direct read.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_read_direct(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t        err;
    int32_t        temp;
    uint32_t       func;
    uint32_t       adr;
    uint8_t        data;
    int32_t        sd_port;
    int16_t        cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please input function number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input function number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    func = (uint32_t)temp;
    printf("\r\n");

    printf("Please input address to read(with 0x):");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input address to read(with 0x) is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    adr = (uint32_t)temp;
    printf("\r\n");

    err = sdio_read_direct(sd_port, &data, func, adr);
    if (SD_OK != err)
    {
        printf("error:read error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }
    else
    {
        printf("pass\r\n");
    }

    printf( "%08x : %02x\r\n", adr, data );

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_read_direct
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_write_direct
 * Description  : "IOWRITE_D" command
 *              : SDIO direct write.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_write_direct(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t       err;
    int32_t       temp;
    char_t        temp_s[16];
    uint32_t      func;
    uint32_t      adr;
    uint8_t       data;
    uint32_t      raw_flag;
    int32_t       sd_port;
    int16_t       cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    printf("Please input function number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input function number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    func = (uint32_t)temp;
    printf("\r\n");

    printf("Please input address to write(with 0x):");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input address to write(with 0x) is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    adr = (uint32_t)temp;
    printf("\r\n");

    printf("Please input data to write(with 0x):");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input data to write(with 0x) is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    data = (uint8_t)temp;
    printf("\r\n");

    printf("Verify after Read?:");
    cmd_err = sd_get_char(pCom, &temp_s[0], sizeof(temp_s));
    if (CMD_OK != cmd_err)
    {
        printf("Verify after Read is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' == temp_s[0]) || ('Y' == temp_s[0]) )
    {
        raw_flag = SD_IO_VERIFY_WRITE;
        printf(" Verify after Read\r\n");
    }
    else
    {
        raw_flag = SD_IO_SIMPLE_WRITE;
        printf(" Simple Read\r\n");
    }

    err = sdio_write_direct(sd_port, &data, func, adr, raw_flag);
    if (SD_OK != err)
    {
        printf("error:write error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }
    else
    {
        printf("pass\r\n");
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_write_direct
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_read
 * Description  : "IOREAD" command
 *              : SDIO read.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_read(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t       err;
    int32_t       temp;
    char_t        temp_s[16];
    uint32_t      func;
    uint32_t      adr;
    uint32_t      bytes;
    uint32_t      op_code;
    int32_t       sd_port;
    uint8_t       *p_rw_buff;
    int32_t       cnt;
    int16_t       cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    p_rw_buff = (uint8_t *)&s_sd_rw_buff2[sd_port][0];

    printf("Please input function number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input function number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    func = (uint32_t)temp;
    printf("\r\n");

    printf("Please input address to read(with 0x):");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input address to read(with 0x) is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    adr = (uint32_t)temp;
    printf("\r\n");

    printf("Please input size to read:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input size to read is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    cnt = (int32_t)temp;
    printf("\r\n");

    if ( cnt > (SD_TEST_SECTOR_NUM * 512) )
    {
        printf("error: sector count is over than buffer size!!\r\n");
        return CMD_OK;
    }

    printf("Incremental Address?:");
    cmd_err = sd_get_char(pCom, &temp_s[0], sizeof(temp_s));
    if (CMD_OK != cmd_err)
    {
        printf("Incremental Address is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' == temp_s[0]) || ('Y' == temp_s[0]) )
    {
        op_code = SD_IO_INCREMENT_ADDR;
        printf(" Incremental Address\r\n");
    }
    else
    {
        op_code = SD_IO_FIXED_ADDR;
        printf(" Fixed Address\r\n");
    }

    printf("Use CMD53_R_BLOCK Command?:");
    cmd_err = sd_get_char(pCom, &temp_s[0], sizeof(temp_s));
    if (CMD_OK != cmd_err)
    {
        printf("Use CMD53_R_BLOCK Command is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' != temp_s[0]) && ('Y' != temp_s[0]) )
    {
        op_code |= SD_IO_FORCE_BYTE;
        printf(" Force to Use CMD53_R_BYTE\r\n");
    }

    err = sdio_read(sd_port, p_rw_buff, func, adr, cnt, op_code);
    if (SD_OK != err)
    {
        printf("error:read error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }
    else
    {
        printf("pass\r\n");
    }

    /* Cast to an appropriate type */
    bytes = (uint32_t)cnt;
    sd_print_put_dmp2(&p_rw_buff[0], bytes, 4, 16);

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_read
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdio_write
 * Description  : "IOWRITE" command
 *              : SDIO write.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdio_write(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t       loop;
    int32_t       err;
    int32_t       temp;
    char_t        temp_s[16];
    uint32_t      func;
    uint32_t      adr;
    uint32_t      op_code;
    int32_t       sd_port;
    uint8_t       *p_rw_buff;
    int32_t       start_value;
    int32_t       cnt;
    int32_t       bytes_temp;
    int16_t       cmd_err;

    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("Please select SDHI port:");
    cmd_err = sd_check_port(pCom, &sd_port);
    if (CMD_OK != cmd_err)
    {
        printf("Select SDHI port is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    /* Cast to an appropriate type */
    p_rw_buff = (uint8_t *)&s_sd_rw_buff2[sd_port][0];

    printf("Please input function number:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input function number is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    func = (uint32_t)temp;
    printf("\r\n");

    printf("Please input address to write(with 0x):");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input address to write(with 0x) is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    adr = (uint32_t)temp;
    printf("\r\n");

    printf("Please input size to write:");
    cmd_err = sd_get_num(pCom, &temp);
    if (CMD_OK != cmd_err)
    {
        printf("Input size to write is invalid.\r\n");
        return CMD_OK;
    }

    /* Cast to an appropriate type */
    cnt = (int32_t)temp;
    printf("\r\n");

    if ( cnt > (SD_TEST_SECTOR_NUM * 512) )
    {
        printf("error: sector count is over than buffer size!!\r\n");
        return CMD_OK;
    }

    printf("Incremental Address?:");
    cmd_err = sd_get_char(pCom, &temp_s[0], sizeof(temp_s));
    if (CMD_OK != cmd_err)
    {
        printf("Incremental Address is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' == temp_s[0]) || ('Y' == temp_s[0]) )
    {
        op_code = SD_IO_INCREMENT_ADDR;
        printf(" Incremental Address\r\n");
    }
    else
    {
        op_code = SD_IO_FIXED_ADDR;
        printf(" Fixed Address\r\n");
    }

    printf("Use CMD53_W_BLOCK Command?:");
    cmd_err = sd_get_char(pCom, &temp_s[0], sizeof(temp_s));
    if (CMD_OK != cmd_err)
    {
        printf("Use CMD53_W_BLOCK Command is invalid.\r\n");
        return CMD_OK;
    }
    if ( ('y' != temp_s[0]) && ('Y' != temp_s[0]) )
    {
        op_code |= SD_IO_FORCE_BYTE;
        printf(" Force to Use CMD53_W_BYTE\r\n");
    }

    printf("Please input Start Value to write data:");
    cmd_err = sd_get_num(pCom, &start_value);
    if (CMD_OK != cmd_err)
    {
        printf("Input Start Value to write data is invalid.\r\n");
        return CMD_OK;
    }
    printf("\r\n");

    loop        = 0;
    bytes_temp  = cnt;
    do
    {
        sd_make_buffer_data(&p_rw_buff[512 * loop], 512, 8, start_value);
        bytes_temp -= 512;
        if ( bytes_temp <= 0 )
        {
            break;
        }
        start_value++;
        loop++;

    }
    while (1);

    err = sdio_write(sd_port, p_rw_buff, func, adr, cnt, op_code);
    if (SD_OK != err)
    {
        printf("error:write error\r\n");
        err = sd_get_error(sd_port);
        sdhi_show_error_msg(err);
        return CMD_OK;
    }
    else
    {
        printf("pass\r\n");
    }

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdio_write
******************************************************************************/

/******************************************************************************
 * Function Name: cmd_sdhi_help
 * Description  : "SDHELP" command
 *              : Output the command description of the SD driver to the terminal.
 * Arguments    : IN  iArgCount - The number of arguments in the argument list
                : IN  ppszArgument - The argument list
                : IN  pCom - Pointer to the command object
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t cmd_sdhi_help(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    /* cast parameters to void to suppress unused parameter warning */
    AVOID_UNUSED_WARNING;

    printf("\r\n");
    printf("  INIT      : Initialize SD Driver and SDIP\r\n");
    printf("  FINAL     : Finalize SD Driver and SDIP\r\n");
    printf("  IOATT     : Mount SD Card\r\n");
    printf("  IODET     : Unmount SD Card\r\n");
    printf("  FORMAT    : Format SD Card\r\n");

    /*    printf("  TYPE      : Display SD Card Type Information\r\n"); */
    printf("  SDTYPE    : Display SD Card Type Information\r\n");
    printf("  WP        : Display SD Card Write Protect Information\r\n");
    printf("  REG       : Display SD Card Register\r\n");
    printf("  RCA       : Display SD Card RCA Register\r\n");
    printf("  SDSTATUS  : Display SD Card SDSTATUS Register\r\n");
    printf("  SPEED     : Display SD Card Speed Information\r\n");
    printf("  SSEC      : Set SECCNT Value\r\n");
    printf("  GSEC      : Display SECCNT Value\r\n");
    printf("  SIZE      : Display SD Card Size Infotmation\r\n");
    printf("  IPVER     : Display SDIP Version Infotmation\r\n");
    printf("  LOCK      : Lock SD Card Operation\r\n");
    printf("  UNLOCK    : Unlock SD Card Operation\r\n");
    printf("  READ      : Read SD Card Sector\r\n");
    printf("  WRITE     : Write SD Card Sector\r\n");
    printf("\r\n");
    printf("  SDIOCIA   : Read  SDIO CIA\r\n");
    printf("  SDIOOCR   : Read  SDIO OCR\r\n");
    printf("  SDIOINFO  : Read  SDIO INFORMATION\r\n");
    printf("  SDIOENB   : Set   SDIO IO Function Enable\r\n");
    printf("  SDIORESET : Reset SDIO IO\r\n");
    printf("  SDIOGRDY  : Get   SDIO IO Function Ready\r\n");
    printf("  SDIOGINT  : Get   SDIO IO Interrupt Setting\r\n");
    printf("  SDIOINT   : Set   SDIO IO Interrupt [Enable or Disable]\r\n");
    printf("  SDIOMASK  : Set   SDIO_IO Interrupt Mask [Enable or Disable]\r\n");
    printf("  SDIOSETBLK: Set   SDIO_IO Block Size\r\n");
    printf("  SDIOGETBLK: Get   SDIO_IO Block Size\r\n");
    printf("  IOREAD_D  : Read  SDIO with CMD52_R\r\n");
    printf("  IOWRITE_D : Write SDIO with CMD52_W\r\n");
    printf("  IOREAD    : Read  SDIO with CMD53_R\r\n");
    printf("  IOWRITE   : Write SDIO with CMD53_W\r\n");
    printf("  SBLK      : Set SDIO Block Count Value\r\n");
    printf("  GBLK      : Display SDIO Block Count Value\r\n");
    printf("\r\n");

    return CMD_OK;
}
/******************************************************************************
End of function cmd_sdhi_help
******************************************************************************/

/******************************************************************************
 * Function Name: sdhi_show_error_msg
 * Description  : Display error message.
 * Arguments    : IN  errorno - Error number
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t sdhi_show_error_msg(int32_t errorno)
{
    int32_t        i;

    for (i = 0; 0 != s_sd_error_msg[i].p_msg; i++)
    {
        if (s_sd_error_msg[i].errorno == errorno)
        {
            printf("ERROR (%d): %s\r\n", errorno, s_sd_error_msg[i].p_msg);
            return CMD_OK;
        }
    }

    printf("ERROR (%d): UNKNWON ERROR NO\r\n", errorno);

    return CMD_OK;
}
/******************************************************************************
End of function sdhi_show_error_msg
******************************************************************************/

/******************************************************************************
 * Function Name: sd_int_callback
 * Description  : Interrupt callback function.
 * Arguments    : IN  sd_port - channel no (0 or 1)
                : IN  cd - SD card insert or extract
 * Return value : CMD_OK for success
 ******************************************************************************/
int32_t sd_int_callback(int32_t sd_port, int32_t cd)
{
    if (0 == sd_port)
    {
        if (cd)
        {
            /* printf() can not use. */
            /* printf("SD Card insert in port0!\r\n"); */
            ;
        }
        else
        {
            /* printf("SD Card extract in port0!\r\n"); */
            ;
        }
    }
    else
    {
        if (cd)
        {
            /* printf("SD Card insert in port1!\r\n"); */
            ;
        }
        else
        {
            /* printf("SD Card extract in port1!\r\n"); */
            ;
        }
    }

    return CMD_OK;
}
/******************************************************************************
End of function sd_int_callback
******************************************************************************/

/******************************************************************************
 * Function Name: sdio_int_callback
 * Description  : SDIO Interrupt callback function.
 * Arguments    : IN  sd_port - channel no (0 or 1)
 * Return value : CMD_OK for success
 ******************************************************************************/
int32_t sdio_int_callback(int32_t sd_port)
{
    if (0 == sd_port)
    {
        /* printf() can not use. */
        /* printf("SDIO Interrupt in port0!\r\n"); */
        s_sdio_interrupt_flag[0] = 1;
    }
    else
    {
        /* printf() can not use. */
        /* printf("SDIO Interrupt in port1!\r\n"); */
        s_sdio_interrupt_flag[1] = 1;
    }

    return CMD_OK;
}
/******************************************************************************
End of function sdio_int_callback
******************************************************************************/

/******************************************************************************
 * Function Name: sdio_clear_sdio_interrupt
 * Description  : Clear SDIO interrupt signal.
 * Arguments    : IN  sd_port - channel no (0 or 1)
 * Return value : CMD_OK for success
 ******************************************************************************/
static int16_t sdio_clear_sdio_interrupt(int32_t sd_port)
{
    int32_t        err;
    int32_t        err2;

    /*    uint32_t       func; */
    /*    uint32_t       adr; */
    /*    uint8_t        data; */

    s_sdio_interrupt_flag[sd_port] = 0;

    /* clear SDIO Inrerrupt Signal */
    /*    func = 0; */
    /*    adr  = 0; */

    /*    err = sdio_read_direct(sd_port, &data, func, adr); */
    /*    if (SD_OK != err) */
    /*    { */
    /*        printf("error:read error\r\n"); */
    /*        err = sd_get_error(sd_port); */
    /*        sdhi_show_error_msg(err); */
    /*        return 1; */
    /*    } */
    /*    else */
    /*    { */
    /*        printf("pass\r\n"); */
    /*    } */

    /*    err = sdio_write_direct(sd_port, &data, func, adr, SD_IO_SIMPLE_WRITE); */
    /*    if (SD_OK != err) */
    /*    { */
    /*        printf("error:read error\r\n"); */
    /*        err = sd_get_error(sd_port); */
    /*        sdhi_show_error_msg(err); */
    /*        return 1; */
    /*    } */
    /*    else */
    /*    { */
    /*        printf("pass\r\n"); */
    /*    } */

    /* mask clear SDIO Interrupt */
    err = sdio_enable_int(sd_port);
    if (SD_OK != err)
    {
        err2 = sd_get_error(sd_port);
        if (SD_OK != err2)
        {
            err = err2;
        }
        sdhi_show_error_msg(err);
        return CMD_OK;
    }

    return CMD_OK;
}
/******************************************************************************
End of function sdio_clear_sdio_interrupt
******************************************************************************/

/******************************************************************************
 * Function Name: sd_print_put_dmp
 * Description  : Outputs normally the information according to the value specified
 * Arguments    : IN  Ptr - Data Pointer
                : IN  Size - Data Size
                : IN  Spc - Line Head Space
                : IN  Wid - Line Data Count
 * Return value : none
 ******************************************************************************/
static void sd_print_put_dmp(uint8_t *Ptr, uint16_t Size, uint16_t Spc, uint16_t Wid)
{
    uint16_t    i;
    uint16_t    j;
    uint16_t    k;

    if (0 != Wid)
    {
        for ( i = 0; i < Size;  )
        {
            for ( k = 0; k < Spc; ++k )
            {
                printf( "  " );
            }

            for ( j = 0; (i < Size) && (j < Wid); ++j, ++i, ++Ptr )
            {
                printf( "%02x", *Ptr );
                printf( "  " );
            }
            if ( i < Size )
            {
                printf( "\r\n" );
            }
        }
    }
    printf( "\r\n" );
}
/******************************************************************************
End of function sd_print_put_dmp
******************************************************************************/

/******************************************************************************
 * Function Name: sd_print_put_dmp2
 * Description  : Outputs normally the information according to the value specified
 * Arguments    : IN  Ptr - Data Pointer
                : IN  Size - Data Size
                : IN  Spc - Line Head Space
                : IN  Wid - Line Data Count
 * Return value : none
 ******************************************************************************/
static void sd_print_put_dmp2(uint8_t *Ptr, uint32_t Size, uint16_t Spc, uint16_t Wid)
{
    uint32_t        dump;

    do
    {
        if ( Size > 256 )
        {
            dump = 256;
        }
        else
        {
            /* Cast to an appropriate type */
            dump = (uint16_t)Size;
        }

        /* Cast to an appropriate type */
        sd_print_put_dmp( Ptr, (uint16_t)dump, Spc, Wid );
        printf( "\r\n" );

        Ptr  += dump;
        Size -= dump;

    }
    while ( Size > 0 );
}
/******************************************************************************
End of function sd_print_put_dmp2
******************************************************************************/

/******************************************************************************
 * Function Name: sd_make_buffer_data
 * Description  : Outputs normally the information according to the value specified
 * Arguments    : IN  Ptr - Data Pointer
                : IN  Size - Data Size
                : IN  Wid - Line Data Count
                : IN  start_value - Start Value
 * Return value : none
 ******************************************************************************/
static void sd_make_buffer_data(uint8_t *Ptr, uint32_t Size, uint16_t Wid, uint32_t start_value)
{
    int32_t         loop;
    uint8_t         value_char;
    uint16_t        value_short;
    uint32_t        value_long;
    uint8_t         *p_char;
    uint32_t        *p_long;
    uint16_t        *p_short;

    if (8 == Wid)
    {
        /* Cast to an appropriate type */
        value_char = (uint8_t)start_value;

        /* Cast to an appropriate type */
        p_char = (uint8_t *)Ptr;
        for ( loop = 0; loop < Size; loop++ )
        {
            *p_char = value_char;
            p_char++;
            value_char++;
        }
    }
    else if (16 == Wid)
    {
        /* Cast to an appropriate type */
        value_short = (uint16_t)start_value;

        /* Cast to an appropriate type */
        p_short = (uint16_t *)Ptr;
        for ( loop = 0; loop < Size; (loop = loop + 2) )
        {
            *p_short = value_short;
            p_short++;
            value_short++;
        }
    }
    else
    {
        /* Cast to an appropriate type */
        value_long = (uint32_t)start_value;

        /* Cast to an appropriate type */
        p_long = (uint32_t *)Ptr;
        for ( loop = 0; loop < Size; (loop = loop + 4) )
        {
            *p_long = value_long;
            p_long++;
            value_long++;
        }
    }
}
/******************************************************************************
End of function sd_make_buffer_data
******************************************************************************/

/*******************************************************************************
 * Function Name: sdio_check_sdio_interrupt_flag
 * Description  : Check if the SDIO interrupt signal is set.
 * Arguments    : none
 * Return Value : none
 ******************************************************************************/
void sdio_check_sdio_interrupt_flag(void)
{
    /* clear SDIO interrupt signal */
    if (1 == s_sdio_interrupt_flag[0])
    {
        sdio_clear_sdio_interrupt(0);
    }

    if (1 == s_sdio_interrupt_flag[1])
    {
        sdio_clear_sdio_interrupt(1);
    }
}
/******************************************************************************
End of function sdio_check_sdio_interrupt_flag
******************************************************************************/

/* End of File */
