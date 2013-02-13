#include "contiki.h"
#include "contiki-lib.h"

#include "cetic-6lbr.h"
#include "nvm-config.h"
#include "slip-cmds.h"

void
platform_init(void)
{
  process_start(&border_router_cmd_process, NULL);
}
