/*
 * An interface to the TI TMP102 temperature sensor
 * 12 bit temperature reading, 0.5 deg. Celsius accuracy
 * -----------------------------------------------------------------
 *
 * Author  : Hedde Bosman (heddebosman@incas3.eu)
 */

#ifndef __TMP102_SENSOR_H__
#define __TMP102_SENSOR_H__

#include "i2c.h"

#include "lib/sensors.h"

extern const struct sensors_sensor tmp102_sensor;

#define TMP102_VALUE_TYPE_DEFAULT 0

#define TMP102_ADDR				0x48 // if A0 @ ground
//#define TMP102_ADDR				0x49 // if A0 @ V+
//#define TMP102_ADDR				0x4A // if A0 @ SDA
//#define TMP102_ADDR				0x4B // if A0 @ SCL

#define TMP102_REGISTER_TEMPERATURE		0x00
#define TMP102_REGISTER_CONFIGURATION	0x01
#define TMP102_REGISTERO_T_LOW			0x02
#define TMP102_REGISTERO_T_HIGH			0x03


#define TMP102_CONF_EXTENDED_MODE 		0x10
#define TMP102_CONF_ALERT				0x20
#define TMP102_CONF_CONVERSION_RATE		0xC0 // 2 bits indicating conversion rate (0.25, 1, 4, 8 Hz)

#define TMP102_CONF_SHUTDOWN_MODE		0x01
#define TMP102_CONF_THERMOSTAT_MODE		0x02 // 0 = comparator mode, 1 = interrupt mode
#define TMP102_CONF_POLARITY			0x04
#define TMP102_CONF_FAULT_QUEUE			0x18 // 2 bits indicating number of faults
#define TMP102_CONF_RESOLUTION			0x60 // 2 bits indicating resolution, default = b11 = 0x60
#define TMP102_CONF_ONESHOT_READY		0x80 // 


#endif

