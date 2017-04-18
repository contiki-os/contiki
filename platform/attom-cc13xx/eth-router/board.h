/*
 * Copyright (c) 2016, Atto Engenharia de Sistemas - http://attosistemas.com.br/
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
/*---------------------------------------------------------------------------*/
/** \addtogroup cc13xx-attom-tag
 * @{
 *
 * \defgroup cc13xx-attom-router-peripherals Peripherals for the
 * AttoM Router + CC1310
 *
 * Defines related to the AttoM Ethernet Router Board with a TI CC1310.
 *
 * This file is not meant to be modified by the user.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the AttoM
 * Board with a TI CC1310.
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_
/*---------------------------------------------------------------------------*/
#include "board-peripherals.h"
/*---------------------------------------------------------------------------*/
/**
 * \name ENC28J60 SPI ETHERNET IOID mapping
 *
 * Those values are not meant to be modified by the user, for now using
 * same LCD configuration
 * @{
 */

#define BOARD_IOID_SPI_ETHERNET_CS       IOID_14
#define BOARD_IOID_SPI_ETHERNET_RST      IOID_5
#define BOARD_IOID_SPI_ETHERNET_MISO     BOARD_IOID_SPI_MISO
#define BOARD_IOID_SPI_ETHERNET_MOSI     BOARD_IOID_SPI_MOSI
#define BOARD_IOID_SPI_ETHERNET_SCK      BOARD_IOID_SPI_SCK
#define BOARD_SPI_ETHERNET_CS            (1 << BOARD_IOID_SPI_ETHERNET_CS)
#define BOARD_SPI_ETHERNET_RST           (1 << BOARD_IOID_SPI_ETHERNET_RST)
#define BOARD_SPI_ETHERNET_MISO          BOARD_SPI_MISO
#define BOARD_SPI_ETHERNET_MOSI          BOARD_SPI_MOSI
#define BOARD_SPI_ETHERNET_SCK           BOARD_SPI_SCK
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "Attom Ethernet Gateway CC1310"
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* BOARD_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
