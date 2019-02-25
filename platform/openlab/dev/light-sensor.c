#include "contiki.h"
#include "lib/sensors.h"
#include "dev/light-sensor.h"
#include "periph/isl29020.h"

const struct sensors_sensor light_sensor;

static struct {
  isl29020_light_t light;
  isl29020_resolution_t resolution;
  isl29020_range_t range;

  int active;
} conf = {0};

static int value(int type)
{
  float value = isl29020_read_sample();
  return (int)(value * LIGHT_SENSOR_VALUE_SCALE);
}

/*---------------------------------------------------------------------------*/

static int status(int type)
{
  return conf.active;
}

/*---------------------------------------------------------------------------*/

static void light_start()
{
  isl29020_prepare(conf.light, conf.resolution, conf.range);
  isl29020_sample_continuous();
}
static void light_stop()
{
  isl29020_powerdown();
}

static int configure(int type, int c)
{
  switch (type) {
    case SENSORS_HW_INIT:
      configure(SENSORS_ACTIVE, 0);
      configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
      configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
      configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
      break;
    case SENSORS_ACTIVE:
      if ((conf.active = c))
        light_start();
      else
        light_stop();
      break;
    case SENSORS_READY:
      return conf.active;  // return value
      break;

    // Configuration
    case LIGHT_SENSOR_SOURCE:
      conf.light = c;
      break;
    case LIGHT_SENSOR_RESOLUTION:
      conf.resolution = c;
      break;
    case LIGHT_SENSOR_RANGE:
      conf.range = c;
      break;

    default:
      return 1;  // error
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(light_sensor, "Light", value, configure, status);
