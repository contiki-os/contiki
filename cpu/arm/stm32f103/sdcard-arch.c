#include <stm32f10x_map.h>
#include <sdcard.h>
#include <sys/process.h>
#include <sys/etimer.h>
#include <cfs/cfs.h>
#include <efs.h>
#include <ls.h>
#include <interfaces/sd.h>
#include <gpio.h>
#include <stdio.h>

process_event_t sdcard_inserted_event;

process_event_t sdcard_removed_event;

static struct process *event_process = NULL;

#if 0
#undef TXT
#define TXT(x) x
#undef DBG
#define DBG(x) printf x
#endif

static void
init_spi()
{
  SPI1->CR1 &= ~SPI_CR1_SPE;
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  GPIO_CONF_INPUT_PORT(A,0,FLOATING);
  GPIO_CONF_INPUT_PORT(A,1,FLOATING);
  GPIO_CONF_OUTPUT_PORT(A,4,PUSH_PULL,50);
  GPIOA->BSRR = GPIO_BSRR_BS4;
  GPIO_CONF_OUTPUT_PORT(A,5,ALT_PUSH_PULL,50);
  GPIO_CONF_INPUT_PORT(A,6,FLOATING);
  GPIO_CONF_OUTPUT_PORT(A,7,ALT_PUSH_PULL,50);
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  SPI1->CR2 = SPI_CR2_SSOE;
  SPI1->CR1 = (SPI_CR1_SPE
	       | (SPI_CR1_BR_2)	/* fPCLK / 32 */
	       | SPI_CR1_MSTR
	       | SPI_CR1_CPOL | SPI_CR1_CPHA
	       | SPI_CR1_SSM | SPI_CR1_SSI);

}

void
if_spiInit(hwInterface *iface)
{
  unsigned int i;
  GPIOA->BSRR = GPIO_BSRR_BS4;
  for(i=0;i<20;i++) {
    if_spiSend(iface, 0xff);
  }
  GPIOA->BSRR = GPIO_BSRR_BR4;
}
/* Borrowed from at91_spi.c  (c)2006 Martin Thomas */

esint8
if_initInterface(hwInterface* file, eint8* opts)
{
  euint32 sc;
  if_spiInit(file);
  if(sd_Init(file)<0)     {
    DBG((TXT("Card failed to init, breaking up...\n")));
    return(-1);
  }
  
  if(sd_State(file)<0){
    DBG((TXT("Card didn't return the ready state, breaking up...\n")
	 ));
    return(-2);
  }
  

  
  sd_getDriveSize(file, &sc);
  file->sectorCount = sc/512;
  DBG((TXT("Card Capacity is %lu Bytes (%lu Sectors)\n"), sc, file->sectorCount));
  
  
  return(0);
}

/* Borrowed from lpc2000_spi.c  (c)2005 Martin Thomas */

esint8
if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
        return(sd_readSector(file,address,buf,512));
}

esint8
if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
        return(sd_writeSector(file,address, buf));
}

esint8
if_setPos(hwInterface* file,euint32 address)
{
        return(0);
}


euint8
if_spiSend(hwInterface *iface, euint8 outgoing)
{
  euint8 ingoing;
  SPI1->DR = outgoing;
  while(!(SPI1->SR & SPI_SR_RXNE));
  ingoing = SPI1->DR;
  /*   printf(">%02x <%02x\n", outgoing, ingoing); */
  return ingoing;
}

#define MAX_FDS 4

static EmbeddedFileSystem sdcard_efs;
static File file_descriptors[MAX_FDS];

static int
find_free_fd()
{
  int fd;
  for (fd = 0; fd < MAX_FDS; fd++) {
    if (!file_getAttr(&file_descriptors[fd], FILE_STATUS_OPEN)) {
      return fd;
    }
  }
  return -1;
}

static File *
get_file(int fd)
{
  if (sdcard_efs.myCard.sectorCount == 0) return NULL;
  if (fd >= MAX_FDS || fd < 0) return NULL;
  if (!file_getAttr(&file_descriptors[fd], FILE_STATUS_OPEN)) return NULL;
  return &file_descriptors[fd];
}

int
cfs_open (const char *name, int flags)
{
  eint8 mode;
  int fd;
  if (sdcard_efs.myCard.sectorCount == 0) return -1;
  fd = find_free_fd();
  if (fd < 0) return -1;
  if (flags == CFS_READ) {
    mode = MODE_READ;
  } else {
    mode = MODE_APPEND;
  }
  if (file_fopen(&file_descriptors[fd], &sdcard_efs.myFs,
		 (char*)name, mode) < 0) {
    return -1;
  }
  return fd;
}

void
cfs_close(int fd)
{
  File *file = get_file(fd);
  if (!file) return;
  file_fclose(file);
  fs_flushFs(&sdcard_efs.myFs);
}

int
cfs_read (int fd, void *buf, unsigned int len)
{
  File *file = get_file(fd);
  if (!file) return 0;
  return file_read(file, len, (euint8*)buf);
}

int
cfs_write (int fd, const void *buf, unsigned int len)
{
  File *file = get_file(fd);
  if (!file) return 0;
  return file_write(file, len, (euint8*)buf);
}

cfs_offset_t
cfs_seek (int fd, cfs_offset_t offset, int whence)
{
  File *file = get_file(fd);
  if (!file) return 0;
  /* TODO take whence int account */
  if (file_setpos(file, offset) != 0) return -1;
  return file->FilePtr;
}

int
cfs_remove(const char *name)
{
  return (rmfile(&sdcard_efs.myFs,(euint8*)name) == 0) ? 0 : -1;
}

/* Cause a compile time error if expr is false */
#ifdef __GNUC__
#define COMPILE_TIME_CHECK(expr) \
(void) (__builtin_choose_expr ((expr), 0, ((void)0))+3)
#else
#define COMPILE_TIME_CHECK(expr)
#endif

#define MAX_DIR_LISTS 4
DirList dir_lists[MAX_DIR_LISTS];

static DirList *
find_free_dir_list()
{
  unsigned int l;
  for(l = 0; l < MAX_DIR_LISTS; l++) {
    if (dir_lists[l].fs == NULL) {
      return &dir_lists[l];
    }
  }
  return NULL;
}

int
cfs_opendir (struct cfs_dir *dirp, const char *name)
{
  DirList *dirs;
  COMPILE_TIME_CHECK(sizeof(DirList*) <= sizeof(struct cfs_dir));
  if (sdcard_efs.myCard.sectorCount == 0) return -1;
  dirs = find_free_dir_list();
  if (!dirs) return -1;
  if (ls_openDir(dirs, &sdcard_efs.myFs, (eint8*)name) != 0) {
    dirs->fs = NULL;
    return -1;
  }
  *(DirList**)dirp = dirs;
  return 0;
}

int
cfs_readdir (struct cfs_dir *dirp, struct cfs_dirent *dirent)
{
  euint8 *start;
  euint8 *end;
  char *to = dirent->name;
  DirList *dirs = *(DirList**)dirp;
  if (sdcard_efs.myCard.sectorCount == 0) return 1;
  if (ls_getNext(dirs) != 0) return 1;
  start = dirs->currentEntry.FileName;
  end = start + 7;
  while(end > start) {
    if (*end > ' ') {
      end++;
      break;
    }
    end--;
  }
  while(start < end) {
    *to++ = *start++;
  }
  start = dirs->currentEntry.FileName + 8;
  end = start + 3;
  if (*start > ' ') {
    *to++ = '.';
    *to++ = *start++;
    while(start < end && *start > ' ') {
      *to++ = *start++;
    }
  }
  *to = '\0';
  if (dirs->currentEntry.Attribute & ATTR_DIRECTORY) {
    dirent->size = 0;
  } else {
    dirent->size = dirs->currentEntry.FileSize;
  }
  return 0;
}

void
cfs_closedir (struct cfs_dir *dirp)
{
  (*(DirList**)dirp)->fs = NULL;
}


PROCESS(sdcard_process, "SD card process");

PROCESS_THREAD(sdcard_process, ev , data)
{
  int fd;
  static struct etimer timer;
  PROCESS_BEGIN();
  /* Mark all file descriptors as free */
  for (fd = 0; fd < MAX_FDS; fd++) {
    file_setAttr(&file_descriptors[fd], FILE_STATUS_OPEN,0);
  }
  /* Card not inserted */
  sdcard_efs.myCard.sectorCount = 0;
  init_spi();

  etimer_set(&timer, CLOCK_SECOND);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     ev== PROCESS_EVENT_TIMER || ev == PROCESS_EVENT_POLL);
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_TIMER) {
      if (!(GPIOA->IDR & (1<<0))) {
	if (sdcard_efs.myCard.sectorCount == 0) {
	  etimer_set(&timer,CLOCK_SECOND/2);
	  PROCESS_WAIT_EVENT_UNTIL(ev== PROCESS_EVENT_TIMER);
	  if (efs_init(&sdcard_efs,0) == 0) {
	    if (event_process) {
	      process_post(event_process, sdcard_inserted_event, NULL);
	    }
	    printf("SD card inserted\n");
	  } else {
	    printf("SD card insertion failed\n");
	  }
	}
      } else {
	if (sdcard_efs.myCard.sectorCount != 0) {
	  /* Card removed */
	  fs_umount(&sdcard_efs.myFs);
	  sdcard_efs.myCard.sectorCount = 0;
	  if (event_process) {
	    process_post(event_process, sdcard_removed_event, NULL);
	  }
	  printf("SD card removed\n");
	}
      }
      etimer_set(&timer, CLOCK_SECOND);
      
    }
  }
  PROCESS_END();
}

void
sdcard_init()
{
  sdcard_inserted_event = process_alloc_event();
  sdcard_removed_event = process_alloc_event();
  process_start(&sdcard_process, NULL);
}

int
sdcard_ready()
{
  return sdcard_efs.myCard.sectorCount > 0;
}

void
sdcard_event_process(struct process *p)
{
  event_process = p;
}
