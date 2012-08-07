#include "descriptors.h"
#include "contiki-conf.h"
#include "cdc.h"
#include "usb-arch.h"

#if USB_CONF_CLASS == 1

const struct usb_st_device_descriptor device_descriptor =
  {
    sizeof(struct usb_st_device_descriptor),
    DEVICE,
    0x0200,
    CDC,
    0,
    0,
    CTRL_EP_SIZE,
    0x0617, /* EPFL usb ID */
    0x000B, /* Don't mess with it please */
    0x0000,
    1,
    2,
    3,
    1
  };

const struct configuration_st {
  struct usb_st_configuration_descriptor configuration;
  struct usb_st_interface_descriptor comm;
  struct usb_cdc_header_func_descriptor header;
  struct usb_cdc_abstract_ctrl_mgmnt_func_descriptor abstract_ctrl;
  struct usb_cdc_union_func_descriptor union_descr;
  struct usb_cdc_call_mgmnt_func_descriptor call_mgmt;
#if 1
  struct usb_st_endpoint_descriptor ep_notification;
#endif
  struct usb_st_interface_descriptor data;
  struct usb_st_endpoint_descriptor ep_in;
  struct usb_st_endpoint_descriptor ep_out;
} BYTE_ALIGNED configuration_block =
  {
    /* Configuration */
    {
      sizeof(configuration_block.configuration),
      CONFIGURATION,
      sizeof(configuration_block),
      2,
      1,
      0,
      0x80,
      250
    },
    {
      sizeof(configuration_block.comm),
      INTERFACE,
      0,
      0,
      1,
      CDC,
      ABSTRACT_CONTROL_MODEL,
      V_25TER_PROTOCOL,
      0
    },
    {
      sizeof(configuration_block.header),
      CS_INTERFACE,
      CDC_FUNC_DESCR_HEADER,
      0x0110
    },
    {
      sizeof(configuration_block.abstract_ctrl),
      CS_INTERFACE,
      CDC_FUNC_DESCR_ABSTRACT_CTRL_MGMNT,
      0x2, // Set line coding
    },
    {
      sizeof(configuration_block.union_descr),
      CS_INTERFACE,
      CDC_FUNC_DESCR_UNION,
      0, /* Master */
      {1}  /* Slave */
    },
    {
      sizeof(configuration_block.call_mgmt),
      CS_INTERFACE,
      CDC_FUNC_DESCR_CALL_MGMNT,
      0x00,
      1 /* data interface */
    },
    {
      sizeof(configuration_block.ep_notification),
      ENDPOINT,
      0x81,
      0x03,
      USB_EP1_SIZE,
      255 /* 255ms polling, not really used so maximum value used */
    },
    {
      sizeof(configuration_block.data),
      INTERFACE,
      1,
      0,
      2,
      CDC_DATA,
      0,
      /*TRANSPARENT_PROTOCOL*/ 0,
      0
    },
    {
      sizeof(configuration_block.ep_in),
      ENDPOINT,
      0x82,
      0x02,
      USB_EP2_SIZE,
      0
    },
    {
      sizeof(configuration_block.ep_out),
      ENDPOINT,
      0x03,
      0x02,
      USB_EP3_SIZE,
      0
    }

  };

const struct usb_st_configuration_descriptor const *configuration_head =
(struct usb_st_configuration_descriptor const*)&configuration_block;

#endif
