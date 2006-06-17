#ifndef __PIR_SENSOR_H__
#define __PIR_SENSOR_H__

#include "lib/sensors.h"

extern const struct sensors_sensor pir_sensor;

#define PIR_SENSOR "PIR"

void pir_sensor_changed(int strength);

#endif /* __PIR_SENSOR_H__ */
