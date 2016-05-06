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

  GPIODirModeSet(GPIO_PIN_15, GPIO_DIR_MODE_OUT);
  GPIOPinWrite(GPIO_PIN_15, 1);

  // Load address of reset function from the fixed location of the image's
  // reset vector and jump.
  /**
   *  Initial attempt
   */
  /*
  __asm(" LDR R0, =0x1004 ");
  __asm(" BX R0 ");
  */

  // This part throws a compiler error
  /*
  __asm("LDR R0, =0x1000");
  __asm("MSR VTOR, R0");
  __asm("ISB");
  */

  /**
   *  Recommendation per G. Dyess (ARM)
   */
  /*
   uint8_t counter;
   uint32_t *vectorTable =  (uint32_t*) 0x20000000;
   uint32_t *flashVectors = 0x1000;

   // Write image specific interrupt vectors into RAM vector table.
   for(counter = 0; counter < 15; ++counter)
   {
     *vectorTable++ = *flashVectors++;
   }
   */

  /*
  __asm("LDR R0, =0x1004"); //  RESET vector of target image
  __asm("LDR R1, [R0]");    //  Get the branch address
  __asm("ORR R1, #1");      //  Make sure the Thumb State bit is set.
  __asm("BX R1");           //  Branch execution
  */

  return 0;
}
