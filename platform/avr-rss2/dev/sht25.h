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
/**
 * \file  
 *         SHT25 temperature and humidity sensor driver
 * \author from 
 *         Antonio Lignan <alinan@zolertia.com>
 * Authors : Joel Okello <okellojoelacaye@gmail.com>, Mary Nsabagwa <mnsabagwa@cit.ac.ug>
 */
#include "lib/sensors.h"
#include "Typedefs.h"


typedef enum{
 TRIG_T_MEASUREMENT_HM = 0xE3, // command trig. temp meas. hold master
 TRIG_RH_MEASUREMENT_HM = 0xE5, // command trig. humidity meas. hold master
 TRIG_T_MEASUREMENT_POLL = 0xF3, // command trig. temp meas. no hold master
 TRIG_RH_MEASUREMENT_POLL = 0xF5, // command trig. humidity meas. no hold master
 USER_REG_W = 0xE6, // command writing user register
 USER_REG_R = 0xE7, // command reading user register
 SOFT_RESET = 0xFE // command soft reset
}etSHT2xCommand;
typedef enum {
 SHT2x_RES_12_14BIT = 0x00, // RH=12bit, T=14bit
 SHT2x_RES_8_12BIT = 0x01, // RH= 8bit, T=12bit
 SHT2x_RES_10_13BIT = 0x80, // RH=10bit, T=13bit
 SHT2x_RES_11_11BIT = 0x81, // RH=11bit, T=11bit
 SHT2x_RES_MASK = 0x81 // Mask for res. bits (7,0) in user reg.
} etSHT2xResolution;
typedef enum {
 SHT2x_EOB_ON = 0x40, // end of battery
 SHT2x_EOB_MASK = 0x40, // Mask for EOB bit(6) in user reg.
} etSHT2xEob;
typedef enum {
 SHT2x_HEATER_ON = 0x04, // heater on
 SHT2x_HEATER_OFF = 0x00, // heater off
 SHT2x_HEATER_MASK = 0x04, // Mask for Heater bit(2) in user reg.
} etSHT2xHeater;
// measurement signal selection
typedef enum{
 HUMIDITY,
 TEMP
}etSHT2xMeasureType;
typedef enum{
 I2C_ADR_W = 128, // sensor I2C address + write bit
 I2C_ADR_R = 129 // sensor I2C address + read bit
}etI2cHeader;
//==============================================================================
u8t SHT2x_CheckCrc(u8t data[], u8t nbrOfBytes, u8t checksum);
//==============================================================================
// calculates checksum for n bytes of data and compares it with expected
// checksum
// input: data[] checksum is built based on this data
// nbrOfBytes checksum is built for n bytes of data
// checksum expected checksum
// return: error: CHECKSUM_ERROR = checksum does not match
// 0 = checksum matches
//==============================================================================
u8t SHT2x_ReadUserRegister(u8t *pRegisterValue);
//==============================================================================
// reads the SHT2x user register (8bit)
// input : -
// output: *pRegisterValue
// return: error
//==============================================================================
u8t SHT2x_WriteUserRegister(u8t *pRegisterValue);
//==============================================================================
// writes the SHT2x user register (8bit)
// input : *pRegisterValue
// output: -
// return: error
//==============================================================================
u8t SHT2x_MeasurePoll(etSHT2xMeasureType eSHT2xMeasureType, nt16 *pMeasurand);
//==============================================================================
// measures humidity or temperature. This function polls every 10ms until
// measurement is ready.
// input: eSHT2xMeasureType
// output: *pMeasurand: humidity / temperature as raw value
// return: error
// note: timing for timeout may be changed
//==============================================================================
u8t SHT2x_MeasureHM(etSHT2xMeasureType eSHT2xMeasureType, nt16 *pMeasurand);
//==============================================================================
// measures humidity or temperature. This function waits for a hold master until
// measurement is ready or a timeout occurred.
// input: eSHT2xMeasureType
// output: *pMeasurand: humidity / temperature as raw value
// return: error
// note: timing for timeout may be changed
//==============================================================================
u8t SHT2x_SoftReset();
//==============================================================================
// performs a reset
// input: -
// output: -
// return: error
//==============================================================================
float SHT2x_CalcRH(u16t u16sRH);
//==============================================================================
// calculates the relative humidity
// input: sRH: humidity raw value (16bit scaled)
// return: pHumidity relative humidity [%RH]
//==============================================================================
float SHT2x_CalcTemperatureC(u16t u16sT);
//==============================================================================
// calculates temperature
// input: sT: temperature raw value (16bit scaled)
// return: temperature [°C]
//==============================================================================
u8t SHT2x_GetSerialNumber(u8t u8SerialNumber[]);
//==============================================================================
// gets serial number of SHT2x according application note "How To
// Read-Out the Serial Number"
// note: readout of this function is not CRC checked
//
// input: -
// output: u8SerialNumber: Array of 8 bytes (64Bits)
// MSB LSB
// u8SerialNumber[7] u8SerialNumber[0]
// SNA_1 SNA_0 SNB_3 SNB_2 SNB_1 SNB_0 SNC_1 SNC_0
// return: error

uint8_t missing_t_value();
uint8_t missing_rh_value();

#ifndef SHT25_H_
#define SHT25_H_

/* -------------------------------------------------------------------------- */
#define SHT25_ADDR            0x40
#define SHT25_TEMP_HOLD       0xE3
#define SHT25_HUM_HOLD        0xE5
#define SHT25_TEMP_NO_HOLD    0xF3
#define SHT25_HUM_NO_HOLD     0xF5
#define SHT2X_UREG_WRITE      0xE6
#define SHT2X_UREG_READ       0xE7
#define SHT2X_SOFT_RESET      0XFE
#define SHT2X_NULL            0x00
#define SHT25_STATUS_BITS_MASK  0x0003
/* -------------------------------------------------------------------------- */
#define SHT2X_RES_14T_12RH    0x00
#define SHT2X_RES_12T_08RH    0x01
#define SHT2X_RES_13T_10RH    0x80
#define SHT2X_RES_11T_11RH    0x81
#define SHT2X_HEATER_ON       0x04
#define SHT2X_HEATER_OFF      0x00
#define SHT2X_OTP_RELOAD_EN   0x00
#define SHT2X_OTP_RELOAD_DIS  0x02
#define SHT25_RESOLUTION        0x02
/* -------------------------------------------------------------------------- */

#define SHT25_VOLTAGE_ALARM     0x01
#define SHT25_SUCCESS           0x00
#define SHT2X_RES_14T_12RH      0x00
#define SHT25_SOFT_RESET        0x01
#define SHT2X_RES_12T_08RH      0x01
#define SHT2X_RES_13T_10RH      0x80
#define SHT2X_RES_11T_11RH      0x81
#define SHT2X_HEATER_ON         0x04
#define SHT2X_HEATER_OFF        0x00
#define SHT2X_OTP_RELOAD_EN     0x00
#define SHT25_RESET_DELAY       15000
#define SHT2X_OTP_RELOAD_DIS    0x02
#define SHT2x_LOW_VOLTAGE_MASK  0x40
#define SHT2x_LOW_VOLTAGE_SHIFT 0x06
#define SHT25_ACTIVE            SENSORS_ACTIVE

#define SHT25_VAL_TEMP        SHT25_TEMP_HOLD
#define SHT25_VAL_HUM         SHT25_HUM_HOLD
#define SHT25_ERROR           -1
/* -------------------------------------------------------------------------- */
#define SHT25_SENSOR "SHT25 Sensor"

/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor sht25_sensor;
/* -------------------------------------------------------------------------- */
//static float value1(int type);
#endif /* ifndef SHT25_H_ */
