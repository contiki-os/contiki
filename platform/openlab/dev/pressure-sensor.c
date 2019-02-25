#include "contiki.h"
#include "lib/sensors.h"
#include "dev/pressure-sensor.h"

const struct sensors_sensor pressure_sensor;

struct {
  int active;
  lps331ap_datarate_t datarate;
} conf = {0};

/*---------------------------------------------------------------------------*/
/** Pressure in mbar */
static int value(int type)
{
  uint32_t raw;
  lps331ap_read_pres(&raw);
  return raw;
}

static int status(int type)
{
  return conf.active;
}


/*---------------------------------------------------------------------------*/

static void pressure_start()
{
  lps331ap_set_datarate(conf.datarate);
  // could not get interrupt working
  // lps331ap_set_drdy_int(measure_isr, NULL);
}
static void pressure_stop()
{
  lps331ap_powerdown();
}

static int configure(int type, int c)
{
  switch (type) {
    case SENSORS_HW_INIT:
      configure(SENSORS_ACTIVE, 0);
      configure(PRESSURE_SENSOR_DATARATE, LPS331AP_P_1HZ_T_1HZ);
      break;
    case SENSORS_ACTIVE:
      if ((conf.active = c))
        pressure_start();
      else
        pressure_stop();
      break;
    case SENSORS_READY:
      return conf.active;  // return value
      break;

    /* Configuration */
    case PRESSURE_SENSOR_DATARATE:
      conf.datarate = c;
      break;

    default:
      return 1;  // error
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(pressure_sensor, "Pressure sensor", value, configure, status);
