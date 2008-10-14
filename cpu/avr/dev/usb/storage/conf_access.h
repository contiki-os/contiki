/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file conf_access.h *********************************************************************
 *
 * \brief
 *      This file contains the possible external configuration of the control access
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

#ifndef _CONF_ACCESS_H_
#define _CONF_ACCESS_H_


// Active the Logical Unit
#define  LUN_0                DISABLE   // On-Chip flash vitual memory
#define  LUN_1                DISABLE   // NF 2KB
#define  LUN_2                DISABLE   // NF 512B
#define  LUN_3                DISABLE    // Data Flash
#define  LUN_4                ENABLE     //On-chip 32KB
#define  LUN_5                DISABLE
#define  LUN_6                DISABLE
#define  LUN_7                DISABLE
#define  LUN_USB              DISABLE

// LUN 0 DEFINE
#define  LUN_0_INCLUDE                           "lib_mem\virtual_mem\virtual_mem.h"
#define  Lun_0_test_unit_ready()                 virtual_test_unit_ready()
#define  Lun_0_read_capacity(nb_sect)            virtual_read_capacity(nb_sect)
#define  Lun_0_wr_protect()                      virtual_wr_protect()
#define  Lun_0_removal()                         virtual_removal()
#define  Lun_0_read_10(ad, sec)                  virtual_read_10(ad, sec)
#define  Lun_0_usb_read()                        virtual_usb_read()
#define  Lun_0_write_10(ad, sec)                 virtual_write_10(ad, sec)
#define  Lun_0_usb_write()                       virtual_usb_write()
#define  Lun_0_ram_2_mem(addr , ram)             virtual_ram_2_mem(addr, ram)
#define  Lun_0_ram_2_mem_write()                 virtual_ram_2_mem_write()
#define  Lun_0_mem_2_ram(addr , ram)             virtual_mem_2_ram(addr, ram)
#define  Lun_0_mem_2_ram_read()                  virtual_mem_2_ram_read()
#define  LUN_0_NAME                              "VIRTUAL_MEM_ON_CHIP"

// LUN 1 DEFINE
#if (LUN_1 == ENABLE)
#define  NF_2K_MEM                               ENABLE
#else
#define	 NF_2K_MEM                               DISABLE
#endif
#define  LUN_1_INCLUDE                           "lib_mem\nf_1x_2kb\nf_mem.h"
#define  Lun_1_test_unit_ready()                 nf_test_unit_ready()
#define  Lun_1_read_capacity(nb_sect)            nf_read_capacity(nb_sect)
#define  Lun_1_wr_protect()                      nf_wr_protect()
#define  Lun_1_removal()                         nf_removal()
#define  Lun_1_read_10(ad, sec)                  nf_read_10(ad, sec)
#define  Lun_1_usb_read()                        nf_usb_read()
#define  Lun_1_write_10(ad, sec)                 nf_write_10(ad, sec)
#define  Lun_1_usb_write()                       nf_usb_write()
#define  Lun_1_ram_2_mem(addr , ram)             TODO
#define  Lun_1_ram_2_mem_write()                 TODO
#define  Lun_1_mem_2_ram(addr , ram)             TODO
#define  Lun_1_mem_2_ram_read()                  TODO

// LUN 2 DEFINE
#if (LUN_2 == ENABLE)
#define  NF_512_MEM                              ENABLE
#else
#define	 NF_512_MEM                             DISABLE
#endif
#define  LUN_2_INCLUDE                           "lib_mem\nf_1x_512\nf_mem.h"
#define  Lun_2_test_unit_ready()                 nf_test_unit_ready()
#define  Lun_2_read_capacity(nb_sect)            nf_read_capacity(nb_sect)
#define  Lun_2_wr_protect()                      nf_wr_protect()
#define  Lun_2_removal()                         nf_removal()
#define  Lun_2_read_10(ad, sec)                  nf_read_10(ad, sec)
#define  Lun_2_usb_read()                        nf_usb_read()
#define  Lun_2_write_10(ad, sec)                 nf_write_10(ad, sec)
#define  Lun_2_usb_write()                       nf_usb_write()
#define  Lun_2_ram_2_mem(addr , ram)             TODO
#define  Lun_2_ram_2_mem_write()                 TODO
#define  Lun_2_mem_2_ram(addr , ram)             TODO
#define  Lun_2_mem_2_ram_read()                  TODO

// LUN 3 DEFINE
#if (LUN_3 == ENABLE)
#define  DF_MEM                                  ENABLE
#else
#define	DF_MEM                                  DISABLE
#endif
#define  LUN_ID_DF                               LUN_ID_3
#define  LUN_3_INCLUDE                           "lib_mem\df\df_mem.h"
#define  Lun_3_test_unit_ready()                 df_test_unit_ready()
#define  Lun_3_read_capacity(nb_sect)            df_read_capacity(nb_sect)
#define  Lun_3_wr_protect()                      df_wr_protect()
#define  Lun_3_removal()                         df_removal()
#define  Lun_3_read_10(ad, sec)                  df_read_10(ad, sec)
#define  Lun_3_usb_read()                        df_usb_read()
#define  Lun_3_write_10(ad, sec)                 df_write_10(ad, sec)
#define  Lun_3_usb_write()                       df_usb_write()
#define  Lun_3_ram_2_mem(addr , ram)             df_ram_2_df(addr, ram)
#define  Lun_3_ram_2_mem_write()                 df_ram_2_df_write()
#define  Lun_3_mem_2_ram(addr , ram)             df_df_2_ram(addr, ram)
#define  Lun_3_mem_2_ram_read()                  df_df_2_ram_read()
#define  LUN_3_NAME                               "\"On board data flash\""

// LUN 3 DEFINE
#if (LUN_3 == ENABLE)
#define  AVR_MEM                                  ENABLE
#else
#define	AVR_MEM                                  DISABLE
#endif
#define  LUN_ID_AVRF                             LUN_ID_4
#define  LUN_4_INCLUDE                           "storage/avr_flash.h"
#define  Lun_4_test_unit_ready()                 avrf_test_unit_ready()            
#define  Lun_4_read_capacity(nb_sect)            avrf_read_capacity(nb_sect)
#define  Lun_4_wr_protect()                      avrf_wr_protect()
#define  Lun_4_removal()                         avrf_removal()
#define  Lun_4_read_10(ad, sec)                  avrf_read_10(ad, sec)
#define  Lun_4_usb_read()                        avrf_usb_read()
#define  Lun_4_write_10(ad, sec)                 avrf_write_10(ad,sec)
#define  Lun_4_usb_write()                       avrf_usb_write()
#define  Lun_4_ram_2_mem(addr , ram)             TODO
#define  Lun_4_ram_2_mem_write()                 TODO
#define  Lun_4_mem_2_ram(addr , ram)             TODO
#define  Lun_4_mem_2_ram_read()                  TODO
#define  LUN_4_NAME                               "\"AVR Flash Memory\""


// LUN USB DEFINE
#if (LUN_USB == ENABLE)
#define  MEM_USB                                  ENABLE
#else
#define	MEM_USB                                  DISABLE
#endif
#define  LUN_USB_INCLUDE                           "lib_mem\host_mem\host_mem.h"
#define  Lun_usb_test_unit_ready(lun)              host_test_unit_ready(lun)
#define  Lun_usb_read_capacity(lun,nb_sect)        host_read_capacity(lun,nb_sect)
#define  Lun_usb_wr_protect(lun)                   host_wr_protect(lun)
#define  Lun_usb_removal()                         host_removal()
#define  Lun_usb_ram_2_mem(addr , ram)             host_write_10_ram(addr, ram)
#define  Lun_usb_mem_2_ram(addr , ram)             host_read_10_ram(addr, ram)
#define  LUN_USB_NAME                              "\"USB Remote memory\""


// ATMEL DEFINE - DO NOT MODIFY

// Active interface fonction
#define  ACCESS_USB           ENABLED  // USB I/O in control access
#define  ACCESS_STREAM        DISABLED  // Stream I/O in control access
#define  ACCESS_STREAM_RECORD DISABLED // Stream I/O in control access AND RECORD MODE
#define  ACCESS_MEM_TO_RAM    DISABLED  // RAM to Mem I/O in control access
#define  ACCESS_MEM_TO_MEM    DISABLED // Mem to Mem I/O in control access



// Specific option control access
#define  GLOBAL_WR_PROTECT    DISABLED // To manage a global write protection


#endif  //! _CONF_ACCESS_H_

/** @} */
