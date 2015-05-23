#ifndef SPC2_STRUCT_H_6R1KTJP0NJ__
#define SPC2_STRUCT_H_6R1KTJP0NJ__

typedef uint8_t uint40_bytes[5];
typedef uint8_t uint32_bytes[4];
typedef uint8_t uint16_bytes[2];

#ifndef CC_BYTE_ALIGNED
#ifdef __GNUC__
#define CC_BYTE_ALIGNED __attribute__ ((packed))
#endif
#endif

#ifndef CC_BYTE_ALIGNED
#define CC_BYTE_ALIGNED
#endif

#define HOST16_TO_BE_BYTES(x) {(((x) >> 8) & 0xff), ((x) & 0xff)}
#define HOST32_TO_BE_BYTES(x) {(((x) >> 24) & 0xff), (((x) >> 16) & 0xff), \
 (((x) >> 8) & 0xff), ((x) & 0xff)}
#define HOST40_TO_BE_BYTES(x) {(((x) >> 32) & 0xff), (((x) >> 24) & 0xff), (((x) >> 16) & 0xff), \
 (((x) >> 8) & 0xff), ((x) & 0xff)}

inline unsigned long
be16_to_host(uint16_bytes bytes)
{
  return bytes[0] << 8 | bytes[1];
}

inline unsigned long
be32_to_host(uint32_bytes bytes)
{
  return (be16_to_host(bytes) << 16) | be16_to_host(bytes + 2);
}


#define BE16_TO_HOST(bytes) ((bytes)[0] << 8 | (bytes)[1])
#define BE32_TO_HOST(bytes) \
((BE16_TO_HOST(bytes) << 16) | BE16_TO_HOST((bytes)+2))
struct spc2_inquiry_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t page;
  uint8_t reserved;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct spc2_std_inquiry_data
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

struct spc2_vital_product_data_head
{
  uint8_t device;
  uint8_t page;
  uint8_t reserved;
  uint8_t page_length;
} CC_BYTE_ALIGNED;

struct spc2_identification_descriptor
{
  uint8_t code_set;
  uint8_t identifier_type;
  uint8_t reserved;
  uint8_t identifier_length;
};

struct spc2_request_sense_cmd
{
  uint8_t op_code;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct spc2_sense_data
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

struct spc2_mode_sence_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t page_code;
  uint8_t reserved;
  uint8_t allocation_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct spc2_mode_select_6_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t parameter_list_length;
  uint8_t control;
};

struct spc2_mode_parameter_header_6
{
  uint8_t mode_data_length;
  uint8_t medium_type;
  uint8_t device_specific_parameter;
  uint8_t block_descriptor_length;
} CC_BYTE_ALIGNED;

struct spc2_mode_page
{
  uint8_t page_code;
  uint8_t page_length;
} CC_BYTE_ALIGNED;

#endif /* SPC2_STRUCT_H_6R1KTJP0NJ__ */
