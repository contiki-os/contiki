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

  //  (1) Get the metadata of whatever firmware is currently installed
  OTAMetadata_t current_firmware = get_current_metadata();
  //  (2) Are there any newer firmware images in ext-flash?
  uint8_t newest_ota_slot = find_newest_ota_image();
  OTAMetadata_t newest_firmware = get_ota_slot_metadata( newest_ota_slot );
/*
  erase_ota_image( 1 );
  erase_ota_image( 2 );
  erase_ota_image( 3 );
  return 0;
*/
  uint32_t led_pin;
  switch (newest_ota_slot)
  {
    case 0:
      led_pin = IOID_25;
      break;
    case 1:
      led_pin = IOID_27;
      break;
    case 2:
      led_pin = IOID_7;
      break;
    case 3:
      led_pin = IOID_6;
      break;
  }
  GPIODirModeSet( (1 << led_pin), GPIO_DIR_MODE_OUT);
  GPIOPinWrite( (1 << led_pin), true );

  //  (3) If there's any newer images in storage, update the current firmware!
  if ( newest_ota_slot && (newest_firmware.version > current_firmware.version) ) {
    update_firmware( newest_ota_slot );
  }

  GPIOPinWrite( (1 << led_pin), false );
  jump_to_image( (CURRENT_FIRMWARE<<12) );

  //  ( ) Boot to the current firmware
  return 0;
}
