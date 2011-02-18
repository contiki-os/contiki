#include "contiki.h"

#include "dev/battery-sensor.h"
#include <stdio.h> /* For printf() */


/*---------------------------------------------------------------------------*/
PROCESS(aplicacio, "Aplicacio de prova");
AUTOSTART_PROCESSES(&aplicacio);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aplicacio, ev, data)
{

  PROCESS_BEGIN();
  
  SENSORS_ACTIVATE(battery_sensor);
  
  while (1)
  {
  
    uint16_t bateria = battery_sensor.value(0);
  
    printf("Battery: %i\n", bateria);
  }
  
  SENSORS_DEACTIVATE(battery_sensor);
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

