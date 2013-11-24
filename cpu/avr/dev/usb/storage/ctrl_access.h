/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ctrl_access.h *******************************************************
 *
 * \brief
 *      This file contains the interface :
 *         - between USB <-> MEMORY
 *      OR
 *         - between USB <- Access Memory Ctrl -> Memory
 *
 *      This interface may be controled by a "Access Memory Control" for :
 *         - include a management of write protect global or specific
 *         - include a management of access password
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 ******************************************************************************/
/* 
   Copyright (c) 2004  ATMEL Corporation
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

#ifndef MEM_CTRL_H_
#define MEM_CTRL_H_

#include "storage/conf_access.h"
#include "storage/ctrl_status.h"

// FYC: Memory = Logical Unit
#if (LUN_0 == ENABLE)
   #include    LUN_0_INCLUDE
#endif
#if (LUN_1 == ENABLE)
   #include    LUN_1_INCLUDE
#endif
#if (LUN_2 == ENABLE)
   #include    LUN_2_INCLUDE
#endif
#if (LUN_3 == ENABLE)
   #include    LUN_3_INCLUDE
#endif
#if (LUN_4 == ENABLE)
   #include    LUN_4_INCLUDE
#endif
#if (LUN_5 == ENABLE)
   #include    LUN_5_INCLUDE
#endif
#if (LUN_6 == ENABLE)
   #include    LUN_6_INCLUDE
#endif
#if (LUN_7 == ENABLE)
   #include    LUN_7_INCLUDE
#endif
#if (LUN_USB == ENABLE)
   #include    LUN_USB_INCLUDE
#endif


//------- Test the configuration in conf_access.h

// Specific option control access
#ifndef  GLOBAL_WR_PROTECT
#  error GLOBAL_WR_PROTECT must be defined with ENABLED or DISABLED in conf_access.h
#endif
//------- END OF Test the configuration in conf_access.h



//_____ D E F I N I T I O N S ______________________________________________



//!**** Listing of commun interface ****************************************

U8             get_nb_lun();
U8             get_cur_lun();
Ctrl_status    mem_test_unit_ready( U8 lun );
Ctrl_status    mem_read_capacity( U8 lun , U32 _MEM_TYPE_SLOW_ *u32_nb_sector );
Bool           mem_wr_protect( U8 lun );
Bool           mem_removal( U8 lun );
U8 FLASH*       mem_name( U8 lun );


//!**** Listing of READ/WRITE interface ************************************


//---- Interface for USB ---------------------------------------------------
   Ctrl_status memory_2_usb( U8 lun , U32 addr , U16 nb_sector );
   Ctrl_status usb_2_memory( U8 lun , U32 addr , U16 nb_sector );
#include "usb_drv.h"
#include "conf_usb.h"
//--------------------------------------------------------------------------

//---- Interface for RAM to MEM --------------------------------------------
Ctrl_status    memory_2_ram( U8 lun , const U32 _MEM_TYPE_SLOW_ *addr , U8 _MEM_TYPE_SLOW_ *ram );
Ctrl_status    ram_2_memory( U8 lun , const U32 _MEM_TYPE_SLOW_ *addr , U8 _MEM_TYPE_SLOW_ * ram );
//--------------------------------------------------------------------------


//!---- Interface for streaming interface ----------------------------------
#if (ACCESS_STREAM == ENABLED)

#define  ID_STREAM_ERR        0xFF


// RLE #include "dfc_mngt.h"

#define  TYPE_STREAM_AUDIO    DFC_ID_AUDIOPROC1
#define  TYPE_STREAM_AUDIO2   DFC_ID_AUDIOPROC2
#define  TYPE_STREAM_SIO      DFC_ID_SIO
#define  TYPE_STREAM_SPI      DFC_ID_SPI
#define  TYPE_STREAM_DEVNULL  DFC_ID_NULL_DEV

   U8          stream_mem_to_mem       ( U8 src_lun , U32 src_addr , U8 dest_lun , U32 dest_addr , U16 nb_sector );
   U8          stream_read_10_start    ( U8 TypeStream , U8 lun , U32 addr , U16 nb_sector );
   U8          stream_write_10_start   ( U8 TypeStream , U8 lun , U32 addr , U16 nb_sector );
   U8          stream_to_stream_start  ( U8 TypeStream_src , U8 TypeStream_dest  , U16 nb_sector );
   Ctrl_status stream_state( U8 Id );
   U16         stream_stop( U8 Id );

#endif   // (ACCESS_STREAM == ENABLED)
//--------------------------------------------------------------------------

#endif   //MEM_CTRL_H_
/**@}*/
