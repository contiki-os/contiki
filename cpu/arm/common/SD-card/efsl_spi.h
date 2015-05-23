#ifndef EFSL_SPI_H_6B6MX40N68__
#define EFSL_SPI_H_6B6MX40N68__

#include <types.h>

struct  hwInterface{
        eint32          sectorCount;
};
typedef struct hwInterface hwInterface;

esint8 if_initInterface(hwInterface* file,eint8* opts);
esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_setPos(hwInterface* file,euint32 address);

void if_spiInit(hwInterface *iface);
void if_spiSetSpeed(euint8 speed);
euint8 if_spiSend(hwInterface *iface, euint8 outgoing);


#endif /* EFSL_SPI_H_6B6MX40N68__ */
