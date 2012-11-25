#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <sys/autostart.h>
#include <sys/clock.h>
#include "debug-uart.h"

unsigned int idle_count = 0;

int
main()
{
  setup_debug_uart();
  printf("\rInitializing\r\n");
  clock_init();
  process_init();
  process_start(&etimer_process, NULL );
  autostart_start(autostart_processes);

  printf("\rProcesses running\r\n");
  while (1)
    {
      do
        {
        }
      while (process_run() > 0);
      idle_count++;
      /* Idle! */
      /* Stop processor clock */
      /* asm("wfi"::); */
    }
  return 0;
}

