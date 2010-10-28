/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file  usb_descriptors.h ***************************************************
 *
 * \brief
 *      This file contains the usb parameters that uniquely identify the
 *      application through descriptor tables.
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com
 * \author
 *      Colin O'Flynn <coflynn@newae.com>
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   Copyright (c) 2008 Colin O'Flynn
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
  \addtogroup usbconf
  @{
  */

#ifndef _USB_USERCONFIG_H_
#define _USB_USERCONFIG_H_

//_____ I N C L U D E S ____________________________________________________

#include "config.h"
#include "usb_standard_request.h"
#include "conf_usb.h"
#include <avr/pgmspace.h>

//_____ M A C R O S ________________________________________________________


//_____ U S B    D E F I N E _______________________________________________

                  // USB Device descriptor
#define USB_SPECIFICATION     0x0200
#if USB_CONF_MACINTOSH
#define COMPOSITE_DEVICE_CLASS       0x02      // Misc
#define COMPOSITE_DEVICE_SUB_CLASS   0x00      // Common
#define COMPOSITE_DEVICE_PROTOCOL    0x00      // IAD
#else //Windows wants these for composite device
//Above seems to work for Vista and Win7 but XP and Ubuntu 904 might need the old values
#define COMPOSITE_DEVICE_CLASS       0xEF      // Misc
#define COMPOSITE_DEVICE_SUB_CLASS   0x02      // Common
#define COMPOSITE_DEVICE_PROTOCOL    0x01      // IAD
#endif

#define NETWORK_DEVICE_CLASS          0x02      // CDC ACM
#define NETWORK_DEVICE_SUB_CLASS      0x02      //
#define NETWORK_DEVICE_PROTOCOL       0xFF      // Vendor-specific

#define MASS_DEVICE_CLASS            0x00      //
#define MASS_DEVICE_SUB_CLASS        0x00      //
#define MASS_DEVICE_PROTOCOL         0x00      //

#define EEM_DEVICE_CLASS              0x02      // CDC
#define EEM_DEVICE_SUB_CLASS          0x0C      // EEM
#define EEM_DEVICE_PROTOCOL           0x07      // EEM

#define EP_CONTROL_LENGTH     64
#define VENDOR_ID             0x03EB // Atmel vendor ID = 03EBh

#if USB_CONF_MACINTOSH
//A different product ID avoids instant windows corruption when it tries to use the cached drivers
//TODO:Get some valid ID's from Atmel
#define COMPOSITE_PRODUCT_ID  0x9921 //Product ID for composite device
#define NETWORK_PRODUCT_ID    0x9919 //Product ID for just CDC-ECM device
#else
#define COMPOSITE_PRODUCT_ID  0x2021 //Product ID for composite device
#define NETWORK_PRODUCT_ID    0x2019 //Product ID for just RNDIS device
#endif

#define MASS_PRODUCT_ID       0x202F //Product ID for mass storage
#define RELEASE_NUMBER        0x1000

enum {
	USB_STRING_NONE = 0,
	USB_STRING_MAN = 1,
	USB_STRING_PRODUCT,
	USB_STRING_SERIAL,
	USB_STRING_MAC_ADDRESS,
	USB_STRING_CONFIG_COMPOSITE,
	USB_STRING_CONFIG_RNDIS,
	USB_STRING_CONFIG_EEM,
	USB_STRING_CONFIG_ECM,
	USB_STRING_CONFIG_ECM_DEBUG,
	USB_STRING_CONFIG_MS,
	USB_STRING_INTERFACE_RNDIS,
	USB_STRING_INTERFACE_EEM,
	USB_STRING_INTERFACE_ECM,
	USB_STRING_INTERFACE_ECM_ATTACHED,
	USB_STRING_INTERFACE_ECM_DETACHED,
	USB_STRING_INTERFACE_SERIAL,
	USB_STRING_INTERFACE_MS,

};

enum {
	USB_CONFIG_UNCONFIGURED = 0,

	USB_CONFIG_RNDIS		= 1,
	USB_CONFIG_RNDIS_DEBUG	= 1+(1<<7),

	USB_CONFIG_ECM			= 2,
	USB_CONFIG_ECM_DEBUG	= 2+(1<<7),
	
	USB_CONFIG_EEM			= 3,

#if USB_CONF_STORAGE
	USB_CONFIG_MS			= 4,
#endif
};

#define USB_CONFIG_HAS_DEBUG_PORT(x)	((x==USB_CONFIG_ECM_DEBUG)||(x==USB_CONFIG_RNDIS_DEBUG))

//#define USB_CONFIG_COUNT (USB_CONFIG_COUNT_PLUS_ONE-1)
//#define NB_CONFIGURATION      USB_CONFIG_COUNT

#define NETWORK_NB_INTERFACE         2
#define COMPOSITE_NB_INTERFACE       4
#define MASS_NB_INTERFACE            1
#define EEM_NB_INTERFACE          1
#define CONF_NB            1
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_BUSPOWERED
#define MAX_POWER          50          // 100 mA

/*** CDC RNDIS CONFIGURATION CONFIGURATION ***/

// Interface 0 descriptor
#define INTERFACE0_NB        0
#define ALTERNATE0           0
#define NB_ENDPOINT0         1
#define INTERFACE0_CLASS     0x02    // CDC ACM Com
#define INTERFACE0_SUB_CLASS 0x02
#define INTERFACE0_PROTOCOL  0xFF	 // Vendor specific
#define INTERFACE0_INDEX     0

// Interface 1 descriptor
#define INTERFACE1_NB        1
#define ALTERNATE1           0
#define NB_ENDPOINT1         2
#define INTERFACE1_CLASS     0x0A    // CDC ACM Data
#define INTERFACE1_SUB_CLASS 0
#define INTERFACE1_PROTOCOL  0
#define INTERFACE1_INDEX     0

             // USB Endpoint 1 descriptor
				 // Interrupt IN
#define ENDPOINT_NB_1       0x80 | INT_EP
#define EP_ATTRIBUTES_1     0x03          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_1           0x08
#define EP_INTERVAL_1       0x01 //ms interrupt pooling from host

             // USB Endpoint 1 descriptor
				 // Bulk IN
#define ENDPOINT_NB_2       0x80 | TX_EP
#define EP_ATTRIBUTES_2     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_2           0x40  //64 byte max size
#define EP_INTERVAL_2       0x01

             // USB Endpoint 2 descriptor
				 //Bulk OUT  RX endpoint
#define ENDPOINT_NB_3       RX_EP
#define EP_ATTRIBUTES_3     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_3           0x40  //64 byte max size
#define EP_INTERVAL_3       0x01

/*** CDC Virtual Serial Port ***/

             // Interface 2 descriptor
#define INTERFACE2_NB        2
#define ALTERNATE2           0
#define NB_ENDPOINT2         1
#define INTERFACE2_CLASS     0x02    // CDC ACM Com
#define INTERFACE2_SUB_CLASS 0x02
#define INTERFACE2_PROTOCOL  0x01
#define INTERFACE2_INDEX     0

             // Interface 3 descriptor
#define INTERFACE3_NB        3
#define ALTERNATE3           0
#define NB_ENDPOINT3         2
#define INTERFACE3_CLASS     0x0A    // CDC ACM Data
#define INTERFACE3_SUB_CLASS 0
#define INTERFACE3_PROTOCOL  0
#define INTERFACE3_INDEX     0

             // USB Endpoint 4 descriptor
				 // Interrupt IN
#define TX_EP_SIZE          0x20
#define ENDPOINT_NB_4       0x80 | VCP_INT_EP
#define EP_ATTRIBUTES_4     0x03          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_4           TX_EP_SIZE
#define EP_INTERVAL_4       0xFF //ms interrupt pooling from host

             // USB Endpoint 5 descriptor
				 // Bulk IN
#define ENDPOINT_NB_5       0x80 | VCP_TX_EP
#define EP_ATTRIBUTES_5     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_5           0x20
#define EP_INTERVAL_5       0x01

             // USB Endpoint 6 descriptor
				 // Bulk OUT
#define ENDPOINT_NB_6       VCP_RX_EP
#define EP_ATTRIBUTES_6     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_6           0x20
#define EP_INTERVAL_6       0x01

/*** Mass Storage ***/

#define MS_INTERFACE_NB        0
#define MS_ALTERNATE           0
#define MS_NB_ENDPOINT         2
#define MS_INTERFACE_CLASS     0x08   		// Mass Storage Class
#define MS_INTERFACE_SUB_CLASS 0x06			// SCSI transparent Command Set
#define MS_INTERFACE_PROTOCOL  0x50			// Bulk-Only Transport
#define MS_INTERFACE_INDEX     0

// USB Endpoint 1 descriptor FS
#define MS_ENDPOINT_NB_1       (MS_IN_EP | 0x80)
#define MS_EP_ATTRIBUTES_1     0x02          // BULK = 0x02, INTERUPT = 0x03
#define MS_EP_IN_LENGTH        64
#define MS_EP_SIZE_1           MS_EP_IN_LENGTH
#define MS_EP_INTERVAL_1       0x00


// USB Endpoint 2 descriptor FS
#define MS_ENDPOINT_NB_2       MS_OUT_EP
#define MS_EP_ATTRIBUTES_2     0x02          // BULK = 0x02, INTERUPT = 0x03
#define MS_EP_IN_LENGTH        64
#define MS_EP_SIZE_2           MS_EP_IN_LENGTH
#define MS_EP_INTERVAL_2       0x00

/******* EEM Configuration *******/

// Interface 0 descriptor
#define EEM_INTERFACE0_NB        0
#define EEM_ALTERNATE0           0
#define EEM_NB_ENDPOINT0         2
#define EEM_INTERFACE0_CLASS     0x02    // CDC ACM Com
#define EEM_INTERFACE0_SUB_CLASS 0x0C    // EEM
#define EEM_INTERFACE0_PROTOCOL  0x07    // EEM
#define EEM_INTERFACE0_INDEX     0

            // USB Endpoint 1 descriptor
            // Bulk IN
#define EEM_ENDPOINT_NB_1       0x80 | TX_EP
#define EEM_EP_ATTRIBUTES_1     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EEM_EP_SIZE_1           0x40 //64 byte max size
#define EEM_EP_INTERVAL_1       0x01

             // USB Endpoint 2 descriptor
             // Bulk OUT
#define EEM_ENDPOINT_NB_2       RX_EP
#define EEM_EP_ATTRIBUTES_2     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EEM_EP_SIZE_2           0x40  //64 byte max size
#define EEM_EP_INTERVAL_2       0x01

/******* ECM Configuration *******/

// Interface 0 descriptor
#define ECM_INTERFACE0_NB        0
#define ECM_ALTERNATE0           0
#define ECM_NB_ENDPOINT0         2
#define ECM_INTERFACE0_CLASS     0x02    // CDC ACM Com
#define ECM_INTERFACE0_SUB_CLASS 0x06    // ECM
#define ECM_INTERFACE0_PROTOCOL  0x00    // Empty
#define ECM_INTERFACE0_INDEX     0



#define DEVICE_STATUS         0x00 // TBD
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00

#define LANGUAGE_ID           0x0409

                  //! Usb Request
typedef struct
{
   U8      bmRequestType;        //!< Characteristics of the request
   U8      bRequest;             //!< Specific request
   U16     wValue;               //!< field that varies according to request
   U16     wIndex;               //!< field that varies according to request
   U16     wLength;              //!< Number of bytes to transfer if Data
}  S_UsbRequest;

                //! Usb Device Descriptor
typedef struct {
   U8      bLength;              //!< Size of this descriptor in bytes
   U8      bDescriptorType;      //!< DEVICE descriptor type
   U16     bscUSB;               //!< Binay Coded Decimal Spec. release
   U8      bDeviceClass;         //!< Class code assigned by the USB
   U8      bDeviceSubClass;      //!< Sub-class code assigned by the USB
   U8      bDeviceProtocol;      //!< Protocol code assigned by the USB
   U8      bMaxPacketSize0;      //!< Max packet size for EP0
   U16     idVendor;             //!< Vendor ID. ATMEL = 0x03EB
   U16     idProduct;            //!< Product ID assigned by the manufacturer
   U16     bcdDevice;            //!< Device release number
   U8      iManufacturer;        //!< Index of manu. string descriptor
   U8      iProduct;             //!< Index of prod. string descriptor
   U8      iSerialNumber;        //!< Index of S.N.  string descriptor
   U8      bNumConfigurations;   //!< Number of possible configurations
}  S_usb_device_descriptor;


          //! Usb Configuration Descriptor
typedef struct {
   U8      bLength;              //!< size of this descriptor in bytes
   U8      bDescriptorType;      //!< CONFIGURATION descriptor type
   U16     wTotalLength;         //!< total length of data returned
   U8      bNumInterfaces;       //!< number of interfaces for this conf.
   U8      bConfigurationValue;  //!< value for SetConfiguration resquest
   U8      iConfiguration;       //!< index of string descriptor
   U8      bmAttibutes;          //!< Configuration characteristics
   U8      MaxPower;             //!< maximum power consumption
}  S_usb_configuration_descriptor;


              //! Usb Interface Descriptor
typedef struct {
   U8      bLength;               //!< size of this descriptor in bytes
   U8      bDescriptorType;       //!< INTERFACE descriptor type
   U8      bInterfaceNumber;      //!< Number of interface
   U8      bAlternateSetting;     //!< value to select alternate setting
   U8      bNumEndpoints;         //!< Number of EP except EP 0
   U8      bInterfaceClass;       //!< Class code assigned by the USB
   U8      bInterfaceSubClass;    //!< Sub-class code assigned by the USB
   U8      bInterfaceProtocol;    //!< Protocol code assigned by the USB
   U8      iInterface;            //!< Index of string descriptor
}  S_usb_interface_descriptor;


               //! Usb Endpoint Descriptor
typedef struct {
   U8      bLength;               //!< Size of this descriptor in bytes
   U8      bDescriptorType;       //!< ENDPOINT descriptor type
   U8      bEndpointAddress;      //!< Address of the endpoint
   U8      bmAttributes;          //!< Endpoint's attributes
   U16     wMaxPacketSize;        //!< Maximum packet size for this EP
   U8      bInterval;             //!< Interval for polling EP in ms
} S_usb_endpoint_descriptor;


               //! Usb Device Qualifier Descriptor
typedef struct {
   U8      bLength;               //!< Size of this descriptor in bytes
   U8      bDescriptorType;       //!< Device Qualifier descriptor type
   U16     bscUSB;                //!< Binay Coded Decimal Spec. release
   U8      bDeviceClass;          //!< Class code assigned by the USB
   U8      bDeviceSubClass;       //!< Sub-class code assigned by the USB
   U8      bDeviceProtocol;       //!< Protocol code assigned by the USB
   U8      bMaxPacketSize0;       //!< Max packet size for EP0
   U8      bNumConfigurations;    //!< Number of possible configurations
   U8      bReserved;             //!< Reserved for future use, must be zero
} S_usb_device_qualifier_descriptor;


               //! Usb Language Descriptor
typedef struct {
   U8      bLength;               //!< size of this descriptor in bytes
   U8      bDescriptorType;       //!< STRING descriptor type
   U16     wlangid;               //!< language id
} S_usb_language_id;


/*_____ U S B   I A D _______________________________________________________*/

#define DSC_TYPE_IAD  11

typedef struct
{
	U8 bLength;
	U8 bDescriptorType;
	U8 bFirstInterface;
	U8 bInterfaceCount;
	U8 bFunctionClass;
	U8 bFunctionSubClass;
	U8 bFunctionProtocol;
	U8 iInterface;
} S_usb_interface_association_descriptor;

/*_____ U S B   D E S C R I P T O R __________________________________*/

/* RNDIS + Serial Port */
typedef struct
{
   S_usb_configuration_descriptor cfg;

  S_usb_interface_association_descriptor   iad0;
   S_usb_interface_descriptor     ifc0;
	U8 CS1_INTERFACE[19];
 S_usb_endpoint_descriptor      ep1;
   S_usb_interface_descriptor     ifc1;
   S_usb_endpoint_descriptor      ep2;
   S_usb_endpoint_descriptor      ep3;

  S_usb_interface_association_descriptor   iad1;
   S_usb_interface_descriptor     ifc2;
	U8 CS2_INTERFACE[19];
   S_usb_endpoint_descriptor      ep4;
   S_usb_interface_descriptor     ifc3;
   S_usb_endpoint_descriptor      ep5;
   S_usb_endpoint_descriptor      ep6;

} S_usb_user_configuration_descriptor_composite;

/* Just RNDIS */
typedef struct
{
   S_usb_configuration_descriptor cfg;
   S_usb_interface_descriptor     ifc0;
	U8 CS1_INTERFACE[19];
   S_usb_endpoint_descriptor      ep1;
   S_usb_interface_descriptor     ifc1;
   S_usb_endpoint_descriptor      ep2;
   S_usb_endpoint_descriptor      ep3;

} S_usb_user_configuration_descriptor_network;

/* EEM */
typedef struct
{
   S_usb_configuration_descriptor cfg;
   S_usb_interface_descriptor     ifc0;
   S_usb_endpoint_descriptor      ep1;
   S_usb_endpoint_descriptor      ep2;
} S_usb_user_configuration_descriptor_eem;




typedef struct
{
	U8 bLength;
	U8 bDescriptorType;
	U8 bDescriptorSubtype;
	U8 iMACAddress;
	U32 bmEthernetStatistics;
	U16 wMaxSegmentSize;
	U16 wNumberMCFilters;
	U8 bNumberPowerFilters;
} S_usb_ethernet_networking_functional_descriptor;


/* ECM */
typedef struct
{
   S_usb_configuration_descriptor cfg;
   S_usb_interface_descriptor     ifc0;
   U8 CS1_INTERFACE[5+5];
   S_usb_ethernet_networking_functional_descriptor fd0;
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
   S_usb_endpoint_descriptor      ep1;
#endif
#if 0
   S_usb_interface_descriptor     ifc1_0;
#endif
   S_usb_interface_descriptor     ifc1_1;
   S_usb_endpoint_descriptor      ep2;
   S_usb_endpoint_descriptor      ep3;
} S_usb_user_configuration_descriptor_ecm;


/* ECM + Serial Port */
typedef struct
{
   S_usb_configuration_descriptor cfg;
  S_usb_interface_association_descriptor   iad0;
   S_usb_interface_descriptor     ifc0;
   U8 CS1_INTERFACE[5+5];
   S_usb_ethernet_networking_functional_descriptor fd0;
#if CDC_ECM_USES_INTERRUPT_ENDPOINT
   S_usb_endpoint_descriptor      ep1;
#endif
#if 0
   S_usb_interface_descriptor     ifc1_0;
#endif
   S_usb_interface_descriptor     ifc1_1;
   S_usb_endpoint_descriptor      ep2;
   S_usb_endpoint_descriptor      ep3;

  S_usb_interface_association_descriptor   iad1;
   S_usb_interface_descriptor     ifc2;
	U8 CS2_INTERFACE[19];
   S_usb_endpoint_descriptor      ep4;
   S_usb_interface_descriptor     ifc3;
   S_usb_endpoint_descriptor      ep5;
   S_usb_endpoint_descriptor      ep6;

} S_usb_user_configuration_descriptor_ecm_debug;


/* Mass Storage */

typedef struct
{
   S_usb_configuration_descriptor cfg;
   S_usb_interface_descriptor     ifc0;
   S_usb_endpoint_descriptor      ep1;
   S_usb_endpoint_descriptor      ep2;

} S_usb_user_configuration_descriptor_mass;



PGM_VOID_P Usb_get_dev_desc_pointer(void);
U8 Usb_get_dev_desc_length(void);
PGM_VOID_P  Usb_get_conf_desc_pointer(U8 index) ;
U8  Usb_get_conf_desc_length(U8 index);

#endif // _USB_USERCONFIG_H_

/** @} */
