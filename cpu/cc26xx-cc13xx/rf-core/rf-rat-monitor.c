/*
 * rfcore-rat-moninor.c
 *
 *  Created on: 21 èþë. 2017 ã.
 *      Author: alexrayne
 *
 * here is RAT timer overflow monitoring moved from ieee-mode
 * to populate it for prop-mode.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup rf-core
 * @{
 *
 * \addtogroup rf-core-rat
 * @{
 *
 * \file
 * Implementation of the CC13xx/CC26xx RAT sync monitor for NETSTACK_RADIO driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/radio.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "ti-lib.h"
#include "rf-core/rf-core.h"
#include "rf-core/rf-switch.h"
#include "rf-core/rf-rat.h"
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#undef DEBUG
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if DEBUG > 1
#define ANNOTATE(...) printf(__VA_ARGS__)
#define WHEN_ANNOTATE(x)    x
#else
#define ANNOTATE(...)
#define WHEN_ANNOTATE(x)
#endif



/*---------------------------------------------------------------------------*/
// this code imported from ieee-mode
extern int32_t rat_offset;

#if RF_RAT_STYLE > 0
/* SFD timestamp in RAT ticks (but 64 bits) */
//static uint64_t last_rat_timestamp64 = 0;

/* For RAT overflow handling */
static struct ctimer rat_overflow_timer;
typedef uint64_t rtc64_t;
struct {
    rtc64_t                 last_time;
    volatile uint_fast8_t   counter;
} rat_overflow;

//static
uint64_t      last_timestamp;

/* RAT has 32-bit register, overflows once 18 minutes */
#define RAT_RANGE  0x100000000ull
/* approximate value */
#define RAT_OVERFLOW_PERIOD_SECONDS (60 * 18)

static struct rf_rat_controler rf_control;

rf_rat_time_t rf_rat_now(){
    return HWREG(RFC_RAT_BASE + RATCNT);
}

rtc64_t rtc64_now()
{
  return ti_lib_aon_rtc_current_64_bit_value_get() >> 16;
}

/*---------------------------------------------------------------------------*/
#define GAP_RATE 4
static const rtimer_clock_t OVERFLOW_GAP = (RAT_OVERFLOW_PERIOD_SECONDS * RTIMER_SECOND / GAP_RATE);
static const rtimer_clock_t RAT_OVERFLOW_PERIOD_RT = RADIO_TO_RTIMER(RAT_RANGE);
        //(RAT_OVERFLOW_PERIOD_SECONDS * RTIMER_SECOND);
uint_fast8_t rf_rat_check_overflow(bool first_time)
{
  static uint32_t last_value;
  uint32_t current_value;
  uint8_t interrupts_disabled;

  /* Bail out if the RF is not on */
  if(!rf_core_is_accessible()) {
    return 0;
  }

  interrupts_disabled = ti_lib_int_master_disable();
  // need to protect timestamp calculation vs situation when
  //   evaluation time occurs after overflow, but actual stamp exposed before
  // to fix it, use virtual oveflow terminator with some gap before oveflow.
  // this gap can be compensated at evaluation time.
  if(first_time) {
    last_value = rf_rat_now();
    rtc64_t now = rtc64_now();
    if (now < (RAT_OVERFLOW_PERIOD_RT - OVERFLOW_GAP)){
    rat_overflow.counter = 0;
    rat_overflow.last_time = now;
    if ( last_value <= (RAT_RANGE / GAP_RATE) )
        rat_overflow.last_time += OVERFLOW_GAP;
    PRINTF("rat: init =0 , now=%lx rat=%lx\n", now, last_value);
    }
    else {
        int64_t diff = /*llabs*/( now - rat_overflow.last_time );
    if ( diff < (RAT_OVERFLOW_PERIOD_RT - OVERFLOW_GAP-16) )
        // rat_overflow.counter no need update since last check time
        // so avoid u64 divison
        {
        PRINTF("rat: leave %lu , now=%lu ..last=%lu\n"
                , rat_overflow.counter
                , now, rat_overflow.last_time
                );
        }
    else{
        rat_overflow.counter = now / RAT_OVERFLOW_PERIOD_RT;
        rat_overflow.last_time = rat_overflow.counter*RAT_OVERFLOW_PERIOD_RT; //now;
        if ( last_value <= (RAT_RANGE / GAP_RATE) )
            rat_overflow.last_time += OVERFLOW_GAP;
    }
    PRINTF("rat: set %lu , now=%lx.%lx ..last=...%lx  rat=%lx\n"
            , rat_overflow.counter
            , (uint32_t)(now>>32), (uint32_t)now
            , rat_overflow.last_time
            , last_value
            );
    }
  } else {
    current_value = rf_rat_now();// + RAT_RANGE / GAP_RATE;
    if( current_value < last_value) {
      /* Overflow detected */
      //* last_time assign to end of safe gap period from virtual overflow, after that
      //* RTtime of timestamp is surely after overflow
      rat_overflow.last_time = RTIMER_NOW() +OVERFLOW_GAP;
      rat_overflow.counter++;
    }
    last_value = current_value;
  }
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
static const clock_time_t RAT_CHECK_PERIOD = RAT_OVERFLOW_PERIOD_SECONDS * CLOCK_SECOND / 6;
static void
handle_rat_overflow(void *unused)
{
  if(rf_core_is_accessible()) {
      //* if RF is powerdown, nothing to check
      uint_fast8_t success = rf_rat_check_overflow(false);
      (void) success;

  ANNOTATE("RF RAT overflow check %d : overs=%d at %lu\n"
          , success
          , rat_overflow.counter, rat_overflow.last_time);
  }

  ctimer_set(&rat_overflow_timer, RAT_CHECK_PERIOD,
               handle_rat_overflow, NULL);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief prepare RAT timer time, for later timestamp evaluate rf_rat_calc_last_packet_rttime
 * \param rat_timestamp - RAT timer value - time of last RF operation
 */
static
rf_rat_time_t last_rat;
void     rf_rat_last_timestamp(rf_rat_time_t rat_timestamp)
{
    int     adjusted_overflow_counter;
    last_rat = rat_timestamp;

    rf_rat_check_overflow(false);
    adjusted_overflow_counter = rat_overflow.counter;

  /* if the timestamp is large and the last oveflow was recently,
     assume that the timestamp refers to the time before the overflow */
  if( rat_timestamp > (uint32_t)(RAT_RANGE - (RAT_RANGE/GAP_RATE)) ) {
    if( RTIMER_CLOCK_LT(RTIMER_NOW(), rat_overflow.last_time) )
    {
        adjusted_overflow_counter--;
        }
      }
  /* add the overflowed time to the timestamp */
  last_timestamp = (rat_timestamp - rat_offset)
                + RAT_RANGE * adjusted_overflow_counter;
}

/**
* \brief evaluates timestamp of prepared RAT value at last rf_rat_last_packet_timestamp
* \return - timestamp in RTTimer ticks
*/
uint32_t rf_rat_calc_last_rttime(){
  return RADIO_TO_RTIMER(last_timestamp);
}

void rf_rat_monitor_init(const struct rf_rat_controler* rf){
    memcpy(&rf_control, rf, sizeof(rf_control));
    last_timestamp          = 0;
    rat_overflow.last_time  = 0;
    rat_overflow.counter    = 0;
    rf_rat_check_overflow(true);
    ctimer_set(&rat_overflow_timer, RAT_CHECK_PERIOD,
               handle_rat_overflow, NULL);
}

void rf_rat_debug_dump(void){
    PRINTF("RAT calc: last stamp $%llx rat $%lx [rt %lu] , overs %d, over_time %lu\n"
            , last_timestamp, last_rat
            , rf_rat_calc_last_rttime()
            , rat_overflow.counter, rat_overflow.last_time);
}

#else //RF_RAT_STYLE > 0
rf_rat_time_t rf_rat_last_timestamp;
#endif

/**
 * @}
 * @}
 */
