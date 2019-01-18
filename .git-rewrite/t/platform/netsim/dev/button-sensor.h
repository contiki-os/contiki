#ifndef __BUTTON_SENSOR_H__
#define __BUTTON_SENSOR_H__

#include "lib/sensors.h"

extern const struct sensors_sensor button_sensor;

#define BUTTON_SENSOR "Button"

void button_press(void);

#endif /* __BUTTON_SENSOR_H__ */
