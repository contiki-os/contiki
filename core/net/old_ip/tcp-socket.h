/*
 * Copyright (c) 2012-2014, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "uip.h"

struct tcp_socket;

typedef enum {
  TCP_SOCKET_CONNECTED,
  TCP_SOCKET_CLOSED,
  TCP_SOCKET_TIMEDOUT,
  TCP_SOCKET_ABORTED,
  TCP_SOCKET_DATA_SENT
} tcp_socket_event_t;

/**
 * \brief      TCP data callback function
 * \param s    A pointer to a TCP socket
 * \param ptr  A user-defined pointer
 * \param input_data_ptr A pointer to the incoming data
 * \param input_data_len The length of the incoming data
 * \return     The function should return the number of bytes to leave in the input buffer
 *
 *             The TCP socket input callback function gets
 *             called whenever there is new data on the socket. The
 *             function can choose to either consume the data
 *             directly, or leave it in the buffer for later. The
 *             function must return the amount of data to leave in the
 *             buffer. I.e., if the callback function consumes all
 *             incoming data, it should return 0.
 */
typedef int (* tcp_socket_data_callback_t)(struct tcp_socket *s,
                                           void *ptr,
                                           const uint8_t *input_data_ptr,
                                           int input_data_len);


/**
 * \brief      TCP event callback function
 * \param s    A pointer to a TCP socket
 * \param ptr  A user-defined pointer
 * \param event The event number
 *
 *             The TCP socket event callback function gets
 *             called whenever there is an event on a socket, such as
 *             the socket getting connected or closed.
 */
typedef void (* tcp_socket_event_callback_t)(struct tcp_socket *s,
                                             void *ptr,
                                             tcp_socket_event_t event);

struct tcp_socket {
  struct tcp_socket *next;

  tcp_socket_data_callback_t input_callback;
  tcp_socket_event_callback_t event_callback;
  void *ptr;

  struct process *p;

  uint8_t *input_data_ptr;
  uint8_t *output_data_ptr;

  uint16_t input_data_maxlen;
  uint16_t input_data_len;
  uint16_t output_data_maxlen;
  uint16_t output_data_len;
  uint16_t output_data_send_nxt;
  uint16_t output_senddata_len;
  uint16_t output_data_max_seg;

  uint8_t flags;
  uint16_t listen_port;
  struct uip_conn *c;
};

enum {
  TCP_SOCKET_FLAGS_NONE      = 0x00,
  TCP_SOCKET_FLAGS_LISTENING = 0x01,
  TCP_SOCKET_FLAGS_CLOSING   = 0x02,
};

/**
 * \brief      Register a TCP socket
 * \param s    A pointer to a TCP socket
 * \param ptr  A user-defined pointer that will be sent to callbacks for this socket
 * \param input_databuf A pointer to a memory area this socket will use for input data
 * \param input_databuf_len The size of the input data buffer
 * \param output_databuf A pointer to a memory area this socket will use for outgoing data
 * \param output_databuf_len The size of the output data buffer
 * \param data_callback A pointer to the data callback function for this socket
 * \param event_callback A pointer to the event callback function for this socket
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function registers a TCP socket. The function sets
 *             up the output and input buffers for the socket and
 *             callback pointers.
 *
 *             TCP sockets use input and output buffers for incoming
 *             and outgoing data. The memory for these buffers must be
 *             allocated by the caller. The size of the buffers
 *             determine the amount of data that can be received and
 *             sent, and the principle is that the application that
 *             sets up the TCP socket will know roughly how large
 *             these buffers should be. The rule of thumb is that the
 *             input buffer should be large enough to hold the largest
 *             application layer message that the application will
 *             receive and the output buffer should be large enough to
 *             hold the largest application layer message the
 *             application will send.
 *
 *             TCP throttles incoming data so that if the input buffer
 *             is filled, the connection will halt until the
 *             application has read out the data from the input
 *             buffer.
 *
 */
int tcp_socket_register(struct tcp_socket *s, void *ptr,
                         uint8_t *input_databuf, int input_databuf_len,
                         uint8_t *output_databuf, int output_databuf_len,
                         tcp_socket_data_callback_t data_callback,
                         tcp_socket_event_callback_t event_callback);

/**
 * \brief      Connect a TCP socket to a remote host
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \param ipaddr The IP address of the remote host
 * \param port The TCP port number, in host byte order, of the remote host
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function connects a TCP socket to a remote host.
 *
 *             When the socket has connected, the event callback will
 *             get called with the TCP_SOCKET_CONNECTED event. If the
 *             remote host does not accept the connection, the
 *             TCP_SOCKET_ABORTED will be sent to the callback. If the
 *             connection times out before conecting to the remote
 *             host, the TCP_SOCKET_TIMEDOUT event is sent to the
 *             callback.
 *
 */
int tcp_socket_connect(struct tcp_socket *s,
                       const uip_ipaddr_t *ipaddr,
                       uint16_t port);

/**
 * \brief      Start listening on a specific port
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \param port The TCP port number, in host byte order, of the remote host
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function causes the TCP socket to start listening
 *             on the given TCP port.
 *
 *             Several sockets can listen on the same port. If a
 *             remote host connects to the port, one of the listening
 *             sockets will get connected and the event callback will
 *             be called with the TCP_SOCKET_CONNECTED event. When the
 *             connection closes, the socket will go back to listening
 *             for new connections.
 *
 */
int tcp_socket_listen(struct tcp_socket *s,
                      uint16_t port);

/**
 * \brief      Stop listening for new connections
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function causes a listening TCP socket to stop
 *             listen. The socket must previously been put into listen
 *             mode with tcp_socket_listen().
 *
 */
int tcp_socket_unlisten(struct tcp_socket *s);

/**
 * \brief      Send data on a connected TCP socket
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \param dataptr A pointer to the data to be sent
 * \param datalen The length of the data to be sent
 * \retval -1  If an error occurs
 * \return     The number of bytes that were successfully sent
 *
 *             This function sends data over a connected TCP
 *             socket. The data is placed in the output buffer and
 *             sent to the remote host as soon as possiblce. When the
 *             data has been acknowledged by the remote host, the
 *             event callback is sent with the TCP_SOCKET_DATA_SENT
 *             event.
 */
int tcp_socket_send(struct tcp_socket *s,
                    const uint8_t *dataptr,
                    int datalen);

/**
 * \brief      Send a string on a connected TCP socket
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \param strptr A pointer to the string to be sent
 * \retval -1  If an error occurs
 * \return     The number of bytes that were successfully sent
 *
 *             This is a convenience function for sending strings on a
 *             TCP socket. The function calls tcp_socket_send() to
 *             send the string.
 */
int tcp_socket_send_str(struct tcp_socket *s,
                        const char *strptr);

/**
 * \brief      Close a connected TCP socket
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function closes a connected TCP socket. When the
 *             socket has been successfully closed, the event callback
 *             is called with the TCP_SOCKET_CLOSED event.
 *
 */
int tcp_socket_close(struct tcp_socket *s);

/**
 * \brief      Unregister a registered socket
 * \param s    A pointer to a TCP socket that must have been previously registered with tcp_socket_register()
 * \retval -1  If an error occurs
 * \retval 1   If the operation succeeds.
 *
 *             This function unregisters a previously registered
 *             socket. This must be done if the process will be
 *             unloaded from memory. If the TCP socket is connected,
 *             the connection will be reset.
 *
 */
int tcp_socket_unregister(struct tcp_socket *s);

/**
 * \brief      The maximum amount of data that could currently be sent
 * \param s    A pointer to a TCP socket
 * \return     The number of bytes available in the output buffer
 *
 *             This function queries the TCP socket and returns the
 *             number of bytes available in the output buffer. This
 *             function is used before calling tcp_socket_send() to
 *             ensure that one application level message can be held
 *             in the output buffer.
 *
 */
int tcp_socket_max_sendlen(struct tcp_socket *s);

/**
 * \brief      The number of bytes waiting to be sent
 * \param s    A pointer to a TCP socket
 * \return     The number of bytes that have not yet been acknowledged by the receiver.
 *
 *             This function queries the TCP socket and returns the
 *             number of bytes that are currently not yet known to
 *             have been successfully received by the receiver.
 *
 */
int tcp_socket_queuelen(struct tcp_socket *s);

#endif /* TCP_SOCKET_H */
