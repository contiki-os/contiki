#ifndef KBI_H
#define KBI_H

#define enable_irq_kbi(k)  (set_bit(*CRM_WU_CNTL,(EXT_WU_IEN+k-4)))
#define disable_irq_kbi(k) (clear_bit(*CRM_WU_CNTL,(EXT_WU_IEN+k-4)))

#define kbi_evnt(k) (bit_is_set(*CRM_STATUS,(EXT_WU_EVT+k-4)))

#define kbi_edge(k)  (set_bit(*CRM_WU_CNTL,(EXT_WU_EDGE+k-4)))
#define kbi_level(k) (clear_bit(*CRM_WU_CNTL,(EXT_WU_EDGE+k-4)))

#define kbi_pol_neg(k) (clear_bit(*CRM_WU_CNTL,(EXT_WU_POL+k-4)))
#define kbi_pol_pos(k) (set_bit(*CRM_WU_CNTL,(EXT_WU_POL+k-4)))

/* you have to clear these events by writing a one to them */

#define clear_kbi_evnt(k) (set_bit(*CRM_STATUS,(EXT_WU_EVT+k-4)))

#endif
