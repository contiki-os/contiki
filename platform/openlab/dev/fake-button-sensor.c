/**
 * \file fake-button-sensor.c
 *         A fake button to let more applications to compile
 *
 * \author
 *         Gaëtan Harter <gaetan.harter.at.inria.fr>
 *
 */


#include "contiki.h"
#include "lib/sensors.h"

#define SENSOR_NAME "Button"

const struct sensors_sensor button_sensor;


/*---------------------------------------------------------------------------*/
static int
value(int type)
{
    return 0;
}

/*---------------------------------------------------------------------------*/

static int
configure(int type, int c)
{
    switch (type)
    {
        case SENSORS_HW_INIT:
        case SENSORS_ACTIVE:
            break;
        default:
            return 1;
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static int
status(int type)
{
    return 0;
}

/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, SENSOR_NAME, value, configure, status);

