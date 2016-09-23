/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup remote-rtcc
 * @{
 *
 * Driver for the RE-Mote RTCC (Real Time Clock Calendar)
 * @{
 *
 * \file
 * RTCC configuration file
 *
 * \author
 *
 * Antonio Lignan <alinan@zolertia.com>
 * Aitor Mejias <amejias@zolertia.com>
 * Toni Lozano <tlozano@zolertia.com>
 */
/* -------------------------------------------------------------------------- */
#ifndef RTCC_CONFIG_H_
#define RTCC_CONFIG_H_
/* -------------------------------------------------------------------------- */
#include "rtcc.h"
/* -------------------------------------------------------------------------- */
/**
 * \name RTCC configuration macros
 * @{
 */
#define RTCC_SET_DEFAULT_CONFIG       1
#define RTCC_CLEAR_INT_MANUALLY       1
#define RTCC_SET_AUTOCAL              1
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name RTCC default configuration (if enabled by RTCC_SET_DEFAULT_CONFIG)
 * @{
 */
/* Reset values from the Application Manual */
#define RTCC_DEFAULT_STATUS           0x00
#define RTCC_DEFAULT_CTRL1            0x11
#define RTCC_DEFAULT_CTRL2            0x00
#define RTCC_DEFAULT_INTMASK          0xE0
#define RTCC_DEFAULT_SQW              0x26
#define RTCC_DEFAULT_TIMER_CTRL       0x23
/** @} */
/* -------------------------------------------------------------------------- */
/**
 * \name RTCC default configuration structure
 * @{
 */
typedef struct ab080x_register_config {
  uint8_t reg;
  uint8_t val;
} ab080x_register_config_t;
/* -------------------------------------------------------------------------- */
static const ab080x_register_config_t ab080x_default_setting[] =
{
  { (CONFIG_MAP_OFFSET + STATUS_ADDR), RTCC_DEFAULT_STATUS },
  { (CONFIG_MAP_OFFSET + CTRL_1_ADDR), RTCC_DEFAULT_CTRL1 },
  { (CONFIG_MAP_OFFSET + CTRL_2_ADDR), RTCC_DEFAULT_CTRL2 },
  { (CONFIG_MAP_OFFSET + INT_MASK_ADDR), RTCC_DEFAULT_INTMASK },
  { (CONFIG_MAP_OFFSET + SQW_ADDR), RTCC_DEFAULT_SQW },
  { (CONFIG_MAP_OFFSET + TIMER_CONTROL_ADDR), RTCC_DEFAULT_TIMER_CTRL },
};
/** @} */
/* -------------------------------------------------------------------------- */
#endif /* ifndef RTCC_CONFIG_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */