#include "dev/nullradio.h"


/*---------------------------------------------------------------------------*/
static int
nullradio_init(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_prepare(const void *payload, unsigned short payload_len)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_transmit(unsigned short transmit_len)
{
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_send(const void *payload, unsigned short payload_len)
{
  nullradio_prepare(payload, payload_len);
  return nullradio_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
nullradio_read(void *buf, unsigned short buf_len)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_channel_clear(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_pending_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_on(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
nullradio_off(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
#if RADIO_CONF_EXTENDED_API
static radio_conf_result_t
nullradio_get_config_const(radio_const_t cst_id, void *value)
{
  return RADIO_CONF_UNAVAILABLE_CONST;
}
/*---------------------------------------------------------------------------*/
static radio_conf_result_t
nullradio_set_param(radio_param_t param_id, void *value)
{
  return RADIO_CONF_UNAVAILABLE_PARAM;
}
/*---------------------------------------------------------------------------*/
static radio_conf_result_t
nullradio_get_param(radio_param_t param_id, void *value)
{
  return RADIO_CONF_UNAVAILABLE_PARAM;
}
#endif /* RADIO_CONF_EXTENDED_API */
/*---------------------------------------------------------------------------*/
const struct radio_driver nullradio_driver =
  {
    nullradio_init,
    nullradio_prepare,
    nullradio_transmit,
    nullradio_send,
    nullradio_read,
    nullradio_channel_clear,
    nullradio_receiving_packet,
    nullradio_pending_packet,
    nullradio_on,
    nullradio_off,
#if RADIO_CONF_EXTENDED_API
    nullradio_get_config_const,
    nullradio_set_param,
    nullradio_get_param,
#endif /* RADIO_CONF_EXTENDED_API */
  };
/*---------------------------------------------------------------------------*/
