#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include "lib/sensors.h"
#include "isl29020.h"

/** Configure types */
enum {
  LIGHT_SENSOR_SOURCE = 0,
  LIGHT_SENSOR_RESOLUTION = 1,
  LIGHT_SENSOR_RANGE = 2,
};

enum {
  // should hold for max RANGE in an int
  LIGHT_SENSOR_VALUE_SCALE = 32768,
};

extern const struct sensors_sensor light_sensor;

#endif /* LIGHT-SENSOR_H_ */
