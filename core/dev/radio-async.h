/*
 * Copyright (c) 2015, Scanimetrics - http://www.scanimetrics.com
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

/**
 * \file
 *         Header file for the asynchronous radio API
 * \author
 *         Billy Kozak <bkozak@scanimetrics.com>
 */

/**
 * \addtogroup dev
 * @{
 */

/**
 * \defgroup radio-async Asynchronous Radio API
 *
 * The radio-async API module defines a set of functions that a radio device
 * driver must implement for asyncronous operation.
 *
 * @{
 */
/*---------------------------------------------------------------------------*/
#ifndef __SDCC /* This header fails to build with this compiler */
/*---------------------------------------------------------------------------*/
#ifndef RADIO_ASYNC_H_
#define RADIO_ASYNC_H_

#include "contiki-conf.h"
#include "dev/radio.h"
#include "sys/rtimer.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_USE_RFASYNC
#include "radio-async-arch.h"

#ifndef RFASYNC_RX_STABLE_TIME
#error RFASYNC_RX_STABLE_TIME is a required driver definition
#endif

#ifndef RFASYNC_OFF_TIME
#error RFASYNC_OFF_TIME is a required driver definition
#endif

#ifndef RFASYNC_ON_TIME
#error RFASYNC_ON_TIME is a required driver definition
#endif

#endif /* NETSTACK_CONF_USE_RFASYNC */
/*---------------------------------------------------------------------------*/
/** Return code from radio-async driver functions */
enum radio_async_result {
  /** The command completed ok, or the calback function will later run */
  RADIO_ASYNC_RESULT_OK,
  /** The command completed ok, but needs an off -> on cycle to take effect */
  RADIO_ASYNC_RESULT_REQUIRE_CYCLE,
  /** The requested function is not supported */
  RADIO_ASYNC_RESTULT_NOT_SUPPORTED,
  /** Some parameter value is invalid */
  RADIO_ASYNC_RESULT_INVALID_VALUE,
  /** The radio is already in use; if there was a callback it will not run */
  RADIO_ASYNC_RESULT_IN_USE,
  /** There was a generic error; if there was a callback it will not run */
  RADIO_ASYNC_RESULT_ERROR
};

/** Return code from radio-async driver functions */
enum radio_async_done {
  /** The command was succesfully run */
  RADIO_ASYNC_DONE_OK,
  /** There was a generic error; if there was a callback it will not run */
  RADIO_ASYNC_DONE_ERR,
  /** The command completed succesfully with positive result */
  RADIO_ASYNC_DONE_POSITIVE,
  /** The command completed succesfully with negative result */
  RADIO_ASYNC_DONE_NEGATIVE,
  /** The command completed succesfully but there was an rf collision */
  RADIO_ASYNC_DONE_COLLISION,
  /** The radio tx completed succesfully but there was no MAC level ACK */
  RADIO_ASYNC_DONE_NOACK
};

/**
 * \brief structure containing info on a finished radio command
 *
 * passed into callback on completion of a command.
 */
struct radio_async_cb_info {
  enum radio_async_done status;
  rtimer_clock_t time;
};

/**
 * \brief radio callback function pointer
 *
 * Called on completion of an asynchronous radio command
 * \param status return code indicating the result of the command
 * \param time the time at which the command completed
 * \param ptr user supplied argument passed in to the driver function
 */
typedef void (*radio_cb_func)(struct radio_async_cb_info info, void *ptr);

/**
 * \brief The structure of a asyncronous radio device driver
 *
 * The asyncronous driver interface offers a way to send commands to the radio
 * without having to wait until the radio is done with the command.
 *
 * Unlike the original driver interface, it is not possible to call any driver
 * function in any thread at any time; most functions in this interface will
 * return RADIO_ASYNC_IN_USE if the driver is already in use. It is the
 * responsibility of the calling code to properly coordinate multiple-access
 * to this driver.
 *
 */
struct radio_async_driver {

  /**
   * \brief Initilize the radio & driver
   *
   * Must call this before calling any other radio function. Note that, unlike
   * the original driver interface, this function never turns the radio on.
   *
   * \return RADIO_ASYNC_RESULT_OK on success
   */
  enum radio_async_result (*init)(void);

  /**
   * \brief Sets up reciever callback functions
   *
   * This function must be called with valid function pointers before turning
   * on the radio for the first time. The async radio drivers will run
   * ack_cb when an ack is detected (with sequence number given in call to
   * start_ack_detect) and rx_cb every time that a frame is received by the
   * radio.
   *
   * \param ack_cb a callback which can be used to detect ack packets.
   * \param rx_cb a callback run when rx detected
   */
  void (*set_receiver_callbacks)(void (*ack_cb)(uint8_t seqno),
                                 void (*rx_cb)(bool pending));

  /**
   * \brief Prepare the radio with a packet to be sent.
   *
   * \param payload packet to send
   * \param payload_len size of data in payload
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*prepare)(const void *payload, uint16_t len);

  /**
   * \brief Send the packet that has previously been prepared.
   *
   * The callback may recieve '_DONE_NOACK or '_DONE_COLLISION.
   *
   * \param cb callback which will run on command completion. The status code
   * is an enum radio_tx_result
   * \param ptr ptr to be passed into the callback function when run
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*transmit)(radio_cb_func cb, void *ptr);

  /**
   * \brief Prepare & transmit a packet.
   *
   * The callback may recieve '_DONE_NOACK or '_DONE_COLLISION.
   *
   * \param payload packet data to send
   * \param len length of data to send
   * \param cb callback to run when sending is complete
   * \param ptr arbitrary data which will be passed back to cb
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*send)(const void *payload, uint16_t len,
                                  radio_cb_func cb, void *ptr);

  /**
   * \brief  Read a received packet into a buffer.
   * \param buf buffer to read data into
   * \param but_len size of the buffer given
   * \return number of bytes actually read
   */
  int (*read)(void *buf, uint16_t buf_len);

  /**
   * \brief Perform a Clear-Channel Assessment (CCA) to find out if there is
   * a packet in the air or not.
   *
   * The callback function is given RADIO_ASYNC_DONE_POSITIVE if the there is
   * energy in the channel, and RADIO_ASYNC_DONE_NEGATIVE if clear.
   *
   * \param cb callback function to run on completion
   * \param ptr  arbitrary data which will be passed back to cb
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*channel_clear)(radio_cb_func cb, void *ptr);

  /**
   * \brief Check if the radio driver is currently receiving a packet
   *
   * The callback function is given RADIO_ASYNC_DONE_POSITIVE if the there is
   * a packet in the air, and RADIO_ASYNC_DONE_NEGATIVE if no packet
   *
   * \param cb callback function to run on completion
   * \param ptr  arbitrary data which will be passed back to cb
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*receiving_packet)(radio_cb_func cb, void *ptr);

  /**
   * \brief Check if the radio driver has just received a packet
   *
   * \return true iff a packet is pending
   */
  int (*pending_packet)(void);

  /**
   * \brief Turn the radio on.
   *
   * \param cb callback function wich runs on completion
   * \param ptr  arbitrary data which will be passed back to cb
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*on)(radio_cb_func cb, void *ptr);

  /**
   * \brief Turn the radio off.
   *
   * \param cb callback function wich runs on completion
   * \param ptr  arbitrary data which will be passed back to cb
   * \return an enum radio_async_result with typical meaning
   */
  enum radio_async_result (*off)(radio_cb_func cb, void *ptr);

  /**
   * \brief installs an ack detection handler and enables ack detection
   *
   * After calling, on detection of an ACK with the given sequence number, the
   * radio driver will automatically call the radio ACK callback. The ACK packet
   * may still propagate to the NETSTACK_RDC.input function.
   *
   * After triggering once, the callback will not run again until this function
   * is called again.
   *
   * This function must be called prior to calling transmit on a given packet
   * or it is not gauranteed to work.
   *
   * \param seqno the sequence number to watch for
   * \return RADIO_ASYNC_DONE_OK on success
   */
  enum radio_async_result (*start_ack_detect)(uint8_t seqno);

  /**
   * \brief stops the active ack detection handler from triggering
   *
   * The detection handler will not be called again until start_ack_detect is
   * called. Note that, if called from an ISR, it is possible that the ack
   * callback still runs or is already running.
   */
  enum radio_async_result (*stop_ack_detect)(void);

  /**
   * \brief get a radio parameter value.
   */
  enum radio_async_result (*get_value)(radio_param_t par, radio_value_t *val);

  /**
   * \brief Set a radio parameter value.
   * \param par parameter code to try and change
   * \param val value to set parameter to
   * \return RADIO_ASYNC_RESULT_OK or RADIO_ASYNC_RESULT_REQUIRE_CYCLE on
   * success (in the latter case an off() and on() are required for the
   * change to take effect).
   */
  enum radio_async_result (*set_value)(radio_param_t par, radio_value_t val);

  /**
   * Get a radio parameter object. The argument 'dest' must point to a
   * memory area of at least 'size' bytes, and this memory area will
   * contain the parameter object if the function succeeds.
   */
  enum radio_async_result (*get_object)(radio_param_t param, void *dest,
                                        size_t size);
  /**
   * \brief Set a radio parameter object.
   * \param par parameter code to try and change
   * \param src pointer value to set parameter to
   * \param size the length of data pointed to by src
   * \return RADIO_ASYNC_RESULT_OK or RADIO_ASYNC_RESULT_REQUIRE_CYCLE on
   * success (in the latter case an off() then on() are required for the
   * change to take effect).
   */
  enum radio_async_result (*set_object)(radio_param_t param, const void *src,
                                        size_t size);
};
#endif /* RADIO_ASYNC_H_ */
#endif /* __SDCC */
/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
