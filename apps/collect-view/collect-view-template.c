#include "collect-view.h"

enum {
  SENSOR1,
  SENSOR2,
};

/*---------------------------------------------------------------------------*/
void
collect_view_arch_read_sensors(struct collect_view_data_msg *msg)
{
  msg->sensors[SENSOR1] = 0;
  msg->sensors[SENSOR2] = 0;
}
/*---------------------------------------------------------------------------*/
