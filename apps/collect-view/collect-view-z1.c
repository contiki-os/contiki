#include "collect-view.h"
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"

#include "collect-view-z1.h"

/*---------------------------------------------------------------------------*/
static uint16_t
get_temp()
{
  /* XXX Fix me: check /examples/z1/test-tmp102.c for correct conversion */
  return (uint16_t)tmp102_read_temp_raw();
}
/*---------------------------------------------------------------------------*/
void
collect_view_arch_read_sensors(struct collect_view_data_msg *msg)
{
  static int initialized = 0;

  if(!initialized) {
    tmp102_init();
    initialized = 1;
  }

  msg->sensors[BATTERY_VOLTAGE_SENSOR] = 0;
  msg->sensors[BATTERY_INDICATOR] = 0;
  msg->sensors[LIGHT1_SENSOR] = 0;
  msg->sensors[LIGHT2_SENSOR] = 0;
  msg->sensors[TEMP_SENSOR] = get_temp();
  msg->sensors[HUMIDITY_SENSOR] = 0;
}
/*---------------------------------------------------------------------------*/
