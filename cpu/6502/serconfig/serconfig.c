#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <serial.h>

#include "cfs/cfs.h"

static struct {
  char    *screen;
  uint8_t value;
} baud[] = {
  {"  300 baud", SER_BAUD_300},
  {"  600 baud", SER_BAUD_600},
  {" 1200 baud", SER_BAUD_1200},
  {" 2400 baud", SER_BAUD_2400},
  {" 4800 baud", SER_BAUD_4800},
  {" 9600 baud", SER_BAUD_9600},
  {"19200 baud", SER_BAUD_19200}
};

static struct {
  char    *screen;
  uint8_t value;
} stop[] = {
  {"1 stop bit",  SER_STOP_1},
  {"2 stop bits", SER_STOP_2}
};

static struct {
  char    *screen;
  uint8_t value;
} parity[] = {
  {"  No parity", SER_PAR_NONE},
  {" Odd parity", SER_PAR_ODD},
  {"Even parity", SER_PAR_EVEN}
};

uint8_t           ipcfg[16];
struct ser_params params;

/*-----------------------------------------------------------------------------------*/
uint8_t
choose(uint8_t max)
{
  char val;

  do {
    printf("\n?");
    val = getchar();
  } while(val < '0' || val > max + '0');

  putchar('\n');
  if(val == '0') {
    exit(0);
  }

  putchar('\n');
  return val - '0';
}
/*-----------------------------------------------------------------------------------*/
void
main(void)
{
  int f;
  uint8_t c;

  f = cfs_open("contiki.cfg", CFS_READ);
  if(f == -1) {
    printf("Loading Config - Error\n");
    return;
  }
  cfs_read(f, ipcfg, sizeof(ipcfg));
  cfs_close(f);

  for(c = 0; c < sizeof(baud) / sizeof(baud[0]); ++c) {
    printf("%d: %s\n", c + 1, baud[c].screen);
  }
  params.baudrate = baud[choose(c) - 1].value;

  params.databits = SER_BITS_8;

  for(c = 0; c < sizeof(stop) / sizeof(stop[0]); ++c) {
    printf("%d: %s\n", c + 1, stop[c].screen);
  }
  params.stopbits = stop[choose(c) - 1].value;

  for(c = 0; c < sizeof(parity) / sizeof(parity[0]); ++c) {
    printf("%d: %s\n", c + 1, parity[c].screen);
  }
  params.parity = parity[choose(c) - 1].value;

  params.handshake = SER_HS_HW;

  f = cfs_open("contiki.cfg", CFS_WRITE);
  if(f == -1) {
    printf("\nSaving Config - Error\n");
    return;
  }
  cfs_write(f, ipcfg,   sizeof(ipcfg));
  cfs_write(f, &params, sizeof(params));
  cfs_close(f);

  printf("Saving Config - Done\n");
}
/*-----------------------------------------------------------------------------------*/
