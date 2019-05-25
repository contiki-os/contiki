# WolfSSL module for Contiki OS

This module provides SSL support via WolfSSL.

WolfSSL is Copyright (c) by WolfSSL Inc.

wolfSSL (formerly known as CyaSSL) and wolfCrypt are either licensed for use
under the GPLv2 or a standard commercial license. For our users who cannot use
wolfSSL under GPLv2, a commercial license to wolfSSL and wolfCrypt is available.
Please contact wolfSSL Inc. directly at:

Email: licensing@wolfssl.com
Phone: +1 425 245-8247

More information can be found on the wolfSSL website at www.wolfssl.com.

## Contiki TLS/DTLS API

Specific calls have been create to initialize and connect TLS and DTLS sockets
on top of Contiki's uIP sockets. The API provided in this module is described below.

### SSL Session object

Secure sessions can be created on top of TCP or UDP sockets. For convenience, the 
data type `uip_wolfssl_ctx` is used to keep track of the ssl session and the transport socket
associated to it.

### TLS

To create a TLS session from within a process thread, the following shortcut functions
are available:


```C
    uip_wolfssl_ctx *tls_socket_register(WOLFSSL_METHOD* method)
```

Create a new TLS session, and associates it to a newly created TCP 
socket in uIP. In case of success, it returns a new pointer to a `uip_wolfssl_ctx` object,
keeping track of the TLS session and the associated socket.

`method` is the desired SSL/TLS protocol method used to create the SSL context,
`p` is the invoking process pointer, so that event-driven input/output can be 
configured for the associated socket.



```C
    int tls_socket_accept(uip_wolfssl_ctx *sk)
```

Replace the current listening socket associated to the SSL/TLS session `sk`
with the new connected socket, and initiate the TLS handshake from the server side.
Returns 0 on success, -1 in case of error.




```C
    int tls_socket_connect(uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port)
```

Initiate a connection to the server, which is expected to be listening at address 
`addr`:`port`. Once the TCP socket is connected, a the TLS handshake is initiated
on the client side. 
Returns 0 on success, -1 in case of error.


### DTLS



```C
    uip_wolfssl_ctx *dtls_socket_register(WOLFSSL_METHOD* method);
```

```C
    void dtls_set_endpoint(uip_wolfssl_ctx *sk, const uip_ipaddr_t *addr, uint16_t port);
```
    








