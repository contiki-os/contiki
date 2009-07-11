#include <stdint.h>
#include <scsi_sense.h>
#include <scsi_command.h>

#ifndef CC_BYTE_ALIGNED
#ifdef __GNUC__
#define CC_BYTE_ALIGNED __attribute__ ((packed))
#endif
#endif

#ifndef CC_BYTE_ALIGNED
#define CC_BYTE_ALIGNED
#endif

#define HOST16_TO_BE_BYTES(x) {(((x) >> 8) & 0xff), ((x) & 0xff)}
#define HOST24_TO_BE_BYTES(x) {(((x) >> 16) & 0xff), (((x) >> 8) & 0xff), \
 ((x) & 0xff)}
#define HOST32_TO_BE_BYTES(x) {(((x) >> 24) & 0xff), (((x) >> 16) & 0xff), \
 (((x) >> 8) & 0xff), ((x) & 0xff)}
#define HOST40_TO_BE_BYTES(x) {(((x) >> 32) & 0xff), (((x) >> 24) & 0xff), (((x) >> 16) & 0xff), \
 (((x) >> 8) & 0xff), ((x) & 0xff)}

typedef uint8_t uint40_bytes[5];
typedef uint8_t uint32_bytes[4];
typedef uint8_t int24_bytes[3];
typedef uint8_t uint24_bytes[3];
typedef uint8_t uint16_bytes[2];

inline unsigned long
be16_to_host(uint16_bytes bytes)
{
  return bytes[0] << 8 | bytes[1];
}

inline unsigned long
be24_to_host(uint24_bytes bytes)
{
  return bytes[0] << 16 | bytes[1] << 8 | bytes[2];
}

inline long
signed_be24_to_host(int24_bytes bytes)
{
  return ((bytes[0] << 16 | bytes[1] << 8 | bytes[2]) ^ 0x800000) - 0x800000;
}


inline unsigned long
be32_to_host(uint32_bytes bytes)
{
  return (be16_to_host(bytes) << 16) | be16_to_host(bytes + 2);
}


#define BE16_TO_HOST(bytes) ((bytes)[0] << 8 | (bytes)[1])
#define BE32_TO_HOST(bytes) \
((BE16_TO_HOST(bytes) << 16) | BE16_TO_HOST((bytes)+2))

/* Flag field of INQUIRY command */
#define SCSI_INQUIRY_FLAG_CMDDT 0x02
#define SCSI_INQUIRY_FLAG_EVPD 0x01
#define SCSI_INQUIRY_FLAG_LUN(a) ((a)<<5)

struct scsi_inquiry_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t page;
  uint8_t reserved;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

/* Constant for the standard inquiry data */
#define SCSI_STD_INQUIRY_CONNECTED 0x00
#define SCSI_STD_INQUIRY_NOT_CONNECTED 0x20

#define SCSI_STD_INQUIRY_VERSION_NONE 0x00
#define SCSI_STD_INQUIRY_VERSION_SCSI1 0x01
#define SCSI_STD_INQUIRY_VERSION_SCSI2 0x02
#define SCSI_STD_INQUIRY_VERSION_SPC2 0x04

#define SCSI_STD_INQUIRY_RESPONSE_DATA_FORMAT_SCSI1 0x00
#define SCSI_STD_INQUIRY_RESPONSE_DATA_FORMAT_SCSI2 0x02

#define SCSI_STD_INQUIRY_TYPE_DIRECT_ACCESS 0x00
#define SCSI_STD_INQUIRY_TYPE_SEQUENTIAL 0x01
#define SCSI_STD_INQUIRY_TYPE_PRINTER 0x02
#define SCSI_STD_INQUIRY_TYPE_PROCESSOR 0x03
#define SCSI_STD_INQUIRY_TYPE_WRITE_ONCE 0x04
#define SCSI_STD_INQUIRY_TYPE_CD_ROM 0x05
#define SCSI_STD_INQUIRY_TYPE_SCANNER 0x06
#define SCSI_STD_INQUIRY_TYPE_OPTICAL 0x07
#define SCSI_STD_INQUIRY_TYPE_CHANGER 0x08
#define SCSI_STD_INQUIRY_TYPE_COMM 0x09
#define SCSI_STD_INQUIRY_TYPE_RAID 0x0C
#define SCSI_STD_INQUIRY_TYPE_RBC 0x0E

#define SCSI_STD_INQUIRY_FLAG1_RMB 0x80

#define SCSI_STD_INQUIRY_FLAG2_AERC 0x80
#define SCSI_STD_INQUIRY_FLAG2_AENC 0x80
#define SCSI_STD_INQUIRY_FLAG2_NORMACA 0x20
#define SCSI_STD_INQUIRY_FLAG2_HISUP 0x10
#define SCSI_STD_INQUIRY_FLAG2_RESPONSE_FORMAT 0x02

#define SCSI_STD_INQUIRY_FLAG3_SCCS 0x80

#define SCSI_STD_INQUIRY_FLAG4_BQUE 0x80
#define SCSI_STD_INQUIRY_FLAG4_ENCSERV 0x40
#define SCSI_STD_INQUIRY_FLAG4_VS 0x20
#define SCSI_STD_INQUIRY_FLAG4_MULTIP 0x10
#define SCSI_STD_INQUIRY_FLAG4_MCHNGR 0x08
#define SCSI_STD_INQUIRY_FLAG4_ADDR16 0x01

#define SCSI_STD_INQUIRY_FLAG5_RELADR 0x80
#define SCSI_STD_INQUIRY_FLAG5_WBUS 0x20
#define SCSI_STD_INQUIRY_FLAG5_SYNC 0x10
#define SCSI_STD_INQUIRY_FLAG5_LINKED 0x08
#define SCSI_STD_INQUIRY_FLAG5_CMDQUE 0x02

struct scsi_std_inquiry_data
{
  uint8_t device;
  uint8_t flags1;
  uint8_t version;
  uint8_t flags2;
  uint8_t additional_length;
  uint8_t flags3;
  uint8_t flags4;
  uint8_t flags5;
  char vendor_id[8];
  char product_id[16];
  char product_rev[4];
} CC_BYTE_ALIGNED;

struct scsi_vital_product_data_head
{
  uint8_t device;
  uint8_t page;
  uint8_t reserved;
  uint8_t page_length;
} CC_BYTE_ALIGNED;

struct scsi_identification_descriptor
{
  uint8_t code_set;
  uint8_t identifier_type;
  uint8_t reserved;
  uint8_t identifier_length;
};

struct scsi_request_sense_cmd
{
  uint8_t op_code;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

#define SCSI_SENSE_CURRENT_ERROR 0x70
#define SCSI_SENSE_DEFERRED_ERROR 0x71
#define SCSI_SENSE_INFORMATION_VALID 0x80
#define SCSI_SENSE_FILEMARK 0x80
#define SCSI_SENSE_EOM 0x40
#define SCSI_SENSE_ILI 0x20

struct scsi_sense_data
{
  uint8_t response_code;
  uint8_t obsolete;
  uint8_t sense_key;
  uint8_t information[4];
  uint8_t additional_length;
  uint8_t command_specific[4];
  uint8_t asc;
  uint8_t ascq;
  uint8_t unit_code;
  uint8_t sense_key_specific[3];
} CC_BYTE_ALIGNED;


/* Flag field of INQUIRY command */
#define SCSI_MODE_SENSE_FLAG_DBD 0x08
#define SCSI_MODE_SENSE_FLAG_LUN(a) ((a)<<5)

#define SCSI_MODE_SENSE_PC_CURRENT 0x00
#define SCSI_MODE_SENSE_PC_CHANGEABLE 0x40
#define SCSI_MODE_SENSE_PC_DEFAULT 0x80
#define SCSI_MODE_SENSE_PC_SAVED 0xc0

struct scsi_mode_sence_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t page_code;
  uint8_t reserved;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct scsi_mode_select_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t parameter_list_length;
  uint8_t control;
};

#define SCSI_MODE_PARAM_WP 0x80
#define SCSI_MODE_PARAM_BUFFERED_MODE_SYNC 0x00
#define SCSI_MODE_PARAM_BUFFERED_MODE_ASYNC 0x10
#define SCSI_MODE_PARAM_BUFFERED_MODE_ALL_SYNC 0x10
#define SCSI_MODE_PARAM_SPEED_DEFAULT 0x00
#define SCSI_MODE_PARAM_SPEED_LOWEST 0x01
#define SCSI_MODE_PARAM_SPEED_HIGHEST 0x0f

struct scsi_mode_parameter_header_6
{
  uint8_t mode_data_length;
  uint8_t medium_type;
  uint8_t device_specific_parameter;
  uint8_t block_descriptor_length;
} CC_BYTE_ALIGNED;

#define SCSI_DENSITY_9_800 0x01
#define SCSI_DENSITY_9_1600 0x02
#define SCSI_DENSITY_9_6250 0x03
#define SCSI_DENSITY_4_9_8000 0x05
#define SCSI_DENSITY_9_3200 0x06
#define SCSI_DENSITY_4_6400 0x07
#define SCSI_DENSITY_4_8000 0x08
#define SCSI_DENSITY_18_37871 0x09
#define SCSI_DENSITY_22_6667 0x0a
#define SCSI_DENSITY_4_1600 0x0b
#define SCSI_DENSITY_24_12690 0x0c
#define SCSI_DENSITY_24_25380 0xd
#define SCSI_DENSITY_15_10000 0x0f
#define SCSI_DENSITY_18_10000 0x10
#define SCSI_DENSITY_26_16000 0x11
#define SCSI_DENSITY_30_51667 0x12
#define SCSI_DENSITY_1_2400 0x13
#define SCSI_DENSITY_1_43245 0x14
#define SCSI_DENSITY_1_45434 0x15
#define SCSI_DENSITY_48_10000 0x16
#define SCSI_DENSITY_48_42500 0x17


struct scsi_mode_parameter_block_descriptor
{
  uint8_t density_code;
  uint24_bytes number_of_blocks;
  uint8_t reserved;
  uint24_bytes block_length;
} CC_BYTE_ALIGNED;


#define SCSI_MODE_PAGE_PS 0x80
#define SCSI_MODE_PAGE_CONTROL_MODE 0x0a
#define SCSI_MODE_PAGE_DEVICE_CONFIGURATION 0x10
#define SCSI_MODE_PAGE_CONNECT 0x02
#define SCSI_MODE_PAGE_MEDIUM_PARTITION_1 0x11
#define SCSI_MODE_PAGE_MEDIUM_PARTITION_2 0x12
#define SCSI_MODE_PAGE_MEDIUM_PARTITION_3 0x13
#define SCSI_MODE_PAGE_MEDIUM_PARTITION_4 0x14
#define SCSI_MODE_PAGE_PERIPHERIAL_DEVICE 0x09
#define SCSI_MODE_PAGE_RW_ERROR_RECOVERY 0x01
#define SCSI_MODE_PAGE_VENDOR_SPECIFIC 0x00
#define SCSI_MODE_PAGE_ALL_PAGES 0x3f

struct scsi_mode_page_header
{
  uint8_t page_code;
  uint8_t page_length;
} CC_BYTE_ALIGNED;

#define SCSI_MODE_PAGE_CONTROL_FLAGS1_RLEC 0x01
#define SCSI_MODE_PAGE_CONTROL_FLAGS2_QERR 0x02
#define SCSI_MODE_PAGE_CONTROL_FLAGS2_DQUE 0x02
#define SCSI_MODE_PAGE_CONTROL_FLAGS2_RESTRICED_REORDERING 0x00
#define SCSI_MODE_PAGE_CONTROL_FLAGS2_UNRESTRICED_REORDERING 0x01
#define SCSI_MODE_PAGE_CONTROL_FLAGS3_EECA 0x80
#define SCSI_MODE_PAGE_CONTROL_FLAGS3_UAAENP 0x02
#define SCSI_MODE_PAGE_CONTROL_FLAGS3_EAENP 0x01

struct scsi_mode_page_control
{
  struct scsi_mode_page_header header;
  uint8_t flags1;
  uint8_t flags2;
  uint8_t flags3;
  uint8_t reserved;
  uint16_bytes ready_AEN_holdoff_period;
} CC_BYTE_ALIGNED;

#define SCSI_MODE_PAGE_CONNECT_FLAGS_DTDC_NONE 0x00
#define SCSI_MODE_PAGE_CONNECT_FLAGS_DTDC_TRANSFERRED 0x01
#define SCSI_MODE_PAGE_CONNECT_FLAGS_DTDC_COMPLETE 0x03

struct scsi_mode_page_connect
{
  struct scsi_mode_page_header header;
  uint8_t buffer_full_ratio;
  uint8_t buffer_empty_ratio;
  uint16_bytes bus_inactivity_limit;
  uint16_bytes disconnect_time_limit;
  uint16_bytes connect_time_limit;
  uint16_bytes maximum_burst_size;
  uint8_t flags;
  uint8_t reserved[3];
} CC_BYTE_ALIGNED;

struct scsi_mode_page_peripherial_device
{
  struct scsi_mode_page_header header;
  uint16_bytes interface_identifier;
  uint8_t reserved[4];
  uint16_bytes maximum_burst_size;
} CC_BYTE_ALIGNED;

#define SCSI_MODE_PAGE_DEV_CONF_CAF 0x02
#define SCSI_MODE_PAGE_DEV_CONF_CAP 0x04
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_REW 0x01
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_RBO 0x02
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_SOCF_IGNORE 0x00
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_SOCF_1 0x04
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_SOCF_2 0x08
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_SOCF_3 0xc0
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_AVC 0x10
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_RSMK 0x20
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_BIS 0x40
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS1_DBR 0x40
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_SEW 0x08
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_EEG 0x10
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_EOD_DEFAULT 0x00
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_EOD_ERASE_AREA 0x20
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_EOD_SOCF 0x40
#define SCSI_MODE_PAGE_DEV_CONF_FLAGS2_EOD_NONE 0x60

#define SCSI_MODE_PAGE_DEV_CONF_COMPR_NONE 0x00
#define SCSI_MODE_PAGE_DEV_CONF_COMPR_DEFAULT 0x01

struct scsi_mode_page_device_configuration
{
  struct scsi_mode_page_header header;
  uint8_t active_format;
  uint8_t active_partition;
  uint8_t write_buffer_full_ratio;
  uint8_t read_buffer_empty_ratio;
  uint16_bytes write_delay_time;
  uint8_t flags1;
  uint8_t gap_size;
  uint8_t flags2;
  uint24_bytes buffer_size_at_early_warning;
  uint8_t select_data_compression;
  uint8_t reserved;
};

struct scsi_read_buffer_limits_cmd
{
  uint8_t op_code;
  uint8_t lun;
  uint8_t reserved[3];
  uint8_t control;
} CC_BYTE_ALIGNED;

struct scsi_read_buffer_limits_data
{
  uint8_t reserved;
  uint24_bytes maximum_block_length_limit;
  uint16_bytes minimum_block_length_limit;
} CC_BYTE_ALIGNED;

#define SCSI_READ_FLAGS_FIXED 0x01
#define SCSI_READ_FLAGS_SILI 0x02

struct scsi_read_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint24_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

#define SCSI_WRITE_FLAGS_FIXED 0x01

struct scsi_write_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint24_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

#define SCSI_WRITE_FILEMARKS_FLAGS_IMMED 0x01
#define SCSI_WRITE_FILEMARKS_FLAGS_WSMK 0x02

struct scsi_write_filemarks_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  int24_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

#define SCSI_SPACE_FLAGS_CODE 0x07
#define SCSI_SPACE_FLAGS_CODE_BLOCKS 0x00
#define SCSI_SPACE_FLAGS_CODE_FILEMARKS 0x01
#define SCSI_SPACE_FLAGS_CODE_SEQ_FILEMARKS 0x02
#define SCSI_SPACE_FLAGS_CODE_END_OF_DATA 0x03
#define SCSI_SPACE_FLAGS_CODE_SETMARKS 0x04
#define SCSI_SPACE_FLAGS_CODE_SEQ_SETMARKS 0x05

struct scsi_space_cmd
{
  uint8_t op_code;
  uint8_t flags;
  int24_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;
