
#include "dev/button-sensor.h"

const struct sensors_sensor button_sensor;

/*---------------------------------------------------------------------------*/
void
button_press(void)
{
  sensors_changed(&button_sensor);
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
