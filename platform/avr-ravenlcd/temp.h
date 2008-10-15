/*
 *  Copyright (c) 2008  Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      Interface for the onboard temperature sensor.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef __TEMP_H__
#define __TEMP_H__


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "adc.h"

#define PROGMEM_DECLARE(x) x __attribute__((__progmem__))

/** \addtogroup lcd */
/** \{ */
/** Type used with \ref temp_get() to select temperature unit */
typedef enum {
    TEMP_UNIT_CELCIUS,
    TEMP_UNIT_FAHRENHEIT
} temp_unit_t;

/** \name Definition of Port Pin for temp sensor. */
/** \{ */
#define TEMP_PORT       PORTF
#define TEMP_DDR        DDRF
#define TEMP_PIN        PINF
#define TEMP_BIT_IN     4
#define TEMP_BIT_PWR    6
/** \} */

/** \brief          Initialize the temperature sensor
 *
 * \return                              EOF on error
 */
int temp_init(void);


/** \brief          De-initialize the temperature sensor
 *
 */
void temp_deinit(void);

/** \brief          Read current temperature
 *
 * \param[in]		unit			Selected temperature unit (\ref temp_unit_t)
 *
 *
 * \return                              EOF on error
 */
int16_t temp_get(temp_unit_t unit);
/** \} */
#endif /* __TEMP_H__ */
