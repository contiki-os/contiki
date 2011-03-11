/* Dummy sensor routine */

#include "lib/sensors.h"
#include "dev/button-sensor.h"
const struct sensors_sensor button_sensor;
static int status(int type);
struct sensors_sensor *sensors[1];
unsigned char sensors_flags[1];


static int
value(int type)
{
 return 0;
}

static int
configure(int type, int c)
{
	switch (type) {
	case SENSORS_ACTIVE:
		if (c) {
			if(!status(SENSORS_ACTIVE)) {
			}
		} else {
		}
		return 1;
	}
	return 0;
}

static int
status(int type)
{
	switch (type) {
	case SENSORS_ACTIVE:
	case SENSORS_READY:
		return 1;
	}
	return 0;
}

SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);

