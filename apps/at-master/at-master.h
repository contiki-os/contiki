/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 *
 */
#ifndef AT_MASTER_H_
#define AT_MASTER_H_
#include "contiki.h"
/*---------------------------------------------------------------------------*/
#define AT_DEFAULT_RESPONSE_OK    "\r\nOK\r\n"
#define AT_DEFAULT_RESPONSE_ERROR "\r\nERROR\r\n"
/*---------------------------------------------------------------------------*/
#define AT_RESPONSE(x) at_send((x), (strlen(x)))
/*---------------------------------------------------------------------------*/
extern process_event_t at_cmd_received_event;
struct at_cmd;
/*---------------------------------------------------------------------------*/
typedef enum {
  AT_STATUS_OK,
  AT_STATUS_ERROR,
  AT_STATUS_INVALID_ARGS_ERROR,
} at_status_t;
/*---------------------------------------------------------------------------*/
/**
 * \brief AT    initialization
 * \param uart  selects which UART to use
 *
 * The AT driver invokes this function upon registering a command, this will
 * wait for the serial_line_event_message event
 */
void at_init(uint8_t uart);
/*---------------------------------------------------------------------------*/
/**
 * \brief AT    initialization
 * \param uart  selects which UART to use
 *
 * The AT driver invokes this function upon registering a command, this will
 * wait for the serial_line_event_message event
 */
uint8_t at_send(char *s, uint8_t len);
/*---------------------------------------------------------------------------*/
/**
 * \brief          AT event callback
 * \param cmd      A pointer to the AT command placeholder
 * \param len      Lenght of the received data (including the AT command header)
 * \param data     A user-defined pointer
 *
 * The AT event callback function gets called whenever there is an
 * event on an incoming AT command
 */
typedef void (*at_event_callback_t)(struct at_cmd *cmd,
                                    uint8_t len,
                                    char *data);
/*---------------------------------------------------------------------------*/
struct at_cmd {
  struct at_cmd *next;
  const char *cmd_header;
  uint8_t cmd_hdr_len;
  uint8_t cmd_max_len;
  at_event_callback_t event_callback;
  struct process *app_process;
};
/*---------------------------------------------------------------------------*/
/**
 * \brief                 Registers the callback to return an AT command
 * \param cmd             A pointer to the CMD placeholder
 * \param cmd_hdr         String to compare when an AT command is received
 * \param cmd_len         Lenght of cmd_hdr
 * \param event_callback  Callback function to handle the AT command
 * \return                AT_STATUS_OK or AT_STATUS_INVALID_ARGS_ERROR
 *
 * Register the commands to search for when a valid AT frame has been received
 */
at_status_t at_register(struct at_cmd *cmd,
                        struct process *app_process,
                        const char *cmd_hdr,
                        const uint8_t cmd_hdr_len,
                        const uint8_t cmd_max_len,
                        at_event_callback_t event_callback);
/*---------------------------------------------------------------------------*/
struct at_cmd *at_list(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief                 Registers the callback to return an AT command
 * \param cmd             A pointer to the CMD placeholder
 * \param cmd_hdr         String to compare when an AT command is received
 * \param cmd_len         Lenght of cmd_hdr
 * \param event_callback  Callback function to handle the AT command
 * \return                AT_STATUS_OK or AT_STATUS_INVALID_ARGS_ERROR
 *
 * Register the commands to search for when a valid AT frame has been received
 */
at_status_t at_register(struct at_cmd *cmd,
                        struct process *app_process,
                        const char *cmd_hdr,
                        const uint8_t cmd_hdr_len,
                        const uint8_t cmd_max_len,
                        at_event_callback_t event_callback);
#endif /* AT_MASTER_H_ */
