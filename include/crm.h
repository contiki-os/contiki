#ifndef CRM_H
#define CRM_H

#define CRM_BASE         (0x80003000)
#define CRM_SYS_CNTL     (CRM_BASE+0x00)
#define CRM_WU_CNTL      (CRM_BASE+0x04)
#define CRM_SLEEP_CNTL   (CRM_BASE+0x08)
#define CRM_BS_CNTL      (CRM_BASE+0x0c)
#define CRM_COP_CNTL     (CRM_BASE+0x10)
#define CRM_COP_SERVICE  (CRM_BASE+0x14)
#define CRM_STATUS       (CRM_BASE+0x18)
#define CRM_MOD_STATUS   (CRM_BASE+0x1c)
#define CRM_WU_COUNT     (CRM_BASE+0x20)
#define CRM_WU_TIMEOUT   (CRM_BASE+0x24)
#define CRM_RTC_COUNT    (CRM_BASE+0x28)
#define CRM_RTC_TIMEOUT  (CRM_BASE+0x2c)
#define CRM_CAL_CNTL     (CRM_BASE+0x34)
#define CRM_CAL_COUNT    (CRM_BASE+0x38)
#define CRM_RINGOSC_CTNL (CRM_BASE+0x3c)
#define CRM_XTAL_CNTL    (CRM_BASE+0x40)
#define CRM_XTAL32_CNTL  (CRM_BASE+0x44)
#define CRM_VREG_CNTL    (CRM_BASE+0x48)
#define CRM_SW_RST       (CRM_BASE+0x50)

/* wu_cntl bit locations */
#define EXT_WU_IEN   20      /* 4 bits */ 
#define EXT_WU_EN    4       /* 4 bits */ 
#define EXT_WU_EDGE  8       /* 4 bits */ 
#define EXT_WU_POL   12      /* 4 bits */ 

/* status bit locations */
#define EXT_WU_EVT 4       /* 4 bits */

#define enable_wu_en(k) (set_bit(reg32(CRM_WU_CNTL),(EXT_WU_EN+k-4)))

#endif
