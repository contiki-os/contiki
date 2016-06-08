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
 * Implementation of CC13xx/CC26xx low-power operation functionality
 *
 * @{
 *
 * \file
 * Driver for CC13xx/CC26xx low-power operation
 */
/*---------------------------------------------------------------------------*/
#include "prcm.h"
#include "contiki-conf.h"
#include "ti-lib.h"
#include "lpm.h"
#include "sys/energest.h"
#include "lib/list.h"
#include "dev/aux-ctrl.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/soc-rtc.h"
#include "dev/oscillators.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
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
#define STANDBY_MIN_DURATION  (RTIMER_SECOND >> 11)
#define MINIMAL_SAFE_SCHEDUAL 8u
#define MAX_SLEEP_TIME        RTIMER_SECOND
#define DEFAULT_SLEEP_TIME    RTIMER_SECOND
/*---------------------------------------------------------------------------*/
#define CLK_TO_RT(c) ((c) * (RTIMER_SECOND / CLOCK_SECOND))
/*---------------------------------------------------------------------------*/
/* Prototype of a function in clock.c. Called every time we come out of DS */
void clock_update(void);
/*---------------------------------------------------------------------------*/
void
lpm_shutdown(uint32_t wakeup_pin, uint32_t io_pull, uint32_t wake_on)
{
  lpm_registered_module_t *module;
  int i;
  uint32_t io_cfg = (IOC_STD_INPUT & ~IOC_IOPULL_M) | io_pull | wake_on;
  aux_consumer_module_t aux = { .clocks = AUX_WUC_OSCCTRL_CLOCK };

  /* This procedure may not be interrupted */
  ti_lib_int_master_disable();

  /* Disable the RTC */
  ti_lib_aon_rtc_disable();
  ti_lib_aon_rtc_event_clear(AON_RTC_CH0);
  ti_lib_aon_rtc_event_clear(AON_RTC_CH1);
  ti_lib_aon_rtc_event_clear(AON_RTC_CH2);

  /* Reset AON even fabric to default wakeup sources */
  for(i = AON_EVENT_MCU_WU0; i <= AON_EVENT_MCU_WU3; i++) {
    ti_lib_aon_event_mcu_wake_up_set(i, AON_EVENT_NONE);
  }
  for(i = AON_EVENT_AUX_WU0; i <= AON_EVENT_AUX_WU2; i++) {
    ti_lib_aon_event_aux_wake_up_set(i, AON_EVENT_NONE);
  }

  ti_lib_sys_ctrl_aon_sync();

  watchdog_periodic();

  /* Notify all modules that we're shutting down */
  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->shutdown) {
      module->shutdown(LPM_MODE_SHUTDOWN);
    }
  }

  /* Configure the wakeup trigger */
  if(wakeup_pin != IOID_UNUSED) {
    ti_lib_gpio_dir_mode_set((1 << wakeup_pin), GPIO_DIR_MODE_IN);
    ti_lib_ioc_port_configure_set(wakeup_pin, IOC_PORT_GPIO, io_cfg);
  }

  /* Freeze I/O latches in AON */
  ti_lib_aon_ioc_freeze_enable();

  /* Turn off RFCORE, SERIAL and PERIPH PDs. This will happen immediately */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL |
                               PRCM_DOMAIN_PERIPH);

  /* Register an aux-ctrl consumer to avoid powercycling AUX twice in a row */
  aux_ctrl_register_consumer(&aux);
  oscillators_switch_to_hf_rc();
  oscillators_select_lf_rcosc();

  /* Configure clock sources for MCU: No clock */
  ti_lib_aon_wuc_mcu_power_down_config(AONWUC_NO_CLOCK);

  /* Disable SRAM retention */
  ti_lib_aon_wuc_mcu_sram_config(0);

  /*
   * Request CPU, SYSBYS and VIMS PD off.
   * This will only happen when the CM3 enters deep sleep
   */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_CPU | PRCM_DOMAIN_VIMS |
                               PRCM_DOMAIN_SYSBUS);

  /* Request JTAG domain power off */
  ti_lib_aon_wuc_jtag_power_off();

  /* Turn off AUX */
  aux_ctrl_power_down(true);
  ti_lib_aon_wuc_domain_power_down_enable();

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
  ENERGEST_SWITCH(ENERGEST_TYPE_LPM, ENERGEST_TYPE_CPU);

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

  /* Fire up AUX is the user has requested this */
  aux_ctrl_power_up();

  /*
   * We may or may not have been woken up by an AON RTC tick. If not, we need
   * to adjust our software tick counter
   */
  clock_update();

  watchdog_periodic();

  /* Notify all registered modules that we've just woken up */
  for(module = list_head(modules_list); module != NULL;
      module = module->next) {
    if(module->wakeup) {
      module->wakeup();
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
deep_sleep(void)
{
  uint32_t domains = LOCKABLE_DOMAINS;
  lpm_registered_module_t *module;

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
      module->shutdown(LPM_MODE_DEEP_SLEEP);
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

  /* Shut Down the AUX if the user application is not using it */
  aux_ctrl_power_down(false);

  /* Configure clock sources for MCU: No clock */
  ti_lib_aon_wuc_mcu_power_down_config(AONWUC_NO_CLOCK);

  /* Full RAM retention. */
  ti_lib_aon_wuc_mcu_sram_config(MCU_RAM0_RETENTION | MCU_RAM1_RETENTION |
                                 MCU_RAM2_RETENTION | MCU_RAM3_RETENTION);

  /*
   * Always turn off RFCORE, CPU, SYSBUS and VIMS. RFCORE should be off
   * already
   */
  ti_lib_prcm_power_domain_off(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_CPU |
                               PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS);

  /* Request JTAG domain power off */
  ti_lib_aon_wuc_jtag_power_off();

  /* Allow MCU and AUX powerdown */
  ti_lib_aon_wuc_domain_power_down_enable();

  /* Configure the recharge controller */
  ti_lib_sys_ctrl_set_recharge_before_power_down(XOSC_IN_HIGH_POWER_MODE);

  /*
   * If both PERIPH and SERIAL PDs are off, request the uLDO as the power
   * source while in deep sleep.
   */
  if(domains == LOCKABLE_DOMAINS) {
    ti_lib_pwr_ctrl_source_set(PWRCTRL_PWRSRC_ULDO);
  }

  /* We are only interested in IRQ energest while idle or in LPM */
  ENERGEST_IRQ_RESTORE(irq_energest);
  ENERGEST_SWITCH(ENERGEST_TYPE_CPU, ENERGEST_TYPE_LPM);

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
}
/*---------------------------------------------------------------------------*/
static void
safe_schedule_rtimer(rtimer_clock_t time, rtimer_clock_t now, int pm)
{
  rtimer_clock_t min_sleep;
  rtimer_clock_t max_sleep;

  min_sleep = now + MINIMAL_SAFE_SCHEDUAL;
  max_sleep = now + MAX_SLEEP_TIME;

  if(RTIMER_CLOCK_LT(time, min_sleep)) {
    /* ensure that we schedule sleep a minimal number of ticks into the
       future */
    soc_rtc_schedule_one_shot(AON_RTC_CH1, min_sleep);
  } else if((pm == LPM_MODE_SLEEP) && RTIMER_CLOCK_LT(max_sleep, time)) {
    /* if max_pm is LPM_MODE_SLEEP, we could trigger the watchdog if we slept
       for too long. */
    soc_rtc_schedule_one_shot(AON_RTC_CH1, max_sleep);
  } else {
    soc_rtc_schedule_one_shot(AON_RTC_CH1, time);
  }
}
/*---------------------------------------------------------------------------*/
static int
setup_sleep_mode(void)
{
  rtimer_clock_t et_distance = 0;

  lpm_registered_module_t *module;
  int max_pm;
  int module_pm;
  int etimer_is_pending;
  rtimer_clock_t now;
  rtimer_clock_t et_time;
  rtimer_clock_t next_trig;

  max_pm = LPM_MODE_MAX_SUPPORTED;
  now = RTIMER_NOW();

  if((LPM_MODE_MAX_SUPPORTED == LPM_MODE_AWAKE) || process_nevents()) {
    return LPM_MODE_AWAKE;
  }

  etimer_is_pending = etimer_pending();

  if(etimer_is_pending) {
    et_distance = CLK_TO_RT(etimer_next_expiration_time() - clock_time());

    if(RTIMER_CLOCK_LT(et_distance, 1)) {
      /* there is an etimer which is already expired; we shouldn't go to
         sleep at all */
      return LPM_MODE_AWAKE;
    }
  }

  next_trig = soc_rtc_get_next_trigger();
  if(RTIMER_CLOCK_LT(next_trig, now + STANDBY_MIN_DURATION)) {
    return LPM_MODE_SLEEP;
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

  /* Reschedule AON RTC CH1 to fire just in time for the next etimer event */
  if(etimer_is_pending) {
    et_time = soc_rtc_last_isr_time() + et_distance;

    safe_schedule_rtimer(et_time, now, max_pm);
  } else {
    /* set a maximal sleep period if no etimers are queued */
    soc_rtc_schedule_one_shot(AON_RTC_CH1, now + DEFAULT_SLEEP_TIME);
  }

  return max_pm;
}
/*---------------------------------------------------------------------------*/
void
lpm_drop()
{
  uint8_t max_pm;

  /* Critical. Don't get interrupted! */
  ti_lib_int_master_disable();

  max_pm = setup_sleep_mode();

  /* Drop */
  if(max_pm == LPM_MODE_SLEEP) {
    lpm_sleep();
  } else if(max_pm == LPM_MODE_DEEP_SLEEP) {
    deep_sleep();
  }

  ti_lib_int_master_enable();
}
/*---------------------------------------------------------------------------*/
void
lpm_sleep(void)
{
  ENERGEST_SWITCH(ENERGEST_TYPE_CPU, ENERGEST_TYPE_LPM);

  /* We are only interested in IRQ energest while idle or in LPM */
  ENERGEST_IRQ_RESTORE(irq_energest);

  /* Just to be on the safe side, explicitly disable Deep Sleep */
  HWREG(NVIC_SYS_CTRL) &= ~(NVIC_SYS_CTRL_SLEEPDEEP);

  ti_lib_prcm_sleep();

  /* Remember IRQ energest for next pass */
  ENERGEST_IRQ_SAVE(irq_energest);

  ENERGEST_SWITCH(ENERGEST_TYPE_LPM, ENERGEST_TYPE_CPU);
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

  /* Always wake up on any DIO edge detection */
  ti_lib_aon_event_mcu_wake_up_set(AON_EVENT_MCU_WU2, AON_EVENT_IO);
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
