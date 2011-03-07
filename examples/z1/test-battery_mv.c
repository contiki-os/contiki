#include "contiki.h"
#include "dev/battery-sensor.h"
#include <stdio.h> /* For printf() */


float floor(float x){
 if(x>=0.0f){ return (float) ((int)x);}
else {return(float)((int)x-1);}
}

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
 float mv = (bateria*2.500*2)/4096;
 printf("Battery: %i (%ld.%03d mV)\n", bateria, (long) mv, (unsigned) ((mv-floor(mv))*1000));
  }
  
  SENSORS_DEACTIVATE(battery_sensor);
  
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

