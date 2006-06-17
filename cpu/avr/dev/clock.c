
#include "sys/clock.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// Hack to see if this sets the blue led...
/* #include "hal_emwinet_demoboard.h" */

static unsigned short count;


/*---------------------------------------------------------------------------*/
SIGNAL(SIG_OUTPUT_COMPARE0)
{
/*   if ((count % 10) > 5) SET_BLUE_LED(); */
/*   else CLEAR_BLUE_LED(); */

  ++count;
  if(etimer_pending()) {
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
#if 0
#if __AVR_ENHANCED__
  outp(_BV(CS00) | _BV(CS02) | _BV(WGM01), TCCR0);
#else
  outp(_BV(CS00) | _BV(CS02) | _BV(CTC0), TCCR0);
#endif
  outp(0, TCNT0);
  outp((NUT_CPU_FREQ / (128L * CLOCK_CONF_SECOND) + 0.5/*round*/), OCR0);
  sbi(TIMSK, OCIE0);
#endif /* 0 */


  TCCR0 = _BV(CS00) | _BV(CS02) | _BV(WGM01);
  TCNT0 = 0;
  /*  OCR0 = AVR_CLK_COUNT;*/
  TIMSK |= _BV(OCIE0);

  count = 0;
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
