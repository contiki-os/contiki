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
#include "dev/oscillators.h"
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
/* PDs that may stay on in deep sleep */
#define LOCKABLE_DOMAINS ((uint32_t)(PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH))
/*---------------------------------------------------------------------------*/
/*
 * Don't consider standby mode if the next AON RTC event is scheduled to fire
 * in less than STANDBY_MIN_DURATION rtimer ticks
 */
#define STANDBY_MIN_DURATION (RTIMER_SECOND >> 8)
/*---------------------------------------------------------------------------*/
void
lpm_shutdown(uint32_t wakeup_pin, uint32_t io_pull, uint32_t wake_on)
{
  lpm_registered_module_t *module;
  int i, j;
  uint32_t io_cfg = (IOC_STD_INPUT & ~IOC_IOPULL_M) | io_pull |
    wake_on;

  /* This procedure may not be interrupted */
  ti_lib_int_master_disable();

  /* Disable the RTC */
  ti_lib_aon_rtc_disable();
  ti_lib_aon_rtc_event_clear(AON_RTC_CH0);
  ti_lib_aon_rtc_event_clear(AON_RTC_CH1);
  ti_lib_aon_rtc_event_clear(AON_RTC_CH2);

  /* Reset AON even fabric to default wakeup sources */
  for(i = AON_EVENT_MCU_WU0; i <= AON_EVENT_MCU_WU3; i++) {
    ti_lib_aon_event_mcu_wake_up_set(i, AON_EVENT_NULL);
  }
  for(i = AON_EVENT_AUX_WU0; i <= AON_EVENT_AUX_WU2; i++) {
    ti_lib_aon_event_aux_wake_up_set(i, AON_EVENT_NULL);
  }

  ti_lib_sys_ctrl_aon_sync();

  watchdog_periodic();

  /* fade away....... */
  j = 1000;

  for(i = j; i > 0; --i) {
    leds_on(LEDS_ALL);
    clock_delay_usec(i);
    leds_off(LEDS_ALL);
    clock_delay_usec(j - i);
  }

  leds_off(LEDS_ALL);

  /* Notify all modules that we're shutting down */
  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->shutdown) {
      module->shutdown(LPM_MODE_SHUTDOWN);
    }
  }

  /* Configure the wakeup trigger */
  ti_lib_gpio_dir_mode_set((1 << wakeup_pin), GPIO_DIR_MODE_IN);
  ti_lib_ioc_port_configure_set(wakeup_pin, IOC_PORT_GPIO, io_cfg);

  /* Freeze I/O latches in AON */
  ti_lib_aon_ioc_freeze_enable();

  /* Turn off RFCORE, SERIAL and PERIPH PDs. This will happen immediately */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL |
                               PRCM_DOMAIN_PERIPH);

  oscillators_switch_to_hf_rc();
  oscillators_select_lf_rcosc();

  /* Configure clock sources for MCU and AUX: No clock */
  ti_lib_aon_wuc_mcu_power_down_config(AONWUC_NO_CLOCK);
  ti_lib_aon_wuc_aux_power_down_config(AONWUC_NO_CLOCK);

  /* Disable SRAM and AUX retentions */
  ti_lib_aon_wuc_mcu_sram_config(0);
  ti_lib_aon_wuc_aux_sram_config(false);

  /*
   * Request CPU, SYSBYS and VIMS PD off.
   * This will only happen when the CM3 enters deep sleep
   */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_CPU | PRCM_DOMAIN_VIMS |
                               PRCM_DOMAIN_SYSBUS);

  /* Request JTAG domain power off */
  ti_lib_aon_wuc_jtag_power_off();

  /* Turn off AUX */
  ti_lib_aux_wuc_power_ctrl(AUX_WUC_POWER_OFF);
  ti_lib_aon_wuc_domain_power_down_enable();
  while(ti_lib_aon_wuc_power_status_get() & AONWUC_AUX_POWER_ON);

  /*
   * Request MCU VD power off.
   * This will only happen when the CM3 enters deep sleep
   */
  ti_lib_prcm_mcu_power_off();

  /* Set MCU wakeup to immediate and disable virtual power off */
  ti_lib_aon_wuc_mcu_wake_up_config(MCU_IMM_WAKE_UP);
  ti_lib_aon_wuc_mcu_power_off_config(MCU_VIRT_PWOFF_DISABLE);

  /* Latch the IOs in the padring and enable I/O pad sleep mode */
  ti_lib_pwr_ctrl_io_freeze_enable();

  /* Turn off VIMS cache, CRAM and TRAM - possibly not required */
  ti_lib_prcm_cache_retention_disable();
  ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_OFF);

  /* Enable shutdown and sync AON */
  ti_lib_aon_wuc_shut_down_enable();
  ti_lib_sys_ctrl_aon_sync();

  /* Deep Sleep */
  ti_lib_prcm_deep_sleep();
}
/*---------------------------------------------------------------------------*/
/*
 * Notify all modules that we're back on and rely on them to restore clocks
 * and power domains as required.
 */
static void
wake_up(void)
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
  ti_lib_prcm_cache_retention_enable();

  ti_lib_aon_ioc_freeze_disable();
  ti_lib_sys_ctrl_aon_sync();

  /* Check operating conditions, optimally choose DCDC versus GLDO */
  ti_lib_sys_ctrl_dcdc_voltage_conditional_control();

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
  uint8_t max_pm = LPM_MODE_MAX_SUPPORTED;
  uint8_t module_pm;

  uint32_t domains = LOCKABLE_DOMAINS;

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
     *
     * At this stage, we also collect power domain locks, if any.
     * The argument to PRCMPowerDomainOff() is a bitwise OR, so every time
     * we encounter a lock we just clear the respective bits in the 'domains'
     * variable as required by the lock. In the end the domains variable will
     * just hold whatever has not been cleared
     */
    for(module = list_head(modules_list); module != NULL;
        module = module->next) {
      if(module->shutdown) {
        module->shutdown(max_pm);
      }

      /* Clear the bits specified in the lock */
      domains &= ~module->domain_lock;
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

    /*
     * Before entering Deep Sleep, we must switch off the HF XOSC. The HF XOSC
     * is predominantly controlled by the RF driver. In a build with radio
     * cycling (e.g. ContikiMAC), the RF driver will request the XOSC before
     * using the Freq. Synth, and switch back to the RC when it is about to
     * turn back off.
     *
     * If the radio is on, we won't even reach here, and if it's off the HF
     * clock source should already be the HF RC.
     *
     * Nevertheless, request the switch to the HF RC explicitly here.
     */
    oscillators_switch_to_hf_rc();

    /* Configure clock sources for MCU and AUX: No clock */
    ti_lib_aon_wuc_mcu_power_down_config(AONWUC_NO_CLOCK);
    ti_lib_aon_wuc_aux_power_down_config(AONWUC_NO_CLOCK);

    /* Full RAM retention. */
    ti_lib_aon_wuc_mcu_sram_config(MCU_RAM0_RETENTION | MCU_RAM1_RETENTION |
                                   MCU_RAM2_RETENTION | MCU_RAM3_RETENTION);

    /* Disable retention of AUX RAM */
    ti_lib_aon_wuc_aux_sram_config(false);

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
    while(ti_lib_aon_wuc_power_status_get() & AONWUC_AUX_POWER_ON);

    /* Configure the recharge controller */
    ti_lib_sys_ctrl_set_recharge_before_power_down(false);

    /*
     * If both PERIPH and SERIAL PDs are off, request the uLDO as the power
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
    ti_lib_prcm_cache_retention_disable();
    ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_OFF);

    /* Deep Sleep */
    ti_lib_prcm_deep_sleep();

    /*
     * When we reach here, some interrupt woke us up. The global interrupt
     * flag is off, hence we have a chance to run things here. We will wake up
     * the chip properly, and then we will enable the global interrupt without
     * unpending events so the handlers can fire
     */
    wake_up();

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
lpm_unregister_module(lpm_registered_module_t *module)
{
  list_remove(modules_list, module);
}
/*---------------------------------------------------------------------------*/
void
lpm_init()
{
  list_init(modules_list);
}
/*---------------------------------------------------------------------------*/
void
lpm_pin_set_default_state(uint32_t ioid)
{
  if(ioid == IOID_UNUSED) {
    return;
  }

  ti_lib_ioc_port_configure_set(ioid, IOC_PORT_GPIO, IOC_STD_OUTPUT);
  ti_lib_gpio_dir_mode_set((1 << ioid), GPIO_DIR_MODE_IN);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
