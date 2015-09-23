#include "contiki.h"
//#include "contiki-lib.h"
#include <stdio.h>
#include "contiki-net.h"
#include "dev/radio.h"
#include "dev/cc2538-rf.h"

static radio_value_t value;
//UTIL
static radio_result_t
get_param(radio_param_t param, radio_value_t *value)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.get_value(param, value);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value %d is invalid\n", *value);
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }
  return rv;
}

/*---------------------------------------------------------------------------*/
static radio_result_t
set_param(radio_param_t param, radio_value_t value)
{
  radio_result_t rv;

  rv = NETSTACK_RADIO.set_value(param, value);

  switch(rv) {
  case RADIO_RESULT_ERROR:
    printf("Radio returned an error\n");
    break;
  case RADIO_RESULT_INVALID_VALUE:
    printf("Value %d is invalid\n", value);
    break;
  case RADIO_RESULT_NOT_SUPPORTED:
    printf("Param %u not supported\n", param);
    break;
  case RADIO_RESULT_OK:
    break;
  default:
    printf("Unknown return value\n");
    break;
  }

  return rv;
}

static void
do_rssi(void)
{
  int channel;
  printf("RSSI");
  for(channel = CC2538_RF_CHANNEL_MIN; channel <=CC2538_RF_CHANNEL_MAX; ++channel) {
    set_param(RADIO_PARAM_CHANNEL, channel);
    if(get_param(RADIO_PARAM_RSSI, &value) == RADIO_RESULT_OK) {
      printf("%d ", (value));
    }
  }
  printf("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS(scanner_process, "RSSI Scanner");
AUTOSTART_PROCESSES(&scanner_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(scanner_process, ev, data)
{

  static struct stimer st;
  PROCESS_BEGIN();
  //printf("Starting RSSI scanner\n");
  /* switch mac layer off, and turn radio on */
  NETSTACK_MAC.off(0);
  cc2538_rf_driver.init();

  while(1) {
      do_rssi();
      /* Delay 2-4 seconds */
      stimer_set(&st, CLOCK_SECOND * 60);
 
      PROCESS_PAUSE();
//    PROCESS_YIELD();
//    printf("RSSI: ");
//    if(get_param(RADIO_PARAM_RSSI, &value) == RADIO_RESULT_OK) {
//      printf("%d dBm [0x%04x]\n", value, (uint16_t)value);
//    }
//
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

