
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
	       value, configure, status);
