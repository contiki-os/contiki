#ifndef __RBC_STRUCT_H__10L0HKARRV__
#define __RBC_STRUCT_H__10L0HKARRV__

#include <spc2_struct.h>

struct rbc_read_capacity_cmd
{
  uint8_t op_code;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t reserved4;
  uint8_t reserved5;
  uint8_t reserved6;
  uint8_t reserved7;
  uint8_t reserved8;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct rbc_read_capacity_data
{
  uint32_bytes last_logical_block;
  uint32_bytes block_length;
} CC_BYTE_ALIGNED;

struct rbc_device_parameters_page
{
  struct spc2_mode_page head;
  uint8_t flags1;
  uint16_bytes logical_block_size;
  uint40_bytes num_logical_blocks;
  uint8_t power_performance;
  uint8_t flags2;
  uint8_t reserved;
} CC_BYTE_ALIGNED;

struct rbc_read_cmd
{
  uint8_t op_code;
  uint8_t reserved1;
  uint32_bytes logical_block_address;
  uint8_t reserved2;
  uint16_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;

struct rbc_write_cmd
{
  uint8_t op_code;
  uint8_t flags;
  uint32_bytes logical_block_address;
  uint8_t reserved;
  uint16_bytes transfer_length;
  uint8_t control;
} CC_BYTE_ALIGNED;


#endif /* __RBC_STRUCT_H__10L0HKARRV__ */
