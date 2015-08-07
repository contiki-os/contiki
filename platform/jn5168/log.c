#include <unistd.h>
#include <string.h>

#include "net/ip/uip.h"
#include "sys/log.h"

/*---------------------------------------------------------------------------*/
#if LOG_CONF_ENABLED
void
log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
#endif /* LOG_CONF_ENABLED */
/*---------------------------------------------------------------------------*/
#if UIP_LOGGING
void
uip_log(char *m)
{
  PRINTF("uip_log: %s\n", m);
}
#endif /* UIP_LOGGING */
/*---------------------------------------------------------------------------*/
