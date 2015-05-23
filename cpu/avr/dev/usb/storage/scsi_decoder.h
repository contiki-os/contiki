/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file scsi_decoder.h *******************************************************
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
#ifndef SCSI_DECODER_H_
#define SCSI_DECODER_H_

//_____ I N C L U D E S ____________________________________________________


//_____ M A C R O S ________________________________________________________

typedef struct
{
  U8      key;
  U8      asc;
  U8      ascq;
} s_scsi_sense;


//_____ D E C L A R A T I O N S ____________________________________________

Bool scsi_decode_command  (void);

/****************************************************************************/
/* Command for all SCSI device types                                        */
/****************************************************************************/

#define SBC_CMD_TEST_UNIT_READY                   (0x00)
#define SBC_CMD_REQUEST_SENSE                     (0x03)
#define SBC_CMD_FORMAT_UNIT                       (0x04)
#define SBC_CMD_READ_6                            (0x08)
#define SBC_CMD_INQUIRY                           (0x12)
#define SBC_CMD_MODE_SELECT_6                     (0x15)
#define SBC_CMD_MODE_SENSE_6                      (0x1A)
#define SBC_CMD_START_STOP_UNIT                   (0x1B)
#define SBC_CMD_RECEIVE_DIAGNOSTICS               (0x1C)
#define SBC_CMD_SEND_DIAGNOSTIC                   (0x1D)
#define SBC_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL      (0x1E)
#define SBC_CMD_READ_LONG                         (0x23)
#define SBC_CMD_READ_CAPACITY                     (0x25)
#define SBC_CMD_READ_CD_ROM_CAPACITY              (0x25)
#define SBC_CMD_READ_10                           (0x28)
#define SBC_CMD_WRITE_10                          (0x2A)
#define SBC_CMD_VERIFY_10                         (0x2F)
#define SBC_CMD_SYNCHRONIZE_CACHE                 (0x35)
#define SBC_CMD_WRITE_BUFFER                      (0x3B)
#define SBC_CMD_CHANGE_DEFINITION                 (0x40)
#define SBC_CMD_READ_TOC                          (0x43)
#define SBC_CMD_MODE_SELECT_10                    (0x55)
#define SBC_CMD_RESERVE_10                        (0x56)
#define SBC_CMD_RELEASE_10                        (0x57)
#define SBC_CMD_MODE_SENSE_10                     (0x5A)

#define SBC_CONTROL_BYTE                          (0x00)
#define SBC_CMD_DIR_IN                            (0x80)
#define SBC_CMD_DIR_OUT                           (0x00)


/****************************************************************************/
/* Sense Key Code                                                           */
/****************************************************************************/
#define SBC_SENSE_KEY_NO_SENSE                        (0x00)
#define SBC_SENSE_KEY_RECOVERED_ERROR                 (0x01)
#define SBC_SENSE_KEY_NOT_READY                       (0x02)
#define SBC_SENSE_KEY_MEDIUM_ERROR                    (0x03)
#define SBC_SENSE_KEY_HARDWARE_ERROR                  (0x04)
#define SBC_SENSE_KEY_ILLEGAL_REQUEST                 (0x05)
#define SBC_SENSE_KEY_UNIT_ATTENTION                  (0x06)
#define SBC_SENSE_KEY_DATA_PROTECT                    (0x07)
#define SBC_SENSE_KEY_BLANK_CHECK                     (0x08)
#define SBC_SENSE_KEY_VENDOR_SPECIFIC                 (0x09)
#define SBC_SENSE_KEY_COPY_ABORTED                    (0x0A)
#define SBC_SENSE_KEY_ABORTED_COMMAND                 (0x0B)
#define SBC_SENSE_KEY_VOLUME_OVERFLOW                 (0x0D)
#define SBC_SENSE_KEY_MISCOMPARE                      (0x0E)

/****************************************************************************/
/* ASC code assignments                                                     */
/****************************************************************************/
#define SBC_ASC_NO_ADDITIONAL_SENSE_INFORMATION       (0x00)
#define SBC_ASC_LOGICAL_UNIT_NOT_READY                (0x04)
#define SBC_ASC_INVALID_FIELD_IN_CDB                  (0x24)
#define SBC_ASC_WRITE_PROTECTED                       (0x27)
#define SBC_ASC_FORMAT_ERROR                          (0x31)
#define SBC_ASC_INVALID_COMMAND_OPERATION_CODE        (0x20)
#define SBC_ASC_NOT_READY_TO_READY_CHANGE             (0x28)
#define SBC_ASC_MEDIUM_NOT_PRESENT                    (0x3A)

/****************************************************************************/
/* ASCQ code assignments                                                    */
/****************************************************************************/
#define SBC_ASCQ_FORMAT_COMMAND_FAILED                (0x01)
#define SBC_ASCQ_INITIALIZING_COMMAND_REQUIRED        (0x02)
#define SBC_ASCQ_OPERATION_IN_PROGRESS                (0x07)


/****************************************************************************/
/* REQUEST SENSE PARAMETERS                                                 */
/****************************************************************************/
#define SBC_RESPONSE_CODE_SENSE                       (0x70)
#define SBC_ADDITIONAL_SENSE_LENGTH                   (0x0A)
#define SBC_COMMAND_SPECIFIC_INFORMATION_3            (0x00)
#define SBC_COMMAND_SPECIFIC_INFORMATION_2            (0x00)
#define SBC_COMMAND_SPECIFIC_INFORMATION_1            (0x00)
#define SBC_COMMAND_SPECIFIC_INFORMATION_0            (0x00)
#define SBC_FIELD_REPLACEABLE_UNIT_CODE               (0x00)
#define SBC_SENSE_KEY_SPECIFIC_2                      (0x00)
#define SBC_SENSE_KEY_SPECIFIC_1                      (0x00)
#define SBC_SENSE_KEY_SPECIFIC_0                      (0x00)

/******* number of bytes of READ CAPACITY response *********/
#define SBC_READ_CAPACITY_LENGTH                      (0x08)


/****************************************************************************/
/*MODE SENSE and REQUEST SENSE DEFINITIONS                                  */
/****************************************************************************/

/*************** Direct access medium type ****************/
#define SBC_DEFAULT_MEDIUM_TYPE                       (0x00)
#define SBC_FLEXIBLE_DISK_SINGLE_SIDED_UNSPECIFIED    (0x01)
#define SBC_FLEXIBLE_DISK_DOUBLE_SIDED_UNSPECIFIED    (0x02)

#define SBC_MEDIUM_TYPE                               SBC_DEFAULT_MEDIUM_TYPE

#define SBC_DEV_SPEC_PARAM_WR_ENABLE                  (0x00)
#define SBC_DEV_SPEC_PARAM_WR_PROTECT                 (0x80)
#define SBC_BLOCK_DESCRIPTOR_LENGTH                   (0x00)

#define SBC_MSK_DBD                                   (0x08)
#define SBC_MSK_PAGE_CONTROL                          (0xC0)
#define SBC_MSK_PAGE_CODE                             (0x3F)


/************ General Page Code paramaters *****************/
#define SBC_PAGE_CODE_READ_WRITE_ERROR_RECOVERY       (0x01)
#define SBC_PAGE_CODE_FORMAT_DEVICE                   (0x03)
#define SBC_PAGE_CODE_FLEXIBLE_DISK                   (0x05)
#define SBC_PAGE_CODE_INFORMATIONAL_EXCEPTIONS        (0x1C)
#define SBC_PAGE_CODE_ALL                             (0x3F)


#define SBC_PAGE_LENGTH_INFORMATIONAL_EXCEPTIONS      (0x0A)
#define SBC_PAGE_LENGTH_READ_WRITE_ERROR_RECOVERY     (0x0A)
#define SBC_PAGE_LENGTH_FLEXIBLE_DISK                 (0x1E)
#define SBC_PAGE_LENGTH_FORMAT_DEVICE                 (0x16)



#define SBC_MODE_DATA_LENGTH_INFORMATIONAL_EXCEPTIONS   (SBC_PAGE_LENGTH_INFORMATIONAL_EXCEPTIONS + 2 + 3)
#define SBC_MODE_DATA_LENGTH_READ_WRITE_ERROR_RECOVERY  (SBC_PAGE_LENGTH_READ_WRITE_ERROR_RECOVERY + 2 + 3 )
#define SBC_MODE_DATA_LENGTH_FLEXIBLE_DISK              (SBC_PAGE_LENGTH_FLEXIBLE_DISK + 2 + 3 )
#define SBC_MODE_DATA_LENGTH_FORMAT_DEVICE              (SBC_PAGE_LENGTH_FORMAT_DEVICE + 2 + 3 )
/*SBC_PAGE_LENGTH_FLEXIBLE_DISK + 2  + \*/
#define SBC_MODE_DATA_LENGTH_CODE_ALL                 (SBC_PAGE_LENGTH_READ_WRITE_ERROR_RECOVERY + 2 + \
                                                       SBC_PAGE_LENGTH_INFORMATIONAL_EXCEPTIONS + 2 + \
                                                       SBC_BLOCK_DESCRIPTOR_LENGTH + \
                                                       + 3 )

/*                                                       SBC_MODE_DATA_LENGTH_FORMAT_DEVICE + 2 + \*/
/****** Information exceptions control page parameters *****/
#define SBC_MRIE                                      (0x05)

/*************** Format device page parameters *************/
#define SBC_TRACK_PER_ZONE_MSB                        (0x00)
#define SBC_TRACK_PER_ZONE_LSB                        (0x00)
#define SBC_ALTERNATE_SECTORS_PER_ZONE_MSB            (0x00)
#define SBC_ALTERNATE_SECTORS_PER_ZONE_LSB            (0x00)
#define SBC_ALTERNATE_TRACK_PER_ZONE_MSB              (0x00)
#define SBC_ALTERNATE_TRACK_PER_ZONE_LSB              (0x00)
#define SBC_ALTERNATE_TRACK_PER_LU_MSB                (0x00)
#define SBC_ALTERNATE_TRACK_PER_LU_LSB                (0x00)

/************* Flexible Disk page Parameters ***************/
#define SBC_TRANSFER_RATE_MSB                         (0x13)
#define SBC_TRANSFER_RATE_LSB                         (0x88)
/* 1388h    5 Mbit/s   */
/* 07D0h    2 Mbit/s   */
/* 03E8h    1 Mbit/s   */
/* 01F4h    500 kbit/s */
/* 012Ch    300 kbit/s */
/* 00FAh    250 kbit/s */

#define SBC_NUMBER_OF_HEAD                            (0x04)
#define SBC_SECTOR_PER_TRACK                          (0x20)
#define SBC_DATA_BYTE_PER_SECTOR_MSB                  (0x02)
#define SBC_DATA_BYTE_PER_SECTOR_LSB                  (0x00)
#define SBC_NUMBER_OF_CYLINDERS_MSB                   (0x01)
#define SBC_NUMBER_OF_CYLINDERS_LSB                   (0xE9)
#define SBC_STARTING_CYLINDER_WRITE_COMPENSATION_MSB  (0x00)
#define SBC_STARTING_CYLINDER_WRITE_COMPENSATION_LSB  (0x00)
#define SBC_STARTING_CYLINDER_REDUCED_WRITE_MSB       (0x00)
#define SBC_STARTING_CYLINDER_REDUCED_WRITE_LSB       (0x00)
#define SBC_DEVICE_STEP_RATE_MSB                      (0x00)
#define SBC_DEVICE_STEP_RATE_LSB                      (0x00)
#define SBC_DEVICE_STEP_PULSE_WIDTH                   (0x00)
#define SBC_HEAD_SETTLE_DELAY_MSB                     (0x00)
#define SBC_HEAD_SETTLE_DELAY_LSB                     (0x00)
#define SBC_MOTOR_ON_DELAY                            (0x00)
#define SBC_MOTOR_OFF_DELAY                           (0x00)
#define SBC_STEP_PULSE_PER_CYLINDER                   (0x00)
#define SBC_WRITE_COMPENSATION                        (0x00)
#define SBC_HEAD_LOAD_DELAY                           (0x00)
#define SBC_HEAD_UNLOAD_DELAY                         (0x00)
#define SBC_PIN34_PIN2                                (0x00)
#define SBC_PIN4_PIN1                                 (0x00)
#define SBC_MEDIUM_ROTATION_RATE_MSB                  (0x00)
#define SBC_MEDIUM_ROTATION_RATE_LSB                  (0x00)

/************ Read/Write Error Recovery parameters**********/
#define SBC_READ_RETRY_COUNT                          (0x03)
#define SBC_WRITE_RETRY_COUNT                         (0x80)
#define SBC_CORRECTION_SPAN                           (0x00)
#define SBC_HEAD_OFFSET_COUNT                         (0x00)
#define SBC_DATA_STROBE_OFFSET                        (0x00)
#define SBC_RECOVERY_LIMIT_MSB                        (0x00)
#define SBC_RECOVERY_LIMIT_LSB                        (0x00)


/*_____ D E F I N I T I O N ________________________________________________*/

#define SBC_MAX_INQUIRY_DATA                          (0x60) // value ?

struct sbc_st_std_inquiry_data
{
  Byte    DeviceType : 5;
  Byte    PeripheralQualifier : 3;

  Byte    Reserved1 : 7;
  Byte    RemovableMedia : 1;

  Byte    Version;

  Byte    Reserved3 : 5;
  Byte    NormACA : 1;
  Byte    Obsolete0 : 1;
  Byte    AERC : 1;

  Byte    Reserved4[3];

  Byte    SoftReset : 1;
  Byte    CommandQueue : 1;
  Byte    Reserved5 : 1;
  Byte    LinkedCommands : 1;
  Byte    Synchronous : 1;
  Byte    Wide16Bit : 1;
  Byte    Wide32Bit : 1;
  Byte    RelativeAddressing : 1;
};


//_____ D E C L A R A T I O N S ____________________________________________

#define Sbc_send_failed()                 (g_scsi_status = COMMAND_FAILED)
#define Sbc_send_check_condition()        (g_scsi_status = CHECK_CONDITION)
#define Sbc_send_good()                   (g_scsi_status = COMMAND_PASSED)

/*#define Sbc_valid_write_usb(length)       ( Usb_set_TXRDY(), \
                                            dCBWDataTransferLength -= length, \
                                            while (!Usb_tx_complete()),\
                                            Usb_clear_TXCMPL() )
*/
#define Sbc_build_sense(skey, sasc, sascq)   ( g_scsi_sense.key  = skey, \
                                               g_scsi_sense.asc  = sasc, \
                                               g_scsi_sense.ascq = sascq )

#define Sbc_valid_write_usb(length)          ( Usb_send_in(), \
                                               g_scsi_data_remaining -= length )


#define COMMAND_PASSED                0x00
#define COMMAND_FAILED                0x01
#define CHECK_CONDITION               0x02
#define PHASE_ERROR                   0x02


Bool  sbc_inquiry (void);
Bool  sbc_mode_sense( Bool sense_10 );
Bool  sbc_mode_select_6 (void);
Bool  sbc_request_sense (void);
Bool  sbc_format (void);
Bool  sbc_write_10 (void);
Bool  sbc_read_10  (void);
Bool  sbc_test_unit_ready(void);
Bool  sbc_read_capacity(void);
Bool  sbc_prevent_allow_medium_removal(void);

void  sbc_lun_status_is_good(void);
void  sbc_lun_status_is_busy_or_change(void);
void  sbc_lun_status_is_not_present(void);
void  sbc_lun_status_is_fail(void);
void  sbc_lun_status_is_protected(void);

#endif /*SCSI_DECODER_H_ */
/** @} */
