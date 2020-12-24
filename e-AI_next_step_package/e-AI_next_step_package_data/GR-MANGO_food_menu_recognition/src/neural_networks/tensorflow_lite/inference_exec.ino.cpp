/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
/* Copyright 2020 Renesas Electronics

Renesas has modified part of this file. The following comments have been added to
the modifies / additions.

	start 	Modified by Renesas Electronics {
	end   	Modified by Renesas Electronics }
==============================================================================*/
/* Modified by Renesas Electronics { */
#include "perform.h"
#include "r_cache_lld_rza2m.h"

#include "draw.h"
#include "fontdata.h"
#include "r_bcd_camera.h"
#include "r_bcd_lcd.h"

#include "r_dk2_if.h"
#include <TensorFlowLite.h>

#include "inference_exec.h"

#include "model_settings.h"

#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/testing/micro_test.h"
#include "../../neural_networks/tensorflow_lite/food_menu_recognition.cc"

unsigned int drp_conv_total_num;
unsigned int drp_conv_count;
unsigned int all_conv_count;
unsigned int drp_en_conv_layer;

char cat_drp[20];
char cat_cpu[20];
char price_drp[20];
char cat_drp_jdg[20];
char cat_cpu_jdg[20];
char score_p[3] = {"%"};
float score_drp;
float score_cpu;

extern uint8_t*  memory_address_of_RAW;
extern size_t    memory_size_of_RAW;
extern int_t     n_food;
extern char      cat_ans[20];
extern bool      demo_mode;
extern uint_t    cpu_drp_sel;
extern inference_result_t inference_result;

/* Globals, used for compatibility with Arduino-style sketches. */
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

/* An area of memory to use for input, output, and intermediate arrays. */
constexpr int kTensorArenaSize = 800 * 1024;   /* model 30 int */
static uint8_t tensor_arena[kTensorArenaSize] __attribute__ ((aligned(16)));
}  // namespace

static uint8_t buf[80] __attribute__ ((section("Graphics_OCTA_RAM")));

void inference_exec()
{
    uint32_t us_total;
    uint32_t us_total_cpu;
    uint32_t us_total_drp;
    us_total_drp = 0;
    us_total_cpu = 0;

    int iii, iii_max;
    float xxx, xxx_max, xxx_all ;

    if (demo_mode == false)
    {
        /* CAMERA Input mode */
        input = interpreter->input(0);

        int k = 0;
        for (int i = 0; i < memory_size_of_RAW; i=i+3)
        {
            input->data.f[i+0] = memory_address_of_RAW[k+2] / 255.;
            input->data.f[i+1] = memory_address_of_RAW[k+1] / 255.;
            input->data.f[i+2] = memory_address_of_RAW[k+0] / 255.;
            k=k+3;
        }

        PerformSetStartTime(0);

        all_conv_count = 0;
        drp_conv_count = 0;
        /* Run the model on this input and make sure it succeeds. */
        if (kTfLiteOk != interpreter->Invoke())
        {
            TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
        }

        PerformSetEndTime(0);
        us_total  = PerformGetElapsedTime_us(0);

        TfLiteTensor* output = interpreter->output(0);

        /* Process the inference results. */
        inference_result.caesar_salad_score        = output->data.f[k_caesar_salad];
        inference_result.club_sandwich_score       = output->data.f[k_club_sandwich];
        inference_result.donuts_score              = output->data.f[k_donuts];
        inference_result.edamame_score             = output->data.f[k_edamame];
        inference_result.french_fries_score        = output->data.f[k_french_fries];
        inference_result.fried_rice_score          = output->data.f[k_fried_rice];
        inference_result.hamburger_score           = output->data.f[k_hamburger];
        inference_result.hot_dog_score             = output->data.f[k_hot_dog];
        inference_result.oysters_score             = output->data.f[k_oysters];
        inference_result.pancakes_score            = output->data.f[k_pancakes];
        inference_result.pizza_score               = output->data.f[k_pizza];
        inference_result.ramen_score               = output->data.f[k_ramen];
        inference_result.spaghetti_carbonara_score = output->data.f[k_spaghetti_carbonara];
        inference_result.steak_score               = output->data.f[k_steak];
        inference_result.sushi_score               = output->data.f[k_sushi];

        xxx = 0.0; xxx_max = 0.0; xxx_all = 0.0;
        iii_max = 0;
        for(iii=0 ; iii < kCategoryCount ; iii++)
        {
            xxx = output->data.f[iii];
            xxx_all = xxx_all + xxx ;
            if(xxx > xxx_max )
            {
                xxx_max = xxx ;
                iii_max = iii ;
            }
        }

        if(cpu_drp_sel == 1)
        {
            inference_result.us_total = us_total;
            strcpy(cat_drp, kCategoryLabels[iii_max]);
            strcpy(price_drp, kCategoryPrices[iii_max]);
            inference_result.category = &cat_drp[0];
            inference_result.price    = &price_drp[0];
            inference_result.score = xxx_max;
        }
        else
        {
            inference_result.us_total = us_total;
            strcpy(cat_cpu, kCategoryLabels[iii_max]);
            inference_result.category = &cat_cpu[0];
            inference_result.score = xxx_max;
        }
    }
    else
    {
        R_BCD_LcdClearGraphicsBuffer();

        sprintf((char *)&buf[0],"ANS %s", cat_ans);
        R_BCD_LcdWriteString(&buf[0], 402,  2, WHITE);

        sprintf((char *)&buf[0],"CPU STOP");
        R_BCD_LcdWriteString(&buf[0], 402, 32, RED);
        sprintf((char *)&buf[0]," TIME:");
        R_BCD_LcdWriteString(&buf[0], 402, 62, GREEN);
        sprintf((char *)&buf[0]," PROB:");
        R_BCD_LcdWriteString(&buf[0], 402, 92, GREEN);
        sprintf((char *)&buf[0]," CAT :");
        R_BCD_LcdWriteString(&buf[0], 402,122, GREEN);
        sprintf((char *)&buf[0]," JUDGE: ");
        R_BCD_LcdWriteString(&buf[0], 402,152, YELLOW);

        sprintf((char *)&buf[0],"DRP STOP");
        R_BCD_LcdWriteString(&buf[0], 402,182, RED);
        sprintf((char *)&buf[0]," TIME:");
        R_BCD_LcdWriteString(&buf[0], 402,212, GREEN);
        sprintf((char *)&buf[0]," PROB:");
        R_BCD_LcdWriteString(&buf[0], 402,242, GREEN);
        sprintf((char *)&buf[0]," CAT :");
        R_BCD_LcdWriteString(&buf[0], 402,272, GREEN);
        sprintf((char *)&buf[0]," JUDGE: ");
        R_BCD_LcdWriteString(&buf[0], 402,302, YELLOW);

        /* Display overlay buffer written processing time */
        R_BCD_LcdSwapGraphicsBuffer();

        /* Display wait */
        for(int i=0;i<0x0fffffff;i++){
            ;
        }

        for(cpu_drp_sel = 0 ; cpu_drp_sel < 2 ; cpu_drp_sel++ )
        {
            input = interpreter->input(0);

            int k = 0;
            for (int i = 0; i < memory_size_of_RAW; i=i+3)
            {
                input->data.f[i+0] = memory_address_of_RAW[k+2] / 255.;
                input->data.f[i+1] = memory_address_of_RAW[k+1] / 255.;
                input->data.f[i+2] = memory_address_of_RAW[k+0] / 255.;
                k=k+4;
            }

            R_BCD_LcdClearGraphicsBuffer();

            if( cpu_drp_sel == 1 )
            {
                sprintf((char *)&buf[0],"ANS %s", cat_ans);
                R_BCD_LcdWriteString(&buf[0], 402,  2, WHITE);

                sprintf((char *)&buf[0],"CPU STOP>>START>>END");
                R_BCD_LcdWriteString(&buf[0], 402, 32, RED);
                sprintf((char *)&buf[0]," TIME:%4d ms",(int)(us_total_cpu / 1000));
                R_BCD_LcdWriteString(&buf[0], 402, 62, GREEN);
                sprintf((char *)&buf[0]," PROB:%d.%d %s",(int)(score_cpu * 100), ((int)(score_cpu * 1000) - ((int)(score_cpu * 100))*10), score_p);
                R_BCD_LcdWriteString(&buf[0], 402, 92, GREEN);
                sprintf((char *)&buf[0]," CAT :%s", cat_cpu);
                R_BCD_LcdWriteString(&buf[0], 402,122, GREEN);
                sprintf((char *)&buf[0]," JUDGE: %s", cat_cpu_jdg);
                R_BCD_LcdWriteString(&buf[0], 402,152, YELLOW);

                sprintf((char *)&buf[0],"DRP STOP>>START");
                R_BCD_LcdWriteString(&buf[0], 402,182, RED);
                sprintf((char *)&buf[0]," TIME:");
                R_BCD_LcdWriteString(&buf[0], 402,212, GREEN);
                sprintf((char *)&buf[0]," PROB:");
                R_BCD_LcdWriteString(&buf[0], 402,242, GREEN);
                sprintf((char *)&buf[0]," CAT :");
                R_BCD_LcdWriteString(&buf[0], 402,272, GREEN);
                sprintf((char *)&buf[0]," JUDGE: ");
                R_BCD_LcdWriteString(&buf[0], 402,302, YELLOW);
            }
            else
            {
                sprintf((char *)&buf[0],"ANS %s", cat_ans);
                R_BCD_LcdWriteString(&buf[0], 402,  2, WHITE);

                sprintf((char *)&buf[0],"CPU STOP>>START");
                R_BCD_LcdWriteString(&buf[0], 402, 32, RED);
                sprintf((char *)&buf[0]," TIME:");
                R_BCD_LcdWriteString(&buf[0], 402, 62, GREEN);
                sprintf((char *)&buf[0]," PROB:");
                R_BCD_LcdWriteString(&buf[0], 402, 92, GREEN);
                sprintf((char *)&buf[0]," CAT :");
                R_BCD_LcdWriteString(&buf[0], 402,122, GREEN);
                sprintf((char *)&buf[0]," JUDGE: ");
                R_BCD_LcdWriteString(&buf[0], 402,152, YELLOW);

                sprintf((char *)&buf[0],"DRP STOP");
                R_BCD_LcdWriteString(&buf[0], 402,182, RED);
                sprintf((char *)&buf[0]," TIME:");
                R_BCD_LcdWriteString(&buf[0], 402,212, GREEN);
                sprintf((char *)&buf[0]," PROB:");
                R_BCD_LcdWriteString(&buf[0], 402,242, GREEN);
                sprintf((char *)&buf[0]," CAT :");
                R_BCD_LcdWriteString(&buf[0], 402,272, GREEN);
                sprintf((char *)&buf[0]," JUDGE: ");
                R_BCD_LcdWriteString(&buf[0], 402,302, YELLOW);
            }

            /* Display overlay buffer written processing time */
            R_BCD_LcdSwapGraphicsBuffer();

            PerformSetStartTime(0);

            all_conv_count = 0;
            drp_conv_count = 0;
            /* Run the model on this input and make sure it succeeds. */
            if (kTfLiteOk != interpreter->Invoke())
            {
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
            }

            PerformSetEndTime(0);
            us_total  = PerformGetElapsedTime_us(0);

            TfLiteTensor* output = interpreter->output(0);

            /* Process the inference results. */
            float caesar_salad_score        = output->data.f[k_caesar_salad];
            float club_sandwich_score       = output->data.f[k_club_sandwich];
            float donuts_score              = output->data.f[k_donuts];
            float edamame_score             = output->data.f[k_edamame];
            float french_fries_score        = output->data.f[k_french_fries];
            float fried_rice_score          = output->data.f[k_fried_rice];
            float hamburger_score           = output->data.f[k_hamburger];
            float hot_dog_score             = output->data.f[k_hot_dog];
            float oysters_score             = output->data.f[k_oysters];
            float pancakes_score            = output->data.f[k_pancakes];
            float pizza_score               = output->data.f[k_pizza];
            float ramen_score               = output->data.f[k_ramen];
            float spaghetti_carbonara_score = output->data.f[k_spaghetti_carbonara];
            float steak_score               = output->data.f[k_steak];
            float sushi_score               = output->data.f[k_sushi];

            xxx = 0.0; xxx_max = 0.0; xxx_all = 0.0;
            iii_max = 0;
            for(iii=0 ; iii < kCategoryCount ; iii++)
            {
                xxx = output->data.f[iii];
                xxx_all = xxx_all + xxx ;
                if(xxx > xxx_max )
                {
                    xxx_max = xxx ;
                    iii_max = iii ;
                }
            }

            char cpu_drp[10];
            if(cpu_drp_sel == 1)
            {
                strcpy( cpu_drp, "DRP" );
            }
            else
            {
                strcpy( cpu_drp, "CPU" );
            }

            printf("\n\r");
            printf("%s %02d 00_caesar_salad_score = %1.8f \r\n", cpu_drp,n_food,caesar_salad_score );
            printf("%s %02d 01_club_sandwich_score = %1.8f \r\n", cpu_drp,n_food,club_sandwich_score );
            printf("%s %02d 02_donuts_score = %1.8f \r\n", cpu_drp,n_food,donuts_score );
            printf("%s %02d 03_edamame_score = %1.8f \r\n", cpu_drp,n_food,edamame_score );
            printf("%s %02d 04_french_fries_score = %1.8f \r\n", cpu_drp,n_food,french_fries_score );
            printf("%s %02d 05_fried_rice_score = %1.8f \r\n", cpu_drp,n_food,fried_rice_score );
            printf("%s %02d 06_hamburger_score = %1.8f \r\n", cpu_drp,n_food,hamburger_score );
            printf("%s %02d 07_hot_dog_score = %1.8f \r\n", cpu_drp,n_food,hot_dog_score );
            printf("%s %02d 08_oysters_score = %1.8f \r\n", cpu_drp,n_food,oysters_score );
            printf("%s %02d 09_pancakes_score = %1.8f \r\n", cpu_drp,n_food,pancakes_score );
            printf("%s %02d 10_pizza_score = %1.8f \r\n", cpu_drp,n_food,pizza_score );
            printf("%s %02d 11_ramen_score = %1.8f \r\n", cpu_drp,n_food,ramen_score );
            printf("%s %02d 12_spaghetti_carbonara_score = %1.8f \r\n", cpu_drp,n_food,spaghetti_carbonara_score );
            printf("%s %02d 13_steak_score = %1.8f \r\n", cpu_drp,n_food,steak_score );
            printf("%s %02d 14_sushi_score = %1.8f \r\n", cpu_drp,n_food,sushi_score );
            printf("\n\r");

            printf("iii = %02d, score = %1.8f, all = %1.8f \r\n", iii_max, xxx_max, xxx_all );

            printf("\n\r");
            printf("Total Time %d.%03d[ms]", (int)us_total/1000, (int)(us_total-(us_total/1000)*1000));
            printf("\n\r");
            printf("\n\r");

            if(cpu_drp_sel == 1)
            {
                us_total_drp = us_total;
                strcpy(cat_drp, kCategoryLabels[iii_max]);
                score_drp = xxx_max;
                if(strcmp(cat_cpu,cat_ans))
                {
                    strcpy(cat_drp_jdg,"NG");
                }
                else
                {
                    strcpy(cat_drp_jdg,"OK");
                }
            }
            else
            {
                us_total_cpu = us_total;
                strcpy(cat_cpu, kCategoryLabels[iii_max]);
                score_cpu = xxx_max;
                if(strcmp(cat_cpu,cat_ans))
                {
                    strcpy(cat_cpu_jdg,"NG");
                }
                else
                {
                    strcpy(cat_cpu_jdg,"OK");
                }
            }

            R_BCD_LcdClearGraphicsBuffer();

            if( cpu_drp_sel == 1 )
            {
                sprintf((char *)&buf[0],"ANS %s", cat_ans);
                R_BCD_LcdWriteString(&buf[0], 402,  2, WHITE);

                sprintf((char *)&buf[0],"CPU STOP>>START>>END");
                R_BCD_LcdWriteString(&buf[0], 402, 32, RED);
                sprintf((char *)&buf[0]," TIME:%4d ms",(int)(us_total_cpu / 1000));
                R_BCD_LcdWriteString(&buf[0], 402, 62, GREEN);
                sprintf((char *)&buf[0]," PROB:%d.%d %s",(int)(score_cpu * 100), ((int)(score_cpu * 1000) - ((int)(score_cpu * 100))*10), score_p);
                R_BCD_LcdWriteString(&buf[0], 402, 92, GREEN);
                sprintf((char *)&buf[0]," CAT :%s", cat_cpu);
                R_BCD_LcdWriteString(&buf[0], 402,122, GREEN);
                sprintf((char *)&buf[0]," JUDGE: %s", cat_cpu_jdg);
                R_BCD_LcdWriteString(&buf[0], 402,152, YELLOW);

                sprintf((char *)&buf[0],"DRP STOP>>START>>END");
                R_BCD_LcdWriteString(&buf[0], 402,182, RED);
                sprintf((char *)&buf[0]," TIME:%4d ms",(int)(us_total_drp / 1000));
                R_BCD_LcdWriteString(&buf[0], 402,212, GREEN);
                sprintf((char *)&buf[0]," PROB:%d.%d %s",(int)(score_drp * 100), ((int)(score_drp * 1000) - ((int)(score_drp * 100))*10), score_p);
                R_BCD_LcdWriteString(&buf[0], 402,242, GREEN);
                sprintf((char *)&buf[0]," CAT :%s", cat_drp);
                R_BCD_LcdWriteString(&buf[0], 402,272, GREEN);
                sprintf((char *)&buf[0]," JUDGE: %s", cat_drp_jdg);
                R_BCD_LcdWriteString(&buf[0], 402,302, YELLOW);
            }
            else
            {
                sprintf((char *)&buf[0],"ANS %s", cat_ans);
                R_BCD_LcdWriteString(&buf[0], 402,  2, WHITE);

                sprintf((char *)&buf[0],"CPU STOP>>START>>END");
                R_BCD_LcdWriteString(&buf[0], 402, 32, RED);
                sprintf((char *)&buf[0]," TIME:%4d ms",(int)(us_total_cpu / 1000));
                R_BCD_LcdWriteString(&buf[0], 402, 62, GREEN);
                sprintf((char *)&buf[0]," PROB:%d.%d %s",(int)(score_cpu * 100), ((int)(score_cpu * 1000) - ((int)(score_cpu * 100))*10), score_p);
                R_BCD_LcdWriteString(&buf[0], 402, 92, GREEN);
                sprintf((char *)&buf[0]," CAT :%s", cat_cpu);
                R_BCD_LcdWriteString(&buf[0], 402,122, GREEN);
                sprintf((char *)&buf[0]," JUDGE: %s", cat_cpu_jdg);
                R_BCD_LcdWriteString(&buf[0], 402,152, YELLOW);

                sprintf((char *)&buf[0],"DRP STOP");
                R_BCD_LcdWriteString(&buf[0], 402,182, RED);
                sprintf((char *)&buf[0]," TIME:");
                R_BCD_LcdWriteString(&buf[0], 402,212, GREEN);
                sprintf((char *)&buf[0]," PROB:");
                R_BCD_LcdWriteString(&buf[0], 402,242, GREEN);
                sprintf((char *)&buf[0]," CAT :");
                R_BCD_LcdWriteString(&buf[0], 402,272, GREEN);
                sprintf((char *)&buf[0]," JUDGE: ");
                R_BCD_LcdWriteString(&buf[0], 402,302, YELLOW);
            }
            /* Display overlay buffer written processing time */
            R_BCD_LcdSwapGraphicsBuffer();

            /* Display wait */
            for(int i=0;i<0x1fffffff;i++){
                ;
            }

        }
        printf("No. = %d, ANS = %s, CPU = %s, DRP = %s, JUDGE = %s \r\n", n_food, cat_ans, cat_cpu, cat_drp, cat_drp_jdg);
    }
}

void inference_exec_init(void) {
    
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    model = tflite::GetModel(food_menu_recognition_tflite);
    drp_en_conv_layer = 0x07ff;    /* DRP Layer enable bit */
    drp_conv_total_num = 0;
    for(int i=0;i<16;i++) drp_conv_total_num += ( drp_en_conv_layer >> i )&1;

    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::ops::micro::AllOpsResolver resolver;

    static tflite::MicroInterpreter static_interpreter(
           model, resolver, tensor_arena, kTensorArenaSize, error_reporter);

    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();

    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }
}

void loop_test() {

}
/* Modified by Renesas Electronics } */
