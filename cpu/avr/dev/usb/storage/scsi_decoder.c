/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file scsi_decoder.c *******************************************************
 *
 * \brief
 *      This file is the scsi decoder
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

//_____  I N C L U D E S ___________________________________________________

#include "config.h"
#include "storage/scsi_decoder.h"
#include "conf_usb.h"
#include "usb_drv.h"                   
#include "storage/ctrl_status.h"
#include "storage/ctrl_access.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


U8  g_scsi_command[16];
U8  g_scsi_status;
U32 g_scsi_data_remaining;

FLASH U8    g_sbc_vendor_id[8]   = SBC_VENDOR_ID;
FLASH U8    g_sbc_product_id[16] = SBC_PRODUCT_ID;
FLASH U8    g_sbc_revision_id[4] = SBC_REVISION_ID;

extern  U8  usb_LUN;

 s_scsi_sense  g_scsi_sense;


FLASH struct sbc_st_std_inquiry_data sbc_std_inquiry_data =
{
   /* Byte 0 : 0x00 */
   0x00,        /* DeviceType: Direct-access device */
   0,           /* PeripheralQualifier : Currently connected */

   /* Byte 1 : 0x80 */
   0,           /* Reserved1 */
   1,           /* RMB : Medium is removable (this bit must be at 1, else the medium isn't see on Windows) */

 //  /* Byte 2 : 0x02 */
 //  0x02,        /* Version: Device compliant to ANSI X3.131:1994 */

   /* Byte 2 : 0x00 */
   0x00,        /* Version: Device not compliant to any standard */

   /* Byte 3 : 0x02 */
   2,           /* Response data format */
   0,           /* NormACA */
   0,           /* Obsolete0 */
   0,           /* AERC */

   /* Byte 4 : 0x1F */
   /* Byte 5 : 0x00 */
   /* Byte 6 : 0x00 */
                /* Reserved4[3] */
   {
      0x1F,     /* Additional Length (n-4) */
      0,        /* SCCS : SCC supported */
      0
   },

   /* Byte 7 : 0x00 */
   0,          /* SoftReset */
   0,          /* CommandQueue */
   0,          /* Reserved5 */
   0,          /* LinkedCommands */
   0,          /* Synchronous */
   0,          /* Wide16Bit */
   0,          /* Wide32Bit */
   0,          /* RelativeAddressing */
};


static  void  send_informational_exceptions_page (void);
static  void  send_read_write_error_recovery_page (U8);
static  void sbc_header_mode_sense( Bool b_sense_10 , U8 u8_data_length );


//_____ D E C L A R A T I O N S ____________________________________________
/**
 * @brief SCSI decoder function
 *
 * This function read the SCSI command and launches the appropriate function
 *
 * @warning Code:.. bytes (function FLASH length)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
Bool scsi_decode_command(void)
{
Bool status;

   if (g_scsi_command[0] == SBC_CMD_WRITE_10)
   {
      Scsi_start_write_action();
      status = sbc_write_10();
	  Scsi_stop_write_action();
      return status;
   }
   if (g_scsi_command[0] == SBC_CMD_READ_10 )
   {
      Scsi_start_read_action();
      status = sbc_read_10();
      Scsi_stop_read_action();
      return status;
   }

   switch (g_scsi_command[0])                /* check other command received */
   {
      case SBC_CMD_REQUEST_SENSE:             /* 0x03 - Mandatory */
           return sbc_request_sense();
           break;

      case SBC_CMD_INQUIRY:                   /* 0x12 - Mandatory */
           return sbc_inquiry();
           break;

      case SBC_CMD_TEST_UNIT_READY:           /* 0x00 - Mandatory */
           return sbc_test_unit_ready();
           break;

      case SBC_CMD_READ_CAPACITY:             /* 0x25 - Mandatory */
           return sbc_read_capacity();
           break;

      case SBC_CMD_MODE_SENSE_6:              /* 0x1A - Optional */
           return sbc_mode_sense( FALSE );
           break;

      case SBC_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:/* 0x1E */
           return sbc_prevent_allow_medium_removal();
           break;

      case SBC_CMD_VERIFY_10:                 /* 0x2F - Optional */
           sbc_lun_status_is_good();
           break;
      case SBC_CMD_MODE_SENSE_10:             /* 0x5A - Optional */
           return sbc_mode_sense( TRUE );
           break;

      case SBC_CMD_FORMAT_UNIT:               /* 0x04 - Mandatory */

      case SBC_CMD_MODE_SELECT_6:             /* 0x15 - Optional */




      case SBC_CMD_START_STOP_UNIT:           /* 0x1B - Optional */
      case SBC_CMD_SEND_DIAGNOSTIC:           /* 0x1D -  */
      case SBC_CMD_READ_LONG:                 /* 0x23 - Optional */
      case SBC_CMD_SYNCHRONIZE_CACHE:         /* 0x35 - Optional */
      case SBC_CMD_WRITE_BUFFER:              /* 0x3B - Optional */
      case SBC_CMD_RESERVE_10:                /* 0x56 - Mandatory */
      case SBC_CMD_RELEASE_10:                /* 0x57 - Mandatory - see chapter 7.16 - SPC 2 */
      default:
           { /* Command not supported */
              Sbc_send_failed();
              Sbc_build_sense(SBC_SENSE_KEY_ILLEGAL_REQUEST, SBC_ASC_INVALID_COMMAND_OPERATION_CODE, 0x00);
              return FALSE;
              break;
           }
   }
   return TRUE;
}


/**
 * @brief This function manages the SCSI REQUEST SENSE command (0x03)
 *
 * The SCSI Sense contains the status of the last command
 * This status is composed of 3 Bytes :
 * - sense key  (g_scsi_sense.key)
 * - additional sense code  (g_scsi_sense.asc)
 * - additional sense code qualifier  (g_scsi_sense.ascq)
 *
 * @warning Code:.. bytes (function code length)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
Bool sbc_request_sense (void)
{
  U8  allocation_length, i;
  U8  request_sens_output[18];   /* the maximum size of request is 17 */

  allocation_length = g_scsi_command[4];  /* Allocation length */

  /* Initialize the request sense data */
  request_sens_output[0] = SBC_RESPONSE_CODE_SENSE; /* 70h */
  request_sens_output[1] = 0x00;                    /* Obsolete */
  request_sens_output[2] = g_scsi_sense.key;

  request_sens_output[3] = 0x00;   /* For direct access media, Information field */
  request_sens_output[4] = 0x00;   /* give the unsigned logical block */
  request_sens_output[5] = 0x00;   /* address associated with the sense key */
  request_sens_output[6] = 0x00;

  request_sens_output[7] = SBC_ADDITIONAL_SENSE_LENGTH; /* !! UFI device shall not adjust the Additional sense length to reflect truncation */
  request_sens_output[8] = SBC_COMMAND_SPECIFIC_INFORMATION_3;
  request_sens_output[9] = SBC_COMMAND_SPECIFIC_INFORMATION_2;
  request_sens_output[10] = SBC_COMMAND_SPECIFIC_INFORMATION_1;
  request_sens_output[11] = SBC_COMMAND_SPECIFIC_INFORMATION_0;

  request_sens_output[12] = g_scsi_sense.asc;
  request_sens_output[13] = g_scsi_sense.ascq;

  request_sens_output[14] = SBC_FIELD_REPLACEABLE_UNIT_CODE;
  request_sens_output[15] = SBC_SENSE_KEY_SPECIFIC_2;
  request_sens_output[16] = SBC_SENSE_KEY_SPECIFIC_1;
  request_sens_output[17] = SBC_SENSE_KEY_SPECIFIC_0;

  /* Send the request data */
  for( i=0 ; i<allocation_length ; i++ )
  {
    Usb_write_byte( request_sens_output[i] );
  }
  Sbc_valid_write_usb( allocation_length );

  sbc_lun_status_is_good();

  return TRUE;
}

/**
 * @brief This function manages the SCSI INQUIRY command (0x12)
 *
 * The SCSI Inquiry field contains information regarding parameters
 * of the target. For example:
 * - vendor identification
 * - product identification
 * - peripheral qualifier
 * - peripheral device type
 * - etc
 *
 * @warning Code:.. bytes (function code length)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
Bool sbc_inquiry (void)
{
   U8 allocation_length, i;

#ifdef AVRGCC
   PGM_VOID_P ptr;
#else
   U8 FLASH *ptr;
#endif

   if( (0 == (g_scsi_command[1] & 0x03) )    // CMDT and EPVD bits are 0
   &&  (0 ==  g_scsi_command[2]         ) )  // PAGE or OPERATION CODE fields = 0x00?
   {
      //** send standard inquiry data

      // Check the size of inquiry data
      allocation_length = g_scsi_command[4];
      if (allocation_length > SBC_MAX_INQUIRY_DATA)
      {
         allocation_length = SBC_MAX_INQUIRY_DATA;
      }

      // send first inquiry data (0 to 8)
      ptr = (FLASH U8*) &sbc_std_inquiry_data;

      for ( i=0 ; ((i != 36) && (allocation_length > i)); i++)
      {
         if( 8 == i )
         {  // send vendor id (8 to 16)
              ptr = (FLASH U8 *) &g_sbc_vendor_id;
         }
         if( 16 == i )
         {  // send product id (16 to 32)
            ptr = (FLASH U8 *) &g_sbc_product_id;
         }
         if( 32 == i )
         {  // send revision id (32 to 36)
            ptr = (FLASH U8 *) &g_sbc_revision_id;
         }
#ifndef AVRGCC
         Usb_write_byte((U8)(*ptr++));     // send tab
#else    // AVRGCC does not support point to PGM space
#warning with avrgcc assumes devices descriptors are stored in the lower 64Kbytes of on-chip flash memory
         Usb_write_byte(pgm_read_byte_near((unsigned int)ptr++));
#endif

      }

      //  send data (36 to SBC_MAX_INQUIRY_DATA), and can be tranmitted by Bulk
      //  Description of next bytes (this bytes is always egal to 0) :
      //  VendorSpecific    : 20 Bytes
      //  Next byte         : 1 byte
      //       - InfoUnitSupport   : 1 bit
      //       - QuickArbitSupport : 1 bit
      //       - Clocking          : 2 bits
      //       - Reserved6         : 4 bits
      //  Reserved7         : 1 byte
      //  VersionDescriptor : 8 bytes
      //  Reserved8         : 22 bytes
      //  ...
      while( allocation_length > i )
      {
         if (64 == i)
         {  // for each 64 bytes, send USB packet
            Sbc_valid_write_usb(64);
            allocation_length -= 64;
            i = 0;
         }
         Usb_write_byte(0);       // write value of last bytes of inquiry data
         i++;
      }
      // send last USB packet
      Sbc_valid_write_usb(allocation_length);
      sbc_lun_status_is_good();
      return TRUE;
   }
   else
   {  // (CMDT=EVPD <> 0) or (PAGE CODE <> 0x00)
      Sbc_send_failed();
      Sbc_build_sense(SBC_SENSE_KEY_ILLEGAL_REQUEST, SBC_ASC_INVALID_FIELD_IN_CDB, 0x00);
      return FALSE;
   }
}


Bool sbc_test_unit_ready(void)
{
   switch ( mem_test_unit_ready(usb_LUN) )
   {
   case CTRL_GOOD :
      sbc_lun_status_is_good();
      break;

   case CTRL_NO_PRESENT :
      sbc_lun_status_is_not_present();
      break;

   case CTRL_BUSY :
      sbc_lun_status_is_busy_or_change();
      break;

   case CTRL_FAIL :
   default :
      sbc_lun_status_is_fail();
      break;
   }
   return TRUE;
}


Bool sbc_read_capacity (void)
{
   _MEM_TYPE_SLOW_ U32 mem_size_nb_sector;

   switch ( mem_read_capacity( usb_LUN, &mem_size_nb_sector ) )
   {
   case CTRL_GOOD :
      Usb_write_byte(MSB0(mem_size_nb_sector));  // return nb block
      Usb_write_byte(MSB1(mem_size_nb_sector));
      Usb_write_byte(MSB2(mem_size_nb_sector));
      Usb_write_byte(MSB3(mem_size_nb_sector));
      Usb_write_byte( 0               );        // return block size (= 512B)
      Usb_write_byte( 0               );
      Usb_write_byte( (U8)(512 >> 8)  );
      Usb_write_byte( (U8)(512 & 0xFF));

      Sbc_valid_write_usb(SBC_READ_CAPACITY_LENGTH);
      sbc_lun_status_is_good();
      return TRUE;
      break;

   case CTRL_NO_PRESENT :
      sbc_lun_status_is_not_present();
      break;

   case CTRL_BUSY :
      sbc_lun_status_is_busy_or_change();
      break;

   case CTRL_FAIL :
   default :
      sbc_lun_status_is_fail();
      break;
   }
   return FALSE;
}


Bool sbc_read_10 (void)
{
   U32   mass_addr;                    // rd or wr block address
   U16   mass_size;                    // rd or write nb of blocks

   MSB0(mass_addr) = g_scsi_command[2];  // read address
   MSB1(mass_addr) = g_scsi_command[3];
   MSB2(mass_addr) = g_scsi_command[4];
   MSB3(mass_addr) = g_scsi_command[5];

   MSB(mass_size) = g_scsi_command[7];  // read size
   LSB(mass_size) = g_scsi_command[8];

   if (mass_size != 0)
   {
      switch ( memory_2_usb( usb_LUN , mass_addr, mass_size ) )
      {
      case CTRL_GOOD :
         sbc_lun_status_is_good();
         g_scsi_data_remaining = g_scsi_data_remaining - (512 * (Uint32)mass_size);
         return TRUE;
         break;

      case CTRL_NO_PRESENT :
         sbc_lun_status_is_not_present();
         return FALSE;
         break;

      case CTRL_BUSY :
         sbc_lun_status_is_busy_or_change();
         return FALSE;
         break;

      case CTRL_FAIL :
      default :
         sbc_lun_status_is_fail();
         return FALSE;
         break;
      }
   }
   else
   {  // No data to transfer
      sbc_lun_status_is_good();
   }
   return TRUE;
}


Bool sbc_write_10 (void)
{
   U32   mass_addr;                    // rd or wr block address
   U16   mass_size;                    // rd or write nb of blocks

   MSB0(mass_addr) = g_scsi_command[2];  // read address
   MSB1(mass_addr) = g_scsi_command[3];
   MSB2(mass_addr) = g_scsi_command[4];
   MSB3(mass_addr) = g_scsi_command[5];

   MSB(mass_size) = g_scsi_command[7];  // read size
   LSB(mass_size) = g_scsi_command[8];

   if (mass_size != 0)
   {
      if( TRUE == mem_wr_protect( usb_LUN ) )
      {
         sbc_lun_status_is_protected();
         return FALSE;
#warning For Win98 data must be read to avoid blocking
      }
      else
      {
         switch (usb_2_memory( usb_LUN , mass_addr, mass_size ))
         {
         case CTRL_GOOD :
            sbc_lun_status_is_good();
            g_scsi_data_remaining = g_scsi_data_remaining - (512 * (Uint32)mass_size);
            return TRUE;
            break;

         case CTRL_NO_PRESENT :
            sbc_lun_status_is_not_present();
            return FALSE;
            break;

         case CTRL_BUSY :
            sbc_lun_status_is_busy_or_change();
            return FALSE;
            break;

         case CTRL_FAIL :
         default :
            sbc_lun_status_is_fail();
            return FALSE;
            break;
         }
      }
   }
   else
   {  // No data to transfer
      sbc_lun_status_is_good();
   }
   return TRUE;
}


/**
 * @brief This function manages the SCSI MODE SENSE command (0x1A for sense 6 and 0x5A for sense 10)
 *
 * The SCSI mode sense function returns parameters to an application client.
 * It is a complementary command to the SCSI MODE SELECT command.
 *
 * @warning Code:.. bytes (function code length)
 *
 * @param b_sense_10 ( TRUE = sense 10, TRUE = sense 6)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
Bool sbc_mode_sense( Bool b_sense_10 )
{
   U8 allocation_length;

   if( b_sense_10 )
      allocation_length = g_scsi_command[8];
   else
      allocation_length = g_scsi_command[4];

   // switch for page code
   switch ( g_scsi_command[2] & SBC_MSK_PAGE_CODE )
   {
      case SBC_PAGE_CODE_INFORMATIONAL_EXCEPTIONS:       /* Page Code: Informational exceptions control page */
         sbc_header_mode_sense( b_sense_10 , SBC_MODE_DATA_LENGTH_INFORMATIONAL_EXCEPTIONS );
         send_informational_exceptions_page();
         Sbc_valid_write_usb(SBC_MODE_DATA_LENGTH_INFORMATIONAL_EXCEPTIONS + 1);
         break;

      case SBC_PAGE_CODE_READ_WRITE_ERROR_RECOVERY:
         sbc_header_mode_sense( b_sense_10 , SBC_MODE_DATA_LENGTH_READ_WRITE_ERROR_RECOVERY );
         send_read_write_error_recovery_page(allocation_length);
         Sbc_valid_write_usb(SBC_MODE_DATA_LENGTH_READ_WRITE_ERROR_RECOVERY + 1);
         break;

      case SBC_PAGE_CODE_ALL:
         sbc_header_mode_sense( b_sense_10 , SBC_MODE_DATA_LENGTH_CODE_ALL );
         if( b_sense_10 )
         {
            if (allocation_length == 8)
            {
               Sbc_valid_write_usb(8);
               break;
            }
         }
         else
         {
            if (allocation_length == 4)
            {
               Sbc_valid_write_usb(4);
               break;
            }
         }
         // send page by ascending order code
         send_read_write_error_recovery_page(allocation_length);  // 12 bytes
         if (allocation_length > 12)
         {
            send_informational_exceptions_page();                 // 12 bytes
            Sbc_valid_write_usb(SBC_MODE_DATA_LENGTH_CODE_ALL + 1);
         }
         else
         {
            Sbc_valid_write_usb(allocation_length);
         }
         break;

      default:
           Sbc_send_failed();
           Sbc_build_sense(SBC_SENSE_KEY_ILLEGAL_REQUEST, SBC_ASC_INVALID_FIELD_IN_CDB, 0x00);
           return FALSE;
           break;
   }
   sbc_lun_status_is_good();
   return TRUE;
}


/**
 * @brief This function send the header of the SCSI MODE SENSE command
 *
 * @param b_sense_10 TRUE = sense 10, FALSE = sense 6
 * @param u8_data_length data length in byte
 *
 */
void sbc_header_mode_sense( Bool b_sense_10 , U8 u8_data_length )
{
   // Send Data length
   if( b_sense_10 )
   {
      Usb_write_byte(0);
   }
   Usb_write_byte( u8_data_length );

   // Send device type
   Usb_write_byte(SBC_MEDIUM_TYPE);

   // Write protect status
   if (mem_wr_protect( usb_LUN ))
   {
      Usb_write_byte(SBC_DEV_SPEC_PARAM_WR_PROTECT);  // Device is write protected
   }
   else
   {
      Usb_write_byte(SBC_DEV_SPEC_PARAM_WR_ENABLE);   // Device is write enabled
   }

   if( b_sense_10 )
   {  // Reserved
      Usb_write_byte(0);
      Usb_write_byte(0);
   }

   // Send Block descriptor length
   if( b_sense_10 )
   {
      Usb_write_byte(0);
   }
   Usb_write_byte(SBC_BLOCK_DESCRIPTOR_LENGTH);
}


/**
 * @brief This function writes informational exceptions page parameters
 *
 * @warning Code:.. bytes (function code length)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
void send_informational_exceptions_page (void)
{
   Usb_write_byte(SBC_PAGE_CODE_INFORMATIONAL_EXCEPTIONS);     /* Page Code: Informational exceptions control page */
                                                               /* See chapter 8.3.8 on SPC-2 specification */
   Usb_write_byte(SBC_PAGE_LENGTH_INFORMATIONAL_EXCEPTIONS);   /* Page Length */
   Usb_write_byte(0x00);                                       /* ..., Test bit = 0, ... */
   Usb_write_byte(SBC_MRIE);                                   /* MRIE = 0x05 */
   Usb_write_byte(0x00);                                       /* Interval Timer (MSB) */
   Usb_write_byte(0x00);
   Usb_write_byte(0x00);
   Usb_write_byte(0x00);                                       /* Interval Timer (LSB) */
   Usb_write_byte(0x00);                                       /* Report Count (MSB) */
   Usb_write_byte(0x00);
   Usb_write_byte(0x00);
   Usb_write_byte(0x01);                                       /* Report Count (LSB) */
}


/**
 * @brief This function writes error recovery page
 *
 * @warning Code:.. bytes (function code length)
 *
 * @param length The length of 
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
void send_read_write_error_recovery_page (U8 length)
{
   Usb_write_byte(SBC_PAGE_CODE_READ_WRITE_ERROR_RECOVERY);

   Usb_write_byte(SBC_PAGE_LENGTH_READ_WRITE_ERROR_RECOVERY);   /* Page Length */
   Usb_write_byte(0x80);
   Usb_write_byte(SBC_READ_RETRY_COUNT);
   Usb_write_byte(SBC_CORRECTION_SPAN);
   Usb_write_byte(SBC_HEAD_OFFSET_COUNT);
   Usb_write_byte(SBC_DATA_STROBE_OFFSET);
   Usb_write_byte(0x00);   /* Reserved */

   if (length > 12)
   {
      Usb_write_byte(SBC_WRITE_RETRY_COUNT);
      Usb_write_byte(0x00);
      Usb_write_byte(SBC_RECOVERY_LIMIT_MSB);
      Usb_write_byte(SBC_RECOVERY_LIMIT_LSB);
   }
}

/**
 * @brief This function manages the SCSI PREVENT ALLOW MEDIUM REMOVAL
 *        command (0x1E)
 *
 * The SCSI prevent allow medium removal command requests that the target
 * enable or disable the removal of the medium in the logical unit.
 *
 * @warning Code:.. bytes (function code length)
 *
 * @return  FALSE: result KO,
 *          TRUE:  result OK
 *
 */
Bool sbc_prevent_allow_medium_removal(void)
{
   sbc_lun_status_is_good();
   return TRUE;
}


//! This fonction send the UFI status GOOD
//!
void sbc_lun_status_is_good(void)
{
   Sbc_send_good();
   Sbc_build_sense(SBC_SENSE_KEY_NO_SENSE, SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION, 0x00);
}

//! This fonction send the UFI status "lun not present"
//!
void sbc_lun_status_is_not_present(void)
{
   Sbc_send_failed();
   Sbc_build_sense(SBC_SENSE_KEY_NOT_READY, SBC_ASC_MEDIUM_NOT_PRESENT, 0x00);
}

//! This fonction send the UFI status busy and change
//!
void sbc_lun_status_is_busy_or_change(void)
{
   Sbc_send_failed();
   Sbc_build_sense(SBC_SENSE_KEY_UNIT_ATTENTION, SBC_ASC_NOT_READY_TO_READY_CHANGE, 0x00 );
}

//! This fonction send the UFI status FAIL
//!
void sbc_lun_status_is_fail(void)
{
   Sbc_send_failed();
   Sbc_build_sense(SBC_SENSE_KEY_HARDWARE_ERROR, SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION, 0x00);
}

//! This fonction send the UFI status FAIL because write protection
//!
void sbc_lun_status_is_protected(void)
{
   Sbc_send_failed();
   Sbc_build_sense(SBC_SENSE_KEY_DATA_PROTECT, SBC_ASC_WRITE_PROTECTED, 0x00);
}

/** @} */
