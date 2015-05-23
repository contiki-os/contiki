#ifndef BUTTON_SENSOR_H_
#define BUTTON_SENSOR_H_

#include "lib/sensors.h"

extern const struct sensors_sensor button_sensor;

#define BUTTON_SENSOR "Button"

void button_press(void);

#endif /* BUTTON_SENSOR_H_ */
