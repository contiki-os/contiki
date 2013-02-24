#include <stdint.h>
#include <stdio.h>
#include <debug-uart.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include <clock.h>

#include <libopencm3/stm32/f1/rcc.h>
unsigned int idle_count = 0;

static void
configure_mcu_clocks(void)
{
  rcc_clock_setup_in_hse_25mhz_out_72mhz();

  /* GPIO Clocks */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);

  /* USART 1 */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART1EN);
}

int
main()
{
  configure_mcu_clocks();
  uart_init(115200);
  printf("Initialising\n");

  clock_init();
  process_init();
  process_start(&etimer_process, NULL);
  autostart_start(autostart_processes);
  printf("Processes running\n");
  while(1) {
    do {
    } while(process_run() > 0);
    idle_count++;
  }
  return 0;
}
