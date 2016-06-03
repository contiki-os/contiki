/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf52dk-devices Device drivers
 * @{
 *
 * \addtogroup nrf52dk-devices-button Buttons driver
 * @{
 *
 * \file
 *         Header file for the nRF52dk button driver.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 *
 */
/*---------------------------------------------------------------------------*/
#ifndef BUTTON_SENSOR_H_
#define BUTTON_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR "Button"
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR_VALUE_STATE    0 /**< Can be passed to value() function
                                            to get current button state */
#define BUTTON_SENSOR_VALUE_DURATION 1 /**< Can be passed to value() function
                                            to get low state duration */

#define BUTTON_SENSOR_VALUE_RELEASED 0
#define BUTTON_SENSOR_VALUE_PRESSED  1
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor buttons[];
/*---------------------------------------------------------------------------*/
#define button_1 buttons[0]
#define button_2 buttons[1]
#define button_3 buttons[2]
#define button_4 buttons[3]
/*---------------------------------------------------------------------------*/
#endif /* BUTTON_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
