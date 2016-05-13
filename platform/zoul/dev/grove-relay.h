/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \defgroup zoul-grove-relay Grove Relay actuator
 * @{
 * \file
 *      HLS8-T73 GROVE_RELAY example 
 * \author
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#ifndef GROVE_RELAY_H_
#define GROVE_RELAY_H_
/* -------------------------------------------------------------------------- */
#define GROVE_RELAY_ERROR 	     (-1)
#define GROVE_RELAY_SUCCESS      0
#define GROVE_RELAY       "GROVE_RELAY"
#define GROVE_RELAY_ON           1
#define GROVE_RELAY_OFF          0
/* -------------------------------------------------------------------------- */
#ifdef GROVE_RELAY_CONF_CTRL_PIN
#define GROVE_RELAY_CTRL_PIN         GROVE_RELAY_CONF_CTRL_PIN
#else
#define GROVE_RELAY_CTRL_PIN         3
#endif
#ifdef GROVE_RELAY_CONF_CTRL_PORT
#define GROVE_RELAY_CTRL_PORT        GROVE_RELAY_CONF_CTRL_PORT
#else
#define GROVE_RELAY_CTRL_PORT        GPIO_C_NUM
#endif
/* -------------------------------------------------------------------------- */
//static void grove_relay_configure(void);
/* -------------------------------------------------------------------------- */
//static int grove_relay_set(int state);
/* -------------------------------------------------------------------------- */
#endif /* ifndef GROVE_RELAY_H_ */
/**
 * @}
 * @}
 */
