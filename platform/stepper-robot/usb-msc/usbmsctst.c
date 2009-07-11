#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/process.h>
#include <sys/autostart.h>
#include <sys/etimer.h>
#include <usb-api.h>
#include <msc/usb-msc-bulk.h>

PROCESS(usb_masstest_process, "USB massstorage test process");

#if 0
extern unsigned int idle_count;
#endif

PROCESS_THREAD(usb_masstest_process, ev , data)
{
  static unsigned int last_count;
  PROCESS_BEGIN();
  static struct etimer timer;
  usb_msc_bulk_setup();
  etimer_set(&timer, CLOCK_SECOND);
#if 0
  last_count = idle_count;
#endif
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
#if 0
      printf("Idle count: %d\n", idle_count - last_count);
      last_count = idle_count;
#endif
      etimer_reset(&timer);
    }
  }
  PROCESS_END();
}


AUTOSTART_PROCESSES(&usb_masstest_process);
