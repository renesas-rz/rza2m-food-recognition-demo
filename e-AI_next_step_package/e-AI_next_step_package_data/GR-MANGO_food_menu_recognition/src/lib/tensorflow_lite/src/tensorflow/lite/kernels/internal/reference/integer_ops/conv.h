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
// Ver.1.0  (2020/12/23)  //

#ifndef TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_INTEGER_OPS_CONV_H_
#define TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_INTEGER_OPS_CONV_H_

/* Modified by Renesas Electronics { */
#include <math.h>
#include "perform.h"
#include "../../../generate/sc_drivers/r_drp/inc/r_dk2_if.h"
#include "../../../generate/sc_drivers/r_drp/drp_lib_u/r_drp_cnn_conv.h"
#include "r_cache_lld_rza2m.h"

#include "tensorflow/lite/kernels/internal/common.h"

/*******************************************************************************
Imported global variables and functions (from other files)
*******************************************************************************/
extern uint8_t g_drp_lib_conv_test[];
extern uint8_t g_drp_lib_tflm_conv[];

extern unsigned int cpu_drp_sel;
extern unsigned int drp_conv_total_num;
extern unsigned int drp_en_conv_layer;
extern unsigned int drp_conv_count;
extern unsigned int all_conv_count;

namespace tflite {
namespace reference_integer_ops {

/*******************************************************************************
Macro definitions
*******************************************************************************/
#define TILE_0            (0)
#define TILE_1            (1)
#define TILE_2            (2)
#define TILE_3            (3)
#define TILE_4            (4)
#define TILE_5            (5)

#define DRP_NOT_FINISH    (0)
#define DRP_FINISH        (1)

/*******************************************************************************
  Private global variables and functions
*******************************************************************************/
static uint8_t drp_lib_id[R_DK2_TILE_NUM] = {0};
static volatile uint8_t drp_lib_status[R_DK2_TILE_NUM] = {DRP_NOT_FINISH};

//namespace tflite {
//namespace reference_integer_ops {

/******************************************************************************
Functions Prototypes
******************************************************************************/
static void cb_drp_finish(uint8 id);
static void calc_div_param(r_drp_cnn_conv_t *param);
static int num2div(uint8 ch_num);

/*******************************************************************************
* Function Name: cb_drp_finish
* Description  : This function is a callback function called from the
*              : DRP driver at the finish of the DRP library processing.
* Arguments    : id
*              :   The ID of the DRP library that finished processing.
* Return Value : -
*******************************************************************************/
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
/*******************************************************************************
* End of function cb_drp_finish
*******************************************************************************/

//namespace tflite {
//namespace reference_integer_ops {

/**** CPU *****/
// Fixed-point per-channel-quantization convolution reference kernel.
inline void ConvPerChannel(
    const ConvParams& params, const int32* output_multiplier,
    const int32* output_shift, const RuntimeShape& input_shape,
    const int8* input_data, const RuntimeShape& filter_shape,
    const int8* filter_data, const RuntimeShape& bias_shape,
    const int32* bias_data, const RuntimeShape& output_shape,
    int8* output_data) {

  // Get parameters.
  const int32 input_offset = params.input_offset;  // r = s(q - Z)
  const int stride_width = params.stride_width;
  const int stride_height = params.stride_height;
  const int dilation_width_factor = params.dilation_width_factor;
  const int dilation_height_factor = params.dilation_height_factor;
  const int pad_width = params.padding_values.width;
  const int pad_height = params.padding_values.height;
  const int32 output_offset = params.output_offset;

  // Set min and max value of the output.
  const int32 output_activation_min = params.quantized_activation_min;
  const int32 output_activation_max = params.quantized_activation_max;

  // Sanity check.
  TFLITE_DCHECK_LE(output_activation_min, output_activation_max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  if (bias_data) {                                           // Temp_taka
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);   // Temp_taka
  }                                                          // Temp_taka

  // Check dimensions of the tensors.
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int filter_height = filter_shape.Dims(1);
  const int filter_width = filter_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);

  for (int batch = 0; batch < batches; ++batch) {
    for (int out_y = 0; out_y < output_height; ++out_y) {
      for (int out_x = 0; out_x < output_width; ++out_x) {
        for (int out_channel = 0; out_channel < output_depth; ++out_channel) {
          const int in_x_origin = (out_x * stride_width) - pad_width;
          const int in_y_origin = (out_y * stride_height) - pad_height;
          int32 acc = 0;
          for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
            for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
              for (int in_channel = 0; in_channel < input_depth; ++in_channel) {
                const int in_x = in_x_origin + dilation_width_factor * filter_x;
                const int in_y =
                    in_y_origin + dilation_height_factor * filter_y;
                // Zero padding by omitting the areas outside the image.
                const bool is_point_inside_image =
                    (in_x >= 0) && (in_x < input_width) && (in_y >= 0) &&
                    (in_y < input_height);
                if (is_point_inside_image) {
                  int32 input_val = input_data[Offset(input_shape, batch, in_y,
                                                      in_x, in_channel)];
                  int32 filter_val =
                      filter_data[Offset(filter_shape, out_channel, filter_y,
                                         filter_x, in_channel)];
                  // Accumulate with 32 bits accumulator.
                  // In the nudging process during model quantization, we force
                  // real value of 0.0 be represented by a quantized value. This
                  // guarantees that the input_offset is a int8, even though it
                  // is represented using int32.
                  // int32 += int8 * (int8 - int8) so the highest value we can
                  // get from each accumulation is [-127, 127] * ([-128, 127] -
                  // [-128, 127]), which is [-32512, 32512]. log2(32512)
                  // = 14.98, which means we can accumulate at least 2^16
                  // multiplications without overflow. The accumulator is
                  // applied to a filter so the accumulation logic will hold as
                  // long as the filter size (filter_y * filter_x * in_channel)
                  // does not exceed 2^16, which is the case in all the models
                  // we have seen so far.
                  // TODO(jianlijianli): Add a check to make sure the
                  // accumulator depth is smaller than 2^16.
                  acc += filter_val * (input_val + input_offset);
                }
              }
            }
          }

          if (bias_data) {
            acc += bias_data[out_channel];
          }

          acc = MultiplyByQuantizedMultiplier(
              acc, output_multiplier[out_channel], output_shift[out_channel]);
          acc += output_offset;
          acc = std::max(acc, output_activation_min);
          acc = std::min(acc, output_activation_max);
          output_data[Offset(output_shape, batch, out_y, out_x, out_channel)] =
              static_cast<int8_t>(acc);
        }
      }
    }
  }

}

// Fixed-point per-channel-quantization convolution reference kernel.
// 16-bit data and 8-bit filter
inline void ConvPerChannel(
    const ConvParams& params, const int32* output_multiplier,
    const int32* output_shift, const RuntimeShape& input_shape,
    const int16* input_data, const RuntimeShape& filter_shape,
    const int8* filter_data, const RuntimeShape& bias_shape,
    const std::int64_t* bias_data, const RuntimeShape& output_shape,
    int16* output_data) {
  // Get parameters.
  const int stride_width = params.stride_width;
  const int stride_height = params.stride_height;
  const int dilation_width_factor = params.dilation_width_factor;
  const int dilation_height_factor = params.dilation_height_factor;
  const int pad_width = params.padding_values.width;
  const int pad_height = params.padding_values.height;

  // Set min and max value of the output.
  const int32 output_activation_min = params.quantized_activation_min;
  const int32 output_activation_max = params.quantized_activation_max;

  // Sanity check.
  TFLITE_DCHECK_LE(output_activation_min, output_activation_max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  if (bias_data) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  // Check dimensions of the tensors.
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int filter_height = filter_shape.Dims(1);
  const int filter_width = filter_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);
  for (int batch = 0; batch < batches; ++batch) {
    for (int out_y = 0; out_y < output_height; ++out_y) {
      for (int out_x = 0; out_x < output_width; ++out_x) {
        for (int out_channel = 0; out_channel < output_depth; ++out_channel) {
          const int in_x_origin = (out_x * stride_width) - pad_width;
          const int in_y_origin = (out_y * stride_height) - pad_height;
          std::int64_t acc = 0;
          for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
            for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
              for (int in_channel = 0; in_channel < input_depth; ++in_channel) {
                const int in_x = in_x_origin + dilation_width_factor * filter_x;
                const int in_y =
                    in_y_origin + dilation_height_factor * filter_y;
                // Zero padding by omitting the areas outside the image.
                const bool is_point_inside_image =
                    (in_x >= 0) && (in_x < input_width) && (in_y >= 0) &&
                    (in_y < input_height);
                if (is_point_inside_image) {
                  int32 input_val = input_data[Offset(input_shape, batch, in_y,
                                                      in_x, in_channel)];
                  int32 filter_val =
                      filter_data[Offset(filter_shape, out_channel, filter_y,
                                         filter_x, in_channel)];
                  // Accumulate with 64 bits accumulator.
                  // int64 += int8 * int16 so the highest value we can
                  // get from each accumulation is [-127, 127] * ([-32768,
                  // 32767] -
                  // [-32768, 32767]), which is [-8322945, 8322945].
                  // log2(8322945) = 22.99.
                  acc += filter_val * input_val;
                }
              }
            }
          }
          if (bias_data) {
            acc += bias_data[out_channel];
          }
          int32_t scaled_acc = MultiplyByQuantizedMultiplier(
              acc, output_multiplier[out_channel], output_shift[out_channel]);
          scaled_acc = std::max(scaled_acc, output_activation_min);
          scaled_acc = std::min(scaled_acc, output_activation_max);
          output_data[Offset(output_shape, batch, out_y, out_x, out_channel)] =
              static_cast<int16_t>(scaled_acc);
        }
      }
    }
  }
}


inline void calc_div_param(r_drp_cnn_conv_t *param)
{
	// fmap_width_div
	int in_ch = (param->in_ch == 3) ? 4 : param->in_ch;
	int width_factor = (param->fmap_width % 3 == 0) ? 3 :
	                   (param->fmap_width % 5 == 0) ? 5 :
	                   (param->fmap_width % 7 == 0) ? 7 : 1;
	int width_div1 = std::min<int>(param->fmap_width,
	                  pow(2, floor(log2(8192 / (3 * in_ch) - 2))));
	int width_div2 = std::min<int>(param->fmap_width,
	                  pow(2, floor(log2((8192 / (3 * in_ch) - 2) / width_factor)))
	                          * width_factor);

	if (param->fmap_width % width_div1 != 0) {
	  param->fmap_width_div = width_div2;
	} else if (param->fmap_width % width_div2 != 0) {
	  param->fmap_width_div = width_div1;
	} else {
	  param->fmap_width_div = std::max(width_div1, width_div2);
	}

	param->fmap_width_itr = param->fmap_width / param->fmap_width_div;

	// out_ch_div
	int out_ch_factor = (param->out_ch_half % 3 == 0) ? 3 :
	                    (param->out_ch_half % 5 == 0) ? 5 :
	                    (param->out_ch_half % 7 == 0) ? 7 : 1;
	int out_ch_div1 = std::min<int>(param->out_ch_half,
	                      pow(2, floor(log2(2048 / (9 * in_ch)))));
	int out_ch_div2 = std::min<int>(param->out_ch_half,
	                      pow(2, floor(log2(2048 / (9 * in_ch * out_ch_factor))))
	                              * out_ch_factor);
    if (param->out_ch_half % out_ch_div1 != 0) {
        param->out_ch_div = out_ch_div2;
	} else if (out_ch_div2 == 0 || param->out_ch_half % out_ch_div2 != 0) {
	    param->out_ch_div = out_ch_div1;
	} else {
	    param->out_ch_div = std::max(out_ch_div1, out_ch_div2);
	}

	param->out_ch_itr = param->out_ch_half / param->out_ch_div;
}


static r_drp_cnn_conv_t param_drp_conv[R_DK2_TILE_NUM]  __attribute__ ((section("CACHED_RAM")));

/**** DRP *****/
inline void R_BCD_DRP_CNN_CONV(
    const ConvParams& params,
    const int32* output_multiplier,
    const int32* output_shift,
    const RuntimeShape& input_shape,
    const int8* input_data,
    const RuntimeShape& filter_shape,
    const int8* filter_data,
    const RuntimeShape& bias_shape,
    const int32* bias_data,
    const RuntimeShape& output_shape,
    int8* output_data){

    bool has_inch = (input_shape.Dims(3) == 3) ;
    int32_t ret_val;
    uint32_t tile_no;
    uint32_t us_total;
    uint32_t flt_size = filter_shape.Dims(1) * filter_shape.Dims(2);

    uint16_t inch_inc_num = has_inch ? 1 : input_shape.Dims(3) / 4;
    uint16_t inch_set     = has_inch ? 4 : input_shape.Dims(3);
    uint32_t load_addr_offset    = (output_shape.Dims(3) /2 ) * 4;
    uint32_t load_addr_offset_f  = has_inch ? flt_size * (output_shape.Dims(3) / 2) * (inch_set - 1)
                                            : flt_size * (output_shape.Dims(3) / 2) * inch_set;

    //// for fmap_width_div and outch_div
    r_drp_cnn_conv_t p_calc;

    p_calc.fmap_width  = input_shape.Dims(2);
    p_calc.in_ch       = MatchingDim(input_shape, 3, filter_shape, 3);
    p_calc.out_ch_half = MatchingDim(filter_shape, 0, output_shape, 3) / 2;

    calc_div_param(&p_calc);

	/***************************/
	/*      Load DRP Library   */
	/*        +------------ -+ */
	/* tile 0 |              | */
	/*        +   Conv Test  + */
	/* tile 1 |    (3 Tile)  | */
	/*        +              + */
	/* tile 2 |              | */
	/*        +--------------+ */
	/* tile 3 |              | */
	/*        +   Conv Test  + */
	/* tile 4 |    (3 Tile)  | */
	/*        +              + */
	/* tile 5 |              | */
	/*        +--------------+ */
	/***************************/

    if (drp_conv_count == 0) {
        ret_val = R_DK2_Load(&g_drp_lib_tflm_conv[0], R_DK2_TILE_0 | R_DK2_TILE_3,	R_DK2_TILE_PATTERN_3_3, NULL, &cb_drp_finish, &drp_lib_id[0]);

    /************************/
    /* Activate DRP Library */
    /************************/
        ret_val = R_DK2_Activate(drp_lib_id[TILE_0] | drp_lib_id[TILE_3], 0);

    }

   	/***************************************/
    /* Set R_DK2_Start function parameter  */
    /***************************************/
    for (tile_no = 0; tile_no < R_DK2_TILE_NUM; tile_no+=3) {
        /* Set the address of buffer to be read/write by DRP */
        param_drp_conv[tile_no].in_addr    = (uint32_t)input_data;
        param_drp_conv[tile_no].out_addr   = (tile_no == TILE_0) ? (uint32_t)output_data : (uint32_t)output_data + (MatchingDim(filter_shape, 0, output_shape, 3) / 2);
        param_drp_conv[tile_no].flt_addr   = (tile_no == TILE_0) ? (uint32_t)filter_data : (uint32_t)filter_data + load_addr_offset_f;
        param_drp_conv[tile_no].bias_addr  = (tile_no == TILE_0) ? (uint32_t)bias_data : (uint32_t)bias_data + load_addr_offset;

        /* Set Image size */
        param_drp_conv[tile_no].fmap_width  = input_shape.Dims(2);
        param_drp_conv[tile_no].fmap_hight  = input_shape.Dims(1);
        param_drp_conv[tile_no].in_ch       = MatchingDim(input_shape, 3, filter_shape, 3);
        param_drp_conv[tile_no].out_ch_half = MatchingDim(filter_shape, 0, output_shape, 3) / 2;
        param_drp_conv[tile_no].fmap_width_div  = p_calc.fmap_width_div;
        param_drp_conv[tile_no].fmap_width_itr  = p_calc.fmap_width_itr;
        param_drp_conv[tile_no].line_data_div_size = p_calc.fmap_width_div * input_shape.Dims(3);
        param_drp_conv[tile_no].line_data_size     = input_shape.Dims(2) * input_shape.Dims(3);
        param_drp_conv[tile_no].flt_data_div_size  = flt_size * p_calc.out_ch_div * inch_inc_num;
        param_drp_conv[tile_no].flt_data_size      = flt_size * (output_shape.Dims(3) / 2) * input_shape.Dims(3);

        /* Set the address of extended buffer to be read/write by DRP*/
        param_drp_conv[tile_no].out_mul_addr   = (tile_no == TILE_0) ? (uint32_t)output_multiplier : (uint32_t)output_multiplier + load_addr_offset;
        param_drp_conv[tile_no].out_shift_addr = (tile_no == TILE_0) ? (uint32_t)output_shift : (uint32_t)output_shift + load_addr_offset;
        param_drp_conv[tile_no].out_ch_itr = p_calc.out_ch_itr;
        param_drp_conv[tile_no].out_ch_div = p_calc.out_ch_div;
        param_drp_conv[tile_no].in_offset  = params.input_offset;				//input offset  # ;
        param_drp_conv[tile_no].out_offset = params.output_offset;				//output offset  # ;
        param_drp_conv[tile_no].out_line_addr_inc = input_shape.Dims(2) * (output_shape.Dims(3) / 2);
        param_drp_conv[tile_no].out_div_addr_inc  = p_calc.fmap_width_div * (output_shape.Dims(3) / 2);

        /* Initialize variables to be used in termination judgment of the DRP application */
        drp_lib_status[tile_no] = DRP_NOT_FINISH;

		/*********************/
		/* Start DRP Library */
		/*********************/

        R_CACHE_L1DataCleanAll();

        ret_val = R_DK2_Start(drp_lib_id[tile_no], (void *)&param_drp_conv[tile_no], sizeof(r_drp_cnn_conv_t));

    }
    /***************************************/
    /* Wait until DRP processing is finish */
    /***************************************/
    while (drp_lib_status[TILE_0] == DRP_NOT_FINISH || drp_lib_status[TILE_3] == DRP_NOT_FINISH);

    ///////////// Cache Invalid !!
    int cache_area = output_shape.Dims(1) * output_shape.Dims(2) * output_shape.Dims(3);
    R_CACHE_L1DataInvalidLine(output_data, cache_area);

    /***************************************/
    /* Unload DRP Library                  */
    /***************************************/
    if (drp_conv_count == ( drp_conv_total_num - 1 )) {
        ret_val = R_DK2_Unload(drp_lib_id[TILE_0] | drp_lib_id[TILE_3], &drp_lib_id[0]);
    }
}

}  // namespace reference_integer_ops
}  // namespace tflite
/* Modified by Renesas Electronics } */

#endif  // TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_INTEGER_OPS_CONV_H_
