#include	"contiki.h"
#include	<pty.h>
#include	<stdio.h>
#include	<sys/select.h>

static int ppp_master_fd = -1;
static int ppp_slave_fd = -1;
static char ppp_slave_name[80];

void init_pty() {
  char			cmd[256];

  if (ppp_master_fd == -1) {
    if (openpty( &ppp_master_fd, &ppp_slave_fd, ppp_slave_name, NULL, NULL)==0)
    {
      sprintf( cmd, "stty raw -echo -echoe < %s", ppp_slave_name);
      system(cmd);
      printf("pty opened, slave name = %s\n", ppp_slave_name);
      printf("Now start pppd, for example with:\n");
      printf("/usr/sbin/pppd %s 115200 local noauth nocrtscts nodetach persist asyncmap 0xffffffff 192.168.1.1:192.168.1.2\n",
      		ppp_slave_name);
      close(ppp_slave_fd);
    } else {
      printf("pty open failed\n");
      exit(1);
    }
  }
}

void ppp_arch_putchar(u8_t c) {
  init_pty();
  if (write( ppp_master_fd, &c, 1 ) > 0) {
    //printf("wr=0x%02x '%c'\n", c, c>=' '?c<0x7f?c:' ':' ');
  } else {
    //printf("wr=0x%02x '%c' FAILED\n", c, c>=' '?c<0x7f?c:' ':' ');
  }
}

int ppp_arch_getchar(u8_t *p) {
  fd_set		read_fds;
  struct timeval	timeout;

  init_pty();
  FD_ZERO( &read_fds );
  FD_SET( ppp_master_fd, &read_fds );
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;
  if (select( ppp_master_fd+1, &read_fds, NULL, NULL, &timeout) != 0) {
    if (FD_ISSET( ppp_master_fd, &read_fds )) {
      if ( read( ppp_master_fd, p, 1) > 0 ) {
	//printf("rd=0x%02x\n", *p);
	return 1;
      }
      return 0;
    }
      printf("closing pty: %s\n", ppp_slave_name);
      close( ppp_master_fd );
      ppp_master_fd == -1;
  } else {
    return 0;
  }
}
