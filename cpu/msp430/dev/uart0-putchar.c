#include "dev/uart0.h"

#if !NETSTACK_CONF_WITH_IPV4
/* In case of IPv4: putchar() is defined by the SLIP driver */
int
putchar(int c)
{
  uart0_writeb((char)c);
  return c;
}
#endif /* ! NETSTACK_CONF_WITH_IPV4 */
