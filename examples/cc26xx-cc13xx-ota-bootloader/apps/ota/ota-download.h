/** @file   ota-download.h
 *  @brief  OTA Image Download Mechanism
 *  @author Mark Solters <msolters@gmail.com>
 */

#ifndef OTA_DOWNLOAD_H
#define OTA_DOWNLOAD_H

#include "net/ip/uip.h"
#include "ota.h"

static uip_ipaddr_t ota_server_ipaddr;
#define OTA_SERVER_IP() uip_ip6addr(&ota_server_ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1)

/* OTA Download Thread */
extern struct process* ota_download_th_p; // Pointer to OTA Download thread

#define OTA_BUFFER_SIZE 1024

OTAMetadata_t new_firmware_metadata;
int active_ota_download_slot;
uint8_t ota_buffer[ OTA_BUFFER_SIZE ];
bool metadata_received;
uint32_t ota_bytes_received;
uint32_t ota_bytes_saved;
uint32_t ota_req_start;
uint32_t img_req_position;
bool ota_download_active;
int coap_request_count;

#endif
