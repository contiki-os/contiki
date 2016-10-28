/*
 * Copyright (c) 2016, relayr http://relayr.io/
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
 * \addtogroup ota-update-hello-world
 * @{
 *         CoAP resource for kicking the application in bootloader mode
 * @{
 * \file
 *         res-bootloader
 * \author
 *         Nenad Ilic <nenad@relayr.io>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "dev/flash.h"
#include "dev/rom-util.h"
#include "dev/watchdog.h"
#include "cpu.h"
#include "cfs-coffee-arch.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(M, ...) printf("DEBUG %s:%d: " M, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static void res_put_handler(void *request, void *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset);
/*---------------------------------------------------------------------------*/
RESOURCE(res_bootloader,
         "title=\"Jump to bootloader\";rt=\"block\"",
         NULL,
         res_put_handler,
         res_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
extern const flash_cca_lock_page_t flash_cca_lock_page;

static void
update_cca_startup_address(uint32_t addr)
{
  flash_cca_lock_page_t flash_cca_saved = flash_cca_lock_page;
  uint32_t sector_start = ((uint32_t)&flash_cca_lock_page) & ~(FLASH_PAGE_SIZE - 1);
  flash_cca_saved.app_entry_point = (const void *)addr;
  INTERRUPTS_DISABLE();
  rom_util_page_erase(sector_start, FLASH_PAGE_SIZE);
  rom_util_program_flash((uint32_t *)&flash_cca_saved,
                         (uint32_t)&flash_cca_lock_page,
                         sizeof(flash_cca_lock_page_t));
  INTERRUPTS_ENABLE();
}
/*---------------------------------------------------------------------------*/
static struct ctimer reboot_timer;
static void
do_the_reboot(void *ptr)
{
  watchdog_reboot();
}
/*---------------------------------------------------------------------------*/
static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  uint8_t *incoming = NULL;
  char tmp_buf[16];
  size_t len = 0;
  uint32_t bootloader_address;

  len = REST.get_post_variable(request, "address", (const char **)&incoming);
  /* we address is proved jump there */
  if(len && len < 16) {
    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, incoming, len);
    bootloader_address = (uint32_t)strtol((const char *)tmp_buf, NULL, 16);
  } else {
    /* otherwise we assume this is wher the bootloader sits */
    bootloader_address = COFFEE_START + COFFEE_SIZE;
  }
  /* Note: if there is no bootloader at this address the device will be bricked */
  update_cca_startup_address((uint32_t)bootloader_address);

  ctimer_set(&reboot_timer, CLOCK_SECOND, do_the_reboot, NULL);
  len = snprintf((char *)buffer,
                 preferred_size,
                 "Device will reboot to bootloader starting at: 0x%lx!",
                 bootloader_address);
  PRINTF("%s\n", buffer);
  REST.set_response_payload(response, buffer, len);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
