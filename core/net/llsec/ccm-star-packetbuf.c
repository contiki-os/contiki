/**
 * \file
 *         CCM* convenience functions for LLSEC use
 * \author
 *         Justin King-Lacroix <justin.kinglacroix@gmail.com>
 */

#include "lib/ccm-star.h"
#include "net/packetbuf.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
void ccm_star_mic_packetbuf(const uint8_t *extended_source_address,
    uint8_t *result,
    uint8_t mic_len)
{
  uint8_t *dataptr = packetbuf_dataptr();
  uint8_t data_len = packetbuf_datalen();
  uint8_t *headerptr = packetbuf_hdrptr();
  uint8_t header_len = packetbuf_hdrlen();
  uint8_t iv[13];
  
  memcpy(iv, extended_source_address, 8);
  iv[8] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) >> 8;
  iv[9] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) & 0xff;
  iv[10] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) >> 8;
  iv[11] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) & 0xff;
  iv[12] = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);

  if(packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL) & (1 << 2)) {
    CCM_STAR.mic(dataptr, data_len, iv, 13, headerptr, header_len, result, mic_len);
  } else {
    CCM_STAR.mic(dataptr, 0, iv, 13, headerptr, packetbuf_totlen(), result, mic_len);
  }
}
/*---------------------------------------------------------------------------*/
void ccm_star_ctr_packetbuf(const uint8_t *extended_source_address)
{
  uint8_t *dataptr = packetbuf_dataptr();
  uint8_t data_len = packetbuf_datalen();
  uint8_t iv[13];
  
  memcpy(iv, extended_source_address, 8);
  iv[8] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) >> 8;
  iv[9] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) & 0xff;
  iv[10] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) >> 8;
  iv[11] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) & 0xff;
  iv[12] = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);

  CCM_STAR.ctr(dataptr, data_len, iv, 13);
}
/*---------------------------------------------------------------------------*/
