#include "lib/sensors.h"

#include "dev/button-sensor.h"  // fake button sensor
#include "dev/acc-mag-sensor.h"
#include "dev/gyr-sensor.h"


/** Sensors **/
const struct sensors_sensor *sensors[] = {
    &button_sensor,  // fake button sensor
    &acc_sensor, &mag_sensor,
    &gyr_sensor,
    0
};

unsigned char sensors_flags[(sizeof(sensors) / sizeof(struct sensors_sensor *))];
