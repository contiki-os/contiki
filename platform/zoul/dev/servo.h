/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-servo Generic servo driver
 *
 * Driver for a Generic Servo actuator
 *
 * @{
 *
 * \file
 * Header file for a Generic Servo driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef SERVO_H_
#define SERVO_H_
#include <stdio.h>
#include "dev/pwm.h"
/* -------------------------------------------------------------------------- */
/**
 * \name Servo default settings
 * @{
 */
/* -------------------------------------------------------------------------- */
#ifndef SERVO_CONF_FREQ
#define SERVO_DEFAULT_FREQ       50 /**< 50 Hz */
#else
#define SERVO_DEFAULT_FREQ       SERVO_CONF_FREQ
#endif

#ifndef SERVO_CONF_MAX_DEGREES
#define SERVO_MAX_DEGREES        180
#else
#define SERVO_MAX_DEGREES        SERVO_CONF_MAX_DEGREES
#endif

#ifndef SERVO_CONF_MIN_VAL
#define SERVO_MIN_VAL            9600 /**> roughly equals to 3% duty cycle */
#else
#define SERVO_MIN_VAL            SERVO_CONF_MIN_VAL
#endif

#ifndef SERVO_CONF_MAX_VAL
#define SERVO_MAX_VAL            38400 /**> roughly equals to 12% duty cycle */
#else
#define SERVO_MAX_VAL            SERVO_CONF_MAX_VAL
#endif
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Servo general purpose timers mapping
 * @{
 */
#define SERVO_CHANNEL_1          0x001 /**< GPT0-B */
#define SERVO_CHANNEL_2          0x100 /**< GPT1-A */
#define SERVO_CHANNEL_3          0x101 /**< GPT1-B */
#define SERVO_CHANNEL_4          0x200 /**< GPT2-A */
#define SERVO_CHANNEL_5          0x201 /**< GPT2-B */
#define SERVO_CHANNEL_6          0x300 /**< GPT3-A */
#define SERVO_CHANNEL_7          0x301 /**< GPT3-B */
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Servo general constants
 * @{
 */
#define SERVO_SUCCESS            0
#define SERVO_ERROR              (-1)
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name Servo public funtions
 * @{
 */

/** \brief Configures and positions a servo in a given position (by degrees)
 *         The servo will lock its position as long as it is not stopped
 * \param gptab Servo channel (PWM GPT from 1-7)
 * \param port  Port number to use as PWM
 * \param pin   Pin number to use as PWM
 * \param pos   Position to map the servo to (0-360º, integer)
 * \return \c   SERVO_SUCCESS if successful, else \c SERVO_ERROR
 */
int servo_position(uint16_t gptab, uint8_t port, uint8_t pin, uint16_t pos);

/** \brief Fully stop a servo and reconfigures back the pin/port as GPIO
 * \param gptab Servo channel (PWM GPT from 1-7)
 * \param port  Port number to use as PWM
 * \param pin   Pin number to use as PWM
 * \return \c   SERVO_SUCCESS if successful, else \c SERVO_ERROR
 */
int servo_stop(uint16_t gptab, uint8_t port, uint8_t pin);
/** @} */
/* -------------------------------------------------------------------------- */
#endif
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

