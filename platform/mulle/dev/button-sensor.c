
#include "dev/button-sensor.h"

const struct sensors_sensor button_sensor;

/*---------------------------------------------------------------------------*/
void
button_press(void)
{
  sensors_changed(&button_sensor);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
