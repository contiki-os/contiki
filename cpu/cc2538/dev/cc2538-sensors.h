/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
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
 * \defgroup cc2538-sensors CC2538 Built-In Sensors
 *
 * Module controlling sensors on the CC2538 SoC (Tmp and VDD3)
 * @{
 *
 * \file
 * Generic header usable by all CC2538 sensor drivers
 */
/*---------------------------------------------------------------------------*/
#ifndef CC2538_SENSORS_H_
#define CC2538_SENSORS_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
#include "dev/cc2538-temp-sensor.h"
#include "dev/vdd3-sensor.h"
/*---------------------------------------------------------------------------*/
/**
 * \name CC2538 sensor constants
 *
 * These constants are used by various sensors on the CC2538. They can be used
 * to differentiate between raw and converted readings, to configure ADC
 * decimation rate (where applicable).
 * @{
 */
#define CC2538_SENSORS_VALUE_TYPE_RAW         0 /**< Request the raw reading */
#define CC2538_SENSORS_VALUE_TYPE_CONVERTED   1 /**< Request the converted reading */

#define CC2538_SENSORS_ERROR         0x80000000 /**< Generic Error */
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* CC2538_SENSORS_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
