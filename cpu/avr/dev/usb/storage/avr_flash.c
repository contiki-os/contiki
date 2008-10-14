/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file avr_flash.c *********************************************************
 *
 * \brief
 *      This file writes/reads to/from flash memory internal to the AVR.
 *
 * \addtogroup usbstick
 *
 * \author
 *      Colin O'Flynn <coflynn@newae.com>
 ******************************************************************************/
/* 
   Copyright (c) 2008  Colin O'Flynn
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/**
 \addtogroup usbstorage
 @{
*/

//_____  I N C L U D E S ___________________________________________________

#include "config.h"                         // system configuration
#include "storage/avr_flash.h"

#include <avr/pgmspace.h>
#include <avr/boot.h>

#ifndef SPM_PAGESIZE
#error SPM_PAGESIZE undefined!!!
#endif
 

//_____ M A C R O S ________________________________________________________
//_____ P R I V A T E    D E C L A R A T I O N _____________________________


//_____ D E F I N I T I O N ________________________________________________


#define MEM_BASE_ADDRESS       0x10000UL


DATA    U32  gl_ptr_mem;             /* memory data pointer */


/* Disk management  */
bit     reserved_disk_space = FALSE;    /* reserved space for application on disk */


U32 AVRF_DISK_SIZE = 111; /* 57 KB, some room at end saved for bootloader section */


void  avrf_check_init( void );


//_____ D E C L A R A T I O N ______________________________________________

//!
//! @brief This function initializes the hw/sw ressources required to drive the AVR Flash
//!
//! @warning Code:?? bytes (function code length)
//!
//!/
void avrf_mem_init(void)
{
  ;    
}


//!
//! @brief This function tests the state of the AVR Flash
//!
//! @warning Code:?? bytes (function code length)
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Else           ->    CTRL_NO_PRESENT
//!/
Ctrl_status avrf_test_unit_ready(void)
{
   return CTRL_GOOD;
}


//!
//! @brief This function gives the address of the last valid sector.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param *u32_nb_sector  number of sector (sector = 512B). OUT
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!/
Ctrl_status avrf_read_capacity( U32 *u32_nb_sector )
{
   *u32_nb_sector = AVRF_DISK_SIZE;
   return CTRL_GOOD;
}


//!
//! @brief This function returns the write protected status of the memory.
//!
//! Only used by memory removal with a HARDWARE SPECIFIC write protected detection
//! !!! The customer must unplug the memory to change this write protected status,
//! which cannot be for a DF.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @return FALSE  -> the memory is not write-protected (always)
//!/
Bool  avrf_wr_protect(void)
{
   return FALSE;
}


//!
//! @brief This function tells if the memory has been removed or not.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @return FALSE  -> The memory isn't removed
//!/
Bool  avrf_removal(void)
{
   return FALSE;
}



//------------ STANDARD FUNCTIONS to read/write the memory --------------------

//!
//! @brief This function performs a read operation of n sectors from a given address on.
//! (sector = 512B)
//!
//!         DATA FLOW is: AVRF => USB
//!
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param addr         Sector address to start the read from
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status avrf_read_10( U32 addr , U16 nb_sector )
{
   avrf_read_open(addr);                    // wait device is not busy, then send command & address
   avrf_read_sector(nb_sector);             // transfer data from memory to USB
   avrf_read_close();                    // unselect memory
   return CTRL_GOOD;
}


//! This fonction initialise the memory for a write operation
//!
//!         DATA FLOW is: USB => DF
//!
//!
//! (sector = 512B)
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status avrf_write_10( U32 addr , U16 nb_sector )
{

   avrf_write_open(addr);                    // wait device is not busy, then send command & address
   avrf_write_sector(nb_sector);             // transfer data from memory to USB
   avrf_write_close();                    // unselect memory
   return CTRL_GOOD;
}


//------------ AVR FLASH LOWER LEVEL ROUTIENS -----------------------------------------

//!
//! @brief This function opens a DF memory in read mode at a given sector address.
//!
//! NOTE: Address may not be synchronized on the beginning of a page (depending on the DF page size).
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  pos   Logical sector address
//!
//! @return bit
//!   The open succeeded      -> OK
//!/
bit avrf_read_open (Uint32 pos)
{
  // Set the global memory ptr at a Byte address.
  gl_ptr_mem = (pos * 512) + MEM_BASE_ADDRESS;

  return OK;
}


//!
//! @brief This function unselects the current DF memory.
//!
//! @warning Code:?? bytes (function code length)
//!
//!/
void avrf_read_close (void)
{
  ;
}



//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! DataFlash memory to USB controller
//!
//!         DATA FLOW is: DF => USB
//!
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_read_open() function,
//!   - The USB EPIN must have been previously selected,
//!   - USB ping-pong buffers are free,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to read [IN]
//!
//! @return bit
//!   The read succeeded   -> OK
//!/
bit avrf_read_sector (Uint16 nb_sector)
{
   U8 i,j;
   do
   {
      for (i = 8; i != 0; i--)
      {
         Disable_interrupt();    // Global disable.

		 for (j = 0; j < 64; j++) {
         	Usb_write_byte(pgm_read_byte_far(gl_ptr_mem++));
		 }
         

         //# Send the USB FIFO IN content to the USB Host.
         Usb_send_in();       // Send the FIFO IN content to the USB Host.

         Enable_interrupt();     // Global interrupt re-enable.

         // Wait until the tx is done so that we may write to the FIFO IN again.
         while(Is_usb_write_enabled()==FALSE);
      }    
      nb_sector--;            // 1 more sector read
   }
   while (nb_sector != 0);

  return OK;   // Read done.
}


//!
//! @brief This function opens a DF memory in write mode at a given sector
//! address.
//!
//! NOTE: If page buffer > 512 bytes, page content is first loaded in buffer to
//! be partially updated by write_byte or write64 functions.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  pos   Sector address
//!
//! @return bit
//!   The open succeeded      -> OK
//!/
bit avrf_write_open (Uint32 pos)
{
  // Set the global memory ptr at a Byte address.
  gl_ptr_mem = (pos * 512) + MEM_BASE_ADDRESS;

  return OK;
}


//!
//! @brief This function fills the end of the logical sector (512B) and launch
//! page programming.
//!
//! @warning Code:?? bytes (function code length)
//!
//!/
void avrf_write_close (void)
{
	;
}



/* This code can be setup to work with the DFU bootloader, which comes with the AT90USB1287. However I haven't
   had time to test it with such */
#define LAST_BOOT_ENTRY 0xFFFE


void dfuclone_boot_buffer_write(uint16_t dummy, uint32_t baseaddr, uint16_t pageaddr, uint16_t word);
void dfuclone_boot_page_erase(uint32_t dummy1, uint16_t dummy2, uint32_t address);
void dfuclone_boot_page_write(uint32_t dummy1, uint16_t dummy2, uint32_t address);


/* Enable the use of the AVR DFU bootloader by defining "USE_AVRDFU_BOOTLOADER", it will
then call the low-level routines already in the bootloader. */
#ifdef USE_AVRDFU_BOOTLOADER

#error UNTESTED/UNSUPPORTED AT THIS TIME

// These functions pointers are used to call functions entry points in bootloader
void (*dfu_boot_buffer_write) (uint16_t dummy, uint32_t baseaddr, uint16_t pageaddr, uint16_t word)=
       (void (*)(uint16_t, uint32_t, uint16_t, uint16_t))(LAST_BOOT_ENTRY-6);

void (*dfu_boot_page_write) (uint32_t dummy1, uint16_t dummy2, uint32_t address)=
       (void (*)(uint32_t, uint16_t, uint32_t))(LAST_BOOT_ENTRY-4);

void (*dfu_boot_page_erase) (uint32_t dummy1, uint16_t dummy2, uint32_t address)=
       (void (*)(uint32_t, uint16_t, uint32_t))(LAST_BOOT_ENTRY-2);

#else 

// These functions pointers are used to call functions entry points in bootloader
void (*dfu_boot_buffer_write) (uint16_t dummy, uint32_t baseaddr, uint16_t pageaddr, uint16_t word)=
       dfuclone_boot_buffer_write;

void (*dfu_boot_page_write) (uint32_t dummy1, uint16_t dummy2, uint32_t address)=
       dfuclone_boot_page_write;

void (*dfu_boot_page_erase) (uint32_t dummy1, uint16_t dummy2, uint32_t address)=
       dfuclone_boot_page_erase;



#endif

//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! USB controller to DataFlash memory
//!
//!         DATA FLOW is: USB => DF
//!
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_write_open() function,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - The USB EPOUT must have been previously selected,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to write [IN]
//!
//! @return bit
//!   The write succeeded  -> OK
//!/
bit avrf_write_sector (Uint16 nb_sector)
{
  Byte i, j;
  U16 w; 
  
  U16 pgindex = 0;
  U16 sector_bytecounter = 0;


   do
   {
    //# Write 8x64b = 512b from the USB FIFO OUT.
    for (i = 8; i != 0; i--)
    {
      // Wait end of rx in USB EPOUT.
      while(Is_usb_read_enabled()==FALSE);

  	  Disable_interrupt();    // Global disable.

      //If start of page, erase it! 
	  if (pgindex == 0) {
  	  	 (*dfu_boot_page_erase)(0, 0, gl_ptr_mem);
	  }

      //For all the data in the endpoint, write to flash temp page
	  for (j = 0; j < 32; j++) {
	      w = Usb_read_byte();
	      w += Usb_read_byte() << 8;	
		  (*dfu_boot_buffer_write)(0, gl_ptr_mem, pgindex, w);
		  pgindex += 2;
	  }

      Usb_ack_receive_out();  // USB EPOUT read acknowledgement.

      //If we have filled flash page, write that sucker to memory
	  if (pgindex == SPM_PAGESIZE) {
		  
		  (*dfu_boot_page_write)(0,0, gl_ptr_mem);

		  Enable_interrupt();    // Global enable again
		  
		  gl_ptr_mem += SPM_PAGESIZE;   // Update the memory pointer.
		  pgindex = 0;

		  sector_bytecounter += SPM_PAGESIZE;

		  if(sector_bytecounter == 512) {
      		nb_sector--;                  // 1 more sector written
	  		sector_bytecounter = 0;
		  }

	  }

    } // for (i = 8; i != 0; i--)

   }
   while (nb_sector != 0);

  return OK;                  // Write done
}


#ifndef USE_AVRDFU_BOOTLOADER
/* Perform read/write of FLASH, using same calling convention as low-level routines in AVR DFU bootloader */

BOOTLOADER_SECTION void dfuclone_boot_buffer_write(uint16_t dummy, uint32_t baseaddr, uint16_t pageaddr, uint16_t word)
{
	boot_page_fill(baseaddr + pageaddr, word);
	return;
}


BOOTLOADER_SECTION void dfuclone_boot_page_erase(uint32_t dummy1, uint16_t dummy2, uint32_t address)
{
	boot_page_erase(address);
  	boot_spm_busy_wait();
	boot_rww_enable();
	return;
}

BOOTLOADER_SECTION void dfuclone_boot_page_write(uint32_t dummy1, uint16_t dummy2, uint32_t address)
{
	boot_page_write(address);
    boot_spm_busy_wait();
	boot_rww_enable();
	return;
}
#endif //USE_AVRDFU_BOOTLOADER

//------------ SPECIFIC FONCTION USB TRANSFER -----------------------------------------

//** If your device transfer have a specific transfer for USB (Particularity of Chejudo product, or bootloader)
// !!! In this case the driver must be know the USB access

//! This fonction transfer the memory data (programed in scsi_read_10) directly to the usb interface
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!
Ctrl_status avrf_usb_read()
{
   return CTRL_GOOD;
}


//! This fonction transfer the usb data (programed in scsi_write_10) directly to the memory data
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!
Ctrl_status avrf_usb_write( void )
{
   return CTRL_GOOD;
}

/** @} */
