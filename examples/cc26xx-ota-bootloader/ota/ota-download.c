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

static void
callback(struct http_socket *s, void *ptr,
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
    if ( ((img_req_position % FLASH_PAGE_SIZE) + OTA_METADATA_LENGTH) >= FLASH_PAGE_SIZE ) {
      process_post(ota_download_th_p, OTA_PAGE_DOWNLOAD_COMPLETE, (process_data_t)NULL);
    } else {
      process_post(ota_download_th_p, OTA_HTTP_REQUEST_SUCCESS, (process_data_t)NULL);
    }
  } else if(e == HTTP_SOCKET_DATA) {
    //printf("HTTP socket received %d bytes of data\n", datalen);
    //printf("Data received: ");
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
        else if ( (*(data+1) == 0xa) && (*data == 0xd) )
        {
          page_started = false;
          break;
        }
        //  Otherwise, this is valid data.  Write it down.
        else
        {
          bytes_received++;
          page_buffer[ (OTA_METADATA_LENGTH + (img_req_position++)) % FLASH_PAGE_SIZE ] = *data;
          //printf("%#x ", *data);
        }
      }
      else
      {
        if ( (*data == 0xa) && (*(data - 1) == 0xd) )
        {
          page_started = true;
        }
      }
      *data++;
    }
    //printf("\n");
  }
}


PROCESS_THREAD(ota_download_th, ev, data)
{
  PROCESS_BEGIN();

  //  (1) Configure HTTP client to connect to bbbb::1
  uip_ip6addr_t ip6addr;
  uip_ip6addr(&ip6addr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
  http_socket_init(&s);

  ota_downloading_image = true;
  img_req_position = 0;

  for (page=0; page<25; page++)
  {
    //  (1) Clear Page Buffer
    reset_page_buffer();

    //  (2) Download page
    printf("Downloading Page %u/25:\n", page);
    ota_downloading_page = true;
    while (ota_downloading_page)
    {
      //  (1) Construct a URL requesting the current page of data
      char url[120];
      bytes_received = 0;
      if ( img_req_position ) {
        sprintf(url, "http://[bbbb::1]:3003/%lu/%u", img_req_position, img_req_length);
      } else {
        sprintf(url, "http://[bbbb::1]:3003/%lu/%u", img_req_position, (img_req_length-OTA_METADATA_LENGTH));
      }

      //  (2) Issue HTTP GET request to server
      page_started = false;
      http_socket_get(&s, url, 0, 0, callback, NULL);

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
        }
      }

    }

    //  (3) Print/save page to console/flash
    uint32_t sum = 0;
    for (uint16_t n=0; n<FLASH_PAGE_SIZE; n++)
    {
      sum += page_buffer[n];
      //printf("%#x ", page_buffer[ n ]);
    }
    store_firmware_page( ((page+0x32) << 12), page_buffer );
    printf("\nSum: \t%lu\n", sum);


    //  (4) Are we done?
    if (!ota_downloading_image) {
      break;
    }

  }

  printf("Done downloading!\n");
  jump_to_image( 0x0 );

  PROCESS_END();
}
