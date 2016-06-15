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
  //  (1) Parse the data payload from the CoAP response
  const uint8_t *chunk;
  int len = coap_get_payload(response, &chunk);

  //  (2) Copy the CoAP payload into the page_buffer
  ota_bytes_received += len;
  printf("Downloaded %u bytes\t(%#x)\n", len, ota_bytes_received);
  while (len--) {
    page_buffer[ img_req_position++ ] = *chunk++;
  }

  //  (3) Handle the case where the page_buffer is full
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
  char *ota_url = "/ota";

  //  (2) Initialize download parameters
  reset_page_buffer();
  metadata_received = false;
  page = 0;
  ota_download_active = true;

  //  (3) Initialize CoAP engine
  coap_init_engine();
  static coap_packet_t request[1];

  //  (4) Repeat OTA requests until we have the whole firmware
  coap_request_count = 0;
  while ( ota_download_active ) {
    PRINTF("-----CoAP Request %u Started-----\n", coap_request_count);

    //  (1) Reset data received counters
    //  We always start downloading from the beginning of a page.
    img_req_position = 0;
    //  ota_bytes_received represents the total number of bytes we have received
    //  via CoAP.
    ota_bytes_received = (page<<12);
    //  ota_start_address saves the original start address of our download in
    //  case we need to repeat the download later.
    //  ota_bytes_received will incremement upwards as the data is received,
    //  but ota_start_address will remain static during that period.
    ota_start_address = ota_bytes_received;

    PRINTF("Requesting OTA image starting at address %#x\n", ota_bytes_received);

    //  (2) Construct a blockwise CoAP GET request
    coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
    //  url is always "/ota"
    coap_set_header_uri_path(request, ota_url);
    //  By adding a block2 header to our request, the server will know this is
    //  a blockwise data transfer.
    coap_set_header_block2(request, 0, 0, 1024);
    //  We use the CoAP request payload to tell the server how much of the firmware
    //  we've already saved to ext-flash.  The server will then transmit the remainder.
    coap_set_payload(request, (uint8_t *)&ota_start_address, sizeof(uint32_t));

    //  (3) Issue a GET request to the OTA server
    //      firmware_chunk_handler will process incoming data.
    COAP_BLOCKING_REQUEST(&ota_server_ipaddr, REMOTE_PORT, request, firmware_chunk_handler);

    //  Above CoAP request will block until failure/completion...
    PRINTF("-----CoAP Request %u Terminated-----\n", coap_request_count);

    //  (4) When the request is complete (possibly failed), check how much data
    //      we've received thus far.
    //  If we have metadata, we can determine what the download's total size
    //  should have been.
    if ( metadata_received )
    {
      if ( ota_bytes_received >= (OTA_METADATA_SPACE + new_firmware_metadata.size) ) {
        //  We've received all the bytes we need.  Leave the download loop.
        ota_download_active = false;
        break;
      }
    }
    //  Since the metadata is the first chunk of the data, if we don't have any,
    //  just start the download from the beginning.
    else {
      //  If we don't have metadata, the download should start from the beginning.
      ota_bytes_received = 0;
      page = 0;
    }

    //  (5) If we've made it this far, we need to do another CoAP request.
    coap_request_count++;
  }

  //  (5) Save the last page!  This may not happen in the firmware_chunk_handler
  //      if the last page of the OTA isn't totally full, as img_req_position
  //      will never increment to FLASH_PAGE_SIZE
  PRINTF("\n============================================\n");
  while( store_firmware_page( ((page+ota_images[active_ota_download_slot-1]) << 12), page_buffer ) );
  PRINTF("============================================\n");

  //  (6) Recompute the CRC16 algorithm over the received data.  This value is
  //      called the "CRC Shadow." If it doesn't match the CRC value in the
  //      metadata, our download got messed up somewhere along the way!
  while( verify_ota_slot( active_ota_download_slot ) );

  //  (7) Reboot!
  ti_lib_sys_ctrl_system_reset();

  PROCESS_END();
}
