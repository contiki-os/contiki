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
#include "rndis/rndis_task.h"


//_____ M A C R O S ________________________________________________________


#define USB_ETH_NET_FUNC_DESC(MACAddrString,etherStat,maxSegmentSize,nMCFilters,nPowerFilters) \
 { sizeof(S_usb_ethernet_networking_functional_descriptor)	\
 , 0x24, 0x0F	\
, MACAddrString	\
, etherStat	\
, Usb_write_word_enum_struc(maxSegmentSize)	\
, Usb_write_word_enum_struc(nMCFilters)	\
, nPowerFilters	\
}



#define USB_CONFIG_DESC(totalsize,interfaceCount,value,stringIndex,attr,power) \
 { sizeof(S_usb_configuration_descriptor)	\
 , CONFIGURATION_DESCRIPTOR	\
 , Usb_write_word_enum_struc(totalsize) \
 , interfaceCount	\
 , value	\
 , stringIndex	\
 , attr	\
 , power	\
 }

#define USB_ENDPOINT(nb,attr,maxpktsize,interval) \
{ sizeof(S_usb_endpoint_descriptor) \
 , ENDPOINT_DESCRIPTOR \
 , nb \
 , attr \
 , Usb_write_word_enum_struc(maxpktsize) \
 , interval \
 }

#define USB_IAD(firstinterface,interfacecount,funcClass,funcSubClass,funcProto,stringIndex) \
  { \
    sizeof(S_usb_interface_association_descriptor), \
    DSC_TYPE_IAD, \
    firstinterface,   \
    interfacecount,   \
    funcClass,        \
    funcSubClass,     \
    funcProto,        \
	stringIndex			\
  }

#define USB_INTERFACEDESC(nb,altsetting,endpointCount,interfaceClass,interfaceSubClass,interfaceProto,stringIndex) \
 { sizeof(S_usb_interface_descriptor)	\
 , INTERFACE_DESCRIPTOR	\
 , nb	\
 , altsetting	\
 , endpointCount	\
 , interfaceClass	\
 , interfaceSubClass	\
 , interfaceProto	\
 , stringIndex	\
 }


#define FUNC_DESC_HEADER \
0x05, /* bFunctionLength, Descriptor size in bytes*/ \
0x24, /* bDescriptorType, CS_INTERFACE */ \
0x00, /* bDescriptorSubtype, Header Functional Descriptor */ \
0x10,0x01 /* bcdCDC, CDC specification release number in BCD format (1,1) */ 

#define FUNC_DESC_CALL_MANAGEMENT(caps,dataInterface) \
0x05, /* bFunctionLength, Descriptor size in bytes */ \
0x24, /* bDescriptorType, CS_INTERFACE */ \
0x01, /* bDescriptorSubtype, Call Management Functional Descriptor */ \
caps, /* bmCapabilities */ \
dataInterface /* bDataInterface, Interface used for call management */ 

/* Abstract Control Management Functional Descriptor */
#define FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT(caps) \
0x04, /* bFunctionLength, Descriptor size in bytes */ \
0x24, /* bDescriptorType, CS_INTERFACE */ \
0x02, /* bDescriptorSubtype, Call Management Functional Descriptor */ \
caps /* bmCapabilities */

/* Union Functional Descriptor */ \
#define FUNC_DESC_UNION(masterInterface,slaveInterface) \
0x05, /* bFunctionLength, Descriptor size in bytes */ \
0x24, /* bDescriptorType, CS_INTERFACE */ \
0x06, /* bDescriptorSubtype, Union Functional Descriptor */ \
masterInterface, /* bMasterInterface, The controlling interface for the union (bInterfaceNumber of a Communication or Data Class interface in this configuration) */ \
slaveInterface /* bSlaveInterface0, The controlled interace in the union (bInterfaceNumber of an interface in this configuration) */ \

#define RNDIS_INTERFACES	\
USB_INTERFACEDESC(	\
	INTERFACE0_NB,	\
	ALTERNATE0,	\
	NB_ENDPOINT0,	\
	INTERFACE0_CLASS,	\
	INTERFACE0_SUB_CLASS,	\
	INTERFACE0_PROTOCOL,	\
	USB_STRING_INTERFACE_RNDIS	\
),	\
	{	\
		FUNC_DESC_HEADER, \
		FUNC_DESC_CALL_MANAGEMENT(0x00/*caps*/,0x00/*dataInterface*/), \
		FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT(0x00), \
		FUNC_DESC_UNION(0x00,0x01), \
	}, \
	USB_ENDPOINT(ENDPOINT_NB_1,EP_ATTRIBUTES_1,EP_SIZE_1,EP_INTERVAL_1), \
\
USB_INTERFACEDESC(	\
	INTERFACE1_NB,	\
	ALTERNATE1,	\
	NB_ENDPOINT1,	\
	INTERFACE1_CLASS,	\
	INTERFACE1_SUB_CLASS,	\
	INTERFACE1_PROTOCOL,	\
	USB_STRING_NONE	\
),	\
	USB_ENDPOINT(ENDPOINT_NB_2,EP_ATTRIBUTES_2,EP_SIZE_2,EP_INTERVAL_2), \
	USB_ENDPOINT(ENDPOINT_NB_3,EP_ATTRIBUTES_3,EP_SIZE_3,EP_INTERVAL_3)

#define CDC_SERIAL_INTERFACES \
USB_INTERFACEDESC(	\
	INTERFACE2_NB,	\
	ALTERNATE2,	\
	NB_ENDPOINT2,	\
	INTERFACE2_CLASS,	\
	INTERFACE2_SUB_CLASS,	\
	INTERFACE2_PROTOCOL,	\
	USB_STRING_INTERFACE_SERIAL	\
),	\
{	\
		FUNC_DESC_HEADER, \
		FUNC_DESC_CALL_MANAGEMENT(0x03/*caps*/,0x03/*dataInterface*/), \
		FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT(0x02), \
		FUNC_DESC_UNION(0x02,0x03), \
},	\
USB_ENDPOINT(ENDPOINT_NB_4,EP_ATTRIBUTES_4,EP_SIZE_4,EP_INTERVAL_4),	\
USB_INTERFACEDESC(	\
	INTERFACE3_NB,	\
	ALTERNATE3,	\
	NB_ENDPOINT3,	\
	INTERFACE3_CLASS,	\
	INTERFACE3_SUB_CLASS,	\
	INTERFACE3_PROTOCOL,	\
	USB_STRING_NONE	\
),	\
USB_ENDPOINT(ENDPOINT_NB_5,EP_ATTRIBUTES_5,EP_SIZE_5,EP_INTERVAL_5),	\
USB_ENDPOINT(ENDPOINT_NB_6,EP_ATTRIBUTES_6,EP_SIZE_6,EP_INTERVAL_6)

#define CDC_EEM_INTERFACES	\
USB_INTERFACEDESC(	\
	EEM_INTERFACE0_NB,	\
	EEM_ALTERNATE0,	\
	EEM_NB_ENDPOINT0,	\
	EEM_INTERFACE0_CLASS,	\
	EEM_INTERFACE0_SUB_CLASS,	\
	EEM_INTERFACE0_PROTOCOL,	\
	USB_STRING_INTERFACE_EEM	\
),	\
	USB_ENDPOINT(ENDPOINT_NB_1,EP_ATTRIBUTES_1,EP_SIZE_1,EP_INTERVAL_1), \
	USB_ENDPOINT(ENDPOINT_NB_2,EP_ATTRIBUTES_2,EP_SIZE_2,EP_INTERVAL_2)




#if CDC_ECM_USES_INTERRUPT_ENDPOINT
#define CDC_ECM_CONTROL_ENDPOINT_COUNT	1
#define CDC_ECM_CONTROL_ENDPOINT	USB_ENDPOINT(ENDPOINT_NB_1,EP_ATTRIBUTES_1,EP_SIZE_1,EP_INTERVAL_1),
#else
#define CDC_ECM_CONTROL_ENDPOINT_COUNT	0
#define CDC_ECM_CONTROL_ENDPOINT
#endif


#define CDC_ECM_INTERFACES	\
USB_INTERFACEDESC(	\
	ECM_INTERFACE0_NB,	\
	ECM_ALTERNATE0,	\
	CDC_ECM_CONTROL_ENDPOINT_COUNT,	\
	ECM_INTERFACE0_CLASS,	\
	ECM_INTERFACE0_SUB_CLASS,	\
	ECM_INTERFACE0_PROTOCOL,	\
	USB_STRING_INTERFACE_ECM	\
),	\
{	\
		FUNC_DESC_HEADER, \
		FUNC_DESC_UNION(0x00,0x01), \
},	\
	USB_ETH_NET_FUNC_DESC(	\
		USB_STRING_MAC_ADDRESS,	\
		0,	/* Support no stats for now */ \
		USB_ETH_MTU, /* maxSegmentSize */ \
		0, /* nMCFilters*/	\
		0 /*nPowerFilters*/	\
	), \
	CDC_ECM_CONTROL_ENDPOINT \
USB_INTERFACEDESC(	\
	ECM_INTERFACE0_NB+1,	\
	0,	\
	2,	\
	10,	\
	0,	\
	0,	\
	USB_STRING_NONE	\
),	\
	USB_ENDPOINT(ENDPOINT_NB_2,EP_ATTRIBUTES_2,EP_SIZE_2,EP_INTERVAL_2), \
	USB_ENDPOINT(ENDPOINT_NB_3,EP_ATTRIBUTES_3,EP_SIZE_3,EP_INTERVAL_3)

/*
USB_INTERFACEDESC(	\
	ECM_INTERFACE0_NB+1,	\
	0,	\
	0,	\
	10,	\
	0,	\
	0,	\
	USB_STRING_NONE	\
),	\
*/



//_____ D E F I N I T I O N ________________________________________________

/************* COMPOSITE DEVICE DESCRIPTORS (using IAD) **********/
//TODO:Serial port enumeration will prevent falling through to the
//supported network on Mac or Windows. Linux will take either.
//Once Windows loads the RNDIS driver it will use it on a subsequent
//Mac enumeration, and the device will fail to start.
//This doesn't seem to hurt anything but beware, system corruption is
//a possibility.
#if USB_CONF_MACINTOSH
/* Prefer CDC-ECM network enumeration (Macintosh, linux) */
FLASH uint8_t usb_dev_config_order[] = {
	USB_CONFIG_RNDIS, //windows gets networking only (if not here gets serial only)
#if USB_CONF_SERIAL
	USB_CONFIG_ECM_DEBUG, //mac, linux get networking and serial port
#endif
	USB_CONFIG_ECM,
#if USB_CONF_SERIAL
	USB_CONFIG_RNDIS_DEBUG,
#endif
//	USB_CONFIG_RNDIS,
	USB_CONFIG_EEM,
#if USB_CONF_STORAGE
	USB_CONFIG_MS,
#endif
};
#else
/* Prefer RNDIS network enumeration (Windows, linux) */
FLASH uint8_t usb_dev_config_order[] = {

//	USB_CONFIG_ECM, //windows doesnt like this here, will not go on to RNDIS
#if USB_CONF_SERIAL
	USB_CONFIG_RNDIS_DEBUG,
#else
	USB_CONFIG_RNDIS,
#endif
#if 0 //XP may have a problem with these extra configurations
	USB_CONFIG_RNDIS,
#if USB_CONF_SERIAL
	USB_CONFIG_ECM_DEBUG,
#endif
	USB_CONFIG_ECM,
	USB_CONFIG_EEM,
#if USB_CONF_STORAGE
	USB_CONFIG_MS,
#endif
#endif
};
#endif /* USB_CONF_MACINTOSH */

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
, USB_STRING_MAN
, USB_STRING_PRODUCT
, USB_STRING_SERIAL
, (sizeof(usb_dev_config_order)/sizeof(*usb_dev_config_order)) //sizeof(*) is sizeof first element
};


// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_composite usb_conf_desc_composite = {
	USB_CONFIG_DESC(
		sizeof(usb_conf_desc_composite),
		COMPOSITE_NB_INTERFACE,
		USB_CONFIG_RNDIS_DEBUG,
		USB_STRING_CONFIG_COMPOSITE,
		CONF_ATTRIBUTES,
		MAX_POWER
	),

	USB_IAD(
		0x00,	// First interface
		0x02,	// Interface count
		0x02,	// Function Class
		0x02,	// Function Subclass
		0xFF,	// Function Protocol
		USB_STRING_INTERFACE_RNDIS
	),RNDIS_INTERFACES,

	USB_IAD(
		0x02,	// First interface
		0x02,	// Interface count
		0x02,	// Function Class
		0x02,	// Function Subclass
		0x01,	// Function Protocol
		USB_STRING_INTERFACE_SERIAL
	),CDC_SERIAL_INTERFACES
};

/****************** NETWORK-ONLY DEVICE DESCRIPTORS **************************/

/*
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
, USB_STRING_MAN
, USB_STRING_PRODUCT
, USB_STRING_SERIAL
, NB_CONFIGURATION
};
*/

// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_network usb_conf_desc_network = {
	USB_CONFIG_DESC(
		sizeof(usb_conf_desc_network),
		NETWORK_NB_INTERFACE,
		CONF_NB,	// value
		USB_STRING_CONFIG_RNDIS,
		CONF_ATTRIBUTES,
		MAX_POWER
	),
	RNDIS_INTERFACES
};



/************* EEM-ONLY ***************/

// usb_user_device_descriptor
/*
FLASH S_usb_device_descriptor usb_dev_desc_eem =
{
  sizeof(usb_dev_desc_composite)
, DEVICE_DESCRIPTOR
, Usb_write_word_enum_struc(USB_SPECIFICATION)
, EEM_DEVICE_CLASS
, EEM_DEVICE_SUB_CLASS
, EEM_DEVICE_PROTOCOL
, EP_CONTROL_LENGTH
, Usb_write_word_enum_struc(VENDOR_ID)
, Usb_write_word_enum_struc(COMPOSITE_PRODUCT_ID)
, Usb_write_word_enum_struc(RELEASE_NUMBER)
, USB_STRING_MAN
, USB_STRING_PRODUCT
, USB_STRING_SERIAL
, NB_CONFIGURATION
};
*/


// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_eem usb_conf_desc_eem = {
	USB_CONFIG_DESC(
		sizeof(usb_conf_desc_eem),
		EEM_NB_INTERFACE,
		USB_CONFIG_EEM,	// value
		USB_STRING_CONFIG_EEM,
		CONF_ATTRIBUTES,
		MAX_POWER
	),
	CDC_EEM_INTERFACES
};


FLASH S_usb_user_configuration_descriptor_ecm usb_conf_desc_ecm = {
	USB_CONFIG_DESC(
		sizeof(usb_conf_desc_ecm),
		2,	// Interface Count
		USB_CONFIG_ECM,	// value
		USB_STRING_CONFIG_ECM,
		CONF_ATTRIBUTES,
		MAX_POWER
	),
	CDC_ECM_INTERFACES
};

FLASH S_usb_user_configuration_descriptor_ecm_debug usb_conf_desc_ecm_debug = {
	USB_CONFIG_DESC(
		sizeof(usb_conf_desc_ecm_debug),
		4,	// Interface Count
		USB_CONFIG_ECM_DEBUG,	// value
		USB_STRING_CONFIG_ECM_DEBUG,
		CONF_ATTRIBUTES,
		MAX_POWER
	),

	USB_IAD(
		0x00,	// First interface
		0x02,	// Interface count
		0x02,	// Function Class
		0x06,	// Function Subclass
		0x00,	// Function Protocol
		USB_STRING_INTERFACE_ECM
	),CDC_ECM_INTERFACES,

	USB_IAD(
		0x02,	// First interface
		0x02,	// Interface count
		0x02,	// Function Class
		0x02,	// Function Subclass
		0x01,	// Function Protocol
		USB_STRING_INTERFACE_SERIAL
	),CDC_SERIAL_INTERFACES


	
};



/****************** MASS-STORAGE DEVICE DESCRIPTORS **************************/

/*
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
, USB_STRING_MAN
, USB_STRING_PRODUCT
, USB_STRING_SERIAL
, NB_CONFIGURATION
};
*/
#if USB_CONF_STORAGE

// usb_user_configuration_descriptor FS
FLASH S_usb_user_configuration_descriptor_mass usb_conf_desc_mass = {
 { sizeof(S_usb_configuration_descriptor)
 , CONFIGURATION_DESCRIPTOR
 , Usb_write_word_enum_struc(sizeof(S_usb_user_configuration_descriptor_mass))
 , MASS_NB_INTERFACE
 , CONF_NB
 , USB_STRING_CONFIG_MS
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
 , USB_STRING_INTERFACE_MS
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

#endif

/************* COMMON *****************/



                                      // usb_user_language_id

FLASH S_usb_language_id usb_user_language_id = {
  sizeof(usb_user_language_id)
, STRING_DESCRIPTOR
, Usb_write_word_enum_struc(LANGUAGE_ID)
};





PGM_VOID_P Usb_get_dev_desc_pointer(void)
{
#if 1
	return &(usb_dev_desc_composite.bLength);
#else  //these are all the same length
	if (usb_mode == rndis_only)
		return &(usb_dev_desc_network.bLength);

	if (usb_mode == rndis_debug)
		return &(usb_dev_desc_composite.bLength);

	if (usb_mode == eem)
		return &(usb_dev_desc_eem.bLength);

	return &(usb_dev_desc_mass.bLength);
#endif
}


U8 Usb_get_dev_desc_length(void)
{
#if 1 
	return sizeof(usb_dev_desc_composite);
#else  //these are all the same size
	if (usb_mode == rndis_only)
		return sizeof(usb_dev_desc_network);

	if (usb_mode == rndis_debug)
		return sizeof(usb_dev_desc_composite);

	if (usb_mode == eem)
		return sizeof(usb_dev_desc_eem);

	return sizeof(usb_dev_desc_mass);
#endif
}


PGM_VOID_P  Usb_get_conf_desc_pointer(U8 index)
{
	
	switch(pgm_read_byte_near(&usb_dev_config_order[index])) {
		case USB_CONFIG_ECM:
			return &(usb_conf_desc_ecm.cfg.bLength);
		case USB_CONFIG_ECM_DEBUG:
			return &(usb_conf_desc_ecm_debug.cfg.bLength);
		case USB_CONFIG_RNDIS_DEBUG:
			return &(usb_conf_desc_composite.cfg.bLength);
		case USB_CONFIG_RNDIS:
			return &(usb_conf_desc_network.cfg.bLength);
		case USB_CONFIG_EEM:
			return &(usb_conf_desc_eem.cfg.bLength);		
#if USB_CONF_STORAGE
		case USB_CONFIG_MS:
			return &(usb_conf_desc_mass.cfg.bLength);
#endif
		default:
			return 0;
	}
}


U8  Usb_get_conf_desc_length(U8 index)
{
	switch(pgm_read_byte_near(&usb_dev_config_order[index])) {
		case USB_CONFIG_ECM:
			return sizeof(usb_conf_desc_ecm);
		case USB_CONFIG_ECM_DEBUG:
			return sizeof(usb_conf_desc_ecm_debug);
		case USB_CONFIG_RNDIS_DEBUG:
			return sizeof(usb_conf_desc_composite);
		case USB_CONFIG_RNDIS:
			return sizeof (usb_conf_desc_network);
		case USB_CONFIG_EEM:
			return sizeof(usb_conf_desc_eem);
#if USB_CONF_STORAGE
		case USB_CONFIG_MS:
			return sizeof(usb_conf_desc_mass);
#endif
		default:
			return 0;
	}
}

/** @} */
