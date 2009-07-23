/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file avr_flash.h *********************************************************
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

#ifndef _DFMEM_H_
#define _DFMEM_H_

#include "conf_access.h"
#include "ctrl_status.h"




//_____ D E F I N I T I O N S ______________________________________________

//! FAT specification
#define  ENTRY_SIZE           32    // size of entrie in byte



// First sector of the disk virtual (this disk isn't partitioned, only one partion)
#define  PBR_SECTOR           (0)
#define  FAT_SECTOR           (PBR_SECTOR  + VMEM_RESERVED_SIZE)
#define  ROOT_SECTOR          (FAT_SECTOR  + (VMEM_SIZE_FAT*VMEM_NB_FATS))
#define  FILE_SECTOR          (ROOT_SECTOR + ((ENTRY_SIZE*VMEM_NB_ROOT_ENTRY) / VMEM_SECTOR_SIZE))                   // 1(sector) = size root dir


/* FAT Format Structure */
typedef struct
{
  Uint16 nb_cylinder;
  Byte   nb_head;
  Byte   nb_sector;
  Byte   nb_hidden_sector;
  Byte   nb_sector_per_cluster;
} s_format;

#define MEM_BSY       0
#define MEM_OK        1
#define MEM_KO        2




//---- CONTROL FONCTIONS ----

// those fonctions are declared in df_mem.h
void           avrf_mem_init(void);
Ctrl_status    avrf_test_unit_ready(void);
Ctrl_status    avrf_read_capacity( U32 _MEM_TYPE_SLOW_ *u32_nb_sector );
Bool           avrf_wr_protect(void);
Bool           avrf_removal(void);

//---- AVR FLASH LOW-LEVEL----


bit     avrf_init (void);
bit     avrf_mem_check(void);
bit     avrf_read_open (Uint32);
void    avrf_read_close (void);
bit     avrf_write_open (Uint32);
void    avrf_write_close (void);

//! Funtions to link USB DEVICE flow with data flash
bit     avrf_write_sector (Uint16);
bit     avrf_read_sector (Uint16);


s_format  * avrf_format (void);



//---- ACCESS DATA FONCTIONS ----

// Standard functions for open in read/write mode the device
Ctrl_status    avrf_read_10( U32 addr , U16 nb_sector );
Ctrl_status    avrf_write_10( U32 addr , U16 nb_sector );


//** If your device transfer have a specific transfer for USB (Particularity of Chejudo product, or bootloader)
#ifdef DF_VALIDATION
#include "virtual_usb.h"
#else
#include "usb_drv.h"    // In this case the driver must be known for the USB access
#endif
//Ctrl_status avr_usb_read( void );
//Ctrl_status avr_usb_write( void );
Ctrl_status avrf_usb_read( void );
Ctrl_status avrf_usb_write( void );



#endif   // _DFMEM_H_

/** @} */
