#ifndef CDC_H_K1Q26ESJOC__
#define CDC_H_K1Q26ESJOC__
#include <usb.h>
/*  Communication Class */
/* Class code */
#define CDC			0x02

/* Interface subclass codes */
#define CDC_RESERVED		0x00
#define DIRECT_LINE_CONTROL_MODEL 0x01
#define ABSTRACT_CONTROL_MODEL	0x02
#define TELEPHONE_CONTROL_MODEL 0x03
#define MULTI_CHANNEL_CONTROL_MODEL 0x04
#define CAPI_CONTROL_MODEL	0x05
#define ETHERNET_NETWORKING_CONTROL_MODEL 0x06
#define ATM_NETWORKING_CONTROL_MODEL 0x07

/* Protocols */
#define V_25TER_PROTOCOL	0x01

/* Requests */
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_COMM_FEATURE	0x02
#define GET_COMM_FEATURE	0x03
#define CLEAR_COMM_FEATURE	0x04

#define SET_AUX_LINE_STATE	0x10
#define SET_HOOK_STATE		0x11
#define PULSE_SETUP		0x12
#define SEND_PULSE		0x13
#define SET_PULSE_TIME		0x14
#define RING_AUX_JACK		0x15

#define SET_LINE_CODING		0x20
#define GET_LINE_CODING		0x21
#define SET_CONTROL_LINE_STATE	0x22
#define SEND_BREAK		0x23

#define SET_RINGER_PARMS	0x30
#define GET_RINGER_PARMS	0x31
#define SET_OPERATION_PARMS	0x32
#define GET_OPERATION_PARMS	0x33
#define SET_LINE_PARMS		0x34
#define GET_LINE_PARMS		0x35
#define DIAL_DIGITS		0x36

#define SET_UNIT_PARAMETER	0x37
#define GET_UNIT_PARAMETER	0x38
#define CLEAR_UNIT_PARAMETER	0x39

#define GET_PROFILE		0x3a

#define SET_ETHERNET_MULTICAST_FILTERS 0x40
#define GET_ETHERNET_MULTICAST_FILTERS 0x41
#define GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER 0x42
#define SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER 0x43
#define GET_ETHERNET_STATISTIC 0x44

#define SET_ATM_D ATA_FORMAT	0x50
#define GET_ATM_DEVICE_STATISTICS 0x51
#define SET_ATM_DEFAULT_VC	0x52
#define GET_ATM_VC_STATISTICS	0x53


/* Notifications */
#define NETWORK_CONNECTION	0x00
#define RESPONSE_AVAILABLE	0x01

#define AUX_JACK_HOOK_STATE	0x08
#define RING_DETECT		0x09

#define SERIAL_STATE		0x20

#define CALL_STATE_CHANGE	0x28
#define LINE_STATE_CHANGE	0x29
#define CONNECTION_SPEED_CHANGE	0x2a

/* Data interface */

/* Class code */
#define CDC_DATA		0x0a

/* Protocols */
#define I_430_PROTOCOL		0x30
#define ISO_IEC_3_1993_PROTOCOL	0x31
#define TRANSPARENT_PROTOCOL	0x32
#define Q_921M_PROTOCOL		0x50
#define Q_921_PROTOCOL		0x51
#define Q_921TM_PROTOCOL       	0x52
#define V_42BIS_PROTOCOL       	0x90
#define Q_931_PROTOCOL       	0x91
#define V_120_PROTOCOL       	0x93
#define CDC_PROTOCOL       	0xfe

/* Descriptor subtypes */

#define CDC_FUNC_DESCR_HEADER	0x00
#define CDC_FUNC_DESCR_CALL_MGMNT 0x01
#define CDC_FUNC_DESCR_ABSTRACT_CTRL_MGMNT 0x02
#define CDC_FUNC_DESCR_DIRECT_LINE_MGMNT 0x03
#define CDC_FUNC_DESCR_RINGER_MGMNT 0x04
#define CDC_FUNC_DESCR_TEL_STATE 0x05
#define CDC_FUNC_DESCR_UNION	0x06
#define CDC_FUNC_DESCR_COUNTRY	0x07
#define CDC_FUNC_DESCR_TEL_MODE	0x08
#define CDC_FUNC_DESCR_USB_TERM	0x09
#define CDC_FUNC_DESCR_NET_TERM	0x0a
#define CDC_FUNC_DESCR_PROTOCOL_UNIT 0x0b
#define CDC_FUNC_DESCR_EXTENSION_UNIT 0x0c
#define CDC_FUNC_DESCR_MULTICH_MGMNT 0x0d
#define CDC_FUNC_DESCR_CAPI_MGMNT 0x0e
#define CDC_FUNC_DESCR_ETHERNET	0x0f
#define CDC_FUNC_DESCR_ATM	0x10



struct usb_cdc_header_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_HEADER subtype */
  Uint16 bcdCDC;		/* Revision of class specification */
} BYTE_ALIGNED;

struct usb_cdc_call_mgmnt_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_CALL_MGMNT subtype */
  Uchar  bmCapabilities;	/* Capabilities */
  Uchar  bDataInterface;	/* Management data interface */
} BYTE_ALIGNED;

struct usb_cdc_abstract_ctrl_mgmnt_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_ABSTRACT_CTRL_MGMNT subtype*/
  Uchar  bmCapabilities;	/* Capabilities */
} BYTE_ALIGNED;

struct usb_cdc_direct_line_mgmnt_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_DIRECT_LINE_MGMNT subtype*/
  Uchar  bmCapabilities;	/* Capabilities */
} BYTE_ALIGNED;

struct usb_cdc_ringer_mgmnt_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_RINGER_MGMNT subtype*/
  Uchar  bRingerVolSteps;	/* Ringer volume steps */
  Uchar  bNumRingerPatterns;	/* Number of ringer patterns supported */
} BYTE_ALIGNED;

struct usb_cdc_tel_mode_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_TEL_MODE subtype*/
  Uchar  bmCapabilities;	/* Capabilities */
} BYTE_ALIGNED;

struct usb_cdc_tel_state_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_TEL_STATE subtype*/
  Uchar  bmCapabilities;	/* Capabilities */
} BYTE_ALIGNED;

struct usb_cdc_union_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_UNION subtype*/
  Uchar  bMasterInterface;	/* Master interface for union */
  Uchar  bSlaveInterface[1];	/* Slave interfaces in union */
} BYTE_ALIGNED;

struct usb_cdc_country_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_COUNTRY subtype*/
  Uchar  iCountryCodeRelDate;	/* Release date for country codes */
  Uint16 wCountryCode[1];	/* Country codes */
} BYTE_ALIGNED;

struct usb_cdc_ethernet_func_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;	/* CS_INTERFACE descriptor type */
  Uchar  bDescriptorSubtype;	/* CDC_FUNC_DESCR_ETHERNET subtype*/
  Uchar  iMACAddress;		/* MAC address string descriptor */
  Uint32 bmEthernetStatistics;	/* Supported statistics */
  Uint16 wMaxSegmentSize;
  Uint16 wNumberMCFilters;	/* Number of multicast filters */
  Uchar  bNumberPowerFilters;	/* Number of wake-up pattern filters */;
} BYTE_ALIGNED;

struct usb_cdc_line_coding
{
  Uint32 dwDTERate;
  Uchar  bCharFormat;
  Uchar  bParityType;
  Uchar  bDataBits;
} BYTE_ALIGNED;

#endif /* CDC_H_K1Q26ESJOC__ */
