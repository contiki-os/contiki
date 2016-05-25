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

void
initialize_peripherals() {
  /* Disable global interrupts */
  bool int_disabled = ti_lib_int_master_disable();

  //power_domains_on();

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* I2C controller */
  //board_i2c_wakeup();

  //buzzer_init();

  /* Make sure the external flash is in the lower power mode */
  ext_flash_init();

  //lpm_register_module(&sensortag_module);

  /* For unsupported peripherals, select a default pin configuration */
  //configure_unused_pins();

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
    0, 0, 0x27, 0x1, 0x2, 0x2000
  };
  if ( FlashProgram((uint8_t*)&example_metadata, 0x1000, OTA_METADATA_LENGTH) == FAPI_STATUS_SUCCESS ) {
  //  return 0;
  } else {
  //  return -1;
  }

  generate_fake_metadata();

  jump_to_image( CURRENT_FIRMWARE );
  return 0;
}
