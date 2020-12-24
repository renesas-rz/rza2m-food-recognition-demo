/*******************************************************************************
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
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
// Ver.1.0  (2020/12/23)  //

/*******************************************************************************
* File Name    : r_drp_cnn_conv.h
* Description  : This source code is the header file of
*                  : r_drp_cnn_conv.c
******************************************************************************/
#ifndef R_DRP_CNN_CONV_H
#define R_DRP_CNN_CONV_H

/*******************************************************************************
Global Typedef definitions
*******************************************************************************/
#ifdef BDL
    typedef signed char int8_t ;
    typedef signed short int16_t ;
    typedef signed long int32_t ;
    typedef signed longlong int64_t ;
    typedef unsigned char uint8_t ;
    typedef unsigned short uint16_t ;
    typedef unsigned long uint32_t ;
    typedef unsigned longlong uint64_t ;
#else/*BDL*/
    #include <stdint.h>
#endif /*BDL*/

typedef struct _r_drp_cnn_conv_t {
    uint32_t  in_addr;            /* input feature map buffer address */
    uint32_t  out_addr;	          /* output feature map buffer address*/
    uint32_t  flt_addr;	          /* filter buffer address*/
    uint32_t  bias_addr;          /* bias buffer address */
    uint32_t  out_mul_addr;	      /* outmul buffer address */
    uint32_t  out_shift_addr;     /* outshift buffer address  */
    uint16_t  fmap_width;         /* input, output feature map width */
    uint16_t  fmap_hight;         /* input, output feature map height */
    uint8_t   in_ch;              /* number of input channel */
    uint8_t   out_ch_half;        /* half number of output channel */
    uint8_t   out_ch_itr;         /* out_ch_half / out_ch_div */
    uint8_t   out_ch_div;         /* division size for out_ch_half */
    uint16_t  fmap_width_itr;     /* fmap_width / fmap_width_div */
    uint16_t  fmap_width_div;     /* division size of fmap_width*/
    uint16_t  line_data_div_size; /* fmap_width_div * in_ch */
    uint16_t  line_data_size;     /* fmap_width * in_ch */
    uint16_t  flt_data_div_size;  /* FLT_SIZE(3*3) * out_ch_div * inch_set/4   (inch_set : in_ch or 4) */
    uint16_t  reserve0;           /* Reserved0 */
    uint32_t  flt_data_size;      /* FLT_SIZE(3*3) * out_ch_half * in_ch */
    int16_t   in_offset;          /* input feature map value offset */
    int16_t   out_offset;         /* output feature map value offset */
    uint32_t  out_line_addr_inc;  /* increment size for output address(line) */
    uint32_t  out_div_addr_inc;   /* increment size for output address(div) */
    uint32_t  reserve1;           /* Reserved0 */
} r_drp_cnn_conv_t;

#endif /* R_DRP_CNN_CONV_H */

/* end of file */







