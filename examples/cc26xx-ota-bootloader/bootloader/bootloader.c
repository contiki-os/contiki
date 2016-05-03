#include "ti-lib.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "driverlib/gpio.h"

int
main(void)
{
  /* Disable global interrupts */
  bool int_disabled = ti_lib_int_master_disable();

  /* Turn on the PERIPH PD */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);

  /* Wait for domains to power on */
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }

  GPIODirModeSet(GPIO_PIN_25, GPIO_DIR_MODE_OUT);
  GPIOPinWrite(GPIO_PIN_25, 1);

  // Load address of reset function from the fixed location of the image's
  // reset vector and jump.
  //__asm(" MOV R0, #0x1004 ");
  __asm(" LDR R0, =0x1014 ");
  __asm(" BX R0 ");
  return 0;
}
