#include "ota-download.h"

PROCESS(ota_download_th, "OTA Download Agent");
ota_download_th_p = &ota_download_th;

static bool ota_downloading_page = false;
static bool ota_downloading_image = false;
static uint16_t bytes_received = 0;
static struct http_socket s;

static void
reset_page_buffer() {
  for (uint16_t n=0; n<FLASH_PAGE_SIZE; n++)
  {
    page_buffer[ n ] = 0xff;
  }
}


/*******************************************************************************
 * @fn      firmware_binary_cb
 *
 * @brief   Handle the HTTP GET response to a request for firmware binary data.
 *
 */
static void
firmware_binary_cb(struct http_socket *s, void *ptr,
         http_socket_event_t e,
         const uint8_t *data, uint16_t datalen)
{
  if (!ota_downloading_image) {
    //  If no longer downloading, callbacks are ignored.
    return;
  }
  if(e == HTTP_SOCKET_ERR) {
    printf("HTTP socket error\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    printf("HTTP socket error: timed out\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_ABORTED) {
    printf("HTTP socket error: aborted\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    printf("HTTP socket error: hostname not found\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_CLOSED) {
    printf("HTTP socket closed, %d bytes received\n", bytes_received);
    if ( (img_req_position % FLASH_PAGE_SIZE) >= FLASH_PAGE_SIZE ) {
      process_post(ota_download_th_p, OTA_PAGE_DOWNLOAD_COMPLETE, (process_data_t)NULL);
    } else {
      process_post(ota_download_th_p, OTA_HTTP_REQUEST_SUCCESS, (process_data_t)NULL);
    }
  } else if(e == HTTP_SOCKET_DATA) {
    while (datalen--)
    {
      if (page_started) {
        //  If *data = "EOF"
        if (  (*data == 0x45) &&
              (*(data+1) == 0x4f) &&
              (*(data+2) == 0x46) )
        {
          process_post(ota_download_th_p, OTA_IMAGE_DOWNLOAD_COMPLETE, (process_data_t)NULL);
          return;
        }
        //  If we've reached the end of the HTTP response
        else if ( HTTP_PAYLOAD_END( data ) ) {
          page_started = false;
          break;
        }
        //  Otherwise, this is valid data.  Write it down.
        else {
          bytes_received++;
          page_buffer[ (img_req_position++ % FLASH_PAGE_SIZE) ] = *data;
          //printf("%#x ", *data);
        }
      }
      else {
        if ( HTTP_PAYLOAD_START( data ) ) {
          page_started = true;
        }
      }
      *data++;
    }
  }
}

PROCESS_THREAD(ota_download_th, ev, data)
{
  PROCESS_BEGIN();

  //  (1) Initialize the HTTP download
  http_socket_init(&s);
  reset_page_buffer();
  img_req_position = 0;
  bytes_received = 0;
  ota_downloading_image = true;

  //  (2) Begin downloading the OTA update, page by page.
  for (page=0; page<25; page++)
  {
    printf("\nDownloading Page %u/25:\n", page);

    //  (1) Clear Page Buffer
    reset_page_buffer();

    //  (2) Download page
    ota_downloading_page = true;
    while (ota_downloading_page)
    {
      //  (1) Construct a URL requesting the current page of data
      char url[120];
      bytes_received = 0;
      sprintf(url, OTA_IMAGE_SERVER "/%lu/%u", img_req_position, img_req_length);

      //  (2) Issue HTTP GET request to server
      page_started = false;
      http_socket_get(&s, url, 0, 0, firmware_binary_cb, NULL);

      //  (3) Yield until HTTP request callback returns
      PROCESS_YIELD_UNTIL( (ev == OTA_HTTP_REQUEST_SUCCESS) || (ev == OTA_HTTP_REQUEST_FAIL) || (ev == OTA_HTTP_REQUEST_RETRY) || (ev == OTA_PAGE_DOWNLOAD_COMPLETE) || (ev == OTA_IMAGE_DOWNLOAD_COMPLETE) );

      switch ( ev ) {
        case OTA_PAGE_DOWNLOAD_COMPLETE:
        {
          ota_downloading_page = false;
        } break;
        case OTA_IMAGE_DOWNLOAD_COMPLETE:
        {
          ota_downloading_page = false;
          ota_downloading_image = false;
        } break;
      }
    }

    //  (3) What OTA slot should we download into?
    //      This depends on metadata, so we only do this on the first page of
    //      the image download.
    if ( page == 0 ) {
      //  (1) Extract metadata from the page_buffer
      memcpy( &new_firmware_metadata, page_buffer, img_req_position );
      print_metadata( &new_firmware_metadata );

      //  (2) Check to see if we have any OTA slots already containing
      //      firmware of the same version number as the metadata has.
      active_ota_download_slot = find_matching_ota_slot( new_firmware_metadata.version );
      if ( active_ota_download_slot == -1 ) {
        //  We don't already have a copy of this firmware version, let's download
        //  to an empty OTA slot!
        active_ota_download_slot = find_empty_ota_slot();
      }
      PRINTF("\nDownloading OTA update to OTA slot #%i.\n", active_ota_download_slot);

      //  (3) Erase the destination OTA download slot
      while( erase_ota_image( active_ota_download_slot ) );
    }

    //  (4) Save firmware page to flash
    while( store_firmware_page( ((page+ota_images[active_ota_download_slot-1]) << 12), page_buffer ) );


    //  (5) Are we done?
    if (!ota_downloading_image) {
      break;
    }

  }

  PRINTF("Done downloading!\n");

  //  Make OTA slot metadata as "valid"
  verify_ota_slot( active_ota_download_slot );

  // Reboot!
  ti_lib_sys_ctrl_system_reset();

  PROCESS_END();
}
