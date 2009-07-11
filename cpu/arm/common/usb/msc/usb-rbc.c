#include <usb-msc-bulk.h>
#include <usb-api.h>
#include <usb-core.h>
#include <sys/process.h>
#include <stdio.h>
#include <rbc_const.h>
#include <rbc_struct.h>
#include <string.h>

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef USB_RBC_NUM_BLOCKS
#define USB_RBC_NUM_BLOCKS 32
#endif

static struct spc2_sense_data sense_data =
  {
    SCSI_SENSE_CURRENT_ERROR,
    0,
    0,
    {0},
    (sizeof(struct spc2_sense_data)
     - offsetof(struct spc2_sense_data, command_specific))
  };

static void
scsi_error(unsigned int sense_key, unsigned int asc, int32_t info)
{
  sense_data.response_code = SCSI_SENSE_INFORMATION_VALID | SCSI_SENSE_CURRENT_ERROR;
  sense_data.information[0] = (info >> 24) & 0xff;
  sense_data.information[1] = (info >> 16) & 0xff;
  sense_data.information[2] = (info >> 8) & 0xff;
  sense_data.information[3] = info & 0xff;
  sense_data.sense_key = sense_key;
  sense_data.asc = (asc >> 8) & 0xff;
  sense_data.ascq = asc & 0xff;
}

static void
scsi_ok()
{
  sense_data.response_code = SCSI_SENSE_CURRENT_ERROR;
  sense_data.sense_key = SCSI_SENSE_KEY_NO_SENSE;
  sense_data.asc = 0x00;
  sense_data.ascq = 0x00;
};

static const struct spc2_std_inquiry_data std_inquiry_data =
  {
    SCSI_STD_INQUIRY_CONNECTED | SCSI_STD_INQUIRY_TYPE_RBC,
    0,
    SCSI_STD_INQUIRY_VERSION_SPC2,
    0,
    (sizeof(struct spc2_std_inquiry_data)
     - offsetof(struct spc2_std_inquiry_data, flags3)),
    0,
    0,
    0,
    {'F','l','u','f','w','a','r','e'},
    {'T','e','s','t',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
    {'0','.','1',' '}
  };

#define UNIT_NAME {'F','l','u','f','f','w','a','r','e',' ', \
 'P','s','e','u','d','o',' ','D','i','s','k'}
#define UNIT_NAME_LENGTH 21

static const struct 
{
  struct spc2_vital_product_data_head head;
  struct {
    struct spc2_vital_product_data_head head;
    char unit_name[UNIT_NAME_LENGTH];
  } descriptor;
} CC_BYTE_ALIGNED device_identification_data =
  {
    {
      SCSI_STD_INQUIRY_CONNECTED | SCSI_STD_INQUIRY_TYPE_RBC,
      SCSI_PAGE_DEVICE_IDENTIFICATION,
      0,
      sizeof(device_identification_data.descriptor),
    },
    {
      {
	SCSI_CODE_SET_ACSII,
	SCSI_IDENTIFIER_TYPE_NON_UNIQUE,
	0,
	sizeof(device_identification_data.descriptor.unit_name)
      },
      UNIT_NAME
    }
  };
      

static const struct 
{
  struct spc2_vital_product_data_head head;
  uint8_t supported[3];
} CC_BYTE_ALIGNED supported_pages_data =
  {
    {
      SCSI_STD_INQUIRY_CONNECTED | SCSI_STD_INQUIRY_TYPE_RBC,
      SCSI_PAGE_SUPPORTED_PAGES,
      0,
      sizeof(supported_pages_data.supported),
    },
    {SCSI_PAGE_SUPPORTED_PAGES, SCSI_PAGE_UNIT_SERIAL_NUMBER,
     SCSI_PAGE_DEVICE_IDENTIFICATION}
  };

static const struct 
{
  struct spc2_vital_product_data_head head;
  uint8_t serial_number[8];
} CC_BYTE_ALIGNED unit_serial_number_data = {
  {
      SCSI_STD_INQUIRY_CONNECTED | SCSI_STD_INQUIRY_TYPE_RBC,
      SCSI_PAGE_SUPPORTED_PAGES,
      0,
      sizeof(unit_serial_number_data.serial_number)
  },
  {'1','2','3','4','5','6','7','8'}
};

static usb_msc_handler_status
handle_inquiry_cmd(struct usb_msc_command_state *state)
{
  struct spc2_inquiry_cmd *cmd = (struct spc2_inquiry_cmd*)state->command;
  if (cmd->flags & SCSI_INQUIRY_FLAG_CMDDT) {
    scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
	       cmd->allocation_length);
    return USB_MSC_HANDLER_FAILED;
  }
  if (cmd->flags & SCSI_INQUIRY_FLAG_EVPD) {
    PRINTF("Requested page %02x\n", cmd->page);
    switch (cmd->page) {
    case SCSI_PAGE_SUPPORTED_PAGES:
      usb_msc_send_data((uint8_t *)&supported_pages_data,
			sizeof(supported_pages_data),
			USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
      break;
    case SCSI_PAGE_DEVICE_IDENTIFICATION:
      usb_msc_send_data((uint8_t *)&device_identification_data,
			sizeof(device_identification_data),
			USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
      break;
    case SCSI_PAGE_UNIT_SERIAL_NUMBER:
      usb_msc_send_data((uint8_t *)&unit_serial_number_data,
			sizeof(unit_serial_number_data),
			USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
      break;
    default:
      scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
		 cmd->allocation_length);
      return USB_MSC_HANDLER_FAILED;
    }
    return USB_MSC_HANDLER_OK;
  } else {
    if (cmd->page != 0) {
      scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
		 cmd->allocation_length);
      return USB_MSC_HANDLER_FAILED;
    }
    usb_msc_send_data((uint8_t *)&std_inquiry_data,
			sizeof(std_inquiry_data),
			USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
  }
  return USB_MSC_HANDLER_OK;
}

static usb_msc_handler_status
handle_request_sense_cmd(struct usb_msc_command_state *state)
{
  usb_msc_send_data((uint8_t *)&sense_data,
		    sizeof(sense_data),
		    USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
  return USB_MSC_HANDLER_OK;
}

static usb_msc_handler_status
handle_test_unit_ready_cmd(struct usb_msc_command_state *state)
{
  scsi_ok();
  return USB_MSC_HANDLER_OK;
}

static const struct rbc_read_capacity_data read_capacity_data =
  {
    HOST32_TO_BE_BYTES(USB_RBC_NUM_BLOCKS-1),
    HOST32_TO_BE_BYTES(512)
  };
  
static usb_msc_handler_status
handle_read_capacity(struct usb_msc_command_state *state)
{
  usb_msc_send_data((uint8_t *)&read_capacity_data,
		    sizeof(read_capacity_data),
		    USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
  return USB_MSC_HANDLER_OK;
}

static const struct mode_sense_data {
  struct spc2_mode_parameter_header_6 header;
  struct rbc_device_parameters_page page;
} CC_BYTE_ALIGNED mode_sense_data =
  {
    {
      (sizeof(mode_sense_data)
       - offsetof(struct mode_sense_data, header.medium_type)),
      0,0,0
    },
    {
      {SCSI_MODE_RBC_DEVICE_PAGE | SCSI_MODE_PAGE_SP,
      sizeof(mode_sense_data) - offsetof(struct mode_sense_data, page.flags1)},
      SCSI_MODE_WCD,
      HOST16_TO_BE_BYTES(512),
      HOST40_TO_BE_BYTES((long long)USB_RBC_NUM_BLOCKS),
      0x80,
      (SCSI_MODE_FORMATD | SCSI_MODE_LOCKD),
      0
    }
  };

static usb_msc_handler_status
handle_mode_sense(struct usb_msc_command_state *state)
{
  struct spc2_mode_sence_6_cmd *cmd =
    (struct spc2_mode_sence_6_cmd*)state->command;
  PRINTF("%ld - %ld - %ld\n", sizeof(struct mode_sense_data), offsetof(struct mode_sense_data, page.flags1),offsetof(struct mode_sense_data, page.reserved));
  switch(cmd->page_code) {
  case SCSI_MODE_RBC_DEVICE_PAGE:
  case SCSI_MODE_SENSE_ALL_PAGES:
    usb_msc_send_data((uint8_t *)&mode_sense_data,
		      sizeof(mode_sense_data),
		      USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
    break;
  default:
    scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
	       cmd->allocation_length);
    return USB_MSC_HANDLER_FAILED;
  }
  return USB_MSC_HANDLER_OK;
}

static usb_msc_handler_status
handle_mode_select(struct usb_msc_command_state *state)
{
  /* Can't change anything */
  return USB_MSC_HANDLER_OK;
}
static uint8_t disk_blocks[USB_RBC_NUM_BLOCKS][512];

static usb_msc_handler_status
handle_read(struct usb_msc_command_state *state)
{
  struct rbc_read_cmd *cmd = (struct rbc_read_cmd*)state->command;
  unsigned long lba = be32_to_host(cmd->logical_block_address);
  unsigned long blocks = be16_to_host(cmd->transfer_length);
  PRINTF("Requested %ld blocks at %ld\n", blocks, lba);
  if (lba >= USB_RBC_NUM_BLOCKS || lba + blocks > USB_RBC_NUM_BLOCKS) {
    scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
	       blocks);
    return USB_MSC_HANDLER_FAILED;
  }
  usb_msc_send_data((uint8_t *)&disk_blocks[lba], blocks * 512,
		    USB_MSC_DATA_SEND | USB_MSC_DATA_LAST);
  scsi_ok();
  return USB_MSC_HANDLER_OK;
}

static void
handle_write_done(struct usb_msc_command_state *state)
{
  PRINTF("Wrote data\n");
  state->status = MASS_BULK_CSW_STATUS_PASSED;
  scsi_ok();
}

static usb_msc_handler_status
handle_write(struct usb_msc_command_state *state)
{
  struct rbc_write_cmd *cmd = (struct rbc_write_cmd*)state->command;
  unsigned long lba = be32_to_host(cmd->logical_block_address);
  unsigned long blocks = be16_to_host(cmd->transfer_length);
  if (lba >= USB_RBC_NUM_BLOCKS || lba + blocks > USB_RBC_NUM_BLOCKS) {
    scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,SCSI_ASC_INVALID_FIELD_IN_CDB,
	       blocks);
    return USB_MSC_HANDLER_FAILED;
  }
  PRINTF("Writing %ld blocks at %ld\n", blocks, lba);
  usb_msc_receive_data(disk_blocks[lba], blocks * 512,
		       USB_MSC_DATA_RECEIVE | USB_MSC_DATA_LAST
		       | USB_MSC_DATA_DO_CALLBACK);
  state->data_cb = handle_write_done;
  return USB_MSC_HANDLER_DELAYED;
}

static usb_msc_handler_status
handle_start_stop_unit(struct usb_msc_command_state *state)
{
  scsi_ok();
  return USB_MSC_HANDLER_OK;
}

static usb_msc_handler_status
handle_verify(struct usb_msc_command_state *state)
{
  scsi_ok();
  return USB_MSC_HANDLER_OK;
}

usb_msc_handler_status
usb_msc_handle_command(struct usb_msc_command_state *state)
{

  usb_msc_handler_status ret;
  PRINTF("Got CBW %02x\n", state->command[0]);
  switch(state->command[0]) {
  case SCSI_CMD_INQUIRY:
    ret = handle_inquiry_cmd(state);
    break;
  case SCSI_CMD_REQUEST_SENSE:
    ret = handle_request_sense_cmd(state);
    break;
  case SCSI_CMD_TEST_UNIT_READY:
    ret = handle_test_unit_ready_cmd(state);
    break;
  case SCSI_CMD_READ_CAPACITY:
    ret = handle_read_capacity(state);
    break;
  case SCSI_CMD_MODE_SENSE_6:
    ret = handle_mode_sense(state);
    break;
  case SCSI_CMD_MODE_SELECT_6:
    ret = handle_mode_select(state);
    break;
  case SCSI_CMD_READ_10:
    ret = handle_read(state);
    break;
  case SCSI_CMD_WRITE_10:
    ret = handle_write(state);
    break;
  case SCSI_CMD_VERIFY_10:
    ret = handle_verify(state);
    break;
  case SCSI_CMD_START_STOP_UNIT:
    ret = handle_start_stop_unit(state);
    break;
  default:
    printf("Unhandled request: %02x\n", state->command[0]);
    scsi_error(SCSI_SENSE_KEY_ILLEGAL_REQUEST,
	       SCSI_ASC_INVALID_COMMAND_OPERATION,0);
    return USB_MSC_HANDLER_FAILED;
  }
  return ret;
}

void
usb_msc_command_handler_init()
{
}
