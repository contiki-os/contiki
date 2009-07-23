/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ctrl_access.c *********************************************************************
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


//_____ I N C L U D E S ____________________________________________________

#include "config.h"
#include "storage/ctrl_access.h"
#include "avr_flash.h"


//_____ D E F I N I T I O N S ______________________________________________

#if (ACCESS_MEM_TO_MEM == ENABLED)
   #include "modules/file_system/fat.h"
   U8 buf_sector[FS_SIZE_OF_SECTOR];
#endif


//! Computed the maximum of static lun (don't add the lun of Mass Storage in mode USB Host)
// FYC: Memory = Logical Unit



               // CHECK FOR LUN DEFINE
#ifndef  LUN_0
   #  error LUN_0 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_1
   #  error LUN_1 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_2
   #  error LUN_2 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_3
   #  error LUN_3 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_4
   #  error LUN_4 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_5
   #  error LUN_5 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_6
   #  error LUN_6 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_7
   #  error LUN_7 must be defined with ENABLE or DISABLE in conf_access.h
#endif
#ifndef  LUN_USB
   #  error LUN_USB must be defined with ENABLE or DISABLE in conf_access.h
#endif



#if (LUN_0 == ENABLE)
   #define     LUN_0_EN   1
   U8 FLASH  lun0_name[]=LUN_0_NAME;
   #else
   #define   LUN_0_EN   0
#endif
#if (LUN_1 == ENABLE)
   #define     LUN_1_EN   1
   U8 FLASH  lun1_name[]=LUN_1_NAME;
   #else
   #define     LUN_1_EN   0
#endif
#if (LUN_2 == ENABLE)
   #define     LUN_2_EN   1
   U8 FLASH  lun2_name[]=LUN_2_NAME;
   #else
   #define     LUN_2_EN   0
#endif
#if (LUN_3 == ENABLE)
   #define     LUN_3_EN   1
   U8 FLASH  lun3_name[]=LUN_3_NAME;
   #else
   #define     LUN_3_EN   0
#endif
#if (LUN_4 == ENABLE)
   #define     LUN_4_EN   1
   U8 FLASH  lun4_name[]=LUN_4_NAME;
   #else
   #define     LUN_4_EN   0
#endif
#if (LUN_5 == ENABLE)
   #define     LUN_5_EN   1
   U8 FLASH  lun5_name[]=LUN_5_NAME;
   #else
   #define     LUN_5_EN   0
#endif
#if (LUN_6 == ENABLE)
   #define     LUN_6_EN   1
   U8 FLASH  lun6_name[]=LUN_6_NAME;
   #else
   #define     LUN_6_EN   0
#endif
#if (LUN_7 == ENABLE)
   #define     LUN_7_EN   1
   U8 FLASH  lun7_name[]=LUN_7_NAME;
   #else
   #define     LUN_7_EN   0
#endif
#if (LUN_USB == ENABLE)
   #define     LUN_USB_EN   1
   U8 FLASH  lunusb_name[]=LUN_USB_NAME;
   #else
   #define     LUN_USB_EN   0
#endif


#define  LUN_ID_0        (0)
#define  LUN_ID_1        (LUN_0_EN)
#define  LUN_ID_2        (LUN_0_EN+LUN_1_EN)
#define  LUN_ID_3        (LUN_0_EN+LUN_1_EN+LUN_2_EN)
#define  LUN_ID_4        (LUN_0_EN+LUN_1_EN+LUN_2_EN+LUN_3_EN)
#define  LUN_ID_5        (LUN_0_EN+LUN_1_EN+LUN_2_EN+LUN_3_EN+LUN_4_EN)
#define  LUN_ID_6        (LUN_0_EN+LUN_1_EN+LUN_2_EN+LUN_3_EN+LUN_4_EN+LUN_5_EN)
#define  LUN_ID_7        (LUN_0_EN+LUN_1_EN+LUN_2_EN+LUN_3_EN+LUN_4_EN+LUN_5_EN+LUN_6_EN)
#define  MAX_LUN         (LUN_0_EN+LUN_1_EN+LUN_2_EN+LUN_3_EN+LUN_4_EN+LUN_5_EN+LUN_6_EN+LUN_7_EN)
#define  LUN_ID_USB      (MAX_LUN)

// Check configuration
#if (MAX_LUN == 0)
   #error No memory is active in conf_access.h
#endif

// Write protect variable
#if (GLOBAL_WR_PROTECT == ENABLED)
   static U8 g_u8_wr_protect;
#endif


//_____ D E F I N I T I O N S __ F O N C T I O N S _________________________


//! This fonction return the number of logical unit
//!
//! @return U8   number of logical unit in the system
//!
U8    get_nb_lun()
{
#if   (MEM_USB == ENABLED)
   return   (MAX_LUN + Host_getlun());
#else
   return   MAX_LUN;
#endif
}


//! This fonction return the current logical unit
//!
//! @return U8   number of logical unit in the system
//!
U8    get_cur_lun()
{
   return   0; //TODO
}


//! This fonction test the state of memory, and start the initialisation of the memory
//!
//! MORE (see SPC-3 §5.2.4) : The TEST UNIT READY command allows an application client
//! to poll a logical unit until it is ready without the need to allocate space for returned data.
//! The TEST UNIT READY command may be used to check the media status of logical units with removable media.
//!
//! @param lun        Logical unit number
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!   Not initialize ->    CTRL_BUSY
//!
Ctrl_status mem_test_unit_ready( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         return Lun_0_test_unit_ready();
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         return Lun_1_test_unit_ready();
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         return Lun_2_test_unit_ready();
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         return Lun_3_test_unit_ready();
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         return Lun_4_test_unit_ready();
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         return Lun_5_test_unit_ready();
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         return Lun_6_test_unit_ready();
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         return Lun_7_test_unit_ready();
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return Lun_usb_test_unit_ready(lun - LUN_ID_USB);
         break;
#     endif
   }
   return   CTRL_FAIL;
}

//! This fonction return the capacity of the memory
//!
//! @param lun        Logical unit number
//! @param u32_nb_sector The sector to query
//!
//! @return *u16_nb_sector number of sector (sector = 512B)
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status mem_read_capacity( U8 lun , U32 _MEM_TYPE_SLOW_ *u32_nb_sector )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         return Lun_0_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         return Lun_1_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         return Lun_2_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         return Lun_3_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         return Lun_4_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         return Lun_5_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         return Lun_6_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         return Lun_7_read_capacity( u32_nb_sector );
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return Lun_usb_read_capacity( lun - LUN_ID_USB,u32_nb_sector );
         break;
#     endif
   }
   return   CTRL_FAIL;
}

//! This fonction return is the write protected mode
//!
//! @param lun        Logical unit number
//!
//! Only used by memory removal with a HARDWARE SPECIFIC write protected detection
//! !!! The customer must be unplug the card for change this write protected mode.
//!
//! @return TRUE  -> the memory is protected
//!
Bool  mem_wr_protect( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         return Lun_0_wr_protect();
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         return Lun_1_wr_protect();
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         return Lun_2_wr_protect();
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         return Lun_3_wr_protect();
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         return Lun_4_wr_protect();
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         return Lun_5_wr_protect();
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         return Lun_6_wr_protect();
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         return Lun_7_wr_protect();
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return Lun_usb_wr_protect(lun - LUN_ID_USB);
         break;
#     endif
   }
   return   CTRL_FAIL;
}


//! This fonction inform about the memory type
//!
//! @param lun        Logical unit number
//!
//! @return TRUE  -> The memory is removal
//!
Bool  mem_removal( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         return Lun_0_removal();
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         return Lun_1_removal();
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         return Lun_2_removal();
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         return Lun_3_removal();
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         return Lun_4_removal();
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         return Lun_5_removal();
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         return Lun_6_removal();
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         return Lun_7_removal();
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return Lun_usb_removal();
         break;
#     endif
   }
   return   CTRL_FAIL;
}

//! This fonction returns a pointer to the LUN name
//!
//! @param lun        Logical unit number
//!
//! @return pointer to code string
//!
#if 0         //not used anywhere and the FLASH attribute causes a compilation warning - dak
U8 FLASH*  mem_name( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         return (U8 FLASH*)lun0_name;
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         return (U8 FLASH*)lun1_name;
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         return (U8 FLASH*)lun2_name;
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         return (U8 FLASH*)lun3_name;
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         return (U8 FLASH*)lun4_name;
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         return (U8 FLASH*)lun5_name;
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         return (U8 FLASH*)lun6_name;
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         return (U8 FLASH*)lun7_name;
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return (U8 FLASH*)lunusb_name;
         break;
#     endif
   }
   return 0;   // Remove compiler warning
}
#endif /* 0 */

//************************************************************************************
//!----------- Listing of READ/WRITE interface with MODE ACCESS REGISTER -------------
//************************************************************************************


//! This fonction tranfer a data from memory to usb
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start read (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    memory_2_usb( U8 lun , U32 addr , U16 nb_sector )
{
   Ctrl_status status=0;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         status = Lun_0_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_0_usb_read();
         }
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         status = Lun_1_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_1_usb_read();
         }
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         status = Lun_2_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_2_usb_read();
         }
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         status = Lun_3_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_3_usb_read();
         }
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         status = Lun_4_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_4_usb_read();
         }
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         status = Lun_5_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_5_usb_read();
         }
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         status = Lun_6_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_6_usb_read();
         }
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         status = Lun_7_read_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_7_usb_read();
         }
         break;
#     endif
   }
   return   status;
}

//! This fonction trabsfer a data from usb to memory
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start write (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    usb_2_memory( U8 lun , U32 addr , U16 nb_sector )
{
   Ctrl_status status=0;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         status = Lun_0_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_0_usb_write();
         }
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         status = Lun_1_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_1_usb_write();
         }
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         status = Lun_2_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_2_usb_write();
         }
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         status = Lun_3_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_3_usb_write();
         }
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         status = Lun_4_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_4_usb_write();
         }
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         status = Lun_5_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_5_usb_write();
         }
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         status = Lun_6_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_6_usb_write();
         }
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         status = Lun_7_write_10(addr , nb_sector);
         if (CTRL_GOOD == status)
         {
            status = Lun_7_usb_write();
         }
         break;
#     endif
   }
   return   status;
}

//! Interface for RAM
#if (ACCESS_MEM_TO_RAM == ENABLED)

//! This fonction tranfer one sector data from memory to ram
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start read (sector = 512B)
//! @param ram          Adresse of ram buffer (only xdata)
//!
//! @return TRUE  -> The memory is removal
//!
Ctrl_status    memory_2_ram( U8 lun , const U32 _MEM_TYPE_SLOW_ *addr , U8 _MEM_TYPE_SLOW_ *ram )
{

   Ctrl_status status;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         status = Lun_0_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_0_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         status = Lun_1_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_1_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         status = Lun_2_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_2_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         status = Lun_3_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_3_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         status = Lun_4_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_4_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         status = Lun_5_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_5_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         status = Lun_6_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_6_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         status = Lun_7_mem_2_ram(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_7_mem_2_ram_read();
         }
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return  Lun_usb_mem_2_ram(*addr , ram);
#     endif
   }
   return   status;
}
#endif // ACCESS_MEM_TO_RAM == ENABLED



#if (ACCESS_MEM_TO_RAM==ENABLE)

//! This fonction trabsfer a data from ram to memory
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start write (sector = 512B)
//! @param ram          Adresse of ram buffer (only xdata)
//!
//! @return TRUE  -> The memory is removal
//!
Ctrl_status    ram_2_memory( U8 lun , const U32 _MEM_TYPE_SLOW_ *addr , U8 _MEM_TYPE_SLOW_ * ram )
{
   Ctrl_status status;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
         status = Lun_0_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_0_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
         status = Lun_1_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_1_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
         status = Lun_2_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_2_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
         status = Lun_3_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_3_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
         status = Lun_4_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_4_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
         status = Lun_5_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_5_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
         status = Lun_6_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_6_ram_2_mem_write();
         }
         break;
#     endif
 #     if (LUN_7 == ENABLE)
      case LUN_ID_7:
         status = Lun_7_ram_2_mem(*addr , ram);
         if (CTRL_GOOD == status)
         {
            status = Lun_7_ram_2_mem_write();
         }
         break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
         return  Lun_usb_ram_2_mem(*addr , ram);
         break;
#     endif
   }
   return   status;
}
#endif   // ACCESS_RAM_TO_MEM == ENABLED



//! Interface for streaming interface
#if (ACCESS_STREAM == ENABLED)



// Interface for transfer MEM to MEM
#     if (ACCESS_MEM_TO_MEM == ENABLED)
//! This fonction copy a data from memory to other memory
//!
//! @param src_lun The LUN of the source
//! @param src_addr The address of the source
//! @param dest_lun The LUN of the destination
//! @param dest_addr The address of the destination
//! @param nb_sector    Number of sectors to transfer
//!
U8    stream_mem_to_mem( U8 src_lun , U32 src_addr , U8 dest_lun , U32 dest_addr , U16 nb_sector )
{
   while(nb_sector)
   {
      memory_2_ram( src_lun , &src_addr , buf_sector );
      ram_2_memory( dest_lun , &dest_addr , buf_sector );
      src_addr++;
      dest_addr++;
      nb_sector--;
   }
   return CTRL_GOOD;
}
#     endif   // ACCESS_MEM_TO_MEM == ENABLED




//! Returns the state on a data transfer
//!
//! @param Id           transfer id
//!
//! @return the state of the transfer
//!          CTRL_GOOD              It is finish
//!          CTRL_BUSY              It is running
//!          CTRL_FAIL              It is fail
//!
Ctrl_status stream_state( U8 Id )
{

   return CTRL_GOOD;
}

//! Stop the data transfer
//!
//! @param Id  Transfer id
//!
//! @return the number of sector remainder
//!
U16 stream_stop( U8 Id )
{
   return 0;
}

#endif   // ACCESS_STREAM == ENABLED

/** @} */
