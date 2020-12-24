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
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/*******************************************************************************
* File Name    : main.c
* Device(s)    : RZ/A2M
* Tool-Chain   : e2Studio Ver 7.8.0
*              : GCC ARM Embedded 6.3.1.20170620
* OS           : None
* H/W Platform : RZ/A2M Evaluation Board
* Description  : RZ/A2M Sample Program - Main
* Operation    :
* Limitations  :
*******************************************************************************/


/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "r_typedefs.h"
#include "iodefine.h"
#include "r_cpg_drv_api.h"
#include "r_ostm_drv_api.h"
#include "r_scifa_drv_api.h"
#include "r_gpio_drv_api.h"
#include "r_startup_config.h"
#include "compiler_settings.h"
#include "main.h"
#include "r_os_abstraction_api.h"
#include "r_task_priority.h"

#include "iodefine.h"
#include "ff.h"
#include "fat_sample.h"
#include "r_sdif.h"
//#include "r_sdhi_drv_sc_cfg.h"
#include "r_sdhi_simplified_drv_sc_cfg.h"
#include "r_sd_cfg.h"
#include "application_common.h"

/******************************************************************************
Macro definitions
******************************************************************************/
#define FAT_SAMPLE_PRV_DEMO_CONSOLE

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
/* fat sample channel */
#define FAT_SAMPLE_PRV_CH               (1)
#define FAT_SAMPLE_PRV_DRV_LETTER       (1)
#define FAT_SAMPLE_PRV_CH_NUM           (FF_VOLUMES)
#define FAT_SAMPLE_PRV_RW_PROC_NUM      (10uL)
#define FAT_SAMPLE_PRV_100MS            (100uL)
#define FAT_SAMPLE_PRV_300MS            (300uL)
#define FAT_SAMPLE_PRV_500MS            (500uL)
#define FAT_SAMPLE_PRV_KEY_INPUT_CYCLE  (FAT_SAMPLE_PRV_100MS)
#define FAT_SAMPLE_PRV_QUEUE_GET_CYCLE  (FAT_SAMPLE_PRV_100MS)

#define FAT_SAMPLE_PRV_PATH_LEN         (20)
#define FAT_SAMPLE_PRV_QUEUE_NUM        (10uL)
#define FAT_SAMPLE_PRV_RW_BUFF_SIZE     (512u)
#define FAT_SAMPLE_PRV_SEM_WAITTIME     (500uL)

#define FAT_SAMPLE_PRV_CATT_NUM_LONG    (50uL)
#define FAT_SAMPLE_PRV_CATT_NUM_SHORT   (2uL)

/******************************************************************************
Typedef definitions
******************************************************************************/
typedef void (*p_fatSampleFunc)(void);

typedef enum
{
    FAT_SAMPLE_EVENT_POWER_ON,
    FAT_SAMPLE_EVENT_CD_INSERT,
    FAT_SAMPLE_EVENT_CD_REMOVE,
    FAT_SAMPLE_EVENT_KEY_INPUT,
    FAT_SAMPLE_EVENT_NUM
} e_fat_sample_event_t;

typedef enum
{
    FAT_SAMPLE_MODE_DEFAULT,
    FAT_SAMPLE_MODE_CD_INSERTED,
    FAT_SAMPLE_MODE_CD_REMOVED,
    FAT_SAMPLE_MODE_NUM
} e_fat_sample_mode_t;

typedef enum
{
    FAT_SAMPLE_OK,
    FAT_SAMPLE_NG
} e_fat_sample_ret_t;
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
extern void sample_main(void);

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
static int_t gs_my_gpio_handle;

/* Cast to an appropriate type */
static st_r_drv_gpio_pin_rw_t s_p01_hi = { GPIO_PORT_0_PIN_1, GPIO_LEVEL_HIGH,       GPIO_SUCCESS };

/* Cast to an appropriate type */
static st_r_drv_gpio_pin_rw_t s_p01_lo = { GPIO_PORT_0_PIN_1, GPIO_LEVEL_LOW,        GPIO_SUCCESS };

/* Cast to an appropriate type */
static st_r_drv_gpio_pin_rw_t s_p03_hi = { GPIO_PORT_0_PIN_3, GPIO_LEVEL_HIGH,       GPIO_SUCCESS };

/* Cast to an appropriate type */
static st_r_drv_gpio_pin_rw_t s_p03_lo = { GPIO_PORT_0_PIN_3, GPIO_LEVEL_LOW,        GPIO_SUCCESS };

/* Cast to an appropriate type */
static st_r_drv_gpio_pin_rw_t s_pj1    = { GPIO_PORT_D_PIN_7, GPIO_LEVEL_SC_DEFAULT, GPIO_SUCCESS };
static const r_gpio_port_pin_t gs_led_pin_list[] =
{
    GPIO_PORT_0_PIN_1,

    /* Cast to an appropriate type */
    GPIO_PORT_0_PIN_3
};
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/* Terminal window escape sequences */
static const char_t * const sp_clear_screen = "\x1b[2J";
static const char_t * const sp_cursor_home  = "\x1b[H";

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
static TCHAR                s_fat_sample_drive_path[FAT_SAMPLE_PRV_PATH_LEN];
static TCHAR                s_fat_sample_file_path[FAT_SAMPLE_PRV_PATH_LEN];
static uint32_t             s_fat_sem_key_input = 0uL;
static e_fat_sample_mode_t  s_fat_sample_mode = FAT_SAMPLE_MODE_DEFAULT;

extern void cmd_console(FILE* pIn, FILE *pOut, char_t *pszPrompt);

static e_r_drv_gpio_level_t fat_sample_key_input(void);
static e_fat_sample_ret_t fat_sample_write_proc(void);
static e_fat_sample_ret_t fat_sample_read_proc(void);
static void os_fat_sample_key_input_task_t(void *params);
//static void fat_sample_led_off(void);
//static void fat_sample_led_on(void);
//static void fat_sample_led_error(void);
static void fat_sample_dummy_proc(void);
static void fat_sample_power_on_proc(void);
static void fat_sample_cd_insert_proc(void);
static void fat_sample_cd_remove_proc(void);
static void fat_sample_key_input_proc(void);
static void os_fat_sample_main_task_t(void *params);

static const p_fatSampleFunc s_tbl_fat_sample_proc[FAT_SAMPLE_EVENT_NUM][FAT_SAMPLE_MODE_NUM] =
{
    /* FAT_SAMPLE_MODE_DEFAULT,  FAT_SAMPLE_MODE_CD_INSERTED, FAT_SAMPLE_MODE_CD_REMOVED */
    { fat_sample_power_on_proc, fat_sample_dummy_proc,       fat_sample_dummy_proc     }, /* POWER_ON  */
    { fat_sample_dummy_proc,    fat_sample_dummy_proc,       fat_sample_cd_insert_proc }, /* CD_INSERT */
    { fat_sample_dummy_proc,    fat_sample_cd_remove_proc,   fat_sample_dummy_proc     }, /* CD_REMOVE */
    { fat_sample_dummy_proc,    fat_sample_key_input_proc,   fat_sample_dummy_proc     }  /* KEY_INPUT */
};

static e_fat_sample_event_t    s_fat_sample_isr_event[2];
static p_os_msg_queue_handle_t s_fat_sample_queue_handle;
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/******************************************************************************
* Function Name: os_console_task_t
* Description  : console task
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t os_console_task_t(void)
{
    /* never exits */

    while(1)
    {
       /* ==== Receive command, activate sample software ==== */
#if (FF_FS_EXFAT == 0)
        cmd_console(stdin, stdout, "FAT>");
#else
        cmd_console(stdin, stdout, "exFAT>");
#endif

        R_OS_TaskSleep(500);
    }
}
/*******************************************************************************
 End of function os_console_task_t
 ******************************************************************************/

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
/******************************************************************************
* Function Name: fat_sample_key_input
* Description  : Get the key input value.
* Arguments    : none
* Return Value : the key input value
******************************************************************************/
static e_r_drv_gpio_level_t fat_sample_key_input(void)
{
    int_t gpio_err;
    e_r_drv_gpio_level_t ret;

    gpio_err = direct_control(gs_my_gpio_handle, CTL_GPIO_PIN_READ, &s_pj1);
    if (gpio_err < 0)
    {
        ret = GPIO_LEVEL_SC_DEFAULT;
    }
    else
    {
        ret = s_pj1.level;
    }
    return ret;
}
/*******************************************************************************
 End of function fat_sample_key_input
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_write_proc
* Description  : Fat sample write process
* Arguments    : none
* Return Value : FAT_SAMPLE_OK for success
******************************************************************************/
static e_fat_sample_ret_t fat_sample_write_proc(void)
{
    e_fat_sample_ret_t fat_sample_ret;
    bool_t             chk_sem;
    FRESULT            chk_fr;
    UINT               bw;
#if (FF_FS_EXFAT == 0)
    const char_t       p_write_buff[] = "Renesas FAT sample.";
#else
    const char_t       p_write_buff[] = "Renesas FAT/exFAT sample.";
#endif

    fat_sample_ret = FAT_SAMPLE_NG;
    chk_sem        = R_OS_SemaphoreWait(&g_fat_sem_access, FAT_SAMPLE_PRV_SEM_WAITTIME);
    if (true == chk_sem)
    {
        sprintf(s_fat_sample_file_path, "%d:\\renesas.txt", FAT_SAMPLE_PRV_CH);
        chk_fr = f_open(&g_fil, s_fat_sample_file_path, (FA_OPEN_APPEND | FA_WRITE));
        if (FR_OK == chk_fr)
        {
            memcpy(&g_sample_app_rw_buff[0], &p_write_buff[0], sizeof(p_write_buff));
            chk_fr = f_write(&g_fil
                            ,&g_sample_app_rw_buff[0]
                            ,sizeof(p_write_buff)
                            ,&bw);
            if (FR_OK == chk_fr)
            {
                fat_sample_ret = FAT_SAMPLE_OK;
            }

            /* Cast to an appropriate type */
            (void)f_close(&g_fil);
        }
        R_OS_SemaphoreRelease(&g_fat_sem_access);
    }
    return fat_sample_ret;
}
/*******************************************************************************
 End of function fat_sample_write_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_read_proc
* Description  : Fat sample read process
* Arguments    : none
* Return Value : FAT_SAMPLE_OK for success
******************************************************************************/
static e_fat_sample_ret_t fat_sample_read_proc(void)
{
    e_fat_sample_ret_t fat_sample_ret;
    bool_t             chk_sem;
    FRESULT            chk_fr;
    UINT               br;

    /* uint32_t           cnt; */

    fat_sample_ret = FAT_SAMPLE_NG;
    chk_sem        = R_OS_SemaphoreWait(&g_fat_sem_access, FAT_SAMPLE_PRV_SEM_WAITTIME);
    if (true == chk_sem)
    {
        sprintf(s_fat_sample_file_path, "%d:\\renesas.txt", FAT_SAMPLE_PRV_CH);
        chk_fr = f_open(&g_fil, s_fat_sample_file_path, FA_READ);
        if (FR_OK == chk_fr)
        {
            while (true)
            {
                chk_fr = f_read(&g_fil
                               ,&g_sample_app_rw_buff[0]
                               ,FAT_SAMPLE_PRV_RW_BUFF_SIZE
                               ,&br);
                if (FR_OK != chk_fr)            /* Read error */
                {
                    break;
                }
                else if (0 == br)           /* eof */
                {
                    fat_sample_ret = FAT_SAMPLE_OK;
                    break;
                }
                else
                {
                    /* DO NOTHING */
                    ;

                    /* for (cnt = 0; cnt < br; cnt++) */
                    /* { */
                    /*     printf("%02X ", g_sample_app_rw_buff[cnt]); */

                    /*     if (15 == (cnt % 16)) */
                    /*     { */
                    /*         printf("\r\n"); */
                    /*     } */
                    /* } */
                    /* printf("\r\n"); */
                }
            }

            /* Cast to an appropriate type */
            (void)f_close(&g_fil);
        }
        R_OS_SemaphoreRelease(&g_fat_sem_access);
    }
    return fat_sample_ret;
}
/*******************************************************************************
 End of function fat_sample_read_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: os_fat_sample_key_input_task_t
* Description  : Fat sample key input task
* Arguments    : void *params : no used
* Return Value : none
******************************************************************************/
static void os_fat_sample_key_input_task_t(void *params)
{
    e_r_drv_gpio_level_t key_input_row;
    e_r_drv_gpio_level_t key_input_old;
    uint32_t             key_input_cnt;
    e_fat_sample_event_t key_event;
    bool_t               chk_sem;
    bool_t               chk_queue;

    /* Cast to an appropriate type */
    UNUSED_PARAM(params);

    key_input_row = GPIO_LEVEL_SC_DEFAULT;
    key_input_old = GPIO_LEVEL_SC_DEFAULT;
    key_input_cnt = 0;

    while (true)
    {
        key_input_row = fat_sample_key_input();
        if (key_input_row == key_input_old)
        {
            if ((GPIO_LEVEL_LOW == key_input_row)
             && (key_input_cnt < FAT_SAMPLE_PRV_CATT_NUM_LONG))
            {
                key_input_cnt++;
                if (key_input_cnt >= FAT_SAMPLE_PRV_CATT_NUM_LONG)
                {
                    key_input_cnt = FAT_SAMPLE_PRV_CATT_NUM_LONG;
                    chk_sem = R_OS_SemaphoreWait(&s_fat_sem_key_input, FAT_SAMPLE_PRV_SEM_WAITTIME);
                    if (true == chk_sem)
                    {
                        key_event = FAT_SAMPLE_EVENT_KEY_INPUT; /* long press */
                        chk_queue = R_OS_MessageQueuePut(s_fat_sample_queue_handle, &key_event);
                        if (true != chk_queue)
                        {
//                            fat_sample_led_error();
                        }
                        R_OS_SemaphoreRelease(&s_fat_sem_key_input);
                    }
                }
            }
        }
        else
        {
            if ((key_input_cnt >= FAT_SAMPLE_PRV_CATT_NUM_SHORT)
             && (key_input_cnt <  FAT_SAMPLE_PRV_CATT_NUM_LONG ))
            {
                chk_sem = R_OS_SemaphoreWait(&s_fat_sem_key_input, FAT_SAMPLE_PRV_SEM_WAITTIME);
                if (true == chk_sem)
                {
                    key_event = FAT_SAMPLE_EVENT_KEY_INPUT; /* short press */
                    chk_queue = R_OS_MessageQueuePut(s_fat_sample_queue_handle, &key_event);
                    if (true != chk_queue)
                    {
//                        fat_sample_led_error();
                    }
                    R_OS_SemaphoreRelease(&s_fat_sem_key_input);
                }
            }
            key_input_cnt = 0;
            key_input_old = key_input_row;
        }
        R_OS_TaskSleep(FAT_SAMPLE_PRV_KEY_INPUT_CYCLE);
    }
}
/*******************************************************************************
 End of function os_fat_sample_key_input_task_t
 ******************************************************************************/
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/******************************************************************************
* Function Name: fat_sample_cd_int_cb_function
* Description  : Callback interrupt function for card detection
* Arguments    : int32_t sd_port : channel no (0 or 1)
*              : int32_t cd      : card detect information
* Return Value : success : SD_OK
*              : fail    : SD_ERR
******************************************************************************/
int32_t fat_sample_cd_int_cb_function(int32_t sd_port, int32_t cd)
{
#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
    bool_t  chk_queue;

    if (0 != cd)
    {
        s_fat_sample_isr_event[0] = FAT_SAMPLE_EVENT_CD_INSERT;
        if (FAT_SAMPLE_PRV_CH == sd_port)
        {
            chk_queue = R_OS_MessageQueuePut(s_fat_sample_queue_handle, &s_fat_sample_isr_event[0]);
            if (false == chk_queue)
            {
                /* DO NOTHING */
                ;
            }
        }
    }
    else
    {
        s_fat_sample_isr_event[1] = FAT_SAMPLE_EVENT_CD_REMOVE;
        if (FAT_SAMPLE_PRV_CH == sd_port)
        {
            chk_queue = R_OS_MessageQueuePut(s_fat_sample_queue_handle, &s_fat_sample_isr_event[1]);
            if (false == chk_queue)
            {
                /* DO NOTHING */
                ;
            }
        }
    }
#else /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */
    (void)sd_port;
    (void)cd;
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */
    return SD_OK;
}
/*******************************************************************************
 End of function fat_sample_cd_int_cb_function
 ******************************************************************************/

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
/******************************************************************************
* Function Name: fat_sample_dummy_proc
* Description  : Fat sample dummy process
* Arguments    : none
* Return Value : none
******************************************************************************/
static void fat_sample_dummy_proc(void)
{
//    fat_sample_led_error();
}
/*******************************************************************************
 End of function fat_sample_dummy_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_power_on_proc
* Description  : Fat sample power on process
* Arguments    : none
* Return Value : none
******************************************************************************/
static void fat_sample_power_on_proc(void)
{
    FRESULT chk_fr;

    /*--- initialize ---*/
    memset(&g_fatfs, 0, sizeof(FATFS));

//    fat_sample_led_off();

    /*--- mount ---*/
    sprintf(s_fat_sample_drive_path, "%d:", FAT_SAMPLE_PRV_DRV_LETTER);
    chk_fr = f_mount(&g_fatfs, s_fat_sample_drive_path, 1);
    if (FR_OK == chk_fr)
    {
        /* update fat sample mode */
        s_fat_sample_mode = FAT_SAMPLE_MODE_CD_INSERTED;
    }
    else
    {
        /* FR_NO_FILESYSTEM is not care */
        /* update fat sample mode */
        s_fat_sample_mode = FAT_SAMPLE_MODE_CD_REMOVED;
//        fat_sample_led_error();
    }
    chk_fr = f_chdrive(s_fat_sample_drive_path);
    if (FR_OK != chk_fr)
    {
    	/* Failed to change drive */
//        fat_sample_led_error();
    }
}
/*******************************************************************************
 End of function fat_sample_power_on_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_cd_insert_proc
* Description  : Fat sample card detection is inserted process
* Arguments    : none
* Return Value : none
******************************************************************************/
static void fat_sample_cd_insert_proc(void)
{
    FRESULT chk_fr;

    /*--- initialize ---*/
    memset(&g_fatfs, 0, sizeof(FATFS));

//    fat_sample_led_off();

    /*--- mount ---*/
    sprintf(s_fat_sample_drive_path, "%d:", FAT_SAMPLE_PRV_CH);
    chk_fr = f_mount(&g_fatfs, s_fat_sample_drive_path, 1);
    if (FR_OK != chk_fr)
    {
//        fat_sample_led_error();
    }

    /* update fat sample mode */
    s_fat_sample_mode = FAT_SAMPLE_MODE_CD_INSERTED;
}
/*******************************************************************************
 End of function fat_sample_cd_insert_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_cd_remove_proc
* Description  : Fat sample card detection is removed process
* Arguments    : none
* Return Value : none
******************************************************************************/
static void fat_sample_cd_remove_proc(void)
{
    FRESULT chk_fr;

    /*--- initialize ---*/
    memset(&g_fatfs, 0, sizeof(FATFS));

//    /* fat_sample_led_off(); */

    /*--- unmount ---*/
    sprintf(s_fat_sample_drive_path, "%d:", FAT_SAMPLE_PRV_CH);
    chk_fr = f_unmount(s_fat_sample_drive_path);
    if (FR_OK != chk_fr)
    {
//        fat_sample_led_error();
    }

    /* update fat sample mode */
    s_fat_sample_mode = FAT_SAMPLE_MODE_CD_REMOVED;
}
/*******************************************************************************
 End of function fat_sample_cd_remove_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: fat_sample_key_input_proc
* Description  : Fat sample key input process
* Arguments    : none
* Return Value : none
******************************************************************************/
static void fat_sample_key_input_proc(void)
{
    e_fat_sample_ret_t chk_proc;
    uint32_t           cnt;
    bool_t             chk_sem;

    /* bool_t             chk_queue; */

    chk_proc = FAT_SAMPLE_NG;
    chk_sem  = R_OS_SemaphoreWait(&s_fat_sem_key_input, FAT_SAMPLE_PRV_SEM_WAITTIME);
    if (true == chk_sem)
    {
        for (cnt = 0; cnt < FAT_SAMPLE_PRV_RW_PROC_NUM; cnt++)
        {
            /*--- 1. Writing a string to the file. ---*/
//            fat_sample_led_on();
            chk_proc = fat_sample_write_proc();
            if (FAT_SAMPLE_OK != chk_proc)
            {
                break;
            }

            /*--- 2. Reading a string from the file. ---*/
            chk_proc = fat_sample_read_proc();
            if (FAT_SAMPLE_OK != chk_proc)
            {
                break;
            }
            R_OS_TaskSleep(FAT_SAMPLE_PRV_300MS);

            /*--- 3. No access to the file. ---*/
//            fat_sample_led_off();
            R_OS_TaskSleep(FAT_SAMPLE_PRV_500MS);
        }

        if (FAT_SAMPLE_OK != chk_proc)
        {
//            fat_sample_led_error();

            /* clear a message queue */
            /* chk_queue = R_OS_MessageQueueClear(&s_fat_sample_queue_handle); */
            /* if (true == chk_queue) */
            /* { */
            /*     printf("queue clear ok\r\n"); */
            /* } */
            /* else */
            /* { */
            /*     printf("queue clear ng\r\n"); */
            /* } */
        }
        R_OS_SemaphoreRelease(&s_fat_sem_key_input);
    }
}
/*******************************************************************************
 End of function fat_sample_key_input_proc
 ******************************************************************************/

/******************************************************************************
* Function Name: os_fat_sample_main_task_t
* Description  : Fat sample main task
* Arguments    : void *params : no used
* Return Value : none
******************************************************************************/
static void os_fat_sample_main_task_t(void *params)
{
    bool_t               chk_queue;
    e_fat_sample_event_t *p_receive_event;

    /* Cast to an appropriate type */
    UNUSED_PARAM(params);

    while (true)
    {
        /* Cast to an appropriate type */
        chk_queue = R_OS_MessageQueueGet(s_fat_sample_queue_handle, (p_os_msg_t *)&p_receive_event,
                                        FAT_SAMPLE_PRV_QUEUE_GET_CYCLE, false);
        if (true == chk_queue)
        {
            if (((*p_receive_event)< FAT_SAMPLE_EVENT_NUM)
             && (s_fat_sample_mode < FAT_SAMPLE_MODE_NUM ))
            {
                /* Cast to an appropriate type */
                if (NULL != s_tbl_fat_sample_proc[*p_receive_event][s_fat_sample_mode])
                {
                    (*s_tbl_fat_sample_proc[*p_receive_event][s_fat_sample_mode])();
                }
            }
            else
            {
//                fat_sample_led_error();
            }
        }
    }
}
/*******************************************************************************
 End of function os_fat_sample_main_task_t
 ******************************************************************************/
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

/* extern void show_welcome_msg (FILE *p_out, bool_t clear_screen); */

/******************************************************************************
* Function Name: os_main_task_t
* Description  : FreeRTOS main task called by R_OS_KernelInit()
*              : FreeRTOS is now configured and R_OS_Abstraction calls
*              : can be used.
*              : From this point forward no longer use direct_xxx calls.
*              : For example
*              : in place of   direct_open("ostm2", O_RDWR);
*              : use           open(DEVICE_INDENTIFIER "ostm2", O_RDWR);
*              :
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t os_main_task_t(void)
{
#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
    int_t err;
    st_r_drv_gpio_pin_list_t pin_led;
    char_t data;
    e_fat_sample_event_t  event;
    bool_t                chk_queue;
    bool_t                chk_sem;

    /* For information only
     * Use stdio calls to open drivers once  the kernel is initialised
     *
     * ie
     * int_t ostm3_handle;
     * ostm3_handle = open (DEVICE_INDENTIFIER "ostm2", O_RDWR);
     * close (ostm3_handle);
     */

    gs_my_gpio_handle = open (DEVICE_INDENTIFIER "gpio", O_RDWR);

    /* On error */
    if ( gs_my_gpio_handle < 0 )
    {
        /* stop execute */
        while(1);
    }

    /**************************************************
     * Initialise P6_0 pin parameterised in GPIO_SC_TABLE_MANUAL
     **************************************************/
    pin_led.p_pin_list = gs_led_pin_list;
    pin_led.count = (sizeof(gs_led_pin_list)) / (sizeof(gs_led_pin_list[0]));
    err = direct_control(gs_my_gpio_handle, CTL_GPIO_INIT_BY_PIN_LIST, &pin_led);

    /* On error */
    if ( err < 0 )
    {
        /* stop execute */
        while(1);
    }
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

    /* ==== Output banner message ==== */
    printf("%s%s", sp_clear_screen, sp_cursor_home);

    /* show_welcome_msg(stdout, true); */
    fat_sample_welcome_msg(stdout, true);

#if defined(FAT_SAMPLE_PRV_DEMO_CONSOLE)
    /* Create a task to run the console */
    R_OS_TaskCreate("Console", os_console_task_t, NULL, R_OS_ABSTRACTION_DEFAULT_STACK_SIZE, TASK_CONSOLE_TASK_PRI);
#endif

#if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES)
    /* --- Create message queue start --- */
    chk_queue = R_OS_MessageQueueCreate(&s_fat_sample_queue_handle, FAT_SAMPLE_PRV_QUEUE_NUM);
    if (true == chk_queue)
    {
        event = FAT_SAMPLE_EVENT_POWER_ON;
        chk_queue = R_OS_MessageQueuePut(s_fat_sample_queue_handle, &event);
    }
    if (false == chk_queue)
    {
//        fat_sample_led_error();
        while (true)
        {
            /* Do Nothing */
            ;
        }
    }

    /* --- Create message queue end --- */

    /* --- Create semaphore start --- */
    if (0uL == g_fat_sem_access)
    {
        chk_sem = R_OS_SemaphoreCreate(&g_fat_sem_access, 1uL);
        if (false == chk_sem)
        {
//            fat_sample_led_error();
            while (true)
            {
                /* Spin here forever.. */
                ;
            }
        }
    }
    if (0uL == s_fat_sem_key_input)
    {
        chk_sem = R_OS_SemaphoreCreate(&s_fat_sem_key_input, 1uL);
        if (false == chk_sem)
        {
//            fat_sample_led_error();
            while (true)
            {
                /* Spin here forever.. */
                ;
            }
        }
    }

    /* --- Create semaphore end --- */

    /* --- Create a task to run the fat sample --- */
    /* Cast to an appropriate type */ 
    R_OS_TaskCreate("Fat sample key input", os_fat_sample_key_input_task_t, NULL,
                    R_OS_ABSTRACTION_DEFAULT_STACK_SIZE, TASK_FAT_SAMPLE_KEY_INPUT_TASK_PRI);

    /* Cast to an appropriate type */ 
    R_OS_TaskCreate("Fat sample main", os_fat_sample_main_task_t, NULL,
                    R_OS_ABSTRACTION_DEFAULT_STACK_SIZE, TASK_FAT_SAMPLE_MAIN_TASK_PRI);
#endif /* #if !defined(SELECT_SD_SDIO_SAMPLE_PROCESSING_ROUTINES) */

    sample_main();
    while(1)
    {
        /* Do Nothing */
        ;
    }
}
/******************************************************************************
 * End of function os_main_task_t
 ******************************************************************************/

/******************************************************************************
* Function Name: main
* Description  : C Entry point
*              : opens and configures cpg driver
*              : starts the freertos kernel
* Arguments    : none
* Return Value : 0
******************************************************************************/
int_t main(void)
{
    int_t cpg_handle;

    /* configure any drivers that are required before the Kernel initializes */

    /* Initialize the devlink layer */
    R_DEVLINK_Init();

    /* Initialize CPG */
    cpg_handle = direct_open("cpg", 0);
    if ( cpg_handle < 0 )
    {
        /* stop execute */
        while(1)
        {
            /* Do Nothing */
            ;
        }
    }

    /* Can close handle if no need to change clock after here */
    direct_close(cpg_handle);

    /* Start FreeRTOS */
    /* R_OS_InitKernel should never return */
    R_OS_KernelInit();

}
/******************************************************************************
 * End of function main
 ******************************************************************************/

/* End of File */
