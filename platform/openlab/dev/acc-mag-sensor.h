#ifndef ACC_MAG_SENSOR_H_
#define ACC_MAG_SENSOR_H_

#include "lib/sensors.h"
#include "lsm303dlhc.h"

/** Accelerometer and magnetometer available values */
enum {
  ACC_MAG_SENSOR_X = 0,
  ACC_MAG_SENSOR_Y = 1,
  ACC_MAG_SENSOR_Z = 2,
};

/** Configure types */
enum {
  ACC_MAG_SENSOR_DATARATE = 0,
  ACC_MAG_SENSOR_SCALE = 1,
  ACC_MAG_SENSOR_MODE = 2,
};

extern const struct sensors_sensor acc_sensor;
extern const struct sensors_sensor mag_sensor;

#endif /* ACC_MAG_SENSOR_H_ */
