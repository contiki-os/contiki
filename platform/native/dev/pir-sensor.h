#ifndef PIR_SENSOR_H_
#define PIR_SENSOR_H_

#include "lib/sensors.h"

extern const struct sensors_sensor pir_sensor;

#define PIR_SENSOR "PIR"

void pir_sensor_changed(int strength);

#endif /* PIR_SENSOR_H_ */
