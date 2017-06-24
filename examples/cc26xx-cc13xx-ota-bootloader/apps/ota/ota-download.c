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
reset_ota_buffer() {
  uint16_t n;
  for (n=0; n<OTA_BUFFER_SIZE; n++)
  {
    ota_buffer[ n ] = 0xff;
  }
}

/*******************************************************************************
 * @fn      firmware_chunk_handler
 *
 * @brief   Handle incoming data from the CoAP request.
 *          This mostly involves writing the data to the ota_buffer[] array.
 *          When ota_buffer is full, we copy it into external flash.
 *
 */
void
firmware_chunk_handler(void *response)
{
  //  (1) Parse the data payload from the CoAP response
  const uint8_t *chunk;
  int len = coap_get_payload(response, &chunk);

  //  (2) Copy the CoAP payload into the ota_buffer
  ota_bytes_received += len;
  printf("Downloaded %u bytes\t(%#x)", len, ota_bytes_received);
  while (len--) {
    ota_buffer[ img_req_position++ ] = *chunk++;
  }

  //  (3) Handle metadata-specific information
  if ( metadata_received ) {
    //  If we have metadata already, calculate how much of the download is done.
    int percent = 10000.0 * ((float)( ota_bytes_saved + img_req_position) / (float)(new_firmware_metadata.size + OTA_METADATA_SPACE));
    PRINTF("\t%u.%u%%\n", percent/100, (percent - ((percent/100)*100)));
  } else if ( img_req_position >= OTA_METADATA_LENGTH ) {
    PRINTF("\n\nDownload metadata acquired:\n");
    PRINTF("==============================================\n");
    //  If we don't have metadata yet, get it from the first page
    //  (1) Extract metadata from the ota_buffer
    memcpy( &new_firmware_metadata, ota_buffer, OTA_METADATA_LENGTH );
    print_metadata( &new_firmware_metadata );
    PRINTF("\n");
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
    PRINTF("Downloading OTA update to OTA slot #%i.\n", active_ota_download_slot);

    //  (3) Erase the destination OTA download slot
    while( erase_ota_image( active_ota_download_slot ) );

    PRINTF("==============================================\n\n");
  }

  //  (4) Save the latest ota_buffer to flash if it's full.
  if ( img_req_position >= OTA_BUFFER_SIZE ) {
    PRINTF("==============================================\n");
    while( store_firmware_data( ( ota_bytes_saved + (ota_images[active_ota_download_slot-1] << 12)), ota_buffer, OTA_BUFFER_SIZE ) );
    ota_bytes_saved += img_req_position;
    img_req_position = 0;
    reset_ota_buffer();
    PRINTF("==============================================\n\n");
  }
}

PROCESS_THREAD(ota_download_th, ev, data)
{
  PROCESS_BEGIN();

  //  (1) Set the IP of our CoAP server
  OTA_SERVER_IP();
  char *ota_url = "/ota";

  //  (2) Initialize download parameters
  reset_ota_buffer();
  metadata_received = false;
  ota_download_active = true;

  //  (3) Initialize CoAP engine
  coap_init_engine();
  static coap_packet_t request[1];

  //  (4) Repeat OTA requests until we have the whole firmware
  coap_request_count = 0;
  while ( ota_download_active ) {
    PRINTF("\n-----CoAP Request %u Started-----\n", coap_request_count);

    //  (1) Reset data received counters
    img_req_position = 0;
    ota_req_start = ota_bytes_saved;

    PRINTF("Requesting OTA image starting at address %#x\n", ota_req_start);

    //  (2) Construct a blockwise CoAP GET request
    coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
    //  url is always "/ota"
    coap_set_header_uri_path(request, ota_url);
    //  By adding a block2 header to our request, the server will know this is
    //  a blockwise data transfer.
    coap_set_header_block2(request, 0, 0, 256);
    //  We use the CoAP request payload to tell the server how much of the firmware
    //  we've already saved to ext-flash.  The server will then transmit the remainder.
    coap_set_payload(request, (uint8_t *)&ota_req_start, sizeof(uint32_t));

    //  (3) Issue a GET request to the OTA server
    //      firmware_chunk_handler will process incoming data.
    COAP_BLOCKING_REQUEST(&ota_server_ipaddr, REMOTE_PORT, request, firmware_chunk_handler);

    //  Above CoAP request will block until failure/completion...
    PRINTF("-----CoAP Request %u Terminated-----\n\n", coap_request_count);


    //  (4) When the request is complete (possibly failed), check how much data
    //      we've received thus far.
    if ( metadata_received ) {
      //  If we have metadata, we can determine what the download's total size
      //  should have been.
      if ( ota_bytes_received >= (OTA_METADATA_SPACE + new_firmware_metadata.size) ) {
        //  We've received all the bytes we need.  Leave the download loop.
        ota_download_active = false;
        break;
      } else {
        //  Oh no!  Our download got interrupted somehow!
        //  Rewind the download to the end of the last valid page.
        ota_bytes_saved = (ota_bytes_saved/FLASH_PAGE_SIZE) << 12;
        PRINTF("Erasing %#x\n", (ota_bytes_saved + (ota_images[active_ota_download_slot-1] << 12)));
        while( erase_extflash_page( (ota_bytes_saved + (ota_images[active_ota_download_slot-1] << 12)) ) );
      }
    } else {
      ota_bytes_saved = 0;
      PRINTF("Erasing %#x\n", ota_bytes_saved);
      //  We need to redownload starting with the last page, so let's clear it first.
      while( erase_extflash_page( ota_images[active_ota_download_slot-1] << 12 ) );
    }

    //  (5) If we've made it this far, we need to do another CoAP request.
    ota_bytes_received = ota_bytes_saved;
    coap_request_count++;
  }

  //  (5) Save the last ota_buffer!  This may not happen in the firmware_chunk_handler
  //      if the last ota_buffer wasn't totally full, as img_req_position
  //      will never increment to OTA_BUFFER_SIZE.
  PRINTF("==============================================\n");
  while( store_firmware_data( ( ota_bytes_saved + (ota_images[active_ota_download_slot-1] << 12)), ota_buffer, OTA_BUFFER_SIZE ) );
  ota_bytes_saved += img_req_position;
  PRINTF("==============================================\n\n");

  //  (6) Recompute the CRC16 algorithm over the received data.  This value is
  //      called the "CRC Shadow." If it doesn't match the CRC value in the
  //      metadata, our download got messed up somewhere along the way!
  while( verify_ota_slot( active_ota_download_slot ) );

  //  (7) Reboot!
  ti_lib_sys_ctrl_system_reset();

  PROCESS_END();
}
