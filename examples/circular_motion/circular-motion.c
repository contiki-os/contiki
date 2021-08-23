#include "contiki.h"
#include "random.h"


#include <stdio.h>
#include <math.h>

static float pos_x=100;
static float pos_y=0;


/*---------------------------------------------------------------------------*/
PROCESS(circular_motion_process, "CIRCULAR MOTION example");
AUTOSTART_PROCESSES(&circular_motion_process);
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(circular_motion_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  random_init(25);

  

  static float ang_velocity=0.5;
  static float del_t=0.001;

  
  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, del_t*CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    
 
    pos_x = pos_x - (pos_y*ang_velocity*del_t);
    pos_y = pos_y + (pos_x*ang_velocity*del_t);


  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
