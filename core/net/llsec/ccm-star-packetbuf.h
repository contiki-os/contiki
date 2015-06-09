/**
 * \file
 *         CCM* convenience functions for MAC security
 * \author
 *         Justin King-Lacroix <justin.kinglacroix@gmail.com>
 */

#ifndef CCM_STAR_PACKETBUF_H_
#define CCM_STAR_PACKETBUF_H_

/**
 * \brief Calls CCM_STAR.mic with parameters appropriate for LLSEC.
 */
void ccm_star_mic_packetbuf(const uint8_t *extended_source_address,
    uint8_t *result,
    uint8_t mic_len);

/**
 * \brief Calls CCM_STAR.ctr with parameters appropriate for LLSEC.
 */
void ccm_star_ctr_packetbuf(const uint8_t *extended_source_address);

#endif /* CCM_STAR_PACKETBUF_H_ */

