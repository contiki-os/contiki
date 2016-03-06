/*
 * Copyright (c) 2015, Yanzi Networks AB.
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup ipso-objects An implementation of IPSO Objects
 * @{
 *
 * This application is an implementation of IPSO Objects for
 * OMA Lightweight M2M.
 */

/**
 * \file
 *         Header file for the Contiki IPSO Objects for OMA LWM2M
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#ifndef IPSO_OBJECTS_H_
#define IPSO_OBJECTS_H_

#include "contiki-conf.h"

void ipso_temperature_init(void);
void ipso_button_init(void);
void ipso_light_control_init(void);
void ipso_leds_control_init(void);

/* the init function to register the IPSO objects */
void ipso_objects_init(void);

struct ipso_objects_actuator {
  /**
   * \brief       Initialize the driver.
   */
  void (* init)(void);

  /**
   * \brief       Check if the actuator is on or off.
   *
   * \return      Zero if the actuator is off and non-zero otherwise.
   */
  int  (* is_on)(void);

  /**
   * \brief       Set the actuator to on or off.
   *
   * \param onoroff Zero to set the actuator to off and non-zero otherwise.
   * \return      Zero if ok and a non-zero error code otherwise.
   */
  int  (* set_on)(int onoroff);

  /**
   * \brief       Set the actuator to on or off.
   *
   * \param onoroff Zero to set the actuator to off and non-zero otherwise.
   * \return      Zero if ok and a non-zero error code otherwise.
   */
  int  (* get_dim_level)(void);

  /**
   * \brief       Set the dim level of the actuator.
   *
   * \param level The dim level between 0% and 100%.
   * \return      Zero if ok and a non-zero error code otherwise.
   */
  int  (* set_dim_level)(int level);
};

struct ipso_objects_sensor {
  /**
   * \brief       Initialize the driver.
   */
  void     (* init)(void);

  /**
   * \brief       Read the sensor value in 1/1000 units.
   *
   * \param value A pointer to the variable to hold the sensor value.
   * \return      Zero if ok and a non-zero error code otherwise.
   */
  int (* read_value)(int32_t *value);
};

#endif /* IPSO_OBJECTS_H_ */
/**
 * @}
 * @}
 */
