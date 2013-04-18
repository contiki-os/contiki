/*
 * Copyright (c) 2009, Simon Berg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-usb
 * @{
 *
 * \file
 * CDC-ACM mode USB descriptor for the cc2538 USB controller
 *
 * This file is a copy of cpu/arm/common/usb/cdc-acm/cdc-acm-descriptors.c
 * with the only change being the values for VID and PID
 */
#include "descriptors.h"
#include "contiki-conf.h"
#include "cdc.h"
#include "usb-arch.h"

const struct usb_st_device_descriptor device_descriptor =
  {
    sizeof(struct usb_st_device_descriptor),
    DEVICE,
    0x0200,
    CDC,
    0,
    0,
    CTRL_EP_SIZE,
    0x0451, /* Vendor: TI */
    0x16C8, /* Product: cc2538EM ("CC2538 USB CDC") */
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
} configuration_block =
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
      0x2, /** Set line coding */
    },
    {
      sizeof(configuration_block.union_descr),
      CS_INTERFACE,
      CDC_FUNC_DESCR_UNION,
      0, /** Master */
      {1}  /** Slave */
    },
    {
      sizeof(configuration_block.call_mgmt),
      CS_INTERFACE,
      CDC_FUNC_DESCR_CALL_MGMNT,
      0x00,
      1 /** data interface */
    },
    {
      sizeof(configuration_block.ep_notification),
      ENDPOINT,
      0x81,
      0x03,
      USB_EP1_SIZE,
      255 /** 255ms polling, not really used so maximum value used */
    },
    {
      sizeof(configuration_block.data),
      INTERFACE,
      1,
      0,
      2,
      CDC_DATA,
      0,
      0, /** TRANSPARENT_PROTOCOL*/
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
(struct usb_st_configuration_descriptor const *)&configuration_block;

/** @} */
