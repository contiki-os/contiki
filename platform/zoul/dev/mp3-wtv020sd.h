/*
 * Copyright (c) 2015, Zolertia
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-mp3-wtv020sd MP3 WTV020SD driver compatible with Zoul-based
 *                             platforms
 *
 * Driver to control the MP3 WTV020SD board in MP3 mode (GPIO based) and the
 * 2-line serial mode (CLK/DI).  Loop Mode and Key Modes not implemented.
 * More product information available at:
 * http://avrproject.ru/chasy-budilnik/WTV020SD.pdf
 * An example on how to wire with a sound power amplifier and speakers at
 * http://www.hackster.io/zolertia
 * Based on the Arduino Wtv020sd16p library
 * @{
 *
 * \file
 * Header file for the MP3 WTV020SD driver
 */
/* -------------------------------------------------------------------------- */
#ifndef MP3_WTV020SD_H_
#define MP3_WTV020SD_H_
/* -------------------------------------------------------------------------- */
#include <stdint.h>
/* -------------------------------------------------------------------------- */
#define MP3_WTV020SD_ERROR              -1
#define MP3_WTV020SD_SUCCESS          0x00
#define MP3_WTV020SD_GPIO_MODE        0x01
#define MP3_WTV020SD_LINE_MODE        0x02
#define MP3_WTV020SD_IDLE             0x00
#define MP3_WTV020SD_BUSY             0x0F
/* -------------------------------------------------------------------------- */
#define MP3_WTV020SD_PLAY_PAUSE_VAL   0xFFFE
#define MP3_WTV020SD_STOP_VAL         0xFFFF
#define MP3_WTV020SD_VOLUME_MIN       0xFFF0
#define MP3_WTV020SD_VOLUME_MAX       0xFFF7
/* -------------------------------------------------------------------------- */
#define MP3_USEC_DELAY                1000
#define MP3_USEC_CMD_DELAY            100
#define MP3_USEC_RESET_DELAY          ((MP3_USEC_DELAY) * 30)
/* -------------------------------------------------------------------------- */
#define MP3_TRACK_BASE                0 /* 0000.ad4 */
/* -------------------------------------------------------------------------- */
/**
 * \brief Init function for the MP3 driver
 *
 * Configures the pins required to operate in either driver mode
 *
 * \param mode drive the board using GPIOs or the two-line mode, using
 *        either MP3_WTV020SD_GPIO_MODE or MP3_WTV020SD_LINE_MODE
 * \return MP3_WTV020SD_ERROR if invalid mode selected, otherwise it
 *         will return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_config(uint8_t mode);
/**
 * \brief Function to play a current track
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_gpio_play(void);
/**
 * \brief Function to stop a current track
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_gpio_stop(void);
/**
 * \brief Advances and play the next track, wraps over the playlist
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_gpio_next(void);
/**
 * \brief Get the current status of the device (playing/stopped)
 *
 * \return MP3_WTV020SD_BUSY if a track is playing, otherwise it will
 *         return MP3_WTV020SD_IDLE
 */
int mp3_wtv020sd_busy(void);
/**
 * \brief Trigger a module reset
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_reset(void);
/**
 * \brief Plays the selected track and waits until it stops
 *
 * \param  track forwards and play the selected track, starting from
 *         MP3_TRACK_BASE (0000.ad4) up to MP3_TRACK_BASE + 511
 *         (0511.ad4)
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_sync_play(uint16_t track);
/**
 * \brief Plays the selected track and returns immediately
 *
 * \param  track forwards and play the selected track, starting from
 *         MP3_TRACK_BASE (0000.ad4) up to MP3_TRACK_BASE + 511
 *         (0511.ad4)
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_async_play(uint16_t track);
/**
 * \brief Stops the current track
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_stop(void);
/**
 * \brief Pauses the current track
 *
 * \return MP3_WTV020SD_ERROR if invalid mode used, otherwise it will
 *         return MP3_WTV020SD_SUCCESS
 */
int mp3_wtv020sd_pause(void);

/* -------------------------------------------------------------------------- */
#endif /* ifndef MP3_WTV020SD_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */
