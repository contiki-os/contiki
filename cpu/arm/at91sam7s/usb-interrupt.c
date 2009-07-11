#include <AT91SAM7S64.h>
#include <interrupt-utils.h>
#include <usb-interrupt.h>
#include <usb-api.h>
#include <stdio.h>

static void
usb_int_safe (void) __attribute__((noinline));

static void 
usb_int_safe (void)
{
  unsigned int int_status;
  /* putchar('*'); */
  int_status = *AT91C_UDP_ISR & *AT91C_UDP_IMR;
  
  if (int_status &  (AT91C_UDP_EP0 | AT91C_UDP_EP1 | AT91C_UDP_EP2
		     | AT91C_UDP_EP3)) {
    unsigned int ep_index;
    /* Handle enabled interrupts */
    for (ep_index = 0; ep_index < 4; ep_index++) {
      if (int_status & (1<<ep_index)) {
	usb_arch_transfer_complete(ep_index);
      }
    }
  } else if (int_status &  AT91C_UDP_ENDBUSRES) {
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;
    usb_arch_reset_int();
  } else if (int_status &  AT91C_UDP_RXSUSP) {
    /* puts("Suspend"); */
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP;
    usb_arch_suspend_int();
  } else if (int_status &  AT91C_UDP_RXRSM) {
    /* puts("Resume"); */
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM;
    usb_arch_resume_int();
  } else if (int_status &  AT91C_UDP_SOFINT) {
    /* puts("SOF"); */
    *AT91C_UDP_ICR = AT91C_UDP_SOFINT;
  } else if (int_status &  AT91C_UDP_WAKEUP) {
    /* puts("Wakeup"); */
    *AT91C_UDP_ICR = AT91C_UDP_WAKEUP;
  } else {
    puts("Other USB interrupt"); 
  }
  /* dbg_putchar('<'); */

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
