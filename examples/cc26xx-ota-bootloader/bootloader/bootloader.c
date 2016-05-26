#include <stdint.h>
#include "ti-lib.h"

#include "ota.h"

/**
 *    Begin executing a new firmware image located at destination_address.
 */
void
jump_to_image(uint32_t destination_address)
{
  destination_address += /*OTA_METADATA_LENGTH +*/ OTA_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("ORR R0, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R0");                                             //  Branch execution
}

static void
power_domains_on(void) {
  /* Turn on the PERIPH PD */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);

  /* Wait for domains to power on */
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
        != PRCM_DOMAIN_POWER_ON));
}

void
initialize_peripherals() {
  /* Disable global interrupts */
  bool int_disabled = ti_lib_int_master_disable();

  power_domains_on();

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Make sure the external flash is in the lower power mode */
  ext_flash_init();

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }
}

int
main(void)
{
  initialize_peripherals();

  OTAMetadata_t example_metadata = {
    0, 0, 0x27, 0x1, 0x2, 0x2
  };
  //if ( FlashProgram((uint8_t*)&example_metadata, FLASH_PAGE_SIZE, OTA_METADATA_LENGTH) == FAPI_STATUS_SUCCESS ) {
  //  return 0;
  //} else {
  //  return -1;
  //}

  //generate_fake_metadata();
/*
  uint8_t fake_data[ FLASH_PAGE_SIZE ];
  uint16_t n;
  for (n=0; n<FLASH_PAGE_SIZE; n++) {
    if ( n%2 ) {
      fake_data[ n ] = 2;
    } else {
      fake_data[ n ] = 4;
    }
  }
  store_firmware_page( 0x32000, fake_data );

  for (n=0; n<FLASH_PAGE_SIZE; n++) {
    if ( n%2 ) {
      fake_data[ n ] = 7;
    } else {
      fake_data[ n ] = 2;
    }
  }
  store_firmware_page( 0x33000, fake_data );

  update_firmware( 0 );
*/
  jump_to_image( CURRENT_FIRMWARE<<12 );
  return 0;
}
