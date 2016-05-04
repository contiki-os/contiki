#include "dev/uart1.h"

#if !NETSTACK_CONF_WITH_IPV4
/* In case of IPv4: putchar() is defined by the SLIP driver */
int
putchar(int c)
{
  uart1_writeb((char)c);
  return c;
}
#endif /* ! NETSTACK_CONF_WITH_IPV4 */
