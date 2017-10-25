#include "contiki.h"
#include "contiki-net.h"
#include "er-coap.h"
#include "er-coap-engine.h"
#include "er-coap-dtls.h"

#include "dtls.h"

#include <string.h>

#define DEBUG DEBUG_NONE
#include "dtls_debug.h"

/*---------------------------------------------------------------------------*/

#if defined DTLS_CONF_IDENTITY_HINT && defined DTLS_CONF_IDENTITY_HINT_LENGTH
#define DTLS_IDENTITY_HINT DTLS_CONF_IDENTITY_HINT
#define DTLS_IDENTITY_HINT_LENGTH DTLS_CONF_IDENTITY_HINT_LENGTH
#else
#define DTLS_IDENTITY_HINT "Client_identity"
#define DTLS_IDENTITY_HINT_LENGTH 15
#endif

#if defined DTLS_CONF_PSK_KEY && defined DTLS_CONF_PSK_KEY_LENGTH
#define DTLS_PSK_KEY_VALUE DTLS_CONF_PSK_KEY
#define DTLS_PSK_KEY_VALUE_LENGTH DTLS_CONF_PSK_KEY_LENGTH
#else
#warning "DTLS: Using default secret key !"
#define DTLS_PSK_KEY_VALUE "secretPSK"
#define DTLS_PSK_KEY_VALUE_LENGTH 9
#endif

/*---------------------------------------------------------------------------*/

static int
send_to_peer(struct dtls_context_t *ctx,
             session_t *session, uint8 *data, size_t len);

static int
read_from_peer(struct dtls_context_t *ctx,
               session_t *session, uint8 *data, size_t len);

/*-----------------------------------------------------------------------------------*/
#ifdef DTLS_PSK
/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identiy within this particular
 * session. */
static int
get_psk_info(struct dtls_context_t *ctx, const session_t *session,
         dtls_credentials_type_t type,
         const unsigned char *id, size_t id_len,
         unsigned char *result, size_t result_length) {

  struct keymap_t {
    unsigned char *id;
    size_t id_length;
    unsigned char *key;
    size_t key_length;
  } psk[1] = {
    { (unsigned char *)DTLS_IDENTITY_HINT, DTLS_IDENTITY_HINT_LENGTH, (unsigned char *)DTLS_PSK_KEY_VALUE, DTLS_PSK_KEY_VALUE_LENGTH },
  };
  if (type ==  DTLS_PSK_IDENTITY) {
    if (id_len) {
      dtls_debug("got psk_identity_hint: '%.*s'\n", id_len, id);
    }

    if (result_length < psk[0].id_length) {
      dtls_warn("cannot set psk_identity -- buffer too small\n");
      return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    memcpy(result, psk[0].id, psk[0].id_length);
    return psk[0].id_length;
  } else if (type == DTLS_PSK_KEY) {
    if (id) {
      int i;
      for (i = 0; i < sizeof(psk)/sizeof(struct keymap_t); i++) {
        if (id_len == psk[i].id_length && memcmp(id, psk[i].id, id_len) == 0) {
          if (result_length < psk[i].key_length) {
            dtls_warn("buffer too small for PSK");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
          }

          memcpy(result, psk[i].key, psk[i].key_length);
          return psk[i].key_length;
        }
      }
    }
  } else {
    return 0;
  }

  return dtls_alert_fatal_create(DTLS_ALERT_DECRYPT_ERROR);
}
#endif
/*-----------------------------------------------------------------------------------*/
context_t *
coap_init_communication_layer(uint16_t port)
{
  static dtls_handler_t cb = {
    .write = send_to_peer,
    .read  = read_from_peer,
    .event = NULL,
#ifdef DTLS_PSK
    .get_psk_info = get_psk_info,
#endif
#ifdef DTLS_ECC
    .get_ecdsa_key = NULL,
    .verify_ecdsa_key = NULL,
#endif
  };
  context_t * ctx;

  struct uip_udp_conn *server_conn = udp_new(NULL, 0, NULL);
  udp_bind(server_conn, port);

  dtls_set_log_level(DTLS_LOG_DEBUG);

  ctx = dtls_new_context(server_conn);
  if(ctx) {
    dtls_set_handler(ctx, &cb);
  }
  /* new connection with remote host */
  printf("COAP-DTLS listening on port %u\n", uip_ntohs(server_conn->lport));
  return ctx;
}
/*-----------------------------------------------------------------------------------*/
static int
send_to_peer(struct dtls_context_t *ctx,
             session_t *session, uint8 *data, size_t len)
{

  struct uip_udp_conn *conn = (struct uip_udp_conn *)dtls_get_app_data(ctx);

  uip_ipaddr_copy(&conn->ripaddr, &session->addr);
  conn->rport = session->port;

  uip_udp_packet_send(conn, data, len);

  /* Restore server connection to allow data from any node */
  memset(&conn->ripaddr, 0, sizeof(conn->ripaddr));
  memset(&conn->rport, 0, sizeof(conn->rport));

  return len;
}
/*-----------------------------------------------------------------------------------*/
void
coap_send_message(context_t * ctx, uip_ipaddr_t *addr, uint16_t port, uint8_t *data, uint16_t length)
{
  session_t session;

  dtls_session_init(&session);
  uip_ipaddr_copy(&session.addr, addr);
  session.port = port;

  dtls_write(ctx, &session, data, length);
}
/*-----------------------------------------------------------------------------------*/
static int
read_from_peer(struct dtls_context_t *ctx,
               session_t *session, uint8 *data, size_t len)
{
  uip_len = len;
  memmove(uip_appdata, data, len);
  coap_receive(ctx);
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
coap_handle_receive(context_t *ctx)
{
  session_t session;

  if(uip_newdata()) {
    dtls_session_init(&session);
    uip_ipaddr_copy(&session.addr, &UIP_IP_BUF->srcipaddr);
    session.port = UIP_UDP_BUF->srcport;

    dtls_handle_message(ctx, &session, uip_appdata, uip_datalen());
  }
}
