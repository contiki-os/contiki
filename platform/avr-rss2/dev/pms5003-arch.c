/*
 * Copyright (c) 2017, Peter Sjodin, KTH Royal Institute of Technology
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
 *
 * Author  : Peter Sjodin psj@kth.se
 * Created : 2017-01-06
 */

#include "contiki.h"
#include "i2c.h"
#include "watchdog.h"
#include "dev/pms5003/pms5003.h"
#include "pms5003-arch.h"

static uint8_t standbymode;
/*---------------------------------------------------------------------------*/
/* 
 * Configure low power standby mode (PIN3, SET)
 */
void
pms5003_set_standby_mode(uint8_t mode) {
  SET_PMS_DDR |= (1 << PMS_SET);
  if (mode == STANDBY_MODE_OFF)
    SET_PMS_PORT |= (1 << PMS_SET);
  else if (mode == STANDBY_MODE_ON)
    SET_PMS_PORT &= ~(1 << PMS_SET);
  standbymode = mode;
}
/*---------------------------------------------------------------------------*/
/*
 * Return current standby mode
 */
uint8_t
pms5003_get_standby_mode(void) {
  return standbymode;
}
/*---------------------------------------------------------------------------*/
/*
 * Probe I2C bus for PMS5003 device 
 */
uint8_t
pms5003_i2c_probe(void) {
  watchdog_periodic();
  if(!i2c_start(I2C_PMS5003_ADDR)) {
    i2c_stop();
    i2c_probed |= I2C_PMS5003;
    return 1;

  }
  i2c_probed &= ~I2C_PMS5003;
  return 0;
}
/*---------------------------------------------------------------------------*/
