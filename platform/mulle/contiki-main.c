#include "contiki.h"

#include <stdio.h>

#include "sys/autostart.h"
#include "dev/leds.h"

#include "cfs.h"
#include "cfs-coffee.h"
#include "xmem.h"

#include "core-clocks.h"
#include "uart.h"
#include "udelay.h"
#include "llwu.h"
#include "init-net.h"
#include "power-control.h"
#include "K60.h"

#define DEBUG 1
#if DEBUG
#define PRINTF(...)     printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

void printf_putc(void* dum, char c)
{
  uart_putchar(BOARD_DEBUG_UART_BASE_PTR, c);
}

/*---------------------------------------------------------------------------*/
#define COFFEE_AUTO_FORMAT 1
static void
init_cfs()
{
  int fd;
  PRINTF("Initialize xmem and coffee...\n");
  xmem_init();
  PRINTF("Xmem initialized.\n");
#ifdef COFFEE_AUTO_FORMAT
  if ((fd = cfs_open("formated", CFS_READ)) == -1)
  {
    // Storage is not formated
    PRINTF("Coffee not formated\n");
    if (cfs_coffee_format() == -1)
    {
      // Format failed, bail out
      PRINTF("Failed to format coffee, bail out\n");
      return;
    }
    if ((fd = cfs_open("formated", CFS_WRITE)) == -1)
    {
      // Failed to open file to indicate formated state.
      PRINTF("Failed to open file to indicate formated state\n");
      return;
    }
    cfs_write(fd, "DO NOT REMOVE!", strlen("DO NOT REMOVE!"));
  }
  cfs_close(fd);
#endif // COFFEE_AUTO_FORMAT
  PRINTF("Coffee initialized.\r\n");
}
/*---------------------------------------------------------------------------*/
LLWU_CONTROL(deep_sleep);

/* C entry point (after startup code has executed) */
int main(void)
{
  leds_arch_init();
  /* Set up core clocks so that timings will be correct in all modules */
  core_clocks_init();
  dbg_uart_init();

  llwu_init();
  llwu_enable_wakeup_module(LLWU_WAKEUP_MODULE_LPTMR);
  llwu_register(deep_sleep);
  // Dont allow deep sleep for now because radio cant wake up the mcu from it.
  // TODO(Henrik) Fix this when a new revision is made of the hardware.
#ifndef WITH_SLIP
  llwu_set_allow(deep_sleep, 1);
#else
  llwu_set_allow(deep_sleep, 0);
#endif

  power_control_init();

  /* Turn on power to the on board peripherals */
  power_control_vperiph_set(1);

  udelay_init();
  udelay(0xFFFF);
  udelay(0xFFFF);
#ifndef WITH_SLIP
  init_printf(0, &printf_putc);
  printf("Booted\n");
  printf("CPUID: %x\n", SCB_CPUID);
#endif
  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&etimer_process, NULL);

  ctimer_init();

  clock_init();
  init_cfs();
  init_net();

  autostart_start(autostart_processes);

  while (1)
  {
    while (process_run() > 0);
    llwu_sleep();
  }
}
