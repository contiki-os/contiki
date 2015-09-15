/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
 * \file
 * Timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup timer
 * @{
 */

#include "contiki-conf.h"
#include "sys/clock.h"
#include "sys/timer.h"

/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void
timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_restart() function.
 *
 * \note Must not be executed before timer expired
 *
 * \param t A pointer to the timer.
 * \sa timer_restart()
 */
void
timer_reset(struct timer *t)
{
  t->start += t->interval;
}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
void
timer_restart(struct timer *t)
{
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
int
timer_expired(struct timer *t)
{
  /* Note: Can not return diff >= t->interval so we add 1 to diff and return
     t->interval < diff - required to avoid an internal error in mspgcc. */
  clock_time_t diff = (clock_time() - t->start) + 1;
  return t->interval < diff;

}
/*---------------------------------------------------------------------------*/
/**
 * \brief compare timer expiry times
 *
 * Compares the two timers to see which will expire sooner. The cmp_pt time
 * value is a point in time to compare around. Usually this will just be the
 * current clock time.
 *
 * \param t0 first timer to compare
 * \param t1 second timer to compare
 * \param cmp_pt a point in time to compare against. Usually the current time.
 *
 * \return less than, equal to, or greater than zero if t0 is less than, equal
 * to or greater than t1 respectivley
 */
int
timer_cmp(const struct timer *t0, const struct timer *t1, clock_time_t cmp_pt)
{
  clock_time_t t0_expiry = t0->start + t0->interval;
  clock_time_t t1_expiry = t1->start + t1->interval;

  int t0_expired = t0->interval <= (cmp_pt - t0->start);
  int t1_expired = t1->interval <= (cmp_pt - t1->start);

  if(t0_expired && !t1_expired) {
    /* t0 expired already but t1 didn't, so it compares less */
    return -1;
  } else if(!t0_expired && t1_expired) {
    /* t0 hasn't expired  but t1 has, so it compares greater */
    return 1;
  }
  /* either, both timers are expired or both aren't */

  if(t0_expiry == t1_expiry) {
    /* if the expiry time is the same, then they must be equal whether both
       timers are expired or not expired */
    return 0;
  }

  if(!t0_expired) {
    /* neither are expired, cmp_pt is defined to be less than the expiry
       times so we compare on that basis. The smallest diff to cmp_pt compares
       less. */
    return (t0_expiry - cmp_pt) < (t1_expiry - cmp_pt) ? -1 : 1;
  } else {
    /* both are expired, cmp_pt is defined to be greater than the expiry
       times so we compare on that basis. The largest diff to cmp_pt compares
       less. */
    return (cmp_pt - t0_expiry) > (cmp_pt - t1_expiry) ? -1 : 1;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * The time until the timer expires
 *
 * This function returns the time until the timer expires.
 *
 * \param t A pointer to the timer
 *
 * \return The time until the timer expires
 *
 */
clock_time_t
timer_remaining(struct timer *t)
{
  return t->start + t->interval - clock_time();
}
/*---------------------------------------------------------------------------*/

/** @} */
