#include "contiki.h"

#if SHELL
#include "shell.h"
#include "serial-shell.h"
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include "shell-6lbr.h"
#endif

PROCESS(demo_6lbr_process, "6LBR Demo");

PROCESS_NAME(web_sense_process);
PROCESS_NAME(webserver_nogui_process);
PROCESS_NAME(udp_client_process);

AUTOSTART_PROCESSES(&demo_6lbr_process);

/*---------------------------------------------------------------------------*/
void
start_apps(void)
{
#if UDPCLIENT
  process_start(&udp_client_process, NULL);
#endif

#if WEBSERVER
  process_start(&web_sense_process, NULL);
  process_start(&webserver_nogui_process, NULL);
#endif
}

PROCESS_THREAD(demo_6lbr_process, ev, data)
{
  PROCESS_BEGIN();

#if SHELL
  uart1_set_input(serial_line_input_byte);
  serial_line_init();

  serial_shell_init();
  shell_ping_init();
  shell_6lbr_init();
#else
  start_apps();
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
