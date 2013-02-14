#include "contiki.h"
#include "shell.h"
#include "dev/watchdog.h"

#include <stdio.h>
#include <string.h>

extern void start_apps(void);

PROCESS_NAME(tcpip_process);

/*---------------------------------------------------------------------------*/
PROCESS(shell_fast_reboot_process, "reboot");
SHELL_COMMAND(fast_reboot_command,
	      "reboot",
	      "reboot: reboot the system",
	      &shell_fast_reboot_process);
PROCESS(shell_start_6lbr_process, "6lbr");
SHELL_COMMAND(start6lbr_command,
	      "start6lbr",
	      "start6lbr: Start IP stack and 6LBR apps",
	      &shell_start_6lbr_process);
PROCESS(shell_rfchannel_process, "rfchannel");
SHELL_COMMAND(rfchannel_command,
	      "rfchannel",
	      "rfchannel <channel>: change CC2420 radio channel (11 - 26)",
	      &shell_rfchannel_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_fast_reboot_process, ev, data)
{
  PROCESS_BEGIN();

  shell_output_str(&fast_reboot_command,
		   "Rebooting the node...", "");

  watchdog_reboot();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_start_6lbr_process, ev, data)
{
  PROCESS_BEGIN();

  shell_output_str(&start6lbr_command,
		   "Starting 6lbr apps...", "");

  process_start(&tcpip_process, NULL);
  start_apps();

  shell_output_str(&start6lbr_command,
		   "done", "");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rfchannel_process, ev, data)
{
  uint16_t channel;
  const char *newptr;
  PROCESS_BEGIN();

  channel = shell_strtolong(data, &newptr);

  if(newptr != data) {
    cc2420_set_channel(channel);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_6lbr_init(void)
{
  shell_register_command(&fast_reboot_command);
  shell_register_command(&start6lbr_command);
  shell_register_command(&rfchannel_command);
}
/*---------------------------------------------------------------------------*/
