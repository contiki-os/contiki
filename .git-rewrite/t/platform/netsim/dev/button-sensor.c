
#include "dev/button-sensor.h"

const struct sensors_sensor button_sensor;

/*---------------------------------------------------------------------------*/
void
button_press(void)
{
  sensors_changed(&button_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return 1;
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
	       init, irq, activate, deactivate, active,
	       value, configure, status);
