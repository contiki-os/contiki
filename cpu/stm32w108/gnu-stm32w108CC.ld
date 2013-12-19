/******************************************************************************/
/*                       Peripheral memory map                                */
/******************************************************************************/
_BOOTLOADER_SIZE = 0 ;
/* This value needs to be fine tuned if the NVM section grows */
_NVM_size = 0x1000;

/* include the memory spaces definitions sub-script */
MEMORY
{
  RAM_region (xrw) :          ORIGIN = 0x20000000,            LENGTH = 16K
  ROM_region (rx) :           ORIGIN = 0x08000000,            LENGTH = 256K
  FIB_region (ra) :           ORIGIN = 0x08040000,            LENGTH = 2K
}

INCLUDE "gnu-stm32w108.ld"
