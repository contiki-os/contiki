#include "contiki.h"
#include "lib/sensors.h"
#include "dev/acc-mag-sensor.h"

PROCESS(acc_mag_update, "acc_mag_update");

const struct sensors_sensor acc_sensor;
const struct sensors_sensor mag_sensor;

static struct {
  struct {
    int active;

    lsm303dlhc_acc_datarate_t datarate;
    lsm303dlhc_acc_scale_t scale;
    int sensitivity;
    lsm303dlhc_acc_update_t update;

    int new_val;
    int16_t xyz[3];
  } acc;
  struct {
    int active;

    lsm303dlhc_mag_datarate_t datarate;
    lsm303dlhc_mag_scale_t scale;
    int sensitivity;
    lsm303dlhc_acc_update_t update;

    int new_val;
    int16_t xyz[3];
  } mag;
} conf = {{0}, {0}};


// Sensitivity from lsm303dlhc documentation page 10/42
/* Sensitivity for acc scales values >> 4 (0-3) */
static const int acc_scale_sens[] = {1, 2, 4, 12};
/* Sensitivity for mag scales values >> 6 (0-3) */
static const int mag_scale_sens[] = {1100, 855, 670, 450, 400, 330, 230};

static void measure_isr(void *arg);

/*---------------------------------------------------------------------------*/
/** Accelerometer value in mg
 * type == axis: X Y Z */
static int acc_value(int type)
{
  int raw = 0;
  switch (type) {
    case ACC_MAG_SENSOR_X:
    case ACC_MAG_SENSOR_Y:
    case ACC_MAG_SENSOR_Z:
      raw = conf.acc.xyz[type];
      return raw / conf.acc.sensitivity;
    default:
      return 0;  // invalid argument
  }
}

/*---------------------------------------------------------------------------*/
/** Magnetometer value in mgauss
 * type == axis: X Y Z */
static int mag_value(int type)
{
  int32_t raw = 0;
  switch (type) {
    case ACC_MAG_SENSOR_X:
    case ACC_MAG_SENSOR_Y:
    case ACC_MAG_SENSOR_Z:
      raw = conf.mag.xyz[type];
      return (1000 * raw) / conf.mag.sensitivity;
    default:
      return 0;  // invalid argument
  }
}
/*---------------------------------------------------------------------------*/

static int acc_status(int type)
{
  return conf.acc.active;
}

static int mag_status(int type)
{
  return conf.mag.active;
}

/*---------------------------------------------------------------------------*/

static void acc_start()
{
  // update sensitivity when updating hardware config
  conf.acc.sensitivity = acc_scale_sens[conf.acc.scale >> 4];

  process_start(&acc_mag_update, NULL);
  lsm303dlhc_acc_config(conf.acc.datarate, conf.acc.scale, conf.acc.update);
  lsm303dlhc_acc_set_drdy_int1(measure_isr, &conf.acc.new_val);
  lsm303dlhc_read_acc(conf.acc.xyz);  // start IRQs
}

static void acc_stop()
{
  lsm303dlhc_acc_set_drdy_int1(NULL, NULL);
}

static int acc_configure(int type, int c)
{
  switch (type) {
    case SENSORS_HW_INIT:
      // default config
      acc_configure(SENSORS_ACTIVE, 0);
      acc_configure(ACC_MAG_SENSOR_DATARATE, LSM303DLHC_ACC_RATE_100HZ);
      acc_configure(ACC_MAG_SENSOR_SCALE, LSM303DLHC_ACC_SCALE_2G);
      acc_configure(ACC_MAG_SENSOR_MODE, LSM303DLHC_ACC_UPDATE_ON_READ);
      break;
    case SENSORS_ACTIVE:
      if ((conf.acc.active = c))
        acc_start();
      else
        acc_stop();

      // stop if no more sensors activated
      if (!conf.acc.active && !conf.mag.active) {
        lsm303dlhc_powerdown();
        process_exit(&acc_mag_update);
      }
      break;
    case SENSORS_READY:
      return conf.acc.active;  // return value
      break;

    /*
     * Configuration
     */
    case ACC_MAG_SENSOR_DATARATE:
      conf.acc.datarate = c;
      break;
    case ACC_MAG_SENSOR_SCALE:
      conf.acc.scale = c;
      break;
    case ACC_MAG_SENSOR_MODE:
      // xyz values integrity not ensured with UPDATE_CONTINUOUS
      conf.acc.update = c;
      break;

    default:
      return 1;  // error
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static int mag_configure(int type, int c);

static void mag_start()
{
  // update sensitivity when updating hardware config
  conf.mag.sensitivity = mag_scale_sens[conf.mag.scale >> 6];

  process_start(&acc_mag_update, NULL);
  lsm303dlhc_mag_config(conf.mag.datarate, conf.mag.scale, conf.mag.update,
      LSM303DLHC_TEMP_MODE_OFF);
  lsm303dlhc_mag_set_drdy_int(measure_isr, &conf.mag.new_val);
  lsm303dlhc_read_mag(conf.mag.xyz);  // start IRQs
}

static void mag_stop()
{
  mag_configure(ACC_MAG_SENSOR_MODE, LSM303DLHC_MAG_MODE_OFF);
  lsm303dlhc_mag_config(conf.mag.datarate, conf.mag.scale, conf.mag.update,
      LSM303DLHC_TEMP_MODE_OFF);
  lsm303dlhc_mag_set_drdy_int(NULL, NULL);
}

static int mag_configure(int type, int c)
{
  switch (type) {
    case SENSORS_HW_INIT:
      mag_configure(SENSORS_ACTIVE, 0);
      mag_configure(ACC_MAG_SENSOR_DATARATE, LSM303DLHC_MAG_RATE_0_75HZ);
      mag_configure(ACC_MAG_SENSOR_SCALE, LSM303DLHC_MAG_SCALE_1_3GAUSS);
      mag_configure(ACC_MAG_SENSOR_MODE, LSM303DLHC_MAG_MODE_CONTINUOUS);
      break;
    case SENSORS_ACTIVE:
      if ((conf.mag.active = c))
        mag_start();
      else
        mag_stop();

      // stop if no more sensors activated
      if (!conf.acc.active && !conf.mag.active) {
        lsm303dlhc_powerdown();
        process_exit(&acc_mag_update);
      }
      break;
    case SENSORS_READY:
      return conf.mag.active;  // return value
      break;

    /*
     * Configuration
     */
    case ACC_MAG_SENSOR_DATARATE:
      conf.mag.datarate = c;
      break;
    case ACC_MAG_SENSOR_SCALE:
      conf.mag.scale = c;
      break;
    case ACC_MAG_SENSOR_MODE:
      // xyz values integrity not ensured with UPDATE_CONTINUOUS
      // Mode 'single shot' not managed
      conf.mag.update = c;
      break;

    default:
      return 1;  // error
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static void measure_isr(void *arg)
{
  if (arg)
    *(int *)arg = 1;
  // For accelerometer
  // tried with lsm303dlhc_read_acc_async but it messed up i2c chip
  process_poll(&acc_mag_update);
}

/*
 * Values are updated by a process, to ensure that when reading
 * X, then Y, then Z, in a process, the values are coherent when not yielding
 */

PROCESS_THREAD(acc_mag_update, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(conf.acc.new_val || conf.mag.new_val);

    // pin value is quite always non zero, so I don't use it
    // lsm303dlhc_mag_get_drdy_pin_value())
    if (conf.mag.new_val) {
      conf.mag.new_val = 0;
      lsm303dlhc_read_mag(conf.mag.xyz);
      sensors_changed(&mag_sensor);
    }

    if (conf.acc.new_val) {
      conf.acc.new_val = 0;
      while (lsm303dlhc_acc_get_drdy_int1_pin_value())
        lsm303dlhc_read_acc(conf.acc.xyz);
      sensors_changed(&acc_sensor);
    }

  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(acc_sensor, "Accelerometer", acc_value, acc_configure,
    acc_status);

SENSORS_SENSOR(mag_sensor, "Magnetometer", mag_value, mag_configure,
    mag_status);
