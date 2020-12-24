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
* File Name    : fat_command.c
* Version      : 1.20
* Device(s)    : RZ/A2M
* Tool-Chain   : e2 studio (GCC ARM Embedded)
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M FAT/exFAT File System Sample Program - Command process
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
#include <string.h>
#include <stdlib.h>
#include "r_typedefs.h"
#include "command.h"
#include "r_os_abstraction_api.h"
#include "fat_sample.h"

/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
#if defined(__GNUC__)
uint8_t  g_sample_app_rw_buff[RW_BUFF_SIZE] __attribute__ ((section ("UNCACHED_BSS"), aligned(8)));
FATFS    g_fatfs __attribute__ ((section ("UNCACHED_BSS")));
FIL      g_fil __attribute__ ((section ("UNCACHED_BSS")));
#elif defined(__ICCARM__)
uint8_t  g_sample_app_rw_buff[RW_BUFF_SIZE] @ "UNCACHED_BSS";
FATFS    g_fatfs @ "UNCACHED_BSS";
FIL      g_fil @ "UNCACHED_BSS";
#else
    #error "Unsupported toolchain."

#endif /* __ICCARM__ */

uint32_t g_fat_sem_access = 0uL;

/******************************************************************************
* Function Name: FAT_Sample_Att
* Description  : Connects a drive.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Att(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t side;
    TCHAR   drive_str[DRIVE_LEN];
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    char_t  drive_chr[DRIVE_LEN];
    #endif
#endif
    FRESULT chk;
    const   char_t p_fs_type_str[FS_TYPE_NUM][FS_TYPE_LEN] = { "NON", "FAT12", "FAT16", "FAT32", "EXFAT" };
    bool_t  chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        side = atoi(ppszArgument[1]);
        if (side >= FF_VOLUMES)
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    memset(&g_fatfs, 0, sizeof(FATFS));

#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
    sprintf(drive_str, "%d:", side);
    #else
    sprintf(drive_chr, "%d:", side);
    char_to_tchar(drive_chr, drive_str);
    #endif
#else
    sprintf(drive_str, "%d:", side);
#endif

    if (0uL == g_fat_sem_access)
    {
        chk_sem = R_OS_SemaphoreCreate(&g_fat_sem_access, 1uL);
        if (false == chk_sem)
        {
            printf("semaphore create error.\r\n");
            return CMD_OK;
        }
    }

    chk = f_mount(&g_fatfs, drive_str, 1);  /* Mount logical drive. opt = 1 : Mount immediately */
    if (FR_OK == chk)
    {
        printf("device attach success.\r\n");
        printf("SIDE <%d>\r\n", side);
        if (g_fatfs.fs_type < FS_TYPE_NUM)
        {
            printf("\tFilesystem type          = %s\r\n", p_fs_type_str[g_fatfs.fs_type]);

                                                /* Filesystem type (0:N/A, 1:FAT12, 2:FAT16, 3:FAT32, 4:EXFAT)  */
        }
#if FF_MAX_SS != FF_MIN_SS
        printf("\tSector size [bytes]      = %d\r\n", g_fatfs.ssize);   
                                                /* Sector size (512, 1024, 2048 or 4096) */
#else
        printf("\tSector size [bytes]      = %d\r\n", FF_MAX_SS);
#endif
        printf("\tCluster size [sectors]   = %d\r\n", g_fatfs.csize);   /* Cluster size [sectors]   */
        printf("\tNumber of FATs (1 or 2)  = %d\r\n", g_fatfs.n_fats);  /* Number of FATs (1 or 2)  */
        printf("\tSize of an FAT [sectors] = %d\r\n", g_fatfs.fsize);   /* Size of an FAT [sectors] */
    }
    else
    {
        printf("device attach error.\r\n");
    }
    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Att
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Det
* Description  : Disconnects a drive.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Det(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t side;
    TCHAR   drive_str[DRIVE_LEN];
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    char_t  drive_chr[DRIVE_LEN];
    #endif
#endif
    FRESULT chk;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        side = atoi(ppszArgument[1]);
        if (side >= FF_VOLUMES)
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
    sprintf(drive_str, "%d:", side);
    #else
    sprintf(drive_chr, "%d:", side);
    char_to_tchar(drive_chr, drive_str);
    #endif
#else
    sprintf(drive_str, "%d:", side);
#endif

    chk = f_unmount(drive_str);
    if (FR_OK == chk)
    {
        printf(" device detach success.\r\n");
    }
    else
    {
        printf(" device detach error.\r\n");
    }
    R_OS_SemaphoreDelete(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Det
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Dir
* Description  : Displays a list of files and subdirectories in a directory.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Dir(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  res;
    DIR      dir;
    FILINFO  fno;
#if (FF_USE_LFN != 0)
    int32_t  len;
    int32_t  len_max;
    uint32_t str_num;
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    char_t   path_name_buff_tmp[CMD_READER_LINE_SIZE];
    char_t   *path_name_char;
    #endif
#endif
    TCHAR    *p_path_name;
    uint8_t  file_create_date_str[4+1+2+1+2+1];
    uint8_t  file_create_time_str[2+1+2+1];
    uint8_t  file_size_str1[MAX_FILE_SIZE_STRING_LENGTH + 1];
    uint8_t  file_size_str2[MAX_FILE_SIZE_STRING_LENGTH + 1 + (MAX_FILE_SIZE_STRING_LENGTH / 3)];
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        side = 0;   /* Default drive is 0 */

#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = "\\";
    #else
        path_name_char = "\\";
        char_to_tchar(path_name_char, path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = "\\";
#endif
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }
    /* Cast to an appropriate type */
    res = f_opendir(&dir, (TCHAR *)p_path_name);
    if (FR_OK == res)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if ((FR_OK != res) || (0 == fno.fname[0]))
            {
                /* Error or end of dir */
                break;
            }

#if (FF_FS_EXFAT == 1)
            /* Cast to an appropriate type */
            sprintf((char_t *)file_size_str1, "%*llu", MAX_FILE_SIZE_STRING_LENGTH, fno.fsize);
#else
            /* Cast to an appropriate type */
            sprintf((char_t *)file_size_str1, "%*lu", MAX_FILE_SIZE_STRING_LENGTH, fno.fsize);
#endif
            make_file_size_str(file_size_str2, file_size_str1);

            make_file_create_date_str(file_create_date_str, fno.fdate);
            make_file_create_time_str(file_create_time_str, fno.ftime);

            if (AM_DIR == (fno.fattrib & AM_DIR))
            {
                /* It is a directory */
                printf("<DIR> ");

                /* Cast to an appropriate type */
                printf("%*s ", strlen((const char *)file_size_str2), " ");
            }
            else
            {
                /* It is a file */
                printf("      ");
                printf("%s ", file_size_str2);
            }
            printf("%s ", file_create_date_str);
            printf("%s ", file_create_time_str);

#if (FF_USE_LFN != 0)
            if (FS_EXFAT != g_fatfs.fs_type)
            {
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
                str_num = strlen(fno.altname);
                printf("%s", fno.altname);
    #else
                tchar_to_char(fno.altname, path_name_buff_tmp);
                str_num = strlen(path_name_buff_tmp);
                printf("%s", path_name_buff_tmp);
    #endif
                len_max = ((sizeof(fno.altname))/sizeof(TCHAR));
                len = len_max - str_num;
                if (len_max > str_num)
                {
                    printf("%*s", (len_max-str_num), " ");
                }
            }
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
            printf("%s", fno.fname);
            printf("\r\n");
    #else
            tchar_to_char(fno.fname, path_name_buff_tmp);
            printf("%s", path_name_buff_tmp);
            printf("\r\n");
    #endif
#else
            printf("%s", fno.fname);
            printf("\r\n");
#endif
        }
        res = f_closedir(&dir);
    }
    else
    {
        printf("open dir error.\r\n");
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Dir
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Type
* Description  : Displays the contents of a file.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Type(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FSIZE_t  fsize;
    FRESULT  fr;
    UINT     br;
    UINT     cnt;
    int_t    char_mem;
    uint8_t  file_size_str1[MAX_FILE_SIZE_STRING_LENGTH + 1];
    uint8_t  file_size_str2[MAX_FILE_SIZE_STRING_LENGTH + 1 + (MAX_FILE_SIZE_STRING_LENGTH / 3)];
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR   *p_path_name;
    bool_t  chk_sem;

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_READ);
    if (FR_OK != fr)
    {
        printf("open file error.\r\n");
    }
    else
    {
        fsize = f_size(&g_fil);
#if (FF_FS_EXFAT == 1)
        /* Cast to an appropriate type */
        sprintf((char_t *)file_size_str1, "%*llu", MAX_FILE_SIZE_STRING_LENGTH, fsize);
#else
        /* Cast to an appropriate type */
        sprintf((char_t *)file_size_str1, "%*lu", MAX_FILE_SIZE_STRING_LENGTH, fsize);
#endif

        make_file_size_str(file_size_str2, file_size_str1);
        printf("File size = %s\r\n", file_size_str2);

        for (;;)
        {
            fr = f_read(&g_fil, &g_sample_app_rw_buff[0], RW_BUFF_SIZE, &br);
            if (FR_OK != fr)            /* Read error */
            {
                printf("read error.\r\n");
                break;
            }
            else if (0 == br)           /* eof */
            {
                break;
            }
            else
            {
                for (cnt = 0; cnt < br; cnt++)
                {
                    printf("%02X ", g_sample_app_rw_buff[cnt]);

                    if (15 == (cnt % 16))
                    {
                        printf("\r\n");
                    }
                }
                printf("\r\n");
            }
            printf("Press enter key to continue, other to break.\r\n");

            char_mem = fgetc(pCom->p_in);
            if ('\r' != char_mem)      /* CR process */
            {
                break;
            }
        }
        fr = f_close(&g_fil);
        if (FR_OK != fr)
        {
            printf("file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Type
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Write
* Description  : Write a sample string.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Write(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_FS_EXFAT == 0)
    const char_t  p_write_buff[] = "Renesas FAT sample.";
#else
    const char_t  p_write_buff[] = "Renesas FAT/exFAT sample.";
#endif
    UINT    bw;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr)
    {
        printf("open file error.\r\n");
    }
    else
    {
        memcpy(&g_sample_app_rw_buff[0], &p_write_buff[0], sizeof(p_write_buff));
        fr = f_write(&g_fil, &g_sample_app_rw_buff[0], sizeof(p_write_buff), &bw);
        if (FR_OK != fr)
        {
            printf("write error.\r\n");
        }
        else
        {
            printf("write success.\r\n");
        }
        fr = f_close(&g_fil);
        if (FR_OK != fr)
        {
            printf("file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Write
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Write
* Description  : Write a sample string.
* Arguments    : IN  ppathname - file name.
*              : IN  pwrite_data - write data.
* Return Value : CMD_OK for success
******************************************************************************/
#define BUF_SIZE    30
int16_t FAT_Write(char_t *ppathname, char_t *pwrite_data)
{
    int32_t  side;
    FRESULT  fr;
    UINT     bw;
    bool_t   chk_sem;
    TCHAR    *p_path_name;

    /* Cast to an appropriate type */
    side = get_side((uint8_t *)ppathname);
    if ((side < 0) || (side >= FF_VOLUMES))
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    p_path_name = &ppathname[0];
    
    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr)
    {
        printf("open file error.\r\n");
    }
    else
    {
        memcpy(&g_sample_app_rw_buff[0], &pwrite_data[0], BUF_SIZE);
        fr = f_write(&g_fil, &g_sample_app_rw_buff[0], BUF_SIZE, &bw);
        if (FR_OK != fr)
        {
            printf("write error.\r\n");
        }
        else
        {
            printf("write success.\r\n");
        }
        fr = f_close(&g_fil);
        if (FR_OK != fr)
        {
            printf("file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Write
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Read
* Description  : Read a sample string.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Read(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_FS_EXFAT == 0)
    const char_t  p_write_buff[] = "Renesas FAT sample.";
#else
    const char_t  p_write_buff[] = "Renesas FAT/exFAT sample.";
#endif
    UINT     br;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    bool_t   chk_sem;
    UINT     cnt;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_READ);
    if (FR_OK != fr)
    {
        printf("open file error.\r\n");
    }
    else
    {
        memset(&g_sample_app_rw_buff[0], 0, sizeof(p_write_buff));
        fr = f_read(&g_fil, &g_sample_app_rw_buff[0], RW_BUFF_SIZE, &br);
        if (FR_OK != fr)
        {
            printf("read error.\r\n");
        }
        else
        {
            printf("read success.\r\n");
            for (cnt = 0; cnt < br; cnt++)
            {
                printf("%02X ", g_sample_app_rw_buff[cnt]);

                if (15 == (cnt % 16))
                {
                    printf("\r\n");
                }
            }
            printf("\r\n");
        }
        fr = f_close(&g_fil);
        if (FR_OK != fr)
        {
            printf("file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Read
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Read
* Description  : Read a sample string.
* Arguments    : IN  ppathname - file name.
*              : OUT  pread_data - read data.
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Read(char_t *ppathname, char_t *pread_data)
{
    int32_t  side;
    FRESULT  fr;
    UINT     br;
    bool_t   chk_sem;
    TCHAR    *p_path_name;
    int16_t  ret = CMD_OK;

    /* Cast to an appropriate type */
    side = get_side((uint8_t *)ppathname);
    if ((side < 0) || (side >= FF_VOLUMES))
    {
        printf("arg error. Please type HELP \r\n");
        return (-1);
    }
    p_path_name = &ppathname[0];
    
    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return (-1);
    }

    fr = f_open(&g_fil, p_path_name, FA_READ);
    if (FR_OK != fr)
    {
        printf("open file error.\r\n");
        ret = (-1);
    }
    else
    {
        memset(&g_sample_app_rw_buff[0], 0, sizeof(g_sample_app_rw_buff));
        fr = f_read(&g_fil, &g_sample_app_rw_buff[0], RW_BUFF_SIZE, &br);
        if (FR_OK != fr)
        {
            printf("read error.\r\n");
            ret = (-1);
        }
        else
        {
            printf("read success.\r\n");
            memcpy(&pread_data[0], &g_sample_app_rw_buff[0], BUF_SIZE);
            ret = CMD_OK;

        }
        fr = f_close(&g_fil);
        if (FR_OK != fr)
        {
            printf("file close error.\r\n");
            ret = (-1);
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return ret;
}
/*******************************************************************************
 End of function FAT_Read
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Create
* Description  : Create new file.
* Arguments    : IN  ppathname - file name.
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Create(char_t *ppathname)
{
    int32_t  side;
    FRESULT  fr;
    bool_t   chk_sem;
    TCHAR    *p_path_name;

    /* Cast to an appropriate type */
    side = get_side((uint8_t *)ppathname);
    if ((side < 0) || (side >= FF_VOLUMES))
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    p_path_name = &ppathname[0];
    
    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_CREATE_NEW | FA_WRITE);
    if (FR_OK != fr)
    {
        printf("create file error.\r\n");
    }
    else
    {
        fr = f_close(&g_fil);
        if (FR_OK == fr)
        {
            printf("create file success.\r\n");
        }
        else
        {
            printf("create file, but file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Create
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Create
* Description  : Create new file.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Create(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }

    fr = f_open(&g_fil, p_path_name, FA_CREATE_NEW | FA_WRITE);
    if (FR_OK != fr)
    {
        printf("create file error.\r\n");
    }
    else
    {
        fr = f_close(&g_fil);
        if (FR_OK == fr)
        {
            printf("create file success.\r\n");
        }
        else
        {
            printf("create file, but file close error.\r\n");
        }
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Create
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Del
* Description  : Delete file.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Del(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    FILINFO  fno;
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    fr = f_stat(p_path_name, &fno);
    if (FR_OK == fr)
    {
        if ((fno.fattrib & AM_DIR) != AM_DIR)
        {
            chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
            if (false == chk_sem)
            {
                printf("File access conflicts.\r\n");
                return CMD_OK;
            }
            fr = f_unlink(p_path_name);
            if (FR_OK == fr)
            {
                printf("delete file success.\r\n");
            }
            else
            {
                printf("delete file error.\r\n");
            }
            R_OS_SemaphoreRelease(&g_fat_sem_access);
        }
        else
        {
            printf("delete error.\r\n");
        }
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
    }

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Del
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Mkdir
* Description  : Create new directory.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Mkdir(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
    if (false == chk_sem)
    {
        printf("File access conflicts.\r\n");
        return CMD_OK;
    }
    fr = f_mkdir(p_path_name);
    if (FR_OK == fr)
    {
        printf("create directory success.\r\n");
    }
    else
    {
        printf("create directory error.\r\n");
    }
    R_OS_SemaphoreRelease(&g_fat_sem_access);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Mkdir
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Rmdir
* Description  : Delete directory.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Rmdir(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    int32_t  side;
    FRESULT  fr;
#if (FF_USE_LFN != 0)
    #if (FF_LFN_UNICODE == 1) || (FF_LFN_UNICODE == 3)
    TCHAR    path_name_buff[CMD_READER_LINE_SIZE];
    #endif
#endif
    TCHAR    *p_path_name;
    FILINFO  fno;
    bool_t   chk_sem;

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    if (1 == iArgCount)
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }
    else if (2 == iArgCount)
    {
        /* Cast to an appropriate type */
        side = get_side((uint8_t *)ppszArgument[1]);
        if ((side < 0) || (side >= FF_VOLUMES))
        {
            printf("arg error. Please type HELP \r\n");
            return CMD_OK;
        }
#if (FF_USE_LFN != 0) 
    #if (FF_LFN_UNICODE == 0) || (FF_LFN_UNICODE == 2)
        p_path_name = ppszArgument[1];
    #else
        /* Cast to an appropriate type */
        char_to_tchar((char_t *)ppszArgument[1], path_name_buff);
        p_path_name = path_name_buff;
    #endif
#else
        p_path_name = ppszArgument[1];
#endif
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
        return CMD_OK;
    }

    fr = f_stat(p_path_name, &fno);
    if (FR_OK == fr)
    {
        if (AM_DIR == (fno.fattrib & AM_DIR))
        {
            chk_sem = R_OS_SemaphoreWait(&g_fat_sem_access, FS_SEM_WAITTIME);
            if (false == chk_sem)
            {
                printf("File access conflicts.\r\n");
                return CMD_OK;
            }
            fr = f_unlink(p_path_name);
            if (FR_OK == fr)
            {
                printf("delete directory success.\r\n");
            }
            else
            {
                printf("delete directory error.\r\n");
            }
            R_OS_SemaphoreRelease(&g_fat_sem_access);
        }
        else
        {
            printf("delete error.\r\n");
        }
    }
    else
    {
        printf("arg error. Please type HELP \r\n");
    }
    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Rmdir
 ******************************************************************************/

/******************************************************************************
* Function Name: FAT_Sample_Exit
* Description  : Exit fat sample mode.
* Arguments    : IN  iArgCount - The number of arguments in the argument list
*              : IN  ppszArgument - The argument list
*              : IN  pCom - Pointer to the command object
* Return Value : CMD_OK for success
******************************************************************************/
int16_t FAT_Sample_Exit(int_t iArgCount, char_t **ppszArgument, pst_comset_t pCom)
{
    /* Cast to an appropriate type */
    UNUSED_PARAM(iArgCount);

    /* Cast to an appropriate type */
    UNUSED_PARAM(ppszArgument);

    /* Cast to an appropriate type */
    UNUSED_PARAM(pCom);

    return CMD_OK;
}
/*******************************************************************************
 End of function FAT_Sample_Exit
 ******************************************************************************/

/* End of File */

