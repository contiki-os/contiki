/*
 * conf_usb_host.h
 *
 * Created: 3/25/2013 10:35:22 AM
 *  Author: ioannisg
 */ 

#ifndef CONF_USB_HOST_H_
#define CONF_USB_HOST_H_

#include "uhi_vendor.h" // At the end of conf_usb_host.h file
#include "uhc.h"

#define USB_HOST_UHI        UHI_VENDOR
/* This is violating the protocol of limiting the 
 * device current to 500mA.
 */
#define USB_HOST_POWER_MAX  650

// We externally set the Arduino to authorize USB to run in High speed Mode.
// This enables the correct configuration of the device endpoints and solves
// possible future issues. XXX

#ifndef USB_HOST_HS_SUPPORT
#define USB_HOST_HS_SUPPORT
#endif

// A lot of discussion on how to set these configuration parameters

//#define UHD_USB_INT_LEVEL	0
#define UHD_BULK_NB_BANK	2

 
#define UHI_VENDOR_CHANGE(dev, b_plug) usb_vendor_change(dev, b_plug)
extern void usb_vendor_change(uhc_device_t* dev, bool b_plug);

#define UHI_VENDOR_VID_PID_LIST {USB_ATHEROS_VID,USB_ATHEROS_PID_MSC},{USB_ATHEROS_VID,USB_ATHEROS_PID_WLAN}

#endif /* CONF_USB_HOST_H_ */