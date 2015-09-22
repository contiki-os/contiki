#ifndef COLLECT_VIEW_OPENMOTE_H
#define COLLECT_VIEW_OPENMOTE_H

#include "collect-view.h"

// SENSORS(&button_user_sensor, &cc2538_temp_sensor, &vdd3_sensor, &temp_sensor, &humidity_sensor, &light_sensor, &acceleration_sensor);
enum {
  LIGHT_SENSOR,
  TEMP_SENSOR,
  HUMIDITY_SENSOR,
  ACCELERATION_SENSOR,
  BUTTON_USER_SENSOR,
  CC2538_TEMP_SENSOR,
  VDD3_SENSOR,
  ETX1_SENSOR,
  ETX2_SENSOR,
  ETX3_SENSOR,
  ETX4_SENSOR,
};


#endif /* COLLECT_VIEW_OPENMOTE_H */
