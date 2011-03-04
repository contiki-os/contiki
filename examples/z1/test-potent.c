#include "contiki.h"

#include "dev/potentiometer-sensor.h"
#include <stdio.h> /* For printf() */


/*---------------------------------------------------------------------------*/
PROCESS(aplicacio, "Testing Potentiometer");
AUTOSTART_PROCESSES(&aplicacio);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aplicacio, ev, data)
{

  PROCESS_BEGIN();
  
  // INSERT LINE HERE TO ENABLE POTENTIOMETER
 
  while (1)
  {
  
    uint16_t value
   //INSERT LINE HERE TO READ POTENTIOMETER VALUE
 
    printf("Potentiometer Value: %i\n", v);
  }
  
  SENSORS_DEACTIVATE(potentiometer_sensor);
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

