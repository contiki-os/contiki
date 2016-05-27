#include <stdint.h>
#include "ti-lib.h"

#include "ota.h"

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

  //update_firmware( 0 );
  jump_to_image( CURRENT_FIRMWARE<<12 );
  return 0;
}
