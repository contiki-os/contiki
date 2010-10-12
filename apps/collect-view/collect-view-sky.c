#include "collect-view.h"
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/light-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/sht11-sensor.h"

enum {
  BATTERY_VOLTAGE_SENSOR,
  BATTERY_INDICATOR,
  LIGHT1_SENSOR,
  LIGHT2_SENSOR,
  TEMP_SENSOR,
  HUMIDITY_SENSOR,
  RSSI_SENSOR,
  ETX1_SENSOR,
  ETX2_SENSOR,
  ETX3_SENSOR,
  ETX4_SENSOR,
};

/*---------------------------------------------------------------------------*/
void
collect_view_arch_read_sensors(struct collect_view_data_msg *msg)
{

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  msg->sensors[BATTERY_VOLTAGE_SENSOR] = battery_sensor.value(0);
  msg->sensors[BATTERY_INDICATOR] = sht11_sensor.value(SHT11_SENSOR_BATTERY_INDICATOR);
  msg->sensors[LIGHT1_SENSOR] = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  msg->sensors[LIGHT2_SENSOR] = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
  msg->sensors[TEMP_SENSOR] = sht11_sensor.value(SHT11_SENSOR_TEMP);
  msg->sensors[HUMIDITY_SENSOR] = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);


  SENSORS_DEACTIVATE(light_sensor);
  SENSORS_DEACTIVATE(battery_sensor);
  SENSORS_DEACTIVATE(sht11_sensor);
}
/*---------------------------------------------------------------------------*/
