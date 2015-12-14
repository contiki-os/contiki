/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-pwm-driver CC2538 PWM driver
 *
 * Driver for the CC2538 PWM on GPTIMER
 *
 * The driver uses the timers A and B of the general purpose timers to create
 * a PWM signal, allowing to set a duty cycle value from 1-100%.  This
 * implementation relies on having a peripheral clock of 16MHz, but it can be
 * easily changed (see PWM_FREQ_MIN and PWM_FREQ_MAX values).  The reason it is
 * fixed to these frequencies is to have a consistent duty cycle
 * implementation.
 *
 * Depending on the specific needs these limits can be changed to meet a given
 * duty cycle and lower frequencies by using the prescaler (GPTIMER_TnPR).
 *
 * Running a PWM timer prevents the LPM driver from dropping to PM1+.
 *
 * @{
 *
 * \file
 * Header file for the CC2538 PWM driver
 *
 * \author
 *         Javier Sanchez <jsanchez@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef PWM_H_
#define PWM_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/sys-ctrl.h"
/*---------------------------------------------------------------------------*/
/** \name PWM return values
 * @{
 */
#define PWM_SUCCESS    0
#define PWM_ERROR      (-1)
/** @} */
/*---------------------------------------------------------------------------*/
/** \name PWM recommended values respect to peripheral clock frequency
 * @{
 */
/* Roughly 244 Hz with a 16-MHz system clock, no prescaler */
#define PWM_SYS_16MHZ_NO_PRES_MIN            0xFFFF
#define PWM_SYS_16MHZ_NO_PRES_MIN_FREQ       244
/* Roughly 1 Hz with a 16-MHz system clock, to keep frequency parameter in Hz */
#define PWM_SYS_16MHZ_PRES_MIN               0x00F42400
#define PWM_SYS_16MHZ_PRES_MIN_FREQ          1
/* Yields 160 KHz at 16 MHz and allows down to 1% (integer) duty cycles */
#define PWM_SYS_16MHZ_NO_PRES_MAX            100
#define PWM_SYS_16MHZ_NO_PRES_MAX_FREQ       160000
/** @} */
/*---------------------------------------------------------------------------*/
/** \name PWM driver definitions and configuration values
 * @{
 */
#define PWM_TIMER_A                          0
#define PWM_TIMER_B                          1
#define PWM_TIMER_0                          0
#define PWM_TIMER_1                          1
#define PWM_TIMER_2                          2
#define PWM_TIMER_3                          3
#define PWM_TIMER_MIN                        PWM_TIMER_0
#define PWM_TIMER_MAX                        PWM_TIMER_3
#define PWM_SIGNAL_STRAIGHT                  1
#define PWM_SIGNAL_INVERTED                  0
#define PWM_OFF_WHEN_STOP                    0
#define PWM_ON_WHEN_STOP                     1
#define PWM_GPTIMER_CFG_SPLIT_MODE           0x04
#define PWM_DUTY_MAX                         100
#define PWM_DUTY_MIN                         0
#define PWM_FREQ_MIN                         PWM_SYS_16MHZ_PRES_MIN_FREQ
#define PWM_FREQ_MAX                         PWM_SYS_16MHZ_NO_PRES_MAX_FREQ
/** @} */
/*---------------------------------------------------------------------------*/
/** \name PWM functions
 * @{
 */
/** \brief Configures the general purpose timer in PWM mode
 * \param freq  PWM frequency (in Hz)
 * \param duty  PWM duty cycle (percentage in integers)
 * \param timer General purpose timer to use [0-3]
 * \param ab    Select which timer to use (Timer A or B)
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 */
int8_t pwm_enable(uint32_t freq, uint8_t duty, uint8_t timer, uint8_t ab);
/*---------------------------------------------------------------------------*/
/** \brief Disables a previously PWM configured GPTn
 * \param timer General purpose timer to disable [0-3]
 * \param ab    Select which timer to disable (Timer A or B)
 * \param port  Port number used as PWM to disable (set as input GPIO)
 * \param pin   Pin number used as PWM to disable (set as input GPIO)
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 *
 * This function disables a specific timer (A or B) and reset related registers
 * to default values.  The user must explicitely pass the port/pin number of
 * the pin to disable as PWM and to be configured as input GPIO.
 * The module clock is not disabled with this function
 */
int8_t pwm_disable(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin);
/*---------------------------------------------------------------------------*/
/** \brief Once configured, starts the PWM
 * \param timer General purpose timer to start [0-3]
 * \param ab    Select which timer to start (Timer A or B)
 * \param port  Port number to use as PWM
 * \param pin   Pin number to use as PWM
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 */
int8_t pwm_start(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin);
/*---------------------------------------------------------------------------*/
/** \brief Halts the PWM in a given GPT/timer
 * \param timer General purpose timer to stop [0-3]
 * \param ab    Select which timer to stop (Timer A or B)
 * \param port  Port of the gpio port mapped to the PWM to stop
 * \param pin   Pin of the gpio port mapped to the PWM to stop
 * \param state State to leave the pin once stopped, on (1) or off (0)
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 */
int8_t pwm_stop(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin, uint8_t state);
/*---------------------------------------------------------------------------*/
/** \brief Sets the PWM duty cycle signal direction (high/low)
 * \param timer General purpose timer [0-3]
 * \param ab    Select which timer to use (Timer A or B)
 * \param dir   Direction of the PWM signal, \c PWM_SIGNAL_INVERTED or
 *              \c PWM_SIGNAL_STRAIGHT
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 */
int8_t pwm_set_direction(uint8_t timer, uint8_t ab, uint8_t dir);
/*---------------------------------------------------------------------------*/
/** \brief Toggle the PWM signal direction (inverts the current duty cycle)
 * \param timer General purpose timer to use [0-3]
 * \param ab    Select which timer to use (Timer A or B)
 * \return \c   PWM_SUCCESS if successful, else \c PWM_ERROR
 */
int8_t pwm_toggle_direction(uint8_t timer, uint8_t ab);
/*---------------------------------------------------------------------------*/
/** @} */
#endif /* PWM_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
