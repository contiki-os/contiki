#include "ota-download.h"

PROCESS(ota_download_th, "OTA Download Agent");
ota_download_th_p = &ota_download_th;

static bool ota_downloading_page = false;
static bool ota_downloading_image = false;
static bool metadata_received = false;
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
    PRINTF("HTTP socket error\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_TIMEDOUT) {
    PRINTF("HTTP socket error: timed out\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_ABORTED) {
    PRINTF("HTTP socket error: aborted\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_HOSTNAME_NOT_FOUND) {
    PRINTF("HTTP socket error: hostname not found\n");
    process_post(ota_download_th_p, OTA_HTTP_REQUEST_FAIL, (process_data_t)NULL);
  } else if(e == HTTP_SOCKET_CLOSED) {
    PRINTF("HTTP socket closed, %d bytes received\n", bytes_received);
    if ( img_req_position >= FLASH_PAGE_SIZE ) {
      process_post(ota_download_th_p, OTA_PAGE_DOWNLOAD_COMPLETE, (process_data_t)NULL);
    } else {
      process_post(ota_download_th_p, OTA_HTTP_REQUEST_SUCCESS, (process_data_t)NULL);
    }
  } else if(e == HTTP_SOCKET_DATA) {
    //PRINTF("\n");
    while (datalen--)
    {
      //PRINTF("%#x (%u) ", *data, datalen);
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
        else if ( (HTTP_PAYLOAD_END( data )) || (img_req_position > FLASH_PAGE_SIZE) ) {
          page_started = false;
          break;
        }
        //  Otherwise, this is valid data.  Write it down.
        else {
          bytes_received++;
          page_buffer[ img_req_position++ ] = *data;
        }
      }
      else {
        if ( HTTP_PAYLOAD_START( data ) ) {
          page_started = true;
        }
      }
      *data++;
    }
    //PRINTF("\n");
  }
}

PROCESS_THREAD(ota_download_th, ev, data)
{

  PROCESS_BEGIN();

  //  (1) Initialize the HTTP download
  http_socket_init(&s);
  bytes_received = 0;
  ota_downloading_image = true;
  img_req_position_last = 0;

  //  (2) Begin downloading the OTA update, page by page.
  for (page=0; page<25; page++)
  {
    PRINTF("\nDownloading Page %u/25:\n", page);

    //  (1) Clear Page Buffer
    reset_page_buffer();
    img_req_position = 0;
    img_req_position_last = 0;

    //  (2) Download page
    ota_downloading_page = true;
    while ( ota_downloading_page )
    {
      //  (1) Construct a URL requesting the current page of data
      char url[120];
      bytes_received = 0;
      sPRINTF(url, OTA_IMAGE_SERVER "/%lu/%u", (img_req_position+(page<<12)), img_req_length);

      //  (2) Issue HTTP GET request to server
      page_started = false;
      http_socket_get(&s, url, 0, 0, firmware_binary_cb, NULL);

      //  (3) Yield until HTTP request callback returns
      PROCESS_YIELD_UNTIL( (ev == OTA_HTTP_REQUEST_SUCCESS) || (ev == OTA_HTTP_REQUEST_FAIL) || (ev == OTA_HTTP_REQUEST_RETRY) || (ev == OTA_PAGE_DOWNLOAD_COMPLETE) || (ev == OTA_IMAGE_DOWNLOAD_COMPLETE) );

      switch ( ev ) {
        case OTA_HTTP_REQUEST_SUCCESS:
        {
          PRINTF("Download complete!\n");
          if ( (bytes_received != img_req_length) && ( ((page<<12) + img_req_position) < new_firmware_metadata.size ) ) {
            //  Something's not right.  Redo this download!
            PRINTF("Something was wrong with that download!  Retrying.\n");
            //  Re-erase the destination area of the page buffer
            for (int b=img_req_position_last; b<img_req_position; b++) {
              page_buffer[ b ] = 0xff;
            }
            //  Roll back the page_data pointer to the last position
            img_req_position = img_req_position_last; // rewind our image data pointer
            PRINTF("Rolling back to byte %u of page %u\n", img_req_position_last, page);
            continue;
          } else {
            PRINTF("Now at byte %u of page %u\n", img_req_position, page);
            img_req_position_last = img_req_position;
          }
        } break;
        case OTA_HTTP_REQUEST_FAIL:
        {
          PRINTF("Failed HTTP request.  Retrying.\n");
          continue;
        } break;
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

      //  (3) What OTA slot should we download into?
      //      Note: This code only executes on the very first GET.
      if ( metadata_received ) {
        int percent = 10000.0 * ((float)((page<<12) + img_req_position) / (float)new_firmware_metadata.size);
        PRINTF("========> OTA download %u.%u%%\tcomplete\n\n", percent/100, (percent - ((percent/100)*100)));
      } else {
        //  (1) Extract metadata from the page_buffer
        memcpy( &new_firmware_metadata, page_buffer, OTA_METADATA_LENGTH );
        print_metadata( &new_firmware_metadata );
        metadata_received = true;

        //  (2) Check to see if we have any OTA slots already containing
        //      firmware of the same version number as the metadata has.
        active_ota_download_slot = find_matching_ota_slot( new_firmware_metadata.version );
        if ( active_ota_download_slot == -1 ) {
          //  We don't already have a copy of this firmware version, let's download
          //  to an empty OTA slot!
          active_ota_download_slot = find_empty_ota_slot();
          if ( !active_ota_download_slot ) {
            active_ota_download_slot = 1;
          }
        }
        PRINTF("\nDownloading OTA update to OTA slot #%i.\n", active_ota_download_slot);

        //  (3) Erase the destination OTA download slot
        while( erase_ota_image( active_ota_download_slot ) );
      }
    }

    //  (4) Save firmware page to flash
    PRINTF("\n============================================\n");
    while( store_firmware_page( ((page+ota_images[active_ota_download_slot-1]) << 12), page_buffer ) );
    PRINTF("============================================\n");

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
