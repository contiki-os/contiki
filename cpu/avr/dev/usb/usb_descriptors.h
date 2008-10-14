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

#define COMPOSITE_DEVICE_CLASS       0xEF      // Misc
#define COMPOSITE_DEVICE_SUB_CLASS   0x02      // Common
#define COMPOSITE_DEVICE_PROTOCOL    0x01      // IAD

#define NETWORK_DEVICE_CLASS          0x02      // CDC ACM
#define NETWORK_DEVICE_SUB_CLASS      0x02      // 
#define NETWORK_DEVICE_PROTOCOL       0xFF      // Vendor-specific

#define MASS_DEVICE_CLASS            0x00      //
#define MASS_DEVICE_SUB_CLASS        0x00      //
#define MASS_DEVICE_PROTOCOL         0x00      //

#define EP_CONTROL_LENGTH     64
#define VENDOR_ID             0x03EB // Atmel vendor ID = 03EBh
#define COMPOSITE_PRODUCT_ID  0x2021 //Product ID for composite device
#define NETWORK_PRODUCT_ID    0x2019 //Product ID for just RNDIS device
#define MASS_PRODUCT_ID       0x202F //Product ID for mass storage
#define RELEASE_NUMBER        0x1000
#define MAN_INDEX             0x01
#define PROD_INDEX            0x02
#define SN_INDEX              0x03
#define NB_CONFIGURATION      1

#define NETWORK_NB_INTERFACE         2
#define COMPOSITE_NB_INTERFACE       4
#define MASS_NB_INTERFACE            1
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
#define EP_INTERVAL_2       0x00

             // USB Endpoint 2 descriptor
				 //Bulk OUT  RX endpoint
#define ENDPOINT_NB_3       RX_EP
#define EP_ATTRIBUTES_3     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_3           0x40  //64 byte max size
#define EP_INTERVAL_3       0x00

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
#define EP_INTERVAL_5       0x00

             // USB Endpoint 6 descriptor
				 // Bulk OUT
#define ENDPOINT_NB_6       VCP_RX_EP
#define EP_ATTRIBUTES_6     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_SIZE_6           0x20
#define EP_INTERVAL_6       0x00


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


#define DEVICE_STATUS         0x00 // TBD
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00




#define USB_MN_LENGTH         5
#define USB_MANUFACTURER_NAME \
{ Usb_unicode('A') \
, Usb_unicode('t') \
, Usb_unicode('m') \
, Usb_unicode('e') \
, Usb_unicode('l') \
}

#define USB_PN_LENGTH         16
#define USB_PRODUCT_NAME \
{ Usb_unicode('R') \
 ,Usb_unicode('Z') \
 ,Usb_unicode('R') \
 ,Usb_unicode('A') \
 ,Usb_unicode('V') \
 ,Usb_unicode('E') \
 ,Usb_unicode('N') \
 ,Usb_unicode(' ') \
 ,Usb_unicode('U') \
 ,Usb_unicode('S') \
 ,Usb_unicode('B') \
 ,Usb_unicode(' ') \
 ,Usb_unicode('D') \
 ,Usb_unicode('E') \
 ,Usb_unicode('M') \
 ,Usb_unicode('O') \
}

#define USB_SN_LENGTH         0x05
#define USB_SERIAL_NUMBER \
{Usb_unicode('1') \
 ,Usb_unicode('.') \
 ,Usb_unicode('0') \
 ,Usb_unicode('.') \
 ,Usb_unicode('0') \
}

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


//_____ U S B   M A N U F A C T U R E R   D E S C R I P T O R _______________


//struct usb_st_manufacturer
typedef struct {
   U8  bLength;               // size of this descriptor in bytes
   U8  bDescriptorType;       // STRING descriptor type
   U16 wstring[USB_MN_LENGTH];// unicode characters
} S_usb_manufacturer_string_descriptor;


//_____ U S B   P R O D U C T   D E S C R I P T O R _________________________


//struct usb_st_product
typedef struct {
   U8  bLength;               // size of this descriptor in bytes
   U8  bDescriptorType;       // STRING descriptor type
   U16 wstring[USB_PN_LENGTH];// unicode characters
} S_usb_product_string_descriptor;


//_____ U S B   S E R I A L   N U M B E R   D E S C R I P T O R _____________


//struct usb_st_serial_number
typedef struct {
   U8  bLength;               // size of this descriptor in bytes
   U8  bDescriptorType;       // STRING descriptor type
   U16 wstring[USB_SN_LENGTH];// unicode characters
} S_usb_serial_number;


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
PGM_VOID_P  Usb_get_conf_desc_pointer(void) ;
U8  Usb_get_conf_desc_length(void);

#endif // _USB_USERCONFIG_H_

/** @} */
