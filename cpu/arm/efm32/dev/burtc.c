/*
 * Copyright (c) 2013, Kerlink
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
 */

/**
 * \addtogroup efm32-devices
 * @{
 */

/**
 * \file
 *         EFM32 Backup BURTC driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/clock.h>
#include "contiki.h"
#include "burtc.h"
#include "power.h"
#include "em_burtc.h"
#include "em_cmu.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define UNIX_EPOCH2000 946684800   // January 1 00:00:00 UTC 2000

//#define DAY_VALUE_IN_SEC 86400
#define DAY_VALUE_IN_SEC 60

#define TICKS_PER_SECOND 256
#define DAY_VALUE_IN_TICKS (DAY_VALUE_IN_SEC * TICKS_PER_SECOND)
/*
 * Info : for 256 ticks/s, integer division in second make
 * lose 1 tick per overflow (i.e 3.9ms / 194 days)
 */
#define OVERFLOW_VALUE_IN_SEC (UINT32_MAX / TICKS_PER_SECOND)


/*---------------------------------------------------------------------------*/
void BURTC_IRQHandler(void)
{
  // Find reason of IRQ

  if(BURTC_IntGet() & BURTC_IF_COMP0)
  {
    PRINTF("BURTC ALARM\r\n");

    // Call user function if defined
    if(BURTC_RETREG->p_alarm_callback != NULL)
    {
      ((alarm_callback_t)BURTC_RETREG->p_alarm_callback)();
    }

    // Finally, disable user alarm
    BURTC_RETREG->p_alarm_callback = NULL;
    /* disable interrupt on compare match */
    BURTC_IntClear(BURTC_IEN_COMP0);
    BURTC_IntDisable(BURTC_IEN_COMP0);
  }
  else if(BURTC_IntGet() & BURTC_IF_LFXOFAIL)
  {
    // WARNING : Hardware problem on 32K crystal
    PRINTF("%s: LFXO Fail !!!\r\n",__func__);
  }
  else if(BURTC_IntGet() & BURTC_IF_OF)
  {
    // Overflow
    PRINTF("%s: Overflow !!!\r\n",__func__);

    BURTC_RETREG->s32_overflow_uptime += OVERFLOW_VALUE_IN_SEC;
  }
  else
  {
    PRINTF("%s: unknown reason for BURTC interrupt\r\n",__func__);
  }

  // Clear interrupts
  BURTC_IntClear(_BURTC_IF_MASK);
}

/*---------------------------------------------------------------------------*/
uint32_t burtc_getvalue(void)
{
  return BURTC_CounterGet();
}

/*---------------------------------------------------------------------------*/
uint32_t burtc_getsecvalue(void)
{
  return (BURTC_CounterGet() / TICKS_PER_SECOND);
}

/*---------------------------------------------------------------------------*/
inline uint32_t burtc_getuptime(void)
{
  // Optimization ( / 256) -> (>>8)  ... Done by compiler (same generated ASM code)
  return (BURTC_RETREG->s32_overflow_uptime + (BURTC_CounterGet() / TICKS_PER_SECOND) );
}

/*---------------------------------------------------------------------------*/
inline uint32_t burtc_gettime()
{
  return ( BURTC_RETREG->u32_prod_timestamp + burtc_getuptime());
}

/*---------------------------------------------------------------------------*/
/**
 *  Adjust correction offset with <timeoffset> seconds (limited to ~ +-68 YEARS)
 */
void burtc_set_offset(int32_t timeoffset)
{
  BURTC_RETREG->s32_overflow_uptime += timeoffset;
}

/*---------------------------------------------------------------------------*/
/**
 *  Set Production initial timestamp
 */
void burtc_set_prod_timestamp(uint32_t prod_timestamp)
{
  BURTC_RETREG->u32_prod_timestamp = prod_timestamp;
}

/*---------------------------------------------------------------------------*/
/**
 * Get next programmmed alarm absolute offset (taking into account overflow)
 */
uint32_t burtc_get_nextalarm_offset(void)
{
  if(BURTC_CompareGet(0) > BURTC_CounterGet() )
  {
    return (BURTC_CompareGet(0) - BURTC_CounterGet());
  }
  else
  {
    // Overflow between present counter and next alarm
    return BURTC_CompareGet(0) + (UINT32_MAX - BURTC_CounterGet());
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Adds a user alarm
 */
void burtc_set_useralarm(uint32_t sec_offset, alarm_callback_t alarm_callback)
{
  uint32_t u32_useralarm_offset = (sec_offset * TICKS_PER_SECOND);
  // Save callback
  //_useralarm_callback = alarm_callback;
  BURTC_RETREG->p_alarm_callback = alarm_callback;
  // Set Alarm to user alarm (allowing wrap around)
  BURTC_CompareSet(0, (uint32_t) (burtc_getvalue() + u32_useralarm_offset));

  /* Enable interrupt on compare match */
  BURTC_IntClear(BURTC_IEN_COMP0);
  BURTC_IntEnable(BURTC_IEN_COMP0);
}


/*---------------------------------------------------------------------------*/
/**
 * Initialize BURTC Based on Crystal Oscillator
 */
void burtc_crystal_init(uint32_t resetcause)
{
  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Start LFXO and wait until it is stable */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

#if 0
  /* Status from retention memory */
  PRINTF("Prodtime = %lu, uptime = %ld\r\n",BURTC_RETREG->u32_prod_timestamp,
                                            BURTC_RETREG->s32_overflow_uptime);
#endif

  /* If no brown out was detected and BURTC still configured,
   * restore time from retention registers
   */
  if ( (!(resetcause & RMU_RSTCAUSE_BUBODREG) && !(resetcause & RMU_RSTCAUSE_BUBODUNREG)) &&
       (0 != BITBAND_PeripheralRead(&BURTC->CTRL, _BURTC_CTRL_MODE_SHIFT) )
     )
  {
    // Update overflow uptime if overflow occurs during sleep
    if(BURTC_CounterGet() < BURTC_TimestampGet() )
    {
      //_s32_overflow_uptime += OVERFLOW_VALUE_IN_SEC;
      //BURTC_RETREGSet(1, _s32_overflow_uptime);
      BURTC_RETREG->s32_overflow_uptime += OVERFLOW_VALUE_IN_SEC;
    }
  }

  /* If First startup, initialize BURTC */
  else
  {
    /* Select LFXO as clock source for BURTC */
    burtcInit.clkSel = burtcClkSelLFXO;
    /* Enable BURTC operation in EM0-EM4 */
    burtcInit.mode = burtcModeEM4;
    /* Set prescaler to max. Resolution is not that important here :
     * (256Hz, 3.9 ms precision - overflow 194 days) */
    burtcInit.clkDiv = 128;
    // Ignore 6 LSb (MAX -> cf. Errata : DO NOT Use LPCOMP=7)
    burtcInit.lowPowerComp = 6; // -> 0.25s compare match precision
    /* Enable BURTC timestamp upon backup mode entry*/
    burtcInit.timeStamp = true;
    /* Counter DO NOT reset when CNT == COMP0 */
    burtcInit.compare0Top = false;
    burtcInit.enable = true;

    BURTC_Init(&burtcInit);

    // Initialize Retention registers
    memset(BURTC_RETREG, 0, sizeof(burtc_retreg_struct));
    /*
    BURTC_RETREG->u32_prod_timestamp = 0;
    BURTC_RETREG->s32_overflow_uptime = 0;
    */
  }

  /* Reset timestamp */
  BURTC_StatusClear();

  /* Enable interrupt on compare match */
  BURTC_IntClear(BURTC_IEN_OF);
  BURTC_IntEnable(BURTC_IEN_OF);
  BURTC_IntClear(BURTC_IEN_LFXOFAIL);
  BURTC_IntEnable(BURTC_IEN_LFXOFAIL);

  NVIC_ClearPendingIRQ(BURTC_IRQn);
  NVIC_EnableIRQ(BURTC_IRQn);
}
/** @} */
