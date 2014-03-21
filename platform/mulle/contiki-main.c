#include "stdio.h"

#include "contiki.h"
#include "sys/autostart.h"
#include "dev/leds.h"

#include "MK60N512VMD100.h"
#include "uart.h"
#include "udelay.h"
#include "init-net.h"
#include "power-control.h"

void printf_putc(void* dum, char c)
{
        uart_putchar(c);
}

/* C entry point (after startup code has executed) */
int main(void)
{
  leds_arch_init();
  uart_init();
  power_control_init();
  udelay_init();
  udelay(0xFFFF);
  udelay(0xFFFF);
#ifndef WITH_SLIP
  init_printf(0, &printf_putc);
  printf("Booted\n");
#endif
  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);

  ctimer_init();

  clock_init();
  init_net();

  autostart_start(autostart_processes);

  while (1)
  {
    process_run();
  }
}
