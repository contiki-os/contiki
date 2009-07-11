#include "descriptors.h"
#include "usb-msc-bulk.h"
#include <usb-arch.h>

const struct usb_st_device_descriptor device_descriptor =
  {
    sizeof(struct usb_st_device_descriptor),
    DEVICE,
    0x0210,
    0,
    0,
    0,
    CTRL_EP_SIZE,
    0xffff,
    0xffff,
    0x0030,
    2,
    1,
    3,
    1
  };

const struct configuration_st {
  struct usb_st_configuration_descriptor configuration;
  struct usb_st_interface_descriptor massbulk;
  struct usb_st_endpoint_descriptor ep_in;
  struct usb_st_endpoint_descriptor ep_out;
} BYTE_ALIGNED configuration_block =
  {
    /* Configuration */
    {
      sizeof(configuration_block.configuration),
      CONFIGURATION,
      sizeof(configuration_block),
      1,
      1,
      0,
      0x80,
      50
    },
    {
      sizeof(configuration_block.massbulk),
      INTERFACE,
      0,
      0,
      2,
      MASS_STORAGE,
      MASS_RBC,
      MASS_BULK_ONLY,
      0
    },
    {
      sizeof(configuration_block.ep_in),
      ENDPOINT,
      0x81,
      0x02,
      64,
      0
    },
    {
      sizeof(configuration_block.ep_out),
      ENDPOINT,
      0x02,
      0x02,
      64,
      0
    }
          
  };

const struct usb_st_configuration_descriptor const *configuration_head =
(struct usb_st_configuration_descriptor const*)&configuration_block;
