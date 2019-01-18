
#include "dev/pir-sensor.h"

const struct sensors_sensor pir_sensor;

static int pir_value;

/*---------------------------------------------------------------------------*/
void
pir_sensor_changed(int strength)
{
  pir_value += strength;
  sensors_changed(&pir_sensor);
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
  return pir_value;
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
SENSORS_SENSOR(pir_sensor, PIR_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
