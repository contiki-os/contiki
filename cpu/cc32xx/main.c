/*
 * Copyright (c) 2015, 3B Scientific GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "hw_types.h"
#include "hw_memmap.h"

#include "rom.h"
#include "rom_map.h"

#include "osi.h"
#include "pin_mux_config.h"

#include "interrupt.h"
#include "hw_ints.h"
#include "prcm.h"

#include "contiki-conf.h"

// Vector table
extern void (* const g_pfnVectors[])(void);

// Contiki main task
extern void contiki_main(void *pv_parameters);

// Local defines
#define SPAWN_TASK_PRIORITY     	9
#define CONTIKI_TASK_PRIORITY		1

#ifdef CONTIKI_CONF_STACKSIZE
#define CONTIKI_STACKSIZE 			CONTIKI_CONF_STACKSIZE
#else
#define CONTIKI_STACKSIZE 			4096
#endif

#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************

//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void vAssertCalled(const char *pcFile, unsigned long ulLine)
{
    //Handle Assert here
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationIdleHook(void)
{
    //Handle Idle Hook for Profiling, Power Management etc
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook(OsiTaskHandle *pxTask, signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
#endif //USE_FREERTOS

// Board Initialization & Configuration
static void BoardInit(void)
{
	// In case of TI-RTOS vector table is initialize by OS itself
	//
#ifndef USE_TIRTOS
	// Set vector table base
#if defined(ccs) || defined(gcc)
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#endif

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);
    PRCMCC3200MCUInit();
}

int main(void)
{
	// Board Initialization
	BoardInit();

	// Configure PinMux
	PinMuxConfig();

	// Create contiki main task
	osi_TaskCreate(contiki_main, (const signed char * const)"ContikiWorker", CONTIKI_STACKSIZE, NULL, CONTIKI_TASK_PRIORITY, NULL);

    // Start the task scheduler
    osi_start();

    return 0;
}
