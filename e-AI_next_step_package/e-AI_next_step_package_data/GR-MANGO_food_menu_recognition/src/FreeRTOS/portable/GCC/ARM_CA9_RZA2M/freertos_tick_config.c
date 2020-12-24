/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* Renesas driver includes. */
#include <stdint.h>
#include "iodefine.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "Task.h"

#include "compiler_settings.h"
#include "r_devlink_wrapper.h"
#include "r_os_abstraction_api.h"
#include "r_ostm_drv_api.h"

/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function.  This file contains a function
 * that is suitable for use on the Renesas RZ MPU.
 */

#define R_FREERTOS_RUNTIME_STATS_OSTM_RESOURCE ("\\\\.\\runtime_stats_timer")
#define runtimeCLOCK_SCALE_SHIFT	( 9UL )
#define runtimeOVERFLOW_BIT			( 1UL << ( 32UL - runtimeCLOCK_SCALE_SHIFT ) )

static int_t s_tick_timer_handle = (-1);
static int_t s_runtime_stats_timer_handle = (-1);

void vConfigureTickInterrupt( void )
{
    if (( -1) == s_tick_timer_handle)
    {
        s_tick_timer_handle = open(R_OS_ABSTRACTION_OSTM_RESOURCE, O_RDWR);

        if (( -1) == s_tick_timer_handle)
        {
            R_OS_AssertCalled( __FILE__, __LINE__);
        }
        else
        {
            control(s_tick_timer_handle, CTRL_OSTM_START_TIMER, NULL);
        }
    }
}
/*-----------------------------------------------------------*/

/*
 * Crude implementation of a run time counter used to measure how much time
 * each task spends in the Running state.
 */
unsigned long ulGetRunTimeCounterValue( void )
{
    static unsigned long ulLastCounterValue = 0UL, ulOverflows = 0;
    unsigned long ulValueNow = 0;

    if (( -1) == s_runtime_stats_timer_handle)
    {
        R_OS_AssertCalled( __FILE__, __LINE__);
    }
    else
    {
        control(s_runtime_stats_timer_handle, CTRL_OSTM_READ_COUNTER, &ulValueNow);
    }

    /* Has the value overflowed since it was last read. */
    if( ulValueNow < ulLastCounterValue )
    {
        ulOverflows++;
    }
    ulLastCounterValue = ulValueNow;

    /* There is no prescale on the counter, so simulate in software. */
    ulValueNow >>= runtimeCLOCK_SCALE_SHIFT;
    ulValueNow += ( runtimeOVERFLOW_BIT * ulOverflows );

    return ulValueNow;
}
/*-----------------------------------------------------------*/

void vInitialiseRunTimeStats( void )
{
    if (( -1) == s_runtime_stats_timer_handle)
    {
        s_runtime_stats_timer_handle = open(R_FREERTOS_RUNTIME_STATS_OSTM_RESOURCE, O_RDWR);

        if (( -1) == s_runtime_stats_timer_handle)
        {
            R_OS_AssertCalled( __FILE__, __LINE__);
        }
        else
        {
            control(s_runtime_stats_timer_handle, CTRL_OSTM_START_TIMER, NULL);
        }
    }
}
