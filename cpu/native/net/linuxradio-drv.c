#include "contiki.h"

#include "linuxradio-drv.h"

#include "net/packetbuf.h"
#include "net/netstack.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <linux/sockios.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int sockfd = -1;
static char *sockbuf;
static int buflen;

#if 0
static void my_memmove(char *s1, char *s2, int len)
{
  int i;
  for(i=0; i<len; ++i) {
    *s1++ = *s2++;
  }
}
#endif

static int
init(void)
{
  sockbuf = malloc(256);
  return 0;
}

static int
prepare(const void *payload, unsigned short payload_len)
{
  if(payload_len > 256) {
    return 0;
  }
  memcpy(sockbuf, payload, payload_len);
  buflen = payload_len;

  return 0;
}

static int
transmit(unsigned short transmit_len)
{
  int sent=0;
  sent = send(sockfd, sockbuf, buflen, 0);
  if(sent < 0) {
    perror ("linuxradio send()");
  }
  buflen = 0;
  return RADIO_TX_OK;
}

static int
my_send(const void *payload, unsigned short payload_len)
{
  int ret = -1;

  PRINTF("PREPARE & TRANSMIT %u bytes\n", payload_len);

  if(prepare(payload, payload_len)) {
    return ret;
  }

  ret = transmit(payload_len);

  return ret;
}

static int
my_read(void *buf, unsigned short buf_len)
{
  return 0;
}

static int
channel_clear(void)
{
  return 1;
}

static int
receiving_packet(void)
{
  return 0;
}

static int
pending_packet(void)
{
  return 0;
}

static int
set_fd(fd_set *rset, fd_set *wset)
{
  FD_SET(sockfd, rset);
  return 1;
}

static void
handle_fd(fd_set *rset, fd_set *wset)
{
  if(FD_ISSET(sockfd, rset)) {
    int bytes = read(sockfd, sockbuf, 256);
    buflen = bytes;
    PRINTF("linuxradio: read %d bytes\n", bytes);
    memcpy(packetbuf_dataptr(), sockbuf, bytes);
    packetbuf_set_datalen(bytes);
    NETSTACK_RDC.input();
  }
}

static const struct select_callback linuxradio_sock_callback = { set_fd, handle_fd };

static int
on(void)
{
  sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IEEE802154));
  if(sockfd < 0) {
    perror ("linuxradio socket()");
    return 0;
  } else {
    struct ifreq ifr;
    strncpy((char *)ifr.ifr_name, "wpan0", IFNAMSIZ); // TODO: make interface configurable
    ioctl(sockfd, SIOCGIFINDEX, &ifr);

    struct sockaddr_ll sll;
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IEEE802154);

    if(bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
      perror("linuxradio bind()");
      return 0;
    }

    select_set_callback(sockfd, &linuxradio_sock_callback);
    return 1;
  }
}

static int
off(void)
{
  close(sockfd);
  sockfd = -1;
  return 1;
}

const struct radio_driver linuxradio_driver =
  {
    init,
    prepare,
    transmit,
    my_send,
    my_read,
    channel_clear,
    receiving_packet,
    pending_packet,
    on,
    off,
  };
