/** @file   ota-download.c
 *  @brief  OTA Image Download Mechanism
 *  @author Mark Solters <msolters@gmail.com>
 */

#include "ota-download.h"

PROCESS(ota_download_th, "OTA Download Agent");
ota_download_th_p = &ota_download_th;

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

static void
reset_page_buffer() {
  for (uint16_t n=0; n<FLASH_PAGE_SIZE; n++)
  {
    page_buffer[ n ] = 0xff;
  }
}

/*******************************************************************************
 * @fn      firmware_chunk_handler
 *
 * @brief   Handle incoming data from the CoAP request.
 *          This mostly involves writing the data to the page_buffer[] array.
 *          When page_buffer is full, we copy it into external flash.
 *
 */
void
firmware_chunk_handler(void *response)
{
  const uint8_t *chunk;

  int len = coap_get_payload(response, &chunk);
  bytes_received += len;

  printf("Downloaded %u bytes\t(%#x)\n", len, bytes_received);

  while (len--) {
    page_buffer[ img_req_position++ ] = *chunk++;
  }

  //  ( ) Once we have a whole page of firmware data saved:
  if (img_req_position >= FLASH_PAGE_SIZE) {
    //  (1) Handle metadata-specific information
    if ( metadata_received ) {
      //  If we have metadata already, calculate how much of the download is done.
      int percent = 10000.0 * ((float)((page<<12) + img_req_position) / (float)new_firmware_metadata.size);
      PRINTF("========> OTA download %u.%u%%\tcomplete\n\n", percent/100, (percent - ((percent/100)*100)));
    } else {
      //  If we don't have metadata yet, get it from the first page
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

    //  (2) Save the latest page to flash!
    PRINTF("\n============================================\n");
    while( store_firmware_page( ((page+ota_images[active_ota_download_slot-1]) << 12), page_buffer ) );
    page++;
    img_req_position = 0;
    reset_page_buffer();
    PRINTF("============================================\n");
  }
}

PROCESS_THREAD(ota_download_th, ev, data)
{
  PROCESS_BEGIN();

  //  (1) Set the IP of our CoAP server
  OTA_SERVER_IP();
  const char *ota_url = "/ota";

  //  (2) Initialize download parameters
  reset_page_buffer();
  metadata_received = false;
  bytes_received = 0;
  page = 0;
  img_req_position = 0;

  //  (3) Initialize CoAP engine
  coap_init_engine();
  static coap_packet_t request[1];

  //  (4) Issue a GET request to the OTA server
  //      We will add a block2 header, so that we get the firmware blockwise
  coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
  coap_set_header_uri_path(request, ota_url);
  coap_set_header_block2(request, 0, 0, 1024); // get data blockwise
  COAP_BLOCKING_REQUEST(&ota_server_ipaddr, REMOTE_PORT, request, firmware_chunk_handler);

  //  (5) When the download is complete:
  PRINTF("-----done-----\n");

  //  (6)  Save the last page!
  PRINTF("\n============================================\n");
  while( store_firmware_page( ((page+ota_images[active_ota_download_slot-1]) << 12), page_buffer ) );
  PRINTF("============================================\n");

  //  Make OTA slot metadata as "valid"
  while( verify_ota_slot( active_ota_download_slot ) );

  // Reboot!
  //ti_lib_sys_ctrl_system_reset();

  PROCESS_END();
}
