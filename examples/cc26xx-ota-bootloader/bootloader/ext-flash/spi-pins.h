#ifndef SPI_PINS_H_
#define SPI_PINS_H_
/**
 *    How is the ext-flash hardware connected to your board?
 *    Enter pin definitions here.
 */
#include "ioc.h"

/**
 *  For srf06 board:
 */
 /*
#define BOARD_IOID_FLASH_CS       IOID_14
#define BOARD_IOID_SPI_CLK_FLASH  IOID_10
#define BOARD_IOID_SPI_MOSI       IOID_9
#define BOARD_IOID_SPI_MISO       IOID_8
*/

/**
 *  For Sensortag:
 */
#define BOARD_IOID_FLASH_CS       IOID_14
#define BOARD_IOID_SPI_CLK_FLASH  IOID_17
#define BOARD_IOID_SPI_MOSI       IOID_19
#define BOARD_IOID_SPI_MISO       IOID_18

/**
 *  For Launchpad:
 */
/*
#define BOARD_IOID_FLASH_CS       IOID_20
#define BOARD_IOID_SPI_CLK_FLASH  IOID_10
#define BOARD_IOID_SPI_MOSI       IOID_9
#define BOARD_IOID_SPI_MISO       IOID_8
*/
#endif
