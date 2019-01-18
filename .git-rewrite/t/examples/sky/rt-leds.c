
#include "contiki.h"
#include "sys/rtimer.h"

#include "dev/leds.h"

/*---------------------------------------------------------------------------*/
PROCESS(test_rtimer_process, "Test RT");
AUTOSTART_PROCESSES(&test_rtimer_process);

struct fade {
  struct rtimer rt;
  struct pt pt;
  int led;
  rtimer_clock_t ontime, offtime;
  int addend;
};
/*---------------------------------------------------------------------------*/
static char
fade(struct rtimer *t, void *ptr)
{
  struct fade *f = ptr;

  PT_BEGIN(&f->pt);

  while(1) {
    leds_on(f->led);
    rtimer_set(t, RTIMER_TIME(t) + f->ontime, 1,
	       (rtimer_callback_t)fade, ptr);
    PT_YIELD(&f->pt);
    
    leds_off(f->led);
    rtimer_set(t, RTIMER_TIME(t) + f->offtime, 1,
	       (rtimer_callback_t)fade, ptr);

    f->ontime += f->addend;
    f->offtime -= f->addend;
    if(f->offtime <= 4 || f->offtime >= 100) {
      f->addend = -f->addend;
    }
    PT_YIELD(&f->pt);
  }

  PT_END(&f->pt);
}
/*---------------------------------------------------------------------------*/
static void
init_fade(struct fade *f, int led)
{
  f->led = led;
  f->addend = 4;
  f->ontime = 4;
  f->offtime = 100;
  PT_INIT(&f->pt);
  rtimer_set(&f->rt, RTIMER_NOW() + led, 1, (rtimer_callback_t)fade, f);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_rtimer_process, ev, data)
{
  static struct fade red;
  PROCESS_BEGIN();
  
  rtimer_init();

  init_fade(&red, LEDS_RED);
  
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
