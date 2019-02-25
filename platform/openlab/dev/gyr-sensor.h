#ifndef GYR_SENSOR_H_
#define GYR_SENSOR_H_

#include "lib/sensors.h"
#include "l3g4200d.h"

/** Gyroscope available values */
enum {
  GYR_SENSOR_X = 0,
  GYR_SENSOR_Y = 1,
  GYR_SENSOR_Z = 2,
};

/** Configure types */
enum {
  GYR_SENSOR_DATARATE = 0,
  GYR_SENSOR_SCALE = 1,
};

extern const struct sensors_sensor gyr_sensor;

#endif /* GYR_SENSOR_H_ */
