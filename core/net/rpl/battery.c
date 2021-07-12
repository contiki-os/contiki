#include "contiki.h"
#include "net/rpl/battery.h"
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"


void battery_charge_set(void)
{
  energest_flush();

  uint32_t cpu = energest_type_time(ENERGEST_TYPE_CPU);
  uint32_t cpu_idle = energest_type_time(ENERGEST_TYPE_LPM);
  uint32_t tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  uint32_t rx = energest_type_time(ENERGEST_TYPE_LISTEN);

  uint32_t energy = ((tx * current_tx + rx * current_rx + \
   cpu * current_cpu + cpu_idle * current_cpu_idle) * 3L) /RTIMER_ARCH_SECOND;

  battery_charge_value=energy/CURRENT_UNIT;
}