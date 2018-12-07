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
 * \addtogroup ota-update-bootloader
 * @{
 *         CoAP resource used for uploading the new firmware
 * @{
 * \file
 *         res-firmware
 * \author
 *         Nenad Ilic <nenad@relayr.io>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "er-coap-block1.h"
#include "er-coap-separate.h"
#include "er-coap-transactions.h"
#include "dev/flash.h"
#include "dev/rom-util.h"
#include "dev/watchdog.h"
#include "cpu.h"
#include "lib/crc16.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(M, ...) printf("DEBUG %s:%d: " M, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static void res_put_handler(void *request, void *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset);
/*---------------------------------------------------------------------------*/
RESOURCE(res_firmware,
         "title=\"Firmware update\";rt=\"block\"",
         NULL,
         res_put_handler,
         res_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
struct __attribute__((__packed__)) firmware_hdr {
  uint32_t size;    /* size in bytes */
  uint32_t addr;    /* physical load addr */
  uint32_t crc16;
} firmware_img_hdr;
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
res_put_handler(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset)
{
  coap_packet_t *const coap_req = (coap_packet_t *)request;
  uint8_t *incoming = NULL;
  size_t len = 0;
  unsigned int ct = -1;
  static uint16_t crc16_calc = 0;

  REST.get_header_content_type(request, &ct);

  if(ct != APPLICATION_OCTET_STREAM) {
    const char *error_msg = "Error Content not application/octet-stream!";
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    REST.set_response_payload(response, error_msg, strlen(error_msg));
    return;
  }

  len = REST.get_request_payload(request, (const uint8_t **)&incoming);

  PRINTF("len:0x%x\n", len);

  if(len) {
    uint8_t *flashed_data;
    int i;
    /* first message is the header information */
    if(coap_req->block1_num == 0) {
      /* get the header */
      memcpy(&firmware_img_hdr, incoming, sizeof(struct firmware_hdr));

      crc16_calc = 0;

      PRINTF("firmware address:0x%x\n", firmware_img_hdr.addr);
      PRINTF("firmware size:0x%x\n", firmware_img_hdr.size);
      PRINTF("firmware crc16:0x%x\n", firmware_img_hdr.crc16);

      /* erase flash */
      INTERRUPTS_DISABLE();
      rom_util_page_erase((firmware_img_hdr.addr & ~(FLASH_PAGE_SIZE - 1)),
                          firmware_img_hdr.size);
      INTERRUPTS_ENABLE();

      REST.set_response_status(response, REST.status.CHANGED);
      coap_set_header_block1(response, coap_req->block1_num, 0,
                             coap_req->block1_size);
      return;
    }

    flashed_data = (uint8_t *)(firmware_img_hdr.addr +
                               (coap_req->block1_num - 1) *
                               coap_req->block1_size);

    INTERRUPTS_DISABLE();
    rom_util_program_flash((uint32_t *)incoming, (uint32_t)flashed_data, len);
    INTERRUPTS_ENABLE();

    for(i = 0; i < len; i++) {
      crc16_calc = crc16_add(flashed_data[i], crc16_calc);
    }

    REST.set_response_status(response, REST.status.CHANGED);
    coap_set_header_block1(response, coap_req->block1_num, 0,
                           coap_req->block1_size);

    /* last message */
    if(coap_req->block1_more == 0) {
      /* check checksum */
      if(crc16_calc == firmware_img_hdr.crc16) {
        const char *success_msg = "Firmware updated. Device will reboot!";
        update_cca_startup_address(firmware_img_hdr.addr);
        PRINTF("%s\n", success_msg);

        ctimer_set(&reboot_timer, CLOCK_SECOND, do_the_reboot, NULL);
        REST.set_response_payload(response, success_msg, strlen(success_msg));
      } else {
        const char *error_msg = "CRC doesn't match";
        REST.set_response_status(response, REST.status.NOT_MODIFIED);
        REST.set_response_payload(response, error_msg, strlen(error_msg));
      }
    }
  } else {
    const char *error_msg = "NoPayload";
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    REST.set_response_payload(response, error_msg, strlen(error_msg));
    return;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
