#include "collect-view.h"
#include "dev/antenna.h"
#include "dev/adxl346.h"
#include "dev/sht21.h"
#include "dev/max44009.h"

#include "collect-view-openmote.h"

/*---------------------------------------------------------------------------*/
void
collect_view_arch_read_sensors(struct collect_view_data_msg *msg)
{

  SENSORS_ACTIVATE(temp_sensor);
  SENSORS_ACTIVATE(humidity_sensor);
  SENSORS_ACTIVATE(light_sensor);
//  SENSORS_ACTIVATE(acceleration_sensor);
#if 1
  static unsigned tem;
  tem = sht21_read_temperature();
  msg->sensors[TEMP_SENSOR] = sht21_convert_temperature(tem);
  tem = sht21_read_humidity();
  msg->sensors[HUMIDITY_SENSOR] = sht21_convert_humidity(tem);
  tem = max44009_read_light();
  msg->sensors[LIGHT_SENSOR] = max44009_convert_light(tem);
//  msg->sensors[ACCELERATION_SENSOR] = ue(SHT11_SENSOR_TEMP);
//  msg->sensors[HUMIDITY_SENSOR] = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
#else
  msg->sensors[BUTTON_USER_SENSOR] = 1;
  msg->sensors[CC2538_TEMP_SENSOR] = 2;
  msg->sensors[VDD3_SENSOR] = 4;
  msg->sensors[TEMP_SENSOR] = 27;
  msg->sensors[HUMIDITY_SENSOR] = 80;
  msg->sensors[LIGHT_SENSOR] = 1000;
  msg->sensors[ACCELERATION_SENSOR] = 41;
#endif
 


  SENSORS_DEACTIVATE(temp_sensor);
  SENSORS_DEACTIVATE(humidity_sensor);
  SENSORS_DEACTIVATE(light_sensor);
}
/*---------------------------------------------------------------------------*/
