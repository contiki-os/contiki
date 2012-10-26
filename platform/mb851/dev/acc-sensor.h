/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			Accelerometer header file.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*/
/*---------------------------------------------------------------------------*/

#ifndef __ACC_SENSOR_H__
#define __ACC_SENSOR_H__

#include "lib/sensors.h"
#include "mems.h"

extern const struct sensors_sensor acc_sensor;

#define ACC_SENSOR "Acc"

/* The type values used in the configure() function.
*/

/* Full-scale*/
#define ACC_RANGE   1                     // type

#define ACC_LOW_RANGE   MEMS_LOW_RANGE    // Values
#define ACC_HIGH_RANGE  MEMS_HIGH_RANGE

/* High pass filter and cut-off frequencies*/
#define ACC_HPF 2                       // type

#define ACC_HPF_DISABLE 4                   // Values
#define ACC_1HZ         3
#define ACC_2HZ         2
#define ACC_4HZ         1
#define ACC_8HZ         0

/* The type values used in the value() function.
 * The returned value is expressed in mg units and can be negative. 
*/
#define ACC_X_AXIS    1
#define ACC_Y_AXIS    2
#define ACC_Z_AXIS    3

#endif /* __ACC_SENSOR_H__ */
