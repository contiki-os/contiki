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
 * \addtogroup cc26xx-lpm
 * @{
 *
 * Implementation of CC26xx low-power operation functionality
 *
 * @{
 *
 * \file
 * Driver for CC26xx's low-power operation
 */
/*---------------------------------------------------------------------------*/
#include "prcm.h"
#include "contiki-conf.h"
#include "ti-lib.h"
#include "lpm.h"
#include "sys/energest.h"
#include "lib/list.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/cc26xx-rtc.h"
/*---------------------------------------------------------------------------*/
#if ENERGEST_CONF_ON
static unsigned long irq_energest = 0;

#define ENERGEST_IRQ_SAVE(a) do { \
    a = energest_type_time(ENERGEST_TYPE_IRQ); } while(0)
#define ENERGEST_IRQ_RESTORE(a) do { \
    energest_type_set(ENERGEST_TYPE_IRQ, a); } while(0)
#else
#define ENERGEST_IRQ_SAVE(a) do {} while(0)
#define ENERGEST_IRQ_RESTORE(a) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
LIST(modules_list);
/*---------------------------------------------------------------------------*/
/* Control what power domains we are allow to run under what mode */
LIST(power_domain_locks_list);

/* PDs that may stay on in deep sleep */
#define LOCKABLE_DOMAINS ((uint32_t)(PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH))
/*---------------------------------------------------------------------------*/
/*
 * Don't consider standby mode if the next AON RTC event is scheduled to fire
 * in less than STANDBY_MIN_DURATION rtimer ticks
 */
#define STANDBY_MIN_DURATION (RTIMER_SECOND >> 8)
/*---------------------------------------------------------------------------*/
/* Variables used by the power on/off (Power mode: SHUTDOWN) functionality */
static uint8_t shutdown_requested;
static uint32_t pin;
/*---------------------------------------------------------------------------*/
void
lpm_pd_lock_obtain(lpm_power_domain_lock_t *lock, uint32_t domains)
{
  /* We only accept locks for specific PDs */
  domains &= LOCKABLE_DOMAINS;

  if(domains == 0) {
    return;
  }

  lock->domains = domains;

  list_add(power_domain_locks_list, lock);
}
/*---------------------------------------------------------------------------*/
void
lpm_pd_lock_release(lpm_power_domain_lock_t *lock)
{
  lock->domains = 0;

  list_remove(power_domain_locks_list, lock);
}
/*---------------------------------------------------------------------------*/
void
lpm_shutdown(uint32_t wakeup_pin)
{
  shutdown_requested = 1;
  pin = wakeup_pin;
}
/*---------------------------------------------------------------------------*/
static void
shutdown_now(void)
{
  lpm_registered_module_t *module;
  int i;
  rtimer_clock_t t0;
  uint32_t io_cfg = (IOC_STD_INPUT & ~IOC_IOPULL_M) | IOC_IOPULL_UP |
                     IOC_WAKE_ON_LOW;

  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->shutdown) {
      module->shutdown(LPM_MODE_SHUTDOWN);
    }
  }

  leds_off(LEDS_ALL);

  for(i = 0; i < 5; i++) {
    t0 = RTIMER_NOW();
    leds_toggle(LEDS_ALL);
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (RTIMER_SECOND >> 3))));
  }

  leds_off(LEDS_ALL);

  ti_lib_gpio_dir_mode_set((1 << pin), GPIO_DIR_MODE_IN);
  ti_lib_ioc_port_configure_set(pin, IOC_PORT_GPIO, io_cfg);

  ti_lib_pwr_ctrl_state_set(LPM_MODE_SHUTDOWN);
}
/*---------------------------------------------------------------------------*/
/*
 * We'll get called on three occasions:
 * - While running
 * - While sleeping
 * - While deep sleeping
 *
 * For the former two, we don't need to do anything. For the latter, we
 * notify all modules that we're back on and rely on them to restore clocks
 * and power domains as required.
 */
void
lpm_wake_up()
{
  lpm_registered_module_t *module;

  /* Remember IRQ energest for next pass */
  ENERGEST_IRQ_SAVE(irq_energest);
  ENERGEST_ON(ENERGEST_TYPE_CPU);
  ENERGEST_OFF(ENERGEST_TYPE_LPM);

  /* Sync so that we get the latest values before adjusting recharge settings */
  ti_lib_sys_ctrl_aon_sync();

  /* Adjust recharge settings */
  ti_lib_sys_ctrl_adjust_recharge_after_power_down();

  /*
   * Release the request to the uLDO
   * This is likely not required, since the switch to GLDO/DCDC is automatic
   * when coming back from deep sleep
   */
  ti_lib_prcm_mcu_uldo_configure(false);

  /* Turn on cache again */
  ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_ENABLED);
  ti_lib_prcm_retention_enable(PRCM_DOMAIN_VIMS);

  ti_lib_aon_ioc_freeze_disable();
  ti_lib_sys_ctrl_aon_sync();

  /* Power up AUX and allow it to go to sleep */
  ti_lib_aon_wuc_aux_wakeup_event(AONWUC_AUX_ALLOW_SLEEP);

  /* Notify all registered modules that we've just woken up */
  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->wakeup) {
      module->wakeup();
    }
  }
}
/*---------------------------------------------------------------------------*/
void
lpm_drop()
{
  lpm_registered_module_t *module;
  lpm_power_domain_lock_t *lock;
  uint8_t max_pm = LPM_MODE_MAX_SUPPORTED;
  uint8_t module_pm;

  uint32_t domains = LOCKABLE_DOMAINS;

  if(shutdown_requested) {
    shutdown_now();
  }

  if(RTIMER_CLOCK_LT(cc26xx_rtc_get_next_trigger(),
                     RTIMER_NOW() + STANDBY_MIN_DURATION)) {
    lpm_sleep();
    return;
  }

  /* Collect max allowed PM permission from interested modules */
  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->request_max_pm) {
      module_pm = module->request_max_pm();
      if(module_pm < max_pm) {
        max_pm = module_pm;
      }
    }
  }

  /* Check if any events fired during this process. Last chance to abort */
  if(process_nevents()) {
    return;
  }

  /* Drop */
  if(max_pm == LPM_MODE_SLEEP) {
    lpm_sleep();
  } else {
    /* Critical. Don't get interrupted! */
    ti_lib_int_master_disable();

    /*
     * Notify all registered modules that we are dropping to mode X. We do not
     * need to do this for simple sleep.
     *
     * This is a chance for modules to delay us a little bit until an ongoing
     * operation has finished (e.g. uart TX) or to configure themselves for
     * deep sleep.
     */
    for(module = list_head(modules_list); module != NULL;
        module = module->next) {
      if(module->shutdown) {
        module->shutdown(max_pm);
      }
    }

    /*
     * Iterate PD locks to see what we can and cannot turn off.
     *
     * The argument to PRCMPowerDomainOff() is a bitwise OR, so every time
     * we encounter a lock we just clear the respective bits  in the 'domains'
     * variable as required by the lock. In the end the domains variable will
     * just hold whatever has not been cleared
     */
    for(lock = list_head(power_domain_locks_list); lock != NULL;
        lock = lock->next) {
      /* Clear the bits specified in the lock */
      domains &= ~lock->domains;
    }

    /* Pat the dog: We don't want it to shout right after we wake up */
    watchdog_periodic();

    /* Clear unacceptable bits, just in case a lock provided a bad value */
    domains &= LOCKABLE_DOMAINS;

    /*
     * Freeze the IOs on the boundary between MCU and AON. We only do this if
     * PERIPH is not needed
     */
    if(domains & PRCM_DOMAIN_PERIPH) {
      ti_lib_aon_ioc_freeze_enable();
    }

    /*
     * Among LOCKABLE_DOMAINS, turn off those that are not locked
     *
     * If domains is != 0, pass it as-is
     */
    if(domains) {
      ti_lib_prcm_power_domain_off(domains);
    }

    /* Configure clock sources for MCU and AUX: No clock */
    ti_lib_aon_wuc_mcu_power_down_config(AONWUC_NO_CLOCK);
    ti_lib_aon_wuc_aux_power_down_config(AONWUC_NO_CLOCK);

    /* Full RAM retention. */
    ti_lib_aon_wuc_mcu_sram_config(MCU_RAM0_RETENTION | MCU_RAM1_RETENTION |
                                   MCU_RAM2_RETENTION | MCU_RAM3_RETENTION);

    /* Enable retention on the CPU domain */
    ti_lib_prcm_retention_enable(PRCM_DOMAIN_CPU);

    /* Disable retention of AUX RAM */
    ti_lib_aon_wuc_aux_sram_config(false);

    /* Disable retention in the RFCORE RAM */
    HWREG(PRCM_BASE + PRCM_O_RAMRETEN) &= ~PRCM_RAMRETEN_RFC;

    /* Disable retention of VIMS RAM (TRAM and CRAM) */
    //TODO: This can probably be removed, we are calling ti_lib_prcm_retention_disable(PRCM_DOMAIN_VIMS); further down
    HWREG(PRCM_BASE + PRCM_O_RAMRETEN) &= ~PRCM_RAMRETEN_VIMS_M;

    /*
     * Always turn off RFCORE, CPU, SYSBUS and VIMS. RFCORE should be off
     * already
     */
    ti_lib_prcm_power_domain_off(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_CPU |
                                 PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS);

    /* Request JTAG domain power off */
    ti_lib_aon_wuc_jtag_power_off();

    /* Turn off AUX */
    ti_lib_aux_wuc_power_ctrl(AUX_WUC_POWER_OFF);
    ti_lib_aon_wuc_domain_power_down_enable();
    while(ti_lib_aon_wuc_power_status() & AONWUC_AUX_POWER_ON);

    /* Configure the recharge controller */
    ti_lib_sys_ctrl_set_recharge_before_power_down(false);

    /*
     * If both PERIPH and SERIAL PDs are off, request the uLDO for as the power
     * source while in deep sleep.
     */
    if(domains == LOCKABLE_DOMAINS) {
      ti_lib_pwr_ctrl_source_set(PWRCTRL_PWRSRC_ULDO);
    }

    /* We are only interested in IRQ energest while idle or in LPM */
    ENERGEST_IRQ_RESTORE(irq_energest);
    ENERGEST_OFF(ENERGEST_TYPE_CPU);
    ENERGEST_ON(ENERGEST_TYPE_LPM);

    /* Sync the AON interface to ensure all writes have gone through. */
    ti_lib_sys_ctrl_aon_sync();

    /*
     * Explicitly turn off VIMS cache, CRAM and TRAM. Needed because of
     * retention mismatch between VIMS logic and cache. We wait to do this
     * until right before deep sleep to be able to use the cache for as long
     * as possible.
     */
    ti_lib_prcm_retention_disable(PRCM_DOMAIN_VIMS);
    ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_OFF);

    /* Deep Sleep */
    ti_lib_prcm_deep_sleep();

    /*
     * When we reach here, some interrupt woke us up. The global interrupt
     * flag is off, hence we have a chance to run things here. We will wake up
     * the chip properly, and then we will enable the global interrupt without
     * unpending events so the handlers can fire
     */
    lpm_wake_up();

    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
void
lpm_sleep(void)
{
  ENERGEST_OFF(ENERGEST_TYPE_CPU);
  ENERGEST_ON(ENERGEST_TYPE_LPM);

  /* We are only interested in IRQ energest while idle or in LPM */
  ENERGEST_IRQ_RESTORE(irq_energest);

  /* Just to be on the safe side, explicitly disable Deep Sleep */
  HWREG(NVIC_SYS_CTRL) &= ~(NVIC_SYS_CTRL_SLEEPDEEP);

  ti_lib_prcm_sleep();

  /* Remember IRQ energest for next pass */
  ENERGEST_IRQ_SAVE(irq_energest);

  ENERGEST_ON(ENERGEST_TYPE_CPU);
  ENERGEST_OFF(ENERGEST_TYPE_LPM);
}
/*---------------------------------------------------------------------------*/
void
lpm_register_module(lpm_registered_module_t *module)
{
  list_add(modules_list, module);
}
/*---------------------------------------------------------------------------*/
void
lpm_init()
{
  list_init(modules_list);
  list_init(power_domain_locks_list);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
