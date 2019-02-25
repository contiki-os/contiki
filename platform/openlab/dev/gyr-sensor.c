#include "contiki.h"
#include "lib/sensors.h"
#include "dev/gyr-sensor.h"

PROCESS(gyr_update, "gyr_update");

const struct sensors_sensor gyr_sensor;

static struct {
  int active;

  l3g4200d_datarate_t datarate;
  l3g4200d_scale_t scale;
  int sensitivity;

  int16_t xyz[3];
} conf = {0};


// Sensitivity from l3g4200d documentation page 10/42
/* Sensitivity for gyr scales values >> 4 (0-3) */
// (u°/s)/LSB  (using micro intead of milli to store sensitivity as integer)

static const int gyr_scale_sens[] = {8750, 17500, 0x0, 70000};
static void measure_isr(void *arg);

/*---------------------------------------------------------------------------*/

/** Gyroscope rotation speed in m°/s
 * type == axis: X Y Z */
static int value(int type)
{
  int64_t raw = 0;
  switch (type) {
    case GYR_SENSOR_X:
    case GYR_SENSOR_Y:
    case GYR_SENSOR_Z:
      raw = conf.xyz[type];
      return (int32_t)((raw * conf.sensitivity) / 1000);
    default:
      return 0;  // invalid argument
  }
}
/*---------------------------------------------------------------------------*/

static int status(int type)
{
  return conf.active;
}

/*---------------------------------------------------------------------------*/

static void gyr_start()
{
  // update sensitivity when updating hardware config
  conf.sensitivity = gyr_scale_sens[conf.scale >> 4];

  process_start(&gyr_update, NULL);
  l3g4200d_set_drdy_int(measure_isr, NULL);
  l3g4200d_gyr_config(conf.datarate, conf.scale, 1);

  // bootstrap IRQ
  l3g4200d_read_rot_speed(conf.xyz);

}

static void gyr_stop()
{
  l3g4200d_set_drdy_int(NULL, NULL);
  l3g4200d_powerdown();

  process_exit(&gyr_update);
}

static int configure(int type, int c)
{
  switch (type) {
    case SENSORS_HW_INIT:
      configure(SENSORS_ACTIVE, 0);
      configure(GYR_SENSOR_DATARATE, L3G4200D_100HZ);
      configure(GYR_SENSOR_SCALE, L3G4200D_250DPS);
      break;

    case SENSORS_ACTIVE:
      if ((conf.active = c))
        gyr_start();
      else
        gyr_stop();
      break;
    case SENSORS_READY:
      return conf.active;
      break;

    /*
     * Configuration
     */
    case GYR_SENSOR_DATARATE:
      conf.datarate = c;
      break;
    case GYR_SENSOR_SCALE:
      conf.scale = c;
      break;

    default:
      return 1;  // error
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

static void measure_isr(void *arg)
{
  process_poll(&gyr_update);
}

/*
 * Values are updated by a process, to ensure that when reading
 * X, then Y, then Z, in a process, the values are coherent when not yielding
 *
 * And values MUST be read at each interrupt. No new value if not read.
 */

PROCESS_THREAD(gyr_update, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(l3g4200d_read_drdy());

    l3g4200d_read_rot_speed(conf.xyz);
    sensors_changed(&gyr_sensor);
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(gyr_sensor, "Gyroscope", value, configure, status);
