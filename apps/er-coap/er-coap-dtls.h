#ifndef COAP_DTLS_H_
#define COAP_DTLS_H_

/* Internal configuration of tinydtls for er-coap-dtls */

#if defined DTLS_CONF_IDENTITY && defined DTLS_CONF_IDENTITY_LENGTH
#define DTLS_IDENTITY DTLS_CONF_IDENTITY
#define DTLS_IDENTITY_LENGTH DTLS_CONF_IDENTITY_LENGTH
#else
#define DTLS_IDENTITY "Client_identity"
#define DTLS_IDENTITY_LENGTH 15
#endif

#if defined DTLS_CONF_PSK_KEY && defined DTLS_CONF_PSK_KEY_LENGTH
#define DTLS_PSK_KEY_VALUE DTLS_CONF_PSK_KEY
#define DTLS_PSK_KEY_VALUE_LENGTH DTLS_CONF_PSK_KEY_LENGTH
#else
#warning "DTLS: Using default secret key !"
#define DTLS_PSK_KEY_VALUE "secretPSK"
#define DTLS_PSK_KEY_VALUE_LENGTH 9
#endif

/* Structure that hold tinydtls callbacks, has type 'dtls_handler_t'. */
#ifndef COAP_DTLS_CALLBACK
#ifdef COAP_DTLS_CONF_CALLBACK
#define COAP_DTLS_CALLBACK COAP_DTLS_CONF_CALLBACK
#else /* COAP_DTLS_CONF_CALLBACK */
#define COAP_DTLS_CALLBACK coap_dtls_callback
#endif /* COAP_DTLS_CALLBACK */

/* Send 'data' to peer defined by session */
int coap_dtls_send_to_peer(struct dtls_context_t *ctx,
                           session_t *session, uint8 *data, size_t len);

/* Read 'data' from peer */
int coap_dtls_read_from_peer(struct dtls_context_t *ctx,
                             session_t *session, uint8 *data, size_t len);
#ifdef DTLS_PSK
/* Retrieve the key for given identity withing this session */
int coap_dtls_get_psk_info(struct dtls_context_t *ctx,
                           const session_t *session,
                           dtls_credentials_type_t type,
                           const unsigned char *id, size_t id_len,
                           unsigned char *result, size_t result_length);
#endif

#endif /* COAP_DTLS_H_ */
