/*
 * Copyright (c) 2017, Zolertia <http://www.zolertia.com>
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
 * \file
 *         HP206C Barometer and altimeter sensor driver
 * \author
 *         Javi Sanchez <jsanchez@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/* -------------------------------------------------------------------------- */
#ifndef HP206C_H_
#define HP206C_H_
/* -------------------------------------------------------------------------- */
#define HP206C_ADDR              0x76

#define HP206C_SOFT_RST          0xF2
#define HP206C_CONVERT           0xF3

#define HP206C_CVT_4096_PT       0x40
#define HP206C_CVT_2048_PT       0x44
#define HP206C_CVT_1024_PT       0x48
#define HP206C_CVT_512_PT        0x4C
#define HP206C_CVT_256_PT        0x50
#define HP206C_CVT_128_PT        0x54

#define HP206C_CVT_4096_T        0x40
#define HP206C_CVT_2048_T        0x46
#define HP206C_CVT_1024_T        0x4A
#define HP206C_CVT_512_T         0x4E
#define HP206C_CVT_256_T         0x52
#define HP206C_CVT_128_T         0x56

#define HP206C_READ_P            0x30
#define HP206C_READ_A            0x31
#define HP206C_READ_T            0x32
#define HP206C_READ_PT           0x10
#define HP206C_READ_AT           0x11
#define HP206C_READ_CAL          0X28
#define HP206C_WR_REG_MODE       0xC0
#define HP206C_RD_REG_MODE       0x80

#define HP206C_ALT_OFF_LSB       0x00
#define HP206C_ALT_OFF_MSB       0x01
#define HP206C_PA_H_TH_LSB       0x02
#define HP206C_PA_H_TH_MSB       0x03
#define HP206C_PA_M_TH_LSB       0x04
#define HP206C_PA_M_TH_MSB       0x05
#define HP206C_PA_L_TH_LSB       0x06
#define HP206C_PA_L_TH_MSB       0x07
#define HP206C_T_H_TH            0x08
#define HP206C_T_M_TH            0x09
#define HP206C_T_L_TH            0x0A
#define HP206C_INT_EN            0x0B
#define HP206C_INT_CFG           0x0C
#define HP206C_INT_SRC           0X0D
#define HP206C_INT_DIR           0x0E
#define REG_PARA                 0X0F

#define ERR_WR_DEVID_NACK        0x01
#define ERR_RD_DEVID_NACK        0x02
#define ERR_WR_REGADD_NACK       0x04
#define ERR_WR_REGCMD_NACK       0x08
#define ERR_WR_DATA_NACK         0x10
#define ERR_RD_DATA_MISMATCH     0x20

#define T_WIN_EN                 0X01
#define PA_WIN_EN                0X02
#define T_TRAV_EN                0X04
#define PA_TRAV_EN               0X08
#define PA_RDY_EN                0X20
#define T_RDY_EN                 0X10

#define T_WIN_CFG                0X01
#define PA_WIN_CFG               0X02
#define PA_MODE_P                0X00
#define PA_MODE_A                0X40

#define T_TRAV_CFG               0X04

#define HP206C_ACTIVE            0X80

#define HP206C_SOFT_RST_REG      0x06
#define HP206C_RESET_DELAY       400
#define HP206C_CONVERT_DELAY     2500

#define HP206C_ERROR             (-1)
#define HP206C_SUCCESS           0x00
/* -------------------------------------------------------------------------- */
#define HP206C_SENSOR "HP206C Sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor hp206c;
/* -------------------------------------------------------------------------- */
#endif /* ifndef HP206C_H_ */
/**
 * @}
 * @}
 */
 