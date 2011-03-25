/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details.
 *
 *
 */

#ifndef PWM_H
#define PWM_H

/* Initialize PWM output.
   timer_num = 0, 1, 2, 3
   rate = desired rate in Hz,
   duty = desired duty cycle.  0=always off, 65536=always on.
   enable_timer = whether to actually run the timer, versus just configuring it
   Returns actual PWM rate. */
uint32_t pwm_init_ex(int timer_num, uint32_t rate, uint32_t duty, int enable_timer);

/* Initialize PWM output, helper macros
   timer = TMR0, TMR1, TMR2, TMR2
   rate = desired rate in Hz,
   duty = desired duty cycle.  0=always off, 65536=always on.
   Returns actual PWM rate. */
#define pwm_init(timer,rate,duty) pwm_init_ex(TMR_NUM(timer), rate, duty, 1)
#define pwm_init_stopped(timer,rate,duty) pwm_init_ex(TMR_NUM(timer), rate, duty, 0)

/* Change duty cycle.  Safe to call at any time.
   timer_num = 0, 1, 2, 3
   duty = desired duty cycle.  0=always off, 65536=always on.
*/
void pwm_duty_ex(int timer_num, uint32_t duty);

/* Change duty cycle.  Safe to call at any time.
   timer = TMR0, TMR1, TMR2, TMR2
   duty = desired duty cycle.  0=always off, 65536=always on.
*/
#define pwm_duty(timer,duty) pwm_duty_ex(TMR_NUM(timer), duty)

#endif
