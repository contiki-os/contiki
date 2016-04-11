/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 * \addtogroup openmote-sensors
 * @{
 *
 * \defgroup openmote-adxl346-sensor ADXL346 acceleration sensor
 * @{
 *
 * \file
 * ADXL346 acceleration sensor driver header file
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef ADXL346_H_
#define ADXL346_H_
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the ADXL346 sensor
 */
void adxl346_init(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Reset the ADXL346 sensor
 */
void adxl346_reset(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Check if the ADXL346 sensor is present
 */
uint8_t adxl346_is_present(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the x-axis from the ADXL346 sensor
 */
uint16_t adxl346_read_x(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the y-axis from the ADXL346 sensor
 */
uint16_t adxl346_read_y(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the z-axis from the ADXL346 sensor
 */
uint16_t adxl346_read_z(void);
/*---------------------------------------------------------------------------*/
#endif /* ADXL346_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
