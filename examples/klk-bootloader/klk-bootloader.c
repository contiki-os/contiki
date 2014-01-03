/*
 * Copyright (c) 2013, Kerlink
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
 * $Id: $
 */

/**
 * \file
 *         The Main Bootloader
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>

#include "em_gpio.h"
#include "em_system.h"
#include "em_rmu.h"

#include "shell.h"
#include "clock.h"
#include "debug-uart.h"
#include "dev/flash.h"
#include "ymodem.h"
#include "gpio.h"
#include "arm_utils.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(bootloader_process, "Bootloader");

AUTOSTART_PROCESSES(&bootloader_process);

char shell_prompt_text[] = "Klk-Bootloader> ";

static uint32_t _flash_addr = USERAPP_START_ADDR;

void _print_reset_source(void)
{
  uint32_t reset_cause = 0;

  PRINTF("Reset triggered by: ");
  reset_cause = RMU_ResetCauseGet();
  if(reset_cause & RMU_RSTCAUSE_PORST)       { PRINTF("POR "); }
  if(reset_cause & RMU_RSTCAUSE_BODUNREGRST) { PRINTF("Brown_out_unreg "); }
  if(reset_cause & RMU_RSTCAUSE_BODREGRST)   { PRINTF("Brown_out_reg "); }
  if(reset_cause & RMU_RSTCAUSE_EXTRST)      { PRINTF("External "); }
  if(reset_cause & RMU_RSTCAUSE_WDOGRST)     { PRINTF("Watchdog "); }
  if(reset_cause & RMU_RSTCAUSE_LOCKUPRST)   { PRINTF("M3_Lockup "); }
  if(reset_cause & RMU_RSTCAUSE_SYSREQRST)   { PRINTF("Software "); }

  PRINTF(" (%lX)\r\n",RMU_ResetCauseGet());

  RMU_ResetCauseClear();
}

static int store_file_in_flash(char* in_filename, unsigned char *in_buf, int in_length)
{
  int ret = 0;

  // If address is aligned, erase sector
  if( (_flash_addr & (FLASH_PAGE_SIZE - 1)) == 0)
  {
    flash_erase(_flash_addr,FLASH_PAGE_SIZE);
  }
  ret = flash_write(_flash_addr, (const char *)in_buf, in_length);
  _flash_addr = _flash_addr + in_length;
  return ret;
}

void bootloader_main(uint32_t u32_resetcause)
{
  uint32_t filesize = 0;

  if(!gpio_get_value(GPIO_BOOTLOADER))
  {
    dbg_setup_uart(115200);
    PRINTF( "\r\nKLK - BOOTLOADER - " CONTIKI_VERSION_STRING "\r\n");
    // TBC ... Ymodem, Menu ?
    ymodem_init();
    do
    {
      _flash_addr = USERAPP_START_ADDR;
      filesize = ymodem_receive(store_file_in_flash);
    }while(filesize == 0);

    PRINTF( "Transfer complete ! ... Jump to application \r\n");

    clock_delay_msec(1000);
    arm_jump_to_application(USERAPP_START_ADDR);
  }
  else
  {
    // Todo, verif update/restore flags....
    arm_jump_to_application(USERAPP_START_ADDR);

  }

  PRINTF("Failed to jump ... reboot system !\r\n");
  clock_delay_msec(1000);
  watchdog_reboot();
}


/* Support for autostart (test other platform than kerlink ones) */

static int gpio_irq_handler(int irq, void *priv)
{
  printf("********* GPIO IRQ %d\n\r",irq);
}

void enter_bootloader_menu(void)
{
  SYSTEM_ChipRevision_TypeDef chipRev;
  clock_time_t time = 0;

#ifdef DEBUG
  watchdog_stop();
#endif

  printf("\r\n************************************************\r\n");
  printf( "KLK - BOOTLOADER - " CONTIKI_VERSION_STRING "\r\n");
  printf("************************************************\r\n");

  printf("EFM32 Device: ");
  SYSTEM_ChipRevisionGet(&chipRev);
  printf("%u rev%d.%d (UID %llX)\r\n", SYSTEM_GetPartNumber(), chipRev.major, chipRev.minor, SYSTEM_GetUnique());
  _print_reset_source();

  serial_shell_init();
  shell_time_init();
  shell_reboot_init();

  shell_efm32hw_init();

#ifdef COFFEE
  shell_coffee_init();
  shell_file_init();
#endif

}

/*---------------------------------------------------------------------------*/
static void dump_buf(const uint8_t *buf, int len)
{
    int i=0;

    for(i=0;i<len;i++)
    {
        printf("%02X ",buf[i]);
        if((i%16) == 15) printf("\n\r");
    }
    printf("\n\r");
}
/*---------------------------------------------------------------------------*/
/*
 * Bootloader info :
 * http://markdingst.blogspot.fr/2012/06/make-own-bootloader-for-arm-cortex-m3.html :
 *
 *
 *
 * https://github.com/nickgeoca/maple-bootloader
 */


PROCESS_THREAD(bootloader_process, ev, data)
{
  int irqno = 0;

  PROCESS_BEGIN();

  // HACK : FORCE BOOTLOADER MENU
  gpio_set_mode(GPIO_BOOTLOADER, gpioModeInputPullFilter, 1);
  //if(!gpio_get_value(GPIO_BOOTLOADER))
  if(1)
  {
    irqno = gpio_register_irq(GPIO_BOOTLOADER, IRQ_TYPE_EDGE_BOTH, &gpio_irq_handler, NULL);
    gpio_enable_irq(irqno);
    enter_bootloader_menu();
  }
  else
  {
    arm_jump_to_application(USERAPP_START_ADDR);
  }

  do{
  // Last Waiting loop
  PROCESS_WAIT_EVENT();
  }while(1);

  PROCESS_END();
}
