/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \defgroup cc26xx-lpm CC26xx Low-Power management
 *
 * CC26xx low-power operation
 *
 * @{
 *
 * \file
 * Header file for the management of CC26xx low-power operation
 */
/*---------------------------------------------------------------------------*/
#ifndef LPM_H_
#define LPM_H_
/*---------------------------------------------------------------------------*/
#include "pwr_ctrl.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LPM_MODE_SLEEP         PWRCTRL_ACTIVE
#define LPM_MODE_DEEP_SLEEP    PWRCTRL_POWER_DOWN
#define LPM_MODE_SHUTDOWN      PWRCTRL_SHUTDOWN

#define LPM_MODE_MAX_SUPPORTED LPM_MODE_DEEP_SLEEP
/*---------------------------------------------------------------------------*/
typedef struct lpm_registered_module {
  struct lpm_registered_module *next;
  uint8_t (*request_max_pm)(void);
  void (*shutdown)(uint8_t mode);
  void (*wakeup)(void);
} lpm_registered_module_t;
/*---------------------------------------------------------------------------*/
/**
 * \brief Declare a variable to be used in order to get notifications from LPM
 * \param n the variable name to be declared
 * \param m A pointer to a function which will tell the LPM module the max
 *          PM this module is willing to handle. This function will return
 *          LPM_MODE_SLEEP, LPM_MODE_DEEP_SLEEP etc. The LPM module will ask all
 *          registered modules and will trigger the highest LPM permitted
 * \param s A pointer to a function which will receive a notification just
 *          before entering the low power mode. The callee can prepare for the
 *          imminent LPM state. The argument to this function will be the
 *          upcoming low power mode. This function can e.g. turn off a
 *          peripheral before the LPM module shuts down the power domain.
 * \param w A pointer to a function which will be called just after we have
 *          woken up. This can be used to e.g. turn a peripheral back on. This
 *          function is in charge of turning power domains back on. This
 *          function will normally be called within an interrupt context.
 */
#define LPM_MODULE(n, m, s, w) static lpm_registered_module_t n = \
  { NULL, m, s, w }
/*---------------------------------------------------------------------------*/
/**
 *
 * \brief Data type used to control whether a PD will get shut down when the
 * CM3 drops to deep sleep
 *
 * Modules using these facilities must allocate a variable of this type, but
 * they must not try to manipulate it directly. Instead, the respective
 * functions must be used
 *
 * \sa lpm_pd_lock_obtain(), lpm_pd_lock_release()
 */
typedef struct lpm_power_domain_lock {
  struct lpm_power_domain_lock *next;
  uint32_t domains;
} lpm_power_domain_lock_t;
/*---------------------------------------------------------------------------*/
/**
 * \brief Prohibit a PD from turning off in standby mode
 * \param lock A pointer to a lpm_power_domain_lock_t
 * \param domains Bitwise OR from PRCM_DOMAIN_PERIPH, PRCM_DOMAIN_SERIAL
 *
 * The caller is responsible for allocating lpm_power_domain_lock_t
 *
 * Only the domains listed above can be locked / released, but a single lock
 * can be used for multiple domains
 */
void lpm_pd_lock_obtain(lpm_power_domain_lock_t *lock, uint32_t domains);

/**
 * \brief Permit a PD to turn off in standby mode
 * \param pd A pointer to a previously used lock
 *
 * \sa lpm_pd_lock_obtain()
 */
void lpm_pd_lock_release(lpm_power_domain_lock_t *pd);

/**
 * \brief Drop the cortex to sleep / deep sleep and shut down peripherals
 *
 * Whether the cortex will drop to sleep or deep sleep is configurable. The
 * exact peripherals which will be shut down is also configurable
 */
void lpm_drop(void);

/**
 * \brief Enter sleep mode
 */
void lpm_sleep(void);

/**
 * \brief Put the chip in shutdown power mode
 * \param wakeup_pin The GPIO pin which will wake us up. Must be IOID_0 etc...
 */
void lpm_shutdown(uint32_t wakeup_pin);

/**
 * \brief Wake up from sleep mode
 *
 * This function must be called at the start of any interrupt context which
 * may bring us out of sleep. Current interrupts do this already, but make sure
 * to do the same when adding new ISRs
 */
void lpm_wake_up(void);

/**
 * \brief Register a module for LPM notifications.
 * \param module A pointer to the data structure with the module definition
 *
 * When the LPM module is about to drop to some low power mode, it will first
 * notify all modules about this.
 *
 * This function must not be called before the module has been initialised
 * with lpm_init(). The code does not perform checks: This is the caller's
 * responsibility.
 */
void lpm_register_module(lpm_registered_module_t *module);

/**
 * \brief Initialise the low-power mode management module
 */
void lpm_init(void);
/*---------------------------------------------------------------------------*/
#endif /* LPM_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
