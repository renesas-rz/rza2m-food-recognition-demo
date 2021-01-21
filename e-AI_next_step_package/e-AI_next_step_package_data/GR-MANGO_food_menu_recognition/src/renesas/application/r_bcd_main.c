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
* Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
**********************************************************************************************************************/

/**********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
**********************************************************************************************************************/
#include <stdio.h>
#include <fcntl.h>

#include "r_typedefs.h"
#include "iodefine.h"
#include "jcu_swap.h"
#include "lcd_panel.h"
#include "r_cache_lld_rza2m.h"
#include "r_jcu.h"
#include "r_memory_map.h"
#include "r_rvapi_vdc.h"
#include "rz_co.h"
#include "r_os_abstraction_api.h"
#include "VRAM.h"
#include "inference_exec.h"

#include "r_bcd_camera.h"
#include "r_bcd_lcd.h"
#include "r_bcd_ae.h"
#include "draw.h"
#include "perform.h"
#include "console.h"

#include "r_dk2_if.h"
#include  "r_drp_resize_bilinear_fixed_rgb.h"
#include  "r_drp_simple_isp_colcal_3dnr.h"

#include "version.h"

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/
#undef  DEMO_MODE
#ifdef  DEMO_MODE
#include "JCU_ExampleImage.h"
#endif  /* DEMO_MODE */

#define TILE_0                  (0)
#define TILE_1                  (1)
#define TILE_2                  (2)
#define TILE_3                  (3)
#define TILE_4                  (4)
#define TILE_5                  (5)

#define DRP_NOT_FINISH          (0)
#define DRP_FINISH              (1)

#define DRP_DRV_ASSERT(x) if ((x) != 0) goto sample_finish

#define GS_FRAME_BYTE_PER_PIXEL         (4u)
#define GS_FRAME_BYTE_PER_PIXEL_RGB     (3u)
#define GS_FRAME_WIDTH                  (128u)
#define GS_FRAME_HEIGHT                 (128u)
#define GS_ALPHA_VAL_MAX                (0xFF)

#define GS_N  "\r\n"

#define MEM_SIZE_RGB_OUT        (256 * 256 * 3)

/* Key status */
#define KEY_STAUS_INIT          (0x00000000U)
#define KEY_CHECK_BIT           (0x00000003U)
#define KEY_JUST_ON             (0x00000002U)

/***********************************************************************
* conv_ycbcr_to_bgr() parameter
*   STRIDE   : Input image stride
*   OFFSET_X : Offset position (width) of the input image.
*   OFFSET_Y : Offset position (height) of the input image.
************************************************************************/
#define STRIDE                  (512)       /* 256pixel * 2bpp */
#define OFFSET_X                (0)
#define OFFSET_Y                (0)

/***********************************************************************
* Define: GS_OUTPUT_PIXEL_FORMAT
*    JCU_OUTPUT_RGB565(=2), JCU_OUTPUT_ARGB8888(=1), JCU_OUTPUT_YCbCr422(=0)
************************************************************************/
#define GS_OUTPUT_PIXEL_FORMAT  (JCU_OUTPUT_ARGB8888)

/***********************************************************************
* SimpleISP Parameter
************************************************************************/
#define GAIN_RED_WB             (0x2480U)
#define GAIN_GREEN_WB           (0x1800U)
#define GAIN_BLUE_WB            (0x275bU)
#define BIAS_RED_WB             (0x00F0U)
#define BIAS_GREEN_WB           (0x00F0U)
#define BIAS_BLUE_WB            (0x00F0U)

/* color parameter */
#define COLOR_MATRIX_C11        (0x25ddU)
#define COLOR_MATRIX_C12        (0xfef0U)
#define COLOR_MATRIX_C13        (0xf79aU)
#define COLOR_MATRIX_C21        (0xfd6aU)
#define COLOR_MATRIX_C22        (0x1ef4U)
#define COLOR_MATRIX_C23        (0xfdd5U)
#define COLOR_MATRIX_C31        (0x029eU)
#define COLOR_MATRIX_C32        (0xecf0U)
#define COLOR_MATRIX_C33        (0x2c0dU)

#define ISP_3DNR_Y_COEF         (64u)
#define ISP_3DNR_C_COEF         (32u)
#define ISP_3DNR_Y_ALPHA_MAX    (128u)
#define ISP_3DNR_Y_THRESH_A     (8u)
#define ISP_3DNR_Y_THRESH_B     (16u)
#define ISP_3DNR_Y_TILT         (512u)
#define ISP_3DNR_C_ALPHA_MAX    (128u)
#define ISP_3DNR_C_THRESH_A     (8u)
#define ISP_3DNR_C_THRESH_B     (16u)
#define ISP_3DNR_C_TILT         (512u)

/* default camera gain */
#define CAMERA_GAIN_8DB         (16u)

/* Flip the image horizontally */
#define HORIZONTAL_FLIP         /* define:on */

/**********************************************************************************************************************
Imported global variables and functions (from other files)
**********************************************************************************************************************/
extern uint8_t g_drp_resize_bilinear_fixed_rgb[];
extern uint8_t g_drp_simple_isp_colcal_3dnr_6[];

/**********************************************************************************************************************
Exported global variables
**********************************************************************************************************************/
/* demo mode */
#ifdef  DEMO_MODE
bool       demo_mode = true;               /* false:CAMERA Input On, true:CAMERA Input Off(Jpeg Image) */
#else
bool       demo_mode = false;               /* false:CAMERA Input On, true:CAMERA Input Off(Jpeg Image) */
#endif /* DEMO_MODE */
uint_t     cpu_drp_sel = 1;                 /* 0: Inference by CPU, 1: Inference by DRP */

/* Jpeg image number when demo_mode is a Jpeg image */
int_t      n_food = 0;

/* String of correct answer category after inference when demo_mode is Jpeg image */
char       cat_ans[20];

/* Inference memory */
uint8_t*   memory_address_of_RAW;
size_t     memory_size_of_RAW;
uintptr_t  physical_address_of_RAW;
inference_result_t inference_result;

/**********************************************************************************************************************
Private global variables and functions
**********************************************************************************************************************/
/* Memory for DRP */
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static volatile uint8_t drp_lib_status[R_DK2_TILE_NUM] = {DRP_NOT_FINISH};
static r_drp_resize_bilinear_fixed_rgb_t resize_image_param __attribute__ ((section("UNCACHED_BSS")));
static r_drp_simple_isp_colcal_3dnr_t param __attribute__ ((section("UNCACHED_BSS")));

/* Memory for Simple ISP */
static uint8_t  look_up_table[256] __attribute__ ((section("UNCACHED_BSS")));
static uint32_t ave_result[9] __attribute__ ((section("UNCACHED_BSS")));
static r_bcd_ae_setting_t ae_setting;

/* Terminal window escape sequences */
static const char_t * const sp_clear_screen = "\x1b[2J";
static const char_t * const sp_cursor_home  = "\x1b[H";
#ifdef  DEMO_MODE
static uint8_t buf[80] __attribute__ ((section("Graphics_OCTA_RAM")));
#else
static uint8_t buf[80];
#endif /* DEMO_MODE */
/* Image buffer */
#ifdef  DEMO_MODE
static uint8_t image_buff[256*256*2]  __attribute__ ((section("Video_OCTA_RAM")));
static uint8_t image_rgb_out[256*256*3] __attribute__ ((section("Video_OCTA_RAM")));
#else
static uint8_t image_buff[256*256*2];
static uint8_t image_rgb_out[256*256*3];
#endif /* DEMO_MODE */
static uint8_t *image_argb_out;

/* key status */
static uint32_t sw2_status;
static uint32_t sw3_status;

static void cb_drp_finish(uint8_t id);
static void set_disp_data(void);
static errnum_t R_JCU_SampleDecode(void);
static void conv_ycbcr_to_bgr(uint8_t *src, uint8_t *dst);

/**********************************************************************************************************************
* Function Name: cb_drp_finish
* Description  : This function is a callback function called from the
*              : DRP driver at the finish of the DRP library processing.
* Arguments    : id
*              :   The ID of the DRP library that finished processing.
* Return Value : -
**********************************************************************************************************************/
static void cb_drp_finish(uint8_t id)
{
    uint32_t tile_no;

    /* Change the operation state of the DRP library notified by the argument to finish */
    for (tile_no = 0; tile_no < R_DK2_TILE_NUM; tile_no++)
    {
        if (drp_lib_id[tile_no] == id)
        {
            drp_lib_status[tile_no] = DRP_FINISH;
            break;
        }
    }

    return;
}
/**********************************************************************************************************************
* End of function cb_drp_finish
**********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sample_main
* Description  : First function called after initialization is completed
* Arguments    : -
* Return Value : -
**********************************************************************************************************************/
void sample_main(void)
{
    int32_t frame_buf_id;
    int32_t ret_val;
    uint8_t * p_input_bufadr;
    uint8_t * p_output_bufadr;

    errnum_t e;
    uint32_t k = 0;
    uint32_t i = 0;

    st_os_abstraction_info_t ver_info;

    int16_t sd_stat = (-1);
    uint16_t sd_buf[15];
    const char_t  p_file_name[] = "ispset.txt";

    /* ==== Output banner message ==== */
    printf("%s%s", sp_clear_screen, sp_cursor_home);
    printf("RZ/A2M sample for GCC Ver. %u.%u\r\n", APPLICATION_INFO_VERSION, APPLICATION_INFO_RELEASE);
    printf("Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.\r\n");
    printf("Build Info Date %s at %s \r\n", __DATE__, __TIME__);

    if(R_OS_GetVersion(&ver_info) == 0)
    {
        printf("%s Version %d.%d\r\n", ver_info.p_szdriver_name,
                ver_info.version.sub.major, ver_info.version.sub.minor);
    }

    /* ==== Start applications ==== */
    /* Initialization of VIN and MIPI driver */
    R_BCD_CameraInit();

    /* Initialization of LCD driver */
    R_BCD_LcdInit();
    
    /* Initialization of key read routines */
    /* Set SW2/SW3 readable */
    PORTD.PMR.BIT.PMR6 = 0;
    PORTD.PDR.BIT.PDR6 = 2;
    PORTD.PMR.BIT.PMR7 = 0;
    PORTD.PDR.BIT.PDR7 = 2;
    sw2_status = KEY_STAUS_INIT;
    sw3_status = KEY_STAUS_INIT;

    /* Capture Start */
    R_BCD_CameraCaptureStart();

    /* Initialize of Performance counter */
    PerformInit();

    /* Initialization of DRP driver */
    R_DK2_Initialize();

    R_BCD_AeInit(&ae_setting);

    /* Gamma Table */
    R_BCD_AeMakeGammaTable(&ae_setting, (double)1.2, look_up_table);
    R_BCD_SetCameraGain(CAMERA_GAIN_8DB);

    /* Inference processing initialization */
    inference_exec_init();

    /*************/
    /* Main loop */
    /*************/
    while (1)
    {
        /* Get SW2 key */
#ifdef  DEMO_MODE
        sw2_status = (sw2_status << 1) | !PORTD.PIDR.BIT.PIDR6;
        if ( ( sw2_status & KEY_CHECK_BIT) == KEY_JUST_ON )
        {
            /* SW2 JUST ON */
            /* Change demo mode */
            demo_mode = !demo_mode;
            
            R_BCD_LcdClearGraphicsBuffer();
        }
#endif  /* DEMO_MODE */

        /* Get SW3 key */
        sw3_status = (sw3_status << 1) | !PORTD.PIDR.BIT.PIDR7;
        if ( ( sw3_status & KEY_CHECK_BIT) == KEY_JUST_ON )
        {
            /* SW3 JUST ON */
            /* Change CPU/DRP select */
            if (demo_mode == false)
            {
                cpu_drp_sel = !cpu_drp_sel;
            }
        }
        
        if (demo_mode == false)             /* CAMERA Input mode */
        {
            /******************************/
            /* Load DRP Library           */
            /*        +-----------------+ */
            /* tile 0 |IspLibrary       | */
            /*        |                 | */
            /* tile 1 |                 | */
            /*        |                 | */
            /* tile 2 |                 | */
            /*        |                 | */
            /* tile 3 |                 | */
            /*        |                 | */
            /* tile 4 |                 | */
            /*        |                 | */
            /* tile 5 |                 | */
            /*        +-----------------+ */
            /******************************/
            ret_val = R_DK2_Load(&g_drp_simple_isp_colcal_3dnr_6[0], R_DK2_TILE_0, R_DK2_TILE_PATTERN_6, NULL,
                                &cb_drp_finish, &drp_lib_id[0]);
            DRP_DRV_ASSERT(ret_val);

            /************************/
            /* Activate DRP Library */
            /************************/
            ret_val = R_DK2_Activate(drp_lib_id[TILE_0], 0);
            DRP_DRV_ASSERT(ret_val);
            
            /***************************************/
            /* Set R_DK2_Start function parameters */
            /***************************************/
            /* Set the address of buffer to be read/write by DRP */
            R_MMU_VAtoPA((uint32_t)ave_result, &(param.accumulate));                /* Convert pointer to uint32_t */
            R_MMU_VAtoPA((uint32_t)look_up_table, &(param.table));                  /* Convert pointer to uint32_t */

            /* Image size */
            param.width  = R_BCD_CAMERA_WIDTH;
            param.height = R_BCD_CAMERA_HEIGHT;

            /* Accumlate ( Not use ) */
            param.area1_offset_x = 0;
            param.area1_offset_y = 0;
            param.area1_width    = R_BCD_CAMERA_WIDTH;
            param.area1_height   = R_BCD_CAMERA_HEIGHT;
            param.area2_offset_x = 0;
            param.area2_offset_y = 0;
            param.area2_width    = 0;
            param.area2_height   = 0;
            param.area3_offset_x = 0;
            param.area3_offset_y = 0;
            param.area3_width    = 0;
            param.area3_height   = 0;    
            param.component      = 1;   /* Accumlate ON */

            /* sd card read */
#ifdef  DEMO_MODE
            if (sd_stat != CMD_OK)
            {
                memset(&sd_buf[0], 0, sizeof(sd_buf));
                sd_stat = FAT_Read(&p_file_name[0], &sd_buf[0]);
                
                if (sd_stat == CMD_OK)
                {
                    /* SD card read completed */
                    R_BCD_SetCameraGain((uint8_t)sd_buf[0]);
                }
            }
            if (sd_stat == CMD_OK)
            {
                /* SD card read completed */
                /* Gain and Bias  */
                param.bias_r     = BIAS_RED_WB;
                param.bias_g     = BIAS_GREEN_WB;
                param.bias_b     = BIAS_BLUE_WB;
                param.gain_r     = sd_buf[1];
                param.gain_g     = sd_buf[2];
                param.gain_b     = sd_buf[3];

                /* Color matrix */
                param.matrix_c11 = sd_buf[4];
                param.matrix_c12 = sd_buf[5];
                param.matrix_c13 = sd_buf[6];
                param.matrix_c21 = sd_buf[7];
                param.matrix_c22 = sd_buf[8];
                param.matrix_c23 = sd_buf[9];
                param.matrix_c31 = sd_buf[10];
                param.matrix_c32 = sd_buf[11];
                param.matrix_c33 = sd_buf[12];
            }
            else
#endif  /* DEMO_MODE */
            {
                /* SD card read incomplete */
                /* Gain and Bias  */
                param.bias_r     = BIAS_RED_WB;
                param.bias_g     = BIAS_GREEN_WB;
                param.bias_b     = BIAS_BLUE_WB;
                param.gain_r     = GAIN_RED_WB;
                param.gain_g     = GAIN_GREEN_WB;
                param.gain_b     = GAIN_BLUE_WB;

                /* Color matrix */
                param.matrix_c11 = (int16_t)COLOR_MATRIX_C11; /* actual type is int16_t */
                param.matrix_c12 = (int16_t)COLOR_MATRIX_C12; /* actual type is int16_t */
                param.matrix_c13 = (int16_t)COLOR_MATRIX_C13; /* actual type is int16_t */
                param.matrix_c21 = (int16_t)COLOR_MATRIX_C21; /* actual type is int16_t */
                param.matrix_c22 = (int16_t)COLOR_MATRIX_C22; /* actual type is int16_t */
                param.matrix_c23 = (int16_t)COLOR_MATRIX_C23; /* actual type is int16_t */
                param.matrix_c31 = (int16_t)COLOR_MATRIX_C31; /* actual type is int16_t */
                param.matrix_c32 = (int16_t)COLOR_MATRIX_C32; /* actual type is int16_t */
                param.matrix_c33 = (int16_t)COLOR_MATRIX_C33; /* actual type is int16_t */
            }
            
            /* 3DNR OFF */
            param.y_coef      = ISP_3DNR_Y_COEF;
            param.c_coef      = ISP_3DNR_C_COEF;
            param.y_alpha_max = ISP_3DNR_Y_ALPHA_MAX;
            param.y_thresh_a  = ISP_3DNR_Y_THRESH_A;
            param.y_thresh_b  = ISP_3DNR_Y_THRESH_B;
            param.y_tilt      = ISP_3DNR_Y_TILT;
            param.c_alpha_max = ISP_3DNR_C_ALPHA_MAX;
            param.c_thresh_a  = ISP_3DNR_C_THRESH_A;
            param.c_thresh_b  = ISP_3DNR_C_THRESH_B;
            param.c_tilt      = ISP_3DNR_C_TILT;

            /* Median ON */
            param.blend  = 0x100;       /* set median alpha blend 100% */

            /* Gamma OFF */
            param.gamma = 0x00;         /* Gamma OFF */

            /* Unsharp Mask OFF */
            param.strength = 0;
            param.coring = 0;

            /* Wait until camera capture is complete */
            while ((frame_buf_id = R_BCD_CameraGetCaptureStatus()) == R_BCD_CAMERA_NOT_CAPTURED)
            {
                /* DO NOTHING */
            }

            /* Set the address of buffer to be read/write by DRP */
            p_input_bufadr  = R_BCD_CameraGetFrameAddress(frame_buf_id);

            /* convert to phisical address */
            R_MMU_VAtoPA((uint32_t)p_input_bufadr, &(param.src));       /* Convert pointer to uint32_t */
            R_MMU_VAtoPA((uint32_t)&image_buff[0], &(param.dst));       /* Convert pointer to uint32_t */
			R_CACHE_L1DataCleanInvalidLine(&image_buff[0], R_BCD_CAMERA_WIDTH * R_BCD_CAMERA_HEIGHT * 2 );

            param.prev = 0;             /* 3DNR OFF */

            /* Set start time of process*/
            PerformSetStartTime(2);

            /* Initialize variables to be used in termination judgment of the DRP library */
            drp_lib_status[TILE_0] = DRP_NOT_FINISH;

            /*********************/
            /* Start DRP Library */
            /*********************/
            ret_val = R_DK2_Start(drp_lib_id[TILE_0], (void *)&param, sizeof(r_drp_simple_isp_colcal_3dnr_t));
            DRP_DRV_ASSERT(ret_val);

            /***************************************/
            /* Wait until DRP processing is finish */
            /***************************************/
            while (drp_lib_status[TILE_0] == DRP_NOT_FINISH)
            {
                ;
            }

            /* Set end time of process */
            PerformSetEndTime(2);

            /****************************************************************/
            /* Convert YCbCr to BGR                                         */
            /****************************************************************/
            e= R_SAMPLE_GetVRAM( &memory_address_of_RAW,  &memory_size_of_RAW );
            if(e)
            {
                goto sample_finish;
            }
            memory_size_of_RAW = GS_FRAME_WIDTH * GS_FRAME_BYTE_PER_PIXEL_RGB * GS_FRAME_HEIGHT;

            conv_ycbcr_to_bgr(&image_buff[0], memory_address_of_RAW);

            /* AE */
            /****************************************************************/
            /* Auto Exposure Correction                                     */
            /* ilumi = (R * 0.299 * 4) + (G * 0.587 * 2) + (B * 0.114  * 4) */
            /****************************************************************/
            /* Cast to an appropriate type */
            R_BCD_AeRunAutoExpousure(&ae_setting, 
                (uint16_t)( ( (ave_result[0] * 0.299 * 4) + (ave_result[1] * 0.587 * 2) + (ave_result[2] * 0.114 * 4) )
                / (R_BCD_CAMERA_WIDTH * R_BCD_CAMERA_HEIGHT) ) );

            /**********************/
            /* Unload DRP Library */
            /**********************/
            ret_val = R_DK2_Unload(drp_lib_id[TILE_0], &drp_lib_id[0]);
            DRP_DRV_ASSERT(ret_val);

            /* Clear the current capture state and enable the detection of the next capture completion */
            R_BCD_CameraClearCaptureStatus();

            /**************************************/
            /* Load DRP Library                   */
            /*        +-------------------------+ */
            /* tile 0 |ResizeBilinearFixedRgb   | */
            /*        |                         | */
            /* tile 1 |                         | */
            /*        |                         | */
            /* tile 2 |                         | */
            /*        |                         | */
            /* tile 3 |                         | */
            /*        |                         | */
            /* tile 4 |                         | */
            /*        |                         | */
            /* tile 5 |                         | */
            /*        +-------------------------+ */
            /**************************************/
			R_MMU_VAtoPA((uint32_t)&memory_address_of_RAW[0], &(resize_image_param.src));
			R_MMU_VAtoPA((uint32_t)&image_rgb_out[0], &(resize_image_param.dst));
			R_CACHE_L1DataCleanInvalidAll();
            resize_image_param.src_width  = 128;
            resize_image_param.src_height = 128;
            resize_image_param.fx         = 0x08;
            resize_image_param.fy         = 0x08;

            ret_val = R_DK2_Load(&g_drp_resize_bilinear_fixed_rgb[0], R_DK2_TILE_0, R_DK2_TILE_PATTERN_6, NULL,
                                &cb_drp_finish, &drp_lib_id[0]);
            DRP_DRV_ASSERT(ret_val);
            
            /************************/
            /* Activate DRP Library */
            /************************/
            ret_val = R_DK2_Activate(drp_lib_id[TILE_0], 0);
            DRP_DRV_ASSERT(ret_val);

            /* Initialize variables to be used in termination judgment of the DRP library */
            drp_lib_status[TILE_0] = DRP_NOT_FINISH;

            /*********************/
            /* Start DRP Library */
            /*********************/
            ret_val = R_DK2_Start(drp_lib_id[TILE_0], (void *)&resize_image_param, sizeof(r_drp_resize_bilinear_fixed_rgb_t));
            DRP_DRV_ASSERT(ret_val);

            /***************************************/
            /* Wait until DRP processing is finish */
            /***************************************/
            while (drp_lib_status[TILE_0] == DRP_NOT_FINISH)
            {
                ;
            }

            /**********************/
            /* Unload DRP Library */
            /**********************/
            ret_val = R_DK2_Unload(drp_lib_id[TILE_0], &drp_lib_id[TILE_0]);
            DRP_DRV_ASSERT(ret_val);

            p_output_bufadr = R_BCD_LcdGetVramAddress();
            /* convert to phisical address */
            R_MMU_VAtoPA((uint32_t)p_output_bufadr, &(image_argb_out));

            k = 0;
            for (i = 0; i < MEM_SIZE_RGB_OUT; i=i+3)
            {
                 image_argb_out[k+0] = image_rgb_out[i+0];
                 image_argb_out[k+1] = image_rgb_out[i+1];
                 image_argb_out[k+2] = image_rgb_out[i+2];
                 image_argb_out[k+3] = 0xff;
                 k=k+4;
            }
            /* Write the data(buf) on the cache to physical memory */
            R_CACHE_L1DataCleanLine(R_BCD_LcdGetVramAddress(), (R_BCD_CAMERA_WIDTH * R_BCD_CAMERA_HEIGHT * 4));

            /* Display image processing result */
            R_BCD_LcdSwapVideoBuffer();
            
            /* Execute inference processing */
            inference_exec();
            
            /* Set LCD display */
            set_disp_data();
        }
#ifdef  DEMO_MODE
        else
        {
            /* JPEG image demo mode */
            for( n_food = 0 ; n_food < 64 ; n_food++ )
            {
                /* Clear overlap buffer */
                R_BCD_LcdClearGraphicsBuffer();
                
                /* Decodes JPEG data to a memory */
                R_JCU_SampleDecode();
                
                /* Display image processing result */
                R_BCD_LcdSwapVideoBuffer();
                
                /* Inference processing and result display */
                inference_exec();
            }
        }
#endif  /* DEMO_MODE */
    }
    
sample_finish:

    return;
}
/**********************************************************************************************************************
* End of function sample_main
**********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: R_JCU_SampleDecode
* Description  : Decodes JPEG data to a memory.
* Arguments    : -
* Return Value : Error Code. 0=No Error.
**********************************************************************************************************************/
#ifdef  DEMO_MODE
static errnum_t R_JCU_SampleDecode(void)
{
    uint8_t*            address_of_JPEG_image;
    size_t              size_of_JPEG_image;
    int32_t             ret_val;
    uint8_t *           p_output_bufadr;
    errnum_t            e;
    bool_t              s;
    jcu_decode_param_t  decode;
    jcu_buffer_param_t  buffer;
    bool_t              is_JCU_initialized = false;
    uint8_t*            memory_address_of_JPEG;
    size_t              memory_size_of_JPEG;
    uintptr_t           physical_address_of_JPEG;
    jcu_image_info_t    image;
    uint32_t            decoded_event = 0;

    switch(n_food & 0x3f)
    {
        case  0: address_of_JPEG_image = g_00_ramen_x_mq_img;
                 size_of_JPEG_image = sizeof(g_00_ramen_x_mq_img);
                 strcpy(cat_ans,"ramen");
                 break;
        case  1: address_of_JPEG_image = g_01_caesar_salad_x_mq_img;
                 size_of_JPEG_image = sizeof(g_01_caesar_salad_x_mq_img);
                 strcpy(cat_ans,"caesar_salad");
                 break;
        case  2: address_of_JPEG_image = g_02_ramen_x_mq_img;
                 size_of_JPEG_image = sizeof(g_02_ramen_x_mq_img);
                 strcpy(cat_ans,"ramen");
                 break;
        case  3: address_of_JPEG_image = g_03_oysters_x_mq_img;
                 size_of_JPEG_image = sizeof(g_03_oysters_x_mq_img);
                 strcpy(cat_ans,"oysters");
                 break;
        case  4: address_of_JPEG_image = g_04_steak_x_mq_img;
                 size_of_JPEG_image = sizeof(g_04_steak_x_mq_img);
                 strcpy(cat_ans,"steak");
                 break;
        case  5: address_of_JPEG_image = g_05_hamburger_x_mq_img;
                 size_of_JPEG_image = sizeof(g_05_hamburger_x_mq_img);
                 strcpy(cat_ans,"hamburger");
                 break;
        case  6: address_of_JPEG_image = g_06_caesar_salad_x_mq_img;
                 size_of_JPEG_image = sizeof(g_06_caesar_salad_x_mq_img);
                 strcpy(cat_ans,"caesar_salad");
                 break;
        case  7: address_of_JPEG_image = g_07_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_07_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        case  8: address_of_JPEG_image = g_08_spaghetti_carbonara_x_mq_img;
                 size_of_JPEG_image = sizeof(g_08_spaghetti_carbonara_x_mq_img);
                 strcpy(cat_ans,"spaghetti_carbonara");
                 break;
        case  9: address_of_JPEG_image = g_09_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_09_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 10: address_of_JPEG_image = g_10_edamame_x_mq_img;
                 size_of_JPEG_image = sizeof(g_10_edamame_x_mq_img);
                 strcpy(cat_ans,"edamame");
                 break;
        case 11: address_of_JPEG_image = g_11_donuts_x_mq_img;
                 size_of_JPEG_image = sizeof(g_11_donuts_x_mq_img);
                 strcpy(cat_ans,"donuts");
                 break;
        case 12: address_of_JPEG_image = g_12_club_sandwich_x_mq_img;
                 size_of_JPEG_image = sizeof(g_12_club_sandwich_x_mq_img);
                 strcpy(cat_ans,"club_sandwich");
                 break;
        case 13: address_of_JPEG_image = g_13_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_13_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 14: address_of_JPEG_image = g_14_oysters_x_mq_img;
                 size_of_JPEG_image = sizeof(g_14_oysters_x_mq_img);
                 strcpy(cat_ans,"oysters");
                 break;
        case 15: address_of_JPEG_image = g_15_hamburger_x_mq_img;
                 size_of_JPEG_image = sizeof(g_15_hamburger_x_mq_img);
                 strcpy(cat_ans,"hamburger");
                 break;
        case 16: address_of_JPEG_image = g_16_donuts_x_mq_img;
                 size_of_JPEG_image = sizeof(g_16_donuts_x_mq_img);
                 strcpy(cat_ans,"donuts");
                 break;
        case 17: address_of_JPEG_image = g_17_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_17_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        case 18: address_of_JPEG_image = g_18_french_fries_x_mq_img;
                 size_of_JPEG_image = sizeof(g_18_french_fries_x_mq_img);
                 strcpy(cat_ans,"french_fries");
                 break;
        case 19: address_of_JPEG_image = g_19_donuts_x_mq_img;
                 size_of_JPEG_image = sizeof(g_19_donuts_x_mq_img);
                 strcpy(cat_ans,"donuts");
                 break;
        case 20: address_of_JPEG_image = g_20_hamburger_x_mq_img;
                 size_of_JPEG_image = sizeof(g_20_hamburger_x_mq_img);
                 strcpy(cat_ans,"hamburger");
                 break;
        case 21: address_of_JPEG_image = g_21_spaghetti_carbonara_x_mq_img;
                 size_of_JPEG_image = sizeof(g_21_spaghetti_carbonara_x_mq_img);
                 strcpy(cat_ans,"spaghetti_carbonara");
                 break;
        case 22: address_of_JPEG_image = g_22_hot_dog_x_mq_img;
                 size_of_JPEG_image = sizeof(g_22_hot_dog_x_mq_img);
                 strcpy(cat_ans,"hot_dog");
                 break;
        case 23: address_of_JPEG_image = g_23_steak_x_mq_img;
                 size_of_JPEG_image = sizeof(g_23_steak_x_mq_img);
                 strcpy(cat_ans,"steak");
                 break;
        case 24: address_of_JPEG_image = g_24_steak_x_mq_img;
                 size_of_JPEG_image = sizeof(g_24_steak_x_mq_img);
                 strcpy(cat_ans,"steak");
                 break;
        case 25: address_of_JPEG_image = g_25_edamame_x_mq_img;
                 size_of_JPEG_image = sizeof(g_25_edamame_x_mq_img);
                 strcpy(cat_ans,"edamame");
                 break;
        case 26: address_of_JPEG_image = g_26_fried_rice_x_mq_img;
                 size_of_JPEG_image = sizeof(g_26_fried_rice_x_mq_img);
                 strcpy(cat_ans,"fried_rice");
                 break;
        case 27: address_of_JPEG_image = g_27_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_27_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        case 28: address_of_JPEG_image = g_28_steak_x_mq_img;
                 size_of_JPEG_image = sizeof(g_28_steak_x_mq_img);
                 strcpy(cat_ans,"steak");
                 break;
        case 29: address_of_JPEG_image = g_29_donuts_x_mq_img;
                 size_of_JPEG_image = sizeof(g_29_donuts_x_mq_img);
                 strcpy(cat_ans,"donuts");
                 break;
        case 30: address_of_JPEG_image = g_30_edamame_x_mq_img;
                 size_of_JPEG_image = sizeof(g_30_edamame_x_mq_img);
                 strcpy(cat_ans,"edamame");
                 break;
        case 31: address_of_JPEG_image = g_31_fried_rice_x_mq_img;
                 size_of_JPEG_image = sizeof(g_31_fried_rice_x_mq_img);
                 strcpy(cat_ans,"fried_rice");
                 break;
        case 32: address_of_JPEG_image = g_32_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_32_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 33: address_of_JPEG_image = g_33_pizza_x_mq_img;
                 size_of_JPEG_image = sizeof(g_33_pizza_x_mq_img);
                 strcpy(cat_ans,"pizza");
                 break;
        case 34: address_of_JPEG_image = g_34_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_34_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        case 35: address_of_JPEG_image = g_35_spaghetti_carbonara_x_mq_img;
                 size_of_JPEG_image = sizeof(g_35_spaghetti_carbonara_x_mq_img);
                 strcpy(cat_ans,"spaghetti_carbonara");
                 break;
        case 36: address_of_JPEG_image = g_36_fried_rice_x_mq_img;
                 size_of_JPEG_image = sizeof(g_36_fried_rice_x_mq_img);
                 strcpy(cat_ans,"fried_rice");
                 break;
        case 37: address_of_JPEG_image = g_37_caesar_salad_x_mq_img;
                 size_of_JPEG_image = sizeof(g_37_caesar_salad_x_mq_img);
                 strcpy(cat_ans,"caesar_salad");
                 break;
        case 38: address_of_JPEG_image = g_38_hot_dog_x_mq_img;
                 size_of_JPEG_image = sizeof(g_38_hot_dog_x_mq_img);
                 strcpy(cat_ans,"hot_dog");
                 break;
        case 39: address_of_JPEG_image = g_39_hot_dog_x_mq_img;
                 size_of_JPEG_image = sizeof(g_39_hot_dog_x_mq_img);
                 strcpy(cat_ans,"hot_dog");
                 break;
        case 40: address_of_JPEG_image = g_40_steak_x_mq_img;
                 size_of_JPEG_image = sizeof(g_40_steak_x_mq_img);
                 strcpy(cat_ans,"steak");
                 break;
        case 41: address_of_JPEG_image = g_41_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_41_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 42: address_of_JPEG_image = g_42_oysters_x_mq_img;
                 size_of_JPEG_image = sizeof(g_42_oysters_x_mq_img);
                 strcpy(cat_ans,"oysters");
                 break;
        case 43: address_of_JPEG_image = g_43_club_sandwich_x_mq_img;
                 size_of_JPEG_image = sizeof(g_43_club_sandwich_x_mq_img);
                 strcpy(cat_ans,"club_sandwich");
                 break;
        case 44: address_of_JPEG_image = g_44_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_44_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        case 45: address_of_JPEG_image = g_45_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_45_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 46: address_of_JPEG_image = g_46_club_sandwich_x_mq_img;
                 size_of_JPEG_image = sizeof(g_46_club_sandwich_x_mq_img);
                 strcpy(cat_ans,"club_sandwich");
                 break;
        case 47: address_of_JPEG_image = g_47_edamame_x_mq_img;
                 size_of_JPEG_image = sizeof(g_47_edamame_x_mq_img);
                 strcpy(cat_ans,"edamame");
                 break;
        case 48: address_of_JPEG_image = g_48_pizza_x_mq_img;
                 size_of_JPEG_image = sizeof(g_48_pizza_x_mq_img);
                 strcpy(cat_ans,"pizza");
                 break;
        case 49: address_of_JPEG_image = g_49_hamburger_x_mq_img;
                 size_of_JPEG_image = sizeof(g_49_hamburger_x_mq_img);
                 strcpy(cat_ans,"hamburger");
                 break;
        case 50: address_of_JPEG_image = g_50_fried_rice_x_mq_img;
                 size_of_JPEG_image = sizeof(g_50_fried_rice_x_mq_img);
                 strcpy(cat_ans,"fried_rice");
                 break;
        case 51: address_of_JPEG_image = g_51_oysters_x_mq_img;
                 size_of_JPEG_image = sizeof(g_51_oysters_x_mq_img);
                 strcpy(cat_ans,"oysters");
                 break;
        case 52: address_of_JPEG_image = g_52_pizza_x_mq_img;
                 size_of_JPEG_image = sizeof(g_52_pizza_x_mq_img);
                 strcpy(cat_ans,"pizza");
                 break;
        case 53: address_of_JPEG_image = g_53_hot_dog_x_mq_img;
                 size_of_JPEG_image = sizeof(g_53_hot_dog_x_mq_img);
                 strcpy(cat_ans,"hot_dog");
                 break;
        case 54: address_of_JPEG_image = g_54_club_sandwich_x_mq_img;
                 size_of_JPEG_image = sizeof(g_54_club_sandwich_x_mq_img);
                 strcpy(cat_ans,"club_sandwich");
                 break;
        case 55: address_of_JPEG_image = g_55_pizza_x_mq_img;
                 size_of_JPEG_image = sizeof(g_55_pizza_x_mq_img);
                 strcpy(cat_ans,"pizza");
                 break;
        case 56: address_of_JPEG_image = g_56_caesar_salad_x_mq_img;
                 size_of_JPEG_image = sizeof(g_56_caesar_salad_x_mq_img);
                 strcpy(cat_ans,"caesar_salad");
                 break;
        case 57: address_of_JPEG_image = g_57_spaghetti_carbonara_x_mq_img;
                 size_of_JPEG_image = sizeof(g_57_spaghetti_carbonara_x_mq_img);
                 strcpy(cat_ans,"spaghetti_carbonara");
                 break;
        case 58: address_of_JPEG_image = g_58_fried_rice_x_mq_img;
                 size_of_JPEG_image = sizeof(g_58_fried_rice_x_mq_img);
                 strcpy(cat_ans,"fried_rice");
                 break;
        case 59: address_of_JPEG_image = g_59_caesar_salad_x_mq_img;
                 size_of_JPEG_image = sizeof(g_59_caesar_salad_x_mq_img);
                 strcpy(cat_ans,"caesar_salad");
                 break;
        case 60: address_of_JPEG_image = g_60_french_fries_x_mq_img;
                 size_of_JPEG_image = sizeof(g_60_french_fries_x_mq_img);
                 strcpy(cat_ans,"french_fries");
                 break;
        case 61: address_of_JPEG_image = g_61_sushi_x_mq_img;
                 size_of_JPEG_image = sizeof(g_61_sushi_x_mq_img);
                 strcpy(cat_ans,"sushi");
                 break;
        case 62: address_of_JPEG_image = g_62_french_fries_x_mq_img;
                 size_of_JPEG_image = sizeof(g_62_french_fries_x_mq_img);
                 strcpy(cat_ans,"french_fries");
                 break;
        case 63: address_of_JPEG_image = g_63_pancakes_x_mq_img;
                 size_of_JPEG_image = sizeof(g_63_pancakes_x_mq_img);
                 strcpy(cat_ans,"pancakes");
                 break;
        default:
                 break;
    }

    /* Check JPEG data exists */
    {
        static const  uint8_t  num_0xFF_JPEG_header_letter_1 = 0xFFu;
        static const  uint8_t  num_0xD8_JPEG_header_letter_2 = 0xD8u;

        if (!(

            /* Cast to an appropriate type */
            ( (uint32_t) address_of_JPEG_image[0] == num_0xFF_JPEG_header_letter_1 ) &&

            /* Cast to an appropriate type */
            ( (uint32_t) address_of_JPEG_image[1] == num_0xD8_JPEG_header_letter_2 ) ))
        {
            e = E_OTHER;        /* Error: JPEG data is not in ROM */
            goto fin;
        }
    }

    /* (variables) = ... */
    {
        size_t    frame_size = GS_FRAME_WIDTH * GS_FRAME_BYTE_PER_PIXEL * GS_FRAME_HEIGHT;
        uint8_t*  memory_address_of_VRAM;
        size_t    size_of_VRAM;

        e= R_SAMPLE_GetVRAM( &memory_address_of_VRAM,  &size_of_VRAM );
        if(e)
        {
            goto fin;
        }

        /* (variables) = ... */
        memory_address_of_RAW  = memory_address_of_VRAM;
        memory_size_of_RAW     = frame_size;
        memory_address_of_JPEG = memory_address_of_VRAM + frame_size;
        memory_size_of_JPEG    = size_of_VRAM - frame_size;

        /* physical_address_of_RAW = ... */
        e= R_MEMORY_ToPhysicalAddress( memory_address_of_RAW,  &physical_address_of_RAW );
        if(e)
        {
            goto fin;
        }

        /* physical_address_of_JPEG = ... */
        e= R_MEMORY_ToPhysicalAddress( memory_address_of_JPEG,  &physical_address_of_JPEG );
        if(e)
        {
            goto fin;
        }

        /* Guard */
        if (!( ((uintptr_t) memory_address_of_RAW  % 8) == 0 ))
        {
            e=__LINE__;
            goto fin;
        }

        /* Cast to an appropriate type */
        if (!( ((uintptr_t) memory_address_of_JPEG % 8) == 0 ))
        {
            e=__LINE__;
            goto fin;
        }
        if ( !( memory_size_of_JPEG > size_of_JPEG_image ))
        {
            e=__LINE__;
            goto fin;
        }

        /* decoded_event = ... */
        s= R_OS_SemaphoreCreate( &decoded_event,  1 );
        if(!s)
        {
            e=__LINE__;
            goto fin;
        }
    }

    /* Write JPEG data in physical memory */
    memcpy( memory_address_of_JPEG,  address_of_JPEG_image,  size_of_JPEG_image );
    memset( memory_address_of_RAW,  0,  memory_size_of_RAW );

    /* R_JCU_Initialize() */
    e= R_JCU_Initialize( NULL );
    if(e)
    {
        goto fin;
    }
    is_JCU_initialized = true;

    e= R_JCU_SelectCodec( JCU_DECODE );
    if(e)
    {
        goto fin;
    }

    /* R_JCU_SetDecodeParam() */
    {
        buffer.source.swapSetting       = JCU_SWAP_JPEG_NONE;

        /* Cast to an appropriate type */
        buffer.source.address           = (uint32_t*) physical_address_of_JPEG;

        /* Cast to an appropriate type */
        buffer.destination.address      = (uint32_t*) physical_address_of_RAW;

        /* Cast to an appropriate type */
        buffer.lineOffset               = (int16_t) GS_FRAME_WIDTH;
        decode.decodeFormat             = GS_OUTPUT_PIXEL_FORMAT;

        if ( GS_OUTPUT_PIXEL_FORMAT == JCU_OUTPUT_YCbCr422 )
        {
            buffer.destination.swapSetting  = JCU_SWAP_CB_Y0_CR_Y1_PIXEL0123;
            decode.outputCbCrOffset = JCU_CBCR_OFFSET_128;
        }
        else if ( GS_OUTPUT_PIXEL_FORMAT == JCU_OUTPUT_RGB565 )
        {
            buffer.destination.swapSetting  = JCU_SWAP_RGB565_PIXEL0123;
            decode.outputCbCrOffset = JCU_CBCR_OFFSET_0;
        }
        else
        {
            if (!( GS_OUTPUT_PIXEL_FORMAT == JCU_OUTPUT_ARGB8888 ))
            {
                e=E_OTHER;
                goto fin;
            }
            buffer.destination.swapSetting  = JCU_SWAP_ARGB8888_PIXEL01;
            decode.outputCbCrOffset = JCU_CBCR_OFFSET_0;
        }

        decode.alpha                 = GS_ALPHA_VAL_MAX;
        decode.horizontalSubSampling = JCU_SUB_SAMPLING_1_1;
        decode.verticalSubSampling   = JCU_SUB_SAMPLING_1_1;

        e= R_JCU_SetDecodeParam( &decode,  &buffer );
        if(e)
        {
            goto fin;
        }
    }
    e= R_JCU_SetPauseForImageInfo( true );
    if(e)
    {
        goto fin;
    }

    printf( "Decoding:"GS_N );

    /* Cast to an appropriate type */
    printf( "    memory_address_of_JPEG   = 0x%08X"GS_N,  (uintptr_t) memory_address_of_JPEG );
    printf( "    physical_address_of_JPEG = 0x%08X"GS_N,  physical_address_of_JPEG );

    /* Cast to an appropriate type */
    printf( "    memory_address_of_RAW   = 0x%08X"GS_N,  (uintptr_t) memory_address_of_RAW );
    printf( "    physical_address_of_RAW = 0x%08X"GS_N,  physical_address_of_RAW );

    /* image = ... : R_JCU_GetImageInfo() */
    s= R_OS_SemaphoreWait( &decoded_event,  0 ); /* Clear. Time out must be ignored. */

    /* Cast to an appropriate type */
    e= R_JCU_StartAsync( (r_co_function_t) R_OS_SemaphoreRelease,  &decoded_event );
    if(e)
    {
        goto fin;
    } /* Set */
    s= R_OS_SemaphoreWait( &decoded_event,  R_OS_ABSTRACTION_EV_WAIT_INFINITE );
    if(!s)
    {
        e=__LINE__;
        goto fin;
    }
    e= R_JCU_GetAsyncError();
    if(e)
    {
        goto fin;
    }

    e= R_JCU_GetImageInfo( &image );
    if(e)
    {
        goto fin;
    } /* image = . */

    /* R_JCU_ContinueAsync() : Decode JPEG image */
    s= R_OS_SemaphoreWait( &decoded_event,  0 ); /* Clear. Time out must be ignored. */

    /* Cast to an appropriate type */
    e= R_JCU_ContinueAsync( JCU_IMAGE_INFO,  (r_co_function_t) R_OS_SemaphoreRelease,  &decoded_event );
    if(e)
    {
        goto fin;
    } /* Set */
    s= R_OS_SemaphoreWait( &decoded_event,  R_OS_ABSTRACTION_EV_WAIT_INFINITE );
    if(!s)
    {
        e=__LINE__;
        goto fin;
    }
    e= R_JCU_GetAsyncError();
    if(e)
    {
        goto fin;
    }

    uint32_t k = 0;
    uint32_t i = 0;
    for (i = 0; i < memory_size_of_RAW; i=i+3)
    {
         image_buff[i+0] = memory_address_of_RAW[k+0];
         image_buff[i+1] = memory_address_of_RAW[k+1];
         image_buff[i+2] = memory_address_of_RAW[k+2];
         k=k+4;
    }

    /* ResizeBilinearFixedRgb */
    R_MMU_VAtoPA((uint32_t)&image_buff[0], &(resize_image_param.src));
    R_MMU_VAtoPA((uint32_t)&image_rgb_out[0], &(resize_image_param.dst));
    R_CACHE_L1DataCleanInvalidAll();
    resize_image_param.src_width  = 128;
    resize_image_param.src_height = 128;
    resize_image_param.fx         = 0x08 ;
    resize_image_param.fy         = 0x08 ;

    ret_val = R_DK2_Load(&g_drp_resize_bilinear_fixed_rgb[0], R_DK2_TILE_0, R_DK2_TILE_PATTERN_6, NULL, &cb_drp_finish, &drp_lib_id[0]);
    ret_val = R_DK2_Activate(drp_lib_id[TILE_0], 0);

    drp_lib_status[TILE_0] = DRP_NOT_FINISH;

    ret_val = R_DK2_Start(drp_lib_id[TILE_0], (void *)&resize_image_param, sizeof(r_drp_resize_bilinear_fixed_rgb_t));

    while (drp_lib_status[TILE_0] == DRP_NOT_FINISH)
    {
        ;
    }

    ret_val = R_DK2_Unload(drp_lib_id[TILE_0], &drp_lib_id[TILE_0]);

    p_output_bufadr = R_BCD_LcdGetVramAddress();
    /* convert to phisical address */
    R_MMU_VAtoPA((uint32_t)p_output_bufadr, &(image_argb_out));

    k = 0;
    for (i = 0; i < MEM_SIZE_RGB_OUT; i=i+3)
    {
         image_argb_out[k+0] = image_rgb_out[i+0];
         image_argb_out[k+1] = image_rgb_out[i+1];
         image_argb_out[k+2] = image_rgb_out[i+2];
         image_argb_out[k+3] = 0xff;
         k=k+4;
    }

    /* Write the data(buf) on the cache to physical memory */
    R_CACHE_L1DataCleanLine(R_BCD_LcdGetVramAddress(), (R_BCD_CAMERA_WIDTH * R_BCD_CAMERA_HEIGHT * 4));
    
    R_OS_TaskSleep( 1000 );

    e=0;
fin:
    /* R_JCU_TerminateAsync() */
    if ( is_JCU_initialized )
    {
        errnum_t  ee;
        uint32_t  terminate_event = 0;
        s= R_OS_SemaphoreCreate( &terminate_event,  1 );
        if((!s) && (e==0))
        {
            e=__LINE__;
        }
        s= R_OS_SemaphoreWait( &terminate_event,  0 ); /* Clear. Time out must be ignored. */

        /* Cast to an appropriate type */
        ee = R_JCU_TerminateAsync( (r_co_function_t) R_OS_SemaphoreRelease,  &terminate_event ); /* Set */
        if(e==0)
        {
            e=ee;
        }
        s= R_OS_SemaphoreWait( &terminate_event,  R_OS_ABSTRACTION_EV_WAIT_INFINITE );
        if((!s) && (e==0))
        {
            e=__LINE__;
        }
        ee= R_JCU_GetAsyncError();
        if(e==0)
        {
            e=ee;
        }
        R_OS_SemaphoreDelete( &terminate_event );
    }

    /* ... */
    R_OS_SemaphoreDelete( &decoded_event );
    R_ERROR_Set( e );

    return  e;

}
#endif  /* DEMO_MODE */
/**********************************************************************************************************************
* End of function R_JCU_SampleDecode
**********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: set_disp_data
* Description  : set lcd disp.
* Arguments    : -
* Return Value : -
**********************************************************************************************************************/
static void set_disp_data(void)
{
    char score_p[3] = {"%"};
    
    if (demo_mode == false)             /* CAMERA Input On */
    {
        /* Clear the current capture state and enable the detection of the next capture completion */
        R_BCD_CameraClearCaptureStatus();

        /* Capture Start */
        R_BCD_CameraCaptureStart();

        /* Diplay process time */
        R_BCD_LcdClearGraphicsBuffer();

//        sprintf((char *)&buf[0]," ACCURACY:%d.%d %s",(int)(inference_result.score * 100), 
//                ((int)(inference_result.score * 1000) - ((int)(inference_result.score * 100))*10), score_p);
//        R_BCD_LcdWriteString(&buf[0], 402, 32, GREEN);
        sprintf((char *)&buf[0],"%s %d.%d %s", inference_result.category, (int)(inference_result.score * 100), 
                ((int)(inference_result.score * 1000) - ((int)(inference_result.score * 100))*10), score_p );
        R_BCD_LcdWriteString(&buf[0], 402, 92, WHITE);
        sprintf((char *)&buf[0],"%s", inference_result.price);
        R_BCD_LcdWriteString(&buf[0], 402, 122, WHITE);
        sprintf((char *)&buf[0]," TIME:%4d ms",(int)(inference_result.us_total / 1000));
        R_BCD_LcdWriteString(&buf[0], 402, 272, YELLOW);
        if(cpu_drp_sel == 1)            /* Use DRP */
        {
            sprintf((char *)&buf[0],"DRP");
        }
        else                            /* CPU only */
        {
            sprintf((char *)&buf[0],"CPU");
        }
        R_BCD_LcdWriteString(&buf[0], 600, 272, RED);

        /* Display overlay buffer written processing time */
        R_BCD_LcdSwapGraphicsBuffer();

    }
}
/**********************************************************************************************************************
* End of function set_disp_data
**********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: conv_ycbcr_to_bgr
* Description  : This function convert input image from Ycbcr to BGR and reduce image size by half.
* Arguments    : Input image address.
* Return Value : Output image address.
**********************************************************************************************************************/
static void conv_ycbcr_to_bgr(uint8_t *src, uint8_t *dst)
{
    int32_t i;
    uint32_t j;
    int16_t tmp;
    int16_t ty0,ty1,ty2,ty3;

    /* convert YCbCr to GBR */
    for (j = OFFSET_Y; j< OFFSET_Y+256*STRIDE; j+=(STRIDE*2) )
    {
    #ifdef HORIZONTAL_FLIP
        for (i = OFFSET_X+256*2-4; i >= OFFSET_X; i-=4 )
    #else
        for (i = OFFSET_X; i <= 256*2-4; i+=4 )
    #endif
        {
            ty0 = src[j+i+1];
            ty1 = src[j+i+3];
            ty2 = src[j+i+STRIDE+1];
            ty3 = src[j+i+STRIDE+3];
            
            /* convert YCbCr to B */
            /* B is calculated using formula:1.000*y + 1.772*(cb-128). */
            tmp = ty0+ty1+ty2+ty3+(3.544 * ((int16_t)src[j+i] + (int16_t)src[j+i+STRIDE] - 256));
            if(tmp < 0)
            {
                *dst = 0;
            }
            else if(tmp > 1020)
            {
                *dst = 255;
            }
            else
            {
                *dst = (uint8_t)(tmp/4);
            }
            dst++;
            /* convert YCbCr to G */
            /* G is calculated using formula:1.000*y - 0.344*(cb-128) - 0.714*(cr-128).*/
            tmp = ty0+ty1+ty2+ty3-(0.688 * ((int16_t)src[j+i] + (int16_t)src[j+i+STRIDE] - 256))-(1.428 * ((int16_t)src[j+i+2] + (int16_t)src[j+i+STRIDE+2] - 256));
            if(tmp < 0)
            {
                *dst = 0;
            }
            else if(tmp > 1020)
            {
                *dst = 255;
            }
            else
            {
                *dst = (uint8_t)(tmp/4);
            }
            dst++;
            /* convert YCbCr to R */
            /* R is calculated using formula:1.000*y + 1.402*(cr-128).*/
            tmp = ty0+ty1+ty2+ty3+(2.804 * ((int16_t)src[j+i+2] + (int16_t)src[j+i+STRIDE+2] - 256));
            if(tmp < 0)
            {
                *dst = 0;
            }
            else if(tmp > 1020)
            {
                *dst = 255;
            }
            else
            {
                *dst = (uint8_t)(tmp/4);
            }
            dst++;
        }
    }
}
/**********************************************************************************************************************
* End of function conv_ycbcr_to_bgr
**********************************************************************************************************************/

/* End of File */
