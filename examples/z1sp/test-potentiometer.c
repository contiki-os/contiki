#include "contiki.h"

#include "dev/potentiometer-sensor.h"
#include <stdio.h> /* For printf() */


/*---------------------------------------------------------------------------*/
PROCESS(aplicacio, "Aplicacio de prova");
AUTOSTART_PROCESSES(&aplicacio);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aplicacio, ev, data)
{

  PROCESS_BEGIN();
  
  SENSORS_ACTIVATE(potentiometer_sensor);
  
  while (1)
  {
  
    uint16_t v = potentiometer_sensor.value(0);
  
    printf("Potentiometer Value: %i\n", v);
  }
  
  SENSORS_DEACTIVATE(potentiometer_sensor);
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

