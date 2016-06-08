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

  #if CLEAR_OTA_SLOTS
  erase_ota_image( 1 );
  erase_ota_image( 2 );
  erase_ota_image( 3 );
  #endif

  #if BURN_GOLDEN_IMAGE

  #endif

  //  (1) Get the metadata of whatever firmware is currently installed
  OTAMetadata_t current_firmware;
  get_current_metadata( &current_firmware );

  //  (2) Are there any newer firmware images in ext-flash?
  uint8_t newest_ota_slot = find_newest_ota_image();
  OTAMetadata_t newest_firmware;
  while( get_ota_slot_metadata( newest_ota_slot, &newest_firmware ) );

  //  (3) If there's a newer image, install that version!
  if ( ( newest_ota_slot > 0 ) && (newest_firmware.version > current_firmware.version) ) {
    update_firmware( newest_ota_slot );
    get_current_metadata( &current_firmware ); // update the current firmware metadata!
  }

  //  (4) Verify the current firmware! (Recompute the CRC over the internal flash image)
  verify_current_firmware( &current_firmware );

  //  (5) If the current image is valid, jump to it.
  if ( validate_ota_metadata( &current_firmware ) ) {
    jump_to_image( (CURRENT_FIRMWARE<<12) );
  }

  //  (6) If we've gotten here, something's wrong.  Install the latest valid firmware!
  //      We no longer care if it's newer than what we have now, because we assume it is
  //      invalid data.
  update_firmware( newest_ota_slot );
  ti_lib_sys_ctrl_system_reset(); // reboot

  //  This function should never return
  return 0;
}
