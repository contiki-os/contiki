#ifndef PRESSURE_SENSOR_H_
#define PRESSURE_SENSOR_H_

#include "lib/sensors.h"
#include "lps331ap.h"

/** Configure types */
enum {
  PRESSURE_SENSOR_DATARATE = 0,
};

enum {
  PRESSURE_SENSOR_VALUE_SCALE = 4096,
};

extern const struct sensors_sensor pressure_sensor;

#endif /* PRESSURE_SENSOR_H_ */
