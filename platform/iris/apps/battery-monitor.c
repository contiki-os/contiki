#include "contiki.h"
#include "dev/battery-sensor.h"
#include "lib/sensors.h"
#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(battery_monitor_process, "Battery Voltage Monitor");
AUTOSTART_PROCESSES(&battery_monitor_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(battery_monitor_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(battery_sensor);

  while(1) {

    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    /*
     * Battery voltage calculation formula
     *
     *     V(Battery Voltage) = v(Voltage Reference) * 1024 / ADC
     *
     *     Where:
     *          v = 1.223
     * 
     */
    printf("ADC value : %d\n", battery_sensor.value(0));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
