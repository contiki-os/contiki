#include <AT91SAM7S64.h>
#include <interfaces/sd.h>
#include <efs-sdcard.h>
#include <sys/etimer.h>
#include <stdio.h>

#define SPI_SPEED 10000000  /* 10MHz clock*/

#define SPI_TRANSFER (AT91C_PA12_MISO | AT91C_PA13_MOSI | AT91C_PA14_SPCK)

#define SPI_CS (AT91C_PA11_NPCS0)

static struct process *event_process = NULL;

static void
init_spi()
{
  *AT91C_SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
  *AT91C_PMC_PCER = (1 << AT91C_ID_SPI);
  *AT91C_PIOA_ASR = SPI_TRANSFER | SPI_CS;
  *AT91C_PIOA_PDR = SPI_TRANSFER | SPI_CS;
  *AT91C_PIOA_PPUER = AT91C_PA12_MISO | SPI_CS;
  *AT91C_SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED
		   | AT91C_SPI_MODFDIS);
  
  /* It seems necessary to set the clock speed for chip select 0
     even if it's not used. */

  AT91C_SPI_CSR[0] =
    ((((MCK+SPI_SPEED/2)/SPI_SPEED)<<8) | AT91C_SPI_CPOL
     | AT91C_SPI_BITS_8 | AT91C_SPI_CSAAT);
  *AT91C_SPI_CR = AT91C_SPI_SPIEN;
  
}

void
if_spiInit(hwInterface *iface)
{
  unsigned int i;
  *AT91C_PIOA_SODR = AT91C_PA11_NPCS0;
  *AT91C_PIOA_PER = AT91C_PA11_NPCS0;
  for(i=0;i<20;i++) {
    if_spiSend(iface, 0xff);
  }
  *AT91C_PIOA_PDR = AT91C_PA11_NPCS0;
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
  if( (sc%512) != 0) {
    file->sectorCount--;
  }
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
  *AT91C_SPI_TDR = outgoing;
  while(!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
  ingoing = *AT91C_SPI_RDR;
  /* printf(">%02x <%02x\n", outgoing, ingoing); */
  return ingoing;
}

static EmbeddedFileSystem sdcard_efs;

PROCESS(sdcard_process, "SD card process");

PROCESS_THREAD(sdcard_process, ev , data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  *AT91C_PIOA_PER = AT91C_PIO_PA20 | AT91C_PIO_PA1;
  *AT91C_PIOA_ODR = AT91C_PIO_PA20 | AT91C_PIO_PA1;
  
  
  /* Card not inserted */
  sdcard_efs.myCard.sectorCount = 0;
  init_spi();
  
  while(1) {
    if (!(*AT91C_PIOA_PDSR & AT91C_PA20_IRQ0)) {
      if (sdcard_efs.myCard.sectorCount == 0) {
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
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     ev == PROCESS_EVENT_TIMER);
    if (ev == PROCESS_EVENT_EXIT) break;
    if (!(*AT91C_PIOA_PDSR & AT91C_PA20_IRQ0)) {
      /* Wait for card to be preperly inserted */
      etimer_set(&timer,CLOCK_SECOND/2);
      PROCESS_WAIT_EVENT_UNTIL(ev== PROCESS_EVENT_TIMER);
    }
    
  }
  PROCESS_END();
}

FileSystem *
efs_sdcard_get_fs()
{
  efs_sdcard_init();
  return &sdcard_efs.myFs;
}

void
efs_sdcard_init()
{
  static int initialized = 0;
  if (!initialized) {
    sdcard_inserted_event = process_alloc_event();
    sdcard_removed_event = process_alloc_event();
    process_start(&sdcard_process, NULL);
    initialized = 1;
  }
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
