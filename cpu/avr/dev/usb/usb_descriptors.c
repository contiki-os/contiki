/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file usb_descriptors.c *****************************************************
 *
 * \brief
 *      This file contains the usb parameters that uniquely identify the
 *      application through descriptor tables.
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 * \author
 *      Colin O'Flynn <coflynn@newae.com>
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
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

//_____ I N C L U D E S ____________________________________________________

#include "config.h"

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"


//_____ M A C R O S ________________________________________________________




//_____ D E F I N I T I O N ________________________________________________

/************* COMPOSITE DEVICE DESCRIPTORS (using IAD) **********/

// usb_user_device_descriptor
FLASH S_usb_device_descriptor usb_dev_desc_composite =
{
  sizeof(usb_dev_desc_composite)
, DEVICE_DESCRIPTOR
, Usb_write_word_enum_struc(USB_SPECIFICATION)
, COMPOSITE_DEVICE_CLASS
, COMPOSITE_DEVICE_SUB_CLASS
, COMPOSITE_DEVICE_PROTOCOL
, EP_CONTROL_LENGTH
, Usb_write_word_enum_struc(VENDOR_ID)
, Usb_write_word_enum_struc(COMPOSITE_PRODUCT_ID)
, Usb_write_word_enum_struc(RELEASE_NUMBER)
, MAN_INDEX
, PROD_INDEX
, SN_INDEX
, NB_CONFIGURATION
};


// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_composite usb_conf_desc_composite = {
 { sizeof(S_usb_configuration_descriptor)
 , CONFIGURATION_DESCRIPTOR
 , Usb_write_word_enum_struc(sizeof(S_usb_user_configuration_descriptor_composite))
 //, 0x0043 //TODO: Change to generic codewith sizeof
 , COMPOSITE_NB_INTERFACE
 , CONF_NB
 , CONF_INDEX
 , CONF_ATTRIBUTES
 , MAX_POWER
 },//9

  // --------------------------- IAD ----------------------------
  {                 // Interface Association Descriptor
    sizeof(S_usb_interface_association_descriptor),  // bLength
    DSC_TYPE_IAD,       // bDescriptorType = 11
    0x00,               // bFirstInterface
    0x02,               // bInterfaceCount
    0x02,               // bFunctionClass (Communication Class)
    0x02,               // bFunctionSubClass (Abstract Control Model)
    0xFF,               // bFunctionProcotol (Vendor specific)
    0x00                // iInterface
  },//8
  
 /// RNDIS DEVICE
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE0_NB
 , ALTERNATE0
 , NB_ENDPOINT0
 , INTERFACE0_CLASS
 , INTERFACE0_SUB_CLASS
 , INTERFACE0_PROTOCOL
 , INTERFACE0_INDEX
 } //9
,
{
// Header Functional Descriptor (marks beginning of the concatenated set of Functional Descriptors)
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x00, // bDescriptorSubtype, Header Functional Descriptor
0x10,0x01, // bcdCDC, CDC specification release number in BCD format (1,1)

// Call Management Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x01, // bDescriptorSubtype, Call Management Functional Descriptor
0x00, // bmCapabilities
0x01, // bDataInterface, Interface used for call management

// Abstract Control Management Functional Descriptor
0x04, // bDescriptorLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x02, // bDescriptorSubtype, Abstract Control Management Functional Descriptor
0x00, // bmCapabilities

// Union Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x06, // bDescriptorSubtype, Union Functional Descriptor
0x00, // bMasterInterface, The controlling interface for the union (bInterfaceNumber of a Communication or Data Class interface in this configuration)
0x01, // bSlaveInterface0, The controlled interace in the union (bInterfaceNumber of an interface in this configuration)
},
{ sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_1
 , EP_ATTRIBUTES_1
 , Usb_write_word_enum_struc(EP_SIZE_1)
 , EP_INTERVAL_1
 } //7
 ,
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE1_NB
 , ALTERNATE1
 , NB_ENDPOINT1
 , INTERFACE1_CLASS
 , INTERFACE1_SUB_CLASS
 , INTERFACE1_PROTOCOL
 , INTERFACE1_INDEX
 },
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_2
 , EP_ATTRIBUTES_2
 , Usb_write_word_enum_struc(EP_SIZE_2)
 , EP_INTERVAL_2
 },
{ sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_3
 , EP_ATTRIBUTES_3
 , Usb_write_word_enum_struc(EP_SIZE_3)
 , EP_INTERVAL_3
 },

// --------------------------- IAD ----------------------------
  {                 // Interface Association Descriptor
    sizeof(S_usb_interface_association_descriptor),  // bLength
    DSC_TYPE_IAD,       // bDescriptorType = 11
    0x02,               // bFirstInterface
    0x02,               // bInterfaceCount
    0x02,               // bFunctionClass (Communication Class)
    0x02,               // bFunctionSubClass (Abstract Control Model)
    0x01,               // bFunctionProcotol (V.25ter, Common AT commands)
    0x00                // iInterface
  },//8

 /// SERIAL PORT DEVICE

 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE2_NB
 , ALTERNATE2
 , NB_ENDPOINT2
 , INTERFACE2_CLASS
 , INTERFACE2_SUB_CLASS
 , INTERFACE2_PROTOCOL
 , INTERFACE2_INDEX
 }//9
 ,
 {
// Header Functional Descriptor (marks beginning of the concatenated set of Functional Descriptors)
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x00, // bDescriptorSubtype, Header Functional Descriptor
0x10,0x01, // bcdCDC, CDC specification release number in BCD format (1,1)

// Call Management Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x01, // bDescriptorSubtype, Call Management Functional Descriptor
0x03, // bmCapabilities, can do calls on it's own
0x03, // bDataInterface, Interface used for call management

// Abstract Control Management Functional Descriptor
0x04, // bDescriptorLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x02, // bDescriptorSubtype, Abstract Control Management Functional Descriptor
0x00, // bmCapabilities, support nothing!!! Deal with it

         //Old was 0x06 indicating support for the GET/SET_LINE_CODING, BREAK & SET_CONTROL_LINE_STATE (2->6)

// Union Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x06, // bDescriptorSubtype, Union Functional Descriptor
0x02, // bMasterInterface, The controlling interface for the union (bInterfaceNumber of a Communication or Data Class interface in this configuration)
0x03, // bSlaveInterface0, The controlled interace in the union (bInterfaceNumber of an interface in this configuration)

},
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_4
 , EP_ATTRIBUTES_4
 , Usb_write_word_enum_struc(EP_SIZE_4)
 , EP_INTERVAL_4
 } //7
 ,
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE3_NB
 , ALTERNATE3
 , NB_ENDPOINT3
 , INTERFACE3_CLASS
 , INTERFACE3_SUB_CLASS
 , INTERFACE3_PROTOCOL
 , INTERFACE3_INDEX
 }
 ,
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_5
 , EP_ATTRIBUTES_5
 , Usb_write_word_enum_struc(EP_SIZE_5)
 , EP_INTERVAL_5
 }
  ,
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_6
 , EP_ATTRIBUTES_6
 , Usb_write_word_enum_struc(EP_SIZE_6)
 , EP_INTERVAL_6
 }

};

/****************** NETWORK-ONLY DEVICE DESCRIPTORS **************************/

FLASH S_usb_device_descriptor usb_dev_desc_network =
{
  sizeof(usb_dev_desc_network)
, DEVICE_DESCRIPTOR
, Usb_write_word_enum_struc(USB_SPECIFICATION)
, NETWORK_DEVICE_CLASS
, NETWORK_DEVICE_SUB_CLASS
, NETWORK_DEVICE_PROTOCOL
, EP_CONTROL_LENGTH
, Usb_write_word_enum_struc(VENDOR_ID)
, Usb_write_word_enum_struc(NETWORK_PRODUCT_ID)
, Usb_write_word_enum_struc(RELEASE_NUMBER)
, MAN_INDEX
, PROD_INDEX
, SN_INDEX
, NB_CONFIGURATION
};

// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_network usb_conf_desc_network = {
 { sizeof(S_usb_configuration_descriptor)
 , CONFIGURATION_DESCRIPTOR
 , Usb_write_word_enum_struc(sizeof(S_usb_user_configuration_descriptor_network))
 //, 0x0043 //TODO: Change to generic codewith sizeof
 , NETWORK_NB_INTERFACE
 , CONF_NB
 , CONF_INDEX
 , CONF_ATTRIBUTES
 , MAX_POWER
 },//9

  /// RNDIS DEVICE
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE0_NB
 , ALTERNATE0
 , NB_ENDPOINT0
 , INTERFACE0_CLASS
 , INTERFACE0_SUB_CLASS
 , INTERFACE0_PROTOCOL
 , INTERFACE0_INDEX
 } //9
,
{
// Header Functional Descriptor (marks beginning of the concatenated set of Functional Descriptors)
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x00, // bDescriptorSubtype, Header Functional Descriptor
0x10,0x01, // bcdCDC, CDC specification release number in BCD format (1,1)

// Call Management Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x01, // bDescriptorSubtype, Call Management Functional Descriptor
0x00, // bmCapabilities
0x01, // bDataInterface, Interface used for call management

// Abstract Control Management Functional Descriptor
0x04, // bDescriptorLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x02, // bDescriptorSubtype, Abstract Control Management Functional Descriptor
0x00, // bmCapabilities

// Union Functional Descriptor
0x05, // bFunctionLength, Descriptor size in bytes
0x24, // bDescriptorType, CS_INTERFACE
0x06, // bDescriptorSubtype, Union Functional Descriptor
0x00, // bMasterInterface, The controlling interface for the union (bInterfaceNumber of a Communication or Data Class interface in this configuration)
0x01, // bSlaveInterface0, The controlled interace in the union (bInterfaceNumber of an interface in this configuration)
},
{ sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_1
 , EP_ATTRIBUTES_1
 , Usb_write_word_enum_struc(EP_SIZE_1)
 , EP_INTERVAL_1
 } //7
 ,
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE1_NB
 , ALTERNATE1
 , NB_ENDPOINT1
 , INTERFACE1_CLASS
 , INTERFACE1_SUB_CLASS
 , INTERFACE1_PROTOCOL
 , INTERFACE1_INDEX
 },
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_2
 , EP_ATTRIBUTES_2
 , Usb_write_word_enum_struc(EP_SIZE_2)
 , EP_INTERVAL_2
 },
{ sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_3
 , EP_ATTRIBUTES_3
 , Usb_write_word_enum_struc(EP_SIZE_3)
 , EP_INTERVAL_3
 }
};


/****************** MASS-STORAGE DEVICE DESCRIPTORS **************************/

FLASH S_usb_device_descriptor usb_dev_desc_mass =
{
  sizeof(usb_dev_desc_mass)
, DEVICE_DESCRIPTOR
, Usb_write_word_enum_struc(USB_SPECIFICATION)
, MASS_DEVICE_CLASS
, MASS_DEVICE_SUB_CLASS
, MASS_DEVICE_PROTOCOL
, EP_CONTROL_LENGTH
, Usb_write_word_enum_struc(VENDOR_ID)
, Usb_write_word_enum_struc(MASS_PRODUCT_ID)
, Usb_write_word_enum_struc(RELEASE_NUMBER)
, MAN_INDEX
, PROD_INDEX
, SN_INDEX
, NB_CONFIGURATION
};

// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_mass usb_conf_desc_mass = {
 { sizeof(S_usb_configuration_descriptor)
 , CONFIGURATION_DESCRIPTOR
 , Usb_write_word_enum_struc(sizeof(S_usb_user_configuration_descriptor_mass))
 , MASS_NB_INTERFACE
 , CONF_NB
 , CONF_INDEX
 , CONF_ATTRIBUTES
 , MAX_POWER
 },//9

  /// Mass storage
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , MS_INTERFACE_NB
 , MS_ALTERNATE
 , MS_NB_ENDPOINT
 , MS_INTERFACE_CLASS
 , MS_INTERFACE_SUB_CLASS
 , MS_INTERFACE_PROTOCOL
 , MS_INTERFACE_INDEX
 } //9
,
{ sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , MS_ENDPOINT_NB_1
 , MS_EP_ATTRIBUTES_1
 , Usb_write_word_enum_struc(MS_EP_SIZE_1)
 , MS_EP_INTERVAL_1
 } //7
 ,
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , MS_ENDPOINT_NB_2
 , MS_EP_ATTRIBUTES_2
 , Usb_write_word_enum_struc(MS_EP_SIZE_2)
 , MS_EP_INTERVAL_2
 }
};


/************* COMMON *****************/


                                      // usb_user_manufacturer_string_descriptor
FLASH S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor = {
  sizeof(usb_user_manufacturer_string_descriptor)
, STRING_DESCRIPTOR
, USB_MANUFACTURER_NAME
};


                                      // usb_user_product_string_descriptor

FLASH S_usb_product_string_descriptor usb_user_product_string_descriptor = {
  sizeof(usb_user_product_string_descriptor)
, STRING_DESCRIPTOR
, USB_PRODUCT_NAME
};


                                      // usb_user_serial_number

FLASH S_usb_serial_number usb_user_serial_number = {
  sizeof(usb_user_serial_number)
, STRING_DESCRIPTOR
, USB_SERIAL_NUMBER
};


                                      // usb_user_language_id

FLASH S_usb_language_id usb_user_language_id = {
  sizeof(usb_user_language_id)
, STRING_DESCRIPTOR
, Usb_write_word_enum_struc(LANGUAGE_ID)
};





PGM_VOID_P Usb_get_dev_desc_pointer(void) 
{
	if (usb_mode == rndis_only)
		return &(usb_dev_desc_network.bLength);

	if (usb_mode == rndis_debug)
		return &(usb_dev_desc_composite.bLength);

	return &(usb_dev_desc_mass.bLength);
}


U8 Usb_get_dev_desc_length(void)
{

	if (usb_mode == rndis_only)
		return sizeof(usb_dev_desc_network);

	if (usb_mode == rndis_debug)
		return sizeof(usb_dev_desc_composite);

	return sizeof(usb_dev_desc_mass);
}


PGM_VOID_P  Usb_get_conf_desc_pointer(void) 
{
	if (usb_mode == rndis_only)
		return &(usb_conf_desc_network.cfg.bLength);

	if (usb_mode == rndis_debug)
		return &(usb_conf_desc_composite.cfg.bLength);

	return &(usb_conf_desc_mass.cfg.bLength);
}


U8  Usb_get_conf_desc_length(void)
{

	if (usb_mode == rndis_only)
		return sizeof (usb_conf_desc_network);

	if (usb_mode == rndis_debug)
		return sizeof(usb_conf_desc_composite);

	return sizeof(usb_conf_desc_mass);
}

/** @} */
