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
 *         EFM32 RTC driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/clock.h>
#include "contiki.h"
#include "rtc.h"
#include "em_rtc.h"
#include "em_cmu.h"

static alarm_callback_t alarm_callback = NULL;

#define DAY_VALUE_IN_SEC 86400

static uint32_t _u32_uptime = 0;
// Seconds since Epoch
static uint32_t _u32_seconds_since_epoch = 0;
// INFO: EPOCH = January 1st 2013 00:00:00 UTC (1356998400s in UNIX time format)

// alarm0 value may be adjusted later
static uint32_t _u32_alarm0_secvalue = DAY_VALUE_IN_SEC;

//
// Define 'second' value as variable as it will be adjusted when a derivation will be observed
//
static unsigned long _rtc_second = 1;

/*---------------------------------------------------------------------------*/
void RTC_IRQHandler(void)
{
	// Find reason of IRQ

	if(RTC_IntGet() & RTC_IF_COMP0)
	{
      // Update second counters
      //_u32_seconds_since_epoch += DAY_VALUE_IN_SEC;
      //_u32_uptime += DAY_VALUE_IN_SEC;

      // Update alarm (alarm or second values may be updated)
	  // Get about 100us to take into account Interrupt scheduling time
      //SI32_RTC_0->ALARM0.U32 = (_u32_alarm0_secvalue * _rtc_second) - 2;

	  // Reset Counter
	  RTC_CounterReset();
	}
	else if(RTC_IntGet() & RTC_IF_COMP1)
	{
		if(alarm_callback != NULL) alarm_callback();
		// Disable handler
		alarm_callback = NULL;
	}
	else
	{
		printf("%s: unknown reason for RTC interrupt\r\n",__func__);
	}

	// Clear interrupts
	RTC_IntClear(_RTC_IF_MASK);
}


#if 0
void _rtc_savetime_and_reset(void)
{
	__disable_irq();

	// Save Value - ??? (exec time)
	rtc_getvalue();
	// Reset timer
	SI32_RTC_0->CONTROL.TMRSET = 1;

	__enable_irq();
}
#endif

/*---------------------------------------------------------------------------*/
uint32_t rtc_getvalue(void)
{
	return RTC_CounterGet();
}

/*---------------------------------------------------------------------------*/
uint32_t rtc_getuptime(void)
{
    return (rtc_getvalue() + _u32_uptime);
}

/*---------------------------------------------------------------------------*/
uint32_t rtc_gettime(void)
{
//#if (RTC_PRESCALE == 32768)
	return (rtc_getvalue() + _u32_seconds_since_epoch);
//#endif
}

/*---------------------------------------------------------------------------*/
/**
 *  Adjust absolute RTC timebase with <timeoffset> seconds
 */
void rtc_set_offset(int32_t timeoffset)
{
	_u32_seconds_since_epoch += timeoffset;
}

/*---------------------------------------------------------------------------*/
/**
 * Adds a drift correction to RTC timer
 * The drift correction will be defined as addition of signed <sec_offset> seconds
 * all the <nbdays> days
 */
void rtc_set_driftcorrection(int8_t sec_offset, uint8_t nbdays)
{
	int32_t s32_drift_correction = (sec_offset * _rtc_second) / nbdays;

	// Adjust second value if necessary
	if(abs(s32_drift_correction) > _u32_alarm0_secvalue )
	{
		_rtc_second = _rtc_second + (s32_drift_correction / _u32_alarm0_secvalue);
		// Update drift correction
		s32_drift_correction = s32_drift_correction - (s32_drift_correction % _u32_alarm0_secvalue);
	}
	// Set new day alarm value
	_u32_alarm0_secvalue += s32_drift_correction;
}

/*
 * Gestion de l'horloge GRDF :

Pire cas (en précision) :
 + 1sec tous les 255 jours -> +3.921569ms / jour

Alarm 0 précision : 1s/32768 -> 30,518 us / jour

On obtient donc
Alarm0 = <Default_alarm_value> + s32_drift_correction

avec s32_drift_correction = (32768 * S ) / J ;


De plus, en jouant sur la définition d'une seconde (de base =32768 du compteur RTC) :
86400*32768 = 2831155200 ticks / jour
86400*32767 = 2831068800   .... -> -2,636799219 s /jour
86400*32769 = -> +2,636638286

----> pour une correction > 86400 .... soit 30,518 us / seconde ou 2.7 secondes / jour
Adapter aussi rtc_second pour une adaptation plus juste des timings

 */

/*---------------------------------------------------------------------------*/
//
// Alarm 0 is used for perpetual timebase
//

/*---------------------------------------------------------------------------*/
/**
 * Set alarm to ring in <seconds> seconds
 * \param seconds : alarm offset in seconds (max value : 24H)
 * \param callback : callback function to call (Warning, interrupt context execution)
 */
int rtc_setalarm(uint32_t seconds, alarm_callback_t callback)
{
  uint32_t alarmtime = 0;
  uint32_t offset = seconds * _rtc_second;
  uint32_t now = rtc_getvalue();
  uint32_t time_before_reset = (_u32_alarm0_secvalue*_rtc_second) - now;

  if(seconds > _u32_alarm0_secvalue)
  {
      return -EINVAL;
  }

  // Check if counter will loop before alarm
  if( offset > time_before_reset )
  {
      alarmtime = offset - time_before_reset;
  }
  else
  {
      alarmtime = now + offset;
  }

  // Program Alarm
  RTC_IntClear(RTC_IF_COMP0);

  alarm_callback = callback;
  // Enable Alarm 1
  RTC_CompareSet(1, alarmtime);
  RTC_IntEnable(RTC_IF_COMP0);

  return 0;
}


/*---------------------------------------------------------------------------*/
/**
 * Initialize RTC Based on Crystal Oscillator
 */
void rtc_crystal_init(void)
{
  RTC_Init_TypeDef init;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Use the prescaler to reduce power consumption. */
  CMU_ClockDivSet(cmuClock_RTC, RTC_PRESCALE);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false; /* Start disabled to reset counter */
  init.debugRun = false;
  init.comp0Top = false; /* Count to max before wrapping */
  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);

  /* Start RTC counter */
  RTC_Enable(true);
}
/** @} */
