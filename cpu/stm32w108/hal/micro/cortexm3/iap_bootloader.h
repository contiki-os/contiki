/** @file iap_bootloader.h
 * @brief IAP bootloader specific definitions
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

/**
 * @brief Application start address
 */
#define IAP_BOOTLOADER_APPLICATION_ADDRESS   0x08003000

/**
 * @brief Radio PAN ID for OTA bootloader
 */
#define IAP_BOOTLOADER_PAN_ID 0xb00b

/**
 * @brief Signature used for switch to application
 */
#define IAP_BOOTLOADER_APP_SWITCH_SIGNATURE 0xb001204d

/**
 * @brief Radio default channel for OTA bootloader
 */
#define IAP_BOOTLOADER_DEFAULT_CHANNEL 0x0F

/**
 * @brief IAP bootloader uart mode 
 */
#define IAP_BOOTLOADER_MODE_UART 0

/**
 * @brief IAP bootloader OTA mode 
 */
#define IAP_BOOTLOADER_MODE_OTA 1
