#include <AT91SAM7S64.h>
#include <interrupt-utils.h>
#include <usb-interrupt.h>
#include <usb-proto.h>
#include <stdio.h>

static void
usb_int_safe (void) __attribute__((noinline));

static void 
usb_int_safe (void)
{
  unsigned int int_status;
  /* putchar('*'); */
  int_status = *AT91C_UDP_ISR & *AT91C_UDP_IMR;
  
   if (int_status &  (AT91C_UDP_EP1 | AT91C_UDP_EP2 | AT91C_UDP_EP3)) {
     usb_epx_int();
   } else if (int_status &  AT91C_UDP_ENDBUSRES) {
    usb_reset();
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;
  } else if (int_status &  AT91C_UDP_RXSUSP) {
    /* puts("Suspend"); */
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP;
  } else if (int_status &  AT91C_UDP_RXRSM) {
    /* puts("Resume"); */
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM;
  } else if (int_status &  AT91C_UDP_SOFINT) {
    /* puts("SOF"); */
    *AT91C_UDP_ICR = AT91C_UDP_SOFINT;
  } else if (int_status &  AT91C_UDP_WAKEUP) {
    /* puts("Wakeup"); */
    *AT91C_UDP_ICR = AT91C_UDP_WAKEUP;
  } else if (int_status &  AT91C_UDP_EP0) {
    usb_ep0_int();
  } else {
    puts("Other USB interrupt"); 
  }
  /* putchar('<'); */

}

void NACKEDFUNC
usb_int (void)
{
  ISR_STORE();
  ISR_ENABLE_NEST();
  usb_int_safe();
  ISR_DISABLE_NEST();
  *AT91C_AIC_EOICR = 0; 
  ISR_RESTORE();
}
