#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfs/cfs.h"

static struct {
  char     *screen;
  uint16_t address;
  char     *driver;
} drivers[] = {
#ifdef __APPLE2__
  {"Uthernet",    0xC080, "cs8900a.eth" },
  {"Uthernet II", 0xC084, "w5100.eth"   },
  {"LANceGS",     0xC080, "lan91c96.eth"}
#endif
#ifdef __ATARI__
  {"Dragon Cart", 0xD500, "cs8900a.eth" }
#endif
#ifdef __CBM__
  {"RR-Net",      0xDE08, "cs8900a.eth" },
  {"TFE",         0xDE00, "cs8900a.eth" },
  {"ETH64",       0xDE00, "lan91c96.eth"}
#endif
};

uint8_t ipcfg[16];

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
  uint8_t d;

  f = cfs_open("contiki.cfg", CFS_READ);
  if(f == -1) {
    printf("Loading Config - Error\n");
    return;
  }
  cfs_read(f, ipcfg, sizeof(ipcfg));
  cfs_close(f);

  for(d = 0; d < sizeof(drivers) / sizeof(drivers[0]); ++d) {
    printf("%d: %s\n", d + 1, drivers[d].screen);
  }
  d = choose(d) - 1;

#ifdef __APPLE2__
  printf("Slot (1-7)\n");
  drivers[d].address += choose(7) * 0x10;
#endif

  f = cfs_open("contiki.cfg", CFS_WRITE);
  if(f == -1) {
    printf("Saving Config - Error\n");
    return;
  }
  cfs_write(f, ipcfg, sizeof(ipcfg));
  cfs_write(f, &drivers[d].address, sizeof(drivers[d].address));
  cfs_write(f, drivers[d].driver, strlen(drivers[d].driver));
  cfs_close(f);

  printf("Saving Config - Done\n");
}
/*-----------------------------------------------------------------------------------*/
