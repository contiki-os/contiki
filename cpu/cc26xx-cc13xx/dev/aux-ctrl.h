/*
 * Copyright (c) 2016, University of Bristol - http://www.bris.ac.uk/
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
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-aux-ctrl CC13xx/CC26xx AUX domain controller
 *
 * CC13xx/CC26xx AUX domain power management controller
 *
 * @{
 *
 * \file
 * Header file for the management of the CC13xx/CC26xx AUX domain
 */
/*---------------------------------------------------------------------------*/
#ifndef AUX_CTRL_H_
#define AUX_CTRL_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "ti-lib.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief The data structure to be used for modules that require access to AUX
 *
 * The clocks field should specify the clocks (within AUX) that your module
 * requires in order to perform its functionality. Those clocks are an ORd
 * value of AUX_WUC_xxxx_CLOCK. For instance, the oscillators module specifies
 * AUX_WUC_OSCCTRL_CLOCK | AUX_WUC_SMPH_CLOCK
 */
typedef struct aux_consumer_module {
  struct aux_consumer_module *next;
  uint32_t clocks;
} aux_consumer_module_t;
/*---------------------------------------------------------------------------*/
/**
 * \brief Register a module that no longer requires access to the AUX power domain
 * \param consumer A pointer to the data structure of your AUX consumer
 *
 * Call this function if you are developing a module that requires access to
 * hardware within the AUX PD. Calling this function will achieve a number of
 * things:
 *
 * - It will power up the AUX PD
 * - It will enable the AUX clocks that you require
 *
 * If you call this function, AUX will stay powered-on and clocked during deep
 * sleep, and retention will be enabled (so that you can e.g. use the sensor
 * controller to monitor peripherals while the main MCU in deep sleep). If you
 * do not need AUX enabled during deep sleep, you must release it by calling
 * aux_ctrl_unregister_consumer()
 *
 * \sa aux_ctrl_unregister_consumer
 */
void aux_ctrl_register_consumer(aux_consumer_module_t *consumer);

/**
 * \brief Deregister a module that no longer requires access to the AUX power domain
 * \param consumer A pointer to the data structure of your AUX consumer
 *
 * When your module is finished using features provided from within the AUX
 * domain, you should call this function to signal that AUX is no longer
 * required, so that the LPM module can power it down in deep sleep. If there
 * are no more registered consumers left, this function will also power down
 * AUX.
 *
 * \sa aux_ctrl_register_consumer
 * \sa aux_ctrl_power_down
 */
void aux_ctrl_unregister_consumer(aux_consumer_module_t *consumer);

/**
 * \brief Power-up the AUX power domain
 *
 * This function will power up the AUX power-domain, but only if there are
 * registered consumers for it. If there are not, the PD will stay off.
 *
 * This function will automatically get called by the LPM module whenever the
 * chip comes out of deep sleep.
 *
 * User applications will normally not need to call this function. if you are
 * developing a user application that requires access, to AUX, you should
 * normally call aux_ctrl_register_consumer(), which will automatically power
 * up AUX for you, if it's not already powered.
 */
void aux_ctrl_power_up(void);

/**
 * \brief Power down the AUX power domain
 * \param force Force the power down irrespective of registered consumers
 *
 * This function will shut down the AUX power-domain.
 *
 * The shutdown is unconditional if force is true. If force is false and there
 * are registered consumers, the power-down will be suppressed.
 *
 * This function will automatically get called by the LPM module whenever the
 * chip tries to enter deep sleep or shuts down.
 *
 * User applications will normally not need to call this function. if you are
 * developing a user application that no longer requires access, to AUX, you
 * should normally simply release it by calling aux_ctrl_unregister_consumer().
 * If no other consumers are using AUX, then the lpm module will shut it down.
 */
void aux_ctrl_power_down(bool force);
/*---------------------------------------------------------------------------*/
#endif /* AUX_CTRL_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
