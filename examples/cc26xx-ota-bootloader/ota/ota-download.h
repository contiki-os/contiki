#ifndef OTA_DOWNLOAD_H
#define OTA_DOWNLOAD_H

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "http-socket.h"
#include "ip64-addr.h"
#include "ota.h"

#define OTA_IMAGE_SERVER  "http://[bbbb::1]:3003"

/* OTA Download Thread */
extern int ota_download_th_p; // Pointer to OTA Download thread

//  OTA Download Events
typedef enum {
  OTA_HTTP_REQUEST_SUCCESS,
  OTA_HTTP_REQUEST_FAIL,
  OTA_HTTP_REQUEST_RETRY,
  OTA_PAGE_DOWNLOAD_COMPLETE,
  OTA_IMAGE_DOWNLOAD_COMPLETE
} ota_event_t;

#define img_req_length  1024    //  size of image data chunk we are asking for from server
static uint32_t img_req_position;    //  current start byte of image data we are requesting

#define HTTP_PAYLOAD_START(data)  (*data == 0xa) && (*(data - 1) == 0xd)
#define HTTP_PAYLOAD_END(data)    (*(data+1) == 0xa) && (*data == 0xd)

static OTAMetadata_t new_firmware_metadata;
static int active_ota_download_slot;
static uint8_t page_buffer[ FLASH_PAGE_SIZE ];
static uint8_t page;
static bool ota_downloading_page;
static bool page_started;
static uint16_t bytes_received;

static bool ota_downloading_image;
#endif
