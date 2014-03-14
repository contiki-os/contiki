#ifndef REGS_H_
#define REGS_H_                                           1


#define ReadRegister(a) a
#define WriteRegister(a, b) a = b

/* FLASH_BASE block */
#define DATA_FLASH_BASE_BASE                                 (0x00000000u)
#define DATA_FLASH_BASE_END                                  (0x0001FFFFu)
#define DATA_FLASH_BASE_SIZE                                 (DATA_FLASH_BASE_END - DATA_FLASH_BASE_BASE + 1)

/* FLASH block */
#define DATA_FLASH_BASE                                      (0x08000000u)
#define DATA_FLASH_END                                       (0x0801FFFFu)
#define DATA_FLASH_SIZE                                      (DATA_FLASH_END - DATA_FLASH_BASE + 1)

/* BIG_INFO_BASE block */
#define DATA_BIG_INFO_BASE_BASE                              (0x00000000u)
#define DATA_BIG_INFO_BASE_END                               (0x000007FFu)
#define DATA_BIG_INFO_BASE_SIZE                              (DATA_BIG_INFO_BASE_END - DATA_BIG_INFO_BASE_BASE + 1)

/* BIG_INFO block */
#define DATA_BIG_INFO_BASE                                   (0x08040000u)
#define DATA_BIG_INFO_END                                    (0x080407FFu)
#define DATA_BIG_INFO_SIZE                                   (DATA_BIG_INFO_END - DATA_BIG_INFO_BASE + 1)

/* SMALL_INFO block */
#define DATA_SMALL_INFO_BASE                                 (0x08040800u)
#define DATA_SMALL_INFO_END                                  (0x080409FFu)
#define DATA_SMALL_INFO_SIZE                                 (DATA_SMALL_INFO_END - DATA_SMALL_INFO_BASE + 1)

/* SRAM block */
#define DATA_SRAM_BASE                                       (0x20000000u)
#define DATA_SRAM_END                                        (0x20001FFFu)
#define DATA_SRAM_SIZE                                       (DATA_SRAM_END - DATA_SRAM_BASE + 1)

/* CM_HV block */
#define DATA_CM_HV_BASE                                      (0x40000000u)
#define DATA_CM_HV_END                                       (0x40000044u)
#define DATA_CM_HV_SIZE                                      (DATA_CM_HV_END - DATA_CM_HV_BASE + 1)

#define HV_SPARE                                             *((volatile uint32_t *)0x40000000u)
#define HV_SPARE_REG                                         *((volatile uint32_t *)0x40000000u)
#define HV_SPARE_ADDR                                        (0x40000000u)
#define HV_SPARE_RESET                                       (0x00000000u)
        /* HV_SPARE field */
        #define HV_SPARE_HV_SPARE                            (0x000000FFu)
        #define HV_SPARE_HV_SPARE_MASK                       (0x000000FFu)
        #define HV_SPARE_HV_SPARE_BIT                        (0)
        #define HV_SPARE_HV_SPARE_BITS                       (8)

#define EVENT_CTRL                                           *((volatile uint32_t *)0x40000004u)
#define EVENT_CTRL_REG                                       *((volatile uint32_t *)0x40000004u)
#define EVENT_CTRL_ADDR                                      (0x40000004u)
#define EVENT_CTRL_RESET                                     (0x00000000u)
        /* LV_FREEZE field */
        #define LV_FREEZE                                    (0x00000002u)
        #define LV_FREEZE_MASK                               (0x00000002u)
        #define LV_FREEZE_BIT                                (1)
        #define LV_FREEZE_BITS                               (1)

#define SLEEPTMR_CLKEN                                       *((volatile uint32_t *)0x40000008u)
#define SLEEPTMR_CLKEN_REG                                   *((volatile uint32_t *)0x40000008u)
#define SLEEPTMR_CLKEN_ADDR                                  (0x40000008u)
#define SLEEPTMR_CLKEN_RESET                                 (0x00000002u)
        /* SLEEPTMR_CLK10KEN field */
        #define SLEEPTMR_CLK10KEN                            (0x00000002u)
        #define SLEEPTMR_CLK10KEN_MASK                       (0x00000002u)
        #define SLEEPTMR_CLK10KEN_BIT                        (1)
        #define SLEEPTMR_CLK10KEN_BITS                       (1)
        /* SLEEPTMR_CLK32KEN field */
        #define SLEEPTMR_CLK32KEN                            (0x00000001u)
        #define SLEEPTMR_CLK32KEN_MASK                       (0x00000001u)
        #define SLEEPTMR_CLK32KEN_BIT                        (0)
        #define SLEEPTMR_CLK32KEN_BITS                       (1)

#define CLKRC_TUNE                                           *((volatile uint32_t *)0x4000000Cu)
#define CLKRC_TUNE_REG                                       *((volatile uint32_t *)0x4000000Cu)
#define CLKRC_TUNE_ADDR                                      (0x4000000Cu)
#define CLKRC_TUNE_RESET                                     (0x00000000u)
        /* CLKRC_TUNE_FIELD field */
        #define CLKRC_TUNE_FIELD                             (0x0000000Fu)
        #define CLKRC_TUNE_FIELD_MASK                        (0x0000000Fu)
        #define CLKRC_TUNE_FIELD_BIT                         (0)
        #define CLKRC_TUNE_FIELD_BITS                        (4)

#define CLK1K_CAL                                            *((volatile uint32_t *)0x40000010u)
#define CLK1K_CAL_REG                                        *((volatile uint32_t *)0x40000010u)
#define CLK1K_CAL_ADDR                                       (0x40000010u)
#define CLK1K_CAL_RESET                                      (0x00005000u)
        /* CLK1K_INTEGER field */
        #define CLK1K_INTEGER                                (0x0000F800u)
        #define CLK1K_INTEGER_MASK                           (0x0000F800u)
        #define CLK1K_INTEGER_BIT                            (11)
        #define CLK1K_INTEGER_BITS                           (5)
        /* CLK1K_FRACTIONAL field */
        #define CLK1K_FRACTIONAL                             (0x000007FFu)
        #define CLK1K_FRACTIONAL_MASK                        (0x000007FFu)
        #define CLK1K_FRACTIONAL_BIT                         (0)
        #define CLK1K_FRACTIONAL_BITS                        (11)

#define REGEN_DSLEEP                                         *((volatile uint32_t *)0x40000014u)
#define REGEN_DSLEEP_REG                                     *((volatile uint32_t *)0x40000014u)
#define REGEN_DSLEEP_ADDR                                    (0x40000014u)
#define REGEN_DSLEEP_RESET                                   (0x00000001u)
        /* REGEN_DSLEEP_FIELD field */
        #define REGEN_DSLEEP_FIELD                           (0x00000001u)
        #define REGEN_DSLEEP_FIELD_MASK                      (0x00000001u)
        #define REGEN_DSLEEP_FIELD_BIT                       (0)
        #define REGEN_DSLEEP_FIELD_BITS                      (1)

#define VREG                                                 *((volatile uint32_t *)0x40000018u)
#define VREG_REG                                             *((volatile uint32_t *)0x40000018u)
#define VREG_ADDR                                            (0x40000018u)
#define VREG_RESET                                           (0x00000207u)
        /* VREF_EN field */
        #define VREG_VREF_EN                                 (0x00008000u)
        #define VREG_VREF_EN_MASK                            (0x00008000u)
        #define VREG_VREF_EN_BIT                             (15)
        #define VREG_VREF_EN_BITS                            (1)
        /* VREF_TEST field */
        #define VREG_VREF_TEST                               (0x00004000u)
        #define VREG_VREF_TEST_MASK                          (0x00004000u)
        #define VREG_VREF_TEST_BIT                           (14)
        #define VREG_VREF_TEST_BITS                          (1)
        /* VREG_1V8_EN field */
        #define VREG_VREG_1V8_EN                             (0x00000800u)
        #define VREG_VREG_1V8_EN_MASK                        (0x00000800u)
        #define VREG_VREG_1V8_EN_BIT                         (11)
        #define VREG_VREG_1V8_EN_BITS                        (1)
        /* VREG_1V8_TEST field */
        #define VREG_VREG_1V8_TEST                           (0x00000400u)
        #define VREG_VREG_1V8_TEST_MASK                      (0x00000400u)
        #define VREG_VREG_1V8_TEST_BIT                       (10)
        #define VREG_VREG_1V8_TEST_BITS                      (1)
        /* VREG_1V8_TRIM field */
        #define VREG_VREG_1V8_TRIM                           (0x00000380u)
        #define VREG_VREG_1V8_TRIM_MASK                      (0x00000380u)
        #define VREG_VREG_1V8_TRIM_BIT                       (7)
        #define VREG_VREG_1V8_TRIM_BITS                      (3)
        /* VREG_1V2_EN field */
        #define VREG_VREG_1V2_EN                             (0x00000010u)
        #define VREG_VREG_1V2_EN_MASK                        (0x00000010u)
        #define VREG_VREG_1V2_EN_BIT                         (4)
        #define VREG_VREG_1V2_EN_BITS                        (1)
        /* VREG_1V2_TEST field */
        #define VREG_VREG_1V2_TEST                           (0x00000008u)
        #define VREG_VREG_1V2_TEST_MASK                      (0x00000008u)
        #define VREG_VREG_1V2_TEST_BIT                       (3)
        #define VREG_VREG_1V2_TEST_BITS                      (1)
        /* VREG_1V2_TRIM field */
        #define VREG_VREG_1V2_TRIM                           (0x00000007u)
        #define VREG_VREG_1V2_TRIM_MASK                      (0x00000007u)
        #define VREG_VREG_1V2_TRIM_BIT                       (0)
        #define VREG_VREG_1V2_TRIM_BITS                      (3)

#define WAKE_SEL                                             *((volatile uint32_t *)0x40000020u)
#define WAKE_SEL_REG                                         *((volatile uint32_t *)0x40000020u)
#define WAKE_SEL_ADDR                                        (0x40000020u)
#define WAKE_SEL_RESET                                       (0x00000200u)
        /* WAKE_CSYSPWRUPREQ field */
        #define WAKE_CSYSPWRUPREQ                            (0x00000200u)
        #define WAKE_CSYSPWRUPREQ_MASK                       (0x00000200u)
        #define WAKE_CSYSPWRUPREQ_BIT                        (9)
        #define WAKE_CSYSPWRUPREQ_BITS                       (1)
        /* WAKE_CDBGPWRUPREQ field */
        #define WAKE_CDBGPWRUPREQ                            (0x00000100u)
        #define WAKE_CDBGPWRUPREQ_MASK                       (0x00000100u)
        #define WAKE_CDBGPWRUPREQ_BIT                        (8)
        #define WAKE_CDBGPWRUPREQ_BITS                       (1)
        /* WAKE_WAKE_CORE field */
        #define WAKE_WAKE_CORE                               (0x00000080u)
        #define WAKE_WAKE_CORE_MASK                          (0x00000080u)
        #define WAKE_WAKE_CORE_BIT                           (7)
        #define WAKE_WAKE_CORE_BITS                          (1)
        /* WAKE_SLEEPTMRWRAP field */
        #define WAKE_SLEEPTMRWRAP                            (0x00000040u)
        #define WAKE_SLEEPTMRWRAP_MASK                       (0x00000040u)
        #define WAKE_SLEEPTMRWRAP_BIT                        (6)
        #define WAKE_SLEEPTMRWRAP_BITS                       (1)
        /* WAKE_SLEEPTMRCMPB field */
        #define WAKE_SLEEPTMRCMPB                            (0x00000020u)
        #define WAKE_SLEEPTMRCMPB_MASK                       (0x00000020u)
        #define WAKE_SLEEPTMRCMPB_BIT                        (5)
        #define WAKE_SLEEPTMRCMPB_BITS                       (1)
        /* WAKE_SLEEPTMRCMPA field */
        #define WAKE_SLEEPTMRCMPA                            (0x00000010u)
        #define WAKE_SLEEPTMRCMPA_MASK                       (0x00000010u)
        #define WAKE_SLEEPTMRCMPA_BIT                        (4)
        #define WAKE_SLEEPTMRCMPA_BITS                       (1)
        /* WAKE_IRQD field */
        #define WAKE_IRQD                                    (0x00000008u)
        #define WAKE_IRQD_MASK                               (0x00000008u)
        #define WAKE_IRQD_BIT                                (3)
        #define WAKE_IRQD_BITS                               (1)
        /* WAKE_SC2 field */
        #define WAKE_SC2                                     (0x00000004u)
        #define WAKE_SC2_MASK                                (0x00000004u)
        #define WAKE_SC2_BIT                                 (2)
        #define WAKE_SC2_BITS                                (1)
        /* WAKE_SC1 field */
        #define WAKE_SC1                                     (0x00000002u)
        #define WAKE_SC1_MASK                                (0x00000002u)
        #define WAKE_SC1_BIT                                 (1)
        #define WAKE_SC1_BITS                                (1)
        /* GPIO_WAKE field */
        #define GPIO_WAKE                                    (0x00000001u)
        #define GPIO_WAKE_MASK                               (0x00000001u)
        #define GPIO_WAKE_BIT                                (0)
        #define GPIO_WAKE_BITS                               (1)

#define WAKE_CORE                                            *((volatile uint32_t *)0x40000024u)
#define WAKE_CORE_REG                                        *((volatile uint32_t *)0x40000024u)
#define WAKE_CORE_ADDR                                       (0x40000024u)
#define WAKE_CORE_RESET                                      (0x00000000u)
        /* WAKE_CORE_FIELD field */
        #define WAKE_CORE_FIELD                              (0x00000020u)
        #define WAKE_CORE_FIELD_MASK                         (0x00000020u)
        #define WAKE_CORE_FIELD_BIT                          (5)
        #define WAKE_CORE_FIELD_BITS                         (1)

#define PWRUP_EVENT                                          *((volatile uint32_t *)0x40000028u)
#define PWRUP_EVENT_REG                                      *((volatile uint32_t *)0x40000028u)
#define PWRUP_EVENT_ADDR                                     (0x40000028u)
#define PWRUP_EVENT_RESET                                    (0x00000000u)
        /* PWRUP_CSYSPWRUPREQ field */
        #define PWRUP_CSYSPWRUPREQ                           (0x00000200u)
        #define PWRUP_CSYSPWRUPREQ_MASK                      (0x00000200u)
        #define PWRUP_CSYSPWRUPREQ_BIT                       (9)
        #define PWRUP_CSYSPWRUPREQ_BITS                      (1)
        /* PWRUP_CDBGPWRUPREQ field */
        #define PWRUP_CDBGPWRUPREQ                           (0x00000100u)
        #define PWRUP_CDBGPWRUPREQ_MASK                      (0x00000100u)
        #define PWRUP_CDBGPWRUPREQ_BIT                       (8)
        #define PWRUP_CDBGPWRUPREQ_BITS                      (1)
        /* PWRUP_WAKECORE field */
        #define PWRUP_WAKECORE                               (0x00000080u)
        #define PWRUP_WAKECORE_MASK                          (0x00000080u)
        #define PWRUP_WAKECORE_BIT                           (7)
        #define PWRUP_WAKECORE_BITS                          (1)
        /* PWRUP_SLEEPTMRWRAP field */
        #define PWRUP_SLEEPTMRWRAP                           (0x00000040u)
        #define PWRUP_SLEEPTMRWRAP_MASK                      (0x00000040u)
        #define PWRUP_SLEEPTMRWRAP_BIT                       (6)
        #define PWRUP_SLEEPTMRWRAP_BITS                      (1)
        /* PWRUP_SLEEPTMRCOMPB field */
        #define PWRUP_SLEEPTMRCOMPB                          (0x00000020u)
        #define PWRUP_SLEEPTMRCOMPB_MASK                     (0x00000020u)
        #define PWRUP_SLEEPTMRCOMPB_BIT                      (5)
        #define PWRUP_SLEEPTMRCOMPB_BITS                     (1)
        /* PWRUP_SLEEPTMRCOMPA field */
        #define PWRUP_SLEEPTMRCOMPA                          (0x00000010u)
        #define PWRUP_SLEEPTMRCOMPA_MASK                     (0x00000010u)
        #define PWRUP_SLEEPTMRCOMPA_BIT                      (4)
        #define PWRUP_SLEEPTMRCOMPA_BITS                     (1)
        /* PWRUP_IRQD field */
        #define PWRUP_IRQD                                   (0x00000008u)
        #define PWRUP_IRQD_MASK                              (0x00000008u)
        #define PWRUP_IRQD_BIT                               (3)
        #define PWRUP_IRQD_BITS                              (1)
        /* PWRUP_SC2 field */
        #define PWRUP_SC2                                    (0x00000004u)
        #define PWRUP_SC2_MASK                               (0x00000004u)
        #define PWRUP_SC2_BIT                                (2)
        #define PWRUP_SC2_BITS                               (1)
        /* PWRUP_SC1 field */
        #define PWRUP_SC1                                    (0x00000002u)
        #define PWRUP_SC1_MASK                               (0x00000002u)
        #define PWRUP_SC1_BIT                                (1)
        #define PWRUP_SC1_BITS                               (1)
        /* PWRUP_GPIO field */
        #define PWRUP_GPIO                                   (0x00000001u)
        #define PWRUP_GPIO_MASK                              (0x00000001u)
        #define PWRUP_GPIO_BIT                               (0)
        #define PWRUP_GPIO_BITS                              (1)

#define RESET_EVENT                                          *((volatile uint32_t *)0x4000002Cu)
#define RESET_EVENT_REG                                      *((volatile uint32_t *)0x4000002Cu)
#define RESET_EVENT_ADDR                                     (0x4000002Cu)
#define RESET_EVENT_RESET                                    (0x00000001u)
        /* RESET_CPULOCKUP field */
        #define RESET_CPULOCKUP                              (0x00000080u)
        #define RESET_CPULOCKUP_MASK                         (0x00000080u)
        #define RESET_CPULOCKUP_BIT                          (7)
        #define RESET_CPULOCKUP_BITS                         (1)
        /* RESET_OPTBYTEFAIL field */
        #define RESET_OPTBYTEFAIL                            (0x00000040u)
        #define RESET_OPTBYTEFAIL_MASK                       (0x00000040u)
        #define RESET_OPTBYTEFAIL_BIT                        (6)
        #define RESET_OPTBYTEFAIL_BITS                       (1)
        /* RESET_DSLEEP field */
        #define RESET_DSLEEP                                 (0x00000020u)
        #define RESET_DSLEEP_MASK                            (0x00000020u)
        #define RESET_DSLEEP_BIT                             (5)
        #define RESET_DSLEEP_BITS                            (1)
        /* RESET_SW field */
        #define RESET_SW                                     (0x00000010u)
        #define RESET_SW_MASK                                (0x00000010u)
        #define RESET_SW_BIT                                 (4)
        #define RESET_SW_BITS                                (1)
        /* RESET_WDOG field */
        #define RESET_WDOG                                   (0x00000008u)
        #define RESET_WDOG_MASK                              (0x00000008u)
        #define RESET_WDOG_BIT                               (3)
        #define RESET_WDOG_BITS                              (1)
        /* RESET_NRESET field */
        #define RESET_NRESET                                 (0x00000004u)
        #define RESET_NRESET_MASK                            (0x00000004u)
        #define RESET_NRESET_BIT                             (2)
        #define RESET_NRESET_BITS                            (1)
        /* RESET_PWRLV field */
        #define RESET_PWRLV                                  (0x00000002u)
        #define RESET_PWRLV_MASK                             (0x00000002u)
        #define RESET_PWRLV_BIT                              (1)
        #define RESET_PWRLV_BITS                             (1)
        /* RESET_PWRHV field */
        #define RESET_PWRHV                                  (0x00000001u)
        #define RESET_PWRHV_MASK                             (0x00000001u)
        #define RESET_PWRHV_BIT                              (0)
        #define RESET_PWRHV_BITS                             (1)

#define DBG_MBOX                                             *((volatile uint32_t *)0x40000030u)
#define DBG_MBOX_REG                                         *((volatile uint32_t *)0x40000030u)
#define DBG_MBOX_ADDR                                        (0x40000030u)
#define DBG_MBOX_RESET                                       (0x00000000u)
        /* DBG_MBOX field */
        #define DBG_MBOX_DBG_MBOX                            (0x0000FFFFu)
        #define DBG_MBOX_DBG_MBOX_MASK                       (0x0000FFFFu)
        #define DBG_MBOX_DBG_MBOX_BIT                        (0)
        #define DBG_MBOX_DBG_MBOX_BITS                       (16)

#define CPWRUPREQ_STATUS                                     *((volatile uint32_t *)0x40000034u)
#define CPWRUPREQ_STATUS_REG                                 *((volatile uint32_t *)0x40000034u)
#define CPWRUPREQ_STATUS_ADDR                                (0x40000034u)
#define CPWRUPREQ_STATUS_RESET                               (0x00000000u)
        /* CPWRUPREQ field */
        #define CPWRUPREQ_STATUS_CPWRUPREQ                   (0x00000001u)
        #define CPWRUPREQ_STATUS_CPWRUPREQ_MASK              (0x00000001u)
        #define CPWRUPREQ_STATUS_CPWRUPREQ_BIT               (0)
        #define CPWRUPREQ_STATUS_CPWRUPREQ_BITS              (1)

#define CSYSPWRUPREQ_STATUS                                  *((volatile uint32_t *)0x40000038u)
#define CSYSPWRUPREQ_STATUS_REG                              *((volatile uint32_t *)0x40000038u)
#define CSYSPWRUPREQ_STATUS_ADDR                             (0x40000038u)
#define CSYSPWRUPREQ_STATUS_RESET                            (0x00000000u)
        /* CSYSPWRUPREQ field */
        #define CSYSPWRUPREQ_STATUS_CSYSPWRUPREQ             (0x00000001u)
        #define CSYSPWRUPREQ_STATUS_CSYSPWRUPREQ_MASK        (0x00000001u)
        #define CSYSPWRUPREQ_STATUS_CSYSPWRUPREQ_BIT         (0)
        #define CSYSPWRUPREQ_STATUS_CSYSPWRUPREQ_BITS        (1)

#define CSYSPWRUPACK_STATUS                                  *((volatile uint32_t *)0x4000003Cu)
#define CSYSPWRUPACK_STATUS_REG                              *((volatile uint32_t *)0x4000003Cu)
#define CSYSPWRUPACK_STATUS_ADDR                             (0x4000003Cu)
#define CSYSPWRUPACK_STATUS_RESET                            (0x00000000u)
        /* CSYSPWRUPACK field */
        #define CSYSPWRUPACK_STATUS_CSYSPWRUPACK             (0x00000001u)
        #define CSYSPWRUPACK_STATUS_CSYSPWRUPACK_MASK        (0x00000001u)
        #define CSYSPWRUPACK_STATUS_CSYSPWRUPACK_BIT         (0)
        #define CSYSPWRUPACK_STATUS_CSYSPWRUPACK_BITS        (1)

#define CSYSPWRUPACK_INHIBIT                                 *((volatile uint32_t *)0x40000040u)
#define CSYSPWRUPACK_INHIBIT_REG                             *((volatile uint32_t *)0x40000040u)
#define CSYSPWRUPACK_INHIBIT_ADDR                            (0x40000040u)
#define CSYSPWRUPACK_INHIBIT_RESET                           (0x00000000u)
        /* CSYSPWRUPACK_INHIBIT field */
        #define CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT    (0x00000001u)
        #define CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT_MASK (0x00000001u)
        #define CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT_BIT (0)
        #define CSYSPWRUPACK_INHIBIT_CSYSPWRUPACK_INHIBIT_BITS (1)

#define OPT_ERR_MAINTAIN_WAKE                                *((volatile uint32_t *)0x40000044u)
#define OPT_ERR_MAINTAIN_WAKE_REG                            *((volatile uint32_t *)0x40000044u)
#define OPT_ERR_MAINTAIN_WAKE_ADDR                           (0x40000044u)
#define OPT_ERR_MAINTAIN_WAKE_RESET                          (0x00000000u)
        /* OPT_ERR_MAINTAIN_WAKE field */
        #define OPT_ERR_MAINTAIN_WAKE_OPT_ERR_MAINTAIN_WAKE  (0x00000001u)
        #define OPT_ERR_MAINTAIN_WAKE_OPT_ERR_MAINTAIN_WAKE_MASK (0x00000001u)
        #define OPT_ERR_MAINTAIN_WAKE_OPT_ERR_MAINTAIN_WAKE_BIT (0)
        #define OPT_ERR_MAINTAIN_WAKE_OPT_ERR_MAINTAIN_WAKE_BITS (1)

/* BASEBAND block */
#define DATA_BASEBAND_BASE                                   (0x40001000u)
#define DATA_BASEBAND_END                                    (0x40001114u)
#define DATA_BASEBAND_SIZE                                   (DATA_BASEBAND_END - DATA_BASEBAND_BASE + 1)

#define MOD_CAL_CTRL                                         *((volatile uint32_t *)0x40001000u)
#define MOD_CAL_CTRL_REG                                     *((volatile uint32_t *)0x40001000u)
#define MOD_CAL_CTRL_ADDR                                    (0x40001000u)
#define MOD_CAL_CTRL_RESET                                   (0x00000000u)
        /* MOD_CAL_GO field */
        #define MOD_CAL_CTRL_MOD_CAL_GO                      (0x00008000u)
        #define MOD_CAL_CTRL_MOD_CAL_GO_MASK                 (0x00008000u)
        #define MOD_CAL_CTRL_MOD_CAL_GO_BIT                  (15)
        #define MOD_CAL_CTRL_MOD_CAL_GO_BITS                 (1)
        /* MOD_CAL_DONE field */
        #define MOD_CAL_CTRL_MOD_CAL_DONE                    (0x00000010u)
        #define MOD_CAL_CTRL_MOD_CAL_DONE_MASK               (0x00000010u)
        #define MOD_CAL_CTRL_MOD_CAL_DONE_BIT                (4)
        #define MOD_CAL_CTRL_MOD_CAL_DONE_BITS               (1)
        /* MOD_CAL_CYCLES field */
        #define MOD_CAL_CTRL_MOD_CAL_CYCLES                  (0x00000003u)
        #define MOD_CAL_CTRL_MOD_CAL_CYCLES_MASK             (0x00000003u)
        #define MOD_CAL_CTRL_MOD_CAL_CYCLES_BIT              (0)
        #define MOD_CAL_CTRL_MOD_CAL_CYCLES_BITS             (2)

#define MOD_CAL_COUNT_H                                      *((volatile uint32_t *)0x40001004u)
#define MOD_CAL_COUNT_H_REG                                  *((volatile uint32_t *)0x40001004u)
#define MOD_CAL_COUNT_H_ADDR                                 (0x40001004u)
#define MOD_CAL_COUNT_H_RESET                                (0x00000000u)
        /* MOD_CAL_COUNT_H field */
        #define MOD_CAL_COUNT_H_MOD_CAL_COUNT_H              (0x000000FFu)
        #define MOD_CAL_COUNT_H_MOD_CAL_COUNT_H_MASK         (0x000000FFu)
        #define MOD_CAL_COUNT_H_MOD_CAL_COUNT_H_BIT          (0)
        #define MOD_CAL_COUNT_H_MOD_CAL_COUNT_H_BITS         (8)

#define MOD_CAL_COUNT_L                                      *((volatile uint32_t *)0x40001008u)
#define MOD_CAL_COUNT_L_REG                                  *((volatile uint32_t *)0x40001008u)
#define MOD_CAL_COUNT_L_ADDR                                 (0x40001008u)
#define MOD_CAL_COUNT_L_RESET                                (0x00000000u)
        /* MOD_CAL_COUNT_L field */
        #define MOD_CAL_COUNT_L_MOD_CAL_COUNT_L              (0x0000FFFFu)
        #define MOD_CAL_COUNT_L_MOD_CAL_COUNT_L_MASK         (0x0000FFFFu)
        #define MOD_CAL_COUNT_L_MOD_CAL_COUNT_L_BIT          (0)
        #define MOD_CAL_COUNT_L_MOD_CAL_COUNT_L_BITS         (16)

#define RSSI_ROLLING                                         *((volatile uint32_t *)0x4000100Cu)
#define RSSI_ROLLING_REG                                     *((volatile uint32_t *)0x4000100Cu)
#define RSSI_ROLLING_ADDR                                    (0x4000100Cu)
#define RSSI_ROLLING_RESET                                   (0x00000000u)
        /* RSSI_ROLLING field */
        #define RSSI_ROLLING_RSSI_ROLLING                    (0x00003FFFu)
        #define RSSI_ROLLING_RSSI_ROLLING_MASK               (0x00003FFFu)
        #define RSSI_ROLLING_RSSI_ROLLING_BIT                (0)
        #define RSSI_ROLLING_RSSI_ROLLING_BITS               (14)

#define RSSI_PKT                                             *((volatile uint32_t *)0x40001010u)
#define RSSI_PKT_REG                                         *((volatile uint32_t *)0x40001010u)
#define RSSI_PKT_ADDR                                        (0x40001010u)
#define RSSI_PKT_RESET                                       (0x00000000u)
        /* RSSI_PKT field */
        #define RSSI_PKT_RSSI_PKT                            (0x000000FFu)
        #define RSSI_PKT_RSSI_PKT_MASK                       (0x000000FFu)
        #define RSSI_PKT_RSSI_PKT_BIT                        (0)
        #define RSSI_PKT_RSSI_PKT_BITS                       (8)

#define RX_ADC                                               *((volatile uint32_t *)0x40001014u)
#define RX_ADC_REG                                           *((volatile uint32_t *)0x40001014u)
#define RX_ADC_ADDR                                          (0x40001014u)
#define RX_ADC_RESET                                         (0x00000024u)
        /* RX_ADC field */
        #define RX_ADC_RX_ADC                                (0x0000007Fu)
        #define RX_ADC_RX_ADC_MASK                           (0x0000007Fu)
        #define RX_ADC_RX_ADC_BIT                            (0)
        #define RX_ADC_RX_ADC_BITS                           (7)

#define DEBUG_BB_MODE                                        *((volatile uint32_t *)0x40001018u)
#define DEBUG_BB_MODE_REG                                    *((volatile uint32_t *)0x40001018u)
#define DEBUG_BB_MODE_ADDR                                   (0x40001018u)
#define DEBUG_BB_MODE_RESET                                  (0x00000000u)
        /* DEBUG_BB_MODE_EN field */
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_EN               (0x00008000u)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_EN_MASK          (0x00008000u)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_EN_BIT           (15)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_EN_BITS          (1)
        /* DEBUG_BB_MODE field */
        #define DEBUG_BB_MODE_DEBUG_BB_MODE                  (0x00000003u)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_MASK             (0x00000003u)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_BIT              (0)
        #define DEBUG_BB_MODE_DEBUG_BB_MODE_BITS             (2)

#define BB_DEBUG                                             *((volatile uint32_t *)0x4000101Cu)
#define BB_DEBUG_REG                                         *((volatile uint32_t *)0x4000101Cu)
#define BB_DEBUG_ADDR                                        (0x4000101Cu)
#define BB_DEBUG_RESET                                       (0x00000002u)
        /* SYNC_REG_EN field */
        #define BB_DEBUG_SYNC_REG_EN                         (0x00008000u)
        #define BB_DEBUG_SYNC_REG_EN_MASK                    (0x00008000u)
        #define BB_DEBUG_SYNC_REG_EN_BIT                     (15)
        #define BB_DEBUG_SYNC_REG_EN_BITS                    (1)
        /* DEBUG_MUX_ADDR field */
        #define BB_DEBUG_DEBUG_MUX_ADDR                      (0x000000F0u)
        #define BB_DEBUG_DEBUG_MUX_ADDR_MASK                 (0x000000F0u)
        #define BB_DEBUG_DEBUG_MUX_ADDR_BIT                  (4)
        #define BB_DEBUG_DEBUG_MUX_ADDR_BITS                 (4)
        /* BB_DEBUG_SEL field */
        #define BB_DEBUG_BB_DEBUG_SEL                        (0x00000003u)
        #define BB_DEBUG_BB_DEBUG_SEL_MASK                   (0x00000003u)
        #define BB_DEBUG_BB_DEBUG_SEL_BIT                    (0)
        #define BB_DEBUG_BB_DEBUG_SEL_BITS                   (2)

#define BB_DEBUG_VIEW                                        *((volatile uint32_t *)0x40001020u)
#define BB_DEBUG_VIEW_REG                                    *((volatile uint32_t *)0x40001020u)
#define BB_DEBUG_VIEW_ADDR                                   (0x40001020u)
#define BB_DEBUG_VIEW_RESET                                  (0x00000000u)
        /* BB_DEBUG_VIEW field */
        #define BB_DEBUG_VIEW_BB_DEBUG_VIEW                  (0x0000FFFFu)
        #define BB_DEBUG_VIEW_BB_DEBUG_VIEW_MASK             (0x0000FFFFu)
        #define BB_DEBUG_VIEW_BB_DEBUG_VIEW_BIT              (0)
        #define BB_DEBUG_VIEW_BB_DEBUG_VIEW_BITS             (16)

#define IF_FREQ                                              *((volatile uint32_t *)0x40001024u)
#define IF_FREQ_REG                                          *((volatile uint32_t *)0x40001024u)
#define IF_FREQ_ADDR                                         (0x40001024u)
#define IF_FREQ_RESET                                        (0x00000155u)
        /* TIMING_CORR_EN field */
        #define IF_FREQ_TIMING_CORR_EN                       (0x00008000u)
        #define IF_FREQ_TIMING_CORR_EN_MASK                  (0x00008000u)
        #define IF_FREQ_TIMING_CORR_EN_BIT                   (15)
        #define IF_FREQ_TIMING_CORR_EN_BITS                  (1)
        /* IF_FREQ field */
        #define IF_FREQ_IF_FREQ                              (0x000001FFu)
        #define IF_FREQ_IF_FREQ_MASK                         (0x000001FFu)
        #define IF_FREQ_IF_FREQ_BIT                          (0)
        #define IF_FREQ_IF_FREQ_BITS                         (9)

#define MOD_EN                                               *((volatile uint32_t *)0x40001028u)
#define MOD_EN_REG                                           *((volatile uint32_t *)0x40001028u)
#define MOD_EN_ADDR                                          (0x40001028u)
#define MOD_EN_RESET                                         (0x00000001u)
        /* MOD_EN field */
        #define MOD_EN_MOD_EN                                (0x00000001u)
        #define MOD_EN_MOD_EN_MASK                           (0x00000001u)
        #define MOD_EN_MOD_EN_BIT                            (0)
        #define MOD_EN_MOD_EN_BITS                           (1)

#define PRESCALE_CTRL                                        *((volatile uint32_t *)0x4000102Cu)
#define PRESCALE_CTRL_REG                                    *((volatile uint32_t *)0x4000102Cu)
#define PRESCALE_CTRL_ADDR                                   (0x4000102Cu)
#define PRESCALE_CTRL_RESET                                  (0x00000000u)
        /* PRESCALE_SET field */
        #define PRESCALE_CTRL_PRESCALE_SET                   (0x00008000u)
        #define PRESCALE_CTRL_PRESCALE_SET_MASK              (0x00008000u)
        #define PRESCALE_CTRL_PRESCALE_SET_BIT               (15)
        #define PRESCALE_CTRL_PRESCALE_SET_BITS              (1)
        /* PRESCALE_VAL field */
        #define PRESCALE_CTRL_PRESCALE_VAL                   (0x00000007u)
        #define PRESCALE_CTRL_PRESCALE_VAL_MASK              (0x00000007u)
        #define PRESCALE_CTRL_PRESCALE_VAL_BIT               (0)
        #define PRESCALE_CTRL_PRESCALE_VAL_BITS              (3)

#define ADC_BYPASS_EN                                        *((volatile uint32_t *)0x40001030u)
#define ADC_BYPASS_EN_REG                                    *((volatile uint32_t *)0x40001030u)
#define ADC_BYPASS_EN_ADDR                                   (0x40001030u)
#define ADC_BYPASS_EN_RESET                                  (0x00000000u)
        /* ADC_BYPASS_EN field */
        #define ADC_BYPASS_EN_ADC_BYPASS_EN                  (0x00000001u)
        #define ADC_BYPASS_EN_ADC_BYPASS_EN_MASK             (0x00000001u)
        #define ADC_BYPASS_EN_ADC_BYPASS_EN_BIT              (0)
        #define ADC_BYPASS_EN_ADC_BYPASS_EN_BITS             (1)

#define FIXED_CODE_EN                                        *((volatile uint32_t *)0x40001034u)
#define FIXED_CODE_EN_REG                                    *((volatile uint32_t *)0x40001034u)
#define FIXED_CODE_EN_ADDR                                   (0x40001034u)
#define FIXED_CODE_EN_RESET                                  (0x00000000u)
        /* FIXED_CODE_EN field */
        #define FIXED_CODE_EN_FIXED_CODE_EN                  (0x00000001u)
        #define FIXED_CODE_EN_FIXED_CODE_EN_MASK             (0x00000001u)
        #define FIXED_CODE_EN_FIXED_CODE_EN_BIT              (0)
        #define FIXED_CODE_EN_FIXED_CODE_EN_BITS             (1)

#define FIXED_CODE_H                                         *((volatile uint32_t *)0x40001038u)
#define FIXED_CODE_H_REG                                     *((volatile uint32_t *)0x40001038u)
#define FIXED_CODE_H_ADDR                                    (0x40001038u)
#define FIXED_CODE_H_RESET                                   (0x00000000u)
        /* FIXED_CODE_H field */
        #define FIXED_CODE_H_FIXED_CODE_H                    (0x0000FFFFu)
        #define FIXED_CODE_H_FIXED_CODE_H_MASK               (0x0000FFFFu)
        #define FIXED_CODE_H_FIXED_CODE_H_BIT                (0)
        #define FIXED_CODE_H_FIXED_CODE_H_BITS               (16)

#define FIXED_CODE_L                                         *((volatile uint32_t *)0x4000103Cu)
#define FIXED_CODE_L_REG                                     *((volatile uint32_t *)0x4000103Cu)
#define FIXED_CODE_L_ADDR                                    (0x4000103Cu)
#define FIXED_CODE_L_RESET                                   (0x00000000u)
        /* FIXED_CODE_L field */
        #define FIXED_CODE_L_FIXED_CODE_L                    (0x0000FFFFu)
        #define FIXED_CODE_L_FIXED_CODE_L_MASK               (0x0000FFFFu)
        #define FIXED_CODE_L_FIXED_CODE_L_BIT                (0)
        #define FIXED_CODE_L_FIXED_CODE_L_BITS               (16)

#define FIXED_CODE_L_SHADOW                                  *((volatile uint32_t *)0x40001040u)
#define FIXED_CODE_L_SHADOW_REG                              *((volatile uint32_t *)0x40001040u)
#define FIXED_CODE_L_SHADOW_ADDR                             (0x40001040u)
#define FIXED_CODE_L_SHADOW_RESET                            (0x00000000u)
        /* FIXED_CODE_L_SHADOW field */
        #define FIXED_CODE_L_SHADOW_FIXED_CODE_L_SHADOW      (0x0000FFFFu)
        #define FIXED_CODE_L_SHADOW_FIXED_CODE_L_SHADOW_MASK (0x0000FFFFu)
        #define FIXED_CODE_L_SHADOW_FIXED_CODE_L_SHADOW_BIT  (0)
        #define FIXED_CODE_L_SHADOW_FIXED_CODE_L_SHADOW_BITS (16)

#define RX_GAIN_CTRL                                         *((volatile uint32_t *)0x40001044u)
#define RX_GAIN_CTRL_REG                                     *((volatile uint32_t *)0x40001044u)
#define RX_GAIN_CTRL_ADDR                                    (0x40001044u)
#define RX_GAIN_CTRL_RESET                                   (0x00000000u)
        /* RX_GAIN_MUX field */
        #define RX_GAIN_CTRL_RX_GAIN_MUX                     (0x00008000u)
        #define RX_GAIN_CTRL_RX_GAIN_MUX_MASK                (0x00008000u)
        #define RX_GAIN_CTRL_RX_GAIN_MUX_BIT                 (15)
        #define RX_GAIN_CTRL_RX_GAIN_MUX_BITS                (1)
        /* RX_RF_GAIN_TEST field */
        #define RX_GAIN_CTRL_RX_RF_GAIN_TEST                 (0x00000080u)
        #define RX_GAIN_CTRL_RX_RF_GAIN_TEST_MASK            (0x00000080u)
        #define RX_GAIN_CTRL_RX_RF_GAIN_TEST_BIT             (7)
        #define RX_GAIN_CTRL_RX_RF_GAIN_TEST_BITS            (1)
        /* RX_MIXER_GAIN_TEST field */
        #define RX_GAIN_CTRL_RX_MIXER_GAIN_TEST              (0x00000040u)
        #define RX_GAIN_CTRL_RX_MIXER_GAIN_TEST_MASK         (0x00000040u)
        #define RX_GAIN_CTRL_RX_MIXER_GAIN_TEST_BIT          (6)
        #define RX_GAIN_CTRL_RX_MIXER_GAIN_TEST_BITS         (1)
        /* RX_FILTER_GAIN_TEST field */
        #define RX_GAIN_CTRL_RX_FILTER_GAIN_TEST             (0x00000030u)
        #define RX_GAIN_CTRL_RX_FILTER_GAIN_TEST_MASK        (0x00000030u)
        #define RX_GAIN_CTRL_RX_FILTER_GAIN_TEST_BIT         (4)
        #define RX_GAIN_CTRL_RX_FILTER_GAIN_TEST_BITS        (2)
        /* RX_IF_GAIN_TEST field */
        #define RX_GAIN_CTRL_RX_IF_GAIN_TEST                 (0x0000000Fu)
        #define RX_GAIN_CTRL_RX_IF_GAIN_TEST_MASK            (0x0000000Fu)
        #define RX_GAIN_CTRL_RX_IF_GAIN_TEST_BIT             (0)
        #define RX_GAIN_CTRL_RX_IF_GAIN_TEST_BITS            (4)

#define PD_DITHER_EN                                         *((volatile uint32_t *)0x40001048u)
#define PD_DITHER_EN_REG                                     *((volatile uint32_t *)0x40001048u)
#define PD_DITHER_EN_ADDR                                    (0x40001048u)
#define PD_DITHER_EN_RESET                                   (0x00000001u)
        /* PD_DITHER_EN field */
        #define PD_DITHER_EN_PD_DITHER_EN                    (0x00000001u)
        #define PD_DITHER_EN_PD_DITHER_EN_MASK               (0x00000001u)
        #define PD_DITHER_EN_PD_DITHER_EN_BIT                (0)
        #define PD_DITHER_EN_PD_DITHER_EN_BITS               (1)

#define RX_ERR_THRESH                                        *((volatile uint32_t *)0x4000104Cu)
#define RX_ERR_THRESH_REG                                    *((volatile uint32_t *)0x4000104Cu)
#define RX_ERR_THRESH_ADDR                                   (0x4000104Cu)
#define RX_ERR_THRESH_RESET                                  (0x00004608u)
        /* LPF_RX_ERR_COEFF field */
        #define RX_ERR_THRESH_LPF_RX_ERR_COEFF               (0x0000E000u)
        #define RX_ERR_THRESH_LPF_RX_ERR_COEFF_MASK          (0x0000E000u)
        #define RX_ERR_THRESH_LPF_RX_ERR_COEFF_BIT           (13)
        #define RX_ERR_THRESH_LPF_RX_ERR_COEFF_BITS          (3)
        /* LPF_RX_ERR_THRESH field */
        #define RX_ERR_THRESH_LPF_RX_ERR_THRESH              (0x00001F00u)
        #define RX_ERR_THRESH_LPF_RX_ERR_THRESH_MASK         (0x00001F00u)
        #define RX_ERR_THRESH_LPF_RX_ERR_THRESH_BIT          (8)
        #define RX_ERR_THRESH_LPF_RX_ERR_THRESH_BITS         (5)
        /* RX_ERR_THRESH field */
        #define RX_ERR_THRESH_RX_ERR_THRESH                  (0x0000001Fu)
        #define RX_ERR_THRESH_RX_ERR_THRESH_MASK             (0x0000001Fu)
        #define RX_ERR_THRESH_RX_ERR_THRESH_BIT              (0)
        #define RX_ERR_THRESH_RX_ERR_THRESH_BITS             (5)

#define CARRIER_THRESH                                       *((volatile uint32_t *)0x40001050u)
#define CARRIER_THRESH_REG                                   *((volatile uint32_t *)0x40001050u)
#define CARRIER_THRESH_ADDR                                  (0x40001050u)
#define CARRIER_THRESH_RESET                                 (0x00002332u)
        /* CARRIER_SPIKE_THRESH field */
        #define CARRIER_THRESH_CARRIER_SPIKE_THRESH          (0x0000FF00u)
        #define CARRIER_THRESH_CARRIER_SPIKE_THRESH_MASK     (0x0000FF00u)
        #define CARRIER_THRESH_CARRIER_SPIKE_THRESH_BIT      (8)
        #define CARRIER_THRESH_CARRIER_SPIKE_THRESH_BITS     (8)
        /* CARRIER_THRESH field */
        #define CARRIER_THRESH_CARRIER_THRESH                (0x000000FFu)
        #define CARRIER_THRESH_CARRIER_THRESH_MASK           (0x000000FFu)
        #define CARRIER_THRESH_CARRIER_THRESH_BIT            (0)
        #define CARRIER_THRESH_CARRIER_THRESH_BITS           (8)

#define RSSI_THRESH                                          *((volatile uint32_t *)0x40001054u)
#define RSSI_THRESH_REG                                      *((volatile uint32_t *)0x40001054u)
#define RSSI_THRESH_ADDR                                     (0x40001054u)
#define RSSI_THRESH_RESET                                    (0x00000100u)
        /* RSSI_THRESH field */
        #define RSSI_THRESH_RSSI_THRESH                      (0x0000FFFFu)
        #define RSSI_THRESH_RSSI_THRESH_MASK                 (0x0000FFFFu)
        #define RSSI_THRESH_RSSI_THRESH_BIT                  (0)
        #define RSSI_THRESH_RSSI_THRESH_BITS                 (16)

#define SYNTH_START                                          *((volatile uint32_t *)0x40001058u)
#define SYNTH_START_REG                                      *((volatile uint32_t *)0x40001058u)
#define SYNTH_START_ADDR                                     (0x40001058u)
#define SYNTH_START_RESET                                    (0x00006464u)
        /* SYNTH_WARM_START field */
        #define SYNTH_START_SYNTH_WARM_START                 (0x0000FF00u)
        #define SYNTH_START_SYNTH_WARM_START_MASK            (0x0000FF00u)
        #define SYNTH_START_SYNTH_WARM_START_BIT             (8)
        #define SYNTH_START_SYNTH_WARM_START_BITS            (8)
        /* SYNTH_COLD_START field */
        #define SYNTH_START_SYNTH_COLD_START                 (0x000000FFu)
        #define SYNTH_START_SYNTH_COLD_START_MASK            (0x000000FFu)
        #define SYNTH_START_SYNTH_COLD_START_BIT             (0)
        #define SYNTH_START_SYNTH_COLD_START_BITS            (8)

#define IN_LOCK_EN                                           *((volatile uint32_t *)0x4000105Cu)
#define IN_LOCK_EN_REG                                       *((volatile uint32_t *)0x4000105Cu)
#define IN_LOCK_EN_ADDR                                      (0x4000105Cu)
#define IN_LOCK_EN_RESET                                     (0x00000001u)
        /* IN_LOCK_EN field */
        #define IN_LOCK_EN_IN_LOCK_EN                        (0x00000001u)
        #define IN_LOCK_EN_IN_LOCK_EN_MASK                   (0x00000001u)
        #define IN_LOCK_EN_IN_LOCK_EN_BIT                    (0)
        #define IN_LOCK_EN_IN_LOCK_EN_BITS                   (1)

#define DITHER_AMPLITUDE                                     *((volatile uint32_t *)0x40001060u)
#define DITHER_AMPLITUDE_REG                                 *((volatile uint32_t *)0x40001060u)
#define DITHER_AMPLITUDE_ADDR                                (0x40001060u)
#define DITHER_AMPLITUDE_RESET                               (0x0000003Fu)
        /* DITHER_AMP field */
        #define DITHER_AMPLITUDE_DITHER_AMP                  (0x0000003Fu)
        #define DITHER_AMPLITUDE_DITHER_AMP_MASK             (0x0000003Fu)
        #define DITHER_AMPLITUDE_DITHER_AMP_BIT              (0)
        #define DITHER_AMPLITUDE_DITHER_AMP_BITS             (6)

#define TX_STEP_TIME                                         *((volatile uint32_t *)0x40001064u)
#define TX_STEP_TIME_REG                                     *((volatile uint32_t *)0x40001064u)
#define TX_STEP_TIME_ADDR                                    (0x40001064u)
#define TX_STEP_TIME_RESET                                   (0x00000000u)
        /* TX_STEP_TIME field */
        #define TX_STEP_TIME_TX_STEP_TIME                    (0x000000FFu)
        #define TX_STEP_TIME_TX_STEP_TIME_MASK               (0x000000FFu)
        #define TX_STEP_TIME_TX_STEP_TIME_BIT                (0)
        #define TX_STEP_TIME_TX_STEP_TIME_BITS               (8)

#define GAIN_THRESH_MAX                                      *((volatile uint32_t *)0x40001068u)
#define GAIN_THRESH_MAX_REG                                  *((volatile uint32_t *)0x40001068u)
#define GAIN_THRESH_MAX_ADDR                                 (0x40001068u)
#define GAIN_THRESH_MAX_RESET                                (0x00000060u)
        /* GAIN_THRESH_MAX field */
        #define GAIN_THRESH_MAX_GAIN_THRESH_MAX              (0x000000FFu)
        #define GAIN_THRESH_MAX_GAIN_THRESH_MAX_MASK         (0x000000FFu)
        #define GAIN_THRESH_MAX_GAIN_THRESH_MAX_BIT          (0)
        #define GAIN_THRESH_MAX_GAIN_THRESH_MAX_BITS         (8)

#define GAIN_THRESH_MID                                      *((volatile uint32_t *)0x4000106Cu)
#define GAIN_THRESH_MID_REG                                  *((volatile uint32_t *)0x4000106Cu)
#define GAIN_THRESH_MID_ADDR                                 (0x4000106Cu)
#define GAIN_THRESH_MID_RESET                                (0x00000030u)
        /* GAIN_THRESH_MID field */
        #define GAIN_THRESH_MID_GAIN_THRESH_MID              (0x000000FFu)
        #define GAIN_THRESH_MID_GAIN_THRESH_MID_MASK         (0x000000FFu)
        #define GAIN_THRESH_MID_GAIN_THRESH_MID_BIT          (0)
        #define GAIN_THRESH_MID_GAIN_THRESH_MID_BITS         (8)

#define GAIN_THRESH_MIN                                      *((volatile uint32_t *)0x40001070u)
#define GAIN_THRESH_MIN_REG                                  *((volatile uint32_t *)0x40001070u)
#define GAIN_THRESH_MIN_ADDR                                 (0x40001070u)
#define GAIN_THRESH_MIN_RESET                                (0x00000018u)
        /* GAIN_THRESH_MIN field */
        #define GAIN_THRESH_MIN_GAIN_THRESH_MIN              (0x000000FFu)
        #define GAIN_THRESH_MIN_GAIN_THRESH_MIN_MASK         (0x000000FFu)
        #define GAIN_THRESH_MIN_GAIN_THRESH_MIN_BIT          (0)
        #define GAIN_THRESH_MIN_GAIN_THRESH_MIN_BITS         (8)

#define GAIN_SETTING_0                                       *((volatile uint32_t *)0x40001074u)
#define GAIN_SETTING_0_REG                                   *((volatile uint32_t *)0x40001074u)
#define GAIN_SETTING_0_ADDR                                  (0x40001074u)
#define GAIN_SETTING_0_RESET                                 (0x00000000u)
        /* RX_MIXER_GAIN_0 field */
        #define GAIN_SETTING_0_RX_MIXER_GAIN_0               (0x00000040u)
        #define GAIN_SETTING_0_RX_MIXER_GAIN_0_MASK          (0x00000040u)
        #define GAIN_SETTING_0_RX_MIXER_GAIN_0_BIT           (6)
        #define GAIN_SETTING_0_RX_MIXER_GAIN_0_BITS          (1)
        /* RX_FILTER_GAIN_0 field */
        #define GAIN_SETTING_0_RX_FILTER_GAIN_0              (0x00000030u)
        #define GAIN_SETTING_0_RX_FILTER_GAIN_0_MASK         (0x00000030u)
        #define GAIN_SETTING_0_RX_FILTER_GAIN_0_BIT          (4)
        #define GAIN_SETTING_0_RX_FILTER_GAIN_0_BITS         (2)
        /* RX_IF_GAIN_0 field */
        #define GAIN_SETTING_0_RX_IF_GAIN_0                  (0x0000000Fu)
        #define GAIN_SETTING_0_RX_IF_GAIN_0_MASK             (0x0000000Fu)
        #define GAIN_SETTING_0_RX_IF_GAIN_0_BIT              (0)
        #define GAIN_SETTING_0_RX_IF_GAIN_0_BITS             (4)

#define GAIN_SETTING_1                                       *((volatile uint32_t *)0x40001078u)
#define GAIN_SETTING_1_REG                                   *((volatile uint32_t *)0x40001078u)
#define GAIN_SETTING_1_ADDR                                  (0x40001078u)
#define GAIN_SETTING_1_RESET                                 (0x00000010u)
        /* RX_MIXER_GAIN_1 field */
        #define GAIN_SETTING_1_RX_MIXER_GAIN_1               (0x00000040u)
        #define GAIN_SETTING_1_RX_MIXER_GAIN_1_MASK          (0x00000040u)
        #define GAIN_SETTING_1_RX_MIXER_GAIN_1_BIT           (6)
        #define GAIN_SETTING_1_RX_MIXER_GAIN_1_BITS          (1)
        /* RX_FILTER_GAIN_1 field */
        #define GAIN_SETTING_1_RX_FILTER_GAIN_1              (0x00000030u)
        #define GAIN_SETTING_1_RX_FILTER_GAIN_1_MASK         (0x00000030u)
        #define GAIN_SETTING_1_RX_FILTER_GAIN_1_BIT          (4)
        #define GAIN_SETTING_1_RX_FILTER_GAIN_1_BITS         (2)
        /* RX_IF_GAIN_1 field */
        #define GAIN_SETTING_1_RX_IF_GAIN_1                  (0x0000000Fu)
        #define GAIN_SETTING_1_RX_IF_GAIN_1_MASK             (0x0000000Fu)
        #define GAIN_SETTING_1_RX_IF_GAIN_1_BIT              (0)
        #define GAIN_SETTING_1_RX_IF_GAIN_1_BITS             (4)

#define GAIN_SETTING_2                                       *((volatile uint32_t *)0x4000107Cu)
#define GAIN_SETTING_2_REG                                   *((volatile uint32_t *)0x4000107Cu)
#define GAIN_SETTING_2_ADDR                                  (0x4000107Cu)
#define GAIN_SETTING_2_RESET                                 (0x00000030u)
        /* RX_MIXER_GAIN_2 field */
        #define GAIN_SETTING_2_RX_MIXER_GAIN_2               (0x00000040u)
        #define GAIN_SETTING_2_RX_MIXER_GAIN_2_MASK          (0x00000040u)
        #define GAIN_SETTING_2_RX_MIXER_GAIN_2_BIT           (6)
        #define GAIN_SETTING_2_RX_MIXER_GAIN_2_BITS          (1)
        /* RX_FILTER_GAIN_2 field */
        #define GAIN_SETTING_2_RX_FILTER_GAIN_2              (0x00000030u)
        #define GAIN_SETTING_2_RX_FILTER_GAIN_2_MASK         (0x00000030u)
        #define GAIN_SETTING_2_RX_FILTER_GAIN_2_BIT          (4)
        #define GAIN_SETTING_2_RX_FILTER_GAIN_2_BITS         (2)
        /* RX_IF_GAIN_2 field */
        #define GAIN_SETTING_2_RX_IF_GAIN_2                  (0x0000000Fu)
        #define GAIN_SETTING_2_RX_IF_GAIN_2_MASK             (0x0000000Fu)
        #define GAIN_SETTING_2_RX_IF_GAIN_2_BIT              (0)
        #define GAIN_SETTING_2_RX_IF_GAIN_2_BITS             (4)

#define GAIN_SETTING_3                                       *((volatile uint32_t *)0x40001080u)
#define GAIN_SETTING_3_REG                                   *((volatile uint32_t *)0x40001080u)
#define GAIN_SETTING_3_ADDR                                  (0x40001080u)
#define GAIN_SETTING_3_RESET                                 (0x00000031u)
        /* RX_MIXER_GAIN_3 field */
        #define GAIN_SETTING_3_RX_MIXER_GAIN_3               (0x00000040u)
        #define GAIN_SETTING_3_RX_MIXER_GAIN_3_MASK          (0x00000040u)
        #define GAIN_SETTING_3_RX_MIXER_GAIN_3_BIT           (6)
        #define GAIN_SETTING_3_RX_MIXER_GAIN_3_BITS          (1)
        /* RX_FILTER_GAIN_3 field */
        #define GAIN_SETTING_3_RX_FILTER_GAIN_3              (0x00000030u)
        #define GAIN_SETTING_3_RX_FILTER_GAIN_3_MASK         (0x00000030u)
        #define GAIN_SETTING_3_RX_FILTER_GAIN_3_BIT          (4)
        #define GAIN_SETTING_3_RX_FILTER_GAIN_3_BITS         (2)
        /* RX_IF_GAIN_3 field */
        #define GAIN_SETTING_3_RX_IF_GAIN_3                  (0x0000000Fu)
        #define GAIN_SETTING_3_RX_IF_GAIN_3_MASK             (0x0000000Fu)
        #define GAIN_SETTING_3_RX_IF_GAIN_3_BIT              (0)
        #define GAIN_SETTING_3_RX_IF_GAIN_3_BITS             (4)

#define GAIN_SETTING_4                                       *((volatile uint32_t *)0x40001084u)
#define GAIN_SETTING_4_REG                                   *((volatile uint32_t *)0x40001084u)
#define GAIN_SETTING_4_ADDR                                  (0x40001084u)
#define GAIN_SETTING_4_RESET                                 (0x00000032u)
        /* RX_MIXER_GAIN_4 field */
        #define GAIN_SETTING_4_RX_MIXER_GAIN_4               (0x00000040u)
        #define GAIN_SETTING_4_RX_MIXER_GAIN_4_MASK          (0x00000040u)
        #define GAIN_SETTING_4_RX_MIXER_GAIN_4_BIT           (6)
        #define GAIN_SETTING_4_RX_MIXER_GAIN_4_BITS          (1)
        /* RX_FILTER_GAIN_4 field */
        #define GAIN_SETTING_4_RX_FILTER_GAIN_4              (0x00000030u)
        #define GAIN_SETTING_4_RX_FILTER_GAIN_4_MASK         (0x00000030u)
        #define GAIN_SETTING_4_RX_FILTER_GAIN_4_BIT          (4)
        #define GAIN_SETTING_4_RX_FILTER_GAIN_4_BITS         (2)
        /* RX_IF_GAIN_4 field */
        #define GAIN_SETTING_4_RX_IF_GAIN_4                  (0x0000000Fu)
        #define GAIN_SETTING_4_RX_IF_GAIN_4_MASK             (0x0000000Fu)
        #define GAIN_SETTING_4_RX_IF_GAIN_4_BIT              (0)
        #define GAIN_SETTING_4_RX_IF_GAIN_4_BITS             (4)

#define GAIN_SETTING_5                                       *((volatile uint32_t *)0x40001088u)
#define GAIN_SETTING_5_REG                                   *((volatile uint32_t *)0x40001088u)
#define GAIN_SETTING_5_ADDR                                  (0x40001088u)
#define GAIN_SETTING_5_RESET                                 (0x00000033u)
        /* RX_MIXER_GAIN_5 field */
        #define GAIN_SETTING_5_RX_MIXER_GAIN_5               (0x00000040u)
        #define GAIN_SETTING_5_RX_MIXER_GAIN_5_MASK          (0x00000040u)
        #define GAIN_SETTING_5_RX_MIXER_GAIN_5_BIT           (6)
        #define GAIN_SETTING_5_RX_MIXER_GAIN_5_BITS          (1)
        /* RX_FILTER_GAIN_5 field */
        #define GAIN_SETTING_5_RX_FILTER_GAIN_5              (0x00000030u)
        #define GAIN_SETTING_5_RX_FILTER_GAIN_5_MASK         (0x00000030u)
        #define GAIN_SETTING_5_RX_FILTER_GAIN_5_BIT          (4)
        #define GAIN_SETTING_5_RX_FILTER_GAIN_5_BITS         (2)
        /* RX_IF_GAIN_5 field */
        #define GAIN_SETTING_5_RX_IF_GAIN_5                  (0x0000000Fu)
        #define GAIN_SETTING_5_RX_IF_GAIN_5_MASK             (0x0000000Fu)
        #define GAIN_SETTING_5_RX_IF_GAIN_5_BIT              (0)
        #define GAIN_SETTING_5_RX_IF_GAIN_5_BITS             (4)

#define GAIN_SETTING_6                                       *((volatile uint32_t *)0x4000108Cu)
#define GAIN_SETTING_6_REG                                   *((volatile uint32_t *)0x4000108Cu)
#define GAIN_SETTING_6_ADDR                                  (0x4000108Cu)
#define GAIN_SETTING_6_RESET                                 (0x00000034u)
        /* RX_MIXER_GAIN_6 field */
        #define GAIN_SETTING_6_RX_MIXER_GAIN_6               (0x00000040u)
        #define GAIN_SETTING_6_RX_MIXER_GAIN_6_MASK          (0x00000040u)
        #define GAIN_SETTING_6_RX_MIXER_GAIN_6_BIT           (6)
        #define GAIN_SETTING_6_RX_MIXER_GAIN_6_BITS          (1)
        /* RX_FILTER_GAIN_6 field */
        #define GAIN_SETTING_6_RX_FILTER_GAIN_6              (0x00000030u)
        #define GAIN_SETTING_6_RX_FILTER_GAIN_6_MASK         (0x00000030u)
        #define GAIN_SETTING_6_RX_FILTER_GAIN_6_BIT          (4)
        #define GAIN_SETTING_6_RX_FILTER_GAIN_6_BITS         (2)
        /* RX_IF_GAIN_6 field */
        #define GAIN_SETTING_6_RX_IF_GAIN_6                  (0x0000000Fu)
        #define GAIN_SETTING_6_RX_IF_GAIN_6_MASK             (0x0000000Fu)
        #define GAIN_SETTING_6_RX_IF_GAIN_6_BIT              (0)
        #define GAIN_SETTING_6_RX_IF_GAIN_6_BITS             (4)

#define GAIN_SETTING_7                                       *((volatile uint32_t *)0x40001090u)
#define GAIN_SETTING_7_REG                                   *((volatile uint32_t *)0x40001090u)
#define GAIN_SETTING_7_ADDR                                  (0x40001090u)
#define GAIN_SETTING_7_RESET                                 (0x00000035u)
        /* RX_MIXER_GAIN_7 field */
        #define GAIN_SETTING_7_RX_MIXER_GAIN_7               (0x00000040u)
        #define GAIN_SETTING_7_RX_MIXER_GAIN_7_MASK          (0x00000040u)
        #define GAIN_SETTING_7_RX_MIXER_GAIN_7_BIT           (6)
        #define GAIN_SETTING_7_RX_MIXER_GAIN_7_BITS          (1)
        /* RX_FILTER_GAIN_7 field */
        #define GAIN_SETTING_7_RX_FILTER_GAIN_7              (0x00000030u)
        #define GAIN_SETTING_7_RX_FILTER_GAIN_7_MASK         (0x00000030u)
        #define GAIN_SETTING_7_RX_FILTER_GAIN_7_BIT          (4)
        #define GAIN_SETTING_7_RX_FILTER_GAIN_7_BITS         (2)
        /* RX_IF_GAIN_7 field */
        #define GAIN_SETTING_7_RX_IF_GAIN_7                  (0x0000000Fu)
        #define GAIN_SETTING_7_RX_IF_GAIN_7_MASK             (0x0000000Fu)
        #define GAIN_SETTING_7_RX_IF_GAIN_7_BIT              (0)
        #define GAIN_SETTING_7_RX_IF_GAIN_7_BITS             (4)

#define GAIN_SETTING_8                                       *((volatile uint32_t *)0x40001094u)
#define GAIN_SETTING_8_REG                                   *((volatile uint32_t *)0x40001094u)
#define GAIN_SETTING_8_ADDR                                  (0x40001094u)
#define GAIN_SETTING_8_RESET                                 (0x00000036u)
        /* RX_MIXER_GAIN_8 field */
        #define GAIN_SETTING_8_RX_MIXER_GAIN_8               (0x00000040u)
        #define GAIN_SETTING_8_RX_MIXER_GAIN_8_MASK          (0x00000040u)
        #define GAIN_SETTING_8_RX_MIXER_GAIN_8_BIT           (6)
        #define GAIN_SETTING_8_RX_MIXER_GAIN_8_BITS          (1)
        /* RX_FILTER_GAIN_8 field */
        #define GAIN_SETTING_8_RX_FILTER_GAIN_8              (0x00000030u)
        #define GAIN_SETTING_8_RX_FILTER_GAIN_8_MASK         (0x00000030u)
        #define GAIN_SETTING_8_RX_FILTER_GAIN_8_BIT          (4)
        #define GAIN_SETTING_8_RX_FILTER_GAIN_8_BITS         (2)
        /* RX_IF_GAIN_8 field */
        #define GAIN_SETTING_8_RX_IF_GAIN_8                  (0x0000000Fu)
        #define GAIN_SETTING_8_RX_IF_GAIN_8_MASK             (0x0000000Fu)
        #define GAIN_SETTING_8_RX_IF_GAIN_8_BIT              (0)
        #define GAIN_SETTING_8_RX_IF_GAIN_8_BITS             (4)

#define GAIN_SETTING_9                                       *((volatile uint32_t *)0x40001098u)
#define GAIN_SETTING_9_REG                                   *((volatile uint32_t *)0x40001098u)
#define GAIN_SETTING_9_ADDR                                  (0x40001098u)
#define GAIN_SETTING_9_RESET                                 (0x00000076u)
        /* RX_MIXER_GAIN_9 field */
        #define GAIN_SETTING_9_RX_MIXER_GAIN_9               (0x00000040u)
        #define GAIN_SETTING_9_RX_MIXER_GAIN_9_MASK          (0x00000040u)
        #define GAIN_SETTING_9_RX_MIXER_GAIN_9_BIT           (6)
        #define GAIN_SETTING_9_RX_MIXER_GAIN_9_BITS          (1)
        /* RX_FILTER_GAIN_9 field */
        #define GAIN_SETTING_9_RX_FILTER_GAIN_9              (0x00000030u)
        #define GAIN_SETTING_9_RX_FILTER_GAIN_9_MASK         (0x00000030u)
        #define GAIN_SETTING_9_RX_FILTER_GAIN_9_BIT          (4)
        #define GAIN_SETTING_9_RX_FILTER_GAIN_9_BITS         (2)
        /* RX_IF_GAIN_9 field */
        #define GAIN_SETTING_9_RX_IF_GAIN_9                  (0x0000000Fu)
        #define GAIN_SETTING_9_RX_IF_GAIN_9_MASK             (0x0000000Fu)
        #define GAIN_SETTING_9_RX_IF_GAIN_9_BIT              (0)
        #define GAIN_SETTING_9_RX_IF_GAIN_9_BITS             (4)

#define GAIN_SETTING_10                                      *((volatile uint32_t *)0x4000109Cu)
#define GAIN_SETTING_10_REG                                  *((volatile uint32_t *)0x4000109Cu)
#define GAIN_SETTING_10_ADDR                                 (0x4000109Cu)
#define GAIN_SETTING_10_RESET                                (0x00000077u)
        /* RX_MIXER_GAIN_10 field */
        #define GAIN_SETTING_10_RX_MIXER_GAIN_10             (0x00000040u)
        #define GAIN_SETTING_10_RX_MIXER_GAIN_10_MASK        (0x00000040u)
        #define GAIN_SETTING_10_RX_MIXER_GAIN_10_BIT         (6)
        #define GAIN_SETTING_10_RX_MIXER_GAIN_10_BITS        (1)
        /* RX_FILTER_GAIN_10 field */
        #define GAIN_SETTING_10_RX_FILTER_GAIN_10            (0x00000030u)
        #define GAIN_SETTING_10_RX_FILTER_GAIN_10_MASK       (0x00000030u)
        #define GAIN_SETTING_10_RX_FILTER_GAIN_10_BIT        (4)
        #define GAIN_SETTING_10_RX_FILTER_GAIN_10_BITS       (2)
        /* RX_IF_GAIN_10 field */
        #define GAIN_SETTING_10_RX_IF_GAIN_10                (0x0000000Fu)
        #define GAIN_SETTING_10_RX_IF_GAIN_10_MASK           (0x0000000Fu)
        #define GAIN_SETTING_10_RX_IF_GAIN_10_BIT            (0)
        #define GAIN_SETTING_10_RX_IF_GAIN_10_BITS           (4)

#define GAIN_SETTING_11                                      *((volatile uint32_t *)0x400010A0u)
#define GAIN_SETTING_11_REG                                  *((volatile uint32_t *)0x400010A0u)
#define GAIN_SETTING_11_ADDR                                 (0x400010A0u)
#define GAIN_SETTING_11_RESET                                (0x00000078u)
        /* RX_MIXER_GAIN_11 field */
        #define GAIN_SETTING_11_RX_MIXER_GAIN_11             (0x00000040u)
        #define GAIN_SETTING_11_RX_MIXER_GAIN_11_MASK        (0x00000040u)
        #define GAIN_SETTING_11_RX_MIXER_GAIN_11_BIT         (6)
        #define GAIN_SETTING_11_RX_MIXER_GAIN_11_BITS        (1)
        /* RX_FILTER_GAIN_11 field */
        #define GAIN_SETTING_11_RX_FILTER_GAIN_11            (0x00000030u)
        #define GAIN_SETTING_11_RX_FILTER_GAIN_11_MASK       (0x00000030u)
        #define GAIN_SETTING_11_RX_FILTER_GAIN_11_BIT        (4)
        #define GAIN_SETTING_11_RX_FILTER_GAIN_11_BITS       (2)
        /* RX_IF_GAIN_11 field */
        #define GAIN_SETTING_11_RX_IF_GAIN_11                (0x0000000Fu)
        #define GAIN_SETTING_11_RX_IF_GAIN_11_MASK           (0x0000000Fu)
        #define GAIN_SETTING_11_RX_IF_GAIN_11_BIT            (0)
        #define GAIN_SETTING_11_RX_IF_GAIN_11_BITS           (4)

#define GAIN_CTRL_MIN_RF                                     *((volatile uint32_t *)0x400010A4u)
#define GAIN_CTRL_MIN_RF_REG                                 *((volatile uint32_t *)0x400010A4u)
#define GAIN_CTRL_MIN_RF_ADDR                                (0x400010A4u)
#define GAIN_CTRL_MIN_RF_RESET                               (0x000000F0u)
        /* GAIN_CTRL_MIN_RF field */
        #define GAIN_CTRL_MIN_RF_GAIN_CTRL_MIN_RF            (0x000001FFu)
        #define GAIN_CTRL_MIN_RF_GAIN_CTRL_MIN_RF_MASK       (0x000001FFu)
        #define GAIN_CTRL_MIN_RF_GAIN_CTRL_MIN_RF_BIT        (0)
        #define GAIN_CTRL_MIN_RF_GAIN_CTRL_MIN_RF_BITS       (9)

#define GAIN_CTRL_MAX_RF                                     *((volatile uint32_t *)0x400010A8u)
#define GAIN_CTRL_MAX_RF_REG                                 *((volatile uint32_t *)0x400010A8u)
#define GAIN_CTRL_MAX_RF_ADDR                                (0x400010A8u)
#define GAIN_CTRL_MAX_RF_RESET                               (0x000000FCu)
        /* GAIN_CTRL_MAX_RF field */
        #define GAIN_CTRL_MAX_RF_GAIN_CTRL_MAX_RF            (0x000001FFu)
        #define GAIN_CTRL_MAX_RF_GAIN_CTRL_MAX_RF_MASK       (0x000001FFu)
        #define GAIN_CTRL_MAX_RF_GAIN_CTRL_MAX_RF_BIT        (0)
        #define GAIN_CTRL_MAX_RF_GAIN_CTRL_MAX_RF_BITS       (9)

#define MIXER_GAIN_STEP                                      *((volatile uint32_t *)0x400010ACu)
#define MIXER_GAIN_STEP_REG                                  *((volatile uint32_t *)0x400010ACu)
#define MIXER_GAIN_STEP_ADDR                                 (0x400010ACu)
#define MIXER_GAIN_STEP_RESET                                (0x0000000Cu)
        /* MIXER_GAIN_STEP field */
        #define MIXER_GAIN_STEP_MIXER_GAIN_STEP              (0x0000000Fu)
        #define MIXER_GAIN_STEP_MIXER_GAIN_STEP_MASK         (0x0000000Fu)
        #define MIXER_GAIN_STEP_MIXER_GAIN_STEP_BIT          (0)
        #define MIXER_GAIN_STEP_MIXER_GAIN_STEP_BITS         (4)

#define PREAMBLE_EVENT                                       *((volatile uint32_t *)0x400010B0u)
#define PREAMBLE_EVENT_REG                                   *((volatile uint32_t *)0x400010B0u)
#define PREAMBLE_EVENT_ADDR                                  (0x400010B0u)
#define PREAMBLE_EVENT_RESET                                 (0x00005877u)
        /* PREAMBLE_CONFIRM_THRESH field */
        #define PREAMBLE_EVENT_PREAMBLE_CONFIRM_THRESH       (0x0000FF00u)
        #define PREAMBLE_EVENT_PREAMBLE_CONFIRM_THRESH_MASK  (0x0000FF00u)
        #define PREAMBLE_EVENT_PREAMBLE_CONFIRM_THRESH_BIT   (8)
        #define PREAMBLE_EVENT_PREAMBLE_CONFIRM_THRESH_BITS  (8)
        /* PREAMBLE_EVENT_THRESH field */
        #define PREAMBLE_EVENT_PREAMBLE_EVENT_THRESH         (0x000000FFu)
        #define PREAMBLE_EVENT_PREAMBLE_EVENT_THRESH_MASK    (0x000000FFu)
        #define PREAMBLE_EVENT_PREAMBLE_EVENT_THRESH_BIT     (0)
        #define PREAMBLE_EVENT_PREAMBLE_EVENT_THRESH_BITS    (8)

#define PREAMBLE_ABORT_THRESH                                *((volatile uint32_t *)0x400010B4u)
#define PREAMBLE_ABORT_THRESH_REG                            *((volatile uint32_t *)0x400010B4u)
#define PREAMBLE_ABORT_THRESH_ADDR                           (0x400010B4u)
#define PREAMBLE_ABORT_THRESH_RESET                          (0x00000071u)
        /* PREAMBLE_ABORT_THRESH field */
        #define PREAMBLE_ABORT_THRESH_PREAMBLE_ABORT_THRESH  (0x000000FFu)
        #define PREAMBLE_ABORT_THRESH_PREAMBLE_ABORT_THRESH_MASK (0x000000FFu)
        #define PREAMBLE_ABORT_THRESH_PREAMBLE_ABORT_THRESH_BIT (0)
        #define PREAMBLE_ABORT_THRESH_PREAMBLE_ABORT_THRESH_BITS (8)

#define PREAMBLE_ACCEPT_WINDOW                               *((volatile uint32_t *)0x400010B8u)
#define PREAMBLE_ACCEPT_WINDOW_REG                           *((volatile uint32_t *)0x400010B8u)
#define PREAMBLE_ACCEPT_WINDOW_ADDR                          (0x400010B8u)
#define PREAMBLE_ACCEPT_WINDOW_RESET                         (0x00000003u)
        /* PREAMBLE_ACCEPT_WINDOW field */
        #define PREAMBLE_ACCEPT_WINDOW_PREAMBLE_ACCEPT_WINDOW (0x0000007Fu)
        #define PREAMBLE_ACCEPT_WINDOW_PREAMBLE_ACCEPT_WINDOW_MASK (0x0000007Fu)
        #define PREAMBLE_ACCEPT_WINDOW_PREAMBLE_ACCEPT_WINDOW_BIT (0)
        #define PREAMBLE_ACCEPT_WINDOW_PREAMBLE_ACCEPT_WINDOW_BITS (7)

#define CCA_MODE                                             *((volatile uint32_t *)0x400010BCu)
#define CCA_MODE_REG                                         *((volatile uint32_t *)0x400010BCu)
#define CCA_MODE_ADDR                                        (0x400010BCu)
#define CCA_MODE_RESET                                       (0x00000000u)
        /* CCA_MODE field */
        #define CCA_MODE_CCA_MODE                            (0x00000003u)
        #define CCA_MODE_CCA_MODE_MASK                       (0x00000003u)
        #define CCA_MODE_CCA_MODE_BIT                        (0)
        #define CCA_MODE_CCA_MODE_BITS                       (2)

#define TX_POWER_MAX                                         *((volatile uint32_t *)0x400010C0u)
#define TX_POWER_MAX_REG                                     *((volatile uint32_t *)0x400010C0u)
#define TX_POWER_MAX_ADDR                                    (0x400010C0u)
#define TX_POWER_MAX_RESET                                   (0x00000000u)
        /* MANUAL_POWER field */
        #define TX_POWER_MAX_MANUAL_POWER                    (0x00008000u)
        #define TX_POWER_MAX_MANUAL_POWER_MASK               (0x00008000u)
        #define TX_POWER_MAX_MANUAL_POWER_BIT                (15)
        #define TX_POWER_MAX_MANUAL_POWER_BITS               (1)
        /* TX_POWER_MAX field */
        #define TX_POWER_MAX_TX_POWER_MAX                    (0x0000001Fu)
        #define TX_POWER_MAX_TX_POWER_MAX_MASK               (0x0000001Fu)
        #define TX_POWER_MAX_TX_POWER_MAX_BIT                (0)
        #define TX_POWER_MAX_TX_POWER_MAX_BITS               (5)

#define SYNTH_FREQ_H                                         *((volatile uint32_t *)0x400010C4u)
#define SYNTH_FREQ_H_REG                                     *((volatile uint32_t *)0x400010C4u)
#define SYNTH_FREQ_H_ADDR                                    (0x400010C4u)
#define SYNTH_FREQ_H_RESET                                   (0x00000003u)
        /* SYNTH_FREQ_H field */
        #define SYNTH_FREQ_H_SYNTH_FREQ_H                    (0x00000003u)
        #define SYNTH_FREQ_H_SYNTH_FREQ_H_MASK               (0x00000003u)
        #define SYNTH_FREQ_H_SYNTH_FREQ_H_BIT                (0)
        #define SYNTH_FREQ_H_SYNTH_FREQ_H_BITS               (2)

#define SYNTH_FREQ_L                                         *((volatile uint32_t *)0x400010C8u)
#define SYNTH_FREQ_L_REG                                     *((volatile uint32_t *)0x400010C8u)
#define SYNTH_FREQ_L_ADDR                                    (0x400010C8u)
#define SYNTH_FREQ_L_RESET                                   (0x00003800u)
        /* SYNTH_FREQ_L field */
        #define SYNTH_FREQ_L_SYNTH_FREQ_L                    (0x0000FFFFu)
        #define SYNTH_FREQ_L_SYNTH_FREQ_L_MASK               (0x0000FFFFu)
        #define SYNTH_FREQ_L_SYNTH_FREQ_L_BIT                (0)
        #define SYNTH_FREQ_L_SYNTH_FREQ_L_BITS               (16)

#define RSSI_INST                                            *((volatile uint32_t *)0x400010CCu)
#define RSSI_INST_REG                                        *((volatile uint32_t *)0x400010CCu)
#define RSSI_INST_ADDR                                       (0x400010CCu)
#define RSSI_INST_RESET                                      (0x00000000u)
        /* NEW_RSSI_INST field */
        #define RSSI_INST_NEW_RSSI_INST                      (0x00000200u)
        #define RSSI_INST_NEW_RSSI_INST_MASK                 (0x00000200u)
        #define RSSI_INST_NEW_RSSI_INST_BIT                  (9)
        #define RSSI_INST_NEW_RSSI_INST_BITS                 (1)
        /* RSSI_INST field */
        #define RSSI_INST_RSSI_INST                          (0x000001FFu)
        #define RSSI_INST_RSSI_INST_MASK                     (0x000001FFu)
        #define RSSI_INST_RSSI_INST_BIT                      (0)
        #define RSSI_INST_RSSI_INST_BITS                     (9)

#define FREQ_MEAS_CTRL1                                      *((volatile uint32_t *)0x400010D0u)
#define FREQ_MEAS_CTRL1_REG                                  *((volatile uint32_t *)0x400010D0u)
#define FREQ_MEAS_CTRL1_ADDR                                 (0x400010D0u)
#define FREQ_MEAS_CTRL1_RESET                                (0x00000160u)
        /* AUTO_TUNE_EN field */
        #define FREQ_MEAS_CTRL1_AUTO_TUNE_EN                 (0x00008000u)
        #define FREQ_MEAS_CTRL1_AUTO_TUNE_EN_MASK            (0x00008000u)
        #define FREQ_MEAS_CTRL1_AUTO_TUNE_EN_BIT             (15)
        #define FREQ_MEAS_CTRL1_AUTO_TUNE_EN_BITS            (1)
        /* FREQ_MEAS_EN field */
        #define FREQ_MEAS_CTRL1_FREQ_MEAS_EN                 (0x00004000u)
        #define FREQ_MEAS_CTRL1_FREQ_MEAS_EN_MASK            (0x00004000u)
        #define FREQ_MEAS_CTRL1_FREQ_MEAS_EN_BIT             (14)
        #define FREQ_MEAS_CTRL1_FREQ_MEAS_EN_BITS            (1)
        /* OPEN_LOOP_MANUAL field */
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_MANUAL             (0x00002000u)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_MANUAL_MASK        (0x00002000u)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_MANUAL_BIT         (13)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_MANUAL_BITS        (1)
        /* OPEN_LOOP field */
        #define FREQ_MEAS_CTRL1_OPEN_LOOP                    (0x00001000u)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_MASK               (0x00001000u)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_BIT                (12)
        #define FREQ_MEAS_CTRL1_OPEN_LOOP_BITS               (1)
        /* DELAY_FIRST_MEAS field */
        #define FREQ_MEAS_CTRL1_DELAY_FIRST_MEAS             (0x00000400u)
        #define FREQ_MEAS_CTRL1_DELAY_FIRST_MEAS_MASK        (0x00000400u)
        #define FREQ_MEAS_CTRL1_DELAY_FIRST_MEAS_BIT         (10)
        #define FREQ_MEAS_CTRL1_DELAY_FIRST_MEAS_BITS        (1)
        /* DELAY_ALL_MEAS field */
        #define FREQ_MEAS_CTRL1_DELAY_ALL_MEAS               (0x00000200u)
        #define FREQ_MEAS_CTRL1_DELAY_ALL_MEAS_MASK          (0x00000200u)
        #define FREQ_MEAS_CTRL1_DELAY_ALL_MEAS_BIT           (9)
        #define FREQ_MEAS_CTRL1_DELAY_ALL_MEAS_BITS          (1)
        /* BIN_SEARCH_MSB field */
        #define FREQ_MEAS_CTRL1_BIN_SEARCH_MSB               (0x000001C0u)
        #define FREQ_MEAS_CTRL1_BIN_SEARCH_MSB_MASK          (0x000001C0u)
        #define FREQ_MEAS_CTRL1_BIN_SEARCH_MSB_BIT           (6)
        #define FREQ_MEAS_CTRL1_BIN_SEARCH_MSB_BITS          (3)
        /* TUNE_VCO_INIT field */
        #define FREQ_MEAS_CTRL1_TUNE_VCO_INIT                (0x0000003Fu)
        #define FREQ_MEAS_CTRL1_TUNE_VCO_INIT_MASK           (0x0000003Fu)
        #define FREQ_MEAS_CTRL1_TUNE_VCO_INIT_BIT            (0)
        #define FREQ_MEAS_CTRL1_TUNE_VCO_INIT_BITS           (6)

#define FREQ_MEAS_CTRL2                                      *((volatile uint32_t *)0x400010D4u)
#define FREQ_MEAS_CTRL2_REG                                  *((volatile uint32_t *)0x400010D4u)
#define FREQ_MEAS_CTRL2_ADDR                                 (0x400010D4u)
#define FREQ_MEAS_CTRL2_RESET                                (0x0000201Eu)
        /* FREQ_MEAS_TIMER field */
        #define FREQ_MEAS_CTRL2_FREQ_MEAS_TIMER              (0x0000FF00u)
        #define FREQ_MEAS_CTRL2_FREQ_MEAS_TIMER_MASK         (0x0000FF00u)
        #define FREQ_MEAS_CTRL2_FREQ_MEAS_TIMER_BIT          (8)
        #define FREQ_MEAS_CTRL2_FREQ_MEAS_TIMER_BITS         (8)
        /* TARGET_PERIOD field */
        #define FREQ_MEAS_CTRL2_TARGET_PERIOD                (0x000000FFu)
        #define FREQ_MEAS_CTRL2_TARGET_PERIOD_MASK           (0x000000FFu)
        #define FREQ_MEAS_CTRL2_TARGET_PERIOD_BIT            (0)
        #define FREQ_MEAS_CTRL2_TARGET_PERIOD_BITS           (8)

#define FREQ_MEAS_SHIFT                                      *((volatile uint32_t *)0x400010D8u)
#define FREQ_MEAS_SHIFT_REG                                  *((volatile uint32_t *)0x400010D8u)
#define FREQ_MEAS_SHIFT_ADDR                                 (0x400010D8u)
#define FREQ_MEAS_SHIFT_RESET                                (0x00000035u)
        /* FREQ_MEAS_SHIFT field */
        #define FREQ_MEAS_SHIFT_FREQ_MEAS_SHIFT              (0x000000FFu)
        #define FREQ_MEAS_SHIFT_FREQ_MEAS_SHIFT_MASK         (0x000000FFu)
        #define FREQ_MEAS_SHIFT_FREQ_MEAS_SHIFT_BIT          (0)
        #define FREQ_MEAS_SHIFT_FREQ_MEAS_SHIFT_BITS         (8)

#define FREQ_MEAS_STATUS1                                    *((volatile uint32_t *)0x400010DCu)
#define FREQ_MEAS_STATUS1_REG                                *((volatile uint32_t *)0x400010DCu)
#define FREQ_MEAS_STATUS1_ADDR                               (0x400010DCu)
#define FREQ_MEAS_STATUS1_RESET                              (0x00000000u)
        /* INVALID_EDGE field */
        #define FREQ_MEAS_STATUS1_INVALID_EDGE               (0x00008000u)
        #define FREQ_MEAS_STATUS1_INVALID_EDGE_MASK          (0x00008000u)
        #define FREQ_MEAS_STATUS1_INVALID_EDGE_BIT           (15)
        #define FREQ_MEAS_STATUS1_INVALID_EDGE_BITS          (1)
        /* SIGN_FOUND field */
        #define FREQ_MEAS_STATUS1_SIGN_FOUND                 (0x00004000u)
        #define FREQ_MEAS_STATUS1_SIGN_FOUND_MASK            (0x00004000u)
        #define FREQ_MEAS_STATUS1_SIGN_FOUND_BIT             (14)
        #define FREQ_MEAS_STATUS1_SIGN_FOUND_BITS            (1)
        /* FREQ_SIGN field */
        #define FREQ_MEAS_STATUS1_FREQ_SIGN                  (0x00002000u)
        #define FREQ_MEAS_STATUS1_FREQ_SIGN_MASK             (0x00002000u)
        #define FREQ_MEAS_STATUS1_FREQ_SIGN_BIT              (13)
        #define FREQ_MEAS_STATUS1_FREQ_SIGN_BITS             (1)
        /* PERIOD_FOUND field */
        #define FREQ_MEAS_STATUS1_PERIOD_FOUND               (0x00001000u)
        #define FREQ_MEAS_STATUS1_PERIOD_FOUND_MASK          (0x00001000u)
        #define FREQ_MEAS_STATUS1_PERIOD_FOUND_BIT           (12)
        #define FREQ_MEAS_STATUS1_PERIOD_FOUND_BITS          (1)
        /* NEAREST_DIFF field */
        #define FREQ_MEAS_STATUS1_NEAREST_DIFF               (0x000003FFu)
        #define FREQ_MEAS_STATUS1_NEAREST_DIFF_MASK          (0x000003FFu)
        #define FREQ_MEAS_STATUS1_NEAREST_DIFF_BIT           (0)
        #define FREQ_MEAS_STATUS1_NEAREST_DIFF_BITS          (10)

#define FREQ_MEAS_STATUS2                                    *((volatile uint32_t *)0x400010E0u)
#define FREQ_MEAS_STATUS2_REG                                *((volatile uint32_t *)0x400010E0u)
#define FREQ_MEAS_STATUS2_ADDR                               (0x400010E0u)
#define FREQ_MEAS_STATUS2_RESET                              (0x00000000u)
        /* BEAT_TIMER field */
        #define FREQ_MEAS_STATUS2_BEAT_TIMER                 (0x0000FFC0u)
        #define FREQ_MEAS_STATUS2_BEAT_TIMER_MASK            (0x0000FFC0u)
        #define FREQ_MEAS_STATUS2_BEAT_TIMER_BIT             (6)
        #define FREQ_MEAS_STATUS2_BEAT_TIMER_BITS            (10)
        /* BEATS field */
        #define FREQ_MEAS_STATUS2_BEATS                      (0x0000003Fu)
        #define FREQ_MEAS_STATUS2_BEATS_MASK                 (0x0000003Fu)
        #define FREQ_MEAS_STATUS2_BEATS_BIT                  (0)
        #define FREQ_MEAS_STATUS2_BEATS_BITS                 (6)

#define FREQ_MEAS_STATUS3                                    *((volatile uint32_t *)0x400010E4u)
#define FREQ_MEAS_STATUS3_REG                                *((volatile uint32_t *)0x400010E4u)
#define FREQ_MEAS_STATUS3_ADDR                               (0x400010E4u)
#define FREQ_MEAS_STATUS3_RESET                              (0x00000020u)
        /* TUNE_VCO field */
        #define FREQ_MEAS_STATUS3_TUNE_VCO                   (0x0000003Fu)
        #define FREQ_MEAS_STATUS3_TUNE_VCO_MASK              (0x0000003Fu)
        #define FREQ_MEAS_STATUS3_TUNE_VCO_BIT               (0)
        #define FREQ_MEAS_STATUS3_TUNE_VCO_BITS              (6)

#define SCR_CTRL                                             *((volatile uint32_t *)0x400010E8u)
#define SCR_CTRL_REG                                         *((volatile uint32_t *)0x400010E8u)
#define SCR_CTRL_ADDR                                        (0x400010E8u)
#define SCR_CTRL_RESET                                       (0x00000004u)
        /* SCR_RESET field */
        #define SCR_CTRL_SCR_RESET                           (0x00000004u)
        #define SCR_CTRL_SCR_RESET_MASK                      (0x00000004u)
        #define SCR_CTRL_SCR_RESET_BIT                       (2)
        #define SCR_CTRL_SCR_RESET_BITS                      (1)
        /* SCR_WRITE field */
        #define SCR_CTRL_SCR_WRITE                           (0x00000002u)
        #define SCR_CTRL_SCR_WRITE_MASK                      (0x00000002u)
        #define SCR_CTRL_SCR_WRITE_BIT                       (1)
        #define SCR_CTRL_SCR_WRITE_BITS                      (1)
        /* SCR_READ field */
        #define SCR_CTRL_SCR_READ                            (0x00000001u)
        #define SCR_CTRL_SCR_READ_MASK                       (0x00000001u)
        #define SCR_CTRL_SCR_READ_BIT                        (0)
        #define SCR_CTRL_SCR_READ_BITS                       (1)

#define SCR_BUSY                                             *((volatile uint32_t *)0x400010ECu)
#define SCR_BUSY_REG                                         *((volatile uint32_t *)0x400010ECu)
#define SCR_BUSY_ADDR                                        (0x400010ECu)
#define SCR_BUSY_RESET                                       (0x00000000u)
        /* SCR_BUSY field */
        #define SCR_BUSY_SCR_BUSY                            (0x00000001u)
        #define SCR_BUSY_SCR_BUSY_MASK                       (0x00000001u)
        #define SCR_BUSY_SCR_BUSY_BIT                        (0)
        #define SCR_BUSY_SCR_BUSY_BITS                       (1)

#define SCR_ADDR                                             *((volatile uint32_t *)0x400010F0u)
#define SCR_ADDR_REG                                         *((volatile uint32_t *)0x400010F0u)
#define SCR_ADDR_ADDR                                        (0x400010F0u)
#define SCR_ADDR_RESET                                       (0x00000000u)
        /* SCR_ADDR field */
        #define SCR_ADDR_SCR_ADDR                            (0x000000FFu)
        #define SCR_ADDR_SCR_ADDR_MASK                       (0x000000FFu)
        #define SCR_ADDR_SCR_ADDR_BIT                        (0)
        #define SCR_ADDR_SCR_ADDR_BITS                       (8)

#define SCR_WRITE                                            *((volatile uint32_t *)0x400010F4u)
#define SCR_WRITE_REG                                        *((volatile uint32_t *)0x400010F4u)
#define SCR_WRITE_ADDR                                       (0x400010F4u)
#define SCR_WRITE_RESET                                      (0x00000000u)
        /* SCR_WRITE field */
        #define SCR_WRITE_SCR_WRITE                          (0x0000FFFFu)
        #define SCR_WRITE_SCR_WRITE_MASK                     (0x0000FFFFu)
        #define SCR_WRITE_SCR_WRITE_BIT                      (0)
        #define SCR_WRITE_SCR_WRITE_BITS                     (16)

#define SCR_READ                                             *((volatile uint32_t *)0x400010F8u)
#define SCR_READ_REG                                         *((volatile uint32_t *)0x400010F8u)
#define SCR_READ_ADDR                                        (0x400010F8u)
#define SCR_READ_RESET                                       (0x00000000u)
        /* SCR_READ field */
        #define SCR_READ_SCR_READ                            (0x0000FFFFu)
        #define SCR_READ_SCR_READ_MASK                       (0x0000FFFFu)
        #define SCR_READ_SCR_READ_BIT                        (0)
        #define SCR_READ_SCR_READ_BITS                       (16)

#define SYNTH_LOCK                                           *((volatile uint32_t *)0x400010FCu)
#define SYNTH_LOCK_REG                                       *((volatile uint32_t *)0x400010FCu)
#define SYNTH_LOCK_ADDR                                      (0x400010FCu)
#define SYNTH_LOCK_RESET                                     (0x00000000u)
        /* IN_LOCK field */
        #define SYNTH_LOCK_IN_LOCK                           (0x00000001u)
        #define SYNTH_LOCK_IN_LOCK_MASK                      (0x00000001u)
        #define SYNTH_LOCK_IN_LOCK_BIT                       (0)
        #define SYNTH_LOCK_IN_LOCK_BITS                      (1)

#define AN_CAL_STATUS                                        *((volatile uint32_t *)0x40001100u)
#define AN_CAL_STATUS_REG                                    *((volatile uint32_t *)0x40001100u)
#define AN_CAL_STATUS_ADDR                                   (0x40001100u)
#define AN_CAL_STATUS_RESET                                  (0x00000000u)
        /* VCO_CTRL field */
        #define AN_CAL_STATUS_VCO_CTRL                       (0x0000000Cu)
        #define AN_CAL_STATUS_VCO_CTRL_MASK                  (0x0000000Cu)
        #define AN_CAL_STATUS_VCO_CTRL_BIT                   (2)
        #define AN_CAL_STATUS_VCO_CTRL_BITS                  (2)

#define BIAS_CAL_STATUS                                      *((volatile uint32_t *)0x40001104u)
#define BIAS_CAL_STATUS_REG                                  *((volatile uint32_t *)0x40001104u)
#define BIAS_CAL_STATUS_ADDR                                 (0x40001104u)
#define BIAS_CAL_STATUS_RESET                                (0x00000000u)
        /* VCOMP field */
        #define BIAS_CAL_STATUS_VCOMP                        (0x00000002u)
        #define BIAS_CAL_STATUS_VCOMP_MASK                   (0x00000002u)
        #define BIAS_CAL_STATUS_VCOMP_BIT                    (1)
        #define BIAS_CAL_STATUS_VCOMP_BITS                   (1)
        /* ICOMP field */
        #define BIAS_CAL_STATUS_ICOMP                        (0x00000001u)
        #define BIAS_CAL_STATUS_ICOMP_MASK                   (0x00000001u)
        #define BIAS_CAL_STATUS_ICOMP_BIT                    (0)
        #define BIAS_CAL_STATUS_ICOMP_BITS                   (1)

#define ATEST_SEL                                            *((volatile uint32_t *)0x40001108u)
#define ATEST_SEL_REG                                        *((volatile uint32_t *)0x40001108u)
#define ATEST_SEL_ADDR                                       (0x40001108u)
#define ATEST_SEL_RESET                                      (0x00000000u)
        /* ATEST_CTRL field */
        #define ATEST_SEL_ATEST_CTRL                         (0x0000FF00u)
        #define ATEST_SEL_ATEST_CTRL_MASK                    (0x0000FF00u)
        #define ATEST_SEL_ATEST_CTRL_BIT                     (8)
        #define ATEST_SEL_ATEST_CTRL_BITS                    (8)
        /* ATEST_SEL field */
        #define ATEST_SEL_ATEST_SEL                          (0x0000001Fu)
        #define ATEST_SEL_ATEST_SEL_MASK                     (0x0000001Fu)
        #define ATEST_SEL_ATEST_SEL_BIT                      (0)
        #define ATEST_SEL_ATEST_SEL_BITS                     (5)

#define AN_EN_TEST                                           *((volatile uint32_t *)0x4000110Cu)
#define AN_EN_TEST_REG                                       *((volatile uint32_t *)0x4000110Cu)
#define AN_EN_TEST_ADDR                                      (0x4000110Cu)
#define AN_EN_TEST_RESET                                     (0x00000000u)
        /* AN_TEST_MODE field */
        #define AN_EN_TEST_AN_TEST_MODE                      (0x00008000u)
        #define AN_EN_TEST_AN_TEST_MODE_MASK                 (0x00008000u)
        #define AN_EN_TEST_AN_TEST_MODE_BIT                  (15)
        #define AN_EN_TEST_AN_TEST_MODE_BITS                 (1)
        /* PFD_EN field */
        #define AN_EN_TEST_PFD_EN                            (0x00004000u)
        #define AN_EN_TEST_PFD_EN_MASK                       (0x00004000u)
        #define AN_EN_TEST_PFD_EN_BIT                        (14)
        #define AN_EN_TEST_PFD_EN_BITS                       (1)
        /* ADC_EN field */
        #define AN_EN_TEST_ADC_EN                            (0x00002000u)
        #define AN_EN_TEST_ADC_EN_MASK                       (0x00002000u)
        #define AN_EN_TEST_ADC_EN_BIT                        (13)
        #define AN_EN_TEST_ADC_EN_BITS                       (1)
        /* UNUSED field */
        #define AN_EN_TEST_UNUSED                            (0x00001000u)
        #define AN_EN_TEST_UNUSED_MASK                       (0x00001000u)
        #define AN_EN_TEST_UNUSED_BIT                        (12)
        #define AN_EN_TEST_UNUSED_BITS                       (1)
        /* PRE_FILT_EN field */
        #define AN_EN_TEST_PRE_FILT_EN                       (0x00000800u)
        #define AN_EN_TEST_PRE_FILT_EN_MASK                  (0x00000800u)
        #define AN_EN_TEST_PRE_FILT_EN_BIT                   (11)
        #define AN_EN_TEST_PRE_FILT_EN_BITS                  (1)
        /* IF_AMP_EN field */
        #define AN_EN_TEST_IF_AMP_EN                         (0x00000400u)
        #define AN_EN_TEST_IF_AMP_EN_MASK                    (0x00000400u)
        #define AN_EN_TEST_IF_AMP_EN_BIT                     (10)
        #define AN_EN_TEST_IF_AMP_EN_BITS                    (1)
        /* LNA_EN field */
        #define AN_EN_TEST_LNA_EN                            (0x00000200u)
        #define AN_EN_TEST_LNA_EN_MASK                       (0x00000200u)
        #define AN_EN_TEST_LNA_EN_BIT                        (9)
        #define AN_EN_TEST_LNA_EN_BITS                       (1)
        /* MIXER_EN field */
        #define AN_EN_TEST_MIXER_EN                          (0x00000100u)
        #define AN_EN_TEST_MIXER_EN_MASK                     (0x00000100u)
        #define AN_EN_TEST_MIXER_EN_BIT                      (8)
        #define AN_EN_TEST_MIXER_EN_BITS                     (1)
        /* CH_FILT_EN field */
        #define AN_EN_TEST_CH_FILT_EN                        (0x00000080u)
        #define AN_EN_TEST_CH_FILT_EN_MASK                   (0x00000080u)
        #define AN_EN_TEST_CH_FILT_EN_BIT                    (7)
        #define AN_EN_TEST_CH_FILT_EN_BITS                   (1)
        /* MOD_DAC_EN field */
        #define AN_EN_TEST_MOD_DAC_EN                        (0x00000040u)
        #define AN_EN_TEST_MOD_DAC_EN_MASK                   (0x00000040u)
        #define AN_EN_TEST_MOD_DAC_EN_BIT                    (6)
        #define AN_EN_TEST_MOD_DAC_EN_BITS                   (1)
        /* PA_EN field */
        #define AN_EN_TEST_PA_EN                             (0x00000010u)
        #define AN_EN_TEST_PA_EN_MASK                        (0x00000010u)
        #define AN_EN_TEST_PA_EN_BIT                         (4)
        #define AN_EN_TEST_PA_EN_BITS                        (1)
        /* PRESCALER_EN field */
        #define AN_EN_TEST_PRESCALER_EN                      (0x00000008u)
        #define AN_EN_TEST_PRESCALER_EN_MASK                 (0x00000008u)
        #define AN_EN_TEST_PRESCALER_EN_BIT                  (3)
        #define AN_EN_TEST_PRESCALER_EN_BITS                 (1)
        /* VCO_EN field */
        #define AN_EN_TEST_VCO_EN                            (0x00000004u)
        #define AN_EN_TEST_VCO_EN_MASK                       (0x00000004u)
        #define AN_EN_TEST_VCO_EN_BIT                        (2)
        #define AN_EN_TEST_VCO_EN_BITS                       (1)
        /* BIAS_EN field */
        #define AN_EN_TEST_BIAS_EN                           (0x00000001u)
        #define AN_EN_TEST_BIAS_EN_MASK                      (0x00000001u)
        #define AN_EN_TEST_BIAS_EN_BIT                       (0)
        #define AN_EN_TEST_BIAS_EN_BITS                      (1)

#define TUNE_FILTER_CTRL                                     *((volatile uint32_t *)0x40001110u)
#define TUNE_FILTER_CTRL_REG                                 *((volatile uint32_t *)0x40001110u)
#define TUNE_FILTER_CTRL_ADDR                                (0x40001110u)
#define TUNE_FILTER_CTRL_RESET                               (0x00000000u)
        /* TUNE_FILTER_EN field */
        #define TUNE_FILTER_CTRL_TUNE_FILTER_EN              (0x00000002u)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_EN_MASK         (0x00000002u)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_EN_BIT          (1)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_EN_BITS         (1)
        /* TUNE_FILTER_RESET field */
        #define TUNE_FILTER_CTRL_TUNE_FILTER_RESET           (0x00000001u)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_RESET_MASK      (0x00000001u)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_RESET_BIT       (0)
        #define TUNE_FILTER_CTRL_TUNE_FILTER_RESET_BITS      (1)

#define NOISE_EN                                             *((volatile uint32_t *)0x40001114u)
#define NOISE_EN_REG                                         *((volatile uint32_t *)0x40001114u)
#define NOISE_EN_ADDR                                        (0x40001114u)
#define NOISE_EN_RESET                                       (0x00000000u)
        /* NOISE_EN field */
        #define NOISE_EN_NOISE_EN                            (0x00000001u)
        #define NOISE_EN_NOISE_EN_MASK                       (0x00000001u)
        #define NOISE_EN_NOISE_EN_BIT                        (0)
        #define NOISE_EN_NOISE_EN_BITS                       (1)

/* MAC block */
#define DATA_MAC_BASE                                        (0x40002000u)
#define DATA_MAC_END                                         (0x400020C8u)
#define DATA_MAC_SIZE                                        (DATA_MAC_END - DATA_MAC_BASE + 1)

#define MAC_RX_ST_ADDR_A                                     *((volatile uint32_t *)0x40002000u)
#define MAC_RX_ST_ADDR_A_REG                                 *((volatile uint32_t *)0x40002000u)
#define MAC_RX_ST_ADDR_A_ADDR                                (0x40002000u)
#define MAC_RX_ST_ADDR_A_RESET                               (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_RX_ST_ADDR_A_MAC_RAM_OFFS                (0xFFFFE000u)
        #define MAC_RX_ST_ADDR_A_MAC_RAM_OFFS_MASK           (0xFFFFE000u)
        #define MAC_RX_ST_ADDR_A_MAC_RAM_OFFS_BIT            (13)
        #define MAC_RX_ST_ADDR_A_MAC_RAM_OFFS_BITS           (19)
        /* MAC_RX_ST_ADDR_A field */
        #define MAC_RX_ST_ADDR_A_MAC_RX_ST_ADDR_A            (0x00001FFEu)
        #define MAC_RX_ST_ADDR_A_MAC_RX_ST_ADDR_A_MASK       (0x00001FFEu)
        #define MAC_RX_ST_ADDR_A_MAC_RX_ST_ADDR_A_BIT        (1)
        #define MAC_RX_ST_ADDR_A_MAC_RX_ST_ADDR_A_BITS       (12)

#define MAC_RX_END_ADDR_A                                    *((volatile uint32_t *)0x40002004u)
#define MAC_RX_END_ADDR_A_REG                                *((volatile uint32_t *)0x40002004u)
#define MAC_RX_END_ADDR_A_ADDR                               (0x40002004u)
#define MAC_RX_END_ADDR_A_RESET                              (0x20000088u)
        /* MAC_RAM_OFFS field */
        #define MAC_RX_END_ADDR_A_MAC_RAM_OFFS               (0xFFFFE000u)
        #define MAC_RX_END_ADDR_A_MAC_RAM_OFFS_MASK          (0xFFFFE000u)
        #define MAC_RX_END_ADDR_A_MAC_RAM_OFFS_BIT           (13)
        #define MAC_RX_END_ADDR_A_MAC_RAM_OFFS_BITS          (19)
        /* MAC_RX_END_ADDR_A field */
        #define MAC_RX_END_ADDR_A_MAC_RX_END_ADDR_A          (0x00001FFEu)
        #define MAC_RX_END_ADDR_A_MAC_RX_END_ADDR_A_MASK     (0x00001FFEu)
        #define MAC_RX_END_ADDR_A_MAC_RX_END_ADDR_A_BIT      (1)
        #define MAC_RX_END_ADDR_A_MAC_RX_END_ADDR_A_BITS     (12)

#define MAC_RX_ST_ADDR_B                                     *((volatile uint32_t *)0x40002008u)
#define MAC_RX_ST_ADDR_B_REG                                 *((volatile uint32_t *)0x40002008u)
#define MAC_RX_ST_ADDR_B_ADDR                                (0x40002008u)
#define MAC_RX_ST_ADDR_B_RESET                               (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_RX_ST_ADDR_B_MAC_RAM_OFFS                (0xFFFFE000u)
        #define MAC_RX_ST_ADDR_B_MAC_RAM_OFFS_MASK           (0xFFFFE000u)
        #define MAC_RX_ST_ADDR_B_MAC_RAM_OFFS_BIT            (13)
        #define MAC_RX_ST_ADDR_B_MAC_RAM_OFFS_BITS           (19)
        /* MAC_RX_ST_ADDR_B field */
        #define MAC_RX_ST_ADDR_B_MAC_RX_ST_ADDR_B            (0x00001FFEu)
        #define MAC_RX_ST_ADDR_B_MAC_RX_ST_ADDR_B_MASK       (0x00001FFEu)
        #define MAC_RX_ST_ADDR_B_MAC_RX_ST_ADDR_B_BIT        (1)
        #define MAC_RX_ST_ADDR_B_MAC_RX_ST_ADDR_B_BITS       (12)

#define MAC_RX_END_ADDR_B                                    *((volatile uint32_t *)0x4000200Cu)
#define MAC_RX_END_ADDR_B_REG                                *((volatile uint32_t *)0x4000200Cu)
#define MAC_RX_END_ADDR_B_ADDR                               (0x4000200Cu)
#define MAC_RX_END_ADDR_B_RESET                              (0x20000088u)
        /* MAC_RAM_OFFS field */
        #define MAC_RX_END_ADDR_B_MAC_RAM_OFFS               (0xFFFFE000u)
        #define MAC_RX_END_ADDR_B_MAC_RAM_OFFS_MASK          (0xFFFFE000u)
        #define MAC_RX_END_ADDR_B_MAC_RAM_OFFS_BIT           (13)
        #define MAC_RX_END_ADDR_B_MAC_RAM_OFFS_BITS          (19)
        /* MAC_RX_END_ADDR_B field */
        #define MAC_RX_END_ADDR_B_MAC_RX_END_ADDR_B          (0x00001FFEu)
        #define MAC_RX_END_ADDR_B_MAC_RX_END_ADDR_B_MASK     (0x00001FFEu)
        #define MAC_RX_END_ADDR_B_MAC_RX_END_ADDR_B_BIT      (1)
        #define MAC_RX_END_ADDR_B_MAC_RX_END_ADDR_B_BITS     (12)

#define MAC_TX_ST_ADDR_A                                     *((volatile uint32_t *)0x40002010u)
#define MAC_TX_ST_ADDR_A_REG                                 *((volatile uint32_t *)0x40002010u)
#define MAC_TX_ST_ADDR_A_ADDR                                (0x40002010u)
#define MAC_TX_ST_ADDR_A_RESET                               (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_TX_ST_ADDR_A_MAC_RAM_OFFS                (0xFFFFE000u)
        #define MAC_TX_ST_ADDR_A_MAC_RAM_OFFS_MASK           (0xFFFFE000u)
        #define MAC_TX_ST_ADDR_A_MAC_RAM_OFFS_BIT            (13)
        #define MAC_TX_ST_ADDR_A_MAC_RAM_OFFS_BITS           (19)
        /* MAC_TX_ST_ADDR_A field */
        #define MAC_TX_ST_ADDR_A_MAC_TX_ST_ADDR_A            (0x00001FFEu)
        #define MAC_TX_ST_ADDR_A_MAC_TX_ST_ADDR_A_MASK       (0x00001FFEu)
        #define MAC_TX_ST_ADDR_A_MAC_TX_ST_ADDR_A_BIT        (1)
        #define MAC_TX_ST_ADDR_A_MAC_TX_ST_ADDR_A_BITS       (12)

#define MAC_TX_END_ADDR_A                                    *((volatile uint32_t *)0x40002014u)
#define MAC_TX_END_ADDR_A_REG                                *((volatile uint32_t *)0x40002014u)
#define MAC_TX_END_ADDR_A_ADDR                               (0x40002014u)
#define MAC_TX_END_ADDR_A_RESET                              (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_TX_END_ADDR_A_MAC_RAM_OFFS               (0xFFFFE000u)
        #define MAC_TX_END_ADDR_A_MAC_RAM_OFFS_MASK          (0xFFFFE000u)
        #define MAC_TX_END_ADDR_A_MAC_RAM_OFFS_BIT           (13)
        #define MAC_TX_END_ADDR_A_MAC_RAM_OFFS_BITS          (19)
        /* MAC_TX_END_ADDR_A field */
        #define MAC_TX_END_ADDR_A_MAC_TX_END_ADDR_A          (0x00001FFEu)
        #define MAC_TX_END_ADDR_A_MAC_TX_END_ADDR_A_MASK     (0x00001FFEu)
        #define MAC_TX_END_ADDR_A_MAC_TX_END_ADDR_A_BIT      (1)
        #define MAC_TX_END_ADDR_A_MAC_TX_END_ADDR_A_BITS     (12)

#define MAC_TX_ST_ADDR_B                                     *((volatile uint32_t *)0x40002018u)
#define MAC_TX_ST_ADDR_B_REG                                 *((volatile uint32_t *)0x40002018u)
#define MAC_TX_ST_ADDR_B_ADDR                                (0x40002018u)
#define MAC_TX_ST_ADDR_B_RESET                               (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_TX_ST_ADDR_B_MAC_RAM_OFFS                (0xFFFFE000u)
        #define MAC_TX_ST_ADDR_B_MAC_RAM_OFFS_MASK           (0xFFFFE000u)
        #define MAC_TX_ST_ADDR_B_MAC_RAM_OFFS_BIT            (13)
        #define MAC_TX_ST_ADDR_B_MAC_RAM_OFFS_BITS           (19)
        /* MAC_TX_ST_ADDR_B field */
        #define MAC_TX_ST_ADDR_B_MAC_TX_ST_ADDR_B            (0x00001FFEu)
        #define MAC_TX_ST_ADDR_B_MAC_TX_ST_ADDR_B_MASK       (0x00001FFEu)
        #define MAC_TX_ST_ADDR_B_MAC_TX_ST_ADDR_B_BIT        (1)
        #define MAC_TX_ST_ADDR_B_MAC_TX_ST_ADDR_B_BITS       (12)

#define MAC_TX_END_ADDR_B                                    *((volatile uint32_t *)0x4000201Cu)
#define MAC_TX_END_ADDR_B_REG                                *((volatile uint32_t *)0x4000201Cu)
#define MAC_TX_END_ADDR_B_ADDR                               (0x4000201Cu)
#define MAC_TX_END_ADDR_B_RESET                              (0x20000000u)
        /* MAC_RAM_OFFS field */
        #define MAC_TX_END_ADDR_B_MAC_RAM_OFFS               (0xFFFFE000u)
        #define MAC_TX_END_ADDR_B_MAC_RAM_OFFS_MASK          (0xFFFFE000u)
        #define MAC_TX_END_ADDR_B_MAC_RAM_OFFS_BIT           (13)
        #define MAC_TX_END_ADDR_B_MAC_RAM_OFFS_BITS          (19)
        /* MAC_TX_END_ADDR_B field */
        #define MAC_TX_END_ADDR_B_MAC_TX_END_ADDR_B          (0x00001FFEu)
        #define MAC_TX_END_ADDR_B_MAC_TX_END_ADDR_B_MASK     (0x00001FFEu)
        #define MAC_TX_END_ADDR_B_MAC_TX_END_ADDR_B_BIT      (1)
        #define MAC_TX_END_ADDR_B_MAC_TX_END_ADDR_B_BITS     (12)

#define RX_A_COUNT                                           *((volatile uint32_t *)0x40002020u)
#define RX_A_COUNT_REG                                       *((volatile uint32_t *)0x40002020u)
#define RX_A_COUNT_ADDR                                      (0x40002020u)
#define RX_A_COUNT_RESET                                     (0x00000000u)
        /* RX_A_COUNT field */
        #define RX_A_COUNT_RX_A_COUNT                        (0x000007FFu)
        #define RX_A_COUNT_RX_A_COUNT_MASK                   (0x000007FFu)
        #define RX_A_COUNT_RX_A_COUNT_BIT                    (0)
        #define RX_A_COUNT_RX_A_COUNT_BITS                   (11)

#define RX_B_COUNT                                           *((volatile uint32_t *)0x40002024u)
#define RX_B_COUNT_REG                                       *((volatile uint32_t *)0x40002024u)
#define RX_B_COUNT_ADDR                                      (0x40002024u)
#define RX_B_COUNT_RESET                                     (0x00000000u)
        /* RX_B_COUNT field */
        #define RX_B_COUNT_RX_B_COUNT                        (0x000007FFu)
        #define RX_B_COUNT_RX_B_COUNT_MASK                   (0x000007FFu)
        #define RX_B_COUNT_RX_B_COUNT_BIT                    (0)
        #define RX_B_COUNT_RX_B_COUNT_BITS                   (11)

#define TX_COUNT                                             *((volatile uint32_t *)0x40002028u)
#define TX_COUNT_REG                                         *((volatile uint32_t *)0x40002028u)
#define TX_COUNT_ADDR                                        (0x40002028u)
#define TX_COUNT_RESET                                       (0x00000000u)
        /* TX_COUNT field */
        #define TX_COUNT_TX_COUNT                            (0x000007FFu)
        #define TX_COUNT_TX_COUNT_MASK                       (0x000007FFu)
        #define TX_COUNT_TX_COUNT_BIT                        (0)
        #define TX_COUNT_TX_COUNT_BITS                       (11)

#define MAC_DMA_STATUS                                       *((volatile uint32_t *)0x4000202Cu)
#define MAC_DMA_STATUS_REG                                   *((volatile uint32_t *)0x4000202Cu)
#define MAC_DMA_STATUS_ADDR                                  (0x4000202Cu)
#define MAC_DMA_STATUS_RESET                                 (0x00000000u)
        /* TX_ACTIVE_B field */
        #define MAC_DMA_STATUS_TX_ACTIVE_B                   (0x00000008u)
        #define MAC_DMA_STATUS_TX_ACTIVE_B_MASK              (0x00000008u)
        #define MAC_DMA_STATUS_TX_ACTIVE_B_BIT               (3)
        #define MAC_DMA_STATUS_TX_ACTIVE_B_BITS              (1)
        /* TX_ACTIVE_A field */
        #define MAC_DMA_STATUS_TX_ACTIVE_A                   (0x00000004u)
        #define MAC_DMA_STATUS_TX_ACTIVE_A_MASK              (0x00000004u)
        #define MAC_DMA_STATUS_TX_ACTIVE_A_BIT               (2)
        #define MAC_DMA_STATUS_TX_ACTIVE_A_BITS              (1)
        /* RX_ACTIVE_B field */
        #define MAC_DMA_STATUS_RX_ACTIVE_B                   (0x00000002u)
        #define MAC_DMA_STATUS_RX_ACTIVE_B_MASK              (0x00000002u)
        #define MAC_DMA_STATUS_RX_ACTIVE_B_BIT               (1)
        #define MAC_DMA_STATUS_RX_ACTIVE_B_BITS              (1)
        /* RX_ACTIVE_A field */
        #define MAC_DMA_STATUS_RX_ACTIVE_A                   (0x00000001u)
        #define MAC_DMA_STATUS_RX_ACTIVE_A_MASK              (0x00000001u)
        #define MAC_DMA_STATUS_RX_ACTIVE_A_BIT               (0)
        #define MAC_DMA_STATUS_RX_ACTIVE_A_BITS              (1)

#define MAC_DMA_CONFIG                                       *((volatile uint32_t *)0x40002030u)
#define MAC_DMA_CONFIG_REG                                   *((volatile uint32_t *)0x40002030u)
#define MAC_DMA_CONFIG_ADDR                                  (0x40002030u)
#define MAC_DMA_CONFIG_RESET                                 (0x00000000u)
        /* TX_DMA_RESET field */
        #define MAC_DMA_CONFIG_TX_DMA_RESET                  (0x00000020u)
        #define MAC_DMA_CONFIG_TX_DMA_RESET_MASK             (0x00000020u)
        #define MAC_DMA_CONFIG_TX_DMA_RESET_BIT              (5)
        #define MAC_DMA_CONFIG_TX_DMA_RESET_BITS             (1)
        /* RX_DMA_RESET field */
        #define MAC_DMA_CONFIG_RX_DMA_RESET                  (0x00000010u)
        #define MAC_DMA_CONFIG_RX_DMA_RESET_MASK             (0x00000010u)
        #define MAC_DMA_CONFIG_RX_DMA_RESET_BIT              (4)
        #define MAC_DMA_CONFIG_RX_DMA_RESET_BITS             (1)
        /* TX_LOAD_B field */
        #define MAC_DMA_CONFIG_TX_LOAD_B                     (0x00000008u)
        #define MAC_DMA_CONFIG_TX_LOAD_B_MASK                (0x00000008u)
        #define MAC_DMA_CONFIG_TX_LOAD_B_BIT                 (3)
        #define MAC_DMA_CONFIG_TX_LOAD_B_BITS                (1)
        /* TX_LOAD_A field */
        #define MAC_DMA_CONFIG_TX_LOAD_A                     (0x00000004u)
        #define MAC_DMA_CONFIG_TX_LOAD_A_MASK                (0x00000004u)
        #define MAC_DMA_CONFIG_TX_LOAD_A_BIT                 (2)
        #define MAC_DMA_CONFIG_TX_LOAD_A_BITS                (1)
        /* RX_LOAD_B field */
        #define MAC_DMA_CONFIG_RX_LOAD_B                     (0x00000002u)
        #define MAC_DMA_CONFIG_RX_LOAD_B_MASK                (0x00000002u)
        #define MAC_DMA_CONFIG_RX_LOAD_B_BIT                 (1)
        #define MAC_DMA_CONFIG_RX_LOAD_B_BITS                (1)
        /* RX_LOAD_A field */
        #define MAC_DMA_CONFIG_RX_LOAD_A                     (0x00000001u)
        #define MAC_DMA_CONFIG_RX_LOAD_A_MASK                (0x00000001u)
        #define MAC_DMA_CONFIG_RX_LOAD_A_BIT                 (0)
        #define MAC_DMA_CONFIG_RX_LOAD_A_BITS                (1)

#define MAC_TIMER                                            *((volatile uint32_t *)0x40002038u)
#define MAC_TIMER_REG                                        *((volatile uint32_t *)0x40002038u)
#define MAC_TIMER_ADDR                                       (0x40002038u)
#define MAC_TIMER_RESET                                      (0x00000000u)
        /* MAC_TIMER field */
        #define MAC_TIMER_MAC_TIMER                          (0x000FFFFFu)
        #define MAC_TIMER_MAC_TIMER_MASK                     (0x000FFFFFu)
        #define MAC_TIMER_MAC_TIMER_BIT                      (0)
        #define MAC_TIMER_MAC_TIMER_BITS                     (20)

#define MAC_TIMER_COMPARE_A_H                                *((volatile uint32_t *)0x40002040u)
#define MAC_TIMER_COMPARE_A_H_REG                            *((volatile uint32_t *)0x40002040u)
#define MAC_TIMER_COMPARE_A_H_ADDR                           (0x40002040u)
#define MAC_TIMER_COMPARE_A_H_RESET                          (0x00000000u)
        /* MAC_COMPARE_A_H field */
        #define MAC_TIMER_COMPARE_A_H_MAC_COMPARE_A_H        (0x0000000Fu)
        #define MAC_TIMER_COMPARE_A_H_MAC_COMPARE_A_H_MASK   (0x0000000Fu)
        #define MAC_TIMER_COMPARE_A_H_MAC_COMPARE_A_H_BIT    (0)
        #define MAC_TIMER_COMPARE_A_H_MAC_COMPARE_A_H_BITS   (4)

#define MAC_TIMER_COMPARE_A_L                                *((volatile uint32_t *)0x40002044u)
#define MAC_TIMER_COMPARE_A_L_REG                            *((volatile uint32_t *)0x40002044u)
#define MAC_TIMER_COMPARE_A_L_ADDR                           (0x40002044u)
#define MAC_TIMER_COMPARE_A_L_RESET                          (0x00000000u)
        /* MAC_COMPARE_A_L field */
        #define MAC_TIMER_COMPARE_A_L_MAC_COMPARE_A_L        (0x0000FFFFu)
        #define MAC_TIMER_COMPARE_A_L_MAC_COMPARE_A_L_MASK   (0x0000FFFFu)
        #define MAC_TIMER_COMPARE_A_L_MAC_COMPARE_A_L_BIT    (0)
        #define MAC_TIMER_COMPARE_A_L_MAC_COMPARE_A_L_BITS   (16)

#define MAC_TIMER_COMPARE_B_H                                *((volatile uint32_t *)0x40002048u)
#define MAC_TIMER_COMPARE_B_H_REG                            *((volatile uint32_t *)0x40002048u)
#define MAC_TIMER_COMPARE_B_H_ADDR                           (0x40002048u)
#define MAC_TIMER_COMPARE_B_H_RESET                          (0x00000000u)
        /* MAC_COMPARE_B_H field */
        #define MAC_TIMER_COMPARE_B_H_MAC_COMPARE_B_H        (0x0000000Fu)
        #define MAC_TIMER_COMPARE_B_H_MAC_COMPARE_B_H_MASK   (0x0000000Fu)
        #define MAC_TIMER_COMPARE_B_H_MAC_COMPARE_B_H_BIT    (0)
        #define MAC_TIMER_COMPARE_B_H_MAC_COMPARE_B_H_BITS   (4)

#define MAC_TIMER_COMPARE_B_L                                *((volatile uint32_t *)0x4000204Cu)
#define MAC_TIMER_COMPARE_B_L_REG                            *((volatile uint32_t *)0x4000204Cu)
#define MAC_TIMER_COMPARE_B_L_ADDR                           (0x4000204Cu)
#define MAC_TIMER_COMPARE_B_L_RESET                          (0x00000000u)
        /* MAC_COMPARE_B_L field */
        #define MAC_TIMER_COMPARE_B_L_MAC_COMPARE_B_L        (0x0000FFFFu)
        #define MAC_TIMER_COMPARE_B_L_MAC_COMPARE_B_L_MASK   (0x0000FFFFu)
        #define MAC_TIMER_COMPARE_B_L_MAC_COMPARE_B_L_BIT    (0)
        #define MAC_TIMER_COMPARE_B_L_MAC_COMPARE_B_L_BITS   (16)

#define MAC_TIMER_CAPTURE_H                                  *((volatile uint32_t *)0x40002050u)
#define MAC_TIMER_CAPTURE_H_REG                              *((volatile uint32_t *)0x40002050u)
#define MAC_TIMER_CAPTURE_H_ADDR                             (0x40002050u)
#define MAC_TIMER_CAPTURE_H_RESET                            (0x00000000u)
        /* MAC_SFD_CAPTURE_HIGH field */
        #define MAC_TIMER_CAPTURE_H_MAC_SFD_CAPTURE_HIGH     (0x0000000Fu)
        #define MAC_TIMER_CAPTURE_H_MAC_SFD_CAPTURE_HIGH_MASK (0x0000000Fu)
        #define MAC_TIMER_CAPTURE_H_MAC_SFD_CAPTURE_HIGH_BIT (0)
        #define MAC_TIMER_CAPTURE_H_MAC_SFD_CAPTURE_HIGH_BITS (4)

#define MAC_TIMER_CAPTURE_L                                  *((volatile uint32_t *)0x40002054u)
#define MAC_TIMER_CAPTURE_L_REG                              *((volatile uint32_t *)0x40002054u)
#define MAC_TIMER_CAPTURE_L_ADDR                             (0x40002054u)
#define MAC_TIMER_CAPTURE_L_RESET                            (0x00000000u)
        /* MAC_SFD_CAPTURE_LOW field */
        #define MAC_TIMER_CAPTURE_L_MAC_SFD_CAPTURE_LOW      (0x0000FFFFu)
        #define MAC_TIMER_CAPTURE_L_MAC_SFD_CAPTURE_LOW_MASK (0x0000FFFFu)
        #define MAC_TIMER_CAPTURE_L_MAC_SFD_CAPTURE_LOW_BIT  (0)
        #define MAC_TIMER_CAPTURE_L_MAC_SFD_CAPTURE_LOW_BITS (16)

#define MAC_BO_TIMER                                         *((volatile uint32_t *)0x40002058u)
#define MAC_BO_TIMER_REG                                     *((volatile uint32_t *)0x40002058u)
#define MAC_BO_TIMER_ADDR                                    (0x40002058u)
#define MAC_BO_TIMER_RESET                                   (0x00000000u)
        /* MAC_BO_TIMER field */
        #define MAC_BO_TIMER_MAC_BO_TIMER                    (0x00000FFFu)
        #define MAC_BO_TIMER_MAC_BO_TIMER_MASK               (0x00000FFFu)
        #define MAC_BO_TIMER_MAC_BO_TIMER_BIT                (0)
        #define MAC_BO_TIMER_MAC_BO_TIMER_BITS               (12)

#define MAC_BOP_TIMER                                        *((volatile uint32_t *)0x4000205Cu)
#define MAC_BOP_TIMER_REG                                    *((volatile uint32_t *)0x4000205Cu)
#define MAC_BOP_TIMER_ADDR                                   (0x4000205Cu)
#define MAC_BOP_TIMER_RESET                                  (0x00000000u)
        /* MAC_BOP_TIMER field */
        #define MAC_BOP_TIMER_MAC_BOP_TIMER                  (0x0000007Fu)
        #define MAC_BOP_TIMER_MAC_BOP_TIMER_MASK             (0x0000007Fu)
        #define MAC_BOP_TIMER_MAC_BOP_TIMER_BIT              (0)
        #define MAC_BOP_TIMER_MAC_BOP_TIMER_BITS             (7)

#define MAC_TX_STROBE                                        *((volatile uint32_t *)0x40002060u)
#define MAC_TX_STROBE_REG                                    *((volatile uint32_t *)0x40002060u)
#define MAC_TX_STROBE_ADDR                                   (0x40002060u)
#define MAC_TX_STROBE_RESET                                  (0x00000000u)
        /* AUTO_CRC_TX field */
        #define MAC_TX_STROBE_AUTO_CRC_TX                    (0x00000008u)
        #define MAC_TX_STROBE_AUTO_CRC_TX_MASK               (0x00000008u)
        #define MAC_TX_STROBE_AUTO_CRC_TX_BIT                (3)
        #define MAC_TX_STROBE_AUTO_CRC_TX_BITS               (1)
        /* CCA_ON field */
        #define MAC_TX_STROBE_CCA_ON                         (0x00000004u)
        #define MAC_TX_STROBE_CCA_ON_MASK                    (0x00000004u)
        #define MAC_TX_STROBE_CCA_ON_BIT                     (2)
        #define MAC_TX_STROBE_CCA_ON_BITS                    (1)
        /* MAC_TX_RST field */
        #define MAC_TX_STROBE_MAC_TX_RST                     (0x00000002u)
        #define MAC_TX_STROBE_MAC_TX_RST_MASK                (0x00000002u)
        #define MAC_TX_STROBE_MAC_TX_RST_BIT                 (1)
        #define MAC_TX_STROBE_MAC_TX_RST_BITS                (1)
        /* START_TX field */
        #define MAC_TX_STROBE_START_TX                       (0x00000001u)
        #define MAC_TX_STROBE_START_TX_MASK                  (0x00000001u)
        #define MAC_TX_STROBE_START_TX_BIT                   (0)
        #define MAC_TX_STROBE_START_TX_BITS                  (1)

#define MAC_ACK_STROBE                                       *((volatile uint32_t *)0x40002064u)
#define MAC_ACK_STROBE_REG                                   *((volatile uint32_t *)0x40002064u)
#define MAC_ACK_STROBE_ADDR                                  (0x40002064u)
#define MAC_ACK_STROBE_RESET                                 (0x00000000u)
        /* MANUAL_ACK field */
        #define MAC_ACK_STROBE_MANUAL_ACK                    (0x00000002u)
        #define MAC_ACK_STROBE_MANUAL_ACK_MASK               (0x00000002u)
        #define MAC_ACK_STROBE_MANUAL_ACK_BIT                (1)
        #define MAC_ACK_STROBE_MANUAL_ACK_BITS               (1)
        /* FRAME_PENDING field */
        #define MAC_ACK_STROBE_FRAME_PENDING                 (0x00000001u)
        #define MAC_ACK_STROBE_FRAME_PENDING_MASK            (0x00000001u)
        #define MAC_ACK_STROBE_FRAME_PENDING_BIT             (0)
        #define MAC_ACK_STROBE_FRAME_PENDING_BITS            (1)

#define MAC_STATUS                                           *((volatile uint32_t *)0x40002068u)
#define MAC_STATUS_REG                                       *((volatile uint32_t *)0x40002068u)
#define MAC_STATUS_ADDR                                      (0x40002068u)
#define MAC_STATUS_RESET                                     (0x00000000u)
        /* RX_B_PEND_TX_ACK field */
        #define MAC_STATUS_RX_B_PEND_TX_ACK                  (0x00000800u)
        #define MAC_STATUS_RX_B_PEND_TX_ACK_MASK             (0x00000800u)
        #define MAC_STATUS_RX_B_PEND_TX_ACK_BIT              (11)
        #define MAC_STATUS_RX_B_PEND_TX_ACK_BITS             (1)
        /* RX_A_PEND_TX_ACK field */
        #define MAC_STATUS_RX_A_PEND_TX_ACK                  (0x00000400u)
        #define MAC_STATUS_RX_A_PEND_TX_ACK_MASK             (0x00000400u)
        #define MAC_STATUS_RX_A_PEND_TX_ACK_BIT              (10)
        #define MAC_STATUS_RX_A_PEND_TX_ACK_BITS             (1)
        /* RX_B_LAST_UNLOAD field */
        #define MAC_STATUS_RX_B_LAST_UNLOAD                  (0x00000200u)
        #define MAC_STATUS_RX_B_LAST_UNLOAD_MASK             (0x00000200u)
        #define MAC_STATUS_RX_B_LAST_UNLOAD_BIT              (9)
        #define MAC_STATUS_RX_B_LAST_UNLOAD_BITS             (1)
        /* RX_A_LAST_UNLOAD field */
        #define MAC_STATUS_RX_A_LAST_UNLOAD                  (0x00000100u)
        #define MAC_STATUS_RX_A_LAST_UNLOAD_MASK             (0x00000100u)
        #define MAC_STATUS_RX_A_LAST_UNLOAD_BIT              (8)
        #define MAC_STATUS_RX_A_LAST_UNLOAD_BITS             (1)
        /* WRONG_FORMAT field */
        #define MAC_STATUS_WRONG_FORMAT                      (0x00000080u)
        #define MAC_STATUS_WRONG_FORMAT_MASK                 (0x00000080u)
        #define MAC_STATUS_WRONG_FORMAT_BIT                  (7)
        #define MAC_STATUS_WRONG_FORMAT_BITS                 (1)
        /* WRONG_ADDRESS field */
        #define MAC_STATUS_WRONG_ADDRESS                     (0x00000040u)
        #define MAC_STATUS_WRONG_ADDRESS_MASK                (0x00000040u)
        #define MAC_STATUS_WRONG_ADDRESS_BIT                 (6)
        #define MAC_STATUS_WRONG_ADDRESS_BITS                (1)
        /* RX_ACK_REC field */
        #define MAC_STATUS_RX_ACK_REC                        (0x00000020u)
        #define MAC_STATUS_RX_ACK_REC_MASK                   (0x00000020u)
        #define MAC_STATUS_RX_ACK_REC_BIT                    (5)
        #define MAC_STATUS_RX_ACK_REC_BITS                   (1)
        /* SENDING_ACK field */
        #define MAC_STATUS_SENDING_ACK                       (0x00000010u)
        #define MAC_STATUS_SENDING_ACK_MASK                  (0x00000010u)
        #define MAC_STATUS_SENDING_ACK_BIT                   (4)
        #define MAC_STATUS_SENDING_ACK_BITS                  (1)
        /* RUN_BO field */
        #define MAC_STATUS_RUN_BO                            (0x00000008u)
        #define MAC_STATUS_RUN_BO_MASK                       (0x00000008u)
        #define MAC_STATUS_RUN_BO_BIT                        (3)
        #define MAC_STATUS_RUN_BO_BITS                       (1)
        /* TX_FRAME field */
        #define MAC_STATUS_TX_FRAME                          (0x00000004u)
        #define MAC_STATUS_TX_FRAME_MASK                     (0x00000004u)
        #define MAC_STATUS_TX_FRAME_BIT                      (2)
        #define MAC_STATUS_TX_FRAME_BITS                     (1)
        /* RX_FRAME field */
        #define MAC_STATUS_RX_FRAME                          (0x00000002u)
        #define MAC_STATUS_RX_FRAME_MASK                     (0x00000002u)
        #define MAC_STATUS_RX_FRAME_BIT                      (1)
        #define MAC_STATUS_RX_FRAME_BITS                     (1)
        /* RX_CRC_PASS field */
        #define MAC_STATUS_RX_CRC_PASS                       (0x00000001u)
        #define MAC_STATUS_RX_CRC_PASS_MASK                  (0x00000001u)
        #define MAC_STATUS_RX_CRC_PASS_BIT                   (0)
        #define MAC_STATUS_RX_CRC_PASS_BITS                  (1)

#define TX_CRC                                               *((volatile uint32_t *)0x4000206Cu)
#define TX_CRC_REG                                           *((volatile uint32_t *)0x4000206Cu)
#define TX_CRC_ADDR                                          (0x4000206Cu)
#define TX_CRC_RESET                                         (0x00000000u)
        /* TX_CRC field */
        #define TX_CRC_TX_CRC                                (0x0000FFFFu)
        #define TX_CRC_TX_CRC_MASK                           (0x0000FFFFu)
        #define TX_CRC_TX_CRC_BIT                            (0)
        #define TX_CRC_TX_CRC_BITS                           (16)

#define RX_CRC                                               *((volatile uint32_t *)0x40002070u)
#define RX_CRC_REG                                           *((volatile uint32_t *)0x40002070u)
#define RX_CRC_ADDR                                          (0x40002070u)
#define RX_CRC_RESET                                         (0x00000000u)
        /* RX_CRC field */
        #define RX_CRC_RX_CRC                                (0x0000FFFFu)
        #define RX_CRC_RX_CRC_MASK                           (0x0000FFFFu)
        #define RX_CRC_RX_CRC_BIT                            (0)
        #define RX_CRC_RX_CRC_BITS                           (16)

#define MAC_ACK_TO                                           *((volatile uint32_t *)0x40002074u)
#define MAC_ACK_TO_REG                                       *((volatile uint32_t *)0x40002074u)
#define MAC_ACK_TO_ADDR                                      (0x40002074u)
#define MAC_ACK_TO_RESET                                     (0x00000300u)
        /* ACK_TO field */
        #define MAC_ACK_TO_ACK_TO                            (0x00003FFFu)
        #define MAC_ACK_TO_ACK_TO_MASK                       (0x00003FFFu)
        #define MAC_ACK_TO_ACK_TO_BIT                        (0)
        #define MAC_ACK_TO_ACK_TO_BITS                       (14)

#define MAC_BOP_COMPARE                                      *((volatile uint32_t *)0x40002078u)
#define MAC_BOP_COMPARE_REG                                  *((volatile uint32_t *)0x40002078u)
#define MAC_BOP_COMPARE_ADDR                                 (0x40002078u)
#define MAC_BOP_COMPARE_RESET                                (0x00000014u)
        /* MAC_BOP_COMPARE field */
        #define MAC_BOP_COMPARE_MAC_BOP_COMPARE              (0x0000007Fu)
        #define MAC_BOP_COMPARE_MAC_BOP_COMPARE_MASK         (0x0000007Fu)
        #define MAC_BOP_COMPARE_MAC_BOP_COMPARE_BIT          (0)
        #define MAC_BOP_COMPARE_MAC_BOP_COMPARE_BITS         (7)

#define MAC_TX_ACK_FRAME                                     *((volatile uint32_t *)0x4000207Cu)
#define MAC_TX_ACK_FRAME_REG                                 *((volatile uint32_t *)0x4000207Cu)
#define MAC_TX_ACK_FRAME_ADDR                                (0x4000207Cu)
#define MAC_TX_ACK_FRAME_RESET                               (0x00000002u)
        /* ACK_SRC_AM field */
        #define MAC_TX_ACK_FRAME_ACK_SRC_AM                  (0x0000C000u)
        #define MAC_TX_ACK_FRAME_ACK_SRC_AM_MASK             (0x0000C000u)
        #define MAC_TX_ACK_FRAME_ACK_SRC_AM_BIT              (14)
        #define MAC_TX_ACK_FRAME_ACK_SRC_AM_BITS             (2)
        /* RES1213 field */
        #define MAC_TX_ACK_FRAME_RES1213                     (0x00003000u)
        #define MAC_TX_ACK_FRAME_RES1213_MASK                (0x00003000u)
        #define MAC_TX_ACK_FRAME_RES1213_BIT                 (12)
        #define MAC_TX_ACK_FRAME_RES1213_BITS                (2)
        /* ACK_DST_AM field */
        #define MAC_TX_ACK_FRAME_ACK_DST_AM                  (0x00000C00u)
        #define MAC_TX_ACK_FRAME_ACK_DST_AM_MASK             (0x00000C00u)
        #define MAC_TX_ACK_FRAME_ACK_DST_AM_BIT              (10)
        #define MAC_TX_ACK_FRAME_ACK_DST_AM_BITS             (2)
        /* RES789 field */
        #define MAC_TX_ACK_FRAME_RES789                      (0x00000380u)
        #define MAC_TX_ACK_FRAME_RES789_MASK                 (0x00000380u)
        #define MAC_TX_ACK_FRAME_RES789_BIT                  (7)
        #define MAC_TX_ACK_FRAME_RES789_BITS                 (3)
        /* ACK_IP field */
        #define MAC_TX_ACK_FRAME_ACK_IP                      (0x00000040u)
        #define MAC_TX_ACK_FRAME_ACK_IP_MASK                 (0x00000040u)
        #define MAC_TX_ACK_FRAME_ACK_IP_BIT                  (6)
        #define MAC_TX_ACK_FRAME_ACK_IP_BITS                 (1)
        /* ACK_ACK_REQ field */
        #define MAC_TX_ACK_FRAME_ACK_ACK_REQ                 (0x00000020u)
        #define MAC_TX_ACK_FRAME_ACK_ACK_REQ_MASK            (0x00000020u)
        #define MAC_TX_ACK_FRAME_ACK_ACK_REQ_BIT             (5)
        #define MAC_TX_ACK_FRAME_ACK_ACK_REQ_BITS            (1)
        /* ACK_FRAME_P field */
        #define MAC_TX_ACK_FRAME_ACK_FRAME_P                 (0x00000010u)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_P_MASK            (0x00000010u)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_P_BIT             (4)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_P_BITS            (1)
        /* ACK_SEC_EN field */
        #define MAC_TX_ACK_FRAME_ACK_SEC_EN                  (0x00000008u)
        #define MAC_TX_ACK_FRAME_ACK_SEC_EN_MASK             (0x00000008u)
        #define MAC_TX_ACK_FRAME_ACK_SEC_EN_BIT              (3)
        #define MAC_TX_ACK_FRAME_ACK_SEC_EN_BITS             (1)
        /* ACK_FRAME_T field */
        #define MAC_TX_ACK_FRAME_ACK_FRAME_T                 (0x00000007u)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_T_MASK            (0x00000007u)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_T_BIT             (0)
        #define MAC_TX_ACK_FRAME_ACK_FRAME_T_BITS            (3)

#define MAC_CONFIG                                           *((volatile uint32_t *)0x40002080u)
#define MAC_CONFIG_REG                                       *((volatile uint32_t *)0x40002080u)
#define MAC_CONFIG_ADDR                                      (0x40002080u)
#define MAC_CONFIG_RESET                                     (0x00000000u)
        /* RSSI_INST_EN field */
        #define MAC_CONFIG_RSSI_INST_EN                      (0x00000004u)
        #define MAC_CONFIG_RSSI_INST_EN_MASK                 (0x00000004u)
        #define MAC_CONFIG_RSSI_INST_EN_BIT                  (2)
        #define MAC_CONFIG_RSSI_INST_EN_BITS                 (1)
        /* SPI_SPY_EN field */
        #define MAC_CONFIG_SPI_SPY_EN                        (0x00000002u)
        #define MAC_CONFIG_SPI_SPY_EN_MASK                   (0x00000002u)
        #define MAC_CONFIG_SPI_SPY_EN_BIT                    (1)
        #define MAC_CONFIG_SPI_SPY_EN_BITS                   (1)
        /* MAC_MODE field */
        #define MAC_CONFIG_MAC_MODE                          (0x00000001u)
        #define MAC_CONFIG_MAC_MODE_MASK                     (0x00000001u)
        #define MAC_CONFIG_MAC_MODE_BIT                      (0)
        #define MAC_CONFIG_MAC_MODE_BITS                     (1)

#define MAC_RX_CONFIG                                        *((volatile uint32_t *)0x40002084u)
#define MAC_RX_CONFIG_REG                                    *((volatile uint32_t *)0x40002084u)
#define MAC_RX_CONFIG_ADDR                                   (0x40002084u)
#define MAC_RX_CONFIG_RESET                                  (0x00000000u)
        /* AUTO_ACK field */
        #define MAC_RX_CONFIG_AUTO_ACK                       (0x00000080u)
        #define MAC_RX_CONFIG_AUTO_ACK_MASK                  (0x00000080u)
        #define MAC_RX_CONFIG_AUTO_ACK_BIT                   (7)
        #define MAC_RX_CONFIG_AUTO_ACK_BITS                  (1)
        /* APPEND_INFO field */
        #define MAC_RX_CONFIG_APPEND_INFO                    (0x00000040u)
        #define MAC_RX_CONFIG_APPEND_INFO_MASK               (0x00000040u)
        #define MAC_RX_CONFIG_APPEND_INFO_BIT                (6)
        #define MAC_RX_CONFIG_APPEND_INFO_BITS               (1)
        /* COORDINATOR field */
        #define MAC_RX_CONFIG_COORDINATOR                    (0x00000020u)
        #define MAC_RX_CONFIG_COORDINATOR_MASK               (0x00000020u)
        #define MAC_RX_CONFIG_COORDINATOR_BIT                (5)
        #define MAC_RX_CONFIG_COORDINATOR_BITS               (1)
        /* FILT_ADDR_ON field */
        #define MAC_RX_CONFIG_FILT_ADDR_ON                   (0x00000010u)
        #define MAC_RX_CONFIG_FILT_ADDR_ON_MASK              (0x00000010u)
        #define MAC_RX_CONFIG_FILT_ADDR_ON_BIT               (4)
        #define MAC_RX_CONFIG_FILT_ADDR_ON_BITS              (1)
        /* RES_FILT_PASS_ADDR field */
        #define MAC_RX_CONFIG_RES_FILT_PASS_ADDR             (0x00000008u)
        #define MAC_RX_CONFIG_RES_FILT_PASS_ADDR_MASK        (0x00000008u)
        #define MAC_RX_CONFIG_RES_FILT_PASS_ADDR_BIT         (3)
        #define MAC_RX_CONFIG_RES_FILT_PASS_ADDR_BITS        (1)
        /* RES_FILT_PASS field */
        #define MAC_RX_CONFIG_RES_FILT_PASS                  (0x00000004u)
        #define MAC_RX_CONFIG_RES_FILT_PASS_MASK             (0x00000004u)
        #define MAC_RX_CONFIG_RES_FILT_PASS_BIT              (2)
        #define MAC_RX_CONFIG_RES_FILT_PASS_BITS             (1)
        /* FILT_FORMAT_ON field */
        #define MAC_RX_CONFIG_FILT_FORMAT_ON                 (0x00000002u)
        #define MAC_RX_CONFIG_FILT_FORMAT_ON_MASK            (0x00000002u)
        #define MAC_RX_CONFIG_FILT_FORMAT_ON_BIT             (1)
        #define MAC_RX_CONFIG_FILT_FORMAT_ON_BITS            (1)
        /* MAC_RX_RST field */
        #define MAC_RX_CONFIG_MAC_RX_RST                     (0x00000001u)
        #define MAC_RX_CONFIG_MAC_RX_RST_MASK                (0x00000001u)
        #define MAC_RX_CONFIG_MAC_RX_RST_BIT                 (0)
        #define MAC_RX_CONFIG_MAC_RX_RST_BITS                (1)

#define MAC_TX_CONFIG                                        *((volatile uint32_t *)0x40002088u)
#define MAC_TX_CONFIG_REG                                    *((volatile uint32_t *)0x40002088u)
#define MAC_TX_CONFIG_ADDR                                   (0x40002088u)
#define MAC_TX_CONFIG_RESET                                  (0x00000008u)
        /* SLOTTED field */
        #define MAC_TX_CONFIG_SLOTTED                        (0x00000010u)
        #define MAC_TX_CONFIG_SLOTTED_MASK                   (0x00000010u)
        #define MAC_TX_CONFIG_SLOTTED_BIT                    (4)
        #define MAC_TX_CONFIG_SLOTTED_BITS                   (1)
        /* CCA_DELAY field */
        #define MAC_TX_CONFIG_CCA_DELAY                      (0x00000008u)
        #define MAC_TX_CONFIG_CCA_DELAY_MASK                 (0x00000008u)
        #define MAC_TX_CONFIG_CCA_DELAY_BIT                  (3)
        #define MAC_TX_CONFIG_CCA_DELAY_BITS                 (1)
        /* SLOTTED_ACK field */
        #define MAC_TX_CONFIG_SLOTTED_ACK                    (0x00000004u)
        #define MAC_TX_CONFIG_SLOTTED_ACK_MASK               (0x00000004u)
        #define MAC_TX_CONFIG_SLOTTED_ACK_BIT                (2)
        #define MAC_TX_CONFIG_SLOTTED_ACK_BITS               (1)
        /* INFINITE_CRC field */
        #define MAC_TX_CONFIG_INFINITE_CRC                   (0x00000002u)
        #define MAC_TX_CONFIG_INFINITE_CRC_MASK              (0x00000002u)
        #define MAC_TX_CONFIG_INFINITE_CRC_BIT               (1)
        #define MAC_TX_CONFIG_INFINITE_CRC_BITS              (1)
        /* WAIT_ACK field */
        #define MAC_TX_CONFIG_WAIT_ACK                       (0x00000001u)
        #define MAC_TX_CONFIG_WAIT_ACK_MASK                  (0x00000001u)
        #define MAC_TX_CONFIG_WAIT_ACK_BIT                   (0)
        #define MAC_TX_CONFIG_WAIT_ACK_BITS                  (1)

#define MAC_TIMER_CTRL                                       *((volatile uint32_t *)0x4000208Cu)
#define MAC_TIMER_CTRL_REG                                   *((volatile uint32_t *)0x4000208Cu)
#define MAC_TIMER_CTRL_ADDR                                  (0x4000208Cu)
#define MAC_TIMER_CTRL_RESET                                 (0x00000000u)
        /* COMP_A_SYNC field */
        #define MAC_TIMER_CTRL_COMP_A_SYNC                   (0x00000040u)
        #define MAC_TIMER_CTRL_COMP_A_SYNC_MASK              (0x00000040u)
        #define MAC_TIMER_CTRL_COMP_A_SYNC_BIT               (6)
        #define MAC_TIMER_CTRL_COMP_A_SYNC_BITS              (1)
        /* BOP_TIMER_RST field */
        #define MAC_TIMER_CTRL_BOP_TIMER_RST                 (0x00000020u)
        #define MAC_TIMER_CTRL_BOP_TIMER_RST_MASK            (0x00000020u)
        #define MAC_TIMER_CTRL_BOP_TIMER_RST_BIT             (5)
        #define MAC_TIMER_CTRL_BOP_TIMER_RST_BITS            (1)
        /* BOP_TIMER_EN field */
        #define MAC_TIMER_CTRL_BOP_TIMER_EN                  (0x00000010u)
        #define MAC_TIMER_CTRL_BOP_TIMER_EN_MASK             (0x00000010u)
        #define MAC_TIMER_CTRL_BOP_TIMER_EN_BIT              (4)
        #define MAC_TIMER_CTRL_BOP_TIMER_EN_BITS             (1)
        /* BO_TIMER_RST field */
        #define MAC_TIMER_CTRL_BO_TIMER_RST                  (0x00000008u)
        #define MAC_TIMER_CTRL_BO_TIMER_RST_MASK             (0x00000008u)
        #define MAC_TIMER_CTRL_BO_TIMER_RST_BIT              (3)
        #define MAC_TIMER_CTRL_BO_TIMER_RST_BITS             (1)
        /* BO_TIMER_EN field */
        #define MAC_TIMER_CTRL_BO_TIMER_EN                   (0x00000004u)
        #define MAC_TIMER_CTRL_BO_TIMER_EN_MASK              (0x00000004u)
        #define MAC_TIMER_CTRL_BO_TIMER_EN_BIT               (2)
        #define MAC_TIMER_CTRL_BO_TIMER_EN_BITS              (1)
        /* MAC_TIMER_RST field */
        #define MAC_TIMER_CTRL_MAC_TIMER_RST                 (0x00000002u)
        #define MAC_TIMER_CTRL_MAC_TIMER_RST_MASK            (0x00000002u)
        #define MAC_TIMER_CTRL_MAC_TIMER_RST_BIT             (1)
        #define MAC_TIMER_CTRL_MAC_TIMER_RST_BITS            (1)
        /* MAC_TIMER_EN field */
        #define MAC_TIMER_CTRL_MAC_TIMER_EN                  (0x00000001u)
        #define MAC_TIMER_CTRL_MAC_TIMER_EN_MASK             (0x00000001u)
        #define MAC_TIMER_CTRL_MAC_TIMER_EN_BIT              (0)
        #define MAC_TIMER_CTRL_MAC_TIMER_EN_BITS             (1)

#define PAN_ID                                               *((volatile uint32_t *)0x40002090u)
#define PAN_ID_REG                                           *((volatile uint32_t *)0x40002090u)
#define PAN_ID_ADDR                                          (0x40002090u)
#define PAN_ID_RESET                                         (0x00000000u)
        /* PAN_ID field */
        #define PAN_ID_PAN_ID                                (0x0000FFFFu)
        #define PAN_ID_PAN_ID_MASK                           (0x0000FFFFu)
        #define PAN_ID_PAN_ID_BIT                            (0)
        #define PAN_ID_PAN_ID_BITS                           (16)

#define SHORT_ADDR                                           *((volatile uint32_t *)0x40002094u)
#define SHORT_ADDR_REG                                       *((volatile uint32_t *)0x40002094u)
#define SHORT_ADDR_ADDR                                      (0x40002094u)
#define SHORT_ADDR_RESET                                     (0x00000000u)
        /* SHORT_ADDR field */
        #define SHORT_ADDR_SHORT_ADDR                        (0x0000FFFFu)
        #define SHORT_ADDR_SHORT_ADDR_MASK                   (0x0000FFFFu)
        #define SHORT_ADDR_SHORT_ADDR_BIT                    (0)
        #define SHORT_ADDR_SHORT_ADDR_BITS                   (16)

#define EXT_ADDR_0                                           *((volatile uint32_t *)0x40002098u)
#define EXT_ADDR_0_REG                                       *((volatile uint32_t *)0x40002098u)
#define EXT_ADDR_0_ADDR                                      (0x40002098u)
#define EXT_ADDR_0_RESET                                     (0x00000000u)
        /* EXT_ADDR_0 field */
        #define EXT_ADDR_0_EXT_ADDR_0                        (0x0000FFFFu)
        #define EXT_ADDR_0_EXT_ADDR_0_MASK                   (0x0000FFFFu)
        #define EXT_ADDR_0_EXT_ADDR_0_BIT                    (0)
        #define EXT_ADDR_0_EXT_ADDR_0_BITS                   (16)

#define EXT_ADDR_1                                           *((volatile uint32_t *)0x4000209Cu)
#define EXT_ADDR_1_REG                                       *((volatile uint32_t *)0x4000209Cu)
#define EXT_ADDR_1_ADDR                                      (0x4000209Cu)
#define EXT_ADDR_1_RESET                                     (0x00000000u)
        /* EXT_ADDR_1 field */
        #define EXT_ADDR_1_EXT_ADDR_1                        (0x0000FFFFu)
        #define EXT_ADDR_1_EXT_ADDR_1_MASK                   (0x0000FFFFu)
        #define EXT_ADDR_1_EXT_ADDR_1_BIT                    (0)
        #define EXT_ADDR_1_EXT_ADDR_1_BITS                   (16)

#define EXT_ADDR_2                                           *((volatile uint32_t *)0x400020A0u)
#define EXT_ADDR_2_REG                                       *((volatile uint32_t *)0x400020A0u)
#define EXT_ADDR_2_ADDR                                      (0x400020A0u)
#define EXT_ADDR_2_RESET                                     (0x00000000u)
        /* EXT_ADDR_2 field */
        #define EXT_ADDR_2_EXT_ADDR_2                        (0x0000FFFFu)
        #define EXT_ADDR_2_EXT_ADDR_2_MASK                   (0x0000FFFFu)
        #define EXT_ADDR_2_EXT_ADDR_2_BIT                    (0)
        #define EXT_ADDR_2_EXT_ADDR_2_BITS                   (16)

#define EXT_ADDR_3                                           *((volatile uint32_t *)0x400020A4u)
#define EXT_ADDR_3_REG                                       *((volatile uint32_t *)0x400020A4u)
#define EXT_ADDR_3_ADDR                                      (0x400020A4u)
#define EXT_ADDR_3_RESET                                     (0x00000000u)
        /* EXT_ADDR_3 field */
        #define EXT_ADDR_3_EXT_ADDR_3                        (0x0000FFFFu)
        #define EXT_ADDR_3_EXT_ADDR_3_MASK                   (0x0000FFFFu)
        #define EXT_ADDR_3_EXT_ADDR_3_BIT                    (0)
        #define EXT_ADDR_3_EXT_ADDR_3_BITS                   (16)

#define MAC_STATE                                            *((volatile uint32_t *)0x400020A8u)
#define MAC_STATE_REG                                        *((volatile uint32_t *)0x400020A8u)
#define MAC_STATE_ADDR                                       (0x400020A8u)
#define MAC_STATE_RESET                                      (0x00000000u)
        /* SPY_STATE field */
        #define MAC_STATE_SPY_STATE                          (0x00000700u)
        #define MAC_STATE_SPY_STATE_MASK                     (0x00000700u)
        #define MAC_STATE_SPY_STATE_BIT                      (8)
        #define MAC_STATE_SPY_STATE_BITS                     (3)
        /* ACK_STATE field */
        #define MAC_STATE_ACK_STATE                          (0x000000C0u)
        #define MAC_STATE_ACK_STATE_MASK                     (0x000000C0u)
        #define MAC_STATE_ACK_STATE_BIT                      (6)
        #define MAC_STATE_ACK_STATE_BITS                     (2)
        /* BO_STATE field */
        #define MAC_STATE_BO_STATE                           (0x0000003Cu)
        #define MAC_STATE_BO_STATE_MASK                      (0x0000003Cu)
        #define MAC_STATE_BO_STATE_BIT                       (2)
        #define MAC_STATE_BO_STATE_BITS                      (4)
        /* TOP_STATE field */
        #define MAC_STATE_TOP_STATE                          (0x00000003u)
        #define MAC_STATE_TOP_STATE_MASK                     (0x00000003u)
        #define MAC_STATE_TOP_STATE_BIT                      (0)
        #define MAC_STATE_TOP_STATE_BITS                     (2)

#define RX_STATE                                             *((volatile uint32_t *)0x400020ACu)
#define RX_STATE_REG                                         *((volatile uint32_t *)0x400020ACu)
#define RX_STATE_ADDR                                        (0x400020ACu)
#define RX_STATE_RESET                                       (0x00000000u)
        /* RX_BUFFER_STATE field */
        #define RX_STATE_RX_BUFFER_STATE                     (0x000001E0u)
        #define RX_STATE_RX_BUFFER_STATE_MASK                (0x000001E0u)
        #define RX_STATE_RX_BUFFER_STATE_BIT                 (5)
        #define RX_STATE_RX_BUFFER_STATE_BITS                (4)
        /* RX_TOP_STATE field */
        #define RX_STATE_RX_TOP_STATE                        (0x0000001Fu)
        #define RX_STATE_RX_TOP_STATE_MASK                   (0x0000001Fu)
        #define RX_STATE_RX_TOP_STATE_BIT                    (0)
        #define RX_STATE_RX_TOP_STATE_BITS                   (5)

#define TX_STATE                                             *((volatile uint32_t *)0x400020B0u)
#define TX_STATE_REG                                         *((volatile uint32_t *)0x400020B0u)
#define TX_STATE_ADDR                                        (0x400020B0u)
#define TX_STATE_RESET                                       (0x00000000u)
        /* TX_BUFFER_STATE field */
        #define TX_STATE_TX_BUFFER_STATE                     (0x000000F0u)
        #define TX_STATE_TX_BUFFER_STATE_MASK                (0x000000F0u)
        #define TX_STATE_TX_BUFFER_STATE_BIT                 (4)
        #define TX_STATE_TX_BUFFER_STATE_BITS                (4)
        /* TX_TOP_STATE field */
        #define TX_STATE_TX_TOP_STATE                        (0x0000000Fu)
        #define TX_STATE_TX_TOP_STATE_MASK                   (0x0000000Fu)
        #define TX_STATE_TX_TOP_STATE_BIT                    (0)
        #define TX_STATE_TX_TOP_STATE_BITS                   (4)

#define DMA_STATE                                            *((volatile uint32_t *)0x400020B4u)
#define DMA_STATE_REG                                        *((volatile uint32_t *)0x400020B4u)
#define DMA_STATE_ADDR                                       (0x400020B4u)
#define DMA_STATE_RESET                                      (0x00000000u)
        /* DMA_RX_STATE field */
        #define DMA_STATE_DMA_RX_STATE                       (0x00000038u)
        #define DMA_STATE_DMA_RX_STATE_MASK                  (0x00000038u)
        #define DMA_STATE_DMA_RX_STATE_BIT                   (3)
        #define DMA_STATE_DMA_RX_STATE_BITS                  (3)
        /* DMA_TX_STATE field */
        #define DMA_STATE_DMA_TX_STATE                       (0x00000007u)
        #define DMA_STATE_DMA_TX_STATE_MASK                  (0x00000007u)
        #define DMA_STATE_DMA_TX_STATE_BIT                   (0)
        #define DMA_STATE_DMA_TX_STATE_BITS                  (3)

#define MAC_DEBUG                                            *((volatile uint32_t *)0x400020B8u)
#define MAC_DEBUG_REG                                        *((volatile uint32_t *)0x400020B8u)
#define MAC_DEBUG_ADDR                                       (0x400020B8u)
#define MAC_DEBUG_RESET                                      (0x00000000u)
        /* SW_DEBUG_OUT field */
        #define MAC_DEBUG_SW_DEBUG_OUT                       (0x00000060u)
        #define MAC_DEBUG_SW_DEBUG_OUT_MASK                  (0x00000060u)
        #define MAC_DEBUG_SW_DEBUG_OUT_BIT                   (5)
        #define MAC_DEBUG_SW_DEBUG_OUT_BITS                  (2)
        /* MAC_DEBUG_MUX field */
        #define MAC_DEBUG_MAC_DEBUG_MUX                      (0x0000001Fu)
        #define MAC_DEBUG_MAC_DEBUG_MUX_MASK                 (0x0000001Fu)
        #define MAC_DEBUG_MAC_DEBUG_MUX_BIT                  (0)
        #define MAC_DEBUG_MAC_DEBUG_MUX_BITS                 (5)

#define MAC_DEBUG_VIEW                                       *((volatile uint32_t *)0x400020BCu)
#define MAC_DEBUG_VIEW_REG                                   *((volatile uint32_t *)0x400020BCu)
#define MAC_DEBUG_VIEW_ADDR                                  (0x400020BCu)
#define MAC_DEBUG_VIEW_RESET                                 (0x00000010u)
        /* MAC_DEBUG_VIEW field */
        #define MAC_DEBUG_VIEW_MAC_DEBUG_VIEW                (0x0000FFFFu)
        #define MAC_DEBUG_VIEW_MAC_DEBUG_VIEW_MASK           (0x0000FFFFu)
        #define MAC_DEBUG_VIEW_MAC_DEBUG_VIEW_BIT            (0)
        #define MAC_DEBUG_VIEW_MAC_DEBUG_VIEW_BITS           (16)

#define MAC_RSSI_DELAY                                       *((volatile uint32_t *)0x400020C0u)
#define MAC_RSSI_DELAY_REG                                   *((volatile uint32_t *)0x400020C0u)
#define MAC_RSSI_DELAY_ADDR                                  (0x400020C0u)
#define MAC_RSSI_DELAY_RESET                                 (0x00000000u)
        /* RSSI_INST_DELAY_OK field */
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_OK            (0x00000FC0u)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_OK_MASK       (0x00000FC0u)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_OK_BIT        (6)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_OK_BITS       (6)
        /* RSSI_INST_DELAY field */
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY               (0x0000003Fu)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_MASK          (0x0000003Fu)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_BIT           (0)
        #define MAC_RSSI_DELAY_RSSI_INST_DELAY_BITS          (6)

#define PANID_COUNT                                          *((volatile uint32_t *)0x400020C4u)
#define PANID_COUNT_REG                                      *((volatile uint32_t *)0x400020C4u)
#define PANID_COUNT_ADDR                                     (0x400020C4u)
#define PANID_COUNT_RESET                                    (0x00000000u)
        /* PANID_COUNT field */
        #define PANID_COUNT_PANID_COUNT                      (0x0000FFFFu)
        #define PANID_COUNT_PANID_COUNT_MASK                 (0x0000FFFFu)
        #define PANID_COUNT_PANID_COUNT_BIT                  (0)
        #define PANID_COUNT_PANID_COUNT_BITS                 (16)

#define NONPAN_COUNT                                         *((volatile uint32_t *)0x400020C8u)
#define NONPAN_COUNT_REG                                     *((volatile uint32_t *)0x400020C8u)
#define NONPAN_COUNT_ADDR                                    (0x400020C8u)
#define NONPAN_COUNT_RESET                                   (0x00000000u)
        /* NONPAN_COUNT field */
        #define NONPAN_COUNT_NONPAN_COUNT                    (0x0000FFFFu)
        #define NONPAN_COUNT_NONPAN_COUNT_MASK               (0x0000FFFFu)
        #define NONPAN_COUNT_NONPAN_COUNT_BIT                (0)
        #define NONPAN_COUNT_NONPAN_COUNT_BITS               (16)

/* SECURITY block */
#define DATA_SECURITY_BASE                                   (0x40003000u)
#define DATA_SECURITY_END                                    (0x40003044u)
#define DATA_SECURITY_SIZE                                   (DATA_SECURITY_END - DATA_SECURITY_BASE + 1)

#define SECURITY_CONFIG                                      *((volatile uint32_t *)0x40003000u)
#define SECURITY_CONFIG_REG                                  *((volatile uint32_t *)0x40003000u)
#define SECURITY_CONFIG_ADDR                                 (0x40003000u)
#define SECURITY_CONFIG_RESET                                (0x00000000u)
        /* SEC_RST field */
        #define SECURITY_CONFIG_SEC_RST                      (0x00000080u)
        #define SECURITY_CONFIG_SEC_RST_MASK                 (0x00000080u)
        #define SECURITY_CONFIG_SEC_RST_BIT                  (7)
        #define SECURITY_CONFIG_SEC_RST_BITS                 (1)
        /* CTR_IN field */
        #define SECURITY_CONFIG_CTR_IN                       (0x00000040u)
        #define SECURITY_CONFIG_CTR_IN_MASK                  (0x00000040u)
        #define SECURITY_CONFIG_CTR_IN_BIT                   (6)
        #define SECURITY_CONFIG_CTR_IN_BITS                  (1)
        /* MIC_XOR_CT field */
        #define SECURITY_CONFIG_MIC_XOR_CT                   (0x00000020u)
        #define SECURITY_CONFIG_MIC_XOR_CT_MASK              (0x00000020u)
        #define SECURITY_CONFIG_MIC_XOR_CT_BIT               (5)
        #define SECURITY_CONFIG_MIC_XOR_CT_BITS              (1)
        /* CBC_XOR_PT field */
        #define SECURITY_CONFIG_CBC_XOR_PT                   (0x00000010u)
        #define SECURITY_CONFIG_CBC_XOR_PT_MASK              (0x00000010u)
        #define SECURITY_CONFIG_CBC_XOR_PT_BIT               (4)
        #define SECURITY_CONFIG_CBC_XOR_PT_BITS              (1)
        /* CT_TO_CBC_ST field */
        #define SECURITY_CONFIG_CT_TO_CBC_ST                 (0x00000008u)
        #define SECURITY_CONFIG_CT_TO_CBC_ST_MASK            (0x00000008u)
        #define SECURITY_CONFIG_CT_TO_CBC_ST_BIT             (3)
        #define SECURITY_CONFIG_CT_TO_CBC_ST_BITS            (1)
        /* WAIT_CT_READ field */
        #define SECURITY_CONFIG_WAIT_CT_READ                 (0x00000004u)
        #define SECURITY_CONFIG_WAIT_CT_READ_MASK            (0x00000004u)
        #define SECURITY_CONFIG_WAIT_CT_READ_BIT             (2)
        #define SECURITY_CONFIG_WAIT_CT_READ_BITS            (1)
        /* WAIT_PT_WRITE field */
        #define SECURITY_CONFIG_WAIT_PT_WRITE                (0x00000002u)
        #define SECURITY_CONFIG_WAIT_PT_WRITE_MASK           (0x00000002u)
        #define SECURITY_CONFIG_WAIT_PT_WRITE_BIT            (1)
        #define SECURITY_CONFIG_WAIT_PT_WRITE_BITS           (1)
        /* START_AES field */
        #define SECURITY_CONFIG_START_AES                    (0x00000001u)
        #define SECURITY_CONFIG_START_AES_MASK               (0x00000001u)
        #define SECURITY_CONFIG_START_AES_BIT                (0)
        #define SECURITY_CONFIG_START_AES_BITS               (1)

#define SECURITY_STATUS                                      *((volatile uint32_t *)0x40003004u)
#define SECURITY_STATUS_REG                                  *((volatile uint32_t *)0x40003004u)
#define SECURITY_STATUS_ADDR                                 (0x40003004u)
#define SECURITY_STATUS_RESET                                (0x00000000u)
        /* SEC_BUSY field */
        #define SECURITY_STATUS_SEC_BUSY                     (0x00000001u)
        #define SECURITY_STATUS_SEC_BUSY_MASK                (0x00000001u)
        #define SECURITY_STATUS_SEC_BUSY_BIT                 (0)
        #define SECURITY_STATUS_SEC_BUSY_BITS                (1)

#define CBC_STATE_0                                          *((volatile uint32_t *)0x40003008u)
#define CBC_STATE_0_REG                                      *((volatile uint32_t *)0x40003008u)
#define CBC_STATE_0_ADDR                                     (0x40003008u)
#define CBC_STATE_0_RESET                                    (0x00000000u)
        /* CBC_STATE field */
        #define CBC_STATE_0_CBC_STATE                        (0xFFFFFFFFu)
        #define CBC_STATE_0_CBC_STATE_MASK                   (0xFFFFFFFFu)
        #define CBC_STATE_0_CBC_STATE_BIT                    (0)
        #define CBC_STATE_0_CBC_STATE_BITS                   (32)

#define CBC_STATE_1                                          *((volatile uint32_t *)0x4000300Cu)
#define CBC_STATE_1_REG                                      *((volatile uint32_t *)0x4000300Cu)
#define CBC_STATE_1_ADDR                                     (0x4000300Cu)
#define CBC_STATE_1_RESET                                    (0x00000000u)
        /* CBC_STATE_1 field */
        #define CBC_STATE_1_CBC_STATE_1                      (0xFFFFFFFFu)
        #define CBC_STATE_1_CBC_STATE_1_MASK                 (0xFFFFFFFFu)
        #define CBC_STATE_1_CBC_STATE_1_BIT                  (0)
        #define CBC_STATE_1_CBC_STATE_1_BITS                 (32)

#define CBC_STATE_2                                          *((volatile uint32_t *)0x40003010u)
#define CBC_STATE_2_REG                                      *((volatile uint32_t *)0x40003010u)
#define CBC_STATE_2_ADDR                                     (0x40003010u)
#define CBC_STATE_2_RESET                                    (0x00000000u)
        /* CBC_STATE_2 field */
        #define CBC_STATE_2_CBC_STATE_2                      (0xFFFFFFFFu)
        #define CBC_STATE_2_CBC_STATE_2_MASK                 (0xFFFFFFFFu)
        #define CBC_STATE_2_CBC_STATE_2_BIT                  (0)
        #define CBC_STATE_2_CBC_STATE_2_BITS                 (32)

#define CBC_STATE_3                                          *((volatile uint32_t *)0x40003014u)
#define CBC_STATE_3_REG                                      *((volatile uint32_t *)0x40003014u)
#define CBC_STATE_3_ADDR                                     (0x40003014u)
#define CBC_STATE_3_RESET                                    (0x00000000u)
        /* CBC_STATE_3 field */
        #define CBC_STATE_3_CBC_STATE_3                      (0xFFFFFFFFu)
        #define CBC_STATE_3_CBC_STATE_3_MASK                 (0xFFFFFFFFu)
        #define CBC_STATE_3_CBC_STATE_3_BIT                  (0)
        #define CBC_STATE_3_CBC_STATE_3_BITS                 (32)

#define PT                                                   *((volatile uint32_t *)0x40003028u)
#define PT_REG                                               *((volatile uint32_t *)0x40003028u)
#define PT_ADDR                                              (0x40003028u)
#define PT_RESET                                             (0x00000000u)
        /* PT field */
        #define PT_PT                                        (0xFFFFFFFFu)
        #define PT_PT_MASK                                   (0xFFFFFFFFu)
        #define PT_PT_BIT                                    (0)
        #define PT_PT_BITS                                   (32)

#define CT                                                   *((volatile uint32_t *)0x40003030u)
#define CT_REG                                               *((volatile uint32_t *)0x40003030u)
#define CT_ADDR                                              (0x40003030u)
#define CT_RESET                                             (0x00000000u)
        /* CT field */
        #define CT_CT                                        (0xFFFFFFFFu)
        #define CT_CT_MASK                                   (0xFFFFFFFFu)
        #define CT_CT_BIT                                    (0)
        #define CT_CT_BITS                                   (32)

#define KEY_0                                                *((volatile uint32_t *)0x40003038u)
#define KEY_0_REG                                            *((volatile uint32_t *)0x40003038u)
#define KEY_0_ADDR                                           (0x40003038u)
#define KEY_0_RESET                                          (0x00000000u)
        /* KEY_O field */
        #define KEY_0_KEY_O                                  (0xFFFFFFFFu)
        #define KEY_0_KEY_O_MASK                             (0xFFFFFFFFu)
        #define KEY_0_KEY_O_BIT                              (0)
        #define KEY_0_KEY_O_BITS                             (32)

#define KEY_1                                                *((volatile uint32_t *)0x4000303Cu)
#define KEY_1_REG                                            *((volatile uint32_t *)0x4000303Cu)
#define KEY_1_ADDR                                           (0x4000303Cu)
#define KEY_1_RESET                                          (0x00000000u)
        /* KEY_1 field */
        #define KEY_1_KEY_1                                  (0xFFFFFFFFu)
        #define KEY_1_KEY_1_MASK                             (0xFFFFFFFFu)
        #define KEY_1_KEY_1_BIT                              (0)
        #define KEY_1_KEY_1_BITS                             (32)

#define KEY_2                                                *((volatile uint32_t *)0x40003040u)
#define KEY_2_REG                                            *((volatile uint32_t *)0x40003040u)
#define KEY_2_ADDR                                           (0x40003040u)
#define KEY_2_RESET                                          (0x00000000u)
        /* KEY_2 field */
        #define KEY_2_KEY_2                                  (0xFFFFFFFFu)
        #define KEY_2_KEY_2_MASK                             (0xFFFFFFFFu)
        #define KEY_2_KEY_2_BIT                              (0)
        #define KEY_2_KEY_2_BITS                             (32)

#define KEY_3                                                *((volatile uint32_t *)0x40003044u)
#define KEY_3_REG                                            *((volatile uint32_t *)0x40003044u)
#define KEY_3_ADDR                                           (0x40003044u)
#define KEY_3_RESET                                          (0x00000000u)
        /* KEY_3 field */
        #define KEY_3_KEY_3                                  (0xFFFFFFFFu)
        #define KEY_3_KEY_3_MASK                             (0xFFFFFFFFu)
        #define KEY_3_KEY_3_BIT                              (0)
        #define KEY_3_KEY_3_BITS                             (32)

/* CM_LV block */
#define BLOCK_CM_LV_BASE                                     (0x40004000u)
#define BLOCK_CM_LV_END                                      (0x40004034u)
#define BLOCK_CM_LV_SIZE                                     (BLOCK_CM_LV_END - BLOCK_CM_LV_BASE + 1)

#define SILICON_ID                                           *((volatile uint32_t *)0x40004000u)
#define SILICON_ID_REG                                       *((volatile uint32_t *)0x40004000u)
#define SILICON_ID_ADDR                                      (0x40004000u)
#define SILICON_ID_RESET                                     (0x069A862Bu)
        /* HW_VERSION field */
        #define SILICON_ID_HW_VERSION                        (0xF0000000u)
        #define SILICON_ID_HW_VERSION_MASK                   (0xF0000000u)
        #define SILICON_ID_HW_VERSION_BIT                    (28)
        #define SILICON_ID_HW_VERSION_BITS                   (4)
        /* ST_DIVISION field */
        #define SILICON_ID_ST_DIVISION                       (0x0F000000u)
        #define SILICON_ID_ST_DIVISION_MASK                  (0x0F000000u)
        #define SILICON_ID_ST_DIVISION_BIT                   (24)
        #define SILICON_ID_ST_DIVISION_BITS                  (4)
        /* CHIP_TYPE field */
        #define SILICON_ID_CHIP_TYPE                         (0x00FF8000u)
        #define SILICON_ID_CHIP_TYPE_MASK                    (0x00FF8000u)
        #define SILICON_ID_CHIP_TYPE_BIT                     (15)
        #define SILICON_ID_CHIP_TYPE_BITS                    (9)
        /* SUB_TYPE field */
        #define SILICON_ID_SUB_TYPE                          (0x00007000u)
        #define SILICON_ID_SUB_TYPE_MASK                     (0x00007000u)
        #define SILICON_ID_SUB_TYPE_BIT                      (12)
        #define SILICON_ID_SUB_TYPE_BITS                     (3)
        /* JEDEC_MAN_ID field */
        #define SILICON_ID_JEDEC_MAN_ID                      (0x00000FFEu)
        #define SILICON_ID_JEDEC_MAN_ID_MASK                 (0x00000FFEu)
        #define SILICON_ID_JEDEC_MAN_ID_BIT                  (1)
        #define SILICON_ID_JEDEC_MAN_ID_BITS                 (11)
        /* ONE field */
        #define SILICON_ID_ONE                               (0x00000001u)
        #define SILICON_ID_ONE_MASK                          (0x00000001u)
        #define SILICON_ID_ONE_BIT                           (0)
        #define SILICON_ID_ONE_BITS                          (1)

#define OSC24M_BIASTRIM                                      *((volatile uint32_t *)0x40004004u)
#define OSC24M_BIASTRIM_REG                                  *((volatile uint32_t *)0x40004004u)
#define OSC24M_BIASTRIM_ADDR                                 (0x40004004u)
#define OSC24M_BIASTRIM_RESET                                (0x0000000Fu)
        /* OSC24M_BIAS_TRIM field */
        #define OSC24M_BIASTRIM_OSC24M_BIAS_TRIM             (0x0000000Fu)
        #define OSC24M_BIASTRIM_OSC24M_BIAS_TRIM_MASK        (0x0000000Fu)
        #define OSC24M_BIASTRIM_OSC24M_BIAS_TRIM_BIT         (0)
        #define OSC24M_BIASTRIM_OSC24M_BIAS_TRIM_BITS        (4)

#define OSCHF_TUNE                                           *((volatile uint32_t *)0x40004008u)
#define OSCHF_TUNE_REG                                       *((volatile uint32_t *)0x40004008u)
#define OSCHF_TUNE_ADDR                                      (0x40004008u)
#define OSCHF_TUNE_RESET                                     (0x00000017u)
        /* OSCHF_TUNE_FIELD field */
        #define OSCHF_TUNE_FIELD                             (0x0000001Fu)
        #define OSCHF_TUNE_FIELD_MASK                        (0x0000001Fu)
        #define OSCHF_TUNE_FIELD_BIT                         (0)
        #define OSCHF_TUNE_FIELD_BITS                        (5)

#define OSC24M_COMP                                          *((volatile uint32_t *)0x4000400Cu)
#define OSC24M_COMP_REG                                      *((volatile uint32_t *)0x4000400Cu)
#define OSC24M_COMP_ADDR                                     (0x4000400Cu)
#define OSC24M_COMP_RESET                                    (0x00000000u)
        /* OSC24M_HI field */
        #define OSC24M_HI                                    (0x00000002u)
        #define OSC24M_HI_MASK                               (0x00000002u)
        #define OSC24M_HI_BIT                                (1)
        #define OSC24M_HI_BITS                               (1)
        /* OSC24M_LO field */
        #define OSC24M_LO                                    (0x00000001u)
        #define OSC24M_LO_MASK                               (0x00000001u)
        #define OSC24M_LO_BIT                                (0)
        #define OSC24M_LO_BITS                               (1)

#define CLK_PERIODMODE                                       *((volatile uint32_t *)0x40004010u)
#define CLK_PERIODMODE_REG                                   *((volatile uint32_t *)0x40004010u)
#define CLK_PERIODMODE_ADDR                                  (0x40004010u)
#define CLK_PERIODMODE_RESET                                 (0x00000000u)
        /* CLK_PERIODMODE_FIELD field */
        #define CLK_PERIODMODE_FIELD                         (0x00000003u)
        #define CLK_PERIODMODE_FIELD_MASK                    (0x00000003u)
        #define CLK_PERIODMODE_FIELD_BIT                     (0)
        #define CLK_PERIODMODE_FIELD_BITS                    (2)

#define CLK_PERIOD                                           *((volatile uint32_t *)0x40004014u)
#define CLK_PERIOD_REG                                       *((volatile uint32_t *)0x40004014u)
#define CLK_PERIOD_ADDR                                      (0x40004014u)
#define CLK_PERIOD_RESET                                     (0x00000000u)
        /* CLK_PERIOD_FIELD field */
        #define CLK_PERIOD_FIELD                             (0x0000FFFFu)
        #define CLK_PERIOD_FIELD_MASK                        (0x0000FFFFu)
        #define CLK_PERIOD_FIELD_BIT                         (0)
        #define CLK_PERIOD_FIELD_BITS                        (16)

#define DITHER_DIS                                           *((volatile uint32_t *)0x40004018u)
#define DITHER_DIS_REG                                       *((volatile uint32_t *)0x40004018u)
#define DITHER_DIS_ADDR                                      (0x40004018u)
#define DITHER_DIS_RESET                                     (0x00000000u)
        /* DITHER_DIS field */
        #define DITHER_DIS_DITHER_DIS                        (0x00000001u)
        #define DITHER_DIS_DITHER_DIS_MASK                   (0x00000001u)
        #define DITHER_DIS_DITHER_DIS_BIT                    (0)
        #define DITHER_DIS_DITHER_DIS_BITS                   (1)

#define OSC24M_CTRL                                          *((volatile uint32_t *)0x4000401Cu)
#define OSC24M_CTRL_REG                                      *((volatile uint32_t *)0x4000401Cu)
#define OSC24M_CTRL_ADDR                                     (0x4000401Cu)
#define OSC24M_CTRL_RESET                                    (0x00000000u)
        /* OSC24M_EN field */
        #define OSC24M_CTRL_OSC24M_EN                        (0x00000002u)
        #define OSC24M_CTRL_OSC24M_EN_MASK                   (0x00000002u)
        #define OSC24M_CTRL_OSC24M_EN_BIT                    (1)
        #define OSC24M_CTRL_OSC24M_EN_BITS                   (1)
        /* OSC24M_SEL field */
        #define OSC24M_CTRL_OSC24M_SEL                       (0x00000001u)
        #define OSC24M_CTRL_OSC24M_SEL_MASK                  (0x00000001u)
        #define OSC24M_CTRL_OSC24M_SEL_BIT                   (0)
        #define OSC24M_CTRL_OSC24M_SEL_BITS                  (1)

#define CPU_CLKSEL                                           *((volatile uint32_t *)0x40004020u)
#define CPU_CLKSEL_REG                                       *((volatile uint32_t *)0x40004020u)
#define CPU_CLKSEL_ADDR                                      (0x40004020u)
#define CPU_CLKSEL_RESET                                     (0x00000000u)
        /* CPU_CLKSEL_FIELD field */
        #define CPU_CLKSEL_FIELD                             (0x00000001u)
        #define CPU_CLKSEL_FIELD_MASK                        (0x00000001u)
        #define CPU_CLKSEL_FIELD_BIT                         (0)
        #define CPU_CLKSEL_FIELD_BITS                        (1)

#define BUS_FAULT                                            *((volatile uint32_t *)0x40004024u)
#define BUS_FAULT_REG                                        *((volatile uint32_t *)0x40004024u)
#define BUS_FAULT_ADDR                                       (0x40004024u)
#define BUS_FAULT_RESET                                      (0x00000000u)
        /* WRONGSIZE field */
        #define BUS_FAULT_WRONGSIZE                          (0x00000008u)
        #define BUS_FAULT_WRONGSIZE_MASK                     (0x00000008u)
        #define BUS_FAULT_WRONGSIZE_BIT                      (3)
        #define BUS_FAULT_WRONGSIZE_BITS                     (1)
        /* PROTECTED field */
        #define BUS_FAULT_PROTECTED                          (0x00000004u)
        #define BUS_FAULT_PROTECTED_MASK                     (0x00000004u)
        #define BUS_FAULT_PROTECTED_BIT                      (2)
        #define BUS_FAULT_PROTECTED_BITS                     (1)
        /* RESERVED field */
        #define BUS_FAULT_RESERVED                           (0x00000002u)
        #define BUS_FAULT_RESERVED_MASK                      (0x00000002u)
        #define BUS_FAULT_RESERVED_BIT                       (1)
        #define BUS_FAULT_RESERVED_BITS                      (1)
        /* MISSED field */
        #define BUS_FAULT_MISSED                             (0x00000001u)
        #define BUS_FAULT_MISSED_MASK                        (0x00000001u)
        #define BUS_FAULT_MISSED_BIT                         (0)
        #define BUS_FAULT_MISSED_BITS                        (1)

#define PCTRACE_SEL                                          *((volatile uint32_t *)0x40004028u)
#define PCTRACE_SEL_REG                                      *((volatile uint32_t *)0x40004028u)
#define PCTRACE_SEL_ADDR                                     (0x40004028u)
#define PCTRACE_SEL_RESET                                    (0x00000000u)
        /* PCTRACE_SEL_FIELD field */
        #define PCTRACE_SEL_FIELD                            (0x00000001u)
        #define PCTRACE_SEL_FIELD_MASK                       (0x00000001u)
        #define PCTRACE_SEL_FIELD_BIT                        (0)
        #define PCTRACE_SEL_FIELD_BITS                       (1)

#define FPEC_CLKREQ                                          *((volatile uint32_t *)0x4000402Cu)
#define FPEC_CLKREQ_REG                                      *((volatile uint32_t *)0x4000402Cu)
#define FPEC_CLKREQ_ADDR                                     (0x4000402Cu)
#define FPEC_CLKREQ_RESET                                    (0x00000000u)
        /* FPEC_CLKREQ_FIELD field */
        #define FPEC_CLKREQ_FIELD                            (0x00000001u)
        #define FPEC_CLKREQ_FIELD_MASK                       (0x00000001u)
        #define FPEC_CLKREQ_FIELD_BIT                        (0)
        #define FPEC_CLKREQ_FIELD_BITS                       (1)

#define FPEC_CLKSTAT                                         *((volatile uint32_t *)0x40004030u)
#define FPEC_CLKSTAT_REG                                     *((volatile uint32_t *)0x40004030u)
#define FPEC_CLKSTAT_ADDR                                    (0x40004030u)
#define FPEC_CLKSTAT_RESET                                   (0x00000000u)
        /* FPEC_CLKBSY field */
        #define FPEC_CLKBSY                                  (0x00000002u)
        #define FPEC_CLKBSY_MASK                             (0x00000002u)
        #define FPEC_CLKBSY_BIT                              (1)
        #define FPEC_CLKBSY_BITS                             (1)
        /* FPEC_CLKACK field */
        #define FPEC_CLKACK                                  (0x00000001u)
        #define FPEC_CLKACK_MASK                             (0x00000001u)
        #define FPEC_CLKACK_BIT                              (0)
        #define FPEC_CLKACK_BITS                             (1)

#define LV_SPARE                                             *((volatile uint32_t *)0x40004034u)
#define LV_SPARE_REG                                         *((volatile uint32_t *)0x40004034u)
#define LV_SPARE_ADDR                                        (0x40004034u)
#define LV_SPARE_RESET                                       (0x00000000u)
        /* LV_SPARE field */
        #define LV_SPARE_LV_SPARE                            (0x000000FFu)
        #define LV_SPARE_LV_SPARE_MASK                       (0x000000FFu)
        #define LV_SPARE_LV_SPARE_BIT                        (0)
        #define LV_SPARE_LV_SPARE_BITS                       (8)

/* RAM_CTRL block */
#define DATA_RAM_CTRL_BASE                                   (0x40005000u)
#define DATA_RAM_CTRL_END                                    (0x40005028u)
#define DATA_RAM_CTRL_SIZE                                   (DATA_RAM_CTRL_END - DATA_RAM_CTRL_BASE + 1)

#define MEM_PROT_0                                           *((volatile uint32_t *)0x40005000u)
#define MEM_PROT_0_REG                                       *((volatile uint32_t *)0x40005000u)
#define MEM_PROT_0_ADDR                                      (0x40005000u)
#define MEM_PROT_0_RESET                                     (0x00000000u)
        /* MEM_PROT_0 field */
        #define MEM_PROT_0_MEM_PROT_0                        (0xFFFFFFFFu)
        #define MEM_PROT_0_MEM_PROT_0_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_0_MEM_PROT_0_BIT                    (0)
        #define MEM_PROT_0_MEM_PROT_0_BITS                   (32)

#define MEM_PROT_1                                           *((volatile uint32_t *)0x40005004u)
#define MEM_PROT_1_REG                                       *((volatile uint32_t *)0x40005004u)
#define MEM_PROT_1_ADDR                                      (0x40005004u)
#define MEM_PROT_1_RESET                                     (0x00000000u)
        /* MEM_PROT_1 field */
        #define MEM_PROT_1_MEM_PROT_1                        (0xFFFFFFFFu)
        #define MEM_PROT_1_MEM_PROT_1_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_1_MEM_PROT_1_BIT                    (0)
        #define MEM_PROT_1_MEM_PROT_1_BITS                   (32)

#define MEM_PROT_2                                           *((volatile uint32_t *)0x40005008u)
#define MEM_PROT_2_REG                                       *((volatile uint32_t *)0x40005008u)
#define MEM_PROT_2_ADDR                                      (0x40005008u)
#define MEM_PROT_2_RESET                                     (0x00000000u)
        /* MEM_PROT_2 field */
        #define MEM_PROT_2_MEM_PROT_2                        (0xFFFFFFFFu)
        #define MEM_PROT_2_MEM_PROT_2_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_2_MEM_PROT_2_BIT                    (0)
        #define MEM_PROT_2_MEM_PROT_2_BITS                   (32)

#define MEM_PROT_3                                           *((volatile uint32_t *)0x4000500Cu)
#define MEM_PROT_3_REG                                       *((volatile uint32_t *)0x4000500Cu)
#define MEM_PROT_3_ADDR                                      (0x4000500Cu)
#define MEM_PROT_3_RESET                                     (0x00000000u)
        /* MEM_PROT_3 field */
        #define MEM_PROT_3_MEM_PROT_3                        (0xFFFFFFFFu)
        #define MEM_PROT_3_MEM_PROT_3_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_3_MEM_PROT_3_BIT                    (0)
        #define MEM_PROT_3_MEM_PROT_3_BITS                   (32)

#define MEM_PROT_4                                           *((volatile uint32_t *)0x40005010u)
#define MEM_PROT_4_REG                                       *((volatile uint32_t *)0x40005010u)
#define MEM_PROT_4_ADDR                                      (0x40005010u)
#define MEM_PROT_4_RESET                                     (0x00000000u)
        /* MEM_PROT_4 field */
        #define MEM_PROT_4_MEM_PROT_4                        (0xFFFFFFFFu)
        #define MEM_PROT_4_MEM_PROT_4_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_4_MEM_PROT_4_BIT                    (0)
        #define MEM_PROT_4_MEM_PROT_4_BITS                   (32)

#define MEM_PROT_5                                           *((volatile uint32_t *)0x40005014u)
#define MEM_PROT_5_REG                                       *((volatile uint32_t *)0x40005014u)
#define MEM_PROT_5_ADDR                                      (0x40005014u)
#define MEM_PROT_5_RESET                                     (0x00000000u)
        /* MEM_PROT_5 field */
        #define MEM_PROT_5_MEM_PROT_5                        (0xFFFFFFFFu)
        #define MEM_PROT_5_MEM_PROT_5_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_5_MEM_PROT_5_BIT                    (0)
        #define MEM_PROT_5_MEM_PROT_5_BITS                   (32)

#define MEM_PROT_6                                           *((volatile uint32_t *)0x40005018u)
#define MEM_PROT_6_REG                                       *((volatile uint32_t *)0x40005018u)
#define MEM_PROT_6_ADDR                                      (0x40005018u)
#define MEM_PROT_6_RESET                                     (0x00000000u)
        /* MEM_PROT_6 field */
        #define MEM_PROT_6_MEM_PROT_6                        (0xFFFFFFFFu)
        #define MEM_PROT_6_MEM_PROT_6_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_6_MEM_PROT_6_BIT                    (0)
        #define MEM_PROT_6_MEM_PROT_6_BITS                   (32)

#define MEM_PROT_7                                           *((volatile uint32_t *)0x4000501Cu)
#define MEM_PROT_7_REG                                       *((volatile uint32_t *)0x4000501Cu)
#define MEM_PROT_7_ADDR                                      (0x4000501Cu)
#define MEM_PROT_7_RESET                                     (0x00000000u)
        /* MEM_PROT_7 field */
        #define MEM_PROT_7_MEM_PROT_7                        (0xFFFFFFFFu)
        #define MEM_PROT_7_MEM_PROT_7_MASK                   (0xFFFFFFFFu)
        #define MEM_PROT_7_MEM_PROT_7_BIT                    (0)
        #define MEM_PROT_7_MEM_PROT_7_BITS                   (32)

#define DMA_PROT_ADDR                                        *((volatile uint32_t *)0x40005020u)
#define DMA_PROT_ADDR_REG                                    *((volatile uint32_t *)0x40005020u)
#define DMA_PROT_ADDR_ADDR                                   (0x40005020u)
#define DMA_PROT_ADDR_RESET                                  (0x20000000u)
        /* DMA_PROT_OFFS field */
        #define DMA_PROT_ADDR_DMA_PROT_OFFS                  (0xFFFFE000u)
        #define DMA_PROT_ADDR_DMA_PROT_OFFS_MASK             (0xFFFFE000u)
        #define DMA_PROT_ADDR_DMA_PROT_OFFS_BIT              (13)
        #define DMA_PROT_ADDR_DMA_PROT_OFFS_BITS             (19)
        /* DMA_PROT_ADDR field */
        #define DMA_PROT_ADDR_DMA_PROT_ADDR                  (0x00001FFFu)
        #define DMA_PROT_ADDR_DMA_PROT_ADDR_MASK             (0x00001FFFu)
        #define DMA_PROT_ADDR_DMA_PROT_ADDR_BIT              (0)
        #define DMA_PROT_ADDR_DMA_PROT_ADDR_BITS             (13)

#define DMA_PROT_CH                                          *((volatile uint32_t *)0x40005024u)
#define DMA_PROT_CH_REG                                      *((volatile uint32_t *)0x40005024u)
#define DMA_PROT_CH_ADDR                                     (0x40005024u)
#define DMA_PROT_CH_RESET                                    (0x00000000u)
        /* DMA_PROT_CH field */
        #define DMA_PROT_CH_DMA_PROT_CH                      (0x00000007u)
        #define DMA_PROT_CH_DMA_PROT_CH_MASK                 (0x00000007u)
        #define DMA_PROT_CH_DMA_PROT_CH_BIT                  (0)
        #define DMA_PROT_CH_DMA_PROT_CH_BITS                 (3)

#define MEM_PROT_EN                                          *((volatile uint32_t *)0x40005028u)
#define MEM_PROT_EN_REG                                      *((volatile uint32_t *)0x40005028u)
#define MEM_PROT_EN_ADDR                                     (0x40005028u)
#define MEM_PROT_EN_RESET                                    (0x00000000u)
        /* FORCE_PROT field */
        #define MEM_PROT_EN_FORCE_PROT                       (0x00000004u)
        #define MEM_PROT_EN_FORCE_PROT_MASK                  (0x00000004u)
        #define MEM_PROT_EN_FORCE_PROT_BIT                   (2)
        #define MEM_PROT_EN_FORCE_PROT_BITS                  (1)
        /* DMA_PROT_EN_MAC field */
        #define MEM_PROT_EN_DMA_PROT_EN_MAC                  (0x00000002u)
        #define MEM_PROT_EN_DMA_PROT_EN_MAC_MASK             (0x00000002u)
        #define MEM_PROT_EN_DMA_PROT_EN_MAC_BIT              (1)
        #define MEM_PROT_EN_DMA_PROT_EN_MAC_BITS             (1)
        /* DMA_PROT_EN_OTHER field */
        #define MEM_PROT_EN_DMA_PROT_EN_OTHER                (0x00000001u)
        #define MEM_PROT_EN_DMA_PROT_EN_OTHER_MASK           (0x00000001u)
        #define MEM_PROT_EN_DMA_PROT_EN_OTHER_BIT            (0)
        #define MEM_PROT_EN_DMA_PROT_EN_OTHER_BITS           (1)

/* SLOW_TIMERS block */
#define DATA_SLOW_TIMERS_BASE                                (0x40006000u)
#define DATA_SLOW_TIMERS_END                                 (0x40006024u)
#define DATA_SLOW_TIMERS_SIZE                                (DATA_SLOW_TIMERS_END - DATA_SLOW_TIMERS_BASE + 1)

#define WDOG_CFG                                             *((volatile uint32_t *)0x40006000u)
#define WDOG_CFG_REG                                         *((volatile uint32_t *)0x40006000u)
#define WDOG_CFG_ADDR                                        (0x40006000u)
#define WDOG_CFG_RESET                                       (0x00000002u)
        /* WDOG_DISABLE field */
        #define WDOG_DISABLE                                 (0x00000002u)
        #define WDOG_DISABLE_MASK                            (0x00000002u)
        #define WDOG_DISABLE_BIT                             (1)
        #define WDOG_DISABLE_BITS                            (1)
        /* WDOG_ENABLE field */
        #define WDOG_ENABLE                                  (0x00000001u)
        #define WDOG_ENABLE_MASK                             (0x00000001u)
        #define WDOG_ENABLE_BIT                              (0)
        #define WDOG_ENABLE_BITS                             (1)

#define WDOG_KEY                                             *((volatile uint32_t *)0x40006004u)
#define WDOG_KEY_REG                                         *((volatile uint32_t *)0x40006004u)
#define WDOG_KEY_ADDR                                        (0x40006004u)
#define WDOG_KEY_RESET                                       (0x00000000u)
        /* WDOG_KEY_FIELD field */
        #define WDOG_KEY_FIELD                               (0x0000FFFFu)
        #define WDOG_KEY_FIELD_MASK                          (0x0000FFFFu)
        #define WDOG_KEY_FIELD_BIT                           (0)
        #define WDOG_KEY_FIELD_BITS                          (16)

#define WDOG_RESET                                           *((volatile uint32_t *)0x40006008u)
#define WDOG_RESET_REG                                       *((volatile uint32_t *)0x40006008u)
#define WDOG_RESET_ADDR                                      (0x40006008u)
#define WDOG_RESET_RESET                                     (0x00000000u)

#define SLEEPTMR_CFG                                         *((volatile uint32_t *)0x4000600Cu)
#define SLEEPTMR_CFG_REG                                     *((volatile uint32_t *)0x4000600Cu)
#define SLEEPTMR_CFG_ADDR                                    (0x4000600Cu)
#define SLEEPTMR_CFG_RESET                                   (0x00000400u)
        /* SLEEPTMR_REVERSE field */
        #define SLEEPTMR_REVERSE                             (0x00001000u)
        #define SLEEPTMR_REVERSE_MASK                        (0x00001000u)
        #define SLEEPTMR_REVERSE_BIT                         (12)
        #define SLEEPTMR_REVERSE_BITS                        (1)
        /* SLEEPTMR_ENABLE field */
        #define SLEEPTMR_ENABLE                              (0x00000800u)
        #define SLEEPTMR_ENABLE_MASK                         (0x00000800u)
        #define SLEEPTMR_ENABLE_BIT                          (11)
        #define SLEEPTMR_ENABLE_BITS                         (1)
        /* SLEEPTMR_DBGPAUSE field */
        #define SLEEPTMR_DBGPAUSE                            (0x00000400u)
        #define SLEEPTMR_DBGPAUSE_MASK                       (0x00000400u)
        #define SLEEPTMR_DBGPAUSE_BIT                        (10)
        #define SLEEPTMR_DBGPAUSE_BITS                       (1)
        /* SLEEPTMR_CLKDIV field */
        #define SLEEPTMR_CLKDIV                              (0x000000F0u)
        #define SLEEPTMR_CLKDIV_MASK                         (0x000000F0u)
        #define SLEEPTMR_CLKDIV_BIT                          (4)
        #define SLEEPTMR_CLKDIV_BITS                         (4)
        /* SLEEPTMR_CLKSEL field */
        #define SLEEPTMR_CLKSEL                              (0x00000001u)
        #define SLEEPTMR_CLKSEL_MASK                         (0x00000001u)
        #define SLEEPTMR_CLKSEL_BIT                          (0)
        #define SLEEPTMR_CLKSEL_BITS                         (1)

#define SLEEPTMR_CNTH                                        *((volatile uint32_t *)0x40006010u)
#define SLEEPTMR_CNTH_REG                                    *((volatile uint32_t *)0x40006010u)
#define SLEEPTMR_CNTH_ADDR                                   (0x40006010u)
#define SLEEPTMR_CNTH_RESET                                  (0x00000000u)
        /* SLEEPTMR_CNTH_FIELD field */
        #define SLEEPTMR_CNTH_FIELD                          (0x0000FFFFu)
        #define SLEEPTMR_CNTH_FIELD_MASK                     (0x0000FFFFu)
        #define SLEEPTMR_CNTH_FIELD_BIT                      (0)
        #define SLEEPTMR_CNTH_FIELD_BITS                     (16)

#define SLEEPTMR_CNTL                                        *((volatile uint32_t *)0x40006014u)
#define SLEEPTMR_CNTL_REG                                    *((volatile uint32_t *)0x40006014u)
#define SLEEPTMR_CNTL_ADDR                                   (0x40006014u)
#define SLEEPTMR_CNTL_RESET                                  (0x00000000u)
        /* SLEEPTMR_CNTL_FIELD field */
        #define SLEEPTMR_CNTL_FIELD                          (0x0000FFFFu)
        #define SLEEPTMR_CNTL_FIELD_MASK                     (0x0000FFFFu)
        #define SLEEPTMR_CNTL_FIELD_BIT                      (0)
        #define SLEEPTMR_CNTL_FIELD_BITS                     (16)

#define SLEEPTMR_CMPAH                                       *((volatile uint32_t *)0x40006018u)
#define SLEEPTMR_CMPAH_REG                                   *((volatile uint32_t *)0x40006018u)
#define SLEEPTMR_CMPAH_ADDR                                  (0x40006018u)
#define SLEEPTMR_CMPAH_RESET                                 (0x0000FFFFu)
        /* SLEEPTMR_CMPAH_FIELD field */
        #define SLEEPTMR_CMPAH_FIELD                         (0x0000FFFFu)
        #define SLEEPTMR_CMPAH_FIELD_MASK                    (0x0000FFFFu)
        #define SLEEPTMR_CMPAH_FIELD_BIT                     (0)
        #define SLEEPTMR_CMPAH_FIELD_BITS                    (16)

#define SLEEPTMR_CMPAL                                       *((volatile uint32_t *)0x4000601Cu)
#define SLEEPTMR_CMPAL_REG                                   *((volatile uint32_t *)0x4000601Cu)
#define SLEEPTMR_CMPAL_ADDR                                  (0x4000601Cu)
#define SLEEPTMR_CMPAL_RESET                                 (0x0000FFFFu)
        /* SLEEPTMR_CMPAL_FIELD field */
        #define SLEEPTMR_CMPAL_FIELD                         (0x0000FFFFu)
        #define SLEEPTMR_CMPAL_FIELD_MASK                    (0x0000FFFFu)
        #define SLEEPTMR_CMPAL_FIELD_BIT                     (0)
        #define SLEEPTMR_CMPAL_FIELD_BITS                    (16)

#define SLEEPTMR_CMPBH                                       *((volatile uint32_t *)0x40006020u)
#define SLEEPTMR_CMPBH_REG                                   *((volatile uint32_t *)0x40006020u)
#define SLEEPTMR_CMPBH_ADDR                                  (0x40006020u)
#define SLEEPTMR_CMPBH_RESET                                 (0x0000FFFFu)
        /* SLEEPTMR_CMPBH_FIELD field */
        #define SLEEPTMR_CMPBH_FIELD                         (0x0000FFFFu)
        #define SLEEPTMR_CMPBH_FIELD_MASK                    (0x0000FFFFu)
        #define SLEEPTMR_CMPBH_FIELD_BIT                     (0)
        #define SLEEPTMR_CMPBH_FIELD_BITS                    (16)

#define SLEEPTMR_CMPBL                                       *((volatile uint32_t *)0x40006024u)
#define SLEEPTMR_CMPBL_REG                                   *((volatile uint32_t *)0x40006024u)
#define SLEEPTMR_CMPBL_ADDR                                  (0x40006024u)
#define SLEEPTMR_CMPBL_RESET                                 (0x0000FFFFu)
        /* SLEEPTMR_CMPBL_FIELD field */
        #define SLEEPTMR_CMPBL_FIELD                         (0x0000FFFFu)
        #define SLEEPTMR_CMPBL_FIELD_MASK                    (0x0000FFFFu)
        #define SLEEPTMR_CMPBL_FIELD_BIT                     (0)
        #define SLEEPTMR_CMPBL_FIELD_BITS                    (16)

/* CAL_ADC block */
#define DATA_CAL_ADC_BASE                                    (0x40007000u)
#define DATA_CAL_ADC_END                                     (0x40007004u)
#define DATA_CAL_ADC_SIZE                                    (DATA_CAL_ADC_END - DATA_CAL_ADC_BASE + 1)

#define CAL_ADC_DATA                                         *((volatile uint32_t *)0x40007000u)
#define CAL_ADC_DATA_REG                                     *((volatile uint32_t *)0x40007000u)
#define CAL_ADC_DATA_ADDR                                    (0x40007000u)
#define CAL_ADC_DATA_RESET                                   (0x00000000u)
        /* CAL_ADC_DATA field */
        #define CAL_ADC_DATA_CAL_ADC_DATA                    (0x0000FFFFu)
        #define CAL_ADC_DATA_CAL_ADC_DATA_MASK               (0x0000FFFFu)
        #define CAL_ADC_DATA_CAL_ADC_DATA_BIT                (0)
        #define CAL_ADC_DATA_CAL_ADC_DATA_BITS               (16)

#define CAL_ADC_CONFIG                                       *((volatile uint32_t *)0x40007004u)
#define CAL_ADC_CONFIG_REG                                   *((volatile uint32_t *)0x40007004u)
#define CAL_ADC_CONFIG_ADDR                                  (0x40007004u)
#define CAL_ADC_CONFIG_RESET                                 (0x00000000u)
        /* CAL_ADC_RATE field */
        #define CAL_ADC_CONFIG_CAL_ADC_RATE                  (0x00007000u)
        #define CAL_ADC_CONFIG_CAL_ADC_RATE_MASK             (0x00007000u)
        #define CAL_ADC_CONFIG_CAL_ADC_RATE_BIT              (12)
        #define CAL_ADC_CONFIG_CAL_ADC_RATE_BITS             (3)
        /* CAL_ADC_MUX field */
        #define CAL_ADC_CONFIG_CAL_ADC_MUX                   (0x00000F80u)
        #define CAL_ADC_CONFIG_CAL_ADC_MUX_MASK              (0x00000F80u)
        #define CAL_ADC_CONFIG_CAL_ADC_MUX_BIT               (7)
        #define CAL_ADC_CONFIG_CAL_ADC_MUX_BITS              (5)
        /* CAL_ADC_CLKSEL field */
        #define CAL_ADC_CONFIG_CAL_ADC_CLKSEL                (0x00000004u)
        #define CAL_ADC_CONFIG_CAL_ADC_CLKSEL_MASK           (0x00000004u)
        #define CAL_ADC_CONFIG_CAL_ADC_CLKSEL_BIT            (2)
        #define CAL_ADC_CONFIG_CAL_ADC_CLKSEL_BITS           (1)
        /* CAL_ADC_DITHER_DIS field */
        #define CAL_ADC_CONFIG_CAL_ADC_DITHER_DIS            (0x00000002u)
        #define CAL_ADC_CONFIG_CAL_ADC_DITHER_DIS_MASK       (0x00000002u)
        #define CAL_ADC_CONFIG_CAL_ADC_DITHER_DIS_BIT        (1)
        #define CAL_ADC_CONFIG_CAL_ADC_DITHER_DIS_BITS       (1)
        /* CAL_ADC_EN field */
        #define CAL_ADC_CONFIG_CAL_ADC_EN                    (0x00000001u)
        #define CAL_ADC_CONFIG_CAL_ADC_EN_MASK               (0x00000001u)
        #define CAL_ADC_CONFIG_CAL_ADC_EN_BIT                (0)
        #define CAL_ADC_CONFIG_CAL_ADC_EN_BITS               (1)

/* FLASH_CONTROL block */
#define DATA_FLASH_CONTROL_BASE                              (0x40008000u)
#define DATA_FLASH_CONTROL_END                               (0x40008084u)
#define DATA_FLASH_CONTROL_SIZE                              (DATA_FLASH_CONTROL_END - DATA_FLASH_CONTROL_BASE + 1)

#define FLASH_ACCESS                                         *((volatile uint32_t *)0x40008000u)
#define FLASH_ACCESS_REG                                     *((volatile uint32_t *)0x40008000u)
#define FLASH_ACCESS_ADDR                                    (0x40008000u)
#define FLASH_ACCESS_RESET                                   (0x00000031u)
        /* PREFETCH_STATUS field */
        #define FLASH_ACCESS_PREFETCH_STATUS                 (0x00000020u)
        #define FLASH_ACCESS_PREFETCH_STATUS_MASK            (0x00000020u)
        #define FLASH_ACCESS_PREFETCH_STATUS_BIT             (5)
        #define FLASH_ACCESS_PREFETCH_STATUS_BITS            (1)
        /* PREFETCH_EN field */
        #define FLASH_ACCESS_PREFETCH_EN                     (0x00000010u)
        #define FLASH_ACCESS_PREFETCH_EN_MASK                (0x00000010u)
        #define FLASH_ACCESS_PREFETCH_EN_BIT                 (4)
        #define FLASH_ACCESS_PREFETCH_EN_BITS                (1)
        /* HALFCYCLE_ACCESS field */
        #define FLASH_ACCESS_HALFCYCLE_ACCESS                (0x00000008u)
        #define FLASH_ACCESS_HALFCYCLE_ACCESS_MASK           (0x00000008u)
        #define FLASH_ACCESS_HALFCYCLE_ACCESS_BIT            (3)
        #define FLASH_ACCESS_HALFCYCLE_ACCESS_BITS           (1)
        /* CODE_LATENCY field */
        #define FLASH_ACCESS_CODE_LATENCY                    (0x00000007u)
        #define FLASH_ACCESS_CODE_LATENCY_MASK               (0x00000007u)
        #define FLASH_ACCESS_CODE_LATENCY_BIT                (0)
        #define FLASH_ACCESS_CODE_LATENCY_BITS               (3)

#define FPEC_KEY                                             *((volatile uint32_t *)0x40008004u)
#define FPEC_KEY_REG                                         *((volatile uint32_t *)0x40008004u)
#define FPEC_KEY_ADDR                                        (0x40008004u)
#define FPEC_KEY_RESET                                       (0x00000000u)
        /* FKEYR field */
        #define FPEC_KEY_FKEYR                               (0xFFFFFFFFu)
        #define FPEC_KEY_FKEYR_MASK                          (0xFFFFFFFFu)
        #define FPEC_KEY_FKEYR_BIT                           (0)
        #define FPEC_KEY_FKEYR_BITS                          (32)

#define OPT_KEY                                              *((volatile uint32_t *)0x40008008u)
#define OPT_KEY_REG                                          *((volatile uint32_t *)0x40008008u)
#define OPT_KEY_ADDR                                         (0x40008008u)
#define OPT_KEY_RESET                                        (0x00000000u)
        /* OPTKEYR field */
        #define OPT_KEY_OPTKEYR                              (0xFFFFFFFFu)
        #define OPT_KEY_OPTKEYR_MASK                         (0xFFFFFFFFu)
        #define OPT_KEY_OPTKEYR_BIT                          (0)
        #define OPT_KEY_OPTKEYR_BITS                         (32)

#define FLASH_STATUS                                         *((volatile uint32_t *)0x4000800Cu)
#define FLASH_STATUS_REG                                     *((volatile uint32_t *)0x4000800Cu)
#define FLASH_STATUS_ADDR                                    (0x4000800Cu)
#define FLASH_STATUS_RESET                                   (0x00000000u)
        /* EOP field */
        #define FLASH_STATUS_EOP                             (0x00000020u)
        #define FLASH_STATUS_EOP_MASK                        (0x00000020u)
        #define FLASH_STATUS_EOP_BIT                         (5)
        #define FLASH_STATUS_EOP_BITS                        (1)
        /* WRP_ERR field */
        #define FLASH_STATUS_WRP_ERR                         (0x00000010u)
        #define FLASH_STATUS_WRP_ERR_MASK                    (0x00000010u)
        #define FLASH_STATUS_WRP_ERR_BIT                     (4)
        #define FLASH_STATUS_WRP_ERR_BITS                    (1)
        /* PAGE_PROG_ERR field */
        #define FLASH_STATUS_PAGE_PROG_ERR                   (0x00000008u)
        #define FLASH_STATUS_PAGE_PROG_ERR_MASK              (0x00000008u)
        #define FLASH_STATUS_PAGE_PROG_ERR_BIT               (3)
        #define FLASH_STATUS_PAGE_PROG_ERR_BITS              (1)
        /* PROG_ERR field */
        #define FLASH_STATUS_PROG_ERR                        (0x00000004u)
        #define FLASH_STATUS_PROG_ERR_MASK                   (0x00000004u)
        #define FLASH_STATUS_PROG_ERR_BIT                    (2)
        #define FLASH_STATUS_PROG_ERR_BITS                   (1)
        /* EARLY_BSY field */
        #define FLASH_STATUS_EARLY_BSY                       (0x00000002u)
        #define FLASH_STATUS_EARLY_BSY_MASK                  (0x00000002u)
        #define FLASH_STATUS_EARLY_BSY_BIT                   (1)
        #define FLASH_STATUS_EARLY_BSY_BITS                  (1)
        /* FLA_BSY field */
        #define FLASH_STATUS_FLA_BSY                         (0x00000001u)
        #define FLASH_STATUS_FLA_BSY_MASK                    (0x00000001u)
        #define FLASH_STATUS_FLA_BSY_BIT                     (0)
        #define FLASH_STATUS_FLA_BSY_BITS                    (1)

#define FLASH_CTRL                                           *((volatile uint32_t *)0x40008010u)
#define FLASH_CTRL_REG                                       *((volatile uint32_t *)0x40008010u)
#define FLASH_CTRL_ADDR                                      (0x40008010u)
#define FLASH_CTRL_RESET                                     (0x00000080u)
        /* EOPIE field */
        #define FLASH_CTRL_EOPIE                             (0x00001000u)
        #define FLASH_CTRL_EOPIE_MASK                        (0x00001000u)
        #define FLASH_CTRL_EOPIE_BIT                         (12)
        #define FLASH_CTRL_EOPIE_BITS                        (1)
        /* EARLYBSYIE field */
        #define FLASH_CTRL_EARLYBSYIE                        (0x00000800u)
        #define FLASH_CTRL_EARLYBSYIE_MASK                   (0x00000800u)
        #define FLASH_CTRL_EARLYBSYIE_BIT                    (11)
        #define FLASH_CTRL_EARLYBSYIE_BITS                   (1)
        /* ERRIE field */
        #define FLASH_CTRL_ERRIE                             (0x00000400u)
        #define FLASH_CTRL_ERRIE_MASK                        (0x00000400u)
        #define FLASH_CTRL_ERRIE_BIT                         (10)
        #define FLASH_CTRL_ERRIE_BITS                        (1)
        /* OPTWREN field */
        #define FLASH_CTRL_OPTWREN                           (0x00000200u)
        #define FLASH_CTRL_OPTWREN_MASK                      (0x00000200u)
        #define FLASH_CTRL_OPTWREN_BIT                       (9)
        #define FLASH_CTRL_OPTWREN_BITS                      (1)
        /* FSTPROG field */
        #define FLASH_CTRL_FSTPROG                           (0x00000100u)
        #define FLASH_CTRL_FSTPROG_MASK                      (0x00000100u)
        #define FLASH_CTRL_FSTPROG_BIT                       (8)
        #define FLASH_CTRL_FSTPROG_BITS                      (1)
        /* LOCK field */
        #define FLASH_CTRL_LOCK                              (0x00000080u)
        #define FLASH_CTRL_LOCK_MASK                         (0x00000080u)
        #define FLASH_CTRL_LOCK_BIT                          (7)
        #define FLASH_CTRL_LOCK_BITS                         (1)
        /* FLA_START field */
        #define FLASH_CTRL_FLA_START                         (0x00000040u)
        #define FLASH_CTRL_FLA_START_MASK                    (0x00000040u)
        #define FLASH_CTRL_FLA_START_BIT                     (6)
        #define FLASH_CTRL_FLA_START_BITS                    (1)
        /* OPTERASE field */
        #define FLASH_CTRL_OPTERASE                          (0x00000020u)
        #define FLASH_CTRL_OPTERASE_MASK                     (0x00000020u)
        #define FLASH_CTRL_OPTERASE_BIT                      (5)
        #define FLASH_CTRL_OPTERASE_BITS                     (1)
        /* OPTPROG field */
        #define FLASH_CTRL_OPTPROG                           (0x00000010u)
        #define FLASH_CTRL_OPTPROG_MASK                      (0x00000010u)
        #define FLASH_CTRL_OPTPROG_BIT                       (4)
        #define FLASH_CTRL_OPTPROG_BITS                      (1)
        /* GLOBALERASE field */
        #define FLASH_CTRL_GLOBALERASE                       (0x00000008u)
        #define FLASH_CTRL_GLOBALERASE_MASK                  (0x00000008u)
        #define FLASH_CTRL_GLOBALERASE_BIT                   (3)
        #define FLASH_CTRL_GLOBALERASE_BITS                  (1)
        /* MASSERASE field */
        #define FLASH_CTRL_MASSERASE                         (0x00000004u)
        #define FLASH_CTRL_MASSERASE_MASK                    (0x00000004u)
        #define FLASH_CTRL_MASSERASE_BIT                     (2)
        #define FLASH_CTRL_MASSERASE_BITS                    (1)
        /* PAGEERASE field */
        #define FLASH_CTRL_PAGEERASE                         (0x00000002u)
        #define FLASH_CTRL_PAGEERASE_MASK                    (0x00000002u)
        #define FLASH_CTRL_PAGEERASE_BIT                     (1)
        #define FLASH_CTRL_PAGEERASE_BITS                    (1)
        /* PROG field */
        #define FLASH_CTRL_PROG                              (0x00000001u)
        #define FLASH_CTRL_PROG_MASK                         (0x00000001u)
        #define FLASH_CTRL_PROG_BIT                          (0)
        #define FLASH_CTRL_PROG_BITS                         (1)

#define FLASH_ADDR                                           *((volatile uint32_t *)0x40008014u)
#define FLASH_ADDR_REG                                       *((volatile uint32_t *)0x40008014u)
#define FLASH_ADDR_ADDR                                      (0x40008014u)
#define FLASH_ADDR_RESET                                     (0x00000000u)
        /* FAR field */
        #define FLASH_ADDR_FAR                               (0xFFFFFFFFu)
        #define FLASH_ADDR_FAR_MASK                          (0xFFFFFFFFu)
        #define FLASH_ADDR_FAR_BIT                           (0)
        #define FLASH_ADDR_FAR_BITS                          (32)

#define OPT_BYTE                                             *((volatile uint32_t *)0x4000801Cu)
#define OPT_BYTE_REG                                         *((volatile uint32_t *)0x4000801Cu)
#define OPT_BYTE_ADDR                                        (0x4000801Cu)
#define OPT_BYTE_RESET                                       (0xFBFFFFFEu)
        /* RSVD field */
        #define OPT_BYTE_RSVD                                (0xF8000000u)
        #define OPT_BYTE_RSVD_MASK                           (0xF8000000u)
        #define OPT_BYTE_RSVD_BIT                            (27)
        #define OPT_BYTE_RSVD_BITS                           (5)
        /* OBR field */
        #define OPT_BYTE_OBR                                 (0x07FFFFFCu)
        #define OPT_BYTE_OBR_MASK                            (0x07FFFFFCu)
        #define OPT_BYTE_OBR_BIT                             (2)
        #define OPT_BYTE_OBR_BITS                            (25)
        /* RDPROT field */
        #define OPT_BYTE_RDPROT                              (0x00000002u)
        #define OPT_BYTE_RDPROT_MASK                         (0x00000002u)
        #define OPT_BYTE_RDPROT_BIT                          (1)
        #define OPT_BYTE_RDPROT_BITS                         (1)
        /* OPT_ERR field */
        #define OPT_BYTE_OPT_ERR                             (0x00000001u)
        #define OPT_BYTE_OPT_ERR_MASK                        (0x00000001u)
        #define OPT_BYTE_OPT_ERR_BIT                         (0)
        #define OPT_BYTE_OPT_ERR_BITS                        (1)

#define WRPROT                                               *((volatile uint32_t *)0x40008020u)
#define WRPROT_REG                                           *((volatile uint32_t *)0x40008020u)
#define WRPROT_ADDR                                          (0x40008020u)
#define WRPROT_RESET                                         (0xFFFFFFFFu)
        /* WRP field */
        #define WRPROT_WRP                                   (0xFFFFFFFFu)
        #define WRPROT_WRP_MASK                              (0xFFFFFFFFu)
        #define WRPROT_WRP_BIT                               (0)
        #define WRPROT_WRP_BITS                              (32)

#define FLASH_TEST_CTRL                                      *((volatile uint32_t *)0x40008080u)
#define FLASH_TEST_CTRL_REG                                  *((volatile uint32_t *)0x40008080u)
#define FLASH_TEST_CTRL_ADDR                                 (0x40008080u)
#define FLASH_TEST_CTRL_RESET                                (0x00000000u)
        /* TMR field */
        #define FLASH_TEST_CTRL_TMR                          (0x00001000u)
        #define FLASH_TEST_CTRL_TMR_MASK                     (0x00001000u)
        #define FLASH_TEST_CTRL_TMR_BIT                      (12)
        #define FLASH_TEST_CTRL_TMR_BITS                     (1)
        /* ERASE field */
        #define FLASH_TEST_CTRL_ERASE                        (0x00000800u)
        #define FLASH_TEST_CTRL_ERASE_MASK                   (0x00000800u)
        #define FLASH_TEST_CTRL_ERASE_BIT                    (11)
        #define FLASH_TEST_CTRL_ERASE_BITS                   (1)
        /* MAS1 field */
        #define FLASH_TEST_CTRL_MAS1                         (0x00000400u)
        #define FLASH_TEST_CTRL_MAS1_MASK                    (0x00000400u)
        #define FLASH_TEST_CTRL_MAS1_BIT                     (10)
        #define FLASH_TEST_CTRL_MAS1_BITS                    (1)
        /* TEST_PROG field */
        #define FLASH_TEST_CTRL_TEST_PROG                    (0x00000200u)
        #define FLASH_TEST_CTRL_TEST_PROG_MASK               (0x00000200u)
        #define FLASH_TEST_CTRL_TEST_PROG_BIT                (9)
        #define FLASH_TEST_CTRL_TEST_PROG_BITS               (1)
        /* NVSTR field */
        #define FLASH_TEST_CTRL_NVSTR                        (0x00000100u)
        #define FLASH_TEST_CTRL_NVSTR_MASK                   (0x00000100u)
        #define FLASH_TEST_CTRL_NVSTR_BIT                    (8)
        #define FLASH_TEST_CTRL_NVSTR_BITS                   (1)
        /* SE field */
        #define FLASH_TEST_CTRL_SE                           (0x00000080u)
        #define FLASH_TEST_CTRL_SE_MASK                      (0x00000080u)
        #define FLASH_TEST_CTRL_SE_BIT                       (7)
        #define FLASH_TEST_CTRL_SE_BITS                      (1)
        /* IFREN field */
        #define FLASH_TEST_CTRL_IFREN                        (0x00000040u)
        #define FLASH_TEST_CTRL_IFREN_MASK                   (0x00000040u)
        #define FLASH_TEST_CTRL_IFREN_BIT                    (6)
        #define FLASH_TEST_CTRL_IFREN_BITS                   (1)
        /* YE field */
        #define FLASH_TEST_CTRL_YE                           (0x00000020u)
        #define FLASH_TEST_CTRL_YE_MASK                      (0x00000020u)
        #define FLASH_TEST_CTRL_YE_BIT                       (5)
        #define FLASH_TEST_CTRL_YE_BITS                      (1)
        /* XE field */
        #define FLASH_TEST_CTRL_XE                           (0x00000010u)
        #define FLASH_TEST_CTRL_XE_MASK                      (0x00000010u)
        #define FLASH_TEST_CTRL_XE_BIT                       (4)
        #define FLASH_TEST_CTRL_XE_BITS                      (1)
        /* SW_CTRL field */
        #define FLASH_TEST_CTRL_SW_CTRL                      (0x00000008u)
        #define FLASH_TEST_CTRL_SW_CTRL_MASK                 (0x00000008u)
        #define FLASH_TEST_CTRL_SW_CTRL_BIT                  (3)
        #define FLASH_TEST_CTRL_SW_CTRL_BITS                 (1)
        /* SW field */
        #define FLASH_TEST_CTRL_SW                           (0x00000006u)
        #define FLASH_TEST_CTRL_SW_MASK                      (0x00000006u)
        #define FLASH_TEST_CTRL_SW_BIT                       (1)
        #define FLASH_TEST_CTRL_SW_BITS                      (2)
        /* SW_EN field */
        #define FLASH_TEST_CTRL_SW_EN                        (0x00000001u)
        #define FLASH_TEST_CTRL_SW_EN_MASK                   (0x00000001u)
        #define FLASH_TEST_CTRL_SW_EN_BIT                    (0)
        #define FLASH_TEST_CTRL_SW_EN_BITS                   (1)

#define FLASH_DATA0                                          *((volatile uint32_t *)0x40008084u)
#define FLASH_DATA0_REG                                      *((volatile uint32_t *)0x40008084u)
#define FLASH_DATA0_ADDR                                     (0x40008084u)
#define FLASH_DATA0_RESET                                    (0xFFFFFFFFu)
        /* FDR0 field */
        #define FLASH_DATA0_FDR0                             (0xFFFFFFFFu)
        #define FLASH_DATA0_FDR0_MASK                        (0xFFFFFFFFu)
        #define FLASH_DATA0_FDR0_BIT                         (0)
        #define FLASH_DATA0_FDR0_BITS                        (32)

/* EMU_REGS block */
#define DATA_EMU_REGS_BASE                                   (0x40009000u)
#define DATA_EMU_REGS_END                                    (0x40009000u)
#define DATA_EMU_REGS_SIZE                                   (DATA_EMU_REGS_END - DATA_EMU_REGS_BASE + 1)

#define I_AM_AN_EMULATOR                                     *((volatile uint32_t *)0x40009000u)
#define I_AM_AN_EMULATOR_REG                                 *((volatile uint32_t *)0x40009000u)
#define I_AM_AN_EMULATOR_ADDR                                (0x40009000u)
#define I_AM_AN_EMULATOR_RESET                               (0x00000000u)
        /* I_AM_AN_EMULATOR field */
        #define I_AM_AN_EMULATOR_I_AM_AN_EMULATOR            (0x00000001u)
        #define I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_MASK       (0x00000001u)
        #define I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_BIT        (0)
        #define I_AM_AN_EMULATOR_I_AM_AN_EMULATOR_BITS       (1)

/* INTERRUPTS block */
#define BLOCK_INTERRUPTS_BASE                                (0x4000A000u)
#define BLOCK_INTERRUPTS_END                                 (0x4000A86Cu)
#define BLOCK_INTERRUPTS_SIZE                                (BLOCK_INTERRUPTS_END - BLOCK_INTERRUPTS_BASE + 1)

#define MAC_RX_INT_SRC                                       *((volatile uint32_t *)0x4000A000u)
#define MAC_RX_INT_SRC_REG                                   *((volatile uint32_t *)0x4000A000u)
#define MAC_RX_INT_SRC_ADDR                                  (0x4000A000u)
#define MAC_RX_INT_SRC_RESET                                 (0x00000000u)
        /* TX_B_ACK_ERR_SRC field */
        #define MAC_RX_INT_SRC_TX_B_ACK_ERR_SRC              (0x00008000u)
        #define MAC_RX_INT_SRC_TX_B_ACK_ERR_SRC_MASK         (0x00008000u)
        #define MAC_RX_INT_SRC_TX_B_ACK_ERR_SRC_BIT          (15)
        #define MAC_RX_INT_SRC_TX_B_ACK_ERR_SRC_BITS         (1)
        /* TX_A_ACK_ERR_SRC field */
        #define MAC_RX_INT_SRC_TX_A_ACK_ERR_SRC              (0x00004000u)
        #define MAC_RX_INT_SRC_TX_A_ACK_ERR_SRC_MASK         (0x00004000u)
        #define MAC_RX_INT_SRC_TX_A_ACK_ERR_SRC_BIT          (14)
        #define MAC_RX_INT_SRC_TX_A_ACK_ERR_SRC_BITS         (1)
        /* RX_OVFLW_SRC field */
        #define MAC_RX_INT_SRC_RX_OVFLW_SRC                  (0x00002000u)
        #define MAC_RX_INT_SRC_RX_OVFLW_SRC_MASK             (0x00002000u)
        #define MAC_RX_INT_SRC_RX_OVFLW_SRC_BIT              (13)
        #define MAC_RX_INT_SRC_RX_OVFLW_SRC_BITS             (1)
        /* RX_ERROR_SRC field */
        #define MAC_RX_INT_SRC_RX_ERROR_SRC                  (0x00001000u)
        #define MAC_RX_INT_SRC_RX_ERROR_SRC_MASK             (0x00001000u)
        #define MAC_RX_INT_SRC_RX_ERROR_SRC_BIT              (12)
        #define MAC_RX_INT_SRC_RX_ERROR_SRC_BITS             (1)
        /* BB_RX_LEN_ERR_SRC field */
        #define MAC_RX_INT_SRC_BB_RX_LEN_ERR_SRC             (0x00000800u)
        #define MAC_RX_INT_SRC_BB_RX_LEN_ERR_SRC_MASK        (0x00000800u)
        #define MAC_RX_INT_SRC_BB_RX_LEN_ERR_SRC_BIT         (11)
        #define MAC_RX_INT_SRC_BB_RX_LEN_ERR_SRC_BITS        (1)
        /* TX_COLL_RX_SRC field */
        #define MAC_RX_INT_SRC_TX_COLL_RX_SRC                (0x00000400u)
        #define MAC_RX_INT_SRC_TX_COLL_RX_SRC_MASK           (0x00000400u)
        #define MAC_RX_INT_SRC_TX_COLL_RX_SRC_BIT            (10)
        #define MAC_RX_INT_SRC_TX_COLL_RX_SRC_BITS           (1)
        /* RSSI_INST_MEAS_SRC field */
        #define MAC_RX_INT_SRC_RSSI_INST_MEAS_SRC            (0x00000200u)
        #define MAC_RX_INT_SRC_RSSI_INST_MEAS_SRC_MASK       (0x00000200u)
        #define MAC_RX_INT_SRC_RSSI_INST_MEAS_SRC_BIT        (9)
        #define MAC_RX_INT_SRC_RSSI_INST_MEAS_SRC_BITS       (1)
        /* TX_B_ACK_SRC field */
        #define MAC_RX_INT_SRC_TX_B_ACK_SRC                  (0x00000100u)
        #define MAC_RX_INT_SRC_TX_B_ACK_SRC_MASK             (0x00000100u)
        #define MAC_RX_INT_SRC_TX_B_ACK_SRC_BIT              (8)
        #define MAC_RX_INT_SRC_TX_B_ACK_SRC_BITS             (1)
        /* TX_A_ACK_SRC field */
        #define MAC_RX_INT_SRC_TX_A_ACK_SRC                  (0x00000080u)
        #define MAC_RX_INT_SRC_TX_A_ACK_SRC_MASK             (0x00000080u)
        #define MAC_RX_INT_SRC_TX_A_ACK_SRC_BIT              (7)
        #define MAC_RX_INT_SRC_TX_A_ACK_SRC_BITS             (1)
        /* RX_B_UNLOAD_COMP_SRC field */
        #define MAC_RX_INT_SRC_RX_B_UNLOAD_COMP_SRC          (0x00000040u)
        #define MAC_RX_INT_SRC_RX_B_UNLOAD_COMP_SRC_MASK     (0x00000040u)
        #define MAC_RX_INT_SRC_RX_B_UNLOAD_COMP_SRC_BIT      (6)
        #define MAC_RX_INT_SRC_RX_B_UNLOAD_COMP_SRC_BITS     (1)
        /* RX_A_UNLOAD_COMP_SRC field */
        #define MAC_RX_INT_SRC_RX_A_UNLOAD_COMP_SRC          (0x00000020u)
        #define MAC_RX_INT_SRC_RX_A_UNLOAD_COMP_SRC_MASK     (0x00000020u)
        #define MAC_RX_INT_SRC_RX_A_UNLOAD_COMP_SRC_BIT      (5)
        #define MAC_RX_INT_SRC_RX_A_UNLOAD_COMP_SRC_BITS     (1)
        /* RX_B_ADDR_REC_SRC field */
        #define MAC_RX_INT_SRC_RX_B_ADDR_REC_SRC             (0x00000010u)
        #define MAC_RX_INT_SRC_RX_B_ADDR_REC_SRC_MASK        (0x00000010u)
        #define MAC_RX_INT_SRC_RX_B_ADDR_REC_SRC_BIT         (4)
        #define MAC_RX_INT_SRC_RX_B_ADDR_REC_SRC_BITS        (1)
        /* RX_A_ADDR_REC_SRC field */
        #define MAC_RX_INT_SRC_RX_A_ADDR_REC_SRC             (0x00000008u)
        #define MAC_RX_INT_SRC_RX_A_ADDR_REC_SRC_MASK        (0x00000008u)
        #define MAC_RX_INT_SRC_RX_A_ADDR_REC_SRC_BIT         (3)
        #define MAC_RX_INT_SRC_RX_A_ADDR_REC_SRC_BITS        (1)
        /* RX_B_FILT_COMP_SRC field */
        #define MAC_RX_INT_SRC_RX_B_FILT_COMP_SRC            (0x00000004u)
        #define MAC_RX_INT_SRC_RX_B_FILT_COMP_SRC_MASK       (0x00000004u)
        #define MAC_RX_INT_SRC_RX_B_FILT_COMP_SRC_BIT        (2)
        #define MAC_RX_INT_SRC_RX_B_FILT_COMP_SRC_BITS       (1)
        /* RX_A_FILT_COMP_SRC field */
        #define MAC_RX_INT_SRC_RX_A_FILT_COMP_SRC            (0x00000002u)
        #define MAC_RX_INT_SRC_RX_A_FILT_COMP_SRC_MASK       (0x00000002u)
        #define MAC_RX_INT_SRC_RX_A_FILT_COMP_SRC_BIT        (1)
        #define MAC_RX_INT_SRC_RX_A_FILT_COMP_SRC_BITS       (1)
        /* RX_FRAME_SRC field */
        #define MAC_RX_INT_SRC_RX_FRAME_SRC                  (0x00000001u)
        #define MAC_RX_INT_SRC_RX_FRAME_SRC_MASK             (0x00000001u)
        #define MAC_RX_INT_SRC_RX_FRAME_SRC_BIT              (0)
        #define MAC_RX_INT_SRC_RX_FRAME_SRC_BITS             (1)

#define MAC_TX_INT_SRC                                       *((volatile uint32_t *)0x4000A004u)
#define MAC_TX_INT_SRC_REG                                   *((volatile uint32_t *)0x4000A004u)
#define MAC_TX_INT_SRC_ADDR                                  (0x4000A004u)
#define MAC_TX_INT_SRC_RESET                                 (0x00000000u)
        /* RX_B_ACK_SRC field */
        #define MAC_TX_INT_SRC_RX_B_ACK_SRC                  (0x00000800u)
        #define MAC_TX_INT_SRC_RX_B_ACK_SRC_MASK             (0x00000800u)
        #define MAC_TX_INT_SRC_RX_B_ACK_SRC_BIT              (11)
        #define MAC_TX_INT_SRC_RX_B_ACK_SRC_BITS             (1)
        /* RX_A_ACK_SRC field */
        #define MAC_TX_INT_SRC_RX_A_ACK_SRC                  (0x00000400u)
        #define MAC_TX_INT_SRC_RX_A_ACK_SRC_MASK             (0x00000400u)
        #define MAC_TX_INT_SRC_RX_A_ACK_SRC_BIT              (10)
        #define MAC_TX_INT_SRC_RX_A_ACK_SRC_BITS             (1)
        /* TX_B_UNLOAD_SRC field */
        #define MAC_TX_INT_SRC_TX_B_UNLOAD_SRC               (0x00000200u)
        #define MAC_TX_INT_SRC_TX_B_UNLOAD_SRC_MASK          (0x00000200u)
        #define MAC_TX_INT_SRC_TX_B_UNLOAD_SRC_BIT           (9)
        #define MAC_TX_INT_SRC_TX_B_UNLOAD_SRC_BITS          (1)
        /* TX_A_UNLOAD_SRC field */
        #define MAC_TX_INT_SRC_TX_A_UNLOAD_SRC               (0x00000100u)
        #define MAC_TX_INT_SRC_TX_A_UNLOAD_SRC_MASK          (0x00000100u)
        #define MAC_TX_INT_SRC_TX_A_UNLOAD_SRC_BIT           (8)
        #define MAC_TX_INT_SRC_TX_A_UNLOAD_SRC_BITS          (1)
        /* ACK_EXPIRED_SRC field */
        #define MAC_TX_INT_SRC_ACK_EXPIRED_SRC               (0x00000080u)
        #define MAC_TX_INT_SRC_ACK_EXPIRED_SRC_MASK          (0x00000080u)
        #define MAC_TX_INT_SRC_ACK_EXPIRED_SRC_BIT           (7)
        #define MAC_TX_INT_SRC_ACK_EXPIRED_SRC_BITS          (1)
        /* TX_LOCK_FAIL_SRC field */
        #define MAC_TX_INT_SRC_TX_LOCK_FAIL_SRC              (0x00000040u)
        #define MAC_TX_INT_SRC_TX_LOCK_FAIL_SRC_MASK         (0x00000040u)
        #define MAC_TX_INT_SRC_TX_LOCK_FAIL_SRC_BIT          (6)
        #define MAC_TX_INT_SRC_TX_LOCK_FAIL_SRC_BITS         (1)
        /* TX_UNDERFLOW_SRC field */
        #define MAC_TX_INT_SRC_TX_UNDERFLOW_SRC              (0x00000020u)
        #define MAC_TX_INT_SRC_TX_UNDERFLOW_SRC_MASK         (0x00000020u)
        #define MAC_TX_INT_SRC_TX_UNDERFLOW_SRC_BIT          (5)
        #define MAC_TX_INT_SRC_TX_UNDERFLOW_SRC_BITS         (1)
        /* CCA_FAIL_SRC field */
        #define MAC_TX_INT_SRC_CCA_FAIL_SRC                  (0x00000010u)
        #define MAC_TX_INT_SRC_CCA_FAIL_SRC_MASK             (0x00000010u)
        #define MAC_TX_INT_SRC_CCA_FAIL_SRC_BIT              (4)
        #define MAC_TX_INT_SRC_CCA_FAIL_SRC_BITS             (1)
        /* SFD_SENT_SRC field */
        #define MAC_TX_INT_SRC_SFD_SENT_SRC                  (0x00000008u)
        #define MAC_TX_INT_SRC_SFD_SENT_SRC_MASK             (0x00000008u)
        #define MAC_TX_INT_SRC_SFD_SENT_SRC_BIT              (3)
        #define MAC_TX_INT_SRC_SFD_SENT_SRC_BITS             (1)
        /* BO_COMPLETE_SRC field */
        #define MAC_TX_INT_SRC_BO_COMPLETE_SRC               (0x00000004u)
        #define MAC_TX_INT_SRC_BO_COMPLETE_SRC_MASK          (0x00000004u)
        #define MAC_TX_INT_SRC_BO_COMPLETE_SRC_BIT           (2)
        #define MAC_TX_INT_SRC_BO_COMPLETE_SRC_BITS          (1)
        /* RX_ACK_SRC field */
        #define MAC_TX_INT_SRC_RX_ACK_SRC                    (0x00000002u)
        #define MAC_TX_INT_SRC_RX_ACK_SRC_MASK               (0x00000002u)
        #define MAC_TX_INT_SRC_RX_ACK_SRC_BIT                (1)
        #define MAC_TX_INT_SRC_RX_ACK_SRC_BITS               (1)
        /* TX_COMPLETE_SRC field */
        #define MAC_TX_INT_SRC_TX_COMPLETE_SRC               (0x00000001u)
        #define MAC_TX_INT_SRC_TX_COMPLETE_SRC_MASK          (0x00000001u)
        #define MAC_TX_INT_SRC_TX_COMPLETE_SRC_BIT           (0)
        #define MAC_TX_INT_SRC_TX_COMPLETE_SRC_BITS          (1)

#define MAC_TIMER_INT_SRC                                    *((volatile uint32_t *)0x4000A008u)
#define MAC_TIMER_INT_SRC_REG                                *((volatile uint32_t *)0x4000A008u)
#define MAC_TIMER_INT_SRC_ADDR                               (0x4000A008u)
#define MAC_TIMER_INT_SRC_RESET                              (0x00000000u)
        /* TIMER_COMP_B_SRC field */
        #define MAC_TIMER_INT_SRC_TIMER_COMP_B_SRC           (0x00000004u)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_B_SRC_MASK      (0x00000004u)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_B_SRC_BIT       (2)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_B_SRC_BITS      (1)
        /* TIMER_COMP_A_SRC field */
        #define MAC_TIMER_INT_SRC_TIMER_COMP_A_SRC           (0x00000002u)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_A_SRC_MASK      (0x00000002u)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_A_SRC_BIT       (1)
        #define MAC_TIMER_INT_SRC_TIMER_COMP_A_SRC_BITS      (1)
        /* TIMER_WRAP_SRC field */
        #define MAC_TIMER_INT_SRC_TIMER_WRAP_SRC             (0x00000001u)
        #define MAC_TIMER_INT_SRC_TIMER_WRAP_SRC_MASK        (0x00000001u)
        #define MAC_TIMER_INT_SRC_TIMER_WRAP_SRC_BIT         (0)
        #define MAC_TIMER_INT_SRC_TIMER_WRAP_SRC_BITS        (1)

#define BB_INT_SRC                                           *((volatile uint32_t *)0x4000A00Cu)
#define BB_INT_SRC_REG                                       *((volatile uint32_t *)0x4000A00Cu)
#define BB_INT_SRC_ADDR                                      (0x4000A00Cu)
#define BB_INT_SRC_RESET                                     (0x00000000u)
        /* RSSI_INT_SRC field */
        #define BB_INT_SRC_RSSI_INT_SRC                      (0x00000002u)
        #define BB_INT_SRC_RSSI_INT_SRC_MASK                 (0x00000002u)
        #define BB_INT_SRC_RSSI_INT_SRC_BIT                  (1)
        #define BB_INT_SRC_RSSI_INT_SRC_BITS                 (1)
        /* BASEBAND_INT_SRC field */
        #define BB_INT_SRC_BASEBAND_INT_SRC                  (0x00000001u)
        #define BB_INT_SRC_BASEBAND_INT_SRC_MASK             (0x00000001u)
        #define BB_INT_SRC_BASEBAND_INT_SRC_BIT              (0)
        #define BB_INT_SRC_BASEBAND_INT_SRC_BITS             (1)

#define SEC_INT_SRC                                          *((volatile uint32_t *)0x4000A010u)
#define SEC_INT_SRC_REG                                      *((volatile uint32_t *)0x4000A010u)
#define SEC_INT_SRC_ADDR                                     (0x4000A010u)
#define SEC_INT_SRC_RESET                                    (0x00000000u)
        /* CT_WORD_VALID_SRC field */
        #define SEC_INT_SRC_CT_WORD_VALID_SRC                (0x00000004u)
        #define SEC_INT_SRC_CT_WORD_VALID_SRC_MASK           (0x00000004u)
        #define SEC_INT_SRC_CT_WORD_VALID_SRC_BIT            (2)
        #define SEC_INT_SRC_CT_WORD_VALID_SRC_BITS           (1)
        /* PT_WORD_REQ_SRC field */
        #define SEC_INT_SRC_PT_WORD_REQ_SRC                  (0x00000002u)
        #define SEC_INT_SRC_PT_WORD_REQ_SRC_MASK             (0x00000002u)
        #define SEC_INT_SRC_PT_WORD_REQ_SRC_BIT              (1)
        #define SEC_INT_SRC_PT_WORD_REQ_SRC_BITS             (1)
        /* ENC_COMPLETE_SRC field */
        #define SEC_INT_SRC_ENC_COMPLETE_SRC                 (0x00000001u)
        #define SEC_INT_SRC_ENC_COMPLETE_SRC_MASK            (0x00000001u)
        #define SEC_INT_SRC_ENC_COMPLETE_SRC_BIT             (0)
        #define SEC_INT_SRC_ENC_COMPLETE_SRC_BITS            (1)

#define INT_SLEEPTMRFLAG                                     *((volatile uint32_t *)0x4000A014u)
#define INT_SLEEPTMRFLAG_REG                                 *((volatile uint32_t *)0x4000A014u)
#define INT_SLEEPTMRFLAG_ADDR                                (0x4000A014u)
#define INT_SLEEPTMRFLAG_RESET                               (0x00000000u)
        /* INT_SLEEPTMRCMPB field */
        #define INT_SLEEPTMRCMPB                             (0x00000004u)
        #define INT_SLEEPTMRCMPB_MASK                        (0x00000004u)
        #define INT_SLEEPTMRCMPB_BIT                         (2)
        #define INT_SLEEPTMRCMPB_BITS                        (1)
        /* INT_SLEEPTMRCMPA field */
        #define INT_SLEEPTMRCMPA                             (0x00000002u)
        #define INT_SLEEPTMRCMPA_MASK                        (0x00000002u)
        #define INT_SLEEPTMRCMPA_BIT                         (1)
        #define INT_SLEEPTMRCMPA_BITS                        (1)
        /* INT_SLEEPTMRWRAP field */
        #define INT_SLEEPTMRWRAP                             (0x00000001u)
        #define INT_SLEEPTMRWRAP_MASK                        (0x00000001u)
        #define INT_SLEEPTMRWRAP_BIT                         (0)
        #define INT_SLEEPTMRWRAP_BITS                        (1)

#define INT_MGMTFLAG                                         *((volatile uint32_t *)0x4000A018u)
#define INT_MGMTFLAG_REG                                     *((volatile uint32_t *)0x4000A018u)
#define INT_MGMTFLAG_ADDR                                    (0x4000A018u)
#define INT_MGMTFLAG_RESET                                   (0x00000000u)
        /* INT_MGMTDMAPROT field */
        #define INT_MGMTDMAPROT                              (0x00000010u)
        #define INT_MGMTDMAPROT_MASK                         (0x00000010u)
        #define INT_MGMTDMAPROT_BIT                          (4)
        #define INT_MGMTDMAPROT_BITS                         (1)
        /* INT_MGMTCALADC field */
        #define INT_MGMTCALADC                               (0x00000008u)
        #define INT_MGMTCALADC_MASK                          (0x00000008u)
        #define INT_MGMTCALADC_BIT                           (3)
        #define INT_MGMTCALADC_BITS                          (1)
        /* INT_MGMTFPEC field */
        #define INT_MGMTFPEC                                 (0x00000004u)
        #define INT_MGMTFPEC_MASK                            (0x00000004u)
        #define INT_MGMTFPEC_BIT                             (2)
        #define INT_MGMTFPEC_BITS                            (1)
        /* INT_MGMTOSC24MHI field */
        #define INT_MGMTOSC24MHI                             (0x00000002u)
        #define INT_MGMTOSC24MHI_MASK                        (0x00000002u)
        #define INT_MGMTOSC24MHI_BIT                         (1)
        #define INT_MGMTOSC24MHI_BITS                        (1)
        /* INT_MGMTOSC24MLO field */
        #define INT_MGMTOSC24MLO                             (0x00000001u)
        #define INT_MGMTOSC24MLO_MASK                        (0x00000001u)
        #define INT_MGMTOSC24MLO_BIT                         (0)
        #define INT_MGMTOSC24MLO_BITS                        (1)

#define INT_NMIFLAG                                          *((volatile uint32_t *)0x4000A01Cu)
#define INT_NMIFLAG_REG                                      *((volatile uint32_t *)0x4000A01Cu)
#define INT_NMIFLAG_ADDR                                     (0x4000A01Cu)
#define INT_NMIFLAG_RESET                                    (0x00000000u)
        /* INT_NMICLK24M field */
        #define INT_NMICLK24M                                (0x00000002u)
        #define INT_NMICLK24M_MASK                           (0x00000002u)
        #define INT_NMICLK24M_BIT                            (1)
        #define INT_NMICLK24M_BITS                           (1)
        /* INT_NMIWDOG field */
        #define INT_NMIWDOG                                  (0x00000001u)
        #define INT_NMIWDOG_MASK                             (0x00000001u)
        #define INT_NMIWDOG_BIT                              (0)
        #define INT_NMIWDOG_BITS                             (1)

#define INT_SLEEPTMRFORCE                                    *((volatile uint32_t *)0x4000A020u)
#define INT_SLEEPTMRFORCE_REG                                *((volatile uint32_t *)0x4000A020u)
#define INT_SLEEPTMRFORCE_ADDR                               (0x4000A020u)
#define INT_SLEEPTMRFORCE_RESET                              (0x00000000u)
        /* INT_SLEEPTMRCMPB field */
        #define INT_SLEEPTMRCMPB                             (0x00000004u)
        #define INT_SLEEPTMRCMPB_MASK                        (0x00000004u)
        #define INT_SLEEPTMRCMPB_BIT                         (2)
        #define INT_SLEEPTMRCMPB_BITS                        (1)
        /* INT_SLEEPTMRCMPA field */
        #define INT_SLEEPTMRCMPA                             (0x00000002u)
        #define INT_SLEEPTMRCMPA_MASK                        (0x00000002u)
        #define INT_SLEEPTMRCMPA_BIT                         (1)
        #define INT_SLEEPTMRCMPA_BITS                        (1)
        /* INT_SLEEPTMRWRAP field */
        #define INT_SLEEPTMRWRAP                             (0x00000001u)
        #define INT_SLEEPTMRWRAP_MASK                        (0x00000001u)
        #define INT_SLEEPTMRWRAP_BIT                         (0)
        #define INT_SLEEPTMRWRAP_BITS                        (1)

#define TEST_FORCE_ALL_INT                                   *((volatile uint32_t *)0x4000A024u)
#define TEST_FORCE_ALL_INT_REG                               *((volatile uint32_t *)0x4000A024u)
#define TEST_FORCE_ALL_INT_ADDR                              (0x4000A024u)
#define TEST_FORCE_ALL_INT_RESET                             (0x00000000u)
        /* FORCE_ALL_INT field */
        #define TEST_FORCE_ALL_INT_FORCE_ALL_INT             (0x00000001u)
        #define TEST_FORCE_ALL_INT_FORCE_ALL_INT_MASK        (0x00000001u)
        #define TEST_FORCE_ALL_INT_FORCE_ALL_INT_BIT         (0)
        #define TEST_FORCE_ALL_INT_FORCE_ALL_INT_BITS        (1)

#define MAC_RX_INT_MASK                                      *((volatile uint32_t *)0x4000A040u)
#define MAC_RX_INT_MASK_REG                                  *((volatile uint32_t *)0x4000A040u)
#define MAC_RX_INT_MASK_ADDR                                 (0x4000A040u)
#define MAC_RX_INT_MASK_RESET                                (0x00000000u)
        /* TX_B_ACK_ERR_MSK field */
        #define MAC_RX_INT_MASK_TX_B_ACK_ERR_MSK             (0x00008000u)
        #define MAC_RX_INT_MASK_TX_B_ACK_ERR_MSK_MASK        (0x00008000u)
        #define MAC_RX_INT_MASK_TX_B_ACK_ERR_MSK_BIT         (15)
        #define MAC_RX_INT_MASK_TX_B_ACK_ERR_MSK_BITS        (1)
        /* TX_A_ACK_ERR_MSK field */
        #define MAC_RX_INT_MASK_TX_A_ACK_ERR_MSK             (0x00004000u)
        #define MAC_RX_INT_MASK_TX_A_ACK_ERR_MSK_MASK        (0x00004000u)
        #define MAC_RX_INT_MASK_TX_A_ACK_ERR_MSK_BIT         (14)
        #define MAC_RX_INT_MASK_TX_A_ACK_ERR_MSK_BITS        (1)
        /* RX_OVFLW_MSK field */
        #define MAC_RX_INT_MASK_RX_OVFLW_MSK                 (0x00002000u)
        #define MAC_RX_INT_MASK_RX_OVFLW_MSK_MASK            (0x00002000u)
        #define MAC_RX_INT_MASK_RX_OVFLW_MSK_BIT             (13)
        #define MAC_RX_INT_MASK_RX_OVFLW_MSK_BITS            (1)
        /* RX_ERROR_MSK field */
        #define MAC_RX_INT_MASK_RX_ERROR_MSK                 (0x00001000u)
        #define MAC_RX_INT_MASK_RX_ERROR_MSK_MASK            (0x00001000u)
        #define MAC_RX_INT_MASK_RX_ERROR_MSK_BIT             (12)
        #define MAC_RX_INT_MASK_RX_ERROR_MSK_BITS            (1)
        /* BB_RX_LEN_ERR_MSK field */
        #define MAC_RX_INT_MASK_BB_RX_LEN_ERR_MSK            (0x00000800u)
        #define MAC_RX_INT_MASK_BB_RX_LEN_ERR_MSK_MASK       (0x00000800u)
        #define MAC_RX_INT_MASK_BB_RX_LEN_ERR_MSK_BIT        (11)
        #define MAC_RX_INT_MASK_BB_RX_LEN_ERR_MSK_BITS       (1)
        /* TX_COLL_RX_MSK field */
        #define MAC_RX_INT_MASK_TX_COLL_RX_MSK               (0x00000400u)
        #define MAC_RX_INT_MASK_TX_COLL_RX_MSK_MASK          (0x00000400u)
        #define MAC_RX_INT_MASK_TX_COLL_RX_MSK_BIT           (10)
        #define MAC_RX_INT_MASK_TX_COLL_RX_MSK_BITS          (1)
        /* RSSI_INST_MEAS_MSK field */
        #define MAC_RX_INT_MASK_RSSI_INST_MEAS_MSK           (0x00000200u)
        #define MAC_RX_INT_MASK_RSSI_INST_MEAS_MSK_MASK      (0x00000200u)
        #define MAC_RX_INT_MASK_RSSI_INST_MEAS_MSK_BIT       (9)
        #define MAC_RX_INT_MASK_RSSI_INST_MEAS_MSK_BITS      (1)
        /* TX_B_ACK_MSK field */
        #define MAC_RX_INT_MASK_TX_B_ACK_MSK                 (0x00000100u)
        #define MAC_RX_INT_MASK_TX_B_ACK_MSK_MASK            (0x00000100u)
        #define MAC_RX_INT_MASK_TX_B_ACK_MSK_BIT             (8)
        #define MAC_RX_INT_MASK_TX_B_ACK_MSK_BITS            (1)
        /* TX_A_ACK_MSK field */
        #define MAC_RX_INT_MASK_TX_A_ACK_MSK                 (0x00000080u)
        #define MAC_RX_INT_MASK_TX_A_ACK_MSK_MASK            (0x00000080u)
        #define MAC_RX_INT_MASK_TX_A_ACK_MSK_BIT             (7)
        #define MAC_RX_INT_MASK_TX_A_ACK_MSK_BITS            (1)
        /* RX_B_UNLOAD_COMP_MSK field */
        #define MAC_RX_INT_MASK_RX_B_UNLOAD_COMP_MSK         (0x00000040u)
        #define MAC_RX_INT_MASK_RX_B_UNLOAD_COMP_MSK_MASK    (0x00000040u)
        #define MAC_RX_INT_MASK_RX_B_UNLOAD_COMP_MSK_BIT     (6)
        #define MAC_RX_INT_MASK_RX_B_UNLOAD_COMP_MSK_BITS    (1)
        /* RX_A_UNLOAD_COMP_MSK field */
        #define MAC_RX_INT_MASK_RX_A_UNLOAD_COMP_MSK         (0x00000020u)
        #define MAC_RX_INT_MASK_RX_A_UNLOAD_COMP_MSK_MASK    (0x00000020u)
        #define MAC_RX_INT_MASK_RX_A_UNLOAD_COMP_MSK_BIT     (5)
        #define MAC_RX_INT_MASK_RX_A_UNLOAD_COMP_MSK_BITS    (1)
        /* RX_B_ADDR_REC_MSK field */
        #define MAC_RX_INT_MASK_RX_B_ADDR_REC_MSK            (0x00000010u)
        #define MAC_RX_INT_MASK_RX_B_ADDR_REC_MSK_MASK       (0x00000010u)
        #define MAC_RX_INT_MASK_RX_B_ADDR_REC_MSK_BIT        (4)
        #define MAC_RX_INT_MASK_RX_B_ADDR_REC_MSK_BITS       (1)
        /* RX_A_ADDR_REC_MSK field */
        #define MAC_RX_INT_MASK_RX_A_ADDR_REC_MSK            (0x00000008u)
        #define MAC_RX_INT_MASK_RX_A_ADDR_REC_MSK_MASK       (0x00000008u)
        #define MAC_RX_INT_MASK_RX_A_ADDR_REC_MSK_BIT        (3)
        #define MAC_RX_INT_MASK_RX_A_ADDR_REC_MSK_BITS       (1)
        /* RX_B_FILT_COMP_MSK field */
        #define MAC_RX_INT_MASK_RX_B_FILT_COMP_MSK           (0x00000004u)
        #define MAC_RX_INT_MASK_RX_B_FILT_COMP_MSK_MASK      (0x00000004u)
        #define MAC_RX_INT_MASK_RX_B_FILT_COMP_MSK_BIT       (2)
        #define MAC_RX_INT_MASK_RX_B_FILT_COMP_MSK_BITS      (1)
        /* RX_A_FILT_COMP_MSK field */
        #define MAC_RX_INT_MASK_RX_A_FILT_COMP_MSK           (0x00000002u)
        #define MAC_RX_INT_MASK_RX_A_FILT_COMP_MSK_MASK      (0x00000002u)
        #define MAC_RX_INT_MASK_RX_A_FILT_COMP_MSK_BIT       (1)
        #define MAC_RX_INT_MASK_RX_A_FILT_COMP_MSK_BITS      (1)
        /* RX_FRAME_MSK field */
        #define MAC_RX_INT_MASK_RX_FRAME_MSK                 (0x00000001u)
        #define MAC_RX_INT_MASK_RX_FRAME_MSK_MASK            (0x00000001u)
        #define MAC_RX_INT_MASK_RX_FRAME_MSK_BIT             (0)
        #define MAC_RX_INT_MASK_RX_FRAME_MSK_BITS            (1)

#define MAC_TX_INT_MASK                                      *((volatile uint32_t *)0x4000A044u)
#define MAC_TX_INT_MASK_REG                                  *((volatile uint32_t *)0x4000A044u)
#define MAC_TX_INT_MASK_ADDR                                 (0x4000A044u)
#define MAC_TX_INT_MASK_RESET                                (0x00000000u)
        /* RX_B_ACK_MSK field */
        #define MAC_TX_INT_MASK_RX_B_ACK_MSK                 (0x00000800u)
        #define MAC_TX_INT_MASK_RX_B_ACK_MSK_MASK            (0x00000800u)
        #define MAC_TX_INT_MASK_RX_B_ACK_MSK_BIT             (11)
        #define MAC_TX_INT_MASK_RX_B_ACK_MSK_BITS            (1)
        /* RX_A_ACK_MSK field */
        #define MAC_TX_INT_MASK_RX_A_ACK_MSK                 (0x00000400u)
        #define MAC_TX_INT_MASK_RX_A_ACK_MSK_MASK            (0x00000400u)
        #define MAC_TX_INT_MASK_RX_A_ACK_MSK_BIT             (10)
        #define MAC_TX_INT_MASK_RX_A_ACK_MSK_BITS            (1)
        /* TX_B_UNLOAD_MSK field */
        #define MAC_TX_INT_MASK_TX_B_UNLOAD_MSK              (0x00000200u)
        #define MAC_TX_INT_MASK_TX_B_UNLOAD_MSK_MASK         (0x00000200u)
        #define MAC_TX_INT_MASK_TX_B_UNLOAD_MSK_BIT          (9)
        #define MAC_TX_INT_MASK_TX_B_UNLOAD_MSK_BITS         (1)
        /* TX_A_UNLOAD_MSK field */
        #define MAC_TX_INT_MASK_TX_A_UNLOAD_MSK              (0x00000100u)
        #define MAC_TX_INT_MASK_TX_A_UNLOAD_MSK_MASK         (0x00000100u)
        #define MAC_TX_INT_MASK_TX_A_UNLOAD_MSK_BIT          (8)
        #define MAC_TX_INT_MASK_TX_A_UNLOAD_MSK_BITS         (1)
        /* ACK_EXPIRED_MSK field */
        #define MAC_TX_INT_MASK_ACK_EXPIRED_MSK              (0x00000080u)
        #define MAC_TX_INT_MASK_ACK_EXPIRED_MSK_MASK         (0x00000080u)
        #define MAC_TX_INT_MASK_ACK_EXPIRED_MSK_BIT          (7)
        #define MAC_TX_INT_MASK_ACK_EXPIRED_MSK_BITS         (1)
        /* TX_LOCK_FAIL_MSK field */
        #define MAC_TX_INT_MASK_TX_LOCK_FAIL_MSK             (0x00000040u)
        #define MAC_TX_INT_MASK_TX_LOCK_FAIL_MSK_MASK        (0x00000040u)
        #define MAC_TX_INT_MASK_TX_LOCK_FAIL_MSK_BIT         (6)
        #define MAC_TX_INT_MASK_TX_LOCK_FAIL_MSK_BITS        (1)
        /* TX_UNDERFLOW_MSK field */
        #define MAC_TX_INT_MASK_TX_UNDERFLOW_MSK             (0x00000020u)
        #define MAC_TX_INT_MASK_TX_UNDERFLOW_MSK_MASK        (0x00000020u)
        #define MAC_TX_INT_MASK_TX_UNDERFLOW_MSK_BIT         (5)
        #define MAC_TX_INT_MASK_TX_UNDERFLOW_MSK_BITS        (1)
        /* CCA_FAIL_MSK field */
        #define MAC_TX_INT_MASK_CCA_FAIL_MSK                 (0x00000010u)
        #define MAC_TX_INT_MASK_CCA_FAIL_MSK_MASK            (0x00000010u)
        #define MAC_TX_INT_MASK_CCA_FAIL_MSK_BIT             (4)
        #define MAC_TX_INT_MASK_CCA_FAIL_MSK_BITS            (1)
        /* SFD_SENT_MSK field */
        #define MAC_TX_INT_MASK_SFD_SENT_MSK                 (0x00000008u)
        #define MAC_TX_INT_MASK_SFD_SENT_MSK_MASK            (0x00000008u)
        #define MAC_TX_INT_MASK_SFD_SENT_MSK_BIT             (3)
        #define MAC_TX_INT_MASK_SFD_SENT_MSK_BITS            (1)
        /* BO_COMPLETE_MSK field */
        #define MAC_TX_INT_MASK_BO_COMPLETE_MSK              (0x00000004u)
        #define MAC_TX_INT_MASK_BO_COMPLETE_MSK_MASK         (0x00000004u)
        #define MAC_TX_INT_MASK_BO_COMPLETE_MSK_BIT          (2)
        #define MAC_TX_INT_MASK_BO_COMPLETE_MSK_BITS         (1)
        /* RX_ACK_MSK field */
        #define MAC_TX_INT_MASK_RX_ACK_MSK                   (0x00000002u)
        #define MAC_TX_INT_MASK_RX_ACK_MSK_MASK              (0x00000002u)
        #define MAC_TX_INT_MASK_RX_ACK_MSK_BIT               (1)
        #define MAC_TX_INT_MASK_RX_ACK_MSK_BITS              (1)
        /* TX_COMPLETE_MSK field */
        #define MAC_TX_INT_MASK_TX_COMPLETE_MSK              (0x00000001u)
        #define MAC_TX_INT_MASK_TX_COMPLETE_MSK_MASK         (0x00000001u)
        #define MAC_TX_INT_MASK_TX_COMPLETE_MSK_BIT          (0)
        #define MAC_TX_INT_MASK_TX_COMPLETE_MSK_BITS         (1)

#define MAC_TIMER_INT_MASK                                   *((volatile uint32_t *)0x4000A048u)
#define MAC_TIMER_INT_MASK_REG                               *((volatile uint32_t *)0x4000A048u)
#define MAC_TIMER_INT_MASK_ADDR                              (0x4000A048u)
#define MAC_TIMER_INT_MASK_RESET                             (0x00000000u)
        /* TIMER_COMP_B_MSK field */
        #define MAC_TIMER_INT_MASK_TIMER_COMP_B_MSK          (0x00000004u)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_B_MSK_MASK     (0x00000004u)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_B_MSK_BIT      (2)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_B_MSK_BITS     (1)
        /* TIMER_COMP_A_MSK field */
        #define MAC_TIMER_INT_MASK_TIMER_COMP_A_MSK          (0x00000002u)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_A_MSK_MASK     (0x00000002u)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_A_MSK_BIT      (1)
        #define MAC_TIMER_INT_MASK_TIMER_COMP_A_MSK_BITS     (1)
        /* TIMER_WRAP_MSK field */
        #define MAC_TIMER_INT_MASK_TIMER_WRAP_MSK            (0x00000001u)
        #define MAC_TIMER_INT_MASK_TIMER_WRAP_MSK_MASK       (0x00000001u)
        #define MAC_TIMER_INT_MASK_TIMER_WRAP_MSK_BIT        (0)
        #define MAC_TIMER_INT_MASK_TIMER_WRAP_MSK_BITS       (1)

#define BB_INT_MASK                                          *((volatile uint32_t *)0x4000A04Cu)
#define BB_INT_MASK_REG                                      *((volatile uint32_t *)0x4000A04Cu)
#define BB_INT_MASK_ADDR                                     (0x4000A04Cu)
#define BB_INT_MASK_RESET                                    (0x00000000u)
        /* RSSI_INT_MSK field */
        #define BB_INT_MASK_RSSI_INT_MSK                     (0x00000002u)
        #define BB_INT_MASK_RSSI_INT_MSK_MASK                (0x00000002u)
        #define BB_INT_MASK_RSSI_INT_MSK_BIT                 (1)
        #define BB_INT_MASK_RSSI_INT_MSK_BITS                (1)
        /* BASEBAND_INT_MSK field */
        #define BB_INT_MASK_BASEBAND_INT_MSK                 (0x00000001u)
        #define BB_INT_MASK_BASEBAND_INT_MSK_MASK            (0x00000001u)
        #define BB_INT_MASK_BASEBAND_INT_MSK_BIT             (0)
        #define BB_INT_MASK_BASEBAND_INT_MSK_BITS            (1)

#define SEC_INT_MASK                                         *((volatile uint32_t *)0x4000A050u)
#define SEC_INT_MASK_REG                                     *((volatile uint32_t *)0x4000A050u)
#define SEC_INT_MASK_ADDR                                    (0x4000A050u)
#define SEC_INT_MASK_RESET                                   (0x00000000u)
        /* CT_WORD_VALID_MSK field */
        #define SEC_INT_MASK_CT_WORD_VALID_MSK               (0x00000004u)
        #define SEC_INT_MASK_CT_WORD_VALID_MSK_MASK          (0x00000004u)
        #define SEC_INT_MASK_CT_WORD_VALID_MSK_BIT           (2)
        #define SEC_INT_MASK_CT_WORD_VALID_MSK_BITS          (1)
        /* PT_WORD_REQ_MSK field */
        #define SEC_INT_MASK_PT_WORD_REQ_MSK                 (0x00000002u)
        #define SEC_INT_MASK_PT_WORD_REQ_MSK_MASK            (0x00000002u)
        #define SEC_INT_MASK_PT_WORD_REQ_MSK_BIT             (1)
        #define SEC_INT_MASK_PT_WORD_REQ_MSK_BITS            (1)
        /* ENC_COMPLETE_MSK field */
        #define SEC_INT_MASK_ENC_COMPLETE_MSK                (0x00000001u)
        #define SEC_INT_MASK_ENC_COMPLETE_MSK_MASK           (0x00000001u)
        #define SEC_INT_MASK_ENC_COMPLETE_MSK_BIT            (0)
        #define SEC_INT_MASK_ENC_COMPLETE_MSK_BITS           (1)

#define INT_SLEEPTMRCFG                                      *((volatile uint32_t *)0x4000A054u)
#define INT_SLEEPTMRCFG_REG                                  *((volatile uint32_t *)0x4000A054u)
#define INT_SLEEPTMRCFG_ADDR                                 (0x4000A054u)
#define INT_SLEEPTMRCFG_RESET                                (0x00000000u)
        /* INT_SLEEPTMRCMPB field */
        #define INT_SLEEPTMRCMPB                             (0x00000004u)
        #define INT_SLEEPTMRCMPB_MASK                        (0x00000004u)
        #define INT_SLEEPTMRCMPB_BIT                         (2)
        #define INT_SLEEPTMRCMPB_BITS                        (1)
        /* INT_SLEEPTMRCMPA field */
        #define INT_SLEEPTMRCMPA                             (0x00000002u)
        #define INT_SLEEPTMRCMPA_MASK                        (0x00000002u)
        #define INT_SLEEPTMRCMPA_BIT                         (1)
        #define INT_SLEEPTMRCMPA_BITS                        (1)
        /* INT_SLEEPTMRWRAP field */
        #define INT_SLEEPTMRWRAP                             (0x00000001u)
        #define INT_SLEEPTMRWRAP_MASK                        (0x00000001u)
        #define INT_SLEEPTMRWRAP_BIT                         (0)
        #define INT_SLEEPTMRWRAP_BITS                        (1)

#define INT_MGMTCFG                                          *((volatile uint32_t *)0x4000A058u)
#define INT_MGMTCFG_REG                                      *((volatile uint32_t *)0x4000A058u)
#define INT_MGMTCFG_ADDR                                     (0x4000A058u)
#define INT_MGMTCFG_RESET                                    (0x00000000u)
        /* INT_MGMTDMAPROT field */
        #define INT_MGMTDMAPROT                              (0x00000010u)
        #define INT_MGMTDMAPROT_MASK                         (0x00000010u)
        #define INT_MGMTDMAPROT_BIT                          (4)
        #define INT_MGMTDMAPROT_BITS                         (1)
        /* INT_MGMTCALADC field */
        #define INT_MGMTCALADC                               (0x00000008u)
        #define INT_MGMTCALADC_MASK                          (0x00000008u)
        #define INT_MGMTCALADC_BIT                           (3)
        #define INT_MGMTCALADC_BITS                          (1)
        /* INT_MGMTFPEC field */
        #define INT_MGMTFPEC                                 (0x00000004u)
        #define INT_MGMTFPEC_MASK                            (0x00000004u)
        #define INT_MGMTFPEC_BIT                             (2)
        #define INT_MGMTFPEC_BITS                            (1)
        /* INT_MGMTOSC24MHI field */
        #define INT_MGMTOSC24MHI                             (0x00000002u)
        #define INT_MGMTOSC24MHI_MASK                        (0x00000002u)
        #define INT_MGMTOSC24MHI_BIT                         (1)
        #define INT_MGMTOSC24MHI_BITS                        (1)
        /* INT_MGMTOSC24MLO field */
        #define INT_MGMTOSC24MLO                             (0x00000001u)
        #define INT_MGMTOSC24MLO_MASK                        (0x00000001u)
        #define INT_MGMTOSC24MLO_BIT                         (0)
        #define INT_MGMTOSC24MLO_BITS                        (1)

#define INT_TIM1FLAG                                         *((volatile uint32_t *)0x4000A800u)
#define INT_TIM1FLAG_REG                                     *((volatile uint32_t *)0x4000A800u)
#define INT_TIM1FLAG_ADDR                                    (0x4000A800u)
#define INT_TIM1FLAG_RESET                                   (0x00000000u)
        /* INT_TIMRSVD field */
        #define INT_TIMRSVD                                  (0x00001E00u)
        #define INT_TIMRSVD_MASK                             (0x00001E00u)
        #define INT_TIMRSVD_BIT                              (9)
        #define INT_TIMRSVD_BITS                             (4)
        /* INT_TIMTIF field */
        #define INT_TIMTIF                                   (0x00000040u)
        #define INT_TIMTIF_MASK                              (0x00000040u)
        #define INT_TIMTIF_BIT                               (6)
        #define INT_TIMTIF_BITS                              (1)
        /* INT_TIMCC4IF field */
        #define INT_TIMCC4IF                                 (0x00000010u)
        #define INT_TIMCC4IF_MASK                            (0x00000010u)
        #define INT_TIMCC4IF_BIT                             (4)
        #define INT_TIMCC4IF_BITS                            (1)
        /* INT_TIMCC3IF field */
        #define INT_TIMCC3IF                                 (0x00000008u)
        #define INT_TIMCC3IF_MASK                            (0x00000008u)
        #define INT_TIMCC3IF_BIT                             (3)
        #define INT_TIMCC3IF_BITS                            (1)
        /* INT_TIMCC2IF field */
        #define INT_TIMCC2IF                                 (0x00000004u)
        #define INT_TIMCC2IF_MASK                            (0x00000004u)
        #define INT_TIMCC2IF_BIT                             (2)
        #define INT_TIMCC2IF_BITS                            (1)
        /* INT_TIMCC1IF field */
        #define INT_TIMCC1IF                                 (0x00000002u)
        #define INT_TIMCC1IF_MASK                            (0x00000002u)
        #define INT_TIMCC1IF_BIT                             (1)
        #define INT_TIMCC1IF_BITS                            (1)
        /* INT_TIMUIF field */
        #define INT_TIMUIF                                   (0x00000001u)
        #define INT_TIMUIF_MASK                              (0x00000001u)
        #define INT_TIMUIF_BIT                               (0)
        #define INT_TIMUIF_BITS                              (1)

#define INT_TIM2FLAG                                         *((volatile uint32_t *)0x4000A804u)
#define INT_TIM2FLAG_REG                                     *((volatile uint32_t *)0x4000A804u)
#define INT_TIM2FLAG_ADDR                                    (0x4000A804u)
#define INT_TIM2FLAG_RESET                                   (0x00000000u)
        /* INT_TIMRSVD field */
        #define INT_TIMRSVD                                  (0x00001E00u)
        #define INT_TIMRSVD_MASK                             (0x00001E00u)
        #define INT_TIMRSVD_BIT                              (9)
        #define INT_TIMRSVD_BITS                             (4)
        /* INT_TIMTIF field */
        #define INT_TIMTIF                                   (0x00000040u)
        #define INT_TIMTIF_MASK                              (0x00000040u)
        #define INT_TIMTIF_BIT                               (6)
        #define INT_TIMTIF_BITS                              (1)
        /* INT_TIMCC4IF field */
        #define INT_TIMCC4IF                                 (0x00000010u)
        #define INT_TIMCC4IF_MASK                            (0x00000010u)
        #define INT_TIMCC4IF_BIT                             (4)
        #define INT_TIMCC4IF_BITS                            (1)
        /* INT_TIMCC3IF field */
        #define INT_TIMCC3IF                                 (0x00000008u)
        #define INT_TIMCC3IF_MASK                            (0x00000008u)
        #define INT_TIMCC3IF_BIT                             (3)
        #define INT_TIMCC3IF_BITS                            (1)
        /* INT_TIMCC2IF field */
        #define INT_TIMCC2IF                                 (0x00000004u)
        #define INT_TIMCC2IF_MASK                            (0x00000004u)
        #define INT_TIMCC2IF_BIT                             (2)
        #define INT_TIMCC2IF_BITS                            (1)
        /* INT_TIMCC1IF field */
        #define INT_TIMCC1IF                                 (0x00000002u)
        #define INT_TIMCC1IF_MASK                            (0x00000002u)
        #define INT_TIMCC1IF_BIT                             (1)
        #define INT_TIMCC1IF_BITS                            (1)
        /* INT_TIMUIF field */
        #define INT_TIMUIF                                   (0x00000001u)
        #define INT_TIMUIF_MASK                              (0x00000001u)
        #define INT_TIMUIF_BIT                               (0)
        #define INT_TIMUIF_BITS                              (1)

#define INT_SC1FLAG                                          *((volatile uint32_t *)0x4000A808u)
#define INT_SC1FLAG_REG                                      *((volatile uint32_t *)0x4000A808u)
#define INT_SC1FLAG_ADDR                                     (0x4000A808u)
#define INT_SC1FLAG_RESET                                    (0x00000000u)
        /* INT_SC1PARERR field */
        #define INT_SC1PARERR                                (0x00004000u)
        #define INT_SC1PARERR_MASK                           (0x00004000u)
        #define INT_SC1PARERR_BIT                            (14)
        #define INT_SC1PARERR_BITS                           (1)
        /* INT_SC1FRMERR field */
        #define INT_SC1FRMERR                                (0x00002000u)
        #define INT_SC1FRMERR_MASK                           (0x00002000u)
        #define INT_SC1FRMERR_BIT                            (13)
        #define INT_SC1FRMERR_BITS                           (1)
        /* INT_SCTXULDB field */
        #define INT_SCTXULDB                                 (0x00001000u)
        #define INT_SCTXULDB_MASK                            (0x00001000u)
        #define INT_SCTXULDB_BIT                             (12)
        #define INT_SCTXULDB_BITS                            (1)
        /* INT_SCTXULDA field */
        #define INT_SCTXULDA                                 (0x00000800u)
        #define INT_SCTXULDA_MASK                            (0x00000800u)
        #define INT_SCTXULDA_BIT                             (11)
        #define INT_SCTXULDA_BITS                            (1)
        /* INT_SCRXULDB field */
        #define INT_SCRXULDB                                 (0x00000400u)
        #define INT_SCRXULDB_MASK                            (0x00000400u)
        #define INT_SCRXULDB_BIT                             (10)
        #define INT_SCRXULDB_BITS                            (1)
        /* INT_SCRXULDA field */
        #define INT_SCRXULDA                                 (0x00000200u)
        #define INT_SCRXULDA_MASK                            (0x00000200u)
        #define INT_SCRXULDA_BIT                             (9)
        #define INT_SCRXULDA_BITS                            (1)
        /* INT_SCNAK field */
        #define INT_SCNAK                                    (0x00000100u)
        #define INT_SCNAK_MASK                               (0x00000100u)
        #define INT_SCNAK_BIT                                (8)
        #define INT_SCNAK_BITS                               (1)
        /* INT_SCCMDFIN field */
        #define INT_SCCMDFIN                                 (0x00000080u)
        #define INT_SCCMDFIN_MASK                            (0x00000080u)
        #define INT_SCCMDFIN_BIT                             (7)
        #define INT_SCCMDFIN_BITS                            (1)
        /* INT_SCTXFIN field */
        #define INT_SCTXFIN                                  (0x00000040u)
        #define INT_SCTXFIN_MASK                             (0x00000040u)
        #define INT_SCTXFIN_BIT                              (6)
        #define INT_SCTXFIN_BITS                             (1)
        /* INT_SCRXFIN field */
        #define INT_SCRXFIN                                  (0x00000020u)
        #define INT_SCRXFIN_MASK                             (0x00000020u)
        #define INT_SCRXFIN_BIT                              (5)
        #define INT_SCRXFIN_BITS                             (1)
        /* INT_SCTXUND field */
        #define INT_SCTXUND                                  (0x00000010u)
        #define INT_SCTXUND_MASK                             (0x00000010u)
        #define INT_SCTXUND_BIT                              (4)
        #define INT_SCTXUND_BITS                             (1)
        /* INT_SCRXOVF field */
        #define INT_SCRXOVF                                  (0x00000008u)
        #define INT_SCRXOVF_MASK                             (0x00000008u)
        #define INT_SCRXOVF_BIT                              (3)
        #define INT_SCRXOVF_BITS                             (1)
        /* INT_SCTXIDLE field */
        #define INT_SCTXIDLE                                 (0x00000004u)
        #define INT_SCTXIDLE_MASK                            (0x00000004u)
        #define INT_SCTXIDLE_BIT                             (2)
        #define INT_SCTXIDLE_BITS                            (1)
        /* INT_SCTXFREE field */
        #define INT_SCTXFREE                                 (0x00000002u)
        #define INT_SCTXFREE_MASK                            (0x00000002u)
        #define INT_SCTXFREE_BIT                             (1)
        #define INT_SCTXFREE_BITS                            (1)
        /* INT_SCRXVAL field */
        #define INT_SCRXVAL                                  (0x00000001u)
        #define INT_SCRXVAL_MASK                             (0x00000001u)
        #define INT_SCRXVAL_BIT                              (0)
        #define INT_SCRXVAL_BITS                             (1)

#define INT_SC2FLAG                                          *((volatile uint32_t *)0x4000A80Cu)
#define INT_SC2FLAG_REG                                      *((volatile uint32_t *)0x4000A80Cu)
#define INT_SC2FLAG_ADDR                                     (0x4000A80Cu)
#define INT_SC2FLAG_RESET                                    (0x00000000u)
        /* INT_SCTXULDB field */
        #define INT_SCTXULDB                                 (0x00001000u)
        #define INT_SCTXULDB_MASK                            (0x00001000u)
        #define INT_SCTXULDB_BIT                             (12)
        #define INT_SCTXULDB_BITS                            (1)
        /* INT_SCTXULDA field */
        #define INT_SCTXULDA                                 (0x00000800u)
        #define INT_SCTXULDA_MASK                            (0x00000800u)
        #define INT_SCTXULDA_BIT                             (11)
        #define INT_SCTXULDA_BITS                            (1)
        /* INT_SCRXULDB field */
        #define INT_SCRXULDB                                 (0x00000400u)
        #define INT_SCRXULDB_MASK                            (0x00000400u)
        #define INT_SCRXULDB_BIT                             (10)
        #define INT_SCRXULDB_BITS                            (1)
        /* INT_SCRXULDA field */
        #define INT_SCRXULDA                                 (0x00000200u)
        #define INT_SCRXULDA_MASK                            (0x00000200u)
        #define INT_SCRXULDA_BIT                             (9)
        #define INT_SCRXULDA_BITS                            (1)
        /* INT_SCNAK field */
        #define INT_SCNAK                                    (0x00000100u)
        #define INT_SCNAK_MASK                               (0x00000100u)
        #define INT_SCNAK_BIT                                (8)
        #define INT_SCNAK_BITS                               (1)
        /* INT_SCCMDFIN field */
        #define INT_SCCMDFIN                                 (0x00000080u)
        #define INT_SCCMDFIN_MASK                            (0x00000080u)
        #define INT_SCCMDFIN_BIT                             (7)
        #define INT_SCCMDFIN_BITS                            (1)
        /* INT_SCTXFIN field */
        #define INT_SCTXFIN                                  (0x00000040u)
        #define INT_SCTXFIN_MASK                             (0x00000040u)
        #define INT_SCTXFIN_BIT                              (6)
        #define INT_SCTXFIN_BITS                             (1)
        /* INT_SCRXFIN field */
        #define INT_SCRXFIN                                  (0x00000020u)
        #define INT_SCRXFIN_MASK                             (0x00000020u)
        #define INT_SCRXFIN_BIT                              (5)
        #define INT_SCRXFIN_BITS                             (1)
        /* INT_SCTXUND field */
        #define INT_SCTXUND                                  (0x00000010u)
        #define INT_SCTXUND_MASK                             (0x00000010u)
        #define INT_SCTXUND_BIT                              (4)
        #define INT_SCTXUND_BITS                             (1)
        /* INT_SCRXOVF field */
        #define INT_SCRXOVF                                  (0x00000008u)
        #define INT_SCRXOVF_MASK                             (0x00000008u)
        #define INT_SCRXOVF_BIT                              (3)
        #define INT_SCRXOVF_BITS                             (1)
        /* INT_SCTXIDLE field */
        #define INT_SCTXIDLE                                 (0x00000004u)
        #define INT_SCTXIDLE_MASK                            (0x00000004u)
        #define INT_SCTXIDLE_BIT                             (2)
        #define INT_SCTXIDLE_BITS                            (1)
        /* INT_SCTXFREE field */
        #define INT_SCTXFREE                                 (0x00000002u)
        #define INT_SCTXFREE_MASK                            (0x00000002u)
        #define INT_SCTXFREE_BIT                             (1)
        #define INT_SCTXFREE_BITS                            (1)
        /* INT_SCRXVAL field */
        #define INT_SCRXVAL                                  (0x00000001u)
        #define INT_SCRXVAL_MASK                             (0x00000001u)
        #define INT_SCRXVAL_BIT                              (0)
        #define INT_SCRXVAL_BITS                             (1)

#define INT_ADCFLAG                                          *((volatile uint32_t *)0x4000A810u)
#define INT_ADCFLAG_REG                                      *((volatile uint32_t *)0x4000A810u)
#define INT_ADCFLAG_ADDR                                     (0x4000A810u)
#define INT_ADCFLAG_RESET                                    (0x00000000u)
        /* INT_ADCOVF field */
        #define INT_ADCOVF                                   (0x00000010u)
        #define INT_ADCOVF_MASK                              (0x00000010u)
        #define INT_ADCOVF_BIT                               (4)
        #define INT_ADCOVF_BITS                              (1)
        /* INT_ADCSAT field */
        #define INT_ADCSAT                                   (0x00000008u)
        #define INT_ADCSAT_MASK                              (0x00000008u)
        #define INT_ADCSAT_BIT                               (3)
        #define INT_ADCSAT_BITS                              (1)
        /* INT_ADCULDFULL field */
        #define INT_ADCULDFULL                               (0x00000004u)
        #define INT_ADCULDFULL_MASK                          (0x00000004u)
        #define INT_ADCULDFULL_BIT                           (2)
        #define INT_ADCULDFULL_BITS                          (1)
        /* INT_ADCULDHALF field */
        #define INT_ADCULDHALF                               (0x00000002u)
        #define INT_ADCULDHALF_MASK                          (0x00000002u)
        #define INT_ADCULDHALF_BIT                           (1)
        #define INT_ADCULDHALF_BITS                          (1)
        /* INT_ADCFLAGRSVD field */
        #define INT_ADCFLAGRSVD                              (0x00000001u)
        #define INT_ADCFLAGRSVD_MASK                         (0x00000001u)
        #define INT_ADCFLAGRSVD_BIT                          (0)
        #define INT_ADCFLAGRSVD_BITS                         (1)

#define INT_GPIOFLAG                                         *((volatile uint32_t *)0x4000A814u)
#define INT_GPIOFLAG_REG                                     *((volatile uint32_t *)0x4000A814u)
#define INT_GPIOFLAG_ADDR                                    (0x4000A814u)
#define INT_GPIOFLAG_RESET                                   (0x00000000u)
        /* INT_IRQDFLAG field */
        #define INT_IRQDFLAG                                 (0x00000008u)
        #define INT_IRQDFLAG_MASK                            (0x00000008u)
        #define INT_IRQDFLAG_BIT                             (3)
        #define INT_IRQDFLAG_BITS                            (1)
        /* INT_IRQCFLAG field */
        #define INT_IRQCFLAG                                 (0x00000004u)
        #define INT_IRQCFLAG_MASK                            (0x00000004u)
        #define INT_IRQCFLAG_BIT                             (2)
        #define INT_IRQCFLAG_BITS                            (1)
        /* INT_IRQBFLAG field */
        #define INT_IRQBFLAG                                 (0x00000002u)
        #define INT_IRQBFLAG_MASK                            (0x00000002u)
        #define INT_IRQBFLAG_BIT                             (1)
        #define INT_IRQBFLAG_BITS                            (1)
        /* INT_IRQAFLAG field */
        #define INT_IRQAFLAG                                 (0x00000001u)
        #define INT_IRQAFLAG_MASK                            (0x00000001u)
        #define INT_IRQAFLAG_BIT                             (0)
        #define INT_IRQAFLAG_BITS                            (1)

#define INT_TIM1MISS                                         *((volatile uint32_t *)0x4000A818u)
#define INT_TIM1MISS_REG                                     *((volatile uint32_t *)0x4000A818u)
#define INT_TIM1MISS_ADDR                                    (0x4000A818u)
#define INT_TIM1MISS_RESET                                   (0x00000000u)
        /* INT_TIMMISSCC4IF field */
        #define INT_TIMMISSCC4IF                             (0x00001000u)
        #define INT_TIMMISSCC4IF_MASK                        (0x00001000u)
        #define INT_TIMMISSCC4IF_BIT                         (12)
        #define INT_TIMMISSCC4IF_BITS                        (1)
        /* INT_TIMMISSCC3IF field */
        #define INT_TIMMISSCC3IF                             (0x00000800u)
        #define INT_TIMMISSCC3IF_MASK                        (0x00000800u)
        #define INT_TIMMISSCC3IF_BIT                         (11)
        #define INT_TIMMISSCC3IF_BITS                        (1)
        /* INT_TIMMISSCC2IF field */
        #define INT_TIMMISSCC2IF                             (0x00000400u)
        #define INT_TIMMISSCC2IF_MASK                        (0x00000400u)
        #define INT_TIMMISSCC2IF_BIT                         (10)
        #define INT_TIMMISSCC2IF_BITS                        (1)
        /* INT_TIMMISSCC1IF field */
        #define INT_TIMMISSCC1IF                             (0x00000200u)
        #define INT_TIMMISSCC1IF_MASK                        (0x00000200u)
        #define INT_TIMMISSCC1IF_BIT                         (9)
        #define INT_TIMMISSCC1IF_BITS                        (1)
        /* INT_TIMMISSRSVD field */
        #define INT_TIMMISSRSVD                              (0x0000007Fu)
        #define INT_TIMMISSRSVD_MASK                         (0x0000007Fu)
        #define INT_TIMMISSRSVD_BIT                          (0)
        #define INT_TIMMISSRSVD_BITS                         (7)

#define INT_TIM2MISS                                         *((volatile uint32_t *)0x4000A81Cu)
#define INT_TIM2MISS_REG                                     *((volatile uint32_t *)0x4000A81Cu)
#define INT_TIM2MISS_ADDR                                    (0x4000A81Cu)
#define INT_TIM2MISS_RESET                                   (0x00000000u)
        /* INT_TIMMISSCC4IF field */
        #define INT_TIMMISSCC4IF                             (0x00001000u)
        #define INT_TIMMISSCC4IF_MASK                        (0x00001000u)
        #define INT_TIMMISSCC4IF_BIT                         (12)
        #define INT_TIMMISSCC4IF_BITS                        (1)
        /* INT_TIMMISSCC3IF field */
        #define INT_TIMMISSCC3IF                             (0x00000800u)
        #define INT_TIMMISSCC3IF_MASK                        (0x00000800u)
        #define INT_TIMMISSCC3IF_BIT                         (11)
        #define INT_TIMMISSCC3IF_BITS                        (1)
        /* INT_TIMMISSCC2IF field */
        #define INT_TIMMISSCC2IF                             (0x00000400u)
        #define INT_TIMMISSCC2IF_MASK                        (0x00000400u)
        #define INT_TIMMISSCC2IF_BIT                         (10)
        #define INT_TIMMISSCC2IF_BITS                        (1)
        /* INT_TIMMISSCC1IF field */
        #define INT_TIMMISSCC1IF                             (0x00000200u)
        #define INT_TIMMISSCC1IF_MASK                        (0x00000200u)
        #define INT_TIMMISSCC1IF_BIT                         (9)
        #define INT_TIMMISSCC1IF_BITS                        (1)
        /* INT_TIMMISSRSVD field */
        #define INT_TIMMISSRSVD                              (0x0000007Fu)
        #define INT_TIMMISSRSVD_MASK                         (0x0000007Fu)
        #define INT_TIMMISSRSVD_BIT                          (0)
        #define INT_TIMMISSRSVD_BITS                         (7)

#define INT_MISS                                             *((volatile uint32_t *)0x4000A820u)
#define INT_MISS_REG                                         *((volatile uint32_t *)0x4000A820u)
#define INT_MISS_ADDR                                        (0x4000A820u)
#define INT_MISS_RESET                                       (0x00000000u)
        /* INT_MISSIRQD field */
        #define INT_MISSIRQD                                 (0x00008000u)
        #define INT_MISSIRQD_MASK                            (0x00008000u)
        #define INT_MISSIRQD_BIT                             (15)
        #define INT_MISSIRQD_BITS                            (1)
        /* INT_MISSIRQC field */
        #define INT_MISSIRQC                                 (0x00004000u)
        #define INT_MISSIRQC_MASK                            (0x00004000u)
        #define INT_MISSIRQC_BIT                             (14)
        #define INT_MISSIRQC_BITS                            (1)
        /* INT_MISSIRQB field */
        #define INT_MISSIRQB                                 (0x00002000u)
        #define INT_MISSIRQB_MASK                            (0x00002000u)
        #define INT_MISSIRQB_BIT                             (13)
        #define INT_MISSIRQB_BITS                            (1)
        /* INT_MISSIRQA field */
        #define INT_MISSIRQA                                 (0x00001000u)
        #define INT_MISSIRQA_MASK                            (0x00001000u)
        #define INT_MISSIRQA_BIT                             (12)
        #define INT_MISSIRQA_BITS                            (1)
        /* INT_MISSADC field */
        #define INT_MISSADC                                  (0x00000800u)
        #define INT_MISSADC_MASK                             (0x00000800u)
        #define INT_MISSADC_BIT                              (11)
        #define INT_MISSADC_BITS                             (1)
        /* INT_MISSMACRX field */
        #define INT_MISSMACRX                                (0x00000400u)
        #define INT_MISSMACRX_MASK                           (0x00000400u)
        #define INT_MISSMACRX_BIT                            (10)
        #define INT_MISSMACRX_BITS                           (1)
        /* INT_MISSMACTX field */
        #define INT_MISSMACTX                                (0x00000200u)
        #define INT_MISSMACTX_MASK                           (0x00000200u)
        #define INT_MISSMACTX_BIT                            (9)
        #define INT_MISSMACTX_BITS                           (1)
        /* INT_MISSMACTMR field */
        #define INT_MISSMACTMR                               (0x00000100u)
        #define INT_MISSMACTMR_MASK                          (0x00000100u)
        #define INT_MISSMACTMR_BIT                           (8)
        #define INT_MISSMACTMR_BITS                          (1)
        /* INT_MISSSEC field */
        #define INT_MISSSEC                                  (0x00000080u)
        #define INT_MISSSEC_MASK                             (0x00000080u)
        #define INT_MISSSEC_BIT                              (7)
        #define INT_MISSSEC_BITS                             (1)
        /* INT_MISSSC2 field */
        #define INT_MISSSC2                                  (0x00000040u)
        #define INT_MISSSC2_MASK                             (0x00000040u)
        #define INT_MISSSC2_BIT                              (6)
        #define INT_MISSSC2_BITS                             (1)
        /* INT_MISSSC1 field */
        #define INT_MISSSC1                                  (0x00000020u)
        #define INT_MISSSC1_MASK                             (0x00000020u)
        #define INT_MISSSC1_BIT                              (5)
        #define INT_MISSSC1_BITS                             (1)
        /* INT_MISSSLEEP field */
        #define INT_MISSSLEEP                                (0x00000010u)
        #define INT_MISSSLEEP_MASK                           (0x00000010u)
        #define INT_MISSSLEEP_BIT                            (4)
        #define INT_MISSSLEEP_BITS                           (1)
        /* INT_MISSBB field */
        #define INT_MISSBB                                   (0x00000008u)
        #define INT_MISSBB_MASK                              (0x00000008u)
        #define INT_MISSBB_BIT                               (3)
        #define INT_MISSBB_BITS                              (1)
        /* INT_MISSMGMT field */
        #define INT_MISSMGMT                                 (0x00000004u)
        #define INT_MISSMGMT_MASK                            (0x00000004u)
        #define INT_MISSMGMT_BIT                             (2)
        #define INT_MISSMGMT_BITS                            (1)

#define INT_TIM1CFG                                          *((volatile uint32_t *)0x4000A840u)
#define INT_TIM1CFG_REG                                      *((volatile uint32_t *)0x4000A840u)
#define INT_TIM1CFG_ADDR                                     (0x4000A840u)
#define INT_TIM1CFG_RESET                                    (0x00000000u)
        /* INT_TIMTIF field */
        #define INT_TIMTIF                                   (0x00000040u)
        #define INT_TIMTIF_MASK                              (0x00000040u)
        #define INT_TIMTIF_BIT                               (6)
        #define INT_TIMTIF_BITS                              (1)
        /* INT_TIMCC4IF field */
        #define INT_TIMCC4IF                                 (0x00000010u)
        #define INT_TIMCC4IF_MASK                            (0x00000010u)
        #define INT_TIMCC4IF_BIT                             (4)
        #define INT_TIMCC4IF_BITS                            (1)
        /* INT_TIMCC3IF field */
        #define INT_TIMCC3IF                                 (0x00000008u)
        #define INT_TIMCC3IF_MASK                            (0x00000008u)
        #define INT_TIMCC3IF_BIT                             (3)
        #define INT_TIMCC3IF_BITS                            (1)
        /* INT_TIMCC2IF field */
        #define INT_TIMCC2IF                                 (0x00000004u)
        #define INT_TIMCC2IF_MASK                            (0x00000004u)
        #define INT_TIMCC2IF_BIT                             (2)
        #define INT_TIMCC2IF_BITS                            (1)
        /* INT_TIMCC1IF field */
        #define INT_TIMCC1IF                                 (0x00000002u)
        #define INT_TIMCC1IF_MASK                            (0x00000002u)
        #define INT_TIMCC1IF_BIT                             (1)
        #define INT_TIMCC1IF_BITS                            (1)
        /* INT_TIMUIF field */
        #define INT_TIMUIF                                   (0x00000001u)
        #define INT_TIMUIF_MASK                              (0x00000001u)
        #define INT_TIMUIF_BIT                               (0)
        #define INT_TIMUIF_BITS                              (1)

#define INT_TIM2CFG                                          *((volatile uint32_t *)0x4000A844u)
#define INT_TIM2CFG_REG                                      *((volatile uint32_t *)0x4000A844u)
#define INT_TIM2CFG_ADDR                                     (0x4000A844u)
#define INT_TIM2CFG_RESET                                    (0x00000000u)
        /* INT_TIMTIF field */
        #define INT_TIMTIF                                   (0x00000040u)
        #define INT_TIMTIF_MASK                              (0x00000040u)
        #define INT_TIMTIF_BIT                               (6)
        #define INT_TIMTIF_BITS                              (1)
        /* INT_TIMCC4IF field */
        #define INT_TIMCC4IF                                 (0x00000010u)
        #define INT_TIMCC4IF_MASK                            (0x00000010u)
        #define INT_TIMCC4IF_BIT                             (4)
        #define INT_TIMCC4IF_BITS                            (1)
        /* INT_TIMCC3IF field */
        #define INT_TIMCC3IF                                 (0x00000008u)
        #define INT_TIMCC3IF_MASK                            (0x00000008u)
        #define INT_TIMCC3IF_BIT                             (3)
        #define INT_TIMCC3IF_BITS                            (1)
        /* INT_TIMCC2IF field */
        #define INT_TIMCC2IF                                 (0x00000004u)
        #define INT_TIMCC2IF_MASK                            (0x00000004u)
        #define INT_TIMCC2IF_BIT                             (2)
        #define INT_TIMCC2IF_BITS                            (1)
        /* INT_TIMCC1IF field */
        #define INT_TIMCC1IF                                 (0x00000002u)
        #define INT_TIMCC1IF_MASK                            (0x00000002u)
        #define INT_TIMCC1IF_BIT                             (1)
        #define INT_TIMCC1IF_BITS                            (1)
        /* INT_TIMUIF field */
        #define INT_TIMUIF                                   (0x00000001u)
        #define INT_TIMUIF_MASK                              (0x00000001u)
        #define INT_TIMUIF_BIT                               (0)
        #define INT_TIMUIF_BITS                              (1)

#define INT_SC1CFG                                           *((volatile uint32_t *)0x4000A848u)
#define INT_SC1CFG_REG                                       *((volatile uint32_t *)0x4000A848u)
#define INT_SC1CFG_ADDR                                      (0x4000A848u)
#define INT_SC1CFG_RESET                                     (0x00000000u)
        /* INT_SC1PARERR field */
        #define INT_SC1PARERR                                (0x00004000u)
        #define INT_SC1PARERR_MASK                           (0x00004000u)
        #define INT_SC1PARERR_BIT                            (14)
        #define INT_SC1PARERR_BITS                           (1)
        /* INT_SC1FRMERR field */
        #define INT_SC1FRMERR                                (0x00002000u)
        #define INT_SC1FRMERR_MASK                           (0x00002000u)
        #define INT_SC1FRMERR_BIT                            (13)
        #define INT_SC1FRMERR_BITS                           (1)
        /* INT_SCTXULDB field */
        #define INT_SCTXULDB                                 (0x00001000u)
        #define INT_SCTXULDB_MASK                            (0x00001000u)
        #define INT_SCTXULDB_BIT                             (12)
        #define INT_SCTXULDB_BITS                            (1)
        /* INT_SCTXULDA field */
        #define INT_SCTXULDA                                 (0x00000800u)
        #define INT_SCTXULDA_MASK                            (0x00000800u)
        #define INT_SCTXULDA_BIT                             (11)
        #define INT_SCTXULDA_BITS                            (1)
        /* INT_SCRXULDB field */
        #define INT_SCRXULDB                                 (0x00000400u)
        #define INT_SCRXULDB_MASK                            (0x00000400u)
        #define INT_SCRXULDB_BIT                             (10)
        #define INT_SCRXULDB_BITS                            (1)
        /* INT_SCRXULDA field */
        #define INT_SCRXULDA                                 (0x00000200u)
        #define INT_SCRXULDA_MASK                            (0x00000200u)
        #define INT_SCRXULDA_BIT                             (9)
        #define INT_SCRXULDA_BITS                            (1)
        /* INT_SCNAK field */
        #define INT_SCNAK                                    (0x00000100u)
        #define INT_SCNAK_MASK                               (0x00000100u)
        #define INT_SCNAK_BIT                                (8)
        #define INT_SCNAK_BITS                               (1)
        /* INT_SCCMDFIN field */
        #define INT_SCCMDFIN                                 (0x00000080u)
        #define INT_SCCMDFIN_MASK                            (0x00000080u)
        #define INT_SCCMDFIN_BIT                             (7)
        #define INT_SCCMDFIN_BITS                            (1)
        /* INT_SCTXFIN field */
        #define INT_SCTXFIN                                  (0x00000040u)
        #define INT_SCTXFIN_MASK                             (0x00000040u)
        #define INT_SCTXFIN_BIT                              (6)
        #define INT_SCTXFIN_BITS                             (1)
        /* INT_SCRXFIN field */
        #define INT_SCRXFIN                                  (0x00000020u)
        #define INT_SCRXFIN_MASK                             (0x00000020u)
        #define INT_SCRXFIN_BIT                              (5)
        #define INT_SCRXFIN_BITS                             (1)
        /* INT_SCTXUND field */
        #define INT_SCTXUND                                  (0x00000010u)
        #define INT_SCTXUND_MASK                             (0x00000010u)
        #define INT_SCTXUND_BIT                              (4)
        #define INT_SCTXUND_BITS                             (1)
        /* INT_SCRXOVF field */
        #define INT_SCRXOVF                                  (0x00000008u)
        #define INT_SCRXOVF_MASK                             (0x00000008u)
        #define INT_SCRXOVF_BIT                              (3)
        #define INT_SCRXOVF_BITS                             (1)
        /* INT_SCTXIDLE field */
        #define INT_SCTXIDLE                                 (0x00000004u)
        #define INT_SCTXIDLE_MASK                            (0x00000004u)
        #define INT_SCTXIDLE_BIT                             (2)
        #define INT_SCTXIDLE_BITS                            (1)
        /* INT_SCTXFREE field */
        #define INT_SCTXFREE                                 (0x00000002u)
        #define INT_SCTXFREE_MASK                            (0x00000002u)
        #define INT_SCTXFREE_BIT                             (1)
        #define INT_SCTXFREE_BITS                            (1)
        /* INT_SCRXVAL field */
        #define INT_SCRXVAL                                  (0x00000001u)
        #define INT_SCRXVAL_MASK                             (0x00000001u)
        #define INT_SCRXVAL_BIT                              (0)
        #define INT_SCRXVAL_BITS                             (1)

#define INT_SC2CFG                                           *((volatile uint32_t *)0x4000A84Cu)
#define INT_SC2CFG_REG                                       *((volatile uint32_t *)0x4000A84Cu)
#define INT_SC2CFG_ADDR                                      (0x4000A84Cu)
#define INT_SC2CFG_RESET                                     (0x00000000u)
        /* INT_SCTXULDB field */
        #define INT_SCTXULDB                                 (0x00001000u)
        #define INT_SCTXULDB_MASK                            (0x00001000u)
        #define INT_SCTXULDB_BIT                             (12)
        #define INT_SCTXULDB_BITS                            (1)
        /* INT_SCTXULDA field */
        #define INT_SCTXULDA                                 (0x00000800u)
        #define INT_SCTXULDA_MASK                            (0x00000800u)
        #define INT_SCTXULDA_BIT                             (11)
        #define INT_SCTXULDA_BITS                            (1)
        /* INT_SCRXULDB field */
        #define INT_SCRXULDB                                 (0x00000400u)
        #define INT_SCRXULDB_MASK                            (0x00000400u)
        #define INT_SCRXULDB_BIT                             (10)
        #define INT_SCRXULDB_BITS                            (1)
        /* INT_SCRXULDA field */
        #define INT_SCRXULDA                                 (0x00000200u)
        #define INT_SCRXULDA_MASK                            (0x00000200u)
        #define INT_SCRXULDA_BIT                             (9)
        #define INT_SCRXULDA_BITS                            (1)
        /* INT_SCNAK field */
        #define INT_SCNAK                                    (0x00000100u)
        #define INT_SCNAK_MASK                               (0x00000100u)
        #define INT_SCNAK_BIT                                (8)
        #define INT_SCNAK_BITS                               (1)
        /* INT_SCCMDFIN field */
        #define INT_SCCMDFIN                                 (0x00000080u)
        #define INT_SCCMDFIN_MASK                            (0x00000080u)
        #define INT_SCCMDFIN_BIT                             (7)
        #define INT_SCCMDFIN_BITS                            (1)
        /* INT_SCTXFIN field */
        #define INT_SCTXFIN                                  (0x00000040u)
        #define INT_SCTXFIN_MASK                             (0x00000040u)
        #define INT_SCTXFIN_BIT                              (6)
        #define INT_SCTXFIN_BITS                             (1)
        /* INT_SCRXFIN field */
        #define INT_SCRXFIN                                  (0x00000020u)
        #define INT_SCRXFIN_MASK                             (0x00000020u)
        #define INT_SCRXFIN_BIT                              (5)
        #define INT_SCRXFIN_BITS                             (1)
        /* INT_SCTXUND field */
        #define INT_SCTXUND                                  (0x00000010u)
        #define INT_SCTXUND_MASK                             (0x00000010u)
        #define INT_SCTXUND_BIT                              (4)
        #define INT_SCTXUND_BITS                             (1)
        /* INT_SCRXOVF field */
        #define INT_SCRXOVF                                  (0x00000008u)
        #define INT_SCRXOVF_MASK                             (0x00000008u)
        #define INT_SCRXOVF_BIT                              (3)
        #define INT_SCRXOVF_BITS                             (1)
        /* INT_SCTXIDLE field */
        #define INT_SCTXIDLE                                 (0x00000004u)
        #define INT_SCTXIDLE_MASK                            (0x00000004u)
        #define INT_SCTXIDLE_BIT                             (2)
        #define INT_SCTXIDLE_BITS                            (1)
        /* INT_SCTXFREE field */
        #define INT_SCTXFREE                                 (0x00000002u)
        #define INT_SCTXFREE_MASK                            (0x00000002u)
        #define INT_SCTXFREE_BIT                             (1)
        #define INT_SCTXFREE_BITS                            (1)
        /* INT_SCRXVAL field */
        #define INT_SCRXVAL                                  (0x00000001u)
        #define INT_SCRXVAL_MASK                             (0x00000001u)
        #define INT_SCRXVAL_BIT                              (0)
        #define INT_SCRXVAL_BITS                             (1)

#define INT_ADCCFG                                           *((volatile uint32_t *)0x4000A850u)
#define INT_ADCCFG_REG                                       *((volatile uint32_t *)0x4000A850u)
#define INT_ADCCFG_ADDR                                      (0x4000A850u)
#define INT_ADCCFG_RESET                                     (0x00000000u)
        /* INT_ADCOVF field */
        #define INT_ADCOVF                                   (0x00000010u)
        #define INT_ADCOVF_MASK                              (0x00000010u)
        #define INT_ADCOVF_BIT                               (4)
        #define INT_ADCOVF_BITS                              (1)
        /* INT_ADCSAT field */
        #define INT_ADCSAT                                   (0x00000008u)
        #define INT_ADCSAT_MASK                              (0x00000008u)
        #define INT_ADCSAT_BIT                               (3)
        #define INT_ADCSAT_BITS                              (1)
        /* INT_ADCULDFULL field */
        #define INT_ADCULDFULL                               (0x00000004u)
        #define INT_ADCULDFULL_MASK                          (0x00000004u)
        #define INT_ADCULDFULL_BIT                           (2)
        #define INT_ADCULDFULL_BITS                          (1)
        /* INT_ADCULDHALF field */
        #define INT_ADCULDHALF                               (0x00000002u)
        #define INT_ADCULDHALF_MASK                          (0x00000002u)
        #define INT_ADCULDHALF_BIT                           (1)
        #define INT_ADCULDHALF_BITS                          (1)
        /* INT_ADCCFGRSVD field */
        #define INT_ADCCFGRSVD                               (0x00000001u)
        #define INT_ADCCFGRSVD_MASK                          (0x00000001u)
        #define INT_ADCCFGRSVD_BIT                           (0)
        #define INT_ADCCFGRSVD_BITS                          (1)

#define SC1_INTMODE                                          *((volatile uint32_t *)0x4000A854u)
#define SC1_INTMODE_REG                                      *((volatile uint32_t *)0x4000A854u)
#define SC1_INTMODE_ADDR                                     (0x4000A854u)
#define SC1_INTMODE_RESET                                    (0x00000000u)
        /* SC_TXIDLELEVEL field */
        #define SC_TXIDLELEVEL                               (0x00000004u)
        #define SC_TXIDLELEVEL_MASK                          (0x00000004u)
        #define SC_TXIDLELEVEL_BIT                           (2)
        #define SC_TXIDLELEVEL_BITS                          (1)
        /* SC_TXFREELEVEL field */
        #define SC_TXFREELEVEL                               (0x00000002u)
        #define SC_TXFREELEVEL_MASK                          (0x00000002u)
        #define SC_TXFREELEVEL_BIT                           (1)
        #define SC_TXFREELEVEL_BITS                          (1)
        /* SC_RXVALLEVEL field */
        #define SC_RXVALLEVEL                                (0x00000001u)
        #define SC_RXVALLEVEL_MASK                           (0x00000001u)
        #define SC_RXVALLEVEL_BIT                            (0)
        #define SC_RXVALLEVEL_BITS                           (1)

#define SC2_INTMODE                                          *((volatile uint32_t *)0x4000A858u)
#define SC2_INTMODE_REG                                      *((volatile uint32_t *)0x4000A858u)
#define SC2_INTMODE_ADDR                                     (0x4000A858u)
#define SC2_INTMODE_RESET                                    (0x00000000u)
        /* SC_TXIDLELEVEL field */
        #define SC_TXIDLELEVEL                               (0x00000004u)
        #define SC_TXIDLELEVEL_MASK                          (0x00000004u)
        #define SC_TXIDLELEVEL_BIT                           (2)
        #define SC_TXIDLELEVEL_BITS                          (1)
        /* SC_TXFREELEVEL field */
        #define SC_TXFREELEVEL                               (0x00000002u)
        #define SC_TXFREELEVEL_MASK                          (0x00000002u)
        #define SC_TXFREELEVEL_BIT                           (1)
        #define SC_TXFREELEVEL_BITS                          (1)
        /* SC_RXVALLEVEL field */
        #define SC_RXVALLEVEL                                (0x00000001u)
        #define SC_RXVALLEVEL_MASK                           (0x00000001u)
        #define SC_RXVALLEVEL_BIT                            (0)
        #define SC_RXVALLEVEL_BITS                           (1)

#define GPIO_INTCFGA                                         *((volatile uint32_t *)0x4000A860u)
#define GPIO_INTCFGA_REG                                     *((volatile uint32_t *)0x4000A860u)
#define GPIO_INTCFGA_ADDR                                    (0x4000A860u)
#define GPIO_INTCFGA_RESET                                   (0x00000000u)
        /* GPIO_INTFILT field */
        #define GPIO_INTFILT                                 (0x00000100u)
        #define GPIO_INTFILT_MASK                            (0x00000100u)
        #define GPIO_INTFILT_BIT                             (8)
        #define GPIO_INTFILT_BITS                            (1)
        /* GPIO_INTMOD field */
        #define GPIO_INTMOD                                  (0x000000E0u)
        #define GPIO_INTMOD_MASK                             (0x000000E0u)
        #define GPIO_INTMOD_BIT                              (5)
        #define GPIO_INTMOD_BITS                             (3)

#define GPIO_INTCFGB                                         *((volatile uint32_t *)0x4000A864u)
#define GPIO_INTCFGB_REG                                     *((volatile uint32_t *)0x4000A864u)
#define GPIO_INTCFGB_ADDR                                    (0x4000A864u)
#define GPIO_INTCFGB_RESET                                   (0x00000000u)
        /* GPIO_INTFILT field */
        #define GPIO_INTFILT                                 (0x00000100u)
        #define GPIO_INTFILT_MASK                            (0x00000100u)
        #define GPIO_INTFILT_BIT                             (8)
        #define GPIO_INTFILT_BITS                            (1)
        /* GPIO_INTMOD field */
        #define GPIO_INTMOD                                  (0x000000E0u)
        #define GPIO_INTMOD_MASK                             (0x000000E0u)
        #define GPIO_INTMOD_BIT                              (5)
        #define GPIO_INTMOD_BITS                             (3)

#define GPIO_INTCFGC                                         *((volatile uint32_t *)0x4000A868u)
#define GPIO_INTCFGC_REG                                     *((volatile uint32_t *)0x4000A868u)
#define GPIO_INTCFGC_ADDR                                    (0x4000A868u)
#define GPIO_INTCFGC_RESET                                   (0x00000000u)
        /* GPIO_INTFILT field */
        #define GPIO_INTFILT                                 (0x00000100u)
        #define GPIO_INTFILT_MASK                            (0x00000100u)
        #define GPIO_INTFILT_BIT                             (8)
        #define GPIO_INTFILT_BITS                            (1)
        /* GPIO_INTMOD field */
        #define GPIO_INTMOD                                  (0x000000E0u)
        #define GPIO_INTMOD_MASK                             (0x000000E0u)
        #define GPIO_INTMOD_BIT                              (5)
        #define GPIO_INTMOD_BITS                             (3)

#define GPIO_INTCFGD                                         *((volatile uint32_t *)0x4000A86Cu)
#define GPIO_INTCFGD_REG                                     *((volatile uint32_t *)0x4000A86Cu)
#define GPIO_INTCFGD_ADDR                                    (0x4000A86Cu)
#define GPIO_INTCFGD_RESET                                   (0x00000000u)
        /* GPIO_INTFILT field */
        #define GPIO_INTFILT                                 (0x00000100u)
        #define GPIO_INTFILT_MASK                            (0x00000100u)
        #define GPIO_INTFILT_BIT                             (8)
        #define GPIO_INTFILT_BITS                            (1)
        /* GPIO_INTMOD field */
        #define GPIO_INTMOD                                  (0x000000E0u)
        #define GPIO_INTMOD_MASK                             (0x000000E0u)
        #define GPIO_INTMOD_BIT                              (5)
        #define GPIO_INTMOD_BITS                             (3)

/* GPIO block */
#define BLOCK_GPIO_BASE                                      (0x4000B000u)
#define BLOCK_GPIO_END                                       (0x4000BC1Cu)
#define BLOCK_GPIO_SIZE                                      (BLOCK_GPIO_END - BLOCK_GPIO_BASE + 1)

#define GPIO_PACFGL                                          *((volatile uint32_t *)0x4000B000u)
#define GPIO_PACFGL_REG                                      *((volatile uint32_t *)0x4000B000u)
#define GPIO_PACFGL_ADDR                                     (0x4000B000u)
#define GPIO_PACFGL_RESET                                    (0x00004444u)
        /* PA3_CFG field */
        #define PA3_CFG                                      (0x0000F000u)
        #define PA3_CFG_MASK                                 (0x0000F000u)
        #define PA3_CFG_BIT                                  (12)
        #define PA3_CFG_BITS                                 (4)
        /* PA2_CFG field */
        #define PA2_CFG                                      (0x00000F00u)
        #define PA2_CFG_MASK                                 (0x00000F00u)
        #define PA2_CFG_BIT                                  (8)
        #define PA2_CFG_BITS                                 (4)
        /* PA1_CFG field */
        #define PA1_CFG                                      (0x000000F0u)
        #define PA1_CFG_MASK                                 (0x000000F0u)
        #define PA1_CFG_BIT                                  (4)
        #define PA1_CFG_BITS                                 (4)
        /* PA0_CFG field */
        #define PA0_CFG                                      (0x0000000Fu)
        #define PA0_CFG_MASK                                 (0x0000000Fu)
        #define PA0_CFG_BIT                                  (0)
        #define PA0_CFG_BITS                                 (4)
                /* GPIO_PxCFGx Bit Field Values */
                #define GPIOCFG_OUT                          (0x1u)
                #define GPIOCFG_OUT_OD                       (0x5u)
                #define GPIOCFG_OUT_ALT                      (0x9u)
                #define GPIOCFG_OUT_ALT_OD                   (0xDu)
                #define GPIOCFG_ANALOG                       (0x0u)
                #define GPIOCFG_IN                           (0x4u)
                #define GPIOCFG_IN_PUD                       (0x8u)

#define GPIO_PACFGH                                          *((volatile uint32_t *)0x4000B004u)
#define GPIO_PACFGH_REG                                      *((volatile uint32_t *)0x4000B004u)
#define GPIO_PACFGH_ADDR                                     (0x4000B004u)
#define GPIO_PACFGH_RESET                                    (0x00004444u)
        /* PA7_CFG field */
        #define PA7_CFG                                      (0x0000F000u)
        #define PA7_CFG_MASK                                 (0x0000F000u)
        #define PA7_CFG_BIT                                  (12)
        #define PA7_CFG_BITS                                 (4)
        /* PA6_CFG field */
        #define PA6_CFG                                      (0x00000F00u)
        #define PA6_CFG_MASK                                 (0x00000F00u)
        #define PA6_CFG_BIT                                  (8)
        #define PA6_CFG_BITS                                 (4)
        /* PA5_CFG field */
        #define PA5_CFG                                      (0x000000F0u)
        #define PA5_CFG_MASK                                 (0x000000F0u)
        #define PA5_CFG_BIT                                  (4)
        #define PA5_CFG_BITS                                 (4)
        /* PA4_CFG field */
        #define PA4_CFG                                      (0x0000000Fu)
        #define PA4_CFG_MASK                                 (0x0000000Fu)
        #define PA4_CFG_BIT                                  (0)
        #define PA4_CFG_BITS                                 (4)

#define GPIO_PAIN                                            *((volatile uint32_t *)0x4000B008u)
#define GPIO_PAIN_REG                                        *((volatile uint32_t *)0x4000B008u)
#define GPIO_PAIN_ADDR                                       (0x4000B008u)
#define GPIO_PAIN_RESET                                      (0x00000000u)
        /* PA7 field */
        #define PA7                                          (0x00000080u)
        #define PA7_MASK                                     (0x00000080u)
        #define PA7_BIT                                      (7)
        #define PA7_BITS                                     (1)
        /* PA6 field */
        #define PA6                                          (0x00000040u)
        #define PA6_MASK                                     (0x00000040u)
        #define PA6_BIT                                      (6)
        #define PA6_BITS                                     (1)
        /* PA5 field */
        #define PA5                                          (0x00000020u)
        #define PA5_MASK                                     (0x00000020u)
        #define PA5_BIT                                      (5)
        #define PA5_BITS                                     (1)
        /* PA4 field */
        #define PA4                                          (0x00000010u)
        #define PA4_MASK                                     (0x00000010u)
        #define PA4_BIT                                      (4)
        #define PA4_BITS                                     (1)
        /* PA3 field */
        #define PA3                                          (0x00000008u)
        #define PA3_MASK                                     (0x00000008u)
        #define PA3_BIT                                      (3)
        #define PA3_BITS                                     (1)
        /* PA2 field */
        #define PA2                                          (0x00000004u)
        #define PA2_MASK                                     (0x00000004u)
        #define PA2_BIT                                      (2)
        #define PA2_BITS                                     (1)
        /* PA1 field */
        #define PA1                                          (0x00000002u)
        #define PA1_MASK                                     (0x00000002u)
        #define PA1_BIT                                      (1)
        #define PA1_BITS                                     (1)
        /* PA0 field */
        #define PA0                                          (0x00000001u)
        #define PA0_MASK                                     (0x00000001u)
        #define PA0_BIT                                      (0)
        #define PA0_BITS                                     (1)

#define GPIO_PAOUT                                           *((volatile uint32_t *)0x4000B00Cu)
#define GPIO_PAOUT_REG                                       *((volatile uint32_t *)0x4000B00Cu)
#define GPIO_PAOUT_ADDR                                      (0x4000B00Cu)
#define GPIO_PAOUT_RESET                                     (0x00000000u)
        /* PA7 field */
        #define PA7                                          (0x00000080u)
        #define PA7_MASK                                     (0x00000080u)
        #define PA7_BIT                                      (7)
        #define PA7_BITS                                     (1)
        /* PA6 field */
        #define PA6                                          (0x00000040u)
        #define PA6_MASK                                     (0x00000040u)
        #define PA6_BIT                                      (6)
        #define PA6_BITS                                     (1)
        /* PA5 field */
        #define PA5                                          (0x00000020u)
        #define PA5_MASK                                     (0x00000020u)
        #define PA5_BIT                                      (5)
        #define PA5_BITS                                     (1)
        /* PA4 field */
        #define PA4                                          (0x00000010u)
        #define PA4_MASK                                     (0x00000010u)
        #define PA4_BIT                                      (4)
        #define PA4_BITS                                     (1)
        /* PA3 field */
        #define PA3                                          (0x00000008u)
        #define PA3_MASK                                     (0x00000008u)
        #define PA3_BIT                                      (3)
        #define PA3_BITS                                     (1)
        /* PA2 field */
        #define PA2                                          (0x00000004u)
        #define PA2_MASK                                     (0x00000004u)
        #define PA2_BIT                                      (2)
        #define PA2_BITS                                     (1)
        /* PA1 field */
        #define PA1                                          (0x00000002u)
        #define PA1_MASK                                     (0x00000002u)
        #define PA1_BIT                                      (1)
        #define PA1_BITS                                     (1)
        /* PA0 field */
        #define PA0                                          (0x00000001u)
        #define PA0_MASK                                     (0x00000001u)
        #define PA0_BIT                                      (0)
        #define PA0_BITS                                     (1)
                /* GPIO_PxOUT Bit Field Values */
                #define GPIOOUT_PULLUP                       (0x1u)
                #define GPIOOUT_PULLDOWN                     (0x0u)

#define GPIO_PASET                                           *((volatile uint32_t *)0x4000B010u)
#define GPIO_PASET_REG                                       *((volatile uint32_t *)0x4000B010u)
#define GPIO_PASET_ADDR                                      (0x4000B010u)
#define GPIO_PASET_RESET                                     (0x00000000u)
        /* GPIO_PXSETRSVD field */
        #define GPIO_PXSETRSVD                               (0x0000FF00u)
        #define GPIO_PXSETRSVD_MASK                          (0x0000FF00u)
        #define GPIO_PXSETRSVD_BIT                           (8)
        #define GPIO_PXSETRSVD_BITS                          (8)
        /* PA7 field */
        #define PA7                                          (0x00000080u)
        #define PA7_MASK                                     (0x00000080u)
        #define PA7_BIT                                      (7)
        #define PA7_BITS                                     (1)
        /* PA6 field */
        #define PA6                                          (0x00000040u)
        #define PA6_MASK                                     (0x00000040u)
        #define PA6_BIT                                      (6)
        #define PA6_BITS                                     (1)
        /* PA5 field */
        #define PA5                                          (0x00000020u)
        #define PA5_MASK                                     (0x00000020u)
        #define PA5_BIT                                      (5)
        #define PA5_BITS                                     (1)
        /* PA4 field */
        #define PA4                                          (0x00000010u)
        #define PA4_MASK                                     (0x00000010u)
        #define PA4_BIT                                      (4)
        #define PA4_BITS                                     (1)
        /* PA3 field */
        #define PA3                                          (0x00000008u)
        #define PA3_MASK                                     (0x00000008u)
        #define PA3_BIT                                      (3)
        #define PA3_BITS                                     (1)
        /* PA2 field */
        #define PA2                                          (0x00000004u)
        #define PA2_MASK                                     (0x00000004u)
        #define PA2_BIT                                      (2)
        #define PA2_BITS                                     (1)
        /* PA1 field */
        #define PA1                                          (0x00000002u)
        #define PA1_MASK                                     (0x00000002u)
        #define PA1_BIT                                      (1)
        #define PA1_BITS                                     (1)
        /* PA0 field */
        #define PA0                                          (0x00000001u)
        #define PA0_MASK                                     (0x00000001u)
        #define PA0_BIT                                      (0)
        #define PA0_BITS                                     (1)

#define GPIO_PACLR                                           *((volatile uint32_t *)0x4000B014u)
#define GPIO_PACLR_REG                                       *((volatile uint32_t *)0x4000B014u)
#define GPIO_PACLR_ADDR                                      (0x4000B014u)
#define GPIO_PACLR_RESET                                     (0x00000000u)
        /* PA7 field */
        #define PA7                                          (0x00000080u)
        #define PA7_MASK                                     (0x00000080u)
        #define PA7_BIT                                      (7)
        #define PA7_BITS                                     (1)
        /* PA6 field */
        #define PA6                                          (0x00000040u)
        #define PA6_MASK                                     (0x00000040u)
        #define PA6_BIT                                      (6)
        #define PA6_BITS                                     (1)
        /* PA5 field */
        #define PA5                                          (0x00000020u)
        #define PA5_MASK                                     (0x00000020u)
        #define PA5_BIT                                      (5)
        #define PA5_BITS                                     (1)
        /* PA4 field */
        #define PA4                                          (0x00000010u)
        #define PA4_MASK                                     (0x00000010u)
        #define PA4_BIT                                      (4)
        #define PA4_BITS                                     (1)
        /* PA3 field */
        #define PA3                                          (0x00000008u)
        #define PA3_MASK                                     (0x00000008u)
        #define PA3_BIT                                      (3)
        #define PA3_BITS                                     (1)
        /* PA2 field */
        #define PA2                                          (0x00000004u)
        #define PA2_MASK                                     (0x00000004u)
        #define PA2_BIT                                      (2)
        #define PA2_BITS                                     (1)
        /* PA1 field */
        #define PA1                                          (0x00000002u)
        #define PA1_MASK                                     (0x00000002u)
        #define PA1_BIT                                      (1)
        #define PA1_BITS                                     (1)
        /* PA0 field */
        #define PA0                                          (0x00000001u)
        #define PA0_MASK                                     (0x00000001u)
        #define PA0_BIT                                      (0)
        #define PA0_BITS                                     (1)

#define GPIO_PBCFGL                                          *((volatile uint32_t *)0x4000B400u)
#define GPIO_PBCFGL_REG                                      *((volatile uint32_t *)0x4000B400u)
#define GPIO_PBCFGL_ADDR                                     (0x4000B400u)
#define GPIO_PBCFGL_RESET                                    (0x00004444u)
        /* PB3_CFG field */
        #define PB3_CFG                                      (0x0000F000u)
        #define PB3_CFG_MASK                                 (0x0000F000u)
        #define PB3_CFG_BIT                                  (12)
        #define PB3_CFG_BITS                                 (4)
        /* PB2_CFG field */
        #define PB2_CFG                                      (0x00000F00u)
        #define PB2_CFG_MASK                                 (0x00000F00u)
        #define PB2_CFG_BIT                                  (8)
        #define PB2_CFG_BITS                                 (4)
        /* PB1_CFG field */
        #define PB1_CFG                                      (0x000000F0u)
        #define PB1_CFG_MASK                                 (0x000000F0u)
        #define PB1_CFG_BIT                                  (4)
        #define PB1_CFG_BITS                                 (4)
        /* PB0_CFG field */
        #define PB0_CFG                                      (0x0000000Fu)
        #define PB0_CFG_MASK                                 (0x0000000Fu)
        #define PB0_CFG_BIT                                  (0)
        #define PB0_CFG_BITS                                 (4)

#define GPIO_PBCFGH                                          *((volatile uint32_t *)0x4000B404u)
#define GPIO_PBCFGH_REG                                      *((volatile uint32_t *)0x4000B404u)
#define GPIO_PBCFGH_ADDR                                     (0x4000B404u)
#define GPIO_PBCFGH_RESET                                    (0x00004444u)
        /* PB7_CFG field */
        #define PB7_CFG                                      (0x0000F000u)
        #define PB7_CFG_MASK                                 (0x0000F000u)
        #define PB7_CFG_BIT                                  (12)
        #define PB7_CFG_BITS                                 (4)
        /* PB6_CFG field */
        #define PB6_CFG                                      (0x00000F00u)
        #define PB6_CFG_MASK                                 (0x00000F00u)
        #define PB6_CFG_BIT                                  (8)
        #define PB6_CFG_BITS                                 (4)
        /* PB5_CFG field */
        #define PB5_CFG                                      (0x000000F0u)
        #define PB5_CFG_MASK                                 (0x000000F0u)
        #define PB5_CFG_BIT                                  (4)
        #define PB5_CFG_BITS                                 (4)
        /* PB4_CFG field */
        #define PB4_CFG                                      (0x0000000Fu)
        #define PB4_CFG_MASK                                 (0x0000000Fu)
        #define PB4_CFG_BIT                                  (0)
        #define PB4_CFG_BITS                                 (4)

#define GPIO_PBIN                                            *((volatile uint32_t *)0x4000B408u)
#define GPIO_PBIN_REG                                        *((volatile uint32_t *)0x4000B408u)
#define GPIO_PBIN_ADDR                                       (0x4000B408u)
#define GPIO_PBIN_RESET                                      (0x00000000u)
        /* PB7 field */
        #define PB7                                          (0x00000080u)
        #define PB7_MASK                                     (0x00000080u)
        #define PB7_BIT                                      (7)
        #define PB7_BITS                                     (1)
        /* PB6 field */
        #define PB6                                          (0x00000040u)
        #define PB6_MASK                                     (0x00000040u)
        #define PB6_BIT                                      (6)
        #define PB6_BITS                                     (1)
        /* PB5 field */
        #define PB5                                          (0x00000020u)
        #define PB5_MASK                                     (0x00000020u)
        #define PB5_BIT                                      (5)
        #define PB5_BITS                                     (1)
        /* PB4 field */
        #define PB4                                          (0x00000010u)
        #define PB4_MASK                                     (0x00000010u)
        #define PB4_BIT                                      (4)
        #define PB4_BITS                                     (1)
        /* PB3 field */
        #define PB3                                          (0x00000008u)
        #define PB3_MASK                                     (0x00000008u)
        #define PB3_BIT                                      (3)
        #define PB3_BITS                                     (1)
        /* PB2 field */
        #define PB2                                          (0x00000004u)
        #define PB2_MASK                                     (0x00000004u)
        #define PB2_BIT                                      (2)
        #define PB2_BITS                                     (1)
        /* PB1 field */
        #define PB1                                          (0x00000002u)
        #define PB1_MASK                                     (0x00000002u)
        #define PB1_BIT                                      (1)
        #define PB1_BITS                                     (1)
        /* PB0 field */
        #define PB0                                          (0x00000001u)
        #define PB0_MASK                                     (0x00000001u)
        #define PB0_BIT                                      (0)
        #define PB0_BITS                                     (1)

#define GPIO_PBOUT                                           *((volatile uint32_t *)0x4000B40Cu)
#define GPIO_PBOUT_REG                                       *((volatile uint32_t *)0x4000B40Cu)
#define GPIO_PBOUT_ADDR                                      (0x4000B40Cu)
#define GPIO_PBOUT_RESET                                     (0x00000000u)
        /* PB7 field */
        #define PB7                                          (0x00000080u)
        #define PB7_MASK                                     (0x00000080u)
        #define PB7_BIT                                      (7)
        #define PB7_BITS                                     (1)
        /* PB6 field */
        #define PB6                                          (0x00000040u)
        #define PB6_MASK                                     (0x00000040u)
        #define PB6_BIT                                      (6)
        #define PB6_BITS                                     (1)
        /* PB5 field */
        #define PB5                                          (0x00000020u)
        #define PB5_MASK                                     (0x00000020u)
        #define PB5_BIT                                      (5)
        #define PB5_BITS                                     (1)
        /* PB4 field */
        #define PB4                                          (0x00000010u)
        #define PB4_MASK                                     (0x00000010u)
        #define PB4_BIT                                      (4)
        #define PB4_BITS                                     (1)
        /* PB3 field */
        #define PB3                                          (0x00000008u)
        #define PB3_MASK                                     (0x00000008u)
        #define PB3_BIT                                      (3)
        #define PB3_BITS                                     (1)
        /* PB2 field */
        #define PB2                                          (0x00000004u)
        #define PB2_MASK                                     (0x00000004u)
        #define PB2_BIT                                      (2)
        #define PB2_BITS                                     (1)
        /* PB1 field */
        #define PB1                                          (0x00000002u)
        #define PB1_MASK                                     (0x00000002u)
        #define PB1_BIT                                      (1)
        #define PB1_BITS                                     (1)
        /* PB0 field */
        #define PB0                                          (0x00000001u)
        #define PB0_MASK                                     (0x00000001u)
        #define PB0_BIT                                      (0)
        #define PB0_BITS                                     (1)

#define GPIO_PBSET                                           *((volatile uint32_t *)0x4000B410u)
#define GPIO_PBSET_REG                                       *((volatile uint32_t *)0x4000B410u)
#define GPIO_PBSET_ADDR                                      (0x4000B410u)
#define GPIO_PBSET_RESET                                     (0x00000000u)
        /* GPIO_PXSETRSVD field */
        #define GPIO_PXSETRSVD                               (0x0000FF00u)
        #define GPIO_PXSETRSVD_MASK                          (0x0000FF00u)
        #define GPIO_PXSETRSVD_BIT                           (8)
        #define GPIO_PXSETRSVD_BITS                          (8)
        /* PB7 field */
        #define PB7                                          (0x00000080u)
        #define PB7_MASK                                     (0x00000080u)
        #define PB7_BIT                                      (7)
        #define PB7_BITS                                     (1)
        /* PB6 field */
        #define PB6                                          (0x00000040u)
        #define PB6_MASK                                     (0x00000040u)
        #define PB6_BIT                                      (6)
        #define PB6_BITS                                     (1)
        /* PB5 field */
        #define PB5                                          (0x00000020u)
        #define PB5_MASK                                     (0x00000020u)
        #define PB5_BIT                                      (5)
        #define PB5_BITS                                     (1)
        /* PB4 field */
        #define PB4                                          (0x00000010u)
        #define PB4_MASK                                     (0x00000010u)
        #define PB4_BIT                                      (4)
        #define PB4_BITS                                     (1)
        /* PB3 field */
        #define PB3                                          (0x00000008u)
        #define PB3_MASK                                     (0x00000008u)
        #define PB3_BIT                                      (3)
        #define PB3_BITS                                     (1)
        /* PB2 field */
        #define PB2                                          (0x00000004u)
        #define PB2_MASK                                     (0x00000004u)
        #define PB2_BIT                                      (2)
        #define PB2_BITS                                     (1)
        /* PB1 field */
        #define PB1                                          (0x00000002u)
        #define PB1_MASK                                     (0x00000002u)
        #define PB1_BIT                                      (1)
        #define PB1_BITS                                     (1)
        /* PB0 field */
        #define PB0                                          (0x00000001u)
        #define PB0_MASK                                     (0x00000001u)
        #define PB0_BIT                                      (0)
        #define PB0_BITS                                     (1)

#define GPIO_PBCLR                                           *((volatile uint32_t *)0x4000B414u)
#define GPIO_PBCLR_REG                                       *((volatile uint32_t *)0x4000B414u)
#define GPIO_PBCLR_ADDR                                      (0x4000B414u)
#define GPIO_PBCLR_RESET                                     (0x00000000u)
        /* PB7 field */
        #define PB7                                          (0x00000080u)
        #define PB7_MASK                                     (0x00000080u)
        #define PB7_BIT                                      (7)
        #define PB7_BITS                                     (1)
        /* PB6 field */
        #define PB6                                          (0x00000040u)
        #define PB6_MASK                                     (0x00000040u)
        #define PB6_BIT                                      (6)
        #define PB6_BITS                                     (1)
        /* PB5 field */
        #define PB5                                          (0x00000020u)
        #define PB5_MASK                                     (0x00000020u)
        #define PB5_BIT                                      (5)
        #define PB5_BITS                                     (1)
        /* PB4 field */
        #define PB4                                          (0x00000010u)
        #define PB4_MASK                                     (0x00000010u)
        #define PB4_BIT                                      (4)
        #define PB4_BITS                                     (1)
        /* PB3 field */
        #define PB3                                          (0x00000008u)
        #define PB3_MASK                                     (0x00000008u)
        #define PB3_BIT                                      (3)
        #define PB3_BITS                                     (1)
        /* PB2 field */
        #define PB2                                          (0x00000004u)
        #define PB2_MASK                                     (0x00000004u)
        #define PB2_BIT                                      (2)
        #define PB2_BITS                                     (1)
        /* PB1 field */
        #define PB1                                          (0x00000002u)
        #define PB1_MASK                                     (0x00000002u)
        #define PB1_BIT                                      (1)
        #define PB1_BITS                                     (1)
        /* PB0 field */
        #define PB0                                          (0x00000001u)
        #define PB0_MASK                                     (0x00000001u)
        #define PB0_BIT                                      (0)
        #define PB0_BITS                                     (1)

#define GPIO_PCCFGL                                          *((volatile uint32_t *)0x4000B800u)
#define GPIO_PCCFGL_REG                                      *((volatile uint32_t *)0x4000B800u)
#define GPIO_PCCFGL_ADDR                                     (0x4000B800u)
#define GPIO_PCCFGL_RESET                                    (0x00004444u)
        /* PC3_CFG field */
        #define PC3_CFG                                      (0x0000F000u)
        #define PC3_CFG_MASK                                 (0x0000F000u)
        #define PC3_CFG_BIT                                  (12)
        #define PC3_CFG_BITS                                 (4)
        /* PC2_CFG field */
        #define PC2_CFG                                      (0x00000F00u)
        #define PC2_CFG_MASK                                 (0x00000F00u)
        #define PC2_CFG_BIT                                  (8)
        #define PC2_CFG_BITS                                 (4)
        /* PC1_CFG field */
        #define PC1_CFG                                      (0x000000F0u)
        #define PC1_CFG_MASK                                 (0x000000F0u)
        #define PC1_CFG_BIT                                  (4)
        #define PC1_CFG_BITS                                 (4)
        /* PC0_CFG field */
        #define PC0_CFG                                      (0x0000000Fu)
        #define PC0_CFG_MASK                                 (0x0000000Fu)
        #define PC0_CFG_BIT                                  (0)
        #define PC0_CFG_BITS                                 (4)

#define GPIO_PCCFGH                                          *((volatile uint32_t *)0x4000B804u)
#define GPIO_PCCFGH_REG                                      *((volatile uint32_t *)0x4000B804u)
#define GPIO_PCCFGH_ADDR                                     (0x4000B804u)
#define GPIO_PCCFGH_RESET                                    (0x00004444u)
        /* PC7_CFG field */
        #define PC7_CFG                                      (0x0000F000u)
        #define PC7_CFG_MASK                                 (0x0000F000u)
        #define PC7_CFG_BIT                                  (12)
        #define PC7_CFG_BITS                                 (4)
        /* PC6_CFG field */
        #define PC6_CFG                                      (0x00000F00u)
        #define PC6_CFG_MASK                                 (0x00000F00u)
        #define PC6_CFG_BIT                                  (8)
        #define PC6_CFG_BITS                                 (4)
        /* PC5_CFG field */
        #define PC5_CFG                                      (0x000000F0u)
        #define PC5_CFG_MASK                                 (0x000000F0u)
        #define PC5_CFG_BIT                                  (4)
        #define PC5_CFG_BITS                                 (4)
        /* PC4_CFG field */
        #define PC4_CFG                                      (0x0000000Fu)
        #define PC4_CFG_MASK                                 (0x0000000Fu)
        #define PC4_CFG_BIT                                  (0)
        #define PC4_CFG_BITS                                 (4)

#define GPIO_PCIN                                            *((volatile uint32_t *)0x4000B808u)
#define GPIO_PCIN_REG                                        *((volatile uint32_t *)0x4000B808u)
#define GPIO_PCIN_ADDR                                       (0x4000B808u)
#define GPIO_PCIN_RESET                                      (0x00000000u)
        /* PC7 field */
        #define PC7                                          (0x00000080u)
        #define PC7_MASK                                     (0x00000080u)
        #define PC7_BIT                                      (7)
        #define PC7_BITS                                     (1)
        /* PC6 field */
        #define PC6                                          (0x00000040u)
        #define PC6_MASK                                     (0x00000040u)
        #define PC6_BIT                                      (6)
        #define PC6_BITS                                     (1)
        /* PC5 field */
        #define PC5                                          (0x00000020u)
        #define PC5_MASK                                     (0x00000020u)
        #define PC5_BIT                                      (5)
        #define PC5_BITS                                     (1)
        /* PC4 field */
        #define PC4                                          (0x00000010u)
        #define PC4_MASK                                     (0x00000010u)
        #define PC4_BIT                                      (4)
        #define PC4_BITS                                     (1)
        /* PC3 field */
        #define PC3                                          (0x00000008u)
        #define PC3_MASK                                     (0x00000008u)
        #define PC3_BIT                                      (3)
        #define PC3_BITS                                     (1)
        /* PC2 field */
        #define PC2                                          (0x00000004u)
        #define PC2_MASK                                     (0x00000004u)
        #define PC2_BIT                                      (2)
        #define PC2_BITS                                     (1)
        /* PC1 field */
        #define PC1                                          (0x00000002u)
        #define PC1_MASK                                     (0x00000002u)
        #define PC1_BIT                                      (1)
        #define PC1_BITS                                     (1)
        /* PC0 field */
        #define PC0                                          (0x00000001u)
        #define PC0_MASK                                     (0x00000001u)
        #define PC0_BIT                                      (0)
        #define PC0_BITS                                     (1)

#define GPIO_PCOUT                                           *((volatile uint32_t *)0x4000B80Cu)
#define GPIO_PCOUT_REG                                       *((volatile uint32_t *)0x4000B80Cu)
#define GPIO_PCOUT_ADDR                                      (0x4000B80Cu)
#define GPIO_PCOUT_RESET                                     (0x00000000u)
        /* PC7 field */
        #define PC7                                          (0x00000080u)
        #define PC7_MASK                                     (0x00000080u)
        #define PC7_BIT                                      (7)
        #define PC7_BITS                                     (1)
        /* PC6 field */
        #define PC6                                          (0x00000040u)
        #define PC6_MASK                                     (0x00000040u)
        #define PC6_BIT                                      (6)
        #define PC6_BITS                                     (1)
        /* PC5 field */
        #define PC5                                          (0x00000020u)
        #define PC5_MASK                                     (0x00000020u)
        #define PC5_BIT                                      (5)
        #define PC5_BITS                                     (1)
        /* PC4 field */
        #define PC4                                          (0x00000010u)
        #define PC4_MASK                                     (0x00000010u)
        #define PC4_BIT                                      (4)
        #define PC4_BITS                                     (1)
        /* PC3 field */
        #define PC3                                          (0x00000008u)
        #define PC3_MASK                                     (0x00000008u)
        #define PC3_BIT                                      (3)
        #define PC3_BITS                                     (1)
        /* PC2 field */
        #define PC2                                          (0x00000004u)
        #define PC2_MASK                                     (0x00000004u)
        #define PC2_BIT                                      (2)
        #define PC2_BITS                                     (1)
        /* PC1 field */
        #define PC1                                          (0x00000002u)
        #define PC1_MASK                                     (0x00000002u)
        #define PC1_BIT                                      (1)
        #define PC1_BITS                                     (1)
        /* PC0 field */
        #define PC0                                          (0x00000001u)
        #define PC0_MASK                                     (0x00000001u)
        #define PC0_BIT                                      (0)
        #define PC0_BITS                                     (1)

#define GPIO_PCSET                                           *((volatile uint32_t *)0x4000B810u)
#define GPIO_PCSET_REG                                       *((volatile uint32_t *)0x4000B810u)
#define GPIO_PCSET_ADDR                                      (0x4000B810u)
#define GPIO_PCSET_RESET                                     (0x00000000u)
        /* GPIO_PXSETRSVD field */
        #define GPIO_PXSETRSVD                               (0x0000FF00u)
        #define GPIO_PXSETRSVD_MASK                          (0x0000FF00u)
        #define GPIO_PXSETRSVD_BIT                           (8)
        #define GPIO_PXSETRSVD_BITS                          (8)
        /* PC7 field */
        #define PC7                                          (0x00000080u)
        #define PC7_MASK                                     (0x00000080u)
        #define PC7_BIT                                      (7)
        #define PC7_BITS                                     (1)
        /* PC6 field */
        #define PC6                                          (0x00000040u)
        #define PC6_MASK                                     (0x00000040u)
        #define PC6_BIT                                      (6)
        #define PC6_BITS                                     (1)
        /* PC5 field */
        #define PC5                                          (0x00000020u)
        #define PC5_MASK                                     (0x00000020u)
        #define PC5_BIT                                      (5)
        #define PC5_BITS                                     (1)
        /* PC4 field */
        #define PC4                                          (0x00000010u)
        #define PC4_MASK                                     (0x00000010u)
        #define PC4_BIT                                      (4)
        #define PC4_BITS                                     (1)
        /* PC3 field */
        #define PC3                                          (0x00000008u)
        #define PC3_MASK                                     (0x00000008u)
        #define PC3_BIT                                      (3)
        #define PC3_BITS                                     (1)
        /* PC2 field */
        #define PC2                                          (0x00000004u)
        #define PC2_MASK                                     (0x00000004u)
        #define PC2_BIT                                      (2)
        #define PC2_BITS                                     (1)
        /* PC1 field */
        #define PC1                                          (0x00000002u)
        #define PC1_MASK                                     (0x00000002u)
        #define PC1_BIT                                      (1)
        #define PC1_BITS                                     (1)
        /* PC0 field */
        #define PC0                                          (0x00000001u)
        #define PC0_MASK                                     (0x00000001u)
        #define PC0_BIT                                      (0)
        #define PC0_BITS                                     (1)

#define GPIO_PCCLR                                           *((volatile uint32_t *)0x4000B814u)
#define GPIO_PCCLR_REG                                       *((volatile uint32_t *)0x4000B814u)
#define GPIO_PCCLR_ADDR                                      (0x4000B814u)
#define GPIO_PCCLR_RESET                                     (0x00000000u)
        /* PC7 field */
        #define PC7                                          (0x00000080u)
        #define PC7_MASK                                     (0x00000080u)
        #define PC7_BIT                                      (7)
        #define PC7_BITS                                     (1)
        /* PC6 field */
        #define PC6                                          (0x00000040u)
        #define PC6_MASK                                     (0x00000040u)
        #define PC6_BIT                                      (6)
        #define PC6_BITS                                     (1)
        /* PC5 field */
        #define PC5                                          (0x00000020u)
        #define PC5_MASK                                     (0x00000020u)
        #define PC5_BIT                                      (5)
        #define PC5_BITS                                     (1)
        /* PC4 field */
        #define PC4                                          (0x00000010u)
        #define PC4_MASK                                     (0x00000010u)
        #define PC4_BIT                                      (4)
        #define PC4_BITS                                     (1)
        /* PC3 field */
        #define PC3                                          (0x00000008u)
        #define PC3_MASK                                     (0x00000008u)
        #define PC3_BIT                                      (3)
        #define PC3_BITS                                     (1)
        /* PC2 field */
        #define PC2                                          (0x00000004u)
        #define PC2_MASK                                     (0x00000004u)
        #define PC2_BIT                                      (2)
        #define PC2_BITS                                     (1)
        /* PC1 field */
        #define PC1                                          (0x00000002u)
        #define PC1_MASK                                     (0x00000002u)
        #define PC1_BIT                                      (1)
        #define PC1_BITS                                     (1)
        /* PC0 field */
        #define PC0                                          (0x00000001u)
        #define PC0_MASK                                     (0x00000001u)
        #define PC0_BIT                                      (0)
        #define PC0_BITS                                     (1)

#define GPIO_DBGCFG                                          *((volatile uint32_t *)0x4000BC00u)
#define GPIO_DBGCFG_REG                                      *((volatile uint32_t *)0x4000BC00u)
#define GPIO_DBGCFG_ADDR                                     (0x4000BC00u)
#define GPIO_DBGCFG_RESET                                    (0x00000010u)
        /* GPIO_DEBUGDIS field */
        #define GPIO_DEBUGDIS                                (0x00000020u)
        #define GPIO_DEBUGDIS_MASK                           (0x00000020u)
        #define GPIO_DEBUGDIS_BIT                            (5)
        #define GPIO_DEBUGDIS_BITS                           (1)
        /* GPIO_EXTREGEN field */
        #define GPIO_EXTREGEN                                (0x00000010u)
        #define GPIO_EXTREGEN_MASK                           (0x00000010u)
        #define GPIO_EXTREGEN_BIT                            (4)
        #define GPIO_EXTREGEN_BITS                           (1)
        /* GPIO_DBGCFGRSVD field */
        #define GPIO_DBGCFGRSVD                              (0x00000008u)
        #define GPIO_DBGCFGRSVD_MASK                         (0x00000008u)
        #define GPIO_DBGCFGRSVD_BIT                          (3)
        #define GPIO_DBGCFGRSVD_BITS                         (1)

#define GPIO_DBGSTAT                                         *((volatile uint32_t *)0x4000BC04u)
#define GPIO_DBGSTAT_REG                                     *((volatile uint32_t *)0x4000BC04u)
#define GPIO_DBGSTAT_ADDR                                    (0x4000BC04u)
#define GPIO_DBGSTAT_RESET                                   (0x00000000u)
        /* GPIO_BOOTMODE field */
        #define GPIO_BOOTMODE                                (0x00000008u)
        #define GPIO_BOOTMODE_MASK                           (0x00000008u)
        #define GPIO_BOOTMODE_BIT                            (3)
        #define GPIO_BOOTMODE_BITS                           (1)
        /* GPIO_FORCEDBG field */
        #define GPIO_FORCEDBG                                (0x00000002u)
        #define GPIO_FORCEDBG_MASK                           (0x00000002u)
        #define GPIO_FORCEDBG_BIT                            (1)
        #define GPIO_FORCEDBG_BITS                           (1)
        /* GPIO_SWEN field */
        #define GPIO_SWEN                                    (0x00000001u)
        #define GPIO_SWEN_MASK                               (0x00000001u)
        #define GPIO_SWEN_BIT                                (0)
        #define GPIO_SWEN_BITS                               (1)

#define GPIO_PAWAKE                                          *((volatile uint32_t *)0x4000BC08u)
#define GPIO_PAWAKE_REG                                      *((volatile uint32_t *)0x4000BC08u)
#define GPIO_PAWAKE_ADDR                                     (0x4000BC08u)
#define GPIO_PAWAKE_RESET                                    (0x00000000u)
        /* PA7 field */
        #define PA7                                          (0x00000080u)
        #define PA7_MASK                                     (0x00000080u)
        #define PA7_BIT                                      (7)
        #define PA7_BITS                                     (1)
        /* PA6 field */
        #define PA6                                          (0x00000040u)
        #define PA6_MASK                                     (0x00000040u)
        #define PA6_BIT                                      (6)
        #define PA6_BITS                                     (1)
        /* PA5 field */
        #define PA5                                          (0x00000020u)
        #define PA5_MASK                                     (0x00000020u)
        #define PA5_BIT                                      (5)
        #define PA5_BITS                                     (1)
        /* PA4 field */
        #define PA4                                          (0x00000010u)
        #define PA4_MASK                                     (0x00000010u)
        #define PA4_BIT                                      (4)
        #define PA4_BITS                                     (1)
        /* PA3 field */
        #define PA3                                          (0x00000008u)
        #define PA3_MASK                                     (0x00000008u)
        #define PA3_BIT                                      (3)
        #define PA3_BITS                                     (1)
        /* PA2 field */
        #define PA2                                          (0x00000004u)
        #define PA2_MASK                                     (0x00000004u)
        #define PA2_BIT                                      (2)
        #define PA2_BITS                                     (1)
        /* PA1 field */
        #define PA1                                          (0x00000002u)
        #define PA1_MASK                                     (0x00000002u)
        #define PA1_BIT                                      (1)
        #define PA1_BITS                                     (1)
        /* PA0 field */
        #define PA0                                          (0x00000001u)
        #define PA0_MASK                                     (0x00000001u)
        #define PA0_BIT                                      (0)
        #define PA0_BITS                                     (1)

#define GPIO_PBWAKE                                          *((volatile uint32_t *)0x4000BC0Cu)
#define GPIO_PBWAKE_REG                                      *((volatile uint32_t *)0x4000BC0Cu)
#define GPIO_PBWAKE_ADDR                                     (0x4000BC0Cu)
#define GPIO_PBWAKE_RESET                                    (0x00000000u)
        /* PB7 field */
        #define PB7                                          (0x00000080u)
        #define PB7_MASK                                     (0x00000080u)
        #define PB7_BIT                                      (7)
        #define PB7_BITS                                     (1)
        /* PB6 field */
        #define PB6                                          (0x00000040u)
        #define PB6_MASK                                     (0x00000040u)
        #define PB6_BIT                                      (6)
        #define PB6_BITS                                     (1)
        /* PB5 field */
        #define PB5                                          (0x00000020u)
        #define PB5_MASK                                     (0x00000020u)
        #define PB5_BIT                                      (5)
        #define PB5_BITS                                     (1)
        /* PB4 field */
        #define PB4                                          (0x00000010u)
        #define PB4_MASK                                     (0x00000010u)
        #define PB4_BIT                                      (4)
        #define PB4_BITS                                     (1)
        /* PB3 field */
        #define PB3                                          (0x00000008u)
        #define PB3_MASK                                     (0x00000008u)
        #define PB3_BIT                                      (3)
        #define PB3_BITS                                     (1)
        /* PB2 field */
        #define PB2                                          (0x00000004u)
        #define PB2_MASK                                     (0x00000004u)
        #define PB2_BIT                                      (2)
        #define PB2_BITS                                     (1)
        /* PB1 field */
        #define PB1                                          (0x00000002u)
        #define PB1_MASK                                     (0x00000002u)
        #define PB1_BIT                                      (1)
        #define PB1_BITS                                     (1)
        /* PB0 field */
        #define PB0                                          (0x00000001u)
        #define PB0_MASK                                     (0x00000001u)
        #define PB0_BIT                                      (0)
        #define PB0_BITS                                     (1)

#define GPIO_PCWAKE                                          *((volatile uint32_t *)0x4000BC10u)
#define GPIO_PCWAKE_REG                                      *((volatile uint32_t *)0x4000BC10u)
#define GPIO_PCWAKE_ADDR                                     (0x4000BC10u)
#define GPIO_PCWAKE_RESET                                    (0x00000000u)
        /* PC7 field */
        #define PC7                                          (0x00000080u)
        #define PC7_MASK                                     (0x00000080u)
        #define PC7_BIT                                      (7)
        #define PC7_BITS                                     (1)
        /* PC6 field */
        #define PC6                                          (0x00000040u)
        #define PC6_MASK                                     (0x00000040u)
        #define PC6_BIT                                      (6)
        #define PC6_BITS                                     (1)
        /* PC5 field */
        #define PC5                                          (0x00000020u)
        #define PC5_MASK                                     (0x00000020u)
        #define PC5_BIT                                      (5)
        #define PC5_BITS                                     (1)
        /* PC4 field */
        #define PC4                                          (0x00000010u)
        #define PC4_MASK                                     (0x00000010u)
        #define PC4_BIT                                      (4)
        #define PC4_BITS                                     (1)
        /* PC3 field */
        #define PC3                                          (0x00000008u)
        #define PC3_MASK                                     (0x00000008u)
        #define PC3_BIT                                      (3)
        #define PC3_BITS                                     (1)
        /* PC2 field */
        #define PC2                                          (0x00000004u)
        #define PC2_MASK                                     (0x00000004u)
        #define PC2_BIT                                      (2)
        #define PC2_BITS                                     (1)
        /* PC1 field */
        #define PC1                                          (0x00000002u)
        #define PC1_MASK                                     (0x00000002u)
        #define PC1_BIT                                      (1)
        #define PC1_BITS                                     (1)
        /* PC0 field */
        #define PC0                                          (0x00000001u)
        #define PC0_MASK                                     (0x00000001u)
        #define PC0_BIT                                      (0)
        #define PC0_BITS                                     (1)

#define GPIO_IRQCSEL                                         *((volatile uint32_t *)0x4000BC14u)
#define GPIO_IRQCSEL_REG                                     *((volatile uint32_t *)0x4000BC14u)
#define GPIO_IRQCSEL_ADDR                                    (0x4000BC14u)
#define GPIO_IRQCSEL_RESET                                   (0x0000000Fu)
        /* SEL_GPIO field */
        #define SEL_GPIO                                     (0x0000001Fu)
        #define SEL_GPIO_MASK                                (0x0000001Fu)
        #define SEL_GPIO_BIT                                 (0)
        #define SEL_GPIO_BITS                                (5)

#define GPIO_IRQDSEL                                         *((volatile uint32_t *)0x4000BC18u)
#define GPIO_IRQDSEL_REG                                     *((volatile uint32_t *)0x4000BC18u)
#define GPIO_IRQDSEL_ADDR                                    (0x4000BC18u)
#define GPIO_IRQDSEL_RESET                                   (0x00000010u)
        /* SEL_GPIO field */
        #define SEL_GPIO                                     (0x0000001Fu)
        #define SEL_GPIO_MASK                                (0x0000001Fu)
        #define SEL_GPIO_BIT                                 (0)
        #define SEL_GPIO_BITS                                (5)

#define GPIO_WAKEFILT                                        *((volatile uint32_t *)0x4000BC1Cu)
#define GPIO_WAKEFILT_REG                                    *((volatile uint32_t *)0x4000BC1Cu)
#define GPIO_WAKEFILT_ADDR                                   (0x4000BC1Cu)
#define GPIO_WAKEFILT_RESET                                  (0x00000000u)
        /* IRQD_WAKE_FILTER field */
        #define IRQD_WAKE_FILTER                             (0x00000008u)
        #define IRQD_WAKE_FILTER_MASK                        (0x00000008u)
        #define IRQD_WAKE_FILTER_BIT                         (3)
        #define IRQD_WAKE_FILTER_BITS                        (1)
        /* SC2_WAKE_FILTER field */
        #define SC2_WAKE_FILTER                              (0x00000004u)
        #define SC2_WAKE_FILTER_MASK                         (0x00000004u)
        #define SC2_WAKE_FILTER_BIT                          (2)
        #define SC2_WAKE_FILTER_BITS                         (1)
        /* SC1_WAKE_FILTER field */
        #define SC1_WAKE_FILTER                              (0x00000002u)
        #define SC1_WAKE_FILTER_MASK                         (0x00000002u)
        #define SC1_WAKE_FILTER_BIT                          (1)
        #define SC1_WAKE_FILTER_BITS                         (1)
        /* GPIO_WAKE_FILTER field */
        #define GPIO_WAKE_FILTER                             (0x00000001u)
        #define GPIO_WAKE_FILTER_MASK                        (0x00000001u)
        #define GPIO_WAKE_FILTER_BIT                         (0)
        #define GPIO_WAKE_FILTER_BITS                        (1)

/* SERIAL block */
#define BLOCK_SERIAL_BASE                                    (0x4000C000u)
#define BLOCK_SERIAL_END                                     (0x4000C870u)
#define BLOCK_SERIAL_SIZE                                    (BLOCK_SERIAL_END - BLOCK_SERIAL_BASE + 1)

#define SC2_RXBEGA                                           *((volatile uint32_t *)0x4000C000u)
#define SC2_RXBEGA_REG                                       *((volatile uint32_t *)0x4000C000u)
#define SC2_RXBEGA_ADDR                                      (0x4000C000u)
#define SC2_RXBEGA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_RXBEGA_FIXED                             (0xFFFFE000u)
        #define SC2_RXBEGA_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_RXBEGA_FIXED_BIT                         (13)
        #define SC2_RXBEGA_FIXED_BITS                        (19)
        /* SC_RXBEGA field */
        #define SC_RXBEGA                                    (0x00001FFFu)
        #define SC_RXBEGA_MASK                               (0x00001FFFu)
        #define SC_RXBEGA_BIT                                (0)
        #define SC_RXBEGA_BITS                               (13)

#define SC2_RXENDA                                           *((volatile uint32_t *)0x4000C004u)
#define SC2_RXENDA_REG                                       *((volatile uint32_t *)0x4000C004u)
#define SC2_RXENDA_ADDR                                      (0x4000C004u)
#define SC2_RXENDA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_RXENDA_FIXED                             (0xFFFFE000u)
        #define SC2_RXENDA_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_RXENDA_FIXED_BIT                         (13)
        #define SC2_RXENDA_FIXED_BITS                        (19)
        /* SC_RXENDA field */
        #define SC_RXENDA                                    (0x00001FFFu)
        #define SC_RXENDA_MASK                               (0x00001FFFu)
        #define SC_RXENDA_BIT                                (0)
        #define SC_RXENDA_BITS                               (13)

#define SC2_RXBEGB                                           *((volatile uint32_t *)0x4000C008u)
#define SC2_RXBEGB_REG                                       *((volatile uint32_t *)0x4000C008u)
#define SC2_RXBEGB_ADDR                                      (0x4000C008u)
#define SC2_RXBEGB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_RXBEGB_FIXED                             (0xFFFFE000u)
        #define SC2_RXBEGB_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_RXBEGB_FIXED_BIT                         (13)
        #define SC2_RXBEGB_FIXED_BITS                        (19)
        /* SC_RXBEGB field */
        #define SC_RXBEGB                                    (0x00001FFFu)
        #define SC_RXBEGB_MASK                               (0x00001FFFu)
        #define SC_RXBEGB_BIT                                (0)
        #define SC_RXBEGB_BITS                               (13)

#define SC2_RXENDB                                           *((volatile uint32_t *)0x4000C00Cu)
#define SC2_RXENDB_REG                                       *((volatile uint32_t *)0x4000C00Cu)
#define SC2_RXENDB_ADDR                                      (0x4000C00Cu)
#define SC2_RXENDB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_RXENDB_FIXED                             (0xFFFFE000u)
        #define SC2_RXENDB_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_RXENDB_FIXED_BIT                         (13)
        #define SC2_RXENDB_FIXED_BITS                        (19)
        /* SC_RXENDB field */
        #define SC_RXENDB                                    (0x00001FFFu)
        #define SC_RXENDB_MASK                               (0x00001FFFu)
        #define SC_RXENDB_BIT                                (0)
        #define SC_RXENDB_BITS                               (13)

#define SC2_TXBEGA                                           *((volatile uint32_t *)0x4000C010u)
#define SC2_TXBEGA_REG                                       *((volatile uint32_t *)0x4000C010u)
#define SC2_TXBEGA_ADDR                                      (0x4000C010u)
#define SC2_TXBEGA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_TXBEGA_FIXED                             (0xFFFFE000u)
        #define SC2_TXBEGA_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_TXBEGA_FIXED_BIT                         (13)
        #define SC2_TXBEGA_FIXED_BITS                        (19)
        /* SC_TXBEGA field */
        #define SC_TXBEGA                                    (0x00001FFFu)
        #define SC_TXBEGA_MASK                               (0x00001FFFu)
        #define SC_TXBEGA_BIT                                (0)
        #define SC_TXBEGA_BITS                               (13)

#define SC2_TXENDA                                           *((volatile uint32_t *)0x4000C014u)
#define SC2_TXENDA_REG                                       *((volatile uint32_t *)0x4000C014u)
#define SC2_TXENDA_ADDR                                      (0x4000C014u)
#define SC2_TXENDA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_TXENDA_FIXED                             (0xFFFFE000u)
        #define SC2_TXENDA_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_TXENDA_FIXED_BIT                         (13)
        #define SC2_TXENDA_FIXED_BITS                        (19)
        /* SC_TXENDA field */
        #define SC_TXENDA                                    (0x00001FFFu)
        #define SC_TXENDA_MASK                               (0x00001FFFu)
        #define SC_TXENDA_BIT                                (0)
        #define SC_TXENDA_BITS                               (13)

#define SC2_TXBEGB                                           *((volatile uint32_t *)0x4000C018u)
#define SC2_TXBEGB_REG                                       *((volatile uint32_t *)0x4000C018u)
#define SC2_TXBEGB_ADDR                                      (0x4000C018u)
#define SC2_TXBEGB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_TXBEGB_FIXED                             (0xFFFFE000u)
        #define SC2_TXBEGB_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_TXBEGB_FIXED_BIT                         (13)
        #define SC2_TXBEGB_FIXED_BITS                        (19)
        /* SC_TXBEGB field */
        #define SC_TXBEGB                                    (0x00001FFFu)
        #define SC_TXBEGB_MASK                               (0x00001FFFu)
        #define SC_TXBEGB_BIT                                (0)
        #define SC_TXBEGB_BITS                               (13)

#define SC2_TXENDB                                           *((volatile uint32_t *)0x4000C01Cu)
#define SC2_TXENDB_REG                                       *((volatile uint32_t *)0x4000C01Cu)
#define SC2_TXENDB_ADDR                                      (0x4000C01Cu)
#define SC2_TXENDB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC2_TXENDB_FIXED                             (0xFFFFE000u)
        #define SC2_TXENDB_FIXED_MASK                        (0xFFFFE000u)
        #define SC2_TXENDB_FIXED_BIT                         (13)
        #define SC2_TXENDB_FIXED_BITS                        (19)
        /* SC_TXENDB field */
        #define SC_TXENDB                                    (0x00001FFFu)
        #define SC_TXENDB_MASK                               (0x00001FFFu)
        #define SC_TXENDB_BIT                                (0)
        #define SC_TXENDB_BITS                               (13)

#define SC2_RXCNTA                                           *((volatile uint32_t *)0x4000C020u)
#define SC2_RXCNTA_REG                                       *((volatile uint32_t *)0x4000C020u)
#define SC2_RXCNTA_ADDR                                      (0x4000C020u)
#define SC2_RXCNTA_RESET                                     (0x00000000u)
        /* SC_RXCNTA field */
        #define SC_RXCNTA                                    (0x00001FFFu)
        #define SC_RXCNTA_MASK                               (0x00001FFFu)
        #define SC_RXCNTA_BIT                                (0)
        #define SC_RXCNTA_BITS                               (13)

#define SC2_RXCNTB                                           *((volatile uint32_t *)0x4000C024u)
#define SC2_RXCNTB_REG                                       *((volatile uint32_t *)0x4000C024u)
#define SC2_RXCNTB_ADDR                                      (0x4000C024u)
#define SC2_RXCNTB_RESET                                     (0x00000000u)
        /* SC_RXCNTB field */
        #define SC_RXCNTB                                    (0x00001FFFu)
        #define SC_RXCNTB_MASK                               (0x00001FFFu)
        #define SC_RXCNTB_BIT                                (0)
        #define SC_RXCNTB_BITS                               (13)

#define SC2_TXCNT                                            *((volatile uint32_t *)0x4000C028u)
#define SC2_TXCNT_REG                                        *((volatile uint32_t *)0x4000C028u)
#define SC2_TXCNT_ADDR                                       (0x4000C028u)
#define SC2_TXCNT_RESET                                      (0x00000000u)
        /* SC_TXCNT field */
        #define SC_TXCNT                                     (0x00001FFFu)
        #define SC_TXCNT_MASK                                (0x00001FFFu)
        #define SC_TXCNT_BIT                                 (0)
        #define SC_TXCNT_BITS                                (13)

#define SC2_DMASTAT                                          *((volatile uint32_t *)0x4000C02Cu)
#define SC2_DMASTAT_REG                                      *((volatile uint32_t *)0x4000C02Cu)
#define SC2_DMASTAT_ADDR                                     (0x4000C02Cu)
#define SC2_DMASTAT_RESET                                    (0x00000000u)
        /* SC_RXSSEL field */
        #define SC_RXSSEL                                    (0x00001C00u)
        #define SC_RXSSEL_MASK                               (0x00001C00u)
        #define SC_RXSSEL_BIT                                (10)
        #define SC_RXSSEL_BITS                               (3)
        /* SC_RXOVFB field */
        #define SC_RXOVFB                                    (0x00000020u)
        #define SC_RXOVFB_MASK                               (0x00000020u)
        #define SC_RXOVFB_BIT                                (5)
        #define SC_RXOVFB_BITS                               (1)
        /* SC_RXOVFA field */
        #define SC_RXOVFA                                    (0x00000010u)
        #define SC_RXOVFA_MASK                               (0x00000010u)
        #define SC_RXOVFA_BIT                                (4)
        #define SC_RXOVFA_BITS                               (1)
        /* SC_TXACTB field */
        #define SC_TXACTB                                    (0x00000008u)
        #define SC_TXACTB_MASK                               (0x00000008u)
        #define SC_TXACTB_BIT                                (3)
        #define SC_TXACTB_BITS                               (1)
        /* SC_TXACTA field */
        #define SC_TXACTA                                    (0x00000004u)
        #define SC_TXACTA_MASK                               (0x00000004u)
        #define SC_TXACTA_BIT                                (2)
        #define SC_TXACTA_BITS                               (1)
        /* SC_RXACTB field */
        #define SC_RXACTB                                    (0x00000002u)
        #define SC_RXACTB_MASK                               (0x00000002u)
        #define SC_RXACTB_BIT                                (1)
        #define SC_RXACTB_BITS                               (1)
        /* SC_RXACTA field */
        #define SC_RXACTA                                    (0x00000001u)
        #define SC_RXACTA_MASK                               (0x00000001u)
        #define SC_RXACTA_BIT                                (0)
        #define SC_RXACTA_BITS                               (1)

#define SC2_DMACTRL                                          *((volatile uint32_t *)0x4000C030u)
#define SC2_DMACTRL_REG                                      *((volatile uint32_t *)0x4000C030u)
#define SC2_DMACTRL_ADDR                                     (0x4000C030u)
#define SC2_DMACTRL_RESET                                    (0x00000000u)
        /* SC_TXDMARST field */
        #define SC_TXDMARST                                  (0x00000020u)
        #define SC_TXDMARST_MASK                             (0x00000020u)
        #define SC_TXDMARST_BIT                              (5)
        #define SC_TXDMARST_BITS                             (1)
        /* SC_RXDMARST field */
        #define SC_RXDMARST                                  (0x00000010u)
        #define SC_RXDMARST_MASK                             (0x00000010u)
        #define SC_RXDMARST_BIT                              (4)
        #define SC_RXDMARST_BITS                             (1)
        /* SC_TXLODB field */
        #define SC_TXLODB                                    (0x00000008u)
        #define SC_TXLODB_MASK                               (0x00000008u)
        #define SC_TXLODB_BIT                                (3)
        #define SC_TXLODB_BITS                               (1)
        /* SC_TXLODA field */
        #define SC_TXLODA                                    (0x00000004u)
        #define SC_TXLODA_MASK                               (0x00000004u)
        #define SC_TXLODA_BIT                                (2)
        #define SC_TXLODA_BITS                               (1)
        /* SC_RXLODB field */
        #define SC_RXLODB                                    (0x00000002u)
        #define SC_RXLODB_MASK                               (0x00000002u)
        #define SC_RXLODB_BIT                                (1)
        #define SC_RXLODB_BITS                               (1)
        /* SC_RXLODA field */
        #define SC_RXLODA                                    (0x00000001u)
        #define SC_RXLODA_MASK                               (0x00000001u)
        #define SC_RXLODA_BIT                                (0)
        #define SC_RXLODA_BITS                               (1)

#define SC2_RXERRA                                           *((volatile uint32_t *)0x4000C034u)
#define SC2_RXERRA_REG                                       *((volatile uint32_t *)0x4000C034u)
#define SC2_RXERRA_ADDR                                      (0x4000C034u)
#define SC2_RXERRA_RESET                                     (0x00000000u)
        /* SC_RXERRA field */
        #define SC_RXERRA                                    (0x00001FFFu)
        #define SC_RXERRA_MASK                               (0x00001FFFu)
        #define SC_RXERRA_BIT                                (0)
        #define SC_RXERRA_BITS                               (13)

#define SC2_RXERRB                                           *((volatile uint32_t *)0x4000C038u)
#define SC2_RXERRB_REG                                       *((volatile uint32_t *)0x4000C038u)
#define SC2_RXERRB_ADDR                                      (0x4000C038u)
#define SC2_RXERRB_RESET                                     (0x00000000u)
        /* SC_RXERRB field */
        #define SC_RXERRB                                    (0x00001FFFu)
        #define SC_RXERRB_MASK                               (0x00001FFFu)
        #define SC_RXERRB_BIT                                (0)
        #define SC_RXERRB_BITS                               (13)

#define SC2_DATA                                             *((volatile uint32_t *)0x4000C03Cu)
#define SC2_DATA_REG                                         *((volatile uint32_t *)0x4000C03Cu)
#define SC2_DATA_ADDR                                        (0x4000C03Cu)
#define SC2_DATA_RESET                                       (0x00000000u)
        /* SC_DATA field */
        #define SC_DATA                                      (0x000000FFu)
        #define SC_DATA_MASK                                 (0x000000FFu)
        #define SC_DATA_BIT                                  (0)
        #define SC_DATA_BITS                                 (8)

#define SC2_SPISTAT                                          *((volatile uint32_t *)0x4000C040u)
#define SC2_SPISTAT_REG                                      *((volatile uint32_t *)0x4000C040u)
#define SC2_SPISTAT_ADDR                                     (0x4000C040u)
#define SC2_SPISTAT_RESET                                    (0x00000000u)
        /* SC_SPITXIDLE field */
        #define SC_SPITXIDLE                                 (0x00000008u)
        #define SC_SPITXIDLE_MASK                            (0x00000008u)
        #define SC_SPITXIDLE_BIT                             (3)
        #define SC_SPITXIDLE_BITS                            (1)
        /* SC_SPITXFREE field */
        #define SC_SPITXFREE                                 (0x00000004u)
        #define SC_SPITXFREE_MASK                            (0x00000004u)
        #define SC_SPITXFREE_BIT                             (2)
        #define SC_SPITXFREE_BITS                            (1)
        /* SC_SPIRXVAL field */
        #define SC_SPIRXVAL                                  (0x00000002u)
        #define SC_SPIRXVAL_MASK                             (0x00000002u)
        #define SC_SPIRXVAL_BIT                              (1)
        #define SC_SPIRXVAL_BITS                             (1)
        /* SC_SPIRXOVF field */
        #define SC_SPIRXOVF                                  (0x00000001u)
        #define SC_SPIRXOVF_MASK                             (0x00000001u)
        #define SC_SPIRXOVF_BIT                              (0)
        #define SC_SPIRXOVF_BITS                             (1)

#define SC2_TWISTAT                                          *((volatile uint32_t *)0x4000C044u)
#define SC2_TWISTAT_REG                                      *((volatile uint32_t *)0x4000C044u)
#define SC2_TWISTAT_ADDR                                     (0x4000C044u)
#define SC2_TWISTAT_RESET                                    (0x00000000u)
        /* SC_TWICMDFIN field */
        #define SC_TWICMDFIN                                 (0x00000008u)
        #define SC_TWICMDFIN_MASK                            (0x00000008u)
        #define SC_TWICMDFIN_BIT                             (3)
        #define SC_TWICMDFIN_BITS                            (1)
        /* SC_TWIRXFIN field */
        #define SC_TWIRXFIN                                  (0x00000004u)
        #define SC_TWIRXFIN_MASK                             (0x00000004u)
        #define SC_TWIRXFIN_BIT                              (2)
        #define SC_TWIRXFIN_BITS                             (1)
        /* SC_TWITXFIN field */
        #define SC_TWITXFIN                                  (0x00000002u)
        #define SC_TWITXFIN_MASK                             (0x00000002u)
        #define SC_TWITXFIN_BIT                              (1)
        #define SC_TWITXFIN_BITS                             (1)
        /* SC_TWIRXNAK field */
        #define SC_TWIRXNAK                                  (0x00000001u)
        #define SC_TWIRXNAK_MASK                             (0x00000001u)
        #define SC_TWIRXNAK_BIT                              (0)
        #define SC_TWIRXNAK_BITS                             (1)

#define SC2_TWICTRL1                                         *((volatile uint32_t *)0x4000C04Cu)
#define SC2_TWICTRL1_REG                                     *((volatile uint32_t *)0x4000C04Cu)
#define SC2_TWICTRL1_ADDR                                    (0x4000C04Cu)
#define SC2_TWICTRL1_RESET                                   (0x00000000u)
        /* SC_TWISTOP field */
        #define SC_TWISTOP                                   (0x00000008u)
        #define SC_TWISTOP_MASK                              (0x00000008u)
        #define SC_TWISTOP_BIT                               (3)
        #define SC_TWISTOP_BITS                              (1)
        /* SC_TWISTART field */
        #define SC_TWISTART                                  (0x00000004u)
        #define SC_TWISTART_MASK                             (0x00000004u)
        #define SC_TWISTART_BIT                              (2)
        #define SC_TWISTART_BITS                             (1)
        /* SC_TWISEND field */
        #define SC_TWISEND                                   (0x00000002u)
        #define SC_TWISEND_MASK                              (0x00000002u)
        #define SC_TWISEND_BIT                               (1)
        #define SC_TWISEND_BITS                              (1)
        /* SC_TWIRECV field */
        #define SC_TWIRECV                                   (0x00000001u)
        #define SC_TWIRECV_MASK                              (0x00000001u)
        #define SC_TWIRECV_BIT                               (0)
        #define SC_TWIRECV_BITS                              (1)

#define SC2_TWICTRL2                                         *((volatile uint32_t *)0x4000C050u)
#define SC2_TWICTRL2_REG                                     *((volatile uint32_t *)0x4000C050u)
#define SC2_TWICTRL2_ADDR                                    (0x4000C050u)
#define SC2_TWICTRL2_RESET                                   (0x00000000u)
        /* SC_TWIACK field */
        #define SC_TWIACK                                    (0x00000001u)
        #define SC_TWIACK_MASK                               (0x00000001u)
        #define SC_TWIACK_BIT                                (0)
        #define SC_TWIACK_BITS                               (1)

#define SC2_MODE                                             *((volatile uint32_t *)0x4000C054u)
#define SC2_MODE_REG                                         *((volatile uint32_t *)0x4000C054u)
#define SC2_MODE_ADDR                                        (0x4000C054u)
#define SC2_MODE_RESET                                       (0x00000000u)
        /* SC_MODE field */
        #define SC_MODE                                      (0x00000003u)
        #define SC_MODE_MASK                                 (0x00000003u)
        #define SC_MODE_BIT                                  (0)
        #define SC_MODE_BITS                                 (2)
                /* SC_MODE Bit Field Values */
                #define SC2_MODE_DISABLED                    (0)
                #define SC2_MODE_SPI                         (2)
                #define SC2_MODE_I2C                         (3)

#define SC2_SPICFG                                           *((volatile uint32_t *)0x4000C058u)
#define SC2_SPICFG_REG                                       *((volatile uint32_t *)0x4000C058u)
#define SC2_SPICFG_ADDR                                      (0x4000C058u)
#define SC2_SPICFG_RESET                                     (0x00000000u)
        /* SC_SPIRXDRV field */
        #define SC_SPIRXDRV                                  (0x00000020u)
        #define SC_SPIRXDRV_MASK                             (0x00000020u)
        #define SC_SPIRXDRV_BIT                              (5)
        #define SC_SPIRXDRV_BITS                             (1)
        /* SC_SPIMST field */
        #define SC_SPIMST                                    (0x00000010u)
        #define SC_SPIMST_MASK                               (0x00000010u)
        #define SC_SPIMST_BIT                                (4)
        #define SC_SPIMST_BITS                               (1)
        /* SC_SPIRPT field */
        #define SC_SPIRPT                                    (0x00000008u)
        #define SC_SPIRPT_MASK                               (0x00000008u)
        #define SC_SPIRPT_BIT                                (3)
        #define SC_SPIRPT_BITS                               (1)
        /* SC_SPIORD field */
        #define SC_SPIORD                                    (0x00000004u)
        #define SC_SPIORD_MASK                               (0x00000004u)
        #define SC_SPIORD_BIT                                (2)
        #define SC_SPIORD_BITS                               (1)
        /* SC_SPIPHA field */
        #define SC_SPIPHA                                    (0x00000002u)
        #define SC_SPIPHA_MASK                               (0x00000002u)
        #define SC_SPIPHA_BIT                                (1)
        #define SC_SPIPHA_BITS                               (1)
        /* SC_SPIPOL field */
        #define SC_SPIPOL                                    (0x00000001u)
        #define SC_SPIPOL_MASK                               (0x00000001u)
        #define SC_SPIPOL_BIT                                (0)
        #define SC_SPIPOL_BITS                               (1)

#define SC2_RATELIN                                          *((volatile uint32_t *)0x4000C060u)
#define SC2_RATELIN_REG                                      *((volatile uint32_t *)0x4000C060u)
#define SC2_RATELIN_ADDR                                     (0x4000C060u)
#define SC2_RATELIN_RESET                                    (0x00000000u)
        /* SC_RATELIN field */
        #define SC_RATELIN                                   (0x0000000Fu)
        #define SC_RATELIN_MASK                              (0x0000000Fu)
        #define SC_RATELIN_BIT                               (0)
        #define SC_RATELIN_BITS                              (4)

#define SC2_RATEEXP                                          *((volatile uint32_t *)0x4000C064u)
#define SC2_RATEEXP_REG                                      *((volatile uint32_t *)0x4000C064u)
#define SC2_RATEEXP_ADDR                                     (0x4000C064u)
#define SC2_RATEEXP_RESET                                    (0x00000000u)
        /* SC_RATEEXP field */
        #define SC_RATEEXP                                   (0x0000000Fu)
        #define SC_RATEEXP_MASK                              (0x0000000Fu)
        #define SC_RATEEXP_BIT                               (0)
        #define SC_RATEEXP_BITS                              (4)

#define SC2_RXCNTSAVED                                       *((volatile uint32_t *)0x4000C070u)
#define SC2_RXCNTSAVED_REG                                   *((volatile uint32_t *)0x4000C070u)
#define SC2_RXCNTSAVED_ADDR                                  (0x4000C070u)
#define SC2_RXCNTSAVED_RESET                                 (0x00000000u)
        /* SC_RXCNTSAVED field */
        #define SC_RXCNTSAVED                                (0x00001FFFu)
        #define SC_RXCNTSAVED_MASK                           (0x00001FFFu)
        #define SC_RXCNTSAVED_BIT                            (0)
        #define SC_RXCNTSAVED_BITS                           (13)

#define SC1_RXBEGA                                           *((volatile uint32_t *)0x4000C800u)
#define SC1_RXBEGA_REG                                       *((volatile uint32_t *)0x4000C800u)
#define SC1_RXBEGA_ADDR                                      (0x4000C800u)
#define SC1_RXBEGA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_RXBEGA_FIXED                             (0xFFFFE000u)
        #define SC1_RXBEGA_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_RXBEGA_FIXED_BIT                         (13)
        #define SC1_RXBEGA_FIXED_BITS                        (19)
        /* SC_RXBEGA field */
        #define SC_RXBEGA                                    (0x00001FFFu)
        #define SC_RXBEGA_MASK                               (0x00001FFFu)
        #define SC_RXBEGA_BIT                                (0)
        #define SC_RXBEGA_BITS                               (13)

#define SC1_RXENDA                                           *((volatile uint32_t *)0x4000C804u)
#define SC1_RXENDA_REG                                       *((volatile uint32_t *)0x4000C804u)
#define SC1_RXENDA_ADDR                                      (0x4000C804u)
#define SC1_RXENDA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_RXENDA_FIXED                             (0xFFFFE000u)
        #define SC1_RXENDA_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_RXENDA_FIXED_BIT                         (13)
        #define SC1_RXENDA_FIXED_BITS                        (19)
        /* SC_RXENDA field */
        #define SC_RXENDA                                    (0x00001FFFu)
        #define SC_RXENDA_MASK                               (0x00001FFFu)
        #define SC_RXENDA_BIT                                (0)
        #define SC_RXENDA_BITS                               (13)

#define SC1_RXBEGB                                           *((volatile uint32_t *)0x4000C808u)
#define SC1_RXBEGB_REG                                       *((volatile uint32_t *)0x4000C808u)
#define SC1_RXBEGB_ADDR                                      (0x4000C808u)
#define SC1_RXBEGB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_RXBEGB_FIXED                             (0xFFFFE000u)
        #define SC1_RXBEGB_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_RXBEGB_FIXED_BIT                         (13)
        #define SC1_RXBEGB_FIXED_BITS                        (19)
        /* SC_RXBEGB field */
        #define SC_RXBEGB                                    (0x00001FFFu)
        #define SC_RXBEGB_MASK                               (0x00001FFFu)
        #define SC_RXBEGB_BIT                                (0)
        #define SC_RXBEGB_BITS                               (13)

#define SC1_RXENDB                                           *((volatile uint32_t *)0x4000C80Cu)
#define SC1_RXENDB_REG                                       *((volatile uint32_t *)0x4000C80Cu)
#define SC1_RXENDB_ADDR                                      (0x4000C80Cu)
#define SC1_RXENDB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_RXENDB_FIXED                             (0xFFFFE000u)
        #define SC1_RXENDB_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_RXENDB_FIXED_BIT                         (13)
        #define SC1_RXENDB_FIXED_BITS                        (19)
        /* SC_RXENDB field */
        #define SC_RXENDB                                    (0x00001FFFu)
        #define SC_RXENDB_MASK                               (0x00001FFFu)
        #define SC_RXENDB_BIT                                (0)
        #define SC_RXENDB_BITS                               (13)

#define SC1_TXBEGA                                           *((volatile uint32_t *)0x4000C810u)
#define SC1_TXBEGA_REG                                       *((volatile uint32_t *)0x4000C810u)
#define SC1_TXBEGA_ADDR                                      (0x4000C810u)
#define SC1_TXBEGA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_TXBEGA_FIXED                             (0xFFFFE000u)
        #define SC1_TXBEGA_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_TXBEGA_FIXED_BIT                         (13)
        #define SC1_TXBEGA_FIXED_BITS                        (19)
        /* SC_TXBEGA field */
        #define SC_TXBEGA                                    (0x00001FFFu)
        #define SC_TXBEGA_MASK                               (0x00001FFFu)
        #define SC_TXBEGA_BIT                                (0)
        #define SC_TXBEGA_BITS                               (13)

#define SC1_TXENDA                                           *((volatile uint32_t *)0x4000C814u)
#define SC1_TXENDA_REG                                       *((volatile uint32_t *)0x4000C814u)
#define SC1_TXENDA_ADDR                                      (0x4000C814u)
#define SC1_TXENDA_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_TXENDA_FIXED                             (0xFFFFE000u)
        #define SC1_TXENDA_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_TXENDA_FIXED_BIT                         (13)
        #define SC1_TXENDA_FIXED_BITS                        (19)
        /* SC_TXENDA field */
        #define SC_TXENDA                                    (0x00001FFFu)
        #define SC_TXENDA_MASK                               (0x00001FFFu)
        #define SC_TXENDA_BIT                                (0)
        #define SC_TXENDA_BITS                               (13)

#define SC1_TXBEGB                                           *((volatile uint32_t *)0x4000C818u)
#define SC1_TXBEGB_REG                                       *((volatile uint32_t *)0x4000C818u)
#define SC1_TXBEGB_ADDR                                      (0x4000C818u)
#define SC1_TXBEGB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_TXBEGB_FIXED                             (0xFFFFE000u)
        #define SC1_TXBEGB_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_TXBEGB_FIXED_BIT                         (13)
        #define SC1_TXBEGB_FIXED_BITS                        (19)
        /* SC_TXBEGB field */
        #define SC_TXBEGB                                    (0x00001FFFu)
        #define SC_TXBEGB_MASK                               (0x00001FFFu)
        #define SC_TXBEGB_BIT                                (0)
        #define SC_TXBEGB_BITS                               (13)

#define SC1_TXENDB                                           *((volatile uint32_t *)0x4000C81Cu)
#define SC1_TXENDB_REG                                       *((volatile uint32_t *)0x4000C81Cu)
#define SC1_TXENDB_ADDR                                      (0x4000C81Cu)
#define SC1_TXENDB_RESET                                     (0x20000000u)
        /* FIXED field */
        #define SC1_TXENDB_FIXED                             (0xFFFFE000u)
        #define SC1_TXENDB_FIXED_MASK                        (0xFFFFE000u)
        #define SC1_TXENDB_FIXED_BIT                         (13)
        #define SC1_TXENDB_FIXED_BITS                        (19)
        /* SC_TXENDB field */
        #define SC_TXENDB                                    (0x00001FFFu)
        #define SC_TXENDB_MASK                               (0x00001FFFu)
        #define SC_TXENDB_BIT                                (0)
        #define SC_TXENDB_BITS                               (13)

#define SC1_RXCNTA                                           *((volatile uint32_t *)0x4000C820u)
#define SC1_RXCNTA_REG                                       *((volatile uint32_t *)0x4000C820u)
#define SC1_RXCNTA_ADDR                                      (0x4000C820u)
#define SC1_RXCNTA_RESET                                     (0x00000000u)
        /* SC_RXCNTA field */
        #define SC_RXCNTA                                    (0x00001FFFu)
        #define SC_RXCNTA_MASK                               (0x00001FFFu)
        #define SC_RXCNTA_BIT                                (0)
        #define SC_RXCNTA_BITS                               (13)

#define SC1_RXCNTB                                           *((volatile uint32_t *)0x4000C824u)
#define SC1_RXCNTB_REG                                       *((volatile uint32_t *)0x4000C824u)
#define SC1_RXCNTB_ADDR                                      (0x4000C824u)
#define SC1_RXCNTB_RESET                                     (0x00000000u)
        /* SC_RXCNTB field */
        #define SC_RXCNTB                                    (0x00001FFFu)
        #define SC_RXCNTB_MASK                               (0x00001FFFu)
        #define SC_RXCNTB_BIT                                (0)
        #define SC_RXCNTB_BITS                               (13)

#define SC1_TXCNT                                            *((volatile uint32_t *)0x4000C828u)
#define SC1_TXCNT_REG                                        *((volatile uint32_t *)0x4000C828u)
#define SC1_TXCNT_ADDR                                       (0x4000C828u)
#define SC1_TXCNT_RESET                                      (0x00000000u)
        /* SC_TXCNT field */
        #define SC_TXCNT                                     (0x00001FFFu)
        #define SC_TXCNT_MASK                                (0x00001FFFu)
        #define SC_TXCNT_BIT                                 (0)
        #define SC_TXCNT_BITS                                (13)

#define SC1_DMASTAT                                          *((volatile uint32_t *)0x4000C82Cu)
#define SC1_DMASTAT_REG                                      *((volatile uint32_t *)0x4000C82Cu)
#define SC1_DMASTAT_ADDR                                     (0x4000C82Cu)
#define SC1_DMASTAT_RESET                                    (0x00000000u)
        /* SC_RXSSEL field */
        #define SC_RXSSEL                                    (0x00001C00u)
        #define SC_RXSSEL_MASK                               (0x00001C00u)
        #define SC_RXSSEL_BIT                                (10)
        #define SC_RXSSEL_BITS                               (3)
        /* SC_RXFRMB field */
        #define SC_RXFRMB                                    (0x00000200u)
        #define SC_RXFRMB_MASK                               (0x00000200u)
        #define SC_RXFRMB_BIT                                (9)
        #define SC_RXFRMB_BITS                               (1)
        /* SC_RXFRMA field */
        #define SC_RXFRMA                                    (0x00000100u)
        #define SC_RXFRMA_MASK                               (0x00000100u)
        #define SC_RXFRMA_BIT                                (8)
        #define SC_RXFRMA_BITS                               (1)
        /* SC_RXPARB field */
        #define SC_RXPARB                                    (0x00000080u)
        #define SC_RXPARB_MASK                               (0x00000080u)
        #define SC_RXPARB_BIT                                (7)
        #define SC_RXPARB_BITS                               (1)
        /* SC_RXPARA field */
        #define SC_RXPARA                                    (0x00000040u)
        #define SC_RXPARA_MASK                               (0x00000040u)
        #define SC_RXPARA_BIT                                (6)
        #define SC_RXPARA_BITS                               (1)
        /* SC_RXOVFB field */
        #define SC_RXOVFB                                    (0x00000020u)
        #define SC_RXOVFB_MASK                               (0x00000020u)
        #define SC_RXOVFB_BIT                                (5)
        #define SC_RXOVFB_BITS                               (1)
        /* SC_RXOVFA field */
        #define SC_RXOVFA                                    (0x00000010u)
        #define SC_RXOVFA_MASK                               (0x00000010u)
        #define SC_RXOVFA_BIT                                (4)
        #define SC_RXOVFA_BITS                               (1)
        /* SC_TXACTB field */
        #define SC_TXACTB                                    (0x00000008u)
        #define SC_TXACTB_MASK                               (0x00000008u)
        #define SC_TXACTB_BIT                                (3)
        #define SC_TXACTB_BITS                               (1)
        /* SC_TXACTA field */
        #define SC_TXACTA                                    (0x00000004u)
        #define SC_TXACTA_MASK                               (0x00000004u)
        #define SC_TXACTA_BIT                                (2)
        #define SC_TXACTA_BITS                               (1)
        /* SC_RXACTB field */
        #define SC_RXACTB                                    (0x00000002u)
        #define SC_RXACTB_MASK                               (0x00000002u)
        #define SC_RXACTB_BIT                                (1)
        #define SC_RXACTB_BITS                               (1)
        /* SC_RXACTA field */
        #define SC_RXACTA                                    (0x00000001u)
        #define SC_RXACTA_MASK                               (0x00000001u)
        #define SC_RXACTA_BIT                                (0)
        #define SC_RXACTA_BITS                               (1)

#define SC1_DMACTRL                                          *((volatile uint32_t *)0x4000C830u)
#define SC1_DMACTRL_REG                                      *((volatile uint32_t *)0x4000C830u)
#define SC1_DMACTRL_ADDR                                     (0x4000C830u)
#define SC1_DMACTRL_RESET                                    (0x00000000u)
        /* SC_TXDMARST field */
        #define SC_TXDMARST                                  (0x00000020u)
        #define SC_TXDMARST_MASK                             (0x00000020u)
        #define SC_TXDMARST_BIT                              (5)
        #define SC_TXDMARST_BITS                             (1)
        /* SC_RXDMARST field */
        #define SC_RXDMARST                                  (0x00000010u)
        #define SC_RXDMARST_MASK                             (0x00000010u)
        #define SC_RXDMARST_BIT                              (4)
        #define SC_RXDMARST_BITS                             (1)
        /* SC_TXLODB field */
        #define SC_TXLODB                                    (0x00000008u)
        #define SC_TXLODB_MASK                               (0x00000008u)
        #define SC_TXLODB_BIT                                (3)
        #define SC_TXLODB_BITS                               (1)
        /* SC_TXLODA field */
        #define SC_TXLODA                                    (0x00000004u)
        #define SC_TXLODA_MASK                               (0x00000004u)
        #define SC_TXLODA_BIT                                (2)
        #define SC_TXLODA_BITS                               (1)
        /* SC_RXLODB field */
        #define SC_RXLODB                                    (0x00000002u)
        #define SC_RXLODB_MASK                               (0x00000002u)
        #define SC_RXLODB_BIT                                (1)
        #define SC_RXLODB_BITS                               (1)
        /* SC_RXLODA field */
        #define SC_RXLODA                                    (0x00000001u)
        #define SC_RXLODA_MASK                               (0x00000001u)
        #define SC_RXLODA_BIT                                (0)
        #define SC_RXLODA_BITS                               (1)

#define SC1_RXERRA                                           *((volatile uint32_t *)0x4000C834u)
#define SC1_RXERRA_REG                                       *((volatile uint32_t *)0x4000C834u)
#define SC1_RXERRA_ADDR                                      (0x4000C834u)
#define SC1_RXERRA_RESET                                     (0x00000000u)
        /* SC_RXERRA field */
        #define SC_RXERRA                                    (0x00001FFFu)
        #define SC_RXERRA_MASK                               (0x00001FFFu)
        #define SC_RXERRA_BIT                                (0)
        #define SC_RXERRA_BITS                               (13)

#define SC1_RXERRB                                           *((volatile uint32_t *)0x4000C838u)
#define SC1_RXERRB_REG                                       *((volatile uint32_t *)0x4000C838u)
#define SC1_RXERRB_ADDR                                      (0x4000C838u)
#define SC1_RXERRB_RESET                                     (0x00000000u)
        /* SC_RXERRB field */
        #define SC_RXERRB                                    (0x00001FFFu)
        #define SC_RXERRB_MASK                               (0x00001FFFu)
        #define SC_RXERRB_BIT                                (0)
        #define SC_RXERRB_BITS                               (13)

#define SC1_DATA                                             *((volatile uint32_t *)0x4000C83Cu)
#define SC1_DATA_REG                                         *((volatile uint32_t *)0x4000C83Cu)
#define SC1_DATA_ADDR                                        (0x4000C83Cu)
#define SC1_DATA_RESET                                       (0x00000000u)
        /* SC_DATA field */
        #define SC_DATA                                      (0x000000FFu)
        #define SC_DATA_MASK                                 (0x000000FFu)
        #define SC_DATA_BIT                                  (0)
        #define SC_DATA_BITS                                 (8)

#define SC1_SPISTAT                                          *((volatile uint32_t *)0x4000C840u)
#define SC1_SPISTAT_REG                                      *((volatile uint32_t *)0x4000C840u)
#define SC1_SPISTAT_ADDR                                     (0x4000C840u)
#define SC1_SPISTAT_RESET                                    (0x00000000u)
        /* SC_SPITXIDLE field */
        #define SC_SPITXIDLE                                 (0x00000008u)
        #define SC_SPITXIDLE_MASK                            (0x00000008u)
        #define SC_SPITXIDLE_BIT                             (3)
        #define SC_SPITXIDLE_BITS                            (1)
        /* SC_SPITXFREE field */
        #define SC_SPITXFREE                                 (0x00000004u)
        #define SC_SPITXFREE_MASK                            (0x00000004u)
        #define SC_SPITXFREE_BIT                             (2)
        #define SC_SPITXFREE_BITS                            (1)
        /* SC_SPIRXVAL field */
        #define SC_SPIRXVAL                                  (0x00000002u)
        #define SC_SPIRXVAL_MASK                             (0x00000002u)
        #define SC_SPIRXVAL_BIT                              (1)
        #define SC_SPIRXVAL_BITS                             (1)
        /* SC_SPIRXOVF field */
        #define SC_SPIRXOVF                                  (0x00000001u)
        #define SC_SPIRXOVF_MASK                             (0x00000001u)
        #define SC_SPIRXOVF_BIT                              (0)
        #define SC_SPIRXOVF_BITS                             (1)

#define SC1_TWISTAT                                          *((volatile uint32_t *)0x4000C844u)
#define SC1_TWISTAT_REG                                      *((volatile uint32_t *)0x4000C844u)
#define SC1_TWISTAT_ADDR                                     (0x4000C844u)
#define SC1_TWISTAT_RESET                                    (0x00000000u)
        /* SC_TWICMDFIN field */
        #define SC_TWICMDFIN                                 (0x00000008u)
        #define SC_TWICMDFIN_MASK                            (0x00000008u)
        #define SC_TWICMDFIN_BIT                             (3)
        #define SC_TWICMDFIN_BITS                            (1)
        /* SC_TWIRXFIN field */
        #define SC_TWIRXFIN                                  (0x00000004u)
        #define SC_TWIRXFIN_MASK                             (0x00000004u)
        #define SC_TWIRXFIN_BIT                              (2)
        #define SC_TWIRXFIN_BITS                             (1)
        /* SC_TWITXFIN field */
        #define SC_TWITXFIN                                  (0x00000002u)
        #define SC_TWITXFIN_MASK                             (0x00000002u)
        #define SC_TWITXFIN_BIT                              (1)
        #define SC_TWITXFIN_BITS                             (1)
        /* SC_TWIRXNAK field */
        #define SC_TWIRXNAK                                  (0x00000001u)
        #define SC_TWIRXNAK_MASK                             (0x00000001u)
        #define SC_TWIRXNAK_BIT                              (0)
        #define SC_TWIRXNAK_BITS                             (1)

#define SC1_UARTSTAT                                         *((volatile uint32_t *)0x4000C848u)
#define SC1_UARTSTAT_REG                                     *((volatile uint32_t *)0x4000C848u)
#define SC1_UARTSTAT_ADDR                                    (0x4000C848u)
#define SC1_UARTSTAT_RESET                                   (0x00000040u)
        /* SC_UARTTXIDLE field */
        #define SC_UARTTXIDLE                                (0x00000040u)
        #define SC_UARTTXIDLE_MASK                           (0x00000040u)
        #define SC_UARTTXIDLE_BIT                            (6)
        #define SC_UARTTXIDLE_BITS                           (1)
        /* SC_UARTPARERR field */
        #define SC_UARTPARERR                                (0x00000020u)
        #define SC_UARTPARERR_MASK                           (0x00000020u)
        #define SC_UARTPARERR_BIT                            (5)
        #define SC_UARTPARERR_BITS                           (1)
        /* SC_UARTFRMERR field */
        #define SC_UARTFRMERR                                (0x00000010u)
        #define SC_UARTFRMERR_MASK                           (0x00000010u)
        #define SC_UARTFRMERR_BIT                            (4)
        #define SC_UARTFRMERR_BITS                           (1)
        /* SC_UARTRXOVF field */
        #define SC_UARTRXOVF                                 (0x00000008u)
        #define SC_UARTRXOVF_MASK                            (0x00000008u)
        #define SC_UARTRXOVF_BIT                             (3)
        #define SC_UARTRXOVF_BITS                            (1)
        /* SC_UARTTXFREE field */
        #define SC_UARTTXFREE                                (0x00000004u)
        #define SC_UARTTXFREE_MASK                           (0x00000004u)
        #define SC_UARTTXFREE_BIT                            (2)
        #define SC_UARTTXFREE_BITS                           (1)
        /* SC_UARTRXVAL field */
        #define SC_UARTRXVAL                                 (0x00000002u)
        #define SC_UARTRXVAL_MASK                            (0x00000002u)
        #define SC_UARTRXVAL_BIT                             (1)
        #define SC_UARTRXVAL_BITS                            (1)
        /* SC_UARTCTS field */
        #define SC_UARTCTS                                   (0x00000001u)
        #define SC_UARTCTS_MASK                              (0x00000001u)
        #define SC_UARTCTS_BIT                               (0)
        #define SC_UARTCTS_BITS                              (1)

#define SC1_TWICTRL1                                         *((volatile uint32_t *)0x4000C84Cu)
#define SC1_TWICTRL1_REG                                     *((volatile uint32_t *)0x4000C84Cu)
#define SC1_TWICTRL1_ADDR                                    (0x4000C84Cu)
#define SC1_TWICTRL1_RESET                                   (0x00000000u)
        /* SC_TWISTOP field */
        #define SC_TWISTOP                                   (0x00000008u)
        #define SC_TWISTOP_MASK                              (0x00000008u)
        #define SC_TWISTOP_BIT                               (3)
        #define SC_TWISTOP_BITS                              (1)
        /* SC_TWISTART field */
        #define SC_TWISTART                                  (0x00000004u)
        #define SC_TWISTART_MASK                             (0x00000004u)
        #define SC_TWISTART_BIT                              (2)
        #define SC_TWISTART_BITS                             (1)
        /* SC_TWISEND field */
        #define SC_TWISEND                                   (0x00000002u)
        #define SC_TWISEND_MASK                              (0x00000002u)
        #define SC_TWISEND_BIT                               (1)
        #define SC_TWISEND_BITS                              (1)
        /* SC_TWIRECV field */
        #define SC_TWIRECV                                   (0x00000001u)
        #define SC_TWIRECV_MASK                              (0x00000001u)
        #define SC_TWIRECV_BIT                               (0)
        #define SC_TWIRECV_BITS                              (1)

#define SC1_TWICTRL2                                         *((volatile uint32_t *)0x4000C850u)
#define SC1_TWICTRL2_REG                                     *((volatile uint32_t *)0x4000C850u)
#define SC1_TWICTRL2_ADDR                                    (0x4000C850u)
#define SC1_TWICTRL2_RESET                                   (0x00000000u)
        /* SC_TWIACK field */
        #define SC_TWIACK                                    (0x00000001u)
        #define SC_TWIACK_MASK                               (0x00000001u)
        #define SC_TWIACK_BIT                                (0)
        #define SC_TWIACK_BITS                               (1)

#define SC1_MODE                                             *((volatile uint32_t *)0x4000C854u)
#define SC1_MODE_REG                                         *((volatile uint32_t *)0x4000C854u)
#define SC1_MODE_ADDR                                        (0x4000C854u)
#define SC1_MODE_RESET                                       (0x00000000u)
        /* SC_MODE field */
        #define SC_MODE                                      (0x00000003u)
        #define SC_MODE_MASK                                 (0x00000003u)
        #define SC_MODE_BIT                                  (0)
        #define SC_MODE_BITS                                 (2)
                /* SC_MODE Bit Field Values */
                #define SC1_MODE_DISABLED                    (0)
                #define SC1_MODE_UART                        (1)
                #define SC1_MODE_SPI                         (2)
                #define SC1_MODE_I2C                         (3)

#define SC1_SPICFG                                           *((volatile uint32_t *)0x4000C858u)
#define SC1_SPICFG_REG                                       *((volatile uint32_t *)0x4000C858u)
#define SC1_SPICFG_ADDR                                      (0x4000C858u)
#define SC1_SPICFG_RESET                                     (0x00000000u)
        /* SC_SPIRXDRV field */
        #define SC_SPIRXDRV                                  (0x00000020u)
        #define SC_SPIRXDRV_MASK                             (0x00000020u)
        #define SC_SPIRXDRV_BIT                              (5)
        #define SC_SPIRXDRV_BITS                             (1)
        /* SC_SPIMST field */
        #define SC_SPIMST                                    (0x00000010u)
        #define SC_SPIMST_MASK                               (0x00000010u)
        #define SC_SPIMST_BIT                                (4)
        #define SC_SPIMST_BITS                               (1)
        /* SC_SPIRPT field */
        #define SC_SPIRPT                                    (0x00000008u)
        #define SC_SPIRPT_MASK                               (0x00000008u)
        #define SC_SPIRPT_BIT                                (3)
        #define SC_SPIRPT_BITS                               (1)
        /* SC_SPIORD field */
        #define SC_SPIORD                                    (0x00000004u)
        #define SC_SPIORD_MASK                               (0x00000004u)
        #define SC_SPIORD_BIT                                (2)
        #define SC_SPIORD_BITS                               (1)
        /* SC_SPIPHA field */
        #define SC_SPIPHA                                    (0x00000002u)
        #define SC_SPIPHA_MASK                               (0x00000002u)
        #define SC_SPIPHA_BIT                                (1)
        #define SC_SPIPHA_BITS                               (1)
        /* SC_SPIPOL field */
        #define SC_SPIPOL                                    (0x00000001u)
        #define SC_SPIPOL_MASK                               (0x00000001u)
        #define SC_SPIPOL_BIT                                (0)
        #define SC_SPIPOL_BITS                               (1)

#define SC1_UARTCFG                                          *((volatile uint32_t *)0x4000C85Cu)
#define SC1_UARTCFG_REG                                      *((volatile uint32_t *)0x4000C85Cu)
#define SC1_UARTCFG_ADDR                                     (0x4000C85Cu)
#define SC1_UARTCFG_RESET                                    (0x00000000u)
        /* SC_UARTAUTO field */
        #define SC_UARTAUTO                                  (0x00000040u)
        #define SC_UARTAUTO_MASK                             (0x00000040u)
        #define SC_UARTAUTO_BIT                              (6)
        #define SC_UARTAUTO_BITS                             (1)
        /* SC_UARTFLOW field */
        #define SC_UARTFLOW                                  (0x00000020u)
        #define SC_UARTFLOW_MASK                             (0x00000020u)
        #define SC_UARTFLOW_BIT                              (5)
        #define SC_UARTFLOW_BITS                             (1)
        /* SC_UARTODD field */
        #define SC_UARTODD                                   (0x00000010u)
        #define SC_UARTODD_MASK                              (0x00000010u)
        #define SC_UARTODD_BIT                               (4)
        #define SC_UARTODD_BITS                              (1)
        /* SC_UARTPAR field */
        #define SC_UARTPAR                                   (0x00000008u)
        #define SC_UARTPAR_MASK                              (0x00000008u)
        #define SC_UARTPAR_BIT                               (3)
        #define SC_UARTPAR_BITS                              (1)
        /* SC_UART2STP field */
        #define SC_UART2STP                                  (0x00000004u)
        #define SC_UART2STP_MASK                             (0x00000004u)
        #define SC_UART2STP_BIT                              (2)
        #define SC_UART2STP_BITS                             (1)
        /* SC_UART8BIT field */
        #define SC_UART8BIT                                  (0x00000002u)
        #define SC_UART8BIT_MASK                             (0x00000002u)
        #define SC_UART8BIT_BIT                              (1)
        #define SC_UART8BIT_BITS                             (1)
        /* SC_UARTRTS field */
        #define SC_UARTRTS                                   (0x00000001u)
        #define SC_UARTRTS_MASK                              (0x00000001u)
        #define SC_UARTRTS_BIT                               (0)
        #define SC_UARTRTS_BITS                              (1)

#define SC1_RATELIN                                          *((volatile uint32_t *)0x4000C860u)
#define SC1_RATELIN_REG                                      *((volatile uint32_t *)0x4000C860u)
#define SC1_RATELIN_ADDR                                     (0x4000C860u)
#define SC1_RATELIN_RESET                                    (0x00000000u)
        /* SC_RATELIN field */
        #define SC_RATELIN                                   (0x0000000Fu)
        #define SC_RATELIN_MASK                              (0x0000000Fu)
        #define SC_RATELIN_BIT                               (0)
        #define SC_RATELIN_BITS                              (4)

#define SC1_RATEEXP                                          *((volatile uint32_t *)0x4000C864u)
#define SC1_RATEEXP_REG                                      *((volatile uint32_t *)0x4000C864u)
#define SC1_RATEEXP_ADDR                                     (0x4000C864u)
#define SC1_RATEEXP_RESET                                    (0x00000000u)
        /* SC_RATEEXP field */
        #define SC_RATEEXP                                   (0x0000000Fu)
        #define SC_RATEEXP_MASK                              (0x0000000Fu)
        #define SC_RATEEXP_BIT                               (0)
        #define SC_RATEEXP_BITS                              (4)

#define SC1_UARTPER                                          *((volatile uint32_t *)0x4000C868u)
#define SC1_UARTPER_REG                                      *((volatile uint32_t *)0x4000C868u)
#define SC1_UARTPER_ADDR                                     (0x4000C868u)
#define SC1_UARTPER_RESET                                    (0x00000000u)
        /* SC_UARTPER field */
        #define SC_UARTPER                                   (0x0000FFFFu)
        #define SC_UARTPER_MASK                              (0x0000FFFFu)
        #define SC_UARTPER_BIT                               (0)
        #define SC_UARTPER_BITS                              (16)

#define SC1_UARTFRAC                                         *((volatile uint32_t *)0x4000C86Cu)
#define SC1_UARTFRAC_REG                                     *((volatile uint32_t *)0x4000C86Cu)
#define SC1_UARTFRAC_ADDR                                    (0x4000C86Cu)
#define SC1_UARTFRAC_RESET                                   (0x00000000u)
        /* SC_UARTFRAC field */
        #define SC_UARTFRAC                                  (0x00000001u)
        #define SC_UARTFRAC_MASK                             (0x00000001u)
        #define SC_UARTFRAC_BIT                              (0)
        #define SC_UARTFRAC_BITS                             (1)

#define SC1_RXCNTSAVED                                       *((volatile uint32_t *)0x4000C870u)
#define SC1_RXCNTSAVED_REG                                   *((volatile uint32_t *)0x4000C870u)
#define SC1_RXCNTSAVED_ADDR                                  (0x4000C870u)
#define SC1_RXCNTSAVED_RESET                                 (0x00000000u)
        /* SC_RXCNTSAVED field */
        #define SC_RXCNTSAVED                                (0x00001FFFu)
        #define SC_RXCNTSAVED_MASK                           (0x00001FFFu)
        #define SC_RXCNTSAVED_BIT                            (0)
        #define SC_RXCNTSAVED_BITS                           (13)

/* ADC block */
#define BLOCK_ADC_BASE                                       (0x4000D000u)
#define BLOCK_ADC_END                                        (0x4000D024u)
#define BLOCK_ADC_SIZE                                       (BLOCK_ADC_END - BLOCK_ADC_BASE + 1)

#define ADC_DATA                                             *((volatile uint32_t *)0x4000D000u)
#define ADC_DATA_REG                                         *((volatile uint32_t *)0x4000D000u)
#define ADC_DATA_ADDR                                        (0x4000D000u)
#define ADC_DATA_RESET                                       (0x00000000u)
        /* ADC_DATA_FIELD field */
        #define ADC_DATA_FIELD                               (0x0000FFFFu)
        #define ADC_DATA_FIELD_MASK                          (0x0000FFFFu)
        #define ADC_DATA_FIELD_BIT                           (0)
        #define ADC_DATA_FIELD_BITS                          (16)

#define ADC_CFG                                              *((volatile uint32_t *)0x4000D004u)
#define ADC_CFG_REG                                          *((volatile uint32_t *)0x4000D004u)
#define ADC_CFG_ADDR                                         (0x4000D004u)
#define ADC_CFG_RESET                                        (0x00001800u)
        /* ADC_PERIOD field */
        #define ADC_PERIOD                                   (0x0000E000u)
        #define ADC_PERIOD_MASK                              (0x0000E000u)
        #define ADC_PERIOD_BIT                               (13)
        #define ADC_PERIOD_BITS                              (3)
        /* ADC_HVSELP field */
        #define ADC_HVSELP                                   (0x00001000u)
        #define ADC_HVSELP_MASK                              (0x00001000u)
        #define ADC_HVSELP_BIT                               (12)
        #define ADC_HVSELP_BITS                              (1)
        /* ADC_HVSELN field */
        #define ADC_HVSELN                                   (0x00000800u)
        #define ADC_HVSELN_MASK                              (0x00000800u)
        #define ADC_HVSELN_BIT                               (11)
        #define ADC_HVSELN_BITS                              (1)
        /* ADC_MUXP field */
        #define ADC_MUXP                                     (0x00000780u)
        #define ADC_MUXP_MASK                                (0x00000780u)
        #define ADC_MUXP_BIT                                 (7)
        #define ADC_MUXP_BITS                                (4)
        /* ADC_MUXN field */
        #define ADC_MUXN                                     (0x00000078u)
        #define ADC_MUXN_MASK                                (0x00000078u)
        #define ADC_MUXN_BIT                                 (3)
        #define ADC_MUXN_BITS                                (4)
        /* ADC_1MHZCLK field */
        #define ADC_1MHZCLK                                  (0x00000004u)
        #define ADC_1MHZCLK_MASK                             (0x00000004u)
        #define ADC_1MHZCLK_BIT                              (2)
        #define ADC_1MHZCLK_BITS                             (1)
        /* ADC_CFGRSVD field */
        #define ADC_CFGRSVD                                  (0x00000002u)
        #define ADC_CFGRSVD_MASK                             (0x00000002u)
        #define ADC_CFGRSVD_BIT                              (1)
        #define ADC_CFGRSVD_BITS                             (1)
        /* ADC_ENABLE field */
        #define ADC_ENABLE                                   (0x00000001u)
        #define ADC_ENABLE_MASK                              (0x00000001u)
        #define ADC_ENABLE_BIT                               (0)
        #define ADC_ENABLE_BITS                              (1)

#define ADC_OFFSET                                           *((volatile uint32_t *)0x4000D008u)
#define ADC_OFFSET_REG                                       *((volatile uint32_t *)0x4000D008u)
#define ADC_OFFSET_ADDR                                      (0x4000D008u)
#define ADC_OFFSET_RESET                                     (0x00000000u)
        /* ADC_OFFSET_FIELD field */
        #define ADC_OFFSET_FIELD                             (0x0000FFFFu)
        #define ADC_OFFSET_FIELD_MASK                        (0x0000FFFFu)
        #define ADC_OFFSET_FIELD_BIT                         (0)
        #define ADC_OFFSET_FIELD_BITS                        (16)

#define ADC_GAIN                                             *((volatile uint32_t *)0x4000D00Cu)
#define ADC_GAIN_REG                                         *((volatile uint32_t *)0x4000D00Cu)
#define ADC_GAIN_ADDR                                        (0x4000D00Cu)
#define ADC_GAIN_RESET                                       (0x00008000u)
        /* ADC_GAIN_FIELD field */
        #define ADC_GAIN_FIELD                               (0x0000FFFFu)
        #define ADC_GAIN_FIELD_MASK                          (0x0000FFFFu)
        #define ADC_GAIN_FIELD_BIT                           (0)
        #define ADC_GAIN_FIELD_BITS                          (16)

#define ADC_DMACFG                                           *((volatile uint32_t *)0x4000D010u)
#define ADC_DMACFG_REG                                       *((volatile uint32_t *)0x4000D010u)
#define ADC_DMACFG_ADDR                                      (0x4000D010u)
#define ADC_DMACFG_RESET                                     (0x00000000u)
        /* ADC_DMARST field */
        #define ADC_DMARST                                   (0x00000010u)
        #define ADC_DMARST_MASK                              (0x00000010u)
        #define ADC_DMARST_BIT                               (4)
        #define ADC_DMARST_BITS                              (1)
        /* ADC_DMAAUTOWRAP field */
        #define ADC_DMAAUTOWRAP                              (0x00000002u)
        #define ADC_DMAAUTOWRAP_MASK                         (0x00000002u)
        #define ADC_DMAAUTOWRAP_BIT                          (1)
        #define ADC_DMAAUTOWRAP_BITS                         (1)
        /* ADC_DMALOAD field */
        #define ADC_DMALOAD                                  (0x00000001u)
        #define ADC_DMALOAD_MASK                             (0x00000001u)
        #define ADC_DMALOAD_BIT                              (0)
        #define ADC_DMALOAD_BITS                             (1)

#define ADC_DMASTAT                                          *((volatile uint32_t *)0x4000D014u)
#define ADC_DMASTAT_REG                                      *((volatile uint32_t *)0x4000D014u)
#define ADC_DMASTAT_ADDR                                     (0x4000D014u)
#define ADC_DMASTAT_RESET                                    (0x00000000u)
        /* ADC_DMAOVF field */
        #define ADC_DMAOVF                                   (0x00000002u)
        #define ADC_DMAOVF_MASK                              (0x00000002u)
        #define ADC_DMAOVF_BIT                               (1)
        #define ADC_DMAOVF_BITS                              (1)
        /* ADC_DMAACT field */
        #define ADC_DMAACT                                   (0x00000001u)
        #define ADC_DMAACT_MASK                              (0x00000001u)
        #define ADC_DMAACT_BIT                               (0)
        #define ADC_DMAACT_BITS                              (1)

#define ADC_DMABEG                                           *((volatile uint32_t *)0x4000D018u)
#define ADC_DMABEG_REG                                       *((volatile uint32_t *)0x4000D018u)
#define ADC_DMABEG_ADDR                                      (0x4000D018u)
#define ADC_DMABEG_RESET                                     (0x20000000u)
        /* ADC_DMABEG_FIXED field */
        #define ADC_DMABEG_FIXED                             (0xFFFFE000u)
        #define ADC_DMABEG_FIXED_MASK                        (0xFFFFE000u)
        #define ADC_DMABEG_FIXED_BIT                         (13)
        #define ADC_DMABEG_FIXED_BITS                        (19)
        /* ADC_DMABEG_FIELD field */
        #define ADC_DMABEG_FIELD                             (0x00001FFFu)
        #define ADC_DMABEG_FIELD_MASK                        (0x00001FFFu)
        #define ADC_DMABEG_FIELD_BIT                         (0)
        #define ADC_DMABEG_FIELD_BITS                        (13)

#define ADC_DMASIZE                                          *((volatile uint32_t *)0x4000D01Cu)
#define ADC_DMASIZE_REG                                      *((volatile uint32_t *)0x4000D01Cu)
#define ADC_DMASIZE_ADDR                                     (0x4000D01Cu)
#define ADC_DMASIZE_RESET                                    (0x00000000u)
        /* ADC_DMASIZE_FIELD field */
        #define ADC_DMASIZE_FIELD                            (0x00000FFFu)
        #define ADC_DMASIZE_FIELD_MASK                       (0x00000FFFu)
        #define ADC_DMASIZE_FIELD_BIT                        (0)
        #define ADC_DMASIZE_FIELD_BITS                       (12)

#define ADC_DMACUR                                           *((volatile uint32_t *)0x4000D020u)
#define ADC_DMACUR_REG                                       *((volatile uint32_t *)0x4000D020u)
#define ADC_DMACUR_ADDR                                      (0x4000D020u)
#define ADC_DMACUR_RESET                                     (0x20000000u)
        /* ADC_DMACUR_FIXED field */
        #define ADC_DMACUR_FIXED                             (0xFFFFE000u)
        #define ADC_DMACUR_FIXED_MASK                        (0xFFFFE000u)
        #define ADC_DMACUR_FIXED_BIT                         (13)
        #define ADC_DMACUR_FIXED_BITS                        (19)
        /* ADC_DMACUR_FIELD field */
        #define ADC_DMACUR_FIELD                             (0x00001FFFu)
        #define ADC_DMACUR_FIELD_MASK                        (0x00001FFFu)
        #define ADC_DMACUR_FIELD_BIT                         (0)
        #define ADC_DMACUR_FIELD_BITS                        (13)

#define ADC_DMACNT                                           *((volatile uint32_t *)0x4000D024u)
#define ADC_DMACNT_REG                                       *((volatile uint32_t *)0x4000D024u)
#define ADC_DMACNT_ADDR                                      (0x4000D024u)
#define ADC_DMACNT_RESET                                     (0x00000000u)
        /* ADC_DMACNT_FIELD field */
        #define ADC_DMACNT_FIELD                             (0x00000FFFu)
        #define ADC_DMACNT_FIELD_MASK                        (0x00000FFFu)
        #define ADC_DMACNT_FIELD_BIT                         (0)
        #define ADC_DMACNT_FIELD_BITS                        (12)

/* TIM1 block */
#define BLOCK_TIM1_BASE                                      (0x4000E000u)
#define BLOCK_TIM1_END                                       (0x4000E050u)
#define BLOCK_TIM1_SIZE                                      (BLOCK_TIM1_END - BLOCK_TIM1_BASE + 1)

#define TIM1_CR1                                             *((volatile uint32_t *)0x4000E000u)
#define TIM1_CR1_REG                                         *((volatile uint32_t *)0x4000E000u)
#define TIM1_CR1_ADDR                                        (0x4000E000u)
#define TIM1_CR1_RESET                                       (0x00000000u)
        /* TIM_ARBE field */
        #define TIM_ARBE                                     (0x00000080u)
        #define TIM_ARBE_MASK                                (0x00000080u)
        #define TIM_ARBE_BIT                                 (7)
        #define TIM_ARBE_BITS                                (1)
        /* TIM_CMS field */
        #define TIM_CMS                                      (0x00000060u)
        #define TIM_CMS_MASK                                 (0x00000060u)
        #define TIM_CMS_BIT                                  (5)
        #define TIM_CMS_BITS                                 (2)
        /* TIM_DIR field */
        #define TIM_DIR                                      (0x00000010u)
        #define TIM_DIR_MASK                                 (0x00000010u)
        #define TIM_DIR_BIT                                  (4)
        #define TIM_DIR_BITS                                 (1)
        /* TIM_OPM field */
        #define TIM_OPM                                      (0x00000008u)
        #define TIM_OPM_MASK                                 (0x00000008u)
        #define TIM_OPM_BIT                                  (3)
        #define TIM_OPM_BITS                                 (1)
        /* TIM_URS field */
        #define TIM_URS                                      (0x00000004u)
        #define TIM_URS_MASK                                 (0x00000004u)
        #define TIM_URS_BIT                                  (2)
        #define TIM_URS_BITS                                 (1)
        /* TIM_UDIS field */
        #define TIM_UDIS                                     (0x00000002u)
        #define TIM_UDIS_MASK                                (0x00000002u)
        #define TIM_UDIS_BIT                                 (1)
        #define TIM_UDIS_BITS                                (1)
        /* TIM_CEN field */
        #define TIM_CEN                                      (0x00000001u)
        #define TIM_CEN_MASK                                 (0x00000001u)
        #define TIM_CEN_BIT                                  (0)
        #define TIM_CEN_BITS                                 (1)

#define TIM1_CR2                                             *((volatile uint32_t *)0x4000E004u)
#define TIM1_CR2_REG                                         *((volatile uint32_t *)0x4000E004u)
#define TIM1_CR2_ADDR                                        (0x4000E004u)
#define TIM1_CR2_RESET                                       (0x00000000u)
        /* TIM_TI1S field */
        #define TIM_TI1S                                     (0x00000080u)
        #define TIM_TI1S_MASK                                (0x00000080u)
        #define TIM_TI1S_BIT                                 (7)
        #define TIM_TI1S_BITS                                (1)
        /* TIM_MMS field */
        #define TIM_MMS                                      (0x00000070u)
        #define TIM_MMS_MASK                                 (0x00000070u)
        #define TIM_MMS_BIT                                  (4)
        #define TIM_MMS_BITS                                 (3)

#define TIM1_SMCR                                            *((volatile uint32_t *)0x4000E008u)
#define TIM1_SMCR_REG                                        *((volatile uint32_t *)0x4000E008u)
#define TIM1_SMCR_ADDR                                       (0x4000E008u)
#define TIM1_SMCR_RESET                                      (0x00000000u)
        /* TIM_ETP field */
        #define TIM_ETP                                      (0x00008000u)
        #define TIM_ETP_MASK                                 (0x00008000u)
        #define TIM_ETP_BIT                                  (15)
        #define TIM_ETP_BITS                                 (1)
        /* TIM_ECE field */
        #define TIM_ECE                                      (0x00004000u)
        #define TIM_ECE_MASK                                 (0x00004000u)
        #define TIM_ECE_BIT                                  (14)
        #define TIM_ECE_BITS                                 (1)
        /* TIM_ETPS field */
        #define TIM_ETPS                                     (0x00003000u)
        #define TIM_ETPS_MASK                                (0x00003000u)
        #define TIM_ETPS_BIT                                 (12)
        #define TIM_ETPS_BITS                                (2)
        /* TIM_ETF field */
        #define TIM_ETF                                      (0x00000F00u)
        #define TIM_ETF_MASK                                 (0x00000F00u)
        #define TIM_ETF_BIT                                  (8)
        #define TIM_ETF_BITS                                 (4)
        /* TIM_MSM field */
        #define TIM_MSM                                      (0x00000080u)
        #define TIM_MSM_MASK                                 (0x00000080u)
        #define TIM_MSM_BIT                                  (7)
        #define TIM_MSM_BITS                                 (1)
        /* TIM_TS field */
        #define TIM_TS                                       (0x00000070u)
        #define TIM_TS_MASK                                  (0x00000070u)
        #define TIM_TS_BIT                                   (4)
        #define TIM_TS_BITS                                  (3)
        /* TIM_SMS field */
        #define TIM_SMS                                      (0x00000007u)
        #define TIM_SMS_MASK                                 (0x00000007u)
        #define TIM_SMS_BIT                                  (0)
        #define TIM_SMS_BITS                                 (3)

#define TMR1_DIER                                            *((volatile uint32_t *)0x4000E00Cu)
#define TMR1_DIER_REG                                        *((volatile uint32_t *)0x4000E00Cu)
#define TMR1_DIER_ADDR                                       (0x4000E00Cu)
#define TMR1_DIER_RESET                                      (0x00000000u)
        /* TIE field */
        #define TMR1_DIER_TIE                                (0x00000040u)
        #define TMR1_DIER_TIE_MASK                           (0x00000040u)
        #define TMR1_DIER_TIE_BIT                            (6)
        #define TMR1_DIER_TIE_BITS                           (1)
        /* CC4IE field */
        #define TMR1_DIER_CC4IE                              (0x00000010u)
        #define TMR1_DIER_CC4IE_MASK                         (0x00000010u)
        #define TMR1_DIER_CC4IE_BIT                          (4)
        #define TMR1_DIER_CC4IE_BITS                         (1)
        /* CC3IE field */
        #define TMR1_DIER_CC3IE                              (0x00000008u)
        #define TMR1_DIER_CC3IE_MASK                         (0x00000008u)
        #define TMR1_DIER_CC3IE_BIT                          (3)
        #define TMR1_DIER_CC3IE_BITS                         (1)
        /* CC2IE field */
        #define TMR1_DIER_CC2IE                              (0x00000004u)
        #define TMR1_DIER_CC2IE_MASK                         (0x00000004u)
        #define TMR1_DIER_CC2IE_BIT                          (2)
        #define TMR1_DIER_CC2IE_BITS                         (1)
        /* CC1IE field */
        #define TMR1_DIER_CC1IE                              (0x00000002u)
        #define TMR1_DIER_CC1IE_MASK                         (0x00000002u)
        #define TMR1_DIER_CC1IE_BIT                          (1)
        #define TMR1_DIER_CC1IE_BITS                         (1)
        /* UIE field */
        #define TMR1_DIER_UIE                                (0x00000001u)
        #define TMR1_DIER_UIE_MASK                           (0x00000001u)
        #define TMR1_DIER_UIE_BIT                            (0)
        #define TMR1_DIER_UIE_BITS                           (1)

#define TMR1_SR                                              *((volatile uint32_t *)0x4000E010u)
#define TMR1_SR_REG                                          *((volatile uint32_t *)0x4000E010u)
#define TMR1_SR_ADDR                                         (0x4000E010u)
#define TMR1_SR_RESET                                        (0x00000000u)
        /* CC4OF field */
        #define TMR1_SR_CC4OF                                (0x00001000u)
        #define TMR1_SR_CC4OF_MASK                           (0x00001000u)
        #define TMR1_SR_CC4OF_BIT                            (12)
        #define TMR1_SR_CC4OF_BITS                           (1)
        /* CC3OF field */
        #define TMR1_SR_CC3OF                                (0x00000800u)
        #define TMR1_SR_CC3OF_MASK                           (0x00000800u)
        #define TMR1_SR_CC3OF_BIT                            (11)
        #define TMR1_SR_CC3OF_BITS                           (1)
        /* CC2OF field */
        #define TMR1_SR_CC2OF                                (0x00000400u)
        #define TMR1_SR_CC2OF_MASK                           (0x00000400u)
        #define TMR1_SR_CC2OF_BIT                            (10)
        #define TMR1_SR_CC2OF_BITS                           (1)
        /* CC1OF field */
        #define TMR1_SR_CC1OF                                (0x00000200u)
        #define TMR1_SR_CC1OF_MASK                           (0x00000200u)
        #define TMR1_SR_CC1OF_BIT                            (9)
        #define TMR1_SR_CC1OF_BITS                           (1)
        /* TIF field */
        #define TMR1_SR_TIF                                  (0x00000040u)
        #define TMR1_SR_TIF_MASK                             (0x00000040u)
        #define TMR1_SR_TIF_BIT                              (6)
        #define TMR1_SR_TIF_BITS                             (1)
        /* CC4IF field */
        #define TMR1_SR_CC4IF                                (0x00000010u)
        #define TMR1_SR_CC4IF_MASK                           (0x00000010u)
        #define TMR1_SR_CC4IF_BIT                            (4)
        #define TMR1_SR_CC4IF_BITS                           (1)
        /* CC3IF field */
        #define TMR1_SR_CC3IF                                (0x00000008u)
        #define TMR1_SR_CC3IF_MASK                           (0x00000008u)
        #define TMR1_SR_CC3IF_BIT                            (3)
        #define TMR1_SR_CC3IF_BITS                           (1)
        /* CC2IF field */
        #define TMR1_SR_CC2IF                                (0x00000004u)
        #define TMR1_SR_CC2IF_MASK                           (0x00000004u)
        #define TMR1_SR_CC2IF_BIT                            (2)
        #define TMR1_SR_CC2IF_BITS                           (1)
        /* CC1IF field */
        #define TMR1_SR_CC1IF                                (0x00000002u)
        #define TMR1_SR_CC1IF_MASK                           (0x00000002u)
        #define TMR1_SR_CC1IF_BIT                            (1)
        #define TMR1_SR_CC1IF_BITS                           (1)
        /* UIF field */
        #define TMR1_SR_UIF                                  (0x00000001u)
        #define TMR1_SR_UIF_MASK                             (0x00000001u)
        #define TMR1_SR_UIF_BIT                              (0)
        #define TMR1_SR_UIF_BITS                             (1)

#define TIM1_EGR                                             *((volatile uint32_t *)0x4000E014u)
#define TIM1_EGR_REG                                         *((volatile uint32_t *)0x4000E014u)
#define TIM1_EGR_ADDR                                        (0x4000E014u)
#define TIM1_EGR_RESET                                       (0x00000000u)
        /* TIM_TG field */
        #define TIM_TG                                       (0x00000040u)
        #define TIM_TG_MASK                                  (0x00000040u)
        #define TIM_TG_BIT                                   (6)
        #define TIM_TG_BITS                                  (1)
        /* TIM_CC4G field */
        #define TIM_CC4G                                     (0x00000010u)
        #define TIM_CC4G_MASK                                (0x00000010u)
        #define TIM_CC4G_BIT                                 (4)
        #define TIM_CC4G_BITS                                (1)
        /* TIM_CC3G field */
        #define TIM_CC3G                                     (0x00000008u)
        #define TIM_CC3G_MASK                                (0x00000008u)
        #define TIM_CC3G_BIT                                 (3)
        #define TIM_CC3G_BITS                                (1)
        /* TIM_CC2G field */
        #define TIM_CC2G                                     (0x00000004u)
        #define TIM_CC2G_MASK                                (0x00000004u)
        #define TIM_CC2G_BIT                                 (2)
        #define TIM_CC2G_BITS                                (1)
        /* TIM_CC1G field */
        #define TIM_CC1G                                     (0x00000002u)
        #define TIM_CC1G_MASK                                (0x00000002u)
        #define TIM_CC1G_BIT                                 (1)
        #define TIM_CC1G_BITS                                (1)
        /* TIM_UG field */
        #define TIM_UG                                       (0x00000001u)
        #define TIM_UG_MASK                                  (0x00000001u)
        #define TIM_UG_BIT                                   (0)
        #define TIM_UG_BITS                                  (1)

#define TIM1_CCMR1                                           *((volatile uint32_t *)0x4000E018u)
#define TIM1_CCMR1_REG                                       *((volatile uint32_t *)0x4000E018u)
#define TIM1_CCMR1_ADDR                                      (0x4000E018u)
#define TIM1_CCMR1_RESET                                     (0x00000000u)
        /* TIM_IC2F field */
        #define TIM_IC2F                                     (0x0000F000u)
        #define TIM_IC2F_MASK                                (0x0000F000u)
        #define TIM_IC2F_BIT                                 (12)
        #define TIM_IC2F_BITS                                (4)
        /* TIM_IC2PSC field */
        #define TIM_IC2PSC                                   (0x00000C00u)
        #define TIM_IC2PSC_MASK                              (0x00000C00u)
        #define TIM_IC2PSC_BIT                               (10)
        #define TIM_IC2PSC_BITS                              (2)
        /* TIM_IC1F field */
        #define TIM_IC1F                                     (0x000000F0u)
        #define TIM_IC1F_MASK                                (0x000000F0u)
        #define TIM_IC1F_BIT                                 (4)
        #define TIM_IC1F_BITS                                (4)
        /* TIM_IC1PSC field */
        #define TIM_IC1PSC                                   (0x0000000Cu)
        #define TIM_IC1PSC_MASK                              (0x0000000Cu)
        #define TIM_IC1PSC_BIT                               (2)
        #define TIM_IC1PSC_BITS                              (2)
        /* TIM_OC2CE field */
        #define TIM_OC2CE                                    (0x00008000u)
        #define TIM_OC2CE_MASK                               (0x00008000u)
        #define TIM_OC2CE_BIT                                (15)
        #define TIM_OC2CE_BITS                               (1)
        /* TIM_OC2M field */
        #define TIM_OC2M                                     (0x00007000u)
        #define TIM_OC2M_MASK                                (0x00007000u)
        #define TIM_OC2M_BIT                                 (12)
        #define TIM_OC2M_BITS                                (3)
        /* TIM_OC2BE field */
        #define TIM_OC2BE                                    (0x00000800u)
        #define TIM_OC2BE_MASK                               (0x00000800u)
        #define TIM_OC2BE_BIT                                (11)
        #define TIM_OC2BE_BITS                               (1)
        /* TIM_OC2FE field */
        #define TIM_OC2FE                                    (0x00000400u)
        #define TIM_OC2FE_MASK                               (0x00000400u)
        #define TIM_OC2FE_BIT                                (10)
        #define TIM_OC2FE_BITS                               (1)
        /* TIM_CC2S field */
        #define TIM_CC2S                                     (0x00000300u)
        #define TIM_CC2S_MASK                                (0x00000300u)
        #define TIM_CC2S_BIT                                 (8)
        #define TIM_CC2S_BITS                                (2)
        /* TIM_OC1CE field */
        #define TIM_OC1CE                                    (0x00000080u)
        #define TIM_OC1CE_MASK                               (0x00000080u)
        #define TIM_OC1CE_BIT                                (7)
        #define TIM_OC1CE_BITS                               (1)
        /* TIM_OC1M field */
        #define TIM_OC1M                                     (0x00000070u)
        #define TIM_OC1M_MASK                                (0x00000070u)
        #define TIM_OC1M_BIT                                 (4)
        #define TIM_OC1M_BITS                                (3)
        /* TIM_OC1PE field */
        #define TIM_OC1PE                                    (0x00000008u)
        #define TIM_OC1PE_MASK                               (0x00000008u)
        #define TIM_OC1PE_BIT                                (3)
        #define TIM_OC1PE_BITS                               (1)
        /* TIM_OC1FE field */
        #define TIM_OC1FE                                    (0x00000004u)
        #define TIM_OC1FE_MASK                               (0x00000004u)
        #define TIM_OC1FE_BIT                                (2)
        #define TIM_OC1FE_BITS                               (1)
        /* TIM_CC1S field */
        #define TIM_CC1S                                     (0x00000003u)
        #define TIM_CC1S_MASK                                (0x00000003u)
        #define TIM_CC1S_BIT                                 (0)
        #define TIM_CC1S_BITS                                (2)

#define TIM1_CCMR2                                           *((volatile uint32_t *)0x4000E01Cu)
#define TIM1_CCMR2_REG                                       *((volatile uint32_t *)0x4000E01Cu)
#define TIM1_CCMR2_ADDR                                      (0x4000E01Cu)
#define TIM1_CCMR2_RESET                                     (0x00000000u)
        /* TIM_IC4F field */
        #define TIM_IC4F                                     (0x0000F000u)
        #define TIM_IC4F_MASK                                (0x0000F000u)
        #define TIM_IC4F_BIT                                 (12)
        #define TIM_IC4F_BITS                                (4)
        /* TIM_IC4PSC field */
        #define TIM_IC4PSC                                   (0x00000C00u)
        #define TIM_IC4PSC_MASK                              (0x00000C00u)
        #define TIM_IC4PSC_BIT                               (10)
        #define TIM_IC4PSC_BITS                              (2)
        /* TIM_IC3F field */
        #define TIM_IC3F                                     (0x000000F0u)
        #define TIM_IC3F_MASK                                (0x000000F0u)
        #define TIM_IC3F_BIT                                 (4)
        #define TIM_IC3F_BITS                                (4)
        /* TIM_IC3PSC field */
        #define TIM_IC3PSC                                   (0x0000000Cu)
        #define TIM_IC3PSC_MASK                              (0x0000000Cu)
        #define TIM_IC3PSC_BIT                               (2)
        #define TIM_IC3PSC_BITS                              (2)
        /* TIM_OC4CE field */
        #define TIM_OC4CE                                    (0x00008000u)
        #define TIM_OC4CE_MASK                               (0x00008000u)
        #define TIM_OC4CE_BIT                                (15)
        #define TIM_OC4CE_BITS                               (1)
        /* TIM_OC4M field */
        #define TIM_OC4M                                     (0x00007000u)
        #define TIM_OC4M_MASK                                (0x00007000u)
        #define TIM_OC4M_BIT                                 (12)
        #define TIM_OC4M_BITS                                (3)
        /* TIM_OC4BE field */
        #define TIM_OC4BE                                    (0x00000800u)
        #define TIM_OC4BE_MASK                               (0x00000800u)
        #define TIM_OC4BE_BIT                                (11)
        #define TIM_OC4BE_BITS                               (1)
        /* TIM_OC4FE field */
        #define TIM_OC4FE                                    (0x00000400u)
        #define TIM_OC4FE_MASK                               (0x00000400u)
        #define TIM_OC4FE_BIT                                (10)
        #define TIM_OC4FE_BITS                               (1)
        /* TIM_CC4S field */
        #define TIM_CC4S                                     (0x00000300u)
        #define TIM_CC4S_MASK                                (0x00000300u)
        #define TIM_CC4S_BIT                                 (8)
        #define TIM_CC4S_BITS                                (2)
        /* TIM_OC3CE field */
        #define TIM_OC3CE                                    (0x00000080u)
        #define TIM_OC3CE_MASK                               (0x00000080u)
        #define TIM_OC3CE_BIT                                (7)
        #define TIM_OC3CE_BITS                               (1)
        /* TIM_OC3M field */
        #define TIM_OC3M                                     (0x00000070u)
        #define TIM_OC3M_MASK                                (0x00000070u)
        #define TIM_OC3M_BIT                                 (4)
        #define TIM_OC3M_BITS                                (3)
        /* TIM_OC3BE field */
        #define TIM_OC3BE                                    (0x00000008u)
        #define TIM_OC3BE_MASK                               (0x00000008u)
        #define TIM_OC3BE_BIT                                (3)
        #define TIM_OC3BE_BITS                               (1)
        /* TIM_OC3FE field */
        #define TIM_OC3FE                                    (0x00000004u)
        #define TIM_OC3FE_MASK                               (0x00000004u)
        #define TIM_OC3FE_BIT                                (2)
        #define TIM_OC3FE_BITS                               (1)
        /* TIM_CC3S field */
        #define TIM_CC3S                                     (0x00000003u)
        #define TIM_CC3S_MASK                                (0x00000003u)
        #define TIM_CC3S_BIT                                 (0)
        #define TIM_CC3S_BITS                                (2)

#define TIM1_CCER                                            *((volatile uint32_t *)0x4000E020u)
#define TIM1_CCER_REG                                        *((volatile uint32_t *)0x4000E020u)
#define TIM1_CCER_ADDR                                       (0x4000E020u)
#define TIM1_CCER_RESET                                      (0x00000000u)
        /* TIM_CC4P field */
        #define TIM_CC4P                                     (0x00002000u)
        #define TIM_CC4P_MASK                                (0x00002000u)
        #define TIM_CC4P_BIT                                 (13)
        #define TIM_CC4P_BITS                                (1)
        /* TIM_CC4E field */
        #define TIM_CC4E                                     (0x00001000u)
        #define TIM_CC4E_MASK                                (0x00001000u)
        #define TIM_CC4E_BIT                                 (12)
        #define TIM_CC4E_BITS                                (1)
        /* TIM_CC3P field */
        #define TIM_CC3P                                     (0x00000200u)
        #define TIM_CC3P_MASK                                (0x00000200u)
        #define TIM_CC3P_BIT                                 (9)
        #define TIM_CC3P_BITS                                (1)
        /* TIM_CC3E field */
        #define TIM_CC3E                                     (0x00000100u)
        #define TIM_CC3E_MASK                                (0x00000100u)
        #define TIM_CC3E_BIT                                 (8)
        #define TIM_CC3E_BITS                                (1)
        /* TIM_CC2P field */
        #define TIM_CC2P                                     (0x00000020u)
        #define TIM_CC2P_MASK                                (0x00000020u)
        #define TIM_CC2P_BIT                                 (5)
        #define TIM_CC2P_BITS                                (1)
        /* TIM_CC2E field */
        #define TIM_CC2E                                     (0x00000010u)
        #define TIM_CC2E_MASK                                (0x00000010u)
        #define TIM_CC2E_BIT                                 (4)
        #define TIM_CC2E_BITS                                (1)
        /* TIM_CC1P field */
        #define TIM_CC1P                                     (0x00000002u)
        #define TIM_CC1P_MASK                                (0x00000002u)
        #define TIM_CC1P_BIT                                 (1)
        #define TIM_CC1P_BITS                                (1)
        /* TIM_CC1E field */
        #define TIM_CC1E                                     (0x00000001u)
        #define TIM_CC1E_MASK                                (0x00000001u)
        #define TIM_CC1E_BIT                                 (0)
        #define TIM_CC1E_BITS                                (1)

#define TIM1_CNT                                             *((volatile uint32_t *)0x4000E024u)
#define TIM1_CNT_REG                                         *((volatile uint32_t *)0x4000E024u)
#define TIM1_CNT_ADDR                                        (0x4000E024u)
#define TIM1_CNT_RESET                                       (0x00000000u)
        /* TIM_CNT field */
        #define TIM_CNT                                      (0x0000FFFFu)
        #define TIM_CNT_MASK                                 (0x0000FFFFu)
        #define TIM_CNT_BIT                                  (0)
        #define TIM_CNT_BITS                                 (16)

#define TIM1_PSC                                             *((volatile uint32_t *)0x4000E028u)
#define TIM1_PSC_REG                                         *((volatile uint32_t *)0x4000E028u)
#define TIM1_PSC_ADDR                                        (0x4000E028u)
#define TIM1_PSC_RESET                                       (0x00000000u)
        /* TIM_PSC field */
        #define TIM_PSC                                      (0x0000000Fu)
        #define TIM_PSC_MASK                                 (0x0000000Fu)
        #define TIM_PSC_BIT                                  (0)
        #define TIM_PSC_BITS                                 (4)

#define TIM1_ARR                                             *((volatile uint32_t *)0x4000E02Cu)
#define TIM1_ARR_REG                                         *((volatile uint32_t *)0x4000E02Cu)
#define TIM1_ARR_ADDR                                        (0x4000E02Cu)
#define TIM1_ARR_RESET                                       (0x0000FFFFu)
        /* TIM_ARR field */
        #define TIM_ARR                                      (0x0000FFFFu)
        #define TIM_ARR_MASK                                 (0x0000FFFFu)
        #define TIM_ARR_BIT                                  (0)
        #define TIM_ARR_BITS                                 (16)

#define TIM1_CCR1                                            *((volatile uint32_t *)0x4000E034u)
#define TIM1_CCR1_REG                                        *((volatile uint32_t *)0x4000E034u)
#define TIM1_CCR1_ADDR                                       (0x4000E034u)
#define TIM1_CCR1_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM1_CCR2                                            *((volatile uint32_t *)0x4000E038u)
#define TIM1_CCR2_REG                                        *((volatile uint32_t *)0x4000E038u)
#define TIM1_CCR2_ADDR                                       (0x4000E038u)
#define TIM1_CCR2_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM1_CCR3                                            *((volatile uint32_t *)0x4000E03Cu)
#define TIM1_CCR3_REG                                        *((volatile uint32_t *)0x4000E03Cu)
#define TIM1_CCR3_ADDR                                       (0x4000E03Cu)
#define TIM1_CCR3_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM1_CCR4                                            *((volatile uint32_t *)0x4000E040u)
#define TIM1_CCR4_REG                                        *((volatile uint32_t *)0x4000E040u)
#define TIM1_CCR4_ADDR                                       (0x4000E040u)
#define TIM1_CCR4_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM1_OR                                              *((volatile uint32_t *)0x4000E050u)
#define TIM1_OR_REG                                          *((volatile uint32_t *)0x4000E050u)
#define TIM1_OR_ADDR                                         (0x4000E050u)
#define TIM1_OR_RESET                                        (0x00000000u)
        /* TIM_ORRSVD field */
        #define TIM_ORRSVD                                   (0x00000008u)
        #define TIM_ORRSVD_MASK                              (0x00000008u)
        #define TIM_ORRSVD_BIT                               (3)
        #define TIM_ORRSVD_BITS                              (1)
        /* TIM_CLKMSKEN field */
        #define TIM_CLKMSKEN                                 (0x00000004u)
        #define TIM_CLKMSKEN_MASK                            (0x00000004u)
        #define TIM_CLKMSKEN_BIT                             (2)
        #define TIM_CLKMSKEN_BITS                            (1)
        /* TIM1_EXTRIGSEL field */
        #define TIM1_EXTRIGSEL                               (0x00000003u)
        #define TIM1_EXTRIGSEL_MASK                          (0x00000003u)
        #define TIM1_EXTRIGSEL_BIT                           (0)
        #define TIM1_EXTRIGSEL_BITS                          (2)

/* TIM2 block */
#define BLOCK_TIM2_BASE                                      (0x4000F000u)
#define BLOCK_TIM2_END                                       (0x4000F050u)
#define BLOCK_TIM2_SIZE                                      (BLOCK_TIM2_END - BLOCK_TIM2_BASE + 1)

#define TIM2_CR1                                             *((volatile uint32_t *)0x4000F000u)
#define TIM2_CR1_REG                                         *((volatile uint32_t *)0x4000F000u)
#define TIM2_CR1_ADDR                                        (0x4000F000u)
#define TIM2_CR1_RESET                                       (0x00000000u)
        /* TIM_ARBE field */
        #define TIM_ARBE                                     (0x00000080u)
        #define TIM_ARBE_MASK                                (0x00000080u)
        #define TIM_ARBE_BIT                                 (7)
        #define TIM_ARBE_BITS                                (1)
        /* TIM_CMS field */
        #define TIM_CMS                                      (0x00000060u)
        #define TIM_CMS_MASK                                 (0x00000060u)
        #define TIM_CMS_BIT                                  (5)
        #define TIM_CMS_BITS                                 (2)
        /* TIM_DIR field */
        #define TIM_DIR                                      (0x00000010u)
        #define TIM_DIR_MASK                                 (0x00000010u)
        #define TIM_DIR_BIT                                  (4)
        #define TIM_DIR_BITS                                 (1)
        /* TIM_OPM field */
        #define TIM_OPM                                      (0x00000008u)
        #define TIM_OPM_MASK                                 (0x00000008u)
        #define TIM_OPM_BIT                                  (3)
        #define TIM_OPM_BITS                                 (1)
        /* TIM_URS field */
        #define TIM_URS                                      (0x00000004u)
        #define TIM_URS_MASK                                 (0x00000004u)
        #define TIM_URS_BIT                                  (2)
        #define TIM_URS_BITS                                 (1)
        /* TIM_UDIS field */
        #define TIM_UDIS                                     (0x00000002u)
        #define TIM_UDIS_MASK                                (0x00000002u)
        #define TIM_UDIS_BIT                                 (1)
        #define TIM_UDIS_BITS                                (1)
        /* TIM_CEN field */
        #define TIM_CEN                                      (0x00000001u)
        #define TIM_CEN_MASK                                 (0x00000001u)
        #define TIM_CEN_BIT                                  (0)
        #define TIM_CEN_BITS                                 (1)

#define TIM2_CR2                                             *((volatile uint32_t *)0x4000F004u)
#define TIM2_CR2_REG                                         *((volatile uint32_t *)0x4000F004u)
#define TIM2_CR2_ADDR                                        (0x4000F004u)
#define TIM2_CR2_RESET                                       (0x00000000u)
        /* TIM_TI1S field */
        #define TIM_TI1S                                     (0x00000080u)
        #define TIM_TI1S_MASK                                (0x00000080u)
        #define TIM_TI1S_BIT                                 (7)
        #define TIM_TI1S_BITS                                (1)
        /* TIM_MMS field */
        #define TIM_MMS                                      (0x00000070u)
        #define TIM_MMS_MASK                                 (0x00000070u)
        #define TIM_MMS_BIT                                  (4)
        #define TIM_MMS_BITS                                 (3)

#define TIM2_SMCR                                            *((volatile uint32_t *)0x4000F008u)
#define TIM2_SMCR_REG                                        *((volatile uint32_t *)0x4000F008u)
#define TIM2_SMCR_ADDR                                       (0x4000F008u)
#define TIM2_SMCR_RESET                                      (0x00000000u)
        /* TIM_ETP field */
        #define TIM_ETP                                      (0x00008000u)
        #define TIM_ETP_MASK                                 (0x00008000u)
        #define TIM_ETP_BIT                                  (15)
        #define TIM_ETP_BITS                                 (1)
        /* TIM_ECE field */
        #define TIM_ECE                                      (0x00004000u)
        #define TIM_ECE_MASK                                 (0x00004000u)
        #define TIM_ECE_BIT                                  (14)
        #define TIM_ECE_BITS                                 (1)
        /* TIM_ETPS field */
        #define TIM_ETPS                                     (0x00003000u)
        #define TIM_ETPS_MASK                                (0x00003000u)
        #define TIM_ETPS_BIT                                 (12)
        #define TIM_ETPS_BITS                                (2)
        /* TIM_ETF field */
        #define TIM_ETF                                      (0x00000F00u)
        #define TIM_ETF_MASK                                 (0x00000F00u)
        #define TIM_ETF_BIT                                  (8)
        #define TIM_ETF_BITS                                 (4)
        /* TIM_MSM field */
        #define TIM_MSM                                      (0x00000080u)
        #define TIM_MSM_MASK                                 (0x00000080u)
        #define TIM_MSM_BIT                                  (7)
        #define TIM_MSM_BITS                                 (1)
        /* TIM_TS field */
        #define TIM_TS                                       (0x00000070u)
        #define TIM_TS_MASK                                  (0x00000070u)
        #define TIM_TS_BIT                                   (4)
        #define TIM_TS_BITS                                  (3)
        /* TIM_SMS field */
        #define TIM_SMS                                      (0x00000007u)
        #define TIM_SMS_MASK                                 (0x00000007u)
        #define TIM_SMS_BIT                                  (0)
        #define TIM_SMS_BITS                                 (3)

#define TMR2_DIER                                            *((volatile uint32_t *)0x4000F00Cu)
#define TMR2_DIER_REG                                        *((volatile uint32_t *)0x4000F00Cu)
#define TMR2_DIER_ADDR                                       (0x4000F00Cu)
#define TMR2_DIER_RESET                                      (0x00000000u)
        /* TIE field */
        #define TMR2_DIER_TIE                                (0x00000040u)
        #define TMR2_DIER_TIE_MASK                           (0x00000040u)
        #define TMR2_DIER_TIE_BIT                            (6)
        #define TMR2_DIER_TIE_BITS                           (1)
        /* CC4IE field */
        #define TMR2_DIER_CC4IE                              (0x00000010u)
        #define TMR2_DIER_CC4IE_MASK                         (0x00000010u)
        #define TMR2_DIER_CC4IE_BIT                          (4)
        #define TMR2_DIER_CC4IE_BITS                         (1)
        /* CC3IE field */
        #define TMR2_DIER_CC3IE                              (0x00000008u)
        #define TMR2_DIER_CC3IE_MASK                         (0x00000008u)
        #define TMR2_DIER_CC3IE_BIT                          (3)
        #define TMR2_DIER_CC3IE_BITS                         (1)
        /* CC2IE field */
        #define TMR2_DIER_CC2IE                              (0x00000004u)
        #define TMR2_DIER_CC2IE_MASK                         (0x00000004u)
        #define TMR2_DIER_CC2IE_BIT                          (2)
        #define TMR2_DIER_CC2IE_BITS                         (1)
        /* CC1IE field */
        #define TMR2_DIER_CC1IE                              (0x00000002u)
        #define TMR2_DIER_CC1IE_MASK                         (0x00000002u)
        #define TMR2_DIER_CC1IE_BIT                          (1)
        #define TMR2_DIER_CC1IE_BITS                         (1)
        /* UIE field */
        #define TMR2_DIER_UIE                                (0x00000001u)
        #define TMR2_DIER_UIE_MASK                           (0x00000001u)
        #define TMR2_DIER_UIE_BIT                            (0)
        #define TMR2_DIER_UIE_BITS                           (1)

#define TMR2_SR                                              *((volatile uint32_t *)0x4000F010u)
#define TMR2_SR_REG                                          *((volatile uint32_t *)0x4000F010u)
#define TMR2_SR_ADDR                                         (0x4000F010u)
#define TMR2_SR_RESET                                        (0x00000000u)
        /* CC4OF field */
        #define TMR2_SR_CC4OF                                (0x00001000u)
        #define TMR2_SR_CC4OF_MASK                           (0x00001000u)
        #define TMR2_SR_CC4OF_BIT                            (12)
        #define TMR2_SR_CC4OF_BITS                           (1)
        /* CC3OF field */
        #define TMR2_SR_CC3OF                                (0x00000800u)
        #define TMR2_SR_CC3OF_MASK                           (0x00000800u)
        #define TMR2_SR_CC3OF_BIT                            (11)
        #define TMR2_SR_CC3OF_BITS                           (1)
        /* CC2OF field */
        #define TMR2_SR_CC2OF                                (0x00000400u)
        #define TMR2_SR_CC2OF_MASK                           (0x00000400u)
        #define TMR2_SR_CC2OF_BIT                            (10)
        #define TMR2_SR_CC2OF_BITS                           (1)
        /* CC1OF field */
        #define TMR2_SR_CC1OF                                (0x00000200u)
        #define TMR2_SR_CC1OF_MASK                           (0x00000200u)
        #define TMR2_SR_CC1OF_BIT                            (9)
        #define TMR2_SR_CC1OF_BITS                           (1)
        /* TIF field */
        #define TMR2_SR_TIF                                  (0x00000040u)
        #define TMR2_SR_TIF_MASK                             (0x00000040u)
        #define TMR2_SR_TIF_BIT                              (6)
        #define TMR2_SR_TIF_BITS                             (1)
        /* CC4IF field */
        #define TMR2_SR_CC4IF                                (0x00000010u)
        #define TMR2_SR_CC4IF_MASK                           (0x00000010u)
        #define TMR2_SR_CC4IF_BIT                            (4)
        #define TMR2_SR_CC4IF_BITS                           (1)
        /* CC3IF field */
        #define TMR2_SR_CC3IF                                (0x00000008u)
        #define TMR2_SR_CC3IF_MASK                           (0x00000008u)
        #define TMR2_SR_CC3IF_BIT                            (3)
        #define TMR2_SR_CC3IF_BITS                           (1)
        /* CC2IF field */
        #define TMR2_SR_CC2IF                                (0x00000004u)
        #define TMR2_SR_CC2IF_MASK                           (0x00000004u)
        #define TMR2_SR_CC2IF_BIT                            (2)
        #define TMR2_SR_CC2IF_BITS                           (1)
        /* CC1IF field */
        #define TMR2_SR_CC1IF                                (0x00000002u)
        #define TMR2_SR_CC1IF_MASK                           (0x00000002u)
        #define TMR2_SR_CC1IF_BIT                            (1)
        #define TMR2_SR_CC1IF_BITS                           (1)
        /* UIF field */
        #define TMR2_SR_UIF                                  (0x00000001u)
        #define TMR2_SR_UIF_MASK                             (0x00000001u)
        #define TMR2_SR_UIF_BIT                              (0)
        #define TMR2_SR_UIF_BITS                             (1)

#define TIM2_EGR                                             *((volatile uint32_t *)0x4000F014u)
#define TIM2_EGR_REG                                         *((volatile uint32_t *)0x4000F014u)
#define TIM2_EGR_ADDR                                        (0x4000F014u)
#define TIM2_EGR_RESET                                       (0x00000000u)
        /* TIM_TG field */
        #define TIM_TG                                       (0x00000040u)
        #define TIM_TG_MASK                                  (0x00000040u)
        #define TIM_TG_BIT                                   (6)
        #define TIM_TG_BITS                                  (1)
        /* TIM_CC4G field */
        #define TIM_CC4G                                     (0x00000010u)
        #define TIM_CC4G_MASK                                (0x00000010u)
        #define TIM_CC4G_BIT                                 (4)
        #define TIM_CC4G_BITS                                (1)
        /* TIM_CC3G field */
        #define TIM_CC3G                                     (0x00000008u)
        #define TIM_CC3G_MASK                                (0x00000008u)
        #define TIM_CC3G_BIT                                 (3)
        #define TIM_CC3G_BITS                                (1)
        /* TIM_CC2G field */
        #define TIM_CC2G                                     (0x00000004u)
        #define TIM_CC2G_MASK                                (0x00000004u)
        #define TIM_CC2G_BIT                                 (2)
        #define TIM_CC2G_BITS                                (1)
        /* TIM_CC1G field */
        #define TIM_CC1G                                     (0x00000002u)
        #define TIM_CC1G_MASK                                (0x00000002u)
        #define TIM_CC1G_BIT                                 (1)
        #define TIM_CC1G_BITS                                (1)
        /* TIM_UG field */
        #define TIM_UG                                       (0x00000001u)
        #define TIM_UG_MASK                                  (0x00000001u)
        #define TIM_UG_BIT                                   (0)
        #define TIM_UG_BITS                                  (1)

#define TIM2_CCMR1                                           *((volatile uint32_t *)0x4000F018u)
#define TIM2_CCMR1_REG                                       *((volatile uint32_t *)0x4000F018u)
#define TIM2_CCMR1_ADDR                                      (0x4000F018u)
#define TIM2_CCMR1_RESET                                     (0x00000000u)
        /* TIM_IC2F field */
        #define TIM_IC2F                                     (0x0000F000u)
        #define TIM_IC2F_MASK                                (0x0000F000u)
        #define TIM_IC2F_BIT                                 (12)
        #define TIM_IC2F_BITS                                (4)
        /* TIM_IC2PSC field */
        #define TIM_IC2PSC                                   (0x00000C00u)
        #define TIM_IC2PSC_MASK                              (0x00000C00u)
        #define TIM_IC2PSC_BIT                               (10)
        #define TIM_IC2PSC_BITS                              (2)
        /* TIM_IC1F field */
        #define TIM_IC1F                                     (0x000000F0u)
        #define TIM_IC1F_MASK                                (0x000000F0u)
        #define TIM_IC1F_BIT                                 (4)
        #define TIM_IC1F_BITS                                (4)
        /* TIM_IC1PSC field */
        #define TIM_IC1PSC                                   (0x0000000Cu)
        #define TIM_IC1PSC_MASK                              (0x0000000Cu)
        #define TIM_IC1PSC_BIT                               (2)
        #define TIM_IC1PSC_BITS                              (2)
        /* TIM_OC2CE field */
        #define TIM_OC2CE                                    (0x00008000u)
        #define TIM_OC2CE_MASK                               (0x00008000u)
        #define TIM_OC2CE_BIT                                (15)
        #define TIM_OC2CE_BITS                               (1)
        /* TIM_OC2M field */
        #define TIM_OC2M                                     (0x00007000u)
        #define TIM_OC2M_MASK                                (0x00007000u)
        #define TIM_OC2M_BIT                                 (12)
        #define TIM_OC2M_BITS                                (3)
        /* TIM_OC2BE field */
        #define TIM_OC2BE                                    (0x00000800u)
        #define TIM_OC2BE_MASK                               (0x00000800u)
        #define TIM_OC2BE_BIT                                (11)
        #define TIM_OC2BE_BITS                               (1)
        /* TIM_OC2FE field */
        #define TIM_OC2FE                                    (0x00000400u)
        #define TIM_OC2FE_MASK                               (0x00000400u)
        #define TIM_OC2FE_BIT                                (10)
        #define TIM_OC2FE_BITS                               (1)
        /* TIM_CC2S field */
        #define TIM_CC2S                                     (0x00000300u)
        #define TIM_CC2S_MASK                                (0x00000300u)
        #define TIM_CC2S_BIT                                 (8)
        #define TIM_CC2S_BITS                                (2)
        /* TIM_OC1CE field */
        #define TIM_OC1CE                                    (0x00000080u)
        #define TIM_OC1CE_MASK                               (0x00000080u)
        #define TIM_OC1CE_BIT                                (7)
        #define TIM_OC1CE_BITS                               (1)
        /* TIM_OC1M field */
        #define TIM_OC1M                                     (0x00000070u)
        #define TIM_OC1M_MASK                                (0x00000070u)
        #define TIM_OC1M_BIT                                 (4)
        #define TIM_OC1M_BITS                                (3)
        /* TIM_OC1PE field */
        #define TIM_OC1PE                                    (0x00000008u)
        #define TIM_OC1PE_MASK                               (0x00000008u)
        #define TIM_OC1PE_BIT                                (3)
        #define TIM_OC1PE_BITS                               (1)
        /* TIM_OC1FE field */
        #define TIM_OC1FE                                    (0x00000004u)
        #define TIM_OC1FE_MASK                               (0x00000004u)
        #define TIM_OC1FE_BIT                                (2)
        #define TIM_OC1FE_BITS                               (1)
        /* TIM_CC1S field */
        #define TIM_CC1S                                     (0x00000003u)
        #define TIM_CC1S_MASK                                (0x00000003u)
        #define TIM_CC1S_BIT                                 (0)
        #define TIM_CC1S_BITS                                (2)

#define TIM2_CCMR2                                           *((volatile uint32_t *)0x4000F01Cu)
#define TIM2_CCMR2_REG                                       *((volatile uint32_t *)0x4000F01Cu)
#define TIM2_CCMR2_ADDR                                      (0x4000F01Cu)
#define TIM2_CCMR2_RESET                                     (0x00000000u)
        /* TIM_IC4F field */
        #define TIM_IC4F                                     (0x0000F000u)
        #define TIM_IC4F_MASK                                (0x0000F000u)
        #define TIM_IC4F_BIT                                 (12)
        #define TIM_IC4F_BITS                                (4)
        /* TIM_IC4PSC field */
        #define TIM_IC4PSC                                   (0x00000C00u)
        #define TIM_IC4PSC_MASK                              (0x00000C00u)
        #define TIM_IC4PSC_BIT                               (10)
        #define TIM_IC4PSC_BITS                              (2)
        /* TIM_IC3F field */
        #define TIM_IC3F                                     (0x000000F0u)
        #define TIM_IC3F_MASK                                (0x000000F0u)
        #define TIM_IC3F_BIT                                 (4)
        #define TIM_IC3F_BITS                                (4)
        /* TIM_IC3PSC field */
        #define TIM_IC3PSC                                   (0x0000000Cu)
        #define TIM_IC3PSC_MASK                              (0x0000000Cu)
        #define TIM_IC3PSC_BIT                               (2)
        #define TIM_IC3PSC_BITS                              (2)
        /* TIM_OC4CE field */
        #define TIM_OC4CE                                    (0x00008000u)
        #define TIM_OC4CE_MASK                               (0x00008000u)
        #define TIM_OC4CE_BIT                                (15)
        #define TIM_OC4CE_BITS                               (1)
        /* TIM_OC4M field */
        #define TIM_OC4M                                     (0x00007000u)
        #define TIM_OC4M_MASK                                (0x00007000u)
        #define TIM_OC4M_BIT                                 (12)
        #define TIM_OC4M_BITS                                (3)
        /* TIM_OC4BE field */
        #define TIM_OC4BE                                    (0x00000800u)
        #define TIM_OC4BE_MASK                               (0x00000800u)
        #define TIM_OC4BE_BIT                                (11)
        #define TIM_OC4BE_BITS                               (1)
        /* TIM_OC4FE field */
        #define TIM_OC4FE                                    (0x00000400u)
        #define TIM_OC4FE_MASK                               (0x00000400u)
        #define TIM_OC4FE_BIT                                (10)
        #define TIM_OC4FE_BITS                               (1)
        /* TIM_CC4S field */
        #define TIM_CC4S                                     (0x00000300u)
        #define TIM_CC4S_MASK                                (0x00000300u)
        #define TIM_CC4S_BIT                                 (8)
        #define TIM_CC4S_BITS                                (2)
        /* TIM_OC3CE field */
        #define TIM_OC3CE                                    (0x00000080u)
        #define TIM_OC3CE_MASK                               (0x00000080u)
        #define TIM_OC3CE_BIT                                (7)
        #define TIM_OC3CE_BITS                               (1)
        /* TIM_OC3M field */
        #define TIM_OC3M                                     (0x00000070u)
        #define TIM_OC3M_MASK                                (0x00000070u)
        #define TIM_OC3M_BIT                                 (4)
        #define TIM_OC3M_BITS                                (3)
        /* TIM_OC3BE field */
        #define TIM_OC3BE                                    (0x00000008u)
        #define TIM_OC3BE_MASK                               (0x00000008u)
        #define TIM_OC3BE_BIT                                (3)
        #define TIM_OC3BE_BITS                               (1)
        /* TIM_OC3FE field */
        #define TIM_OC3FE                                    (0x00000004u)
        #define TIM_OC3FE_MASK                               (0x00000004u)
        #define TIM_OC3FE_BIT                                (2)
        #define TIM_OC3FE_BITS                               (1)
        /* TIM_CC3S field */
        #define TIM_CC3S                                     (0x00000003u)
        #define TIM_CC3S_MASK                                (0x00000003u)
        #define TIM_CC3S_BIT                                 (0)
        #define TIM_CC3S_BITS                                (2)

#define TIM2_CCER                                            *((volatile uint32_t *)0x4000F020u)
#define TIM2_CCER_REG                                        *((volatile uint32_t *)0x4000F020u)
#define TIM2_CCER_ADDR                                       (0x4000F020u)
#define TIM2_CCER_RESET                                      (0x00000000u)
        /* TIM_CC4P field */
        #define TIM_CC4P                                     (0x00002000u)
        #define TIM_CC4P_MASK                                (0x00002000u)
        #define TIM_CC4P_BIT                                 (13)
        #define TIM_CC4P_BITS                                (1)
        /* TIM_CC4E field */
        #define TIM_CC4E                                     (0x00001000u)
        #define TIM_CC4E_MASK                                (0x00001000u)
        #define TIM_CC4E_BIT                                 (12)
        #define TIM_CC4E_BITS                                (1)
        /* TIM_CC3P field */
        #define TIM_CC3P                                     (0x00000200u)
        #define TIM_CC3P_MASK                                (0x00000200u)
        #define TIM_CC3P_BIT                                 (9)
        #define TIM_CC3P_BITS                                (1)
        /* TIM_CC3E field */
        #define TIM_CC3E                                     (0x00000100u)
        #define TIM_CC3E_MASK                                (0x00000100u)
        #define TIM_CC3E_BIT                                 (8)
        #define TIM_CC3E_BITS                                (1)
        /* TIM_CC2P field */
        #define TIM_CC2P                                     (0x00000020u)
        #define TIM_CC2P_MASK                                (0x00000020u)
        #define TIM_CC2P_BIT                                 (5)
        #define TIM_CC2P_BITS                                (1)
        /* TIM_CC2E field */
        #define TIM_CC2E                                     (0x00000010u)
        #define TIM_CC2E_MASK                                (0x00000010u)
        #define TIM_CC2E_BIT                                 (4)
        #define TIM_CC2E_BITS                                (1)
        /* TIM_CC1P field */
        #define TIM_CC1P                                     (0x00000002u)
        #define TIM_CC1P_MASK                                (0x00000002u)
        #define TIM_CC1P_BIT                                 (1)
        #define TIM_CC1P_BITS                                (1)
        /* TIM_CC1E field */
        #define TIM_CC1E                                     (0x00000001u)
        #define TIM_CC1E_MASK                                (0x00000001u)
        #define TIM_CC1E_BIT                                 (0)
        #define TIM_CC1E_BITS                                (1)

#define TIM2_CNT                                             *((volatile uint32_t *)0x4000F024u)
#define TIM2_CNT_REG                                         *((volatile uint32_t *)0x4000F024u)
#define TIM2_CNT_ADDR                                        (0x4000F024u)
#define TIM2_CNT_RESET                                       (0x00000000u)
        /* TIM_CNT field */
        #define TIM_CNT                                      (0x0000FFFFu)
        #define TIM_CNT_MASK                                 (0x0000FFFFu)
        #define TIM_CNT_BIT                                  (0)
        #define TIM_CNT_BITS                                 (16)

#define TIM2_PSC                                             *((volatile uint32_t *)0x4000F028u)
#define TIM2_PSC_REG                                         *((volatile uint32_t *)0x4000F028u)
#define TIM2_PSC_ADDR                                        (0x4000F028u)
#define TIM2_PSC_RESET                                       (0x00000000u)
        /* TIM_PSC field */
        #define TIM_PSC                                      (0x0000000Fu)
        #define TIM_PSC_MASK                                 (0x0000000Fu)
        #define TIM_PSC_BIT                                  (0)
        #define TIM_PSC_BITS                                 (4)

#define TIM2_ARR                                             *((volatile uint32_t *)0x4000F02Cu)
#define TIM2_ARR_REG                                         *((volatile uint32_t *)0x4000F02Cu)
#define TIM2_ARR_ADDR                                        (0x4000F02Cu)
#define TIM2_ARR_RESET                                       (0x0000FFFFu)
        /* TIM_ARR field */
        #define TIM_ARR                                      (0x0000FFFFu)
        #define TIM_ARR_MASK                                 (0x0000FFFFu)
        #define TIM_ARR_BIT                                  (0)
        #define TIM_ARR_BITS                                 (16)

#define TIM2_CCR1                                            *((volatile uint32_t *)0x4000F034u)
#define TIM2_CCR1_REG                                        *((volatile uint32_t *)0x4000F034u)
#define TIM2_CCR1_ADDR                                       (0x4000F034u)
#define TIM2_CCR1_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM2_CCR2                                            *((volatile uint32_t *)0x4000F038u)
#define TIM2_CCR2_REG                                        *((volatile uint32_t *)0x4000F038u)
#define TIM2_CCR2_ADDR                                       (0x4000F038u)
#define TIM2_CCR2_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM2_CCR3                                            *((volatile uint32_t *)0x4000F03Cu)
#define TIM2_CCR3_REG                                        *((volatile uint32_t *)0x4000F03Cu)
#define TIM2_CCR3_ADDR                                       (0x4000F03Cu)
#define TIM2_CCR3_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM2_CCR4                                            *((volatile uint32_t *)0x4000F040u)
#define TIM2_CCR4_REG                                        *((volatile uint32_t *)0x4000F040u)
#define TIM2_CCR4_ADDR                                       (0x4000F040u)
#define TIM2_CCR4_RESET                                      (0x00000000u)
        /* TIM_CCR field */
        #define TIM_CCR                                      (0x0000FFFFu)
        #define TIM_CCR_MASK                                 (0x0000FFFFu)
        #define TIM_CCR_BIT                                  (0)
        #define TIM_CCR_BITS                                 (16)

#define TIM2_OR                                              *((volatile uint32_t *)0x4000F050u)
#define TIM2_OR_REG                                          *((volatile uint32_t *)0x4000F050u)
#define TIM2_OR_ADDR                                         (0x4000F050u)
#define TIM2_OR_RESET                                        (0x00000000u)
        /* TIM_REMAPC4 field */
        #define TIM_REMAPC4                                  (0x00000080u)
        #define TIM_REMAPC4_MASK                             (0x00000080u)
        #define TIM_REMAPC4_BIT                              (7)
        #define TIM_REMAPC4_BITS                             (1)
        /* TIM_REMAPC3 field */
        #define TIM_REMAPC3                                  (0x00000040u)
        #define TIM_REMAPC3_MASK                             (0x00000040u)
        #define TIM_REMAPC3_BIT                              (6)
        #define TIM_REMAPC3_BITS                             (1)
        /* TIM_REMAPC2 field */
        #define TIM_REMAPC2                                  (0x00000020u)
        #define TIM_REMAPC2_MASK                             (0x00000020u)
        #define TIM_REMAPC2_BIT                              (5)
        #define TIM_REMAPC2_BITS                             (1)
        /* TIM_REMAPC1 field */
        #define TIM_REMAPC1                                  (0x00000010u)
        #define TIM_REMAPC1_MASK                             (0x00000010u)
        #define TIM_REMAPC1_BIT                              (4)
        #define TIM_REMAPC1_BITS                             (1)
        /* TIM_ORRSVD field */
        #define TIM_ORRSVD                                   (0x00000008u)
        #define TIM_ORRSVD_MASK                              (0x00000008u)
        #define TIM_ORRSVD_BIT                               (3)
        #define TIM_ORRSVD_BITS                              (1)
        /* TIM_CLKMSKEN field */
        #define TIM_CLKMSKEN                                 (0x00000004u)
        #define TIM_CLKMSKEN_MASK                            (0x00000004u)
        #define TIM_CLKMSKEN_BIT                             (2)
        #define TIM_CLKMSKEN_BITS                            (1)
        /* TIM1_EXTRIGSEL field */
        #define TIM1_EXTRIGSEL                               (0x00000003u)
        #define TIM1_EXTRIGSEL_MASK                          (0x00000003u)
        #define TIM1_EXTRIGSEL_BIT                           (0)
        #define TIM1_EXTRIGSEL_BITS                          (2)

/* EXT_RAM block */
#define DATA_EXT_RAM_BASE                                    (0x60000000u)
#define DATA_EXT_RAM_END                                     (0x9FFFFFFFu)
#define DATA_EXT_RAM_SIZE                                    (DATA_EXT_RAM_END - DATA_EXT_RAM_BASE + 1)

/* EXT_DEVICE block */
#define DATA_EXT_DEVICE_BASE                                 (0xA0000000u)
#define DATA_EXT_DEVICE_END                                  (0xDFFFFFFFu)
#define DATA_EXT_DEVICE_SIZE                                 (DATA_EXT_DEVICE_END - DATA_EXT_DEVICE_BASE + 1)

/* ITM block */
#define DATA_ITM_BASE                                        (0xE0000000u)
#define DATA_ITM_END                                         (0xE0000FFFu)
#define DATA_ITM_SIZE                                        (DATA_ITM_END - DATA_ITM_BASE + 1)

#define ITM_SP0                                              *((volatile uint32_t *)0xE0000000u)
#define ITM_SP0_REG                                          *((volatile uint32_t *)0xE0000000u)
#define ITM_SP0_ADDR                                         (0xE0000000u)
#define ITM_SP0_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP0_FIFOREADY                            (0x00000001u)
        #define ITM_SP0_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP0_FIFOREADY_BIT                        (0)
        #define ITM_SP0_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP0_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP0_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP0_STIMULUS_BIT                         (0)
        #define ITM_SP0_STIMULUS_BITS                        (32)

#define ITM_SP1                                              *((volatile uint32_t *)0xE0000004u)
#define ITM_SP1_REG                                          *((volatile uint32_t *)0xE0000004u)
#define ITM_SP1_ADDR                                         (0xE0000004u)
#define ITM_SP1_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP1_FIFOREADY                            (0x00000001u)
        #define ITM_SP1_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP1_FIFOREADY_BIT                        (0)
        #define ITM_SP1_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP1_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP1_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP1_STIMULUS_BIT                         (0)
        #define ITM_SP1_STIMULUS_BITS                        (32)

#define ITM_SP2                                              *((volatile uint32_t *)0xE0000008u)
#define ITM_SP2_REG                                          *((volatile uint32_t *)0xE0000008u)
#define ITM_SP2_ADDR                                         (0xE0000008u)
#define ITM_SP2_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP2_FIFOREADY                            (0x00000001u)
        #define ITM_SP2_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP2_FIFOREADY_BIT                        (0)
        #define ITM_SP2_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP2_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP2_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP2_STIMULUS_BIT                         (0)
        #define ITM_SP2_STIMULUS_BITS                        (32)

#define ITM_SP3                                              *((volatile uint32_t *)0xE000000Cu)
#define ITM_SP3_REG                                          *((volatile uint32_t *)0xE000000Cu)
#define ITM_SP3_ADDR                                         (0xE000000Cu)
#define ITM_SP3_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP3_FIFOREADY                            (0x00000001u)
        #define ITM_SP3_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP3_FIFOREADY_BIT                        (0)
        #define ITM_SP3_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP3_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP3_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP3_STIMULUS_BIT                         (0)
        #define ITM_SP3_STIMULUS_BITS                        (32)

#define ITM_SP4                                              *((volatile uint32_t *)0xE0000010u)
#define ITM_SP4_REG                                          *((volatile uint32_t *)0xE0000010u)
#define ITM_SP4_ADDR                                         (0xE0000010u)
#define ITM_SP4_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP4_FIFOREADY                            (0x00000001u)
        #define ITM_SP4_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP4_FIFOREADY_BIT                        (0)
        #define ITM_SP4_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP4_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP4_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP4_STIMULUS_BIT                         (0)
        #define ITM_SP4_STIMULUS_BITS                        (32)

#define ITM_SP5                                              *((volatile uint32_t *)0xE0000014u)
#define ITM_SP5_REG                                          *((volatile uint32_t *)0xE0000014u)
#define ITM_SP5_ADDR                                         (0xE0000014u)
#define ITM_SP5_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP5_FIFOREADY                            (0x00000001u)
        #define ITM_SP5_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP5_FIFOREADY_BIT                        (0)
        #define ITM_SP5_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP5_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP5_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP5_STIMULUS_BIT                         (0)
        #define ITM_SP5_STIMULUS_BITS                        (32)

#define ITM_SP6                                              *((volatile uint32_t *)0xE0000018u)
#define ITM_SP6_REG                                          *((volatile uint32_t *)0xE0000018u)
#define ITM_SP6_ADDR                                         (0xE0000018u)
#define ITM_SP6_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP6_FIFOREADY                            (0x00000001u)
        #define ITM_SP6_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP6_FIFOREADY_BIT                        (0)
        #define ITM_SP6_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP6_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP6_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP6_STIMULUS_BIT                         (0)
        #define ITM_SP6_STIMULUS_BITS                        (32)

#define ITM_SP7                                              *((volatile uint32_t *)0xE000001Cu)
#define ITM_SP7_REG                                          *((volatile uint32_t *)0xE000001Cu)
#define ITM_SP7_ADDR                                         (0xE000001Cu)
#define ITM_SP7_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP7_FIFOREADY                            (0x00000001u)
        #define ITM_SP7_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP7_FIFOREADY_BIT                        (0)
        #define ITM_SP7_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP7_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP7_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP7_STIMULUS_BIT                         (0)
        #define ITM_SP7_STIMULUS_BITS                        (32)

#define ITM_SP8                                              *((volatile uint32_t *)0xE0000020u)
#define ITM_SP8_REG                                          *((volatile uint32_t *)0xE0000020u)
#define ITM_SP8_ADDR                                         (0xE0000020u)
#define ITM_SP8_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP8_FIFOREADY                            (0x00000001u)
        #define ITM_SP8_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP8_FIFOREADY_BIT                        (0)
        #define ITM_SP8_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP8_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP8_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP8_STIMULUS_BIT                         (0)
        #define ITM_SP8_STIMULUS_BITS                        (32)

#define ITM_SP9                                              *((volatile uint32_t *)0xE0000024u)
#define ITM_SP9_REG                                          *((volatile uint32_t *)0xE0000024u)
#define ITM_SP9_ADDR                                         (0xE0000024u)
#define ITM_SP9_RESET                                        (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP9_FIFOREADY                            (0x00000001u)
        #define ITM_SP9_FIFOREADY_MASK                       (0x00000001u)
        #define ITM_SP9_FIFOREADY_BIT                        (0)
        #define ITM_SP9_FIFOREADY_BITS                       (1)
        /* STIMULUS field */
        #define ITM_SP9_STIMULUS                             (0xFFFFFFFFu)
        #define ITM_SP9_STIMULUS_MASK                        (0xFFFFFFFFu)
        #define ITM_SP9_STIMULUS_BIT                         (0)
        #define ITM_SP9_STIMULUS_BITS                        (32)

#define ITM_SP10                                             *((volatile uint32_t *)0xE0000028u)
#define ITM_SP10_REG                                         *((volatile uint32_t *)0xE0000028u)
#define ITM_SP10_ADDR                                        (0xE0000028u)
#define ITM_SP10_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP10_FIFOREADY                           (0x00000001u)
        #define ITM_SP10_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP10_FIFOREADY_BIT                       (0)
        #define ITM_SP10_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP10_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP10_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP10_STIMULUS_BIT                        (0)
        #define ITM_SP10_STIMULUS_BITS                       (32)

#define ITM_SP11                                             *((volatile uint32_t *)0xE000002Cu)
#define ITM_SP11_REG                                         *((volatile uint32_t *)0xE000002Cu)
#define ITM_SP11_ADDR                                        (0xE000002Cu)
#define ITM_SP11_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP11_FIFOREADY                           (0x00000001u)
        #define ITM_SP11_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP11_FIFOREADY_BIT                       (0)
        #define ITM_SP11_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP11_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP11_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP11_STIMULUS_BIT                        (0)
        #define ITM_SP11_STIMULUS_BITS                       (32)

#define ITM_SP12                                             *((volatile uint32_t *)0xE0000030u)
#define ITM_SP12_REG                                         *((volatile uint32_t *)0xE0000030u)
#define ITM_SP12_ADDR                                        (0xE0000030u)
#define ITM_SP12_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP12_FIFOREADY                           (0x00000001u)
        #define ITM_SP12_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP12_FIFOREADY_BIT                       (0)
        #define ITM_SP12_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP12_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP12_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP12_STIMULUS_BIT                        (0)
        #define ITM_SP12_STIMULUS_BITS                       (32)

#define ITM_SP13                                             *((volatile uint32_t *)0xE0000034u)
#define ITM_SP13_REG                                         *((volatile uint32_t *)0xE0000034u)
#define ITM_SP13_ADDR                                        (0xE0000034u)
#define ITM_SP13_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP13_FIFOREADY                           (0x00000001u)
        #define ITM_SP13_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP13_FIFOREADY_BIT                       (0)
        #define ITM_SP13_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP13_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP13_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP13_STIMULUS_BIT                        (0)
        #define ITM_SP13_STIMULUS_BITS                       (32)

#define ITM_SP14                                             *((volatile uint32_t *)0xE0000038u)
#define ITM_SP14_REG                                         *((volatile uint32_t *)0xE0000038u)
#define ITM_SP14_ADDR                                        (0xE0000038u)
#define ITM_SP14_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP14_FIFOREADY                           (0x00000001u)
        #define ITM_SP14_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP14_FIFOREADY_BIT                       (0)
        #define ITM_SP14_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP14_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP14_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP14_STIMULUS_BIT                        (0)
        #define ITM_SP14_STIMULUS_BITS                       (32)

#define ITM_SP15                                             *((volatile uint32_t *)0xE000003Cu)
#define ITM_SP15_REG                                         *((volatile uint32_t *)0xE000003Cu)
#define ITM_SP15_ADDR                                        (0xE000003Cu)
#define ITM_SP15_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP15_FIFOREADY                           (0x00000001u)
        #define ITM_SP15_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP15_FIFOREADY_BIT                       (0)
        #define ITM_SP15_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP15_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP15_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP15_STIMULUS_BIT                        (0)
        #define ITM_SP15_STIMULUS_BITS                       (32)

#define ITM_SP16                                             *((volatile uint32_t *)0xE0000040u)
#define ITM_SP16_REG                                         *((volatile uint32_t *)0xE0000040u)
#define ITM_SP16_ADDR                                        (0xE0000040u)
#define ITM_SP16_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP16_FIFOREADY                           (0x00000001u)
        #define ITM_SP16_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP16_FIFOREADY_BIT                       (0)
        #define ITM_SP16_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP16_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP16_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP16_STIMULUS_BIT                        (0)
        #define ITM_SP16_STIMULUS_BITS                       (32)

#define ITM_SP17                                             *((volatile uint32_t *)0xE0000044u)
#define ITM_SP17_REG                                         *((volatile uint32_t *)0xE0000044u)
#define ITM_SP17_ADDR                                        (0xE0000044u)
#define ITM_SP17_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP17_FIFOREADY                           (0x00000001u)
        #define ITM_SP17_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP17_FIFOREADY_BIT                       (0)
        #define ITM_SP17_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP17_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP17_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP17_STIMULUS_BIT                        (0)
        #define ITM_SP17_STIMULUS_BITS                       (32)

#define ITM_SP18                                             *((volatile uint32_t *)0xE0000048u)
#define ITM_SP18_REG                                         *((volatile uint32_t *)0xE0000048u)
#define ITM_SP18_ADDR                                        (0xE0000048u)
#define ITM_SP18_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP18_FIFOREADY                           (0x00000001u)
        #define ITM_SP18_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP18_FIFOREADY_BIT                       (0)
        #define ITM_SP18_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP18_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP18_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP18_STIMULUS_BIT                        (0)
        #define ITM_SP18_STIMULUS_BITS                       (32)

#define ITM_SP19                                             *((volatile uint32_t *)0xE000004Cu)
#define ITM_SP19_REG                                         *((volatile uint32_t *)0xE000004Cu)
#define ITM_SP19_ADDR                                        (0xE000004Cu)
#define ITM_SP19_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP19_FIFOREADY                           (0x00000001u)
        #define ITM_SP19_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP19_FIFOREADY_BIT                       (0)
        #define ITM_SP19_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP19_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP19_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP19_STIMULUS_BIT                        (0)
        #define ITM_SP19_STIMULUS_BITS                       (32)

#define ITM_SP20                                             *((volatile uint32_t *)0xE0000050u)
#define ITM_SP20_REG                                         *((volatile uint32_t *)0xE0000050u)
#define ITM_SP20_ADDR                                        (0xE0000050u)
#define ITM_SP20_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP20_FIFOREADY                           (0x00000001u)
        #define ITM_SP20_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP20_FIFOREADY_BIT                       (0)
        #define ITM_SP20_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP20_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP20_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP20_STIMULUS_BIT                        (0)
        #define ITM_SP20_STIMULUS_BITS                       (32)

#define ITM_SP21                                             *((volatile uint32_t *)0xE0000054u)
#define ITM_SP21_REG                                         *((volatile uint32_t *)0xE0000054u)
#define ITM_SP21_ADDR                                        (0xE0000054u)
#define ITM_SP21_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP21_FIFOREADY                           (0x00000001u)
        #define ITM_SP21_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP21_FIFOREADY_BIT                       (0)
        #define ITM_SP21_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP21_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP21_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP21_STIMULUS_BIT                        (0)
        #define ITM_SP21_STIMULUS_BITS                       (32)

#define ITM_SP22                                             *((volatile uint32_t *)0xE0000058u)
#define ITM_SP22_REG                                         *((volatile uint32_t *)0xE0000058u)
#define ITM_SP22_ADDR                                        (0xE0000058u)
#define ITM_SP22_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP22_FIFOREADY                           (0x00000001u)
        #define ITM_SP22_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP22_FIFOREADY_BIT                       (0)
        #define ITM_SP22_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP22_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP22_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP22_STIMULUS_BIT                        (0)
        #define ITM_SP22_STIMULUS_BITS                       (32)

#define ITM_SP23                                             *((volatile uint32_t *)0xE000005Cu)
#define ITM_SP23_REG                                         *((volatile uint32_t *)0xE000005Cu)
#define ITM_SP23_ADDR                                        (0xE000005Cu)
#define ITM_SP23_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP23_FIFOREADY                           (0x00000001u)
        #define ITM_SP23_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP23_FIFOREADY_BIT                       (0)
        #define ITM_SP23_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP23_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP23_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP23_STIMULUS_BIT                        (0)
        #define ITM_SP23_STIMULUS_BITS                       (32)

#define ITM_SP24                                             *((volatile uint32_t *)0xE0000060u)
#define ITM_SP24_REG                                         *((volatile uint32_t *)0xE0000060u)
#define ITM_SP24_ADDR                                        (0xE0000060u)
#define ITM_SP24_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP24_FIFOREADY                           (0x00000001u)
        #define ITM_SP24_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP24_FIFOREADY_BIT                       (0)
        #define ITM_SP24_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP24_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP24_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP24_STIMULUS_BIT                        (0)
        #define ITM_SP24_STIMULUS_BITS                       (32)

#define ITM_SP25                                             *((volatile uint32_t *)0xE0000064u)
#define ITM_SP25_REG                                         *((volatile uint32_t *)0xE0000064u)
#define ITM_SP25_ADDR                                        (0xE0000064u)
#define ITM_SP25_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP25_FIFOREADY                           (0x00000001u)
        #define ITM_SP25_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP25_FIFOREADY_BIT                       (0)
        #define ITM_SP25_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP25_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP25_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP25_STIMULUS_BIT                        (0)
        #define ITM_SP25_STIMULUS_BITS                       (32)

#define ITM_SP26                                             *((volatile uint32_t *)0xE0000068u)
#define ITM_SP26_REG                                         *((volatile uint32_t *)0xE0000068u)
#define ITM_SP26_ADDR                                        (0xE0000068u)
#define ITM_SP26_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP26_FIFOREADY                           (0x00000001u)
        #define ITM_SP26_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP26_FIFOREADY_BIT                       (0)
        #define ITM_SP26_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP26_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP26_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP26_STIMULUS_BIT                        (0)
        #define ITM_SP26_STIMULUS_BITS                       (32)

#define ITM_SP27                                             *((volatile uint32_t *)0xE000006Cu)
#define ITM_SP27_REG                                         *((volatile uint32_t *)0xE000006Cu)
#define ITM_SP27_ADDR                                        (0xE000006Cu)
#define ITM_SP27_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP27_FIFOREADY                           (0x00000001u)
        #define ITM_SP27_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP27_FIFOREADY_BIT                       (0)
        #define ITM_SP27_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP27_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP27_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP27_STIMULUS_BIT                        (0)
        #define ITM_SP27_STIMULUS_BITS                       (32)

#define ITM_SP28                                             *((volatile uint32_t *)0xE0000070u)
#define ITM_SP28_REG                                         *((volatile uint32_t *)0xE0000070u)
#define ITM_SP28_ADDR                                        (0xE0000070u)
#define ITM_SP28_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP28_FIFOREADY                           (0x00000001u)
        #define ITM_SP28_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP28_FIFOREADY_BIT                       (0)
        #define ITM_SP28_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP28_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP28_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP28_STIMULUS_BIT                        (0)
        #define ITM_SP28_STIMULUS_BITS                       (32)

#define ITM_SP29                                             *((volatile uint32_t *)0xE0000074u)
#define ITM_SP29_REG                                         *((volatile uint32_t *)0xE0000074u)
#define ITM_SP29_ADDR                                        (0xE0000074u)
#define ITM_SP29_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP29_FIFOREADY                           (0x00000001u)
        #define ITM_SP29_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP29_FIFOREADY_BIT                       (0)
        #define ITM_SP29_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP29_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP29_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP29_STIMULUS_BIT                        (0)
        #define ITM_SP29_STIMULUS_BITS                       (32)

#define ITM_SP30                                             *((volatile uint32_t *)0xE0000078u)
#define ITM_SP30_REG                                         *((volatile uint32_t *)0xE0000078u)
#define ITM_SP30_ADDR                                        (0xE0000078u)
#define ITM_SP30_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP30_FIFOREADY                           (0x00000001u)
        #define ITM_SP30_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP30_FIFOREADY_BIT                       (0)
        #define ITM_SP30_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP30_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP30_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP30_STIMULUS_BIT                        (0)
        #define ITM_SP30_STIMULUS_BITS                       (32)

#define ITM_SP31                                             *((volatile uint32_t *)0xE000007Cu)
#define ITM_SP31_REG                                         *((volatile uint32_t *)0xE000007Cu)
#define ITM_SP31_ADDR                                        (0xE000007Cu)
#define ITM_SP31_RESET                                       (0x00000000u)
        /* FIFOREADY field */
        #define ITM_SP31_FIFOREADY                           (0x00000001u)
        #define ITM_SP31_FIFOREADY_MASK                      (0x00000001u)
        #define ITM_SP31_FIFOREADY_BIT                       (0)
        #define ITM_SP31_FIFOREADY_BITS                      (1)
        /* STIMULUS field */
        #define ITM_SP31_STIMULUS                            (0xFFFFFFFFu)
        #define ITM_SP31_STIMULUS_MASK                       (0xFFFFFFFFu)
        #define ITM_SP31_STIMULUS_BIT                        (0)
        #define ITM_SP31_STIMULUS_BITS                       (32)

#define ITM_TER                                              *((volatile uint32_t *)0xE0000E00u)
#define ITM_TER_REG                                          *((volatile uint32_t *)0xE0000E00u)
#define ITM_TER_ADDR                                         (0xE0000E00u)
#define ITM_TER_RESET                                        (0x00000000u)
        /* STIMENA field */
        #define ITM_TER_STIMENA                              (0xFFFFFFFFu)
        #define ITM_TER_STIMENA_MASK                         (0xFFFFFFFFu)
        #define ITM_TER_STIMENA_BIT                          (0)
        #define ITM_TER_STIMENA_BITS                         (32)

#define ITM_TPR                                              *((volatile uint32_t *)0xE0000E40u)
#define ITM_TPR_REG                                          *((volatile uint32_t *)0xE0000E40u)
#define ITM_TPR_ADDR                                         (0xE0000E40u)
#define ITM_TPR_RESET                                        (0x00000000u)
        /* PRIVMASK field */
        #define ITM_TPR_PRIVMASK                             (0x0000000Fu)
        #define ITM_TPR_PRIVMASK_MASK                        (0x0000000Fu)
        #define ITM_TPR_PRIVMASK_BIT                         (0)
        #define ITM_TPR_PRIVMASK_BITS                        (4)

#define ITM_TCR                                              *((volatile uint32_t *)0xE0000E80u)
#define ITM_TCR_REG                                          *((volatile uint32_t *)0xE0000E80u)
#define ITM_TCR_ADDR                                         (0xE0000E80u)
#define ITM_TCR_RESET                                        (0x00000000u)
        /* BUSY field */
        #define ITM_TCR_BUSY                                 (0x00800000u)
        #define ITM_TCR_BUSY_MASK                            (0x00800000u)
        #define ITM_TCR_BUSY_BIT                             (23)
        #define ITM_TCR_BUSY_BITS                            (1)
        /* ATBID field */
        #define ITM_TCR_ATBID                                (0x007F0000u)
        #define ITM_TCR_ATBID_MASK                           (0x007F0000u)
        #define ITM_TCR_ATBID_BIT                            (16)
        #define ITM_TCR_ATBID_BITS                           (7)
        /* TSPRESCALE field */
        #define ITM_TCR_TSPRESCALE                           (0x00000300u)
        #define ITM_TCR_TSPRESCALE_MASK                      (0x00000300u)
        #define ITM_TCR_TSPRESCALE_BIT                       (8)
        #define ITM_TCR_TSPRESCALE_BITS                      (2)
        /* SWOENA field */
        #define ITM_TCR_SWOENA                               (0x00000010u)
        #define ITM_TCR_SWOENA_MASK                          (0x00000010u)
        #define ITM_TCR_SWOENA_BIT                           (4)
        #define ITM_TCR_SWOENA_BITS                          (1)
        /* DWTENA field */
        #define ITM_TCR_DWTENA                               (0x00000008u)
        #define ITM_TCR_DWTENA_MASK                          (0x00000008u)
        #define ITM_TCR_DWTENA_BIT                           (3)
        #define ITM_TCR_DWTENA_BITS                          (1)
        /* SYNCENA field */
        #define ITM_TCR_SYNCENA                              (0x00000004u)
        #define ITM_TCR_SYNCENA_MASK                         (0x00000004u)
        #define ITM_TCR_SYNCENA_BIT                          (2)
        #define ITM_TCR_SYNCENA_BITS                         (1)
        /* TSENA field */
        #define ITM_TCR_TSENA                                (0x00000002u)
        #define ITM_TCR_TSENA_MASK                           (0x00000002u)
        #define ITM_TCR_TSENA_BIT                            (1)
        #define ITM_TCR_TSENA_BITS                           (1)
        /* ITMEN field */
        #define ITM_TCR_ITMEN                                (0x00000001u)
        #define ITM_TCR_ITMEN_MASK                           (0x00000001u)
        #define ITM_TCR_ITMEN_BIT                            (0)
        #define ITM_TCR_ITMEN_BITS                           (1)

#define ITM_IW                                               *((volatile uint32_t *)0xE0000EF8u)
#define ITM_IW_REG                                           *((volatile uint32_t *)0xE0000EF8u)
#define ITM_IW_ADDR                                          (0xE0000EF8u)
#define ITM_IW_RESET                                         (0x00000000u)
        /* ATVALIDM field */
        #define ITM_IW_ATVALIDM                              (0x00000001u)
        #define ITM_IW_ATVALIDM_MASK                         (0x00000001u)
        #define ITM_IW_ATVALIDM_BIT                          (0)
        #define ITM_IW_ATVALIDM_BITS                         (1)

#define ITM_IR                                               *((volatile uint32_t *)0xE0000EFCu)
#define ITM_IR_REG                                           *((volatile uint32_t *)0xE0000EFCu)
#define ITM_IR_ADDR                                          (0xE0000EFCu)
#define ITM_IR_RESET                                         (0x00000000u)
        /* ATREADYM field */
        #define ITM_IR_ATREADYM                              (0x00000001u)
        #define ITM_IR_ATREADYM_MASK                         (0x00000001u)
        #define ITM_IR_ATREADYM_BIT                          (0)
        #define ITM_IR_ATREADYM_BITS                         (1)

#define ITM_IMC                                              *((volatile uint32_t *)0xE0000F00u)
#define ITM_IMC_REG                                          *((volatile uint32_t *)0xE0000F00u)
#define ITM_IMC_ADDR                                         (0xE0000F00u)
#define ITM_IMC_RESET                                        (0x00000000u)
        /* INTEGRATION field */
        #define ITM_IMC_INTEGRATION                          (0x00000001u)
        #define ITM_IMC_INTEGRATION_MASK                     (0x00000001u)
        #define ITM_IMC_INTEGRATION_BIT                      (0)
        #define ITM_IMC_INTEGRATION_BITS                     (1)

#define ITM_LA                                               *((volatile uint32_t *)0xE0000FB0u)
#define ITM_LA_REG                                           *((volatile uint32_t *)0xE0000FB0u)
#define ITM_LA_ADDR                                          (0xE0000FB0u)
#define ITM_LA_RESET                                         (0x00000000u)
        /* LOCKACC field */
        #define ITM_LA_LOCKACC                               (0xFFFFFFFFu)
        #define ITM_LA_LOCKACC_MASK                          (0xFFFFFFFFu)
        #define ITM_LA_LOCKACC_BIT                           (0)
        #define ITM_LA_LOCKACC_BITS                          (32)

#define ITM_LS                                               *((volatile uint32_t *)0xE0000FB4u)
#define ITM_LS_REG                                           *((volatile uint32_t *)0xE0000FB4u)
#define ITM_LS_ADDR                                          (0xE0000FB4u)
#define ITM_LS_RESET                                         (0x00000000u)
        /* BYTEACC field */
        #define ITM_LS_BYTEACC                               (0x00000004u)
        #define ITM_LS_BYTEACC_MASK                          (0x00000004u)
        #define ITM_LS_BYTEACC_BIT                           (2)
        #define ITM_LS_BYTEACC_BITS                          (1)
        /* ACCESS field */
        #define ITM_LS_ACCESS                                (0x00000002u)
        #define ITM_LS_ACCESS_MASK                           (0x00000002u)
        #define ITM_LS_ACCESS_BIT                            (1)
        #define ITM_LS_ACCESS_BITS                           (1)
        /* PRESENT field */
        #define ITM_LS_PRESENT                               (0x00000001u)
        #define ITM_LS_PRESENT_MASK                          (0x00000001u)
        #define ITM_LS_PRESENT_BIT                           (0)
        #define ITM_LS_PRESENT_BITS                          (1)

#define ITM_PERIPHID4                                        *((volatile uint32_t *)0xE0000FD0u)
#define ITM_PERIPHID4_REG                                    *((volatile uint32_t *)0xE0000FD0u)
#define ITM_PERIPHID4_ADDR                                   (0xE0000FD0u)
#define ITM_PERIPHID4_RESET                                  (0x00000004u)
        /* PERIPHID field */
        #define ITM_PERIPHID4_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID4_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID4_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID4_PERIPHID_BITS                  (32)

#define ITM_PERIPHID5                                        *((volatile uint32_t *)0xE0000FD4u)
#define ITM_PERIPHID5_REG                                    *((volatile uint32_t *)0xE0000FD4u)
#define ITM_PERIPHID5_ADDR                                   (0xE0000FD4u)
#define ITM_PERIPHID5_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define ITM_PERIPHID5_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID5_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID5_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID5_PERIPHID_BITS                  (32)

#define ITM_PERIPHID6                                        *((volatile uint32_t *)0xE0000FD8u)
#define ITM_PERIPHID6_REG                                    *((volatile uint32_t *)0xE0000FD8u)
#define ITM_PERIPHID6_ADDR                                   (0xE0000FD8u)
#define ITM_PERIPHID6_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define ITM_PERIPHID6_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID6_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID6_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID6_PERIPHID_BITS                  (32)

#define ITM_PERIPHID7                                        *((volatile uint32_t *)0xE0000FDCu)
#define ITM_PERIPHID7_REG                                    *((volatile uint32_t *)0xE0000FDCu)
#define ITM_PERIPHID7_ADDR                                   (0xE0000FDCu)
#define ITM_PERIPHID7_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define ITM_PERIPHID7_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID7_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID7_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID7_PERIPHID_BITS                  (32)

#define ITM_PERIPHID0                                        *((volatile uint32_t *)0xE0000FE0u)
#define ITM_PERIPHID0_REG                                    *((volatile uint32_t *)0xE0000FE0u)
#define ITM_PERIPHID0_ADDR                                   (0xE0000FE0u)
#define ITM_PERIPHID0_RESET                                  (0x00000001u)
        /* PERIPHID field */
        #define ITM_PERIPHID0_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID0_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID0_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID0_PERIPHID_BITS                  (32)

#define ITM_PERIPHID1                                        *((volatile uint32_t *)0xE0000FE4u)
#define ITM_PERIPHID1_REG                                    *((volatile uint32_t *)0xE0000FE4u)
#define ITM_PERIPHID1_ADDR                                   (0xE0000FE4u)
#define ITM_PERIPHID1_RESET                                  (0x000000B0u)
        /* PERIPHID field */
        #define ITM_PERIPHID1_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID1_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID1_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID1_PERIPHID_BITS                  (32)

#define ITM_PERIPHID2                                        *((volatile uint32_t *)0xE0000FE8u)
#define ITM_PERIPHID2_REG                                    *((volatile uint32_t *)0xE0000FE8u)
#define ITM_PERIPHID2_ADDR                                   (0xE0000FE8u)
#define ITM_PERIPHID2_RESET                                  (0x0000001Bu)
        /* PERIPHID field */
        #define ITM_PERIPHID2_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID2_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID2_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID2_PERIPHID_BITS                  (32)

#define ITM_PERIPHID3                                        *((volatile uint32_t *)0xE0000FECu)
#define ITM_PERIPHID3_REG                                    *((volatile uint32_t *)0xE0000FECu)
#define ITM_PERIPHID3_ADDR                                   (0xE0000FECu)
#define ITM_PERIPHID3_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define ITM_PERIPHID3_PERIPHID                       (0xFFFFFFFFu)
        #define ITM_PERIPHID3_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define ITM_PERIPHID3_PERIPHID_BIT                   (0)
        #define ITM_PERIPHID3_PERIPHID_BITS                  (32)

#define ITM_CELLID0                                          *((volatile uint32_t *)0xE0000FF0u)
#define ITM_CELLID0_REG                                      *((volatile uint32_t *)0xE0000FF0u)
#define ITM_CELLID0_ADDR                                     (0xE0000FF0u)
#define ITM_CELLID0_RESET                                    (0x0000000Du)
        /* PERIPHID field */
        #define ITM_CELLID0_PERIPHID                         (0xFFFFFFFFu)
        #define ITM_CELLID0_PERIPHID_MASK                    (0xFFFFFFFFu)
        #define ITM_CELLID0_PERIPHID_BIT                     (0)
        #define ITM_CELLID0_PERIPHID_BITS                    (32)

#define ITM_CELLID1                                          *((volatile uint32_t *)0xE0000FF4u)
#define ITM_CELLID1_REG                                      *((volatile uint32_t *)0xE0000FF4u)
#define ITM_CELLID1_ADDR                                     (0xE0000FF4u)
#define ITM_CELLID1_RESET                                    (0x000000E0u)
        /* PERIPHID field */
        #define ITM_CELLID1_PERIPHID                         (0xFFFFFFFFu)
        #define ITM_CELLID1_PERIPHID_MASK                    (0xFFFFFFFFu)
        #define ITM_CELLID1_PERIPHID_BIT                     (0)
        #define ITM_CELLID1_PERIPHID_BITS                    (32)

#define ITM_CELLID2                                          *((volatile uint32_t *)0xE0000FF8u)
#define ITM_CELLID2_REG                                      *((volatile uint32_t *)0xE0000FF8u)
#define ITM_CELLID2_ADDR                                     (0xE0000FF8u)
#define ITM_CELLID2_RESET                                    (0x00000005u)
        /* PERIPHID field */
        #define ITM_CELLID2_PERIPHID                         (0xFFFFFFFFu)
        #define ITM_CELLID2_PERIPHID_MASK                    (0xFFFFFFFFu)
        #define ITM_CELLID2_PERIPHID_BIT                     (0)
        #define ITM_CELLID2_PERIPHID_BITS                    (32)

#define ITM_CELLID3                                          *((volatile uint32_t *)0xE0000FFCu)
#define ITM_CELLID3_REG                                      *((volatile uint32_t *)0xE0000FFCu)
#define ITM_CELLID3_ADDR                                     (0xE0000FFCu)
#define ITM_CELLID3_RESET                                    (0x000000B1u)
        /* PERIPHID field */
        #define ITM_CELLID3_PERIPHID                         (0xFFFFFFFFu)
        #define ITM_CELLID3_PERIPHID_MASK                    (0xFFFFFFFFu)
        #define ITM_CELLID3_PERIPHID_BIT                     (0)
        #define ITM_CELLID3_PERIPHID_BITS                    (32)

/* DWT block */
#define DATA_DWT_BASE                                        (0xE0001000u)
#define DATA_DWT_END                                         (0xE0001FFFu)
#define DATA_DWT_SIZE                                        (DATA_DWT_END - DATA_DWT_BASE + 1)

#define DWT_CTRL                                             *((volatile uint32_t *)0xE0001000u)
#define DWT_CTRL_REG                                         *((volatile uint32_t *)0xE0001000u)
#define DWT_CTRL_ADDR                                        (0xE0001000u)
#define DWT_CTRL_RESET                                       (0x40000000u)
        /* NUMCOMP field */
        #define DWT_CTRL_NUMCOMP                             (0xF0000000u)
        #define DWT_CTRL_NUMCOMP_MASK                        (0xF0000000u)
        #define DWT_CTRL_NUMCOMP_BIT                         (28)
        #define DWT_CTRL_NUMCOMP_BITS                        (4)
        /* CYCEVTENA field */
        #define DWT_CTRL_CYCEVTENA                           (0x00400000u)
        #define DWT_CTRL_CYCEVTENA_MASK                      (0x00400000u)
        #define DWT_CTRL_CYCEVTENA_BIT                       (22)
        #define DWT_CTRL_CYCEVTENA_BITS                      (1)
        /* FOLDEVTENA field */
        #define DWT_CTRL_FOLDEVTENA                          (0x00200000u)
        #define DWT_CTRL_FOLDEVTENA_MASK                     (0x00200000u)
        #define DWT_CTRL_FOLDEVTENA_BIT                      (21)
        #define DWT_CTRL_FOLDEVTENA_BITS                     (1)
        /* LSUEVTENA field */
        #define DWT_CTRL_LSUEVTENA                           (0x00100000u)
        #define DWT_CTRL_LSUEVTENA_MASK                      (0x00100000u)
        #define DWT_CTRL_LSUEVTENA_BIT                       (20)
        #define DWT_CTRL_LSUEVTENA_BITS                      (1)
        /* SLEEPEVTENA field */
        #define DWT_CTRL_SLEEPEVTENA                         (0x00080000u)
        #define DWT_CTRL_SLEEPEVTENA_MASK                    (0x00080000u)
        #define DWT_CTRL_SLEEPEVTENA_BIT                     (19)
        #define DWT_CTRL_SLEEPEVTENA_BITS                    (1)
        /* EXCEVTENA field */
        #define DWT_CTRL_EXCEVTENA                           (0x00040000u)
        #define DWT_CTRL_EXCEVTENA_MASK                      (0x00040000u)
        #define DWT_CTRL_EXCEVTENA_BIT                       (18)
        #define DWT_CTRL_EXCEVTENA_BITS                      (1)
        /* CPIEVTENA field */
        #define DWT_CTRL_CPIEVTENA                           (0x00020000u)
        #define DWT_CTRL_CPIEVTENA_MASK                      (0x00020000u)
        #define DWT_CTRL_CPIEVTENA_BIT                       (17)
        #define DWT_CTRL_CPIEVTENA_BITS                      (1)
        /* EXCTRCENA field */
        #define DWT_CTRL_EXCTRCENA                           (0x00010000u)
        #define DWT_CTRL_EXCTRCENA_MASK                      (0x00010000u)
        #define DWT_CTRL_EXCTRCENA_BIT                       (16)
        #define DWT_CTRL_EXCTRCENA_BITS                      (1)
        /* PCSAMPLEENA field */
        #define DWT_CTRL_PCSAMPLEENA                         (0x00001000u)
        #define DWT_CTRL_PCSAMPLEENA_MASK                    (0x00001000u)
        #define DWT_CTRL_PCSAMPLEENA_BIT                     (12)
        #define DWT_CTRL_PCSAMPLEENA_BITS                    (1)
        /* SYNCTAP field */
        #define DWT_CTRL_SYNCTAP                             (0x00000C00u)
        #define DWT_CTRL_SYNCTAP_MASK                        (0x00000C00u)
        #define DWT_CTRL_SYNCTAP_BIT                         (10)
        #define DWT_CTRL_SYNCTAP_BITS                        (2)
        /* CYCTAP field */
        #define DWT_CTRL_CYCTAP                              (0x00000200u)
        #define DWT_CTRL_CYCTAP_MASK                         (0x00000200u)
        #define DWT_CTRL_CYCTAP_BIT                          (9)
        #define DWT_CTRL_CYCTAP_BITS                         (1)
        /* POSTCNT field */
        #define DWT_CTRL_POSTCNT                             (0x000001E0u)
        #define DWT_CTRL_POSTCNT_MASK                        (0x000001E0u)
        #define DWT_CTRL_POSTCNT_BIT                         (5)
        #define DWT_CTRL_POSTCNT_BITS                        (4)
        /* POSTPRESET field */
        #define DWT_CTRL_POSTPRESET                          (0x0000001Eu)
        #define DWT_CTRL_POSTPRESET_MASK                     (0x0000001Eu)
        #define DWT_CTRL_POSTPRESET_BIT                      (1)
        #define DWT_CTRL_POSTPRESET_BITS                     (4)
        /* CYCCNTENA field */
        #define DWT_CTRL_CYCCNTENA                           (0x00000001u)
        #define DWT_CTRL_CYCCNTENA_MASK                      (0x00000001u)
        #define DWT_CTRL_CYCCNTENA_BIT                       (0)
        #define DWT_CTRL_CYCCNTENA_BITS                      (1)

#define DWT_CYCCNT                                           *((volatile uint32_t *)0xE0001004u)
#define DWT_CYCCNT_REG                                       *((volatile uint32_t *)0xE0001004u)
#define DWT_CYCCNT_ADDR                                      (0xE0001004u)
#define DWT_CYCCNT_RESET                                     (0x00000000u)
        /* CYCCNT field */
        #define DWT_CYCCNT_CYCCNT                            (0xFFFFFFFFu)
        #define DWT_CYCCNT_CYCCNT_MASK                       (0xFFFFFFFFu)
        #define DWT_CYCCNT_CYCCNT_BIT                        (0)
        #define DWT_CYCCNT_CYCCNT_BITS                       (32)

#define DWT_CPICNT                                           *((volatile uint32_t *)0xE0001008u)
#define DWT_CPICNT_REG                                       *((volatile uint32_t *)0xE0001008u)
#define DWT_CPICNT_ADDR                                      (0xE0001008u)
#define DWT_CPICNT_RESET                                     (0x00000000u)
        /* CPICNT field */
        #define DWT_CPICNT_CPICNT                            (0x000000FFu)
        #define DWT_CPICNT_CPICNT_MASK                       (0x000000FFu)
        #define DWT_CPICNT_CPICNT_BIT                        (0)
        #define DWT_CPICNT_CPICNT_BITS                       (8)

#define DWT_EXCCNT                                           *((volatile uint32_t *)0xE000100Cu)
#define DWT_EXCCNT_REG                                       *((volatile uint32_t *)0xE000100Cu)
#define DWT_EXCCNT_ADDR                                      (0xE000100Cu)
#define DWT_EXCCNT_RESET                                     (0x00000000u)
        /* EXCCNT field */
        #define DWT_EXCCNT_EXCCNT                            (0x000000FFu)
        #define DWT_EXCCNT_EXCCNT_MASK                       (0x000000FFu)
        #define DWT_EXCCNT_EXCCNT_BIT                        (0)
        #define DWT_EXCCNT_EXCCNT_BITS                       (8)

#define DWT_SLEEPCNT                                         *((volatile uint32_t *)0xE0001010u)
#define DWT_SLEEPCNT_REG                                     *((volatile uint32_t *)0xE0001010u)
#define DWT_SLEEPCNT_ADDR                                    (0xE0001010u)
#define DWT_SLEEPCNT_RESET                                   (0x00000000u)
        /* SLEEPCNT field */
        #define DWT_SLEEPCNT_SLEEPCNT                        (0x000000FFu)
        #define DWT_SLEEPCNT_SLEEPCNT_MASK                   (0x000000FFu)
        #define DWT_SLEEPCNT_SLEEPCNT_BIT                    (0)
        #define DWT_SLEEPCNT_SLEEPCNT_BITS                   (8)

#define DWT_LSUCNT                                           *((volatile uint32_t *)0xE0001014u)
#define DWT_LSUCNT_REG                                       *((volatile uint32_t *)0xE0001014u)
#define DWT_LSUCNT_ADDR                                      (0xE0001014u)
#define DWT_LSUCNT_RESET                                     (0x00000000u)
        /* CPICNT field */
        #define DWT_LSUCNT_CPICNT                            (0x000000FFu)
        #define DWT_LSUCNT_CPICNT_MASK                       (0x000000FFu)
        #define DWT_LSUCNT_CPICNT_BIT                        (0)
        #define DWT_LSUCNT_CPICNT_BITS                       (8)

#define DWT_FOLDCNT                                          *((volatile uint32_t *)0xE0001018u)
#define DWT_FOLDCNT_REG                                      *((volatile uint32_t *)0xE0001018u)
#define DWT_FOLDCNT_ADDR                                     (0xE0001018u)
#define DWT_FOLDCNT_RESET                                    (0x00000000u)
        /* CPICNT field */
        #define DWT_FOLDCNT_CPICNT                           (0x000000FFu)
        #define DWT_FOLDCNT_CPICNT_MASK                      (0x000000FFu)
        #define DWT_FOLDCNT_CPICNT_BIT                       (0)
        #define DWT_FOLDCNT_CPICNT_BITS                      (8)

#define DWT_PCSR                                             *((volatile uint32_t *)0xE000101Cu)
#define DWT_PCSR_REG                                         *((volatile uint32_t *)0xE000101Cu)
#define DWT_PCSR_ADDR                                        (0xE000101Cu)
#define DWT_PCSR_RESET                                       (0x00000000u)
        /* EIASAMPLE field */
        #define DWT_PCSR_EIASAMPLE                           (0xFFFFFFFFu)
        #define DWT_PCSR_EIASAMPLE_MASK                      (0xFFFFFFFFu)
        #define DWT_PCSR_EIASAMPLE_BIT                       (0)
        #define DWT_PCSR_EIASAMPLE_BITS                      (32)

#define DWT_COMP0                                            *((volatile uint32_t *)0xE0001020u)
#define DWT_COMP0_REG                                        *((volatile uint32_t *)0xE0001020u)
#define DWT_COMP0_ADDR                                       (0xE0001020u)
#define DWT_COMP0_RESET                                      (0x00000000u)
        /* COMP0 field */
        #define DWT_COMP0_COMP0                              (0xFFFFFFFFu)
        #define DWT_COMP0_COMP0_MASK                         (0xFFFFFFFFu)
        #define DWT_COMP0_COMP0_BIT                          (0)
        #define DWT_COMP0_COMP0_BITS                         (32)

#define DWT_MASK0                                            *((volatile uint32_t *)0xE0001024u)
#define DWT_MASK0_REG                                        *((volatile uint32_t *)0xE0001024u)
#define DWT_MASK0_ADDR                                       (0xE0001024u)
#define DWT_MASK0_RESET                                      (0x00000000u)
        /* MASK0 field */
        #define DWT_MASK0_MASK0                              (0x0000001Fu)
        #define DWT_MASK0_MASK0_MASK                         (0x0000001Fu)
        #define DWT_MASK0_MASK0_BIT                          (0)
        #define DWT_MASK0_MASK0_BITS                         (5)

#define DWT_FUNCTION0                                        *((volatile uint32_t *)0xE0001028u)
#define DWT_FUNCTION0_REG                                    *((volatile uint32_t *)0xE0001028u)
#define DWT_FUNCTION0_ADDR                                   (0xE0001028u)
#define DWT_FUNCTION0_RESET                                  (0x00000000u)
        /* MATCHED field */
        #define DWT_FUNCTION0_MATCHED                        (0x01000000u)
        #define DWT_FUNCTION0_MATCHED_MASK                   (0x01000000u)
        #define DWT_FUNCTION0_MATCHED_BIT                    (24)
        #define DWT_FUNCTION0_MATCHED_BITS                   (1)
        /* CYCMATCH field */
        #define DWT_FUNCTION0_CYCMATCH                       (0x00000080u)
        #define DWT_FUNCTION0_CYCMATCH_MASK                  (0x00000080u)
        #define DWT_FUNCTION0_CYCMATCH_BIT                   (7)
        #define DWT_FUNCTION0_CYCMATCH_BITS                  (1)
        /* EMITRANGE field */
        #define DWT_FUNCTION0_EMITRANGE                      (0x00000020u)
        #define DWT_FUNCTION0_EMITRANGE_MASK                 (0x00000020u)
        #define DWT_FUNCTION0_EMITRANGE_BIT                  (5)
        #define DWT_FUNCTION0_EMITRANGE_BITS                 (1)
        /* FUNCTION field */
        #define DWT_FUNCTION0_FUNCTION                       (0x0000000Fu)
        #define DWT_FUNCTION0_FUNCTION_MASK                  (0x0000000Fu)
        #define DWT_FUNCTION0_FUNCTION_BIT                   (0)
        #define DWT_FUNCTION0_FUNCTION_BITS                  (4)

#define DWT_COMP1                                            *((volatile uint32_t *)0xE0001030u)
#define DWT_COMP1_REG                                        *((volatile uint32_t *)0xE0001030u)
#define DWT_COMP1_ADDR                                       (0xE0001030u)
#define DWT_COMP1_RESET                                      (0x00000000u)
        /* COMP1 field */
        #define DWT_COMP1_COMP1                              (0xFFFFFFFFu)
        #define DWT_COMP1_COMP1_MASK                         (0xFFFFFFFFu)
        #define DWT_COMP1_COMP1_BIT                          (0)
        #define DWT_COMP1_COMP1_BITS                         (32)

#define DWT_MASK1                                            *((volatile uint32_t *)0xE0001034u)
#define DWT_MASK1_REG                                        *((volatile uint32_t *)0xE0001034u)
#define DWT_MASK1_ADDR                                       (0xE0001034u)
#define DWT_MASK1_RESET                                      (0x00000000u)
        /* MASK1 field */
        #define DWT_MASK1_MASK1                              (0x0000001Fu)
        #define DWT_MASK1_MASK1_MASK                         (0x0000001Fu)
        #define DWT_MASK1_MASK1_BIT                          (0)
        #define DWT_MASK1_MASK1_BITS                         (5)

#define DWT_FUNCTION1                                        *((volatile uint32_t *)0xE0001038u)
#define DWT_FUNCTION1_REG                                    *((volatile uint32_t *)0xE0001038u)
#define DWT_FUNCTION1_ADDR                                   (0xE0001038u)
#define DWT_FUNCTION1_RESET                                  (0x00000200u)
        /* MATCHED field */
        #define DWT_FUNCTION1_MATCHED                        (0x01000000u)
        #define DWT_FUNCTION1_MATCHED_MASK                   (0x01000000u)
        #define DWT_FUNCTION1_MATCHED_BIT                    (24)
        #define DWT_FUNCTION1_MATCHED_BITS                   (1)
        /* DATAVADDR1 field */
        #define DWT_FUNCTION1_DATAVADDR1                     (0x000F0000u)
        #define DWT_FUNCTION1_DATAVADDR1_MASK                (0x000F0000u)
        #define DWT_FUNCTION1_DATAVADDR1_BIT                 (16)
        #define DWT_FUNCTION1_DATAVADDR1_BITS                (4)
        /* DATAVADDR0 field */
        #define DWT_FUNCTION1_DATAVADDR0                     (0x0000F000u)
        #define DWT_FUNCTION1_DATAVADDR0_MASK                (0x0000F000u)
        #define DWT_FUNCTION1_DATAVADDR0_BIT                 (12)
        #define DWT_FUNCTION1_DATAVADDR0_BITS                (4)
        /* DATAVSIZE field */
        #define DWT_FUNCTION1_DATAVSIZE                      (0x00000C00u)
        #define DWT_FUNCTION1_DATAVSIZE_MASK                 (0x00000C00u)
        #define DWT_FUNCTION1_DATAVSIZE_BIT                  (10)
        #define DWT_FUNCTION1_DATAVSIZE_BITS                 (2)
        /* LNK1ENA field */
        #define DWT_FUNCTION1_LNK1ENA                        (0x00000200u)
        #define DWT_FUNCTION1_LNK1ENA_MASK                   (0x00000200u)
        #define DWT_FUNCTION1_LNK1ENA_BIT                    (9)
        #define DWT_FUNCTION1_LNK1ENA_BITS                   (1)
        /* DATAVMATCH field */
        #define DWT_FUNCTION1_DATAVMATCH                     (0x00000100u)
        #define DWT_FUNCTION1_DATAVMATCH_MASK                (0x00000100u)
        #define DWT_FUNCTION1_DATAVMATCH_BIT                 (8)
        #define DWT_FUNCTION1_DATAVMATCH_BITS                (1)
        /* EMITRANGE field */
        #define DWT_FUNCTION1_EMITRANGE                      (0x00000020u)
        #define DWT_FUNCTION1_EMITRANGE_MASK                 (0x00000020u)
        #define DWT_FUNCTION1_EMITRANGE_BIT                  (5)
        #define DWT_FUNCTION1_EMITRANGE_BITS                 (1)
        /* FUNCTION field */
        #define DWT_FUNCTION1_FUNCTION                       (0x0000000Fu)
        #define DWT_FUNCTION1_FUNCTION_MASK                  (0x0000000Fu)
        #define DWT_FUNCTION1_FUNCTION_BIT                   (0)
        #define DWT_FUNCTION1_FUNCTION_BITS                  (4)

#define DWT_COMP2                                            *((volatile uint32_t *)0xE0001040u)
#define DWT_COMP2_REG                                        *((volatile uint32_t *)0xE0001040u)
#define DWT_COMP2_ADDR                                       (0xE0001040u)
#define DWT_COMP2_RESET                                      (0x00000000u)
        /* COMP2 field */
        #define DWT_COMP2_COMP2                              (0xFFFFFFFFu)
        #define DWT_COMP2_COMP2_MASK                         (0xFFFFFFFFu)
        #define DWT_COMP2_COMP2_BIT                          (0)
        #define DWT_COMP2_COMP2_BITS                         (32)

#define DWT_MASK2                                            *((volatile uint32_t *)0xE0001044u)
#define DWT_MASK2_REG                                        *((volatile uint32_t *)0xE0001044u)
#define DWT_MASK2_ADDR                                       (0xE0001044u)
#define DWT_MASK2_RESET                                      (0x00000000u)
        /* MASK2 field */
        #define DWT_MASK2_MASK2                              (0x0000001Fu)
        #define DWT_MASK2_MASK2_MASK                         (0x0000001Fu)
        #define DWT_MASK2_MASK2_BIT                          (0)
        #define DWT_MASK2_MASK2_BITS                         (5)

#define DWT_FUNCTION2                                        *((volatile uint32_t *)0xE0001048u)
#define DWT_FUNCTION2_REG                                    *((volatile uint32_t *)0xE0001048u)
#define DWT_FUNCTION2_ADDR                                   (0xE0001048u)
#define DWT_FUNCTION2_RESET                                  (0x00000000u)
        /* MATCHED field */
        #define DWT_FUNCTION2_MATCHED                        (0x01000000u)
        #define DWT_FUNCTION2_MATCHED_MASK                   (0x01000000u)
        #define DWT_FUNCTION2_MATCHED_BIT                    (24)
        #define DWT_FUNCTION2_MATCHED_BITS                   (1)
        /* EMITRANGE field */
        #define DWT_FUNCTION2_EMITRANGE                      (0x00000020u)
        #define DWT_FUNCTION2_EMITRANGE_MASK                 (0x00000020u)
        #define DWT_FUNCTION2_EMITRANGE_BIT                  (5)
        #define DWT_FUNCTION2_EMITRANGE_BITS                 (1)
        /* FUNCTION field */
        #define DWT_FUNCTION2_FUNCTION                       (0x0000000Fu)
        #define DWT_FUNCTION2_FUNCTION_MASK                  (0x0000000Fu)
        #define DWT_FUNCTION2_FUNCTION_BIT                   (0)
        #define DWT_FUNCTION2_FUNCTION_BITS                  (4)

#define DWT_COMP3                                            *((volatile uint32_t *)0xE0001050u)
#define DWT_COMP3_REG                                        *((volatile uint32_t *)0xE0001050u)
#define DWT_COMP3_ADDR                                       (0xE0001050u)
#define DWT_COMP3_RESET                                      (0x00000000u)
        /* COMP3 field */
        #define DWT_COMP3_COMP3                              (0xFFFFFFFFu)
        #define DWT_COMP3_COMP3_MASK                         (0xFFFFFFFFu)
        #define DWT_COMP3_COMP3_BIT                          (0)
        #define DWT_COMP3_COMP3_BITS                         (32)

#define DWT_MASK3                                            *((volatile uint32_t *)0xE0001054u)
#define DWT_MASK3_REG                                        *((volatile uint32_t *)0xE0001054u)
#define DWT_MASK3_ADDR                                       (0xE0001054u)
#define DWT_MASK3_RESET                                      (0x00000000u)
        /* MASK3 field */
        #define DWT_MASK3_MASK3                              (0x0000001Fu)
        #define DWT_MASK3_MASK3_MASK                         (0x0000001Fu)
        #define DWT_MASK3_MASK3_BIT                          (0)
        #define DWT_MASK3_MASK3_BITS                         (5)

#define DWT_FUNCTION3                                        *((volatile uint32_t *)0xE0001058u)
#define DWT_FUNCTION3_REG                                    *((volatile uint32_t *)0xE0001058u)
#define DWT_FUNCTION3_ADDR                                   (0xE0001058u)
#define DWT_FUNCTION3_RESET                                  (0x00000000u)
        /* MATCHED field */
        #define DWT_FUNCTION3_MATCHED                        (0x01000000u)
        #define DWT_FUNCTION3_MATCHED_MASK                   (0x01000000u)
        #define DWT_FUNCTION3_MATCHED_BIT                    (24)
        #define DWT_FUNCTION3_MATCHED_BITS                   (1)
        /* EMITRANGE field */
        #define DWT_FUNCTION3_EMITRANGE                      (0x00000020u)
        #define DWT_FUNCTION3_EMITRANGE_MASK                 (0x00000020u)
        #define DWT_FUNCTION3_EMITRANGE_BIT                  (5)
        #define DWT_FUNCTION3_EMITRANGE_BITS                 (1)
        /* FUNCTION field */
        #define DWT_FUNCTION3_FUNCTION                       (0x0000000Fu)
        #define DWT_FUNCTION3_FUNCTION_MASK                  (0x0000000Fu)
        #define DWT_FUNCTION3_FUNCTION_BIT                   (0)
        #define DWT_FUNCTION3_FUNCTION_BITS                  (4)

#define DWT_PERIPHID4                                        *((volatile uint32_t *)0xE0001FD0u)
#define DWT_PERIPHID4_REG                                    *((volatile uint32_t *)0xE0001FD0u)
#define DWT_PERIPHID4_ADDR                                   (0xE0001FD0u)
#define DWT_PERIPHID4_RESET                                  (0x00000004u)
        /* PERIPHID field */
        #define DWT_PERIPHID4_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID4_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID4_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID4_PERIPHID_BITS                  (32)

#define DWT_PERIPHID5                                        *((volatile uint32_t *)0xE0001FD4u)
#define DWT_PERIPHID5_REG                                    *((volatile uint32_t *)0xE0001FD4u)
#define DWT_PERIPHID5_ADDR                                   (0xE0001FD4u)
#define DWT_PERIPHID5_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define DWT_PERIPHID5_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID5_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID5_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID5_PERIPHID_BITS                  (32)

#define DWT_PERIPHID6                                        *((volatile uint32_t *)0xE0001FD8u)
#define DWT_PERIPHID6_REG                                    *((volatile uint32_t *)0xE0001FD8u)
#define DWT_PERIPHID6_ADDR                                   (0xE0001FD8u)
#define DWT_PERIPHID6_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define DWT_PERIPHID6_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID6_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID6_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID6_PERIPHID_BITS                  (32)

#define DWT_PERIPHID7                                        *((volatile uint32_t *)0xE0001FDCu)
#define DWT_PERIPHID7_REG                                    *((volatile uint32_t *)0xE0001FDCu)
#define DWT_PERIPHID7_ADDR                                   (0xE0001FDCu)
#define DWT_PERIPHID7_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define DWT_PERIPHID7_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID7_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID7_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID7_PERIPHID_BITS                  (32)

#define DWT_PERIPHID0                                        *((volatile uint32_t *)0xE0001FE0u)
#define DWT_PERIPHID0_REG                                    *((volatile uint32_t *)0xE0001FE0u)
#define DWT_PERIPHID0_ADDR                                   (0xE0001FE0u)
#define DWT_PERIPHID0_RESET                                  (0x00000002u)
        /* PERIPHID field */
        #define DWT_PERIPHID0_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID0_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID0_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID0_PERIPHID_BITS                  (32)

#define DWT_PERIPHID1                                        *((volatile uint32_t *)0xE0001FE4u)
#define DWT_PERIPHID1_REG                                    *((volatile uint32_t *)0xE0001FE4u)
#define DWT_PERIPHID1_ADDR                                   (0xE0001FE4u)
#define DWT_PERIPHID1_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define DWT_PERIPHID1_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID1_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID1_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID1_PERIPHID_BITS                  (32)

#define DWT_PERIPHID2                                        *((volatile uint32_t *)0xE0001FE8u)
#define DWT_PERIPHID2_REG                                    *((volatile uint32_t *)0xE0001FE8u)
#define DWT_PERIPHID2_ADDR                                   (0xE0001FE8u)
#define DWT_PERIPHID2_RESET                                  (0x0000001Bu)
        /* PERIPHID field */
        #define DWT_PERIPHID2_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID2_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID2_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID2_PERIPHID_BITS                  (32)

#define DWT_PERIPHID3                                        *((volatile uint32_t *)0xE0001FECu)
#define DWT_PERIPHID3_REG                                    *((volatile uint32_t *)0xE0001FECu)
#define DWT_PERIPHID3_ADDR                                   (0xE0001FECu)
#define DWT_PERIPHID3_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define DWT_PERIPHID3_PERIPHID                       (0xFFFFFFFFu)
        #define DWT_PERIPHID3_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define DWT_PERIPHID3_PERIPHID_BIT                   (0)
        #define DWT_PERIPHID3_PERIPHID_BITS                  (32)

#define DWT_CELLID0                                          *((volatile uint32_t *)0xE0001FF0u)
#define DWT_CELLID0_REG                                      *((volatile uint32_t *)0xE0001FF0u)
#define DWT_CELLID0_ADDR                                     (0xE0001FF0u)
#define DWT_CELLID0_RESET                                    (0x0000000Du)
        /* CELLID field */
        #define DWT_CELLID0_CELLID                           (0xFFFFFFFFu)
        #define DWT_CELLID0_CELLID_MASK                      (0xFFFFFFFFu)
        #define DWT_CELLID0_CELLID_BIT                       (0)
        #define DWT_CELLID0_CELLID_BITS                      (32)

#define DWT_CELLID1                                          *((volatile uint32_t *)0xE0001FF4u)
#define DWT_CELLID1_REG                                      *((volatile uint32_t *)0xE0001FF4u)
#define DWT_CELLID1_ADDR                                     (0xE0001FF4u)
#define DWT_CELLID1_RESET                                    (0x000000E0u)
        /* CELLID field */
        #define DWT_CELLID1_CELLID                           (0xFFFFFFFFu)
        #define DWT_CELLID1_CELLID_MASK                      (0xFFFFFFFFu)
        #define DWT_CELLID1_CELLID_BIT                       (0)
        #define DWT_CELLID1_CELLID_BITS                      (32)

#define DWT_CELLID2                                          *((volatile uint32_t *)0xE0001FF8u)
#define DWT_CELLID2_REG                                      *((volatile uint32_t *)0xE0001FF8u)
#define DWT_CELLID2_ADDR                                     (0xE0001FF8u)
#define DWT_CELLID2_RESET                                    (0x00000005u)
        /* CELLID field */
        #define DWT_CELLID2_CELLID                           (0xFFFFFFFFu)
        #define DWT_CELLID2_CELLID_MASK                      (0xFFFFFFFFu)
        #define DWT_CELLID2_CELLID_BIT                       (0)
        #define DWT_CELLID2_CELLID_BITS                      (32)

#define DWT_CELLID3                                          *((volatile uint32_t *)0xE0001FFCu)
#define DWT_CELLID3_REG                                      *((volatile uint32_t *)0xE0001FFCu)
#define DWT_CELLID3_ADDR                                     (0xE0001FFCu)
#define DWT_CELLID3_RESET                                    (0x000000B1u)
        /* CELLID field */
        #define DWT_CELLID3_CELLID                           (0xFFFFFFFFu)
        #define DWT_CELLID3_CELLID_MASK                      (0xFFFFFFFFu)
        #define DWT_CELLID3_CELLID_BIT                       (0)
        #define DWT_CELLID3_CELLID_BITS                      (32)

/* FPB block */
#define DATA_FPB_BASE                                        (0xE0002000u)
#define DATA_FPB_END                                         (0xE0002FFFu)
#define DATA_FPB_SIZE                                        (DATA_FPB_END - DATA_FPB_BASE + 1)

#define FPB_CTRL                                             *((volatile uint32_t *)0xE0002000u)
#define FPB_CTRL_REG                                         *((volatile uint32_t *)0xE0002000u)
#define FPB_CTRL_ADDR                                        (0xE0002000u)
#define FPB_CTRL_RESET                                       (0x00000000u)
        /* NUM_LIT field */
        #define FPB_CTRL_NUM_LIT                             (0x00000F00u)
        #define FPB_CTRL_NUM_LIT_MASK                        (0x00000F00u)
        #define FPB_CTRL_NUM_LIT_BIT                         (8)
        #define FPB_CTRL_NUM_LIT_BITS                        (4)
        /* NUM_CODE field */
        #define FPB_CTRL_NUM_CODE                            (0x000000F0u)
        #define FPB_CTRL_NUM_CODE_MASK                       (0x000000F0u)
        #define FPB_CTRL_NUM_CODE_BIT                        (4)
        #define FPB_CTRL_NUM_CODE_BITS                       (4)
        /* KEY field */
        #define FPB_CTRL_KEY                                 (0x00000002u)
        #define FPB_CTRL_KEY_MASK                            (0x00000002u)
        #define FPB_CTRL_KEY_BIT                             (1)
        #define FPB_CTRL_KEY_BITS                            (1)
        /* enable field */
        #define FPB_CTRL_enable                              (0x00000001u)
        #define FPB_CTRL_enable_MASK                         (0x00000001u)
        #define FPB_CTRL_enable_BIT                          (0)
        #define FPB_CTRL_enable_BITS                         (1)

#define FPB_REMAP                                            *((volatile uint32_t *)0xE0002004u)
#define FPB_REMAP_REG                                        *((volatile uint32_t *)0xE0002004u)
#define FPB_REMAP_ADDR                                       (0xE0002004u)
#define FPB_REMAP_RESET                                      (0x20000000u)
        /* REMAP field */
        #define FPB_REMAP_REMAP                              (0x1FFFFFE0u)
        #define FPB_REMAP_REMAP_MASK                         (0x1FFFFFE0u)
        #define FPB_REMAP_REMAP_BIT                          (5)
        #define FPB_REMAP_REMAP_BITS                         (24)

#define FPB_COMP0                                            *((volatile uint32_t *)0xE0002008u)
#define FPB_COMP0_REG                                        *((volatile uint32_t *)0xE0002008u)
#define FPB_COMP0_ADDR                                       (0xE0002008u)
#define FPB_COMP0_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP0_REPLACE                            (0xC0000000u)
        #define FPB_COMP0_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP0_REPLACE_BIT                        (30)
        #define FPB_COMP0_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP0_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP0_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP0_COMP_BIT                           (2)
        #define FPB_COMP0_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP0_enable                             (0x00000001u)
        #define FPB_COMP0_enable_MASK                        (0x00000001u)
        #define FPB_COMP0_enable_BIT                         (0)
        #define FPB_COMP0_enable_BITS                        (1)

#define FPB_COMP1                                            *((volatile uint32_t *)0xE000200Cu)
#define FPB_COMP1_REG                                        *((volatile uint32_t *)0xE000200Cu)
#define FPB_COMP1_ADDR                                       (0xE000200Cu)
#define FPB_COMP1_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP1_REPLACE                            (0xC0000000u)
        #define FPB_COMP1_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP1_REPLACE_BIT                        (30)
        #define FPB_COMP1_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP1_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP1_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP1_COMP_BIT                           (2)
        #define FPB_COMP1_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP1_enable                             (0x00000001u)
        #define FPB_COMP1_enable_MASK                        (0x00000001u)
        #define FPB_COMP1_enable_BIT                         (0)
        #define FPB_COMP1_enable_BITS                        (1)

#define FPB_COMP2                                            *((volatile uint32_t *)0xE0002010u)
#define FPB_COMP2_REG                                        *((volatile uint32_t *)0xE0002010u)
#define FPB_COMP2_ADDR                                       (0xE0002010u)
#define FPB_COMP2_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP2_REPLACE                            (0xC0000000u)
        #define FPB_COMP2_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP2_REPLACE_BIT                        (30)
        #define FPB_COMP2_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP2_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP2_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP2_COMP_BIT                           (2)
        #define FPB_COMP2_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP2_enable                             (0x00000001u)
        #define FPB_COMP2_enable_MASK                        (0x00000001u)
        #define FPB_COMP2_enable_BIT                         (0)
        #define FPB_COMP2_enable_BITS                        (1)

#define FPB_COMP3                                            *((volatile uint32_t *)0xE0002014u)
#define FPB_COMP3_REG                                        *((volatile uint32_t *)0xE0002014u)
#define FPB_COMP3_ADDR                                       (0xE0002014u)
#define FPB_COMP3_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP3_REPLACE                            (0xC0000000u)
        #define FPB_COMP3_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP3_REPLACE_BIT                        (30)
        #define FPB_COMP3_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP3_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP3_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP3_COMP_BIT                           (2)
        #define FPB_COMP3_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP3_enable                             (0x00000001u)
        #define FPB_COMP3_enable_MASK                        (0x00000001u)
        #define FPB_COMP3_enable_BIT                         (0)
        #define FPB_COMP3_enable_BITS                        (1)

#define FPB_COMP4                                            *((volatile uint32_t *)0xE0002018u)
#define FPB_COMP4_REG                                        *((volatile uint32_t *)0xE0002018u)
#define FPB_COMP4_ADDR                                       (0xE0002018u)
#define FPB_COMP4_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP4_REPLACE                            (0xC0000000u)
        #define FPB_COMP4_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP4_REPLACE_BIT                        (30)
        #define FPB_COMP4_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP4_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP4_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP4_COMP_BIT                           (2)
        #define FPB_COMP4_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP4_enable                             (0x00000001u)
        #define FPB_COMP4_enable_MASK                        (0x00000001u)
        #define FPB_COMP4_enable_BIT                         (0)
        #define FPB_COMP4_enable_BITS                        (1)

#define FPB_COMP5                                            *((volatile uint32_t *)0xE000201Cu)
#define FPB_COMP5_REG                                        *((volatile uint32_t *)0xE000201Cu)
#define FPB_COMP5_ADDR                                       (0xE000201Cu)
#define FPB_COMP5_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP5_REPLACE                            (0xC0000000u)
        #define FPB_COMP5_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP5_REPLACE_BIT                        (30)
        #define FPB_COMP5_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP5_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP5_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP5_COMP_BIT                           (2)
        #define FPB_COMP5_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP5_enable                             (0x00000001u)
        #define FPB_COMP5_enable_MASK                        (0x00000001u)
        #define FPB_COMP5_enable_BIT                         (0)
        #define FPB_COMP5_enable_BITS                        (1)

#define FPB_COMP6                                            *((volatile uint32_t *)0xE0002020u)
#define FPB_COMP6_REG                                        *((volatile uint32_t *)0xE0002020u)
#define FPB_COMP6_ADDR                                       (0xE0002020u)
#define FPB_COMP6_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP6_REPLACE                            (0xC0000000u)
        #define FPB_COMP6_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP6_REPLACE_BIT                        (30)
        #define FPB_COMP6_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP6_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP6_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP6_COMP_BIT                           (2)
        #define FPB_COMP6_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP6_enable                             (0x00000001u)
        #define FPB_COMP6_enable_MASK                        (0x00000001u)
        #define FPB_COMP6_enable_BIT                         (0)
        #define FPB_COMP6_enable_BITS                        (1)

#define FPB_COMP7                                            *((volatile uint32_t *)0xE0002024u)
#define FPB_COMP7_REG                                        *((volatile uint32_t *)0xE0002024u)
#define FPB_COMP7_ADDR                                       (0xE0002024u)
#define FPB_COMP7_RESET                                      (0x00000000u)
        /* REPLACE field */
        #define FPB_COMP7_REPLACE                            (0xC0000000u)
        #define FPB_COMP7_REPLACE_MASK                       (0xC0000000u)
        #define FPB_COMP7_REPLACE_BIT                        (30)
        #define FPB_COMP7_REPLACE_BITS                       (2)
        /* COMP field */
        #define FPB_COMP7_COMP                               (0x1FFFFFFCu)
        #define FPB_COMP7_COMP_MASK                          (0x1FFFFFFCu)
        #define FPB_COMP7_COMP_BIT                           (2)
        #define FPB_COMP7_COMP_BITS                          (27)
        /* enable field */
        #define FPB_COMP7_enable                             (0x00000001u)
        #define FPB_COMP7_enable_MASK                        (0x00000001u)
        #define FPB_COMP7_enable_BIT                         (0)
        #define FPB_COMP7_enable_BITS                        (1)

#define FPB_PERIPHID4                                        *((volatile uint32_t *)0xE0002FD0u)
#define FPB_PERIPHID4_REG                                    *((volatile uint32_t *)0xE0002FD0u)
#define FPB_PERIPHID4_ADDR                                   (0xE0002FD0u)
#define FPB_PERIPHID4_RESET                                  (0x00000004u)
        /* PERIPHID field */
        #define FPB_PERIPHID4_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID4_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID4_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID4_PERIPHID_BITS                  (32)

#define FPB_PERIPHID5                                        *((volatile uint32_t *)0xE0002FD4u)
#define FPB_PERIPHID5_REG                                    *((volatile uint32_t *)0xE0002FD4u)
#define FPB_PERIPHID5_ADDR                                   (0xE0002FD4u)
#define FPB_PERIPHID5_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define FPB_PERIPHID5_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID5_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID5_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID5_PERIPHID_BITS                  (32)

#define FPB_PERIPHID6                                        *((volatile uint32_t *)0xE0002FD8u)
#define FPB_PERIPHID6_REG                                    *((volatile uint32_t *)0xE0002FD8u)
#define FPB_PERIPHID6_ADDR                                   (0xE0002FD8u)
#define FPB_PERIPHID6_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define FPB_PERIPHID6_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID6_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID6_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID6_PERIPHID_BITS                  (32)

#define FPB_PERIPHID7                                        *((volatile uint32_t *)0xE0002FDCu)
#define FPB_PERIPHID7_REG                                    *((volatile uint32_t *)0xE0002FDCu)
#define FPB_PERIPHID7_ADDR                                   (0xE0002FDCu)
#define FPB_PERIPHID7_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define FPB_PERIPHID7_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID7_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID7_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID7_PERIPHID_BITS                  (32)

#define FPB_PERIPHID0                                        *((volatile uint32_t *)0xE0002FE0u)
#define FPB_PERIPHID0_REG                                    *((volatile uint32_t *)0xE0002FE0u)
#define FPB_PERIPHID0_ADDR                                   (0xE0002FE0u)
#define FPB_PERIPHID0_RESET                                  (0x00000003u)
        /* PERIPHID field */
        #define FPB_PERIPHID0_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID0_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID0_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID0_PERIPHID_BITS                  (32)

#define FPB_PERIPHID1                                        *((volatile uint32_t *)0xE0002FE4u)
#define FPB_PERIPHID1_REG                                    *((volatile uint32_t *)0xE0002FE4u)
#define FPB_PERIPHID1_ADDR                                   (0xE0002FE4u)
#define FPB_PERIPHID1_RESET                                  (0x000000B0u)
        /* PERIPHID field */
        #define FPB_PERIPHID1_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID1_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID1_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID1_PERIPHID_BITS                  (32)

#define FPB_PERIPHID2                                        *((volatile uint32_t *)0xE0002FE8u)
#define FPB_PERIPHID2_REG                                    *((volatile uint32_t *)0xE0002FE8u)
#define FPB_PERIPHID2_ADDR                                   (0xE0002FE8u)
#define FPB_PERIPHID2_RESET                                  (0x0000000Bu)
        /* PERIPHID field */
        #define FPB_PERIPHID2_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID2_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID2_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID2_PERIPHID_BITS                  (32)

#define FPB_PERIPHID3                                        *((volatile uint32_t *)0xE0002FECu)
#define FPB_PERIPHID3_REG                                    *((volatile uint32_t *)0xE0002FECu)
#define FPB_PERIPHID3_ADDR                                   (0xE0002FECu)
#define FPB_PERIPHID3_RESET                                  (0x00000000u)
        /* PERIPHID field */
        #define FPB_PERIPHID3_PERIPHID                       (0xFFFFFFFFu)
        #define FPB_PERIPHID3_PERIPHID_MASK                  (0xFFFFFFFFu)
        #define FPB_PERIPHID3_PERIPHID_BIT                   (0)
        #define FPB_PERIPHID3_PERIPHID_BITS                  (32)

#define FPB_CELLID0                                          *((volatile uint32_t *)0xE0002FF0u)
#define FPB_CELLID0_REG                                      *((volatile uint32_t *)0xE0002FF0u)
#define FPB_CELLID0_ADDR                                     (0xE0002FF0u)
#define FPB_CELLID0_RESET                                    (0x0000000Du)
        /* CELLID field */
        #define FPB_CELLID0_CELLID                           (0xFFFFFFFFu)
        #define FPB_CELLID0_CELLID_MASK                      (0xFFFFFFFFu)
        #define FPB_CELLID0_CELLID_BIT                       (0)
        #define FPB_CELLID0_CELLID_BITS                      (32)

#define FPB_CELLID1                                          *((volatile uint32_t *)0xE0002FF4u)
#define FPB_CELLID1_REG                                      *((volatile uint32_t *)0xE0002FF4u)
#define FPB_CELLID1_ADDR                                     (0xE0002FF4u)
#define FPB_CELLID1_RESET                                    (0x000000E0u)
        /* CELLID field */
        #define FPB_CELLID1_CELLID                           (0xFFFFFFFFu)
        #define FPB_CELLID1_CELLID_MASK                      (0xFFFFFFFFu)
        #define FPB_CELLID1_CELLID_BIT                       (0)
        #define FPB_CELLID1_CELLID_BITS                      (32)

#define FPB_CELLID2                                          *((volatile uint32_t *)0xE0002FF8u)
#define FPB_CELLID2_REG                                      *((volatile uint32_t *)0xE0002FF8u)
#define FPB_CELLID2_ADDR                                     (0xE0002FF8u)
#define FPB_CELLID2_RESET                                    (0x00000005u)
        /* CELLID field */
        #define FPB_CELLID2_CELLID                           (0xFFFFFFFFu)
        #define FPB_CELLID2_CELLID_MASK                      (0xFFFFFFFFu)
        #define FPB_CELLID2_CELLID_BIT                       (0)
        #define FPB_CELLID2_CELLID_BITS                      (32)

#define FPB_CELLID3                                          *((volatile uint32_t *)0xE0002FFCu)
#define FPB_CELLID3_REG                                      *((volatile uint32_t *)0xE0002FFCu)
#define FPB_CELLID3_ADDR                                     (0xE0002FFCu)
#define FPB_CELLID3_RESET                                    (0x000000B1u)
        /* CELLID field */
        #define FPB_CELLID3_CELLID                           (0xFFFFFFFFu)
        #define FPB_CELLID3_CELLID_MASK                      (0xFFFFFFFFu)
        #define FPB_CELLID3_CELLID_BIT                       (0)
        #define FPB_CELLID3_CELLID_BITS                      (32)

/* NVIC block */
#define BLOCK_NVIC_BASE                                      (0xE000E000u)
#define BLOCK_NVIC_END                                       (0xE000EFFFu)
#define BLOCK_NVIC_SIZE                                      (BLOCK_NVIC_END - BLOCK_NVIC_BASE + 1)

#define NVIC_MCR                                             *((volatile uint32_t *)0xE000E000u)
#define NVIC_MCR_REG                                         *((volatile uint32_t *)0xE000E000u)
#define NVIC_MCR_ADDR                                        (0xE000E000u)
#define NVIC_MCR_RESET                                       (0x00000000u)

#define NVIC_ICTR                                            *((volatile uint32_t *)0xE000E004u)
#define NVIC_ICTR_REG                                        *((volatile uint32_t *)0xE000E004u)
#define NVIC_ICTR_ADDR                                       (0xE000E004u)
#define NVIC_ICTR_RESET                                      (0x00000000u)
        /* INTLINESNUM field */
        #define NVIC_ICTR_INTLINESNUM                        (0x0000001Fu)
        #define NVIC_ICTR_INTLINESNUM_MASK                   (0x0000001Fu)
        #define NVIC_ICTR_INTLINESNUM_BIT                    (0)
        #define NVIC_ICTR_INTLINESNUM_BITS                   (5)

#define ST_CSR                                               *((volatile uint32_t *)0xE000E010u)
#define ST_CSR_REG                                           *((volatile uint32_t *)0xE000E010u)
#define ST_CSR_ADDR                                          (0xE000E010u)
#define ST_CSR_RESET                                         (0x00000000u)
        /* COUNTFLAG field */
        #define ST_CSR_COUNTFLAG                             (0x00010000u)
        #define ST_CSR_COUNTFLAG_MASK                        (0x00010000u)
        #define ST_CSR_COUNTFLAG_BIT                         (16)
        #define ST_CSR_COUNTFLAG_BITS                        (1)
        /* CLKSOURCE field */
        #define ST_CSR_CLKSOURCE                             (0x00000004u)
        #define ST_CSR_CLKSOURCE_MASK                        (0x00000004u)
        #define ST_CSR_CLKSOURCE_BIT                         (2)
        #define ST_CSR_CLKSOURCE_BITS                        (1)
        /* TICKINT field */
        #define ST_CSR_TICKINT                               (0x00000002u)
        #define ST_CSR_TICKINT_MASK                          (0x00000002u)
        #define ST_CSR_TICKINT_BIT                           (1)
        #define ST_CSR_TICKINT_BITS                          (1)
        /* ENABLE field */
        #define ST_CSR_ENABLE                                (0x00000001u)
        #define ST_CSR_ENABLE_MASK                           (0x00000001u)
        #define ST_CSR_ENABLE_BIT                            (0)
        #define ST_CSR_ENABLE_BITS                           (1)

#define ST_RVR                                               *((volatile uint32_t *)0xE000E014u)
#define ST_RVR_REG                                           *((volatile uint32_t *)0xE000E014u)
#define ST_RVR_ADDR                                          (0xE000E014u)
#define ST_RVR_RESET                                         (0x00000000u)
        /* RELOAD field */
        #define ST_RVR_RELOAD                                (0x00FFFFFFu)
        #define ST_RVR_RELOAD_MASK                           (0x00FFFFFFu)
        #define ST_RVR_RELOAD_BIT                            (0)
        #define ST_RVR_RELOAD_BITS                           (24)

#define ST_CVR                                               *((volatile uint32_t *)0xE000E018u)
#define ST_CVR_REG                                           *((volatile uint32_t *)0xE000E018u)
#define ST_CVR_ADDR                                          (0xE000E018u)
#define ST_CVR_RESET                                         (0x00000000u)
        /* CURRENT field */
        #define ST_CVR_CURRENT                               (0xFFFFFFFFu)
        #define ST_CVR_CURRENT_MASK                          (0xFFFFFFFFu)
        #define ST_CVR_CURRENT_BIT                           (0)
        #define ST_CVR_CURRENT_BITS                          (32)

#define ST_CALVR                                             *((volatile uint32_t *)0xE000E01Cu)
#define ST_CALVR_REG                                         *((volatile uint32_t *)0xE000E01Cu)
#define ST_CALVR_ADDR                                        (0xE000E01Cu)
#define ST_CALVR_RESET                                       (0x00000000u)
        /* NOREF field */
        #define ST_CALVR_NOREF                               (0x80000000u)
        #define ST_CALVR_NOREF_MASK                          (0x80000000u)
        #define ST_CALVR_NOREF_BIT                           (31)
        #define ST_CALVR_NOREF_BITS                          (1)
        /* SKEW field */
        #define ST_CALVR_SKEW                                (0x40000000u)
        #define ST_CALVR_SKEW_MASK                           (0x40000000u)
        #define ST_CALVR_SKEW_BIT                            (30)
        #define ST_CALVR_SKEW_BITS                           (1)
        /* TENMS field */
        #define ST_CALVR_TENMS                               (0x00FFFFFFu)
        #define ST_CALVR_TENMS_MASK                          (0x00FFFFFFu)
        #define ST_CALVR_TENMS_BIT                           (0)
        #define ST_CALVR_TENMS_BITS                          (24)

#define INT_CFGSET                                           *((volatile uint32_t *)0xE000E100u)
#define INT_CFGSET_REG                                       *((volatile uint32_t *)0xE000E100u)
#define INT_CFGSET_ADDR                                      (0xE000E100u)
#define INT_CFGSET_RESET                                     (0x00000000u)
        /* INT_DEBUG field */
        #define INT_DEBUG                                    (0x00010000u)
        #define INT_DEBUG_MASK                               (0x00010000u)
        #define INT_DEBUG_BIT                                (16)
        #define INT_DEBUG_BITS                               (1)
        /* INT_IRQD field */
        #define INT_IRQD                                     (0x00008000u)
        #define INT_IRQD_MASK                                (0x00008000u)
        #define INT_IRQD_BIT                                 (15)
        #define INT_IRQD_BITS                                (1)
        /* INT_IRQC field */
        #define INT_IRQC                                     (0x00004000u)
        #define INT_IRQC_MASK                                (0x00004000u)
        #define INT_IRQC_BIT                                 (14)
        #define INT_IRQC_BITS                                (1)
        /* INT_IRQB field */
        #define INT_IRQB                                     (0x00002000u)
        #define INT_IRQB_MASK                                (0x00002000u)
        #define INT_IRQB_BIT                                 (13)
        #define INT_IRQB_BITS                                (1)
        /* INT_IRQA field */
        #define INT_IRQA                                     (0x00001000u)
        #define INT_IRQA_MASK                                (0x00001000u)
        #define INT_IRQA_BIT                                 (12)
        #define INT_IRQA_BITS                                (1)
        /* INT_ADC field */
        #define INT_ADC                                      (0x00000800u)
        #define INT_ADC_MASK                                 (0x00000800u)
        #define INT_ADC_BIT                                  (11)
        #define INT_ADC_BITS                                 (1)
        /* INT_MACRX field */
        #define INT_MACRX                                    (0x00000400u)
        #define INT_MACRX_MASK                               (0x00000400u)
        #define INT_MACRX_BIT                                (10)
        #define INT_MACRX_BITS                               (1)
        /* INT_MACTX field */
        #define INT_MACTX                                    (0x00000200u)
        #define INT_MACTX_MASK                               (0x00000200u)
        #define INT_MACTX_BIT                                (9)
        #define INT_MACTX_BITS                               (1)
        /* INT_MACTMR field */
        #define INT_MACTMR                                   (0x00000100u)
        #define INT_MACTMR_MASK                              (0x00000100u)
        #define INT_MACTMR_BIT                               (8)
        #define INT_MACTMR_BITS                              (1)
        /* INT_SEC field */
        #define INT_SEC                                      (0x00000080u)
        #define INT_SEC_MASK                                 (0x00000080u)
        #define INT_SEC_BIT                                  (7)
        #define INT_SEC_BITS                                 (1)
        /* INT_SC2 field */
        #define INT_SC2                                      (0x00000040u)
        #define INT_SC2_MASK                                 (0x00000040u)
        #define INT_SC2_BIT                                  (6)
        #define INT_SC2_BITS                                 (1)
        /* INT_SC1 field */
        #define INT_SC1                                      (0x00000020u)
        #define INT_SC1_MASK                                 (0x00000020u)
        #define INT_SC1_BIT                                  (5)
        #define INT_SC1_BITS                                 (1)
        /* INT_SLEEPTMR field */
        #define INT_SLEEPTMR                                 (0x00000010u)
        #define INT_SLEEPTMR_MASK                            (0x00000010u)
        #define INT_SLEEPTMR_BIT                             (4)
        #define INT_SLEEPTMR_BITS                            (1)
        /* INT_BB field */
        #define INT_BB                                       (0x00000008u)
        #define INT_BB_MASK                                  (0x00000008u)
        #define INT_BB_BIT                                   (3)
        #define INT_BB_BITS                                  (1)
        /* INT_MGMT field */
        #define INT_MGMT                                     (0x00000004u)
        #define INT_MGMT_MASK                                (0x00000004u)
        #define INT_MGMT_BIT                                 (2)
        #define INT_MGMT_BITS                                (1)
        /* INT_TIM2 field */
        #define INT_TIM2                                     (0x00000002u)
        #define INT_TIM2_MASK                                (0x00000002u)
        #define INT_TIM2_BIT                                 (1)
        #define INT_TIM2_BITS                                (1)
        /* INT_TIM1 field */
        #define INT_TIM1                                     (0x00000001u)
        #define INT_TIM1_MASK                                (0x00000001u)
        #define INT_TIM1_BIT                                 (0)
        #define INT_TIM1_BITS                                (1)

#define INT_CFGCLR                                           *((volatile uint32_t *)0xE000E180u)
#define INT_CFGCLR_REG                                       *((volatile uint32_t *)0xE000E180u)
#define INT_CFGCLR_ADDR                                      (0xE000E180u)
#define INT_CFGCLR_RESET                                     (0x00000000u)
        /* INT_DEBUG field */
        #define INT_DEBUG                                    (0x00010000u)
        #define INT_DEBUG_MASK                               (0x00010000u)
        #define INT_DEBUG_BIT                                (16)
        #define INT_DEBUG_BITS                               (1)
        /* INT_IRQD field */
        #define INT_IRQD                                     (0x00008000u)
        #define INT_IRQD_MASK                                (0x00008000u)
        #define INT_IRQD_BIT                                 (15)
        #define INT_IRQD_BITS                                (1)
        /* INT_IRQC field */
        #define INT_IRQC                                     (0x00004000u)
        #define INT_IRQC_MASK                                (0x00004000u)
        #define INT_IRQC_BIT                                 (14)
        #define INT_IRQC_BITS                                (1)
        /* INT_IRQB field */
        #define INT_IRQB                                     (0x00002000u)
        #define INT_IRQB_MASK                                (0x00002000u)
        #define INT_IRQB_BIT                                 (13)
        #define INT_IRQB_BITS                                (1)
        /* INT_IRQA field */
        #define INT_IRQA                                     (0x00001000u)
        #define INT_IRQA_MASK                                (0x00001000u)
        #define INT_IRQA_BIT                                 (12)
        #define INT_IRQA_BITS                                (1)
        /* INT_ADC field */
        #define INT_ADC                                      (0x00000800u)
        #define INT_ADC_MASK                                 (0x00000800u)
        #define INT_ADC_BIT                                  (11)
        #define INT_ADC_BITS                                 (1)
        /* INT_MACRX field */
        #define INT_MACRX                                    (0x00000400u)
        #define INT_MACRX_MASK                               (0x00000400u)
        #define INT_MACRX_BIT                                (10)
        #define INT_MACRX_BITS                               (1)
        /* INT_MACTX field */
        #define INT_MACTX                                    (0x00000200u)
        #define INT_MACTX_MASK                               (0x00000200u)
        #define INT_MACTX_BIT                                (9)
        #define INT_MACTX_BITS                               (1)
        /* INT_MACTMR field */
        #define INT_MACTMR                                   (0x00000100u)
        #define INT_MACTMR_MASK                              (0x00000100u)
        #define INT_MACTMR_BIT                               (8)
        #define INT_MACTMR_BITS                              (1)
        /* INT_SEC field */
        #define INT_SEC                                      (0x00000080u)
        #define INT_SEC_MASK                                 (0x00000080u)
        #define INT_SEC_BIT                                  (7)
        #define INT_SEC_BITS                                 (1)
        /* INT_SC2 field */
        #define INT_SC2                                      (0x00000040u)
        #define INT_SC2_MASK                                 (0x00000040u)
        #define INT_SC2_BIT                                  (6)
        #define INT_SC2_BITS                                 (1)
        /* INT_SC1 field */
        #define INT_SC1                                      (0x00000020u)
        #define INT_SC1_MASK                                 (0x00000020u)
        #define INT_SC1_BIT                                  (5)
        #define INT_SC1_BITS                                 (1)
        /* INT_SLEEPTMR field */
        #define INT_SLEEPTMR                                 (0x00000010u)
        #define INT_SLEEPTMR_MASK                            (0x00000010u)
        #define INT_SLEEPTMR_BIT                             (4)
        #define INT_SLEEPTMR_BITS                            (1)
        /* INT_BB field */
        #define INT_BB                                       (0x00000008u)
        #define INT_BB_MASK                                  (0x00000008u)
        #define INT_BB_BIT                                   (3)
        #define INT_BB_BITS                                  (1)
        /* INT_MGMT field */
        #define INT_MGMT                                     (0x00000004u)
        #define INT_MGMT_MASK                                (0x00000004u)
        #define INT_MGMT_BIT                                 (2)
        #define INT_MGMT_BITS                                (1)
        /* INT_TIM2 field */
        #define INT_TIM2                                     (0x00000002u)
        #define INT_TIM2_MASK                                (0x00000002u)
        #define INT_TIM2_BIT                                 (1)
        #define INT_TIM2_BITS                                (1)
        /* INT_TIM1 field */
        #define INT_TIM1                                     (0x00000001u)
        #define INT_TIM1_MASK                                (0x00000001u)
        #define INT_TIM1_BIT                                 (0)
        #define INT_TIM1_BITS                                (1)

#define INT_PENDSET                                          *((volatile uint32_t *)0xE000E200u)
#define INT_PENDSET_REG                                      *((volatile uint32_t *)0xE000E200u)
#define INT_PENDSET_ADDR                                     (0xE000E200u)
#define INT_PENDSET_RESET                                    (0x00000000u)
        /* INT_DEBUG field */
        #define INT_DEBUG                                    (0x00010000u)
        #define INT_DEBUG_MASK                               (0x00010000u)
        #define INT_DEBUG_BIT                                (16)
        #define INT_DEBUG_BITS                               (1)
        /* INT_IRQD field */
        #define INT_IRQD                                     (0x00008000u)
        #define INT_IRQD_MASK                                (0x00008000u)
        #define INT_IRQD_BIT                                 (15)
        #define INT_IRQD_BITS                                (1)
        /* INT_IRQC field */
        #define INT_IRQC                                     (0x00004000u)
        #define INT_IRQC_MASK                                (0x00004000u)
        #define INT_IRQC_BIT                                 (14)
        #define INT_IRQC_BITS                                (1)
        /* INT_IRQB field */
        #define INT_IRQB                                     (0x00002000u)
        #define INT_IRQB_MASK                                (0x00002000u)
        #define INT_IRQB_BIT                                 (13)
        #define INT_IRQB_BITS                                (1)
        /* INT_IRQA field */
        #define INT_IRQA                                     (0x00001000u)
        #define INT_IRQA_MASK                                (0x00001000u)
        #define INT_IRQA_BIT                                 (12)
        #define INT_IRQA_BITS                                (1)
        /* INT_ADC field */
        #define INT_ADC                                      (0x00000800u)
        #define INT_ADC_MASK                                 (0x00000800u)
        #define INT_ADC_BIT                                  (11)
        #define INT_ADC_BITS                                 (1)
        /* INT_MACRX field */
        #define INT_MACRX                                    (0x00000400u)
        #define INT_MACRX_MASK                               (0x00000400u)
        #define INT_MACRX_BIT                                (10)
        #define INT_MACRX_BITS                               (1)
        /* INT_MACTX field */
        #define INT_MACTX                                    (0x00000200u)
        #define INT_MACTX_MASK                               (0x00000200u)
        #define INT_MACTX_BIT                                (9)
        #define INT_MACTX_BITS                               (1)
        /* INT_MACTMR field */
        #define INT_MACTMR                                   (0x00000100u)
        #define INT_MACTMR_MASK                              (0x00000100u)
        #define INT_MACTMR_BIT                               (8)
        #define INT_MACTMR_BITS                              (1)
        /* INT_SEC field */
        #define INT_SEC                                      (0x00000080u)
        #define INT_SEC_MASK                                 (0x00000080u)
        #define INT_SEC_BIT                                  (7)
        #define INT_SEC_BITS                                 (1)
        /* INT_SC2 field */
        #define INT_SC2                                      (0x00000040u)
        #define INT_SC2_MASK                                 (0x00000040u)
        #define INT_SC2_BIT                                  (6)
        #define INT_SC2_BITS                                 (1)
        /* INT_SC1 field */
        #define INT_SC1                                      (0x00000020u)
        #define INT_SC1_MASK                                 (0x00000020u)
        #define INT_SC1_BIT                                  (5)
        #define INT_SC1_BITS                                 (1)
        /* INT_SLEEPTMR field */
        #define INT_SLEEPTMR                                 (0x00000010u)
        #define INT_SLEEPTMR_MASK                            (0x00000010u)
        #define INT_SLEEPTMR_BIT                             (4)
        #define INT_SLEEPTMR_BITS                            (1)
        /* INT_BB field */
        #define INT_BB                                       (0x00000008u)
        #define INT_BB_MASK                                  (0x00000008u)
        #define INT_BB_BIT                                   (3)
        #define INT_BB_BITS                                  (1)
        /* INT_MGMT field */
        #define INT_MGMT                                     (0x00000004u)
        #define INT_MGMT_MASK                                (0x00000004u)
        #define INT_MGMT_BIT                                 (2)
        #define INT_MGMT_BITS                                (1)
        /* INT_TIM2 field */
        #define INT_TIM2                                     (0x00000002u)
        #define INT_TIM2_MASK                                (0x00000002u)
        #define INT_TIM2_BIT                                 (1)
        #define INT_TIM2_BITS                                (1)
        /* INT_TIM1 field */
        #define INT_TIM1                                     (0x00000001u)
        #define INT_TIM1_MASK                                (0x00000001u)
        #define INT_TIM1_BIT                                 (0)
        #define INT_TIM1_BITS                                (1)

#define INT_PENDCLR                                          *((volatile uint32_t *)0xE000E280u)
#define INT_PENDCLR_REG                                      *((volatile uint32_t *)0xE000E280u)
#define INT_PENDCLR_ADDR                                     (0xE000E280u)
#define INT_PENDCLR_RESET                                    (0x00000000u)
        /* INT_DEBUG field */
        #define INT_DEBUG                                    (0x00010000u)
        #define INT_DEBUG_MASK                               (0x00010000u)
        #define INT_DEBUG_BIT                                (16)
        #define INT_DEBUG_BITS                               (1)
        /* INT_IRQD field */
        #define INT_IRQD                                     (0x00008000u)
        #define INT_IRQD_MASK                                (0x00008000u)
        #define INT_IRQD_BIT                                 (15)
        #define INT_IRQD_BITS                                (1)
        /* INT_IRQC field */
        #define INT_IRQC                                     (0x00004000u)
        #define INT_IRQC_MASK                                (0x00004000u)
        #define INT_IRQC_BIT                                 (14)
        #define INT_IRQC_BITS                                (1)
        /* INT_IRQB field */
        #define INT_IRQB                                     (0x00002000u)
        #define INT_IRQB_MASK                                (0x00002000u)
        #define INT_IRQB_BIT                                 (13)
        #define INT_IRQB_BITS                                (1)
        /* INT_IRQA field */
        #define INT_IRQA                                     (0x00001000u)
        #define INT_IRQA_MASK                                (0x00001000u)
        #define INT_IRQA_BIT                                 (12)
        #define INT_IRQA_BITS                                (1)
        /* INT_ADC field */
        #define INT_ADC                                      (0x00000800u)
        #define INT_ADC_MASK                                 (0x00000800u)
        #define INT_ADC_BIT                                  (11)
        #define INT_ADC_BITS                                 (1)
        /* INT_MACRX field */
        #define INT_MACRX                                    (0x00000400u)
        #define INT_MACRX_MASK                               (0x00000400u)
        #define INT_MACRX_BIT                                (10)
        #define INT_MACRX_BITS                               (1)
        /* INT_MACTX field */
        #define INT_MACTX                                    (0x00000200u)
        #define INT_MACTX_MASK                               (0x00000200u)
        #define INT_MACTX_BIT                                (9)
        #define INT_MACTX_BITS                               (1)
        /* INT_MACTMR field */
        #define INT_MACTMR                                   (0x00000100u)
        #define INT_MACTMR_MASK                              (0x00000100u)
        #define INT_MACTMR_BIT                               (8)
        #define INT_MACTMR_BITS                              (1)
        /* INT_SEC field */
        #define INT_SEC                                      (0x00000080u)
        #define INT_SEC_MASK                                 (0x00000080u)
        #define INT_SEC_BIT                                  (7)
        #define INT_SEC_BITS                                 (1)
        /* INT_SC2 field */
        #define INT_SC2                                      (0x00000040u)
        #define INT_SC2_MASK                                 (0x00000040u)
        #define INT_SC2_BIT                                  (6)
        #define INT_SC2_BITS                                 (1)
        /* INT_SC1 field */
        #define INT_SC1                                      (0x00000020u)
        #define INT_SC1_MASK                                 (0x00000020u)
        #define INT_SC1_BIT                                  (5)
        #define INT_SC1_BITS                                 (1)
        /* INT_SLEEPTMR field */
        #define INT_SLEEPTMR                                 (0x00000010u)
        #define INT_SLEEPTMR_MASK                            (0x00000010u)
        #define INT_SLEEPTMR_BIT                             (4)
        #define INT_SLEEPTMR_BITS                            (1)
        /* INT_BB field */
        #define INT_BB                                       (0x00000008u)
        #define INT_BB_MASK                                  (0x00000008u)
        #define INT_BB_BIT                                   (3)
        #define INT_BB_BITS                                  (1)
        /* INT_MGMT field */
        #define INT_MGMT                                     (0x00000004u)
        #define INT_MGMT_MASK                                (0x00000004u)
        #define INT_MGMT_BIT                                 (2)
        #define INT_MGMT_BITS                                (1)
        /* INT_TIM2 field */
        #define INT_TIM2                                     (0x00000002u)
        #define INT_TIM2_MASK                                (0x00000002u)
        #define INT_TIM2_BIT                                 (1)
        #define INT_TIM2_BITS                                (1)
        /* INT_TIM1 field */
        #define INT_TIM1                                     (0x00000001u)
        #define INT_TIM1_MASK                                (0x00000001u)
        #define INT_TIM1_BIT                                 (0)
        #define INT_TIM1_BITS                                (1)

#define INT_ACTIVE                                           *((volatile uint32_t *)0xE000E300u)
#define INT_ACTIVE_REG                                       *((volatile uint32_t *)0xE000E300u)
#define INT_ACTIVE_ADDR                                      (0xE000E300u)
#define INT_ACTIVE_RESET                                     (0x00000000u)
        /* INT_DEBUG field */
        #define INT_DEBUG                                    (0x00010000u)
        #define INT_DEBUG_MASK                               (0x00010000u)
        #define INT_DEBUG_BIT                                (16)
        #define INT_DEBUG_BITS                               (1)
        /* INT_IRQD field */
        #define INT_IRQD                                     (0x00008000u)
        #define INT_IRQD_MASK                                (0x00008000u)
        #define INT_IRQD_BIT                                 (15)
        #define INT_IRQD_BITS                                (1)
        /* INT_IRQC field */
        #define INT_IRQC                                     (0x00004000u)
        #define INT_IRQC_MASK                                (0x00004000u)
        #define INT_IRQC_BIT                                 (14)
        #define INT_IRQC_BITS                                (1)
        /* INT_IRQB field */
        #define INT_IRQB                                     (0x00002000u)
        #define INT_IRQB_MASK                                (0x00002000u)
        #define INT_IRQB_BIT                                 (13)
        #define INT_IRQB_BITS                                (1)
        /* INT_IRQA field */
        #define INT_IRQA                                     (0x00001000u)
        #define INT_IRQA_MASK                                (0x00001000u)
        #define INT_IRQA_BIT                                 (12)
        #define INT_IRQA_BITS                                (1)
        /* INT_ADC field */
        #define INT_ADC                                      (0x00000800u)
        #define INT_ADC_MASK                                 (0x00000800u)
        #define INT_ADC_BIT                                  (11)
        #define INT_ADC_BITS                                 (1)
        /* INT_MACRX field */
        #define INT_MACRX                                    (0x00000400u)
        #define INT_MACRX_MASK                               (0x00000400u)
        #define INT_MACRX_BIT                                (10)
        #define INT_MACRX_BITS                               (1)
        /* INT_MACTX field */
        #define INT_MACTX                                    (0x00000200u)
        #define INT_MACTX_MASK                               (0x00000200u)
        #define INT_MACTX_BIT                                (9)
        #define INT_MACTX_BITS                               (1)
        /* INT_MACTMR field */
        #define INT_MACTMR                                   (0x00000100u)
        #define INT_MACTMR_MASK                              (0x00000100u)
        #define INT_MACTMR_BIT                               (8)
        #define INT_MACTMR_BITS                              (1)
        /* INT_SEC field */
        #define INT_SEC                                      (0x00000080u)
        #define INT_SEC_MASK                                 (0x00000080u)
        #define INT_SEC_BIT                                  (7)
        #define INT_SEC_BITS                                 (1)
        /* INT_SC2 field */
        #define INT_SC2                                      (0x00000040u)
        #define INT_SC2_MASK                                 (0x00000040u)
        #define INT_SC2_BIT                                  (6)
        #define INT_SC2_BITS                                 (1)
        /* INT_SC1 field */
        #define INT_SC1                                      (0x00000020u)
        #define INT_SC1_MASK                                 (0x00000020u)
        #define INT_SC1_BIT                                  (5)
        #define INT_SC1_BITS                                 (1)
        /* INT_SLEEPTMR field */
        #define INT_SLEEPTMR                                 (0x00000010u)
        #define INT_SLEEPTMR_MASK                            (0x00000010u)
        #define INT_SLEEPTMR_BIT                             (4)
        #define INT_SLEEPTMR_BITS                            (1)
        /* INT_BB field */
        #define INT_BB                                       (0x00000008u)
        #define INT_BB_MASK                                  (0x00000008u)
        #define INT_BB_BIT                                   (3)
        #define INT_BB_BITS                                  (1)
        /* INT_MGMT field */
        #define INT_MGMT                                     (0x00000004u)
        #define INT_MGMT_MASK                                (0x00000004u)
        #define INT_MGMT_BIT                                 (2)
        #define INT_MGMT_BITS                                (1)
        /* INT_TIM2 field */
        #define INT_TIM2                                     (0x00000002u)
        #define INT_TIM2_MASK                                (0x00000002u)
        #define INT_TIM2_BIT                                 (1)
        #define INT_TIM2_BITS                                (1)
        /* INT_TIM1 field */
        #define INT_TIM1                                     (0x00000001u)
        #define INT_TIM1_MASK                                (0x00000001u)
        #define INT_TIM1_BIT                                 (0)
        #define INT_TIM1_BITS                                (1)

#define NVIC_IPR_3to0                                        *((volatile uint32_t *)0xE000E400u)
#define NVIC_IPR_3to0_REG                                    *((volatile uint32_t *)0xE000E400u)
#define NVIC_IPR_3to0_ADDR                                   (0xE000E400u)
#define NVIC_IPR_3to0_RESET                                  (0x00000000u)
        /* PRI_3 field */
        #define NVIC_IPR_3to0_PRI_3                          (0xFF000000u)
        #define NVIC_IPR_3to0_PRI_3_MASK                     (0xFF000000u)
        #define NVIC_IPR_3to0_PRI_3_BIT                      (24)
        #define NVIC_IPR_3to0_PRI_3_BITS                     (8)
        /* PRI_2 field */
        #define NVIC_IPR_3to0_PRI_2                          (0x00FF0000u)
        #define NVIC_IPR_3to0_PRI_2_MASK                     (0x00FF0000u)
        #define NVIC_IPR_3to0_PRI_2_BIT                      (16)
        #define NVIC_IPR_3to0_PRI_2_BITS                     (8)
        /* PRI_1 field */
        #define NVIC_IPR_3to0_PRI_1                          (0x0000FF00u)
        #define NVIC_IPR_3to0_PRI_1_MASK                     (0x0000FF00u)
        #define NVIC_IPR_3to0_PRI_1_BIT                      (8)
        #define NVIC_IPR_3to0_PRI_1_BITS                     (8)
        /* PRI_0 field */
        #define NVIC_IPR_3to0_PRI_0                          (0x000000FFu)
        #define NVIC_IPR_3to0_PRI_0_MASK                     (0x000000FFu)
        #define NVIC_IPR_3to0_PRI_0_BIT                      (0)
        #define NVIC_IPR_3to0_PRI_0_BITS                     (8)

#define NVIC_IPR_7to4                                        *((volatile uint32_t *)0xE000E404u)
#define NVIC_IPR_7to4_REG                                    *((volatile uint32_t *)0xE000E404u)
#define NVIC_IPR_7to4_ADDR                                   (0xE000E404u)
#define NVIC_IPR_7to4_RESET                                  (0x00000000u)
        /* PRI_7 field */
        #define NVIC_IPR_7to4_PRI_7                          (0xFF000000u)
        #define NVIC_IPR_7to4_PRI_7_MASK                     (0xFF000000u)
        #define NVIC_IPR_7to4_PRI_7_BIT                      (24)
        #define NVIC_IPR_7to4_PRI_7_BITS                     (8)
        /* PRI_6 field */
        #define NVIC_IPR_7to4_PRI_6                          (0x00FF0000u)
        #define NVIC_IPR_7to4_PRI_6_MASK                     (0x00FF0000u)
        #define NVIC_IPR_7to4_PRI_6_BIT                      (16)
        #define NVIC_IPR_7to4_PRI_6_BITS                     (8)
        /* PRI_5 field */
        #define NVIC_IPR_7to4_PRI_5                          (0x0000FF00u)
        #define NVIC_IPR_7to4_PRI_5_MASK                     (0x0000FF00u)
        #define NVIC_IPR_7to4_PRI_5_BIT                      (8)
        #define NVIC_IPR_7to4_PRI_5_BITS                     (8)
        /* PRI_4 field */
        #define NVIC_IPR_7to4_PRI_4                          (0x000000FFu)
        #define NVIC_IPR_7to4_PRI_4_MASK                     (0x000000FFu)
        #define NVIC_IPR_7to4_PRI_4_BIT                      (0)
        #define NVIC_IPR_7to4_PRI_4_BITS                     (8)

#define NVIC_IPR_11to8                                       *((volatile uint32_t *)0xE000E408u)
#define NVIC_IPR_11to8_REG                                   *((volatile uint32_t *)0xE000E408u)
#define NVIC_IPR_11to8_ADDR                                  (0xE000E408u)
#define NVIC_IPR_11to8_RESET                                 (0x00000000u)
        /* PRI_11 field */
        #define NVIC_IPR_11to8_PRI_11                        (0xFF000000u)
        #define NVIC_IPR_11to8_PRI_11_MASK                   (0xFF000000u)
        #define NVIC_IPR_11to8_PRI_11_BIT                    (24)
        #define NVIC_IPR_11to8_PRI_11_BITS                   (8)
        /* PRI_10 field */
        #define NVIC_IPR_11to8_PRI_10                        (0x00FF0000u)
        #define NVIC_IPR_11to8_PRI_10_MASK                   (0x00FF0000u)
        #define NVIC_IPR_11to8_PRI_10_BIT                    (16)
        #define NVIC_IPR_11to8_PRI_10_BITS                   (8)
        /* PRI_9 field */
        #define NVIC_IPR_11to8_PRI_9                         (0x0000FF00u)
        #define NVIC_IPR_11to8_PRI_9_MASK                    (0x0000FF00u)
        #define NVIC_IPR_11to8_PRI_9_BIT                     (8)
        #define NVIC_IPR_11to8_PRI_9_BITS                    (8)
        /* PRI_8 field */
        #define NVIC_IPR_11to8_PRI_8                         (0x000000FFu)
        #define NVIC_IPR_11to8_PRI_8_MASK                    (0x000000FFu)
        #define NVIC_IPR_11to8_PRI_8_BIT                     (0)
        #define NVIC_IPR_11to8_PRI_8_BITS                    (8)

#define NVIC_IPR_15to12                                      *((volatile uint32_t *)0xE000E40Cu)
#define NVIC_IPR_15to12_REG                                  *((volatile uint32_t *)0xE000E40Cu)
#define NVIC_IPR_15to12_ADDR                                 (0xE000E40Cu)
#define NVIC_IPR_15to12_RESET                                (0x00000000u)
        /* PRI_15 field */
        #define NVIC_IPR_15to12_PRI_15                       (0xFF000000u)
        #define NVIC_IPR_15to12_PRI_15_MASK                  (0xFF000000u)
        #define NVIC_IPR_15to12_PRI_15_BIT                   (24)
        #define NVIC_IPR_15to12_PRI_15_BITS                  (8)
        /* PRI_14 field */
        #define NVIC_IPR_15to12_PRI_14                       (0x00FF0000u)
        #define NVIC_IPR_15to12_PRI_14_MASK                  (0x00FF0000u)
        #define NVIC_IPR_15to12_PRI_14_BIT                   (16)
        #define NVIC_IPR_15to12_PRI_14_BITS                  (8)
        /* PRI_13 field */
        #define NVIC_IPR_15to12_PRI_13                       (0x0000FF00u)
        #define NVIC_IPR_15to12_PRI_13_MASK                  (0x0000FF00u)
        #define NVIC_IPR_15to12_PRI_13_BIT                   (8)
        #define NVIC_IPR_15to12_PRI_13_BITS                  (8)
        /* PRI_12 field */
        #define NVIC_IPR_15to12_PRI_12                       (0x000000FFu)
        #define NVIC_IPR_15to12_PRI_12_MASK                  (0x000000FFu)
        #define NVIC_IPR_15to12_PRI_12_BIT                   (0)
        #define NVIC_IPR_15to12_PRI_12_BITS                  (8)

#define NVIC_IPR_19to16                                      *((volatile uint32_t *)0xE000E410u)
#define NVIC_IPR_19to16_REG                                  *((volatile uint32_t *)0xE000E410u)
#define NVIC_IPR_19to16_ADDR                                 (0xE000E410u)
#define NVIC_IPR_19to16_RESET                                (0x00000000u)
        /* PRI_19 field */
        #define NVIC_IPR_19to16_PRI_19                       (0xFF000000u)
        #define NVIC_IPR_19to16_PRI_19_MASK                  (0xFF000000u)
        #define NVIC_IPR_19to16_PRI_19_BIT                   (24)
        #define NVIC_IPR_19to16_PRI_19_BITS                  (8)
        /* PRI_18 field */
        #define NVIC_IPR_19to16_PRI_18                       (0x00FF0000u)
        #define NVIC_IPR_19to16_PRI_18_MASK                  (0x00FF0000u)
        #define NVIC_IPR_19to16_PRI_18_BIT                   (16)
        #define NVIC_IPR_19to16_PRI_18_BITS                  (8)
        /* PRI_17 field */
        #define NVIC_IPR_19to16_PRI_17                       (0x0000FF00u)
        #define NVIC_IPR_19to16_PRI_17_MASK                  (0x0000FF00u)
        #define NVIC_IPR_19to16_PRI_17_BIT                   (8)
        #define NVIC_IPR_19to16_PRI_17_BITS                  (8)
        /* PRI_16 field */
        #define NVIC_IPR_19to16_PRI_16                       (0x000000FFu)
        #define NVIC_IPR_19to16_PRI_16_MASK                  (0x000000FFu)
        #define NVIC_IPR_19to16_PRI_16_BIT                   (0)
        #define NVIC_IPR_19to16_PRI_16_BITS                  (8)

#define SCS_CPUID                                            *((volatile uint32_t *)0xE000ED00u)
#define SCS_CPUID_REG                                        *((volatile uint32_t *)0xE000ED00u)
#define SCS_CPUID_ADDR                                       (0xE000ED00u)
#define SCS_CPUID_RESET                                      (0x411FC231u)
        /* IMPLEMENTER field */
        #define SCS_CPUID_IMPLEMENTER                        (0xFF000000u)
        #define SCS_CPUID_IMPLEMENTER_MASK                   (0xFF000000u)
        #define SCS_CPUID_IMPLEMENTER_BIT                    (24)
        #define SCS_CPUID_IMPLEMENTER_BITS                   (8)
        /* VARIANT field */
        #define SCS_CPUID_VARIANT                            (0x00F00000u)
        #define SCS_CPUID_VARIANT_MASK                       (0x00F00000u)
        #define SCS_CPUID_VARIANT_BIT                        (20)
        #define SCS_CPUID_VARIANT_BITS                       (4)
        /* CONSTANT field */
        #define SCS_CPUID_CONSTANT                           (0x000F0000u)
        #define SCS_CPUID_CONSTANT_MASK                      (0x000F0000u)
        #define SCS_CPUID_CONSTANT_BIT                       (16)
        #define SCS_CPUID_CONSTANT_BITS                      (4)
        /* PARTNO field */
        #define SCS_CPUID_PARTNO                             (0x0000FFF0u)
        #define SCS_CPUID_PARTNO_MASK                        (0x0000FFF0u)
        #define SCS_CPUID_PARTNO_BIT                         (4)
        #define SCS_CPUID_PARTNO_BITS                        (12)
        /* REVISION field */
        #define SCS_CPUID_REVISION                           (0x0000000Fu)
        #define SCS_CPUID_REVISION_MASK                      (0x0000000Fu)
        #define SCS_CPUID_REVISION_BIT                       (0)
        #define SCS_CPUID_REVISION_BITS                      (4)

#define SCS_ICSR                                             *((volatile uint32_t *)0xE000ED04u)
#define SCS_ICSR_REG                                         *((volatile uint32_t *)0xE000ED04u)
#define SCS_ICSR_ADDR                                        (0xE000ED04u)
#define SCS_ICSR_RESET                                       (0x00000000u)
        /* NMIPENDSET field */
        #define SCS_ICSR_NMIPENDSET                          (0x80000000u)
        #define SCS_ICSR_NMIPENDSET_MASK                     (0x80000000u)
        #define SCS_ICSR_NMIPENDSET_BIT                      (31)
        #define SCS_ICSR_NMIPENDSET_BITS                     (1)
        /* PENDSVSET field */
        #define SCS_ICSR_PENDSVSET                           (0x10000000u)
        #define SCS_ICSR_PENDSVSET_MASK                      (0x10000000u)
        #define SCS_ICSR_PENDSVSET_BIT                       (28)
        #define SCS_ICSR_PENDSVSET_BITS                      (1)
        /* PENDSVCLR field */
        #define SCS_ICSR_PENDSVCLR                           (0x08000000u)
        #define SCS_ICSR_PENDSVCLR_MASK                      (0x08000000u)
        #define SCS_ICSR_PENDSVCLR_BIT                       (27)
        #define SCS_ICSR_PENDSVCLR_BITS                      (1)
        /* PENDSTSET field */
        #define SCS_ICSR_PENDSTSET                           (0x04000000u)
        #define SCS_ICSR_PENDSTSET_MASK                      (0x04000000u)
        #define SCS_ICSR_PENDSTSET_BIT                       (26)
        #define SCS_ICSR_PENDSTSET_BITS                      (1)
        /* PENDSTCLR field */
        #define SCS_ICSR_PENDSTCLR                           (0x02000000u)
        #define SCS_ICSR_PENDSTCLR_MASK                      (0x02000000u)
        #define SCS_ICSR_PENDSTCLR_BIT                       (25)
        #define SCS_ICSR_PENDSTCLR_BITS                      (1)
        /* ISRPREEMPT field */
        #define SCS_ICSR_ISRPREEMPT                          (0x00800000u)
        #define SCS_ICSR_ISRPREEMPT_MASK                     (0x00800000u)
        #define SCS_ICSR_ISRPREEMPT_BIT                      (23)
        #define SCS_ICSR_ISRPREEMPT_BITS                     (1)
        /* ISRPENDING field */
        #define SCS_ICSR_ISRPENDING                          (0x00400000u)
        #define SCS_ICSR_ISRPENDING_MASK                     (0x00400000u)
        #define SCS_ICSR_ISRPENDING_BIT                      (22)
        #define SCS_ICSR_ISRPENDING_BITS                     (1)
        /* VECTPENDING field */
        #define SCS_ICSR_VECTPENDING                         (0x001FF000u)
        #define SCS_ICSR_VECTPENDING_MASK                    (0x001FF000u)
        #define SCS_ICSR_VECTPENDING_BIT                     (12)
        #define SCS_ICSR_VECTPENDING_BITS                    (9)
        /* RETTOBASE field */
        #define SCS_ICSR_RETTOBASE                           (0x00000800u)
        #define SCS_ICSR_RETTOBASE_MASK                      (0x00000800u)
        #define SCS_ICSR_RETTOBASE_BIT                       (11)
        #define SCS_ICSR_RETTOBASE_BITS                      (1)
        /* VECACTIVE field */
        #define SCS_ICSR_VECACTIVE                           (0x000001FFu)
        #define SCS_ICSR_VECACTIVE_MASK                      (0x000001FFu)
        #define SCS_ICSR_VECACTIVE_BIT                       (0)
        #define SCS_ICSR_VECACTIVE_BITS                      (9)

#define SCS_VTOR                                             *((volatile uint32_t *)0xE000ED08u)
#define SCS_VTOR_REG                                         *((volatile uint32_t *)0xE000ED08u)
#define SCS_VTOR_ADDR                                        (0xE000ED08u)
#define SCS_VTOR_RESET                                       (0x00000000u)
        /* TBLBASE field */
        #define SCS_VTOR_TBLBASE                             (0x20000000u)
        #define SCS_VTOR_TBLBASE_MASK                        (0x20000000u)
        #define SCS_VTOR_TBLBASE_BIT                         (29)
        #define SCS_VTOR_TBLBASE_BITS                        (1)
        /* TBLOFF field */
        #define SCS_VTOR_TBLOFF                              (0x1FFFFF00u)
        #define SCS_VTOR_TBLOFF_MASK                         (0x1FFFFF00u)
        #define SCS_VTOR_TBLOFF_BIT                          (8)
        #define SCS_VTOR_TBLOFF_BITS                         (21)

#define SCS_AIRCR                                            *((volatile uint32_t *)0xE000ED0Cu)
#define SCS_AIRCR_REG                                        *((volatile uint32_t *)0xE000ED0Cu)
#define SCS_AIRCR_ADDR                                       (0xE000ED0Cu)
#define SCS_AIRCR_RESET                                      (0x00000000u)
        /* VECTKEYSTAT field */
        #define SCS_AIRCR_VECTKEYSTAT                        (0xFFFF0000u)
        #define SCS_AIRCR_VECTKEYSTAT_MASK                   (0xFFFF0000u)
        #define SCS_AIRCR_VECTKEYSTAT_BIT                    (16)
        #define SCS_AIRCR_VECTKEYSTAT_BITS                   (16)
        /* VECTKEY field */
        #define SCS_AIRCR_VECTKEY                            (0xFFFF0000u)
        #define SCS_AIRCR_VECTKEY_MASK                       (0xFFFF0000u)
        #define SCS_AIRCR_VECTKEY_BIT                        (16)
        #define SCS_AIRCR_VECTKEY_BITS                       (16)
        /* ENDIANESS field */
        #define SCS_AIRCR_ENDIANESS                          (0x00008000u)
        #define SCS_AIRCR_ENDIANESS_MASK                     (0x00008000u)
        #define SCS_AIRCR_ENDIANESS_BIT                      (15)
        #define SCS_AIRCR_ENDIANESS_BITS                     (1)
        /* PRIGROUP field */
        #define SCS_AIRCR_PRIGROUP                           (0x00000700u)
        #define SCS_AIRCR_PRIGROUP_MASK                      (0x00000700u)
        #define SCS_AIRCR_PRIGROUP_BIT                       (8)
        #define SCS_AIRCR_PRIGROUP_BITS                      (3)
        /* SYSRESETREQ field */
        #define SCS_AIRCR_SYSRESETREQ                        (0x00000004u)
        #define SCS_AIRCR_SYSRESETREQ_MASK                   (0x00000004u)
        #define SCS_AIRCR_SYSRESETREQ_BIT                    (2)
        #define SCS_AIRCR_SYSRESETREQ_BITS                   (1)
        /* VECTCLRACTIVE field */
        #define SCS_AIRCR_VECTCLRACTIVE                      (0x00000002u)
        #define SCS_AIRCR_VECTCLRACTIVE_MASK                 (0x00000002u)
        #define SCS_AIRCR_VECTCLRACTIVE_BIT                  (1)
        #define SCS_AIRCR_VECTCLRACTIVE_BITS                 (1)
        /* VECTRESET field */
        #define SCS_AIRCR_VECTRESET                          (0x00000001u)
        #define SCS_AIRCR_VECTRESET_MASK                     (0x00000001u)
        #define SCS_AIRCR_VECTRESET_BIT                      (0)
        #define SCS_AIRCR_VECTRESET_BITS                     (1)

#define SCS_SCR                                              *((volatile uint32_t *)0xE000ED10u)
#define SCS_SCR_REG                                          *((volatile uint32_t *)0xE000ED10u)
#define SCS_SCR_ADDR                                         (0xE000ED10u)
#define SCS_SCR_RESET                                        (0x00000000u)
        /* SEVONPEND field */
        #define SCS_SCR_SEVONPEND                            (0x00000010u)
        #define SCS_SCR_SEVONPEND_MASK                       (0x00000010u)
        #define SCS_SCR_SEVONPEND_BIT                        (4)
        #define SCS_SCR_SEVONPEND_BITS                       (1)
        /* SLEEPDEEP field */
        #define SCS_SCR_SLEEPDEEP                            (0x00000004u)
        #define SCS_SCR_SLEEPDEEP_MASK                       (0x00000004u)
        #define SCS_SCR_SLEEPDEEP_BIT                        (2)
        #define SCS_SCR_SLEEPDEEP_BITS                       (1)
        /* SLEEPONEXIT field */
        #define SCS_SCR_SLEEPONEXIT                          (0x00000002u)
        #define SCS_SCR_SLEEPONEXIT_MASK                     (0x00000002u)
        #define SCS_SCR_SLEEPONEXIT_BIT                      (1)
        #define SCS_SCR_SLEEPONEXIT_BITS                     (1)

#define SCS_CCR                                              *((volatile uint32_t *)0xE000ED14u)
#define SCS_CCR_REG                                          *((volatile uint32_t *)0xE000ED14u)
#define SCS_CCR_ADDR                                         (0xE000ED14u)
#define SCS_CCR_RESET                                        (0x00000000u)
        /* STKALIGN field */
        #define SCS_CCR_STKALIGN                             (0x00000200u)
        #define SCS_CCR_STKALIGN_MASK                        (0x00000200u)
        #define SCS_CCR_STKALIGN_BIT                         (9)
        #define SCS_CCR_STKALIGN_BITS                        (1)
        /* BFHFNMIGN field */
        #define SCS_CCR_BFHFNMIGN                            (0x00000100u)
        #define SCS_CCR_BFHFNMIGN_MASK                       (0x00000100u)
        #define SCS_CCR_BFHFNMIGN_BIT                        (8)
        #define SCS_CCR_BFHFNMIGN_BITS                       (1)
        /* DIV_0_TRP field */
        #define SCS_CCR_DIV_0_TRP                            (0x00000010u)
        #define SCS_CCR_DIV_0_TRP_MASK                       (0x00000010u)
        #define SCS_CCR_DIV_0_TRP_BIT                        (4)
        #define SCS_CCR_DIV_0_TRP_BITS                       (1)
        /* UNALIGN_TRP field */
        #define SCS_CCR_UNALIGN_TRP                          (0x00000008u)
        #define SCS_CCR_UNALIGN_TRP_MASK                     (0x00000008u)
        #define SCS_CCR_UNALIGN_TRP_BIT                      (3)
        #define SCS_CCR_UNALIGN_TRP_BITS                     (1)
        /* USERSETMPEND field */
        #define SCS_CCR_USERSETMPEND                         (0x00000002u)
        #define SCS_CCR_USERSETMPEND_MASK                    (0x00000002u)
        #define SCS_CCR_USERSETMPEND_BIT                     (1)
        #define SCS_CCR_USERSETMPEND_BITS                    (1)
        /* NONBASETHRDENA field */
        #define SCS_CCR_NONBASETHRDENA                       (0x00000001u)
        #define SCS_CCR_NONBASETHRDENA_MASK                  (0x00000001u)
        #define SCS_CCR_NONBASETHRDENA_BIT                   (0)
        #define SCS_CCR_NONBASETHRDENA_BITS                  (1)

#define SCS_SHPR_7to4                                        *((volatile uint32_t *)0xE000ED18u)
#define SCS_SHPR_7to4_REG                                    *((volatile uint32_t *)0xE000ED18u)
#define SCS_SHPR_7to4_ADDR                                   (0xE000ED18u)
#define SCS_SHPR_7to4_RESET                                  (0x00000000u)
        /* PRI_7 field */
        #define SCS_SHPR_7to4_PRI_7                          (0xFF000000u)
        #define SCS_SHPR_7to4_PRI_7_MASK                     (0xFF000000u)
        #define SCS_SHPR_7to4_PRI_7_BIT                      (24)
        #define SCS_SHPR_7to4_PRI_7_BITS                     (8)
        /* PRI_6 field */
        #define SCS_SHPR_7to4_PRI_6                          (0x00FF0000u)
        #define SCS_SHPR_7to4_PRI_6_MASK                     (0x00FF0000u)
        #define SCS_SHPR_7to4_PRI_6_BIT                      (16)
        #define SCS_SHPR_7to4_PRI_6_BITS                     (8)
        /* PRI_5 field */
        #define SCS_SHPR_7to4_PRI_5                          (0x0000FF00u)
        #define SCS_SHPR_7to4_PRI_5_MASK                     (0x0000FF00u)
        #define SCS_SHPR_7to4_PRI_5_BIT                      (8)
        #define SCS_SHPR_7to4_PRI_5_BITS                     (8)
        /* PRI_4 field */
        #define SCS_SHPR_7to4_PRI_4                          (0x000000FFu)
        #define SCS_SHPR_7to4_PRI_4_MASK                     (0x000000FFu)
        #define SCS_SHPR_7to4_PRI_4_BIT                      (0)
        #define SCS_SHPR_7to4_PRI_4_BITS                     (8)

#define SCS_SHPR_11to8                                       *((volatile uint32_t *)0xE000ED1Cu)
#define SCS_SHPR_11to8_REG                                   *((volatile uint32_t *)0xE000ED1Cu)
#define SCS_SHPR_11to8_ADDR                                  (0xE000ED1Cu)
#define SCS_SHPR_11to8_RESET                                 (0x00000000u)
        /* PRI_11 field */
        #define SCS_SHPR_11to8_PRI_11                        (0xFF000000u)
        #define SCS_SHPR_11to8_PRI_11_MASK                   (0xFF000000u)
        #define SCS_SHPR_11to8_PRI_11_BIT                    (24)
        #define SCS_SHPR_11to8_PRI_11_BITS                   (8)
        /* PRI_10 field */
        #define SCS_SHPR_11to8_PRI_10                        (0x00FF0000u)
        #define SCS_SHPR_11to8_PRI_10_MASK                   (0x00FF0000u)
        #define SCS_SHPR_11to8_PRI_10_BIT                    (16)
        #define SCS_SHPR_11to8_PRI_10_BITS                   (8)
        /* PRI_9 field */
        #define SCS_SHPR_11to8_PRI_9                         (0x0000FF00u)
        #define SCS_SHPR_11to8_PRI_9_MASK                    (0x0000FF00u)
        #define SCS_SHPR_11to8_PRI_9_BIT                     (8)
        #define SCS_SHPR_11to8_PRI_9_BITS                    (8)
        /* PRI_8 field */
        #define SCS_SHPR_11to8_PRI_8                         (0x000000FFu)
        #define SCS_SHPR_11to8_PRI_8_MASK                    (0x000000FFu)
        #define SCS_SHPR_11to8_PRI_8_BIT                     (0)
        #define SCS_SHPR_11to8_PRI_8_BITS                    (8)

#define SCS_SHPR_15to12                                      *((volatile uint32_t *)0xE000ED20u)
#define SCS_SHPR_15to12_REG                                  *((volatile uint32_t *)0xE000ED20u)
#define SCS_SHPR_15to12_ADDR                                 (0xE000ED20u)
#define SCS_SHPR_15to12_RESET                                (0x00000000u)
        /* PRI_15 field */
        #define SCS_SHPR_15to12_PRI_15                       (0xFF000000u)
        #define SCS_SHPR_15to12_PRI_15_MASK                  (0xFF000000u)
        #define SCS_SHPR_15to12_PRI_15_BIT                   (24)
        #define SCS_SHPR_15to12_PRI_15_BITS                  (8)
        /* PRI_14 field */
        #define SCS_SHPR_15to12_PRI_14                       (0x00FF0000u)
        #define SCS_SHPR_15to12_PRI_14_MASK                  (0x00FF0000u)
        #define SCS_SHPR_15to12_PRI_14_BIT                   (16)
        #define SCS_SHPR_15to12_PRI_14_BITS                  (8)
        /* PRI_13 field */
        #define SCS_SHPR_15to12_PRI_13                       (0x0000FF00u)
        #define SCS_SHPR_15to12_PRI_13_MASK                  (0x0000FF00u)
        #define SCS_SHPR_15to12_PRI_13_BIT                   (8)
        #define SCS_SHPR_15to12_PRI_13_BITS                  (8)
        /* PRI_12 field */
        #define SCS_SHPR_15to12_PRI_12                       (0x000000FFu)
        #define SCS_SHPR_15to12_PRI_12_MASK                  (0x000000FFu)
        #define SCS_SHPR_15to12_PRI_12_BIT                   (0)
        #define SCS_SHPR_15to12_PRI_12_BITS                  (8)

#define SCS_SHCSR                                            *((volatile uint32_t *)0xE000ED24u)
#define SCS_SHCSR_REG                                        *((volatile uint32_t *)0xE000ED24u)
#define SCS_SHCSR_ADDR                                       (0xE000ED24u)
#define SCS_SHCSR_RESET                                      (0x00000000u)
        /* USGFAULTENA field */
        #define SCS_SHCSR_USGFAULTENA                        (0x00040000u)
        #define SCS_SHCSR_USGFAULTENA_MASK                   (0x00040000u)
        #define SCS_SHCSR_USGFAULTENA_BIT                    (18)
        #define SCS_SHCSR_USGFAULTENA_BITS                   (1)
        /* BUSFAULTENA field */
        #define SCS_SHCSR_BUSFAULTENA                        (0x00020000u)
        #define SCS_SHCSR_BUSFAULTENA_MASK                   (0x00020000u)
        #define SCS_SHCSR_BUSFAULTENA_BIT                    (17)
        #define SCS_SHCSR_BUSFAULTENA_BITS                   (1)
        /* MEMFAULTENA field */
        #define SCS_SHCSR_MEMFAULTENA                        (0x00010000u)
        #define SCS_SHCSR_MEMFAULTENA_MASK                   (0x00010000u)
        #define SCS_SHCSR_MEMFAULTENA_BIT                    (16)
        #define SCS_SHCSR_MEMFAULTENA_BITS                   (1)
        /* SVCALLPENDED field */
        #define SCS_SHCSR_SVCALLPENDED                       (0x00008000u)
        #define SCS_SHCSR_SVCALLPENDED_MASK                  (0x00008000u)
        #define SCS_SHCSR_SVCALLPENDED_BIT                   (15)
        #define SCS_SHCSR_SVCALLPENDED_BITS                  (1)
        /* BUSFAULTPENDED field */
        #define SCS_SHCSR_BUSFAULTPENDED                     (0x00004000u)
        #define SCS_SHCSR_BUSFAULTPENDED_MASK                (0x00004000u)
        #define SCS_SHCSR_BUSFAULTPENDED_BIT                 (14)
        #define SCS_SHCSR_BUSFAULTPENDED_BITS                (1)
        /* MEMFAULTPENDED field */
        #define SCS_SHCSR_MEMFAULTPENDED                     (0x00002000u)
        #define SCS_SHCSR_MEMFAULTPENDED_MASK                (0x00002000u)
        #define SCS_SHCSR_MEMFAULTPENDED_BIT                 (13)
        #define SCS_SHCSR_MEMFAULTPENDED_BITS                (1)
        /* USGFAULTPENDED field */
        #define SCS_SHCSR_USGFAULTPENDED                     (0x00001000u)
        #define SCS_SHCSR_USGFAULTPENDED_MASK                (0x00001000u)
        #define SCS_SHCSR_USGFAULTPENDED_BIT                 (12)
        #define SCS_SHCSR_USGFAULTPENDED_BITS                (1)
        /* SYSTICKACT field */
        #define SCS_SHCSR_SYSTICKACT                         (0x00000800u)
        #define SCS_SHCSR_SYSTICKACT_MASK                    (0x00000800u)
        #define SCS_SHCSR_SYSTICKACT_BIT                     (11)
        #define SCS_SHCSR_SYSTICKACT_BITS                    (1)
        /* PENDSVACT field */
        #define SCS_SHCSR_PENDSVACT                          (0x00000400u)
        #define SCS_SHCSR_PENDSVACT_MASK                     (0x00000400u)
        #define SCS_SHCSR_PENDSVACT_BIT                      (10)
        #define SCS_SHCSR_PENDSVACT_BITS                     (1)
        /* MONITORACT field */
        #define SCS_SHCSR_MONITORACT                         (0x00000100u)
        #define SCS_SHCSR_MONITORACT_MASK                    (0x00000100u)
        #define SCS_SHCSR_MONITORACT_BIT                     (8)
        #define SCS_SHCSR_MONITORACT_BITS                    (1)
        /* SVCALLACT field */
        #define SCS_SHCSR_SVCALLACT                          (0x00000080u)
        #define SCS_SHCSR_SVCALLACT_MASK                     (0x00000080u)
        #define SCS_SHCSR_SVCALLACT_BIT                      (7)
        #define SCS_SHCSR_SVCALLACT_BITS                     (1)
        /* USGFAULTACT field */
        #define SCS_SHCSR_USGFAULTACT                        (0x00000008u)
        #define SCS_SHCSR_USGFAULTACT_MASK                   (0x00000008u)
        #define SCS_SHCSR_USGFAULTACT_BIT                    (3)
        #define SCS_SHCSR_USGFAULTACT_BITS                   (1)
        /* BUSFAULTACT field */
        #define SCS_SHCSR_BUSFAULTACT                        (0x00000002u)
        #define SCS_SHCSR_BUSFAULTACT_MASK                   (0x00000002u)
        #define SCS_SHCSR_BUSFAULTACT_BIT                    (1)
        #define SCS_SHCSR_BUSFAULTACT_BITS                   (1)
        /* MEMFAULTACT field */
        #define SCS_SHCSR_MEMFAULTACT                        (0x00000001u)
        #define SCS_SHCSR_MEMFAULTACT_MASK                   (0x00000001u)
        #define SCS_SHCSR_MEMFAULTACT_BIT                    (0)
        #define SCS_SHCSR_MEMFAULTACT_BITS                   (1)

#define SCS_CFSR                                             *((volatile uint32_t *)0xE000ED28u)
#define SCS_CFSR_REG                                         *((volatile uint32_t *)0xE000ED28u)
#define SCS_CFSR_ADDR                                        (0xE000ED28u)
#define SCS_CFSR_RESET                                       (0x00000000u)
        /* DIVBYZERO field */
        #define SCS_CFSR_DIVBYZERO                           (0x02000000u)
        #define SCS_CFSR_DIVBYZERO_MASK                      (0x02000000u)
        #define SCS_CFSR_DIVBYZERO_BIT                       (25)
        #define SCS_CFSR_DIVBYZERO_BITS                      (1)
        /* UNALIGNED field */
        #define SCS_CFSR_UNALIGNED                           (0x01000000u)
        #define SCS_CFSR_UNALIGNED_MASK                      (0x01000000u)
        #define SCS_CFSR_UNALIGNED_BIT                       (24)
        #define SCS_CFSR_UNALIGNED_BITS                      (1)
        /* NOCP field */
        #define SCS_CFSR_NOCP                                (0x00080000u)
        #define SCS_CFSR_NOCP_MASK                           (0x00080000u)
        #define SCS_CFSR_NOCP_BIT                            (19)
        #define SCS_CFSR_NOCP_BITS                           (1)
        /* INVPC field */
        #define SCS_CFSR_INVPC                               (0x00040000u)
        #define SCS_CFSR_INVPC_MASK                          (0x00040000u)
        #define SCS_CFSR_INVPC_BIT                           (18)
        #define SCS_CFSR_INVPC_BITS                          (1)
        /* INVSTATE field */
        #define SCS_CFSR_INVSTATE                            (0x00020000u)
        #define SCS_CFSR_INVSTATE_MASK                       (0x00020000u)
        #define SCS_CFSR_INVSTATE_BIT                        (17)
        #define SCS_CFSR_INVSTATE_BITS                       (1)
        /* UNDEFINSTR field */
        #define SCS_CFSR_UNDEFINSTR                          (0x00010000u)
        #define SCS_CFSR_UNDEFINSTR_MASK                     (0x00010000u)
        #define SCS_CFSR_UNDEFINSTR_BIT                      (16)
        #define SCS_CFSR_UNDEFINSTR_BITS                     (1)
        /* BFARVALID field */
        #define SCS_CFSR_BFARVALID                           (0x00008000u)
        #define SCS_CFSR_BFARVALID_MASK                      (0x00008000u)
        #define SCS_CFSR_BFARVALID_BIT                       (15)
        #define SCS_CFSR_BFARVALID_BITS                      (1)
        /* STKERR field */
        #define SCS_CFSR_STKERR                              (0x00001000u)
        #define SCS_CFSR_STKERR_MASK                         (0x00001000u)
        #define SCS_CFSR_STKERR_BIT                          (12)
        #define SCS_CFSR_STKERR_BITS                         (1)
        /* UNSTKERR field */
        #define SCS_CFSR_UNSTKERR                            (0x00000800u)
        #define SCS_CFSR_UNSTKERR_MASK                       (0x00000800u)
        #define SCS_CFSR_UNSTKERR_BIT                        (11)
        #define SCS_CFSR_UNSTKERR_BITS                       (1)
        /* IMPRECISERR field */
        #define SCS_CFSR_IMPRECISERR                         (0x00000400u)
        #define SCS_CFSR_IMPRECISERR_MASK                    (0x00000400u)
        #define SCS_CFSR_IMPRECISERR_BIT                     (10)
        #define SCS_CFSR_IMPRECISERR_BITS                    (1)
        /* PRECISERR field */
        #define SCS_CFSR_PRECISERR                           (0x00000200u)
        #define SCS_CFSR_PRECISERR_MASK                      (0x00000200u)
        #define SCS_CFSR_PRECISERR_BIT                       (9)
        #define SCS_CFSR_PRECISERR_BITS                      (1)
        /* IBUSERR field */
        #define SCS_CFSR_IBUSERR                             (0x00000100u)
        #define SCS_CFSR_IBUSERR_MASK                        (0x00000100u)
        #define SCS_CFSR_IBUSERR_BIT                         (8)
        #define SCS_CFSR_IBUSERR_BITS                        (1)
        /* MMARVALID field */
        #define SCS_CFSR_MMARVALID                           (0x00000080u)
        #define SCS_CFSR_MMARVALID_MASK                      (0x00000080u)
        #define SCS_CFSR_MMARVALID_BIT                       (7)
        #define SCS_CFSR_MMARVALID_BITS                      (1)
        /* MSTKERR field */
        #define SCS_CFSR_MSTKERR                             (0x00000010u)
        #define SCS_CFSR_MSTKERR_MASK                        (0x00000010u)
        #define SCS_CFSR_MSTKERR_BIT                         (4)
        #define SCS_CFSR_MSTKERR_BITS                        (1)
        /* MUNSTKERR field */
        #define SCS_CFSR_MUNSTKERR                           (0x00000008u)
        #define SCS_CFSR_MUNSTKERR_MASK                      (0x00000008u)
        #define SCS_CFSR_MUNSTKERR_BIT                       (3)
        #define SCS_CFSR_MUNSTKERR_BITS                      (1)
        /* DACCVIOL field */
        #define SCS_CFSR_DACCVIOL                            (0x00000002u)
        #define SCS_CFSR_DACCVIOL_MASK                       (0x00000002u)
        #define SCS_CFSR_DACCVIOL_BIT                        (1)
        #define SCS_CFSR_DACCVIOL_BITS                       (1)
        /* IACCVIOL field */
        #define SCS_CFSR_IACCVIOL                            (0x00000001u)
        #define SCS_CFSR_IACCVIOL_MASK                       (0x00000001u)
        #define SCS_CFSR_IACCVIOL_BIT                        (0)
        #define SCS_CFSR_IACCVIOL_BITS                       (1)

#define SCS_HFSR                                             *((volatile uint32_t *)0xE000ED2Cu)
#define SCS_HFSR_REG                                         *((volatile uint32_t *)0xE000ED2Cu)
#define SCS_HFSR_ADDR                                        (0xE000ED2Cu)
#define SCS_HFSR_RESET                                       (0x00000000u)
        /* DEBUGEVT field */
        #define SCS_HFSR_DEBUGEVT                            (0x80000000u)
        #define SCS_HFSR_DEBUGEVT_MASK                       (0x80000000u)
        #define SCS_HFSR_DEBUGEVT_BIT                        (31)
        #define SCS_HFSR_DEBUGEVT_BITS                       (1)
        /* FORCED field */
        #define SCS_HFSR_FORCED                              (0x40000000u)
        #define SCS_HFSR_FORCED_MASK                         (0x40000000u)
        #define SCS_HFSR_FORCED_BIT                          (30)
        #define SCS_HFSR_FORCED_BITS                         (1)
        /* VECTTBL field */
        #define SCS_HFSR_VECTTBL                             (0x00000002u)
        #define SCS_HFSR_VECTTBL_MASK                        (0x00000002u)
        #define SCS_HFSR_VECTTBL_BIT                         (1)
        #define SCS_HFSR_VECTTBL_BITS                        (1)

#define SCS_DFSR                                             *((volatile uint32_t *)0xE000ED30u)
#define SCS_DFSR_REG                                         *((volatile uint32_t *)0xE000ED30u)
#define SCS_DFSR_ADDR                                        (0xE000ED30u)
#define SCS_DFSR_RESET                                       (0x00000000u)
        /* EXTERNAL field */
        #define SCS_DFSR_EXTERNAL                            (0x00000010u)
        #define SCS_DFSR_EXTERNAL_MASK                       (0x00000010u)
        #define SCS_DFSR_EXTERNAL_BIT                        (4)
        #define SCS_DFSR_EXTERNAL_BITS                       (1)
        /* VCATCH field */
        #define SCS_DFSR_VCATCH                              (0x00000008u)
        #define SCS_DFSR_VCATCH_MASK                         (0x00000008u)
        #define SCS_DFSR_VCATCH_BIT                          (3)
        #define SCS_DFSR_VCATCH_BITS                         (1)
        /* DWTTRAP field */
        #define SCS_DFSR_DWTTRAP                             (0x00000004u)
        #define SCS_DFSR_DWTTRAP_MASK                        (0x00000004u)
        #define SCS_DFSR_DWTTRAP_BIT                         (2)
        #define SCS_DFSR_DWTTRAP_BITS                        (1)
        /* BKPT field */
        #define SCS_DFSR_BKPT                                (0x00000002u)
        #define SCS_DFSR_BKPT_MASK                           (0x00000002u)
        #define SCS_DFSR_BKPT_BIT                            (1)
        #define SCS_DFSR_BKPT_BITS                           (1)
        /* HALTED field */
        #define SCS_DFSR_HALTED                              (0x00000001u)
        #define SCS_DFSR_HALTED_MASK                         (0x00000001u)
        #define SCS_DFSR_HALTED_BIT                          (0)
        #define SCS_DFSR_HALTED_BITS                         (1)

#define SCS_MMAR                                             *((volatile uint32_t *)0xE000ED34u)
#define SCS_MMAR_REG                                         *((volatile uint32_t *)0xE000ED34u)
#define SCS_MMAR_ADDR                                        (0xE000ED34u)
#define SCS_MMAR_RESET                                       (0x00000000u)
        /* ADDRESS field */
        #define SCS_MMAR_ADDRESS                             (0xFFFFFFFFu)
        #define SCS_MMAR_ADDRESS_MASK                        (0xFFFFFFFFu)
        #define SCS_MMAR_ADDRESS_BIT                         (0)
        #define SCS_MMAR_ADDRESS_BITS                        (32)

#define SCS_BFAR                                             *((volatile uint32_t *)0xE000ED38u)
#define SCS_BFAR_REG                                         *((volatile uint32_t *)0xE000ED38u)
#define SCS_BFAR_ADDR                                        (0xE000ED38u)
#define SCS_BFAR_RESET                                       (0x00000000u)
        /* ADDRESS field */
        #define SCS_BFAR_ADDRESS                             (0xFFFFFFFFu)
        #define SCS_BFAR_ADDRESS_MASK                        (0xFFFFFFFFu)
        #define SCS_BFAR_ADDRESS_BIT                         (0)
        #define SCS_BFAR_ADDRESS_BITS                        (32)

#define SCS_AFSR                                             *((volatile uint32_t *)0xE000ED3Cu)
#define SCS_AFSR_REG                                         *((volatile uint32_t *)0xE000ED3Cu)
#define SCS_AFSR_ADDR                                        (0xE000ED3Cu)
#define SCS_AFSR_RESET                                       (0x00000000u)
        /* WRONGSIZE field */
        #define SCS_AFSR_WRONGSIZE                           (0x00000008u)
        #define SCS_AFSR_WRONGSIZE_MASK                      (0x00000008u)
        #define SCS_AFSR_WRONGSIZE_BIT                       (3)
        #define SCS_AFSR_WRONGSIZE_BITS                      (1)
        /* PROTECTED field */
        #define SCS_AFSR_PROTECTED                           (0x00000004u)
        #define SCS_AFSR_PROTECTED_MASK                      (0x00000004u)
        #define SCS_AFSR_PROTECTED_BIT                       (2)
        #define SCS_AFSR_PROTECTED_BITS                      (1)
        /* RESERVED field */
        #define SCS_AFSR_RESERVED                            (0x00000002u)
        #define SCS_AFSR_RESERVED_MASK                       (0x00000002u)
        #define SCS_AFSR_RESERVED_BIT                        (1)
        #define SCS_AFSR_RESERVED_BITS                       (1)
        /* MISSED field */
        #define SCS_AFSR_MISSED                              (0x00000001u)
        #define SCS_AFSR_MISSED_MASK                         (0x00000001u)
        #define SCS_AFSR_MISSED_BIT                          (0)
        #define SCS_AFSR_MISSED_BITS                         (1)

#define SCS_PFR0                                             *((volatile uint32_t *)0xE000ED40u)
#define SCS_PFR0_REG                                         *((volatile uint32_t *)0xE000ED40u)
#define SCS_PFR0_ADDR                                        (0xE000ED40u)
#define SCS_PFR0_RESET                                       (0x00000030u)
        /* FEATURE field */
        #define SCS_PFR0_FEATURE                             (0xFFFFFFFFu)
        #define SCS_PFR0_FEATURE_MASK                        (0xFFFFFFFFu)
        #define SCS_PFR0_FEATURE_BIT                         (0)
        #define SCS_PFR0_FEATURE_BITS                        (32)

#define SCS_PFR1                                             *((volatile uint32_t *)0xE000ED44u)
#define SCS_PFR1_REG                                         *((volatile uint32_t *)0xE000ED44u)
#define SCS_PFR1_ADDR                                        (0xE000ED44u)
#define SCS_PFR1_RESET                                       (0x00000200u)
        /* FEATURE field */
        #define SCS_PFR1_FEATURE                             (0xFFFFFFFFu)
        #define SCS_PFR1_FEATURE_MASK                        (0xFFFFFFFFu)
        #define SCS_PFR1_FEATURE_BIT                         (0)
        #define SCS_PFR1_FEATURE_BITS                        (32)

#define SCS_DFR0                                             *((volatile uint32_t *)0xE000ED48u)
#define SCS_DFR0_REG                                         *((volatile uint32_t *)0xE000ED48u)
#define SCS_DFR0_ADDR                                        (0xE000ED48u)
#define SCS_DFR0_RESET                                       (0x00100000u)
        /* FEATURE field */
        #define SCS_DFR0_FEATURE                             (0xFFFFFFFFu)
        #define SCS_DFR0_FEATURE_MASK                        (0xFFFFFFFFu)
        #define SCS_DFR0_FEATURE_BIT                         (0)
        #define SCS_DFR0_FEATURE_BITS                        (32)

#define SCS_AFR0                                             *((volatile uint32_t *)0xE000ED4Cu)
#define SCS_AFR0_REG                                         *((volatile uint32_t *)0xE000ED4Cu)
#define SCS_AFR0_ADDR                                        (0xE000ED4Cu)
#define SCS_AFR0_RESET                                       (0x00000000u)
        /* FEATURE field */
        #define SCS_AFR0_FEATURE                             (0xFFFFFFFFu)
        #define SCS_AFR0_FEATURE_MASK                        (0xFFFFFFFFu)
        #define SCS_AFR0_FEATURE_BIT                         (0)
        #define SCS_AFR0_FEATURE_BITS                        (32)

#define SCS_MMFR0                                            *((volatile uint32_t *)0xE000ED50u)
#define SCS_MMFR0_REG                                        *((volatile uint32_t *)0xE000ED50u)
#define SCS_MMFR0_ADDR                                       (0xE000ED50u)
#define SCS_MMFR0_RESET                                      (0x00000030u)
        /* FEATURE field */
        #define SCS_MMFR0_FEATURE                            (0xFFFFFFFFu)
        #define SCS_MMFR0_FEATURE_MASK                       (0xFFFFFFFFu)
        #define SCS_MMFR0_FEATURE_BIT                        (0)
        #define SCS_MMFR0_FEATURE_BITS                       (32)

#define SCS_MMFR1                                            *((volatile uint32_t *)0xE000ED54u)
#define SCS_MMFR1_REG                                        *((volatile uint32_t *)0xE000ED54u)
#define SCS_MMFR1_ADDR                                       (0xE000ED54u)
#define SCS_MMFR1_RESET                                      (0x00000000u)
        /* FEATURE field */
        #define SCS_MMFR1_FEATURE                            (0xFFFFFFFFu)
        #define SCS_MMFR1_FEATURE_MASK                       (0xFFFFFFFFu)
        #define SCS_MMFR1_FEATURE_BIT                        (0)
        #define SCS_MMFR1_FEATURE_BITS                       (32)

#define SCS_MMFR2                                            *((volatile uint32_t *)0xE000ED58u)
#define SCS_MMFR2_REG                                        *((volatile uint32_t *)0xE000ED58u)
#define SCS_MMFR2_ADDR                                       (0xE000ED58u)
#define SCS_MMFR2_RESET                                      (0x00000000u)
        /* FEATURE field */
        #define SCS_MMFR2_FEATURE                            (0xFFFFFFFFu)
        #define SCS_MMFR2_FEATURE_MASK                       (0xFFFFFFFFu)
        #define SCS_MMFR2_FEATURE_BIT                        (0)
        #define SCS_MMFR2_FEATURE_BITS                       (32)

#define SCS_MMFR3                                            *((volatile uint32_t *)0xE000ED5Cu)
#define SCS_MMFR3_REG                                        *((volatile uint32_t *)0xE000ED5Cu)
#define SCS_MMFR3_ADDR                                       (0xE000ED5Cu)
#define SCS_MMFR3_RESET                                      (0x00000000u)
        /* FEATURE field */
        #define SCS_MMFR3_FEATURE                            (0xFFFFFFFFu)
        #define SCS_MMFR3_FEATURE_MASK                       (0xFFFFFFFFu)
        #define SCS_MMFR3_FEATURE_BIT                        (0)
        #define SCS_MMFR3_FEATURE_BITS                       (32)

#define SCS_ISAFR0                                           *((volatile uint32_t *)0xE000ED60u)
#define SCS_ISAFR0_REG                                       *((volatile uint32_t *)0xE000ED60u)
#define SCS_ISAFR0_ADDR                                      (0xE000ED60u)
#define SCS_ISAFR0_RESET                                     (0x01141110u)
        /* FEATURE field */
        #define SCS_ISAFR0_FEATURE                           (0xFFFFFFFFu)
        #define SCS_ISAFR0_FEATURE_MASK                      (0xFFFFFFFFu)
        #define SCS_ISAFR0_FEATURE_BIT                       (0)
        #define SCS_ISAFR0_FEATURE_BITS                      (32)

#define SCS_ISAFR1                                           *((volatile uint32_t *)0xE000ED64u)
#define SCS_ISAFR1_REG                                       *((volatile uint32_t *)0xE000ED64u)
#define SCS_ISAFR1_ADDR                                      (0xE000ED64u)
#define SCS_ISAFR1_RESET                                     (0x02111000u)
        /* FEATURE field */
        #define SCS_ISAFR1_FEATURE                           (0xFFFFFFFFu)
        #define SCS_ISAFR1_FEATURE_MASK                      (0xFFFFFFFFu)
        #define SCS_ISAFR1_FEATURE_BIT                       (0)
        #define SCS_ISAFR1_FEATURE_BITS                      (32)

#define SCS_ISAFR2                                           *((volatile uint32_t *)0xE000ED68u)
#define SCS_ISAFR2_REG                                       *((volatile uint32_t *)0xE000ED68u)
#define SCS_ISAFR2_ADDR                                      (0xE000ED68u)
#define SCS_ISAFR2_RESET                                     (0x21112231u)
        /* FEATURE field */
        #define SCS_ISAFR2_FEATURE                           (0xFFFFFFFFu)
        #define SCS_ISAFR2_FEATURE_MASK                      (0xFFFFFFFFu)
        #define SCS_ISAFR2_FEATURE_BIT                       (0)
        #define SCS_ISAFR2_FEATURE_BITS                      (32)

#define SCS_ISAFR3                                           *((volatile uint32_t *)0xE000ED6Cu)
#define SCS_ISAFR3_REG                                       *((volatile uint32_t *)0xE000ED6Cu)
#define SCS_ISAFR3_ADDR                                      (0xE000ED6Cu)
#define SCS_ISAFR3_RESET                                     (0x11111110u)
        /* FEATURE field */
        #define SCS_ISAFR3_FEATURE                           (0xFFFFFFFFu)
        #define SCS_ISAFR3_FEATURE_MASK                      (0xFFFFFFFFu)
        #define SCS_ISAFR3_FEATURE_BIT                       (0)
        #define SCS_ISAFR3_FEATURE_BITS                      (32)

#define SCS_ISAFR4                                           *((volatile uint32_t *)0xE000ED70u)
#define SCS_ISAFR4_REG                                       *((volatile uint32_t *)0xE000ED70u)
#define SCS_ISAFR4_ADDR                                      (0xE000ED70u)
#define SCS_ISAFR4_RESET                                     (0x01310102u)
        /* FEATURE field */
        #define SCS_ISAFR4_FEATURE                           (0xFFFFFFFFu)
        #define SCS_ISAFR4_FEATURE_MASK                      (0xFFFFFFFFu)
        #define SCS_ISAFR4_FEATURE_BIT                       (0)
        #define SCS_ISAFR4_FEATURE_BITS                      (32)

#define MPU_TYPE                                             *((volatile uint32_t *)0xE000ED90u)
#define MPU_TYPE_REG                                         *((volatile uint32_t *)0xE000ED90u)
#define MPU_TYPE_ADDR                                        (0xE000ED90u)
#define MPU_TYPE_RESET                                       (0x00000800u)
        /* IREGION field */
        #define MPU_TYPE_IREGION                             (0x00FF0000u)
        #define MPU_TYPE_IREGION_MASK                        (0x00FF0000u)
        #define MPU_TYPE_IREGION_BIT                         (16)
        #define MPU_TYPE_IREGION_BITS                        (8)
        /* DREGION field */
        #define MPU_TYPE_DREGION                             (0x0000FF00u)
        #define MPU_TYPE_DREGION_MASK                        (0x0000FF00u)
        #define MPU_TYPE_DREGION_BIT                         (8)
        #define MPU_TYPE_DREGION_BITS                        (8)

#define MPU_CTRL                                             *((volatile uint32_t *)0xE000ED94u)
#define MPU_CTRL_REG                                         *((volatile uint32_t *)0xE000ED94u)
#define MPU_CTRL_ADDR                                        (0xE000ED94u)
#define MPU_CTRL_RESET                                       (0x00000000u)
        /* PRIVDEFENA field */
        #define MPU_CTRL_PRIVDEFENA                          (0x00000004u)
        #define MPU_CTRL_PRIVDEFENA_MASK                     (0x00000004u)
        #define MPU_CTRL_PRIVDEFENA_BIT                      (2)
        #define MPU_CTRL_PRIVDEFENA_BITS                     (1)
        /* HFNMIENA field */
        #define MPU_CTRL_HFNMIENA                            (0x00000002u)
        #define MPU_CTRL_HFNMIENA_MASK                       (0x00000002u)
        #define MPU_CTRL_HFNMIENA_BIT                        (1)
        #define MPU_CTRL_HFNMIENA_BITS                       (1)
        /* ENABLE field */
        #define MPU_CTRL_ENABLE                              (0x00000001u)
        #define MPU_CTRL_ENABLE_MASK                         (0x00000001u)
        #define MPU_CTRL_ENABLE_BIT                          (0)
        #define MPU_CTRL_ENABLE_BITS                         (1)

#define MPU_REGION                                           *((volatile uint32_t *)0xE000ED98u)
#define MPU_REGION_REG                                       *((volatile uint32_t *)0xE000ED98u)
#define MPU_REGION_ADDR                                      (0xE000ED98u)
#define MPU_REGION_RESET                                     (0x00000000u)
        /* REGION field */
        #define MPU_REGION_REGION                            (0x000000FFu)
        #define MPU_REGION_REGION_MASK                       (0x000000FFu)
        #define MPU_REGION_REGION_BIT                        (0)
        #define MPU_REGION_REGION_BITS                       (8)

#define MPU_BASE                                             *((volatile uint32_t *)0xE000ED9Cu)
#define MPU_BASE_REG                                         *((volatile uint32_t *)0xE000ED9Cu)
#define MPU_BASE_ADDR                                        (0xE000ED9Cu)
#define MPU_BASE_RESET                                       (0x00000000u)
        /* ADDRESS field */
        #define MPU_BASE_ADDRESS                             (0xFFFFFFE0u)
        #define MPU_BASE_ADDRESS_MASK                        (0xFFFFFFE0u)
        #define MPU_BASE_ADDRESS_BIT                         (5)
        #define MPU_BASE_ADDRESS_BITS                        (27)
        /* VALID field */
        #define MPU_BASE_VALID                               (0x00000010u)
        #define MPU_BASE_VALID_MASK                          (0x00000010u)
        #define MPU_BASE_VALID_BIT                           (4)
        #define MPU_BASE_VALID_BITS                          (1)
        /* REGION field */
        #define MPU_BASE_REGION                              (0x0000000Fu)
        #define MPU_BASE_REGION_MASK                         (0x0000000Fu)
        #define MPU_BASE_REGION_BIT                          (0)
        #define MPU_BASE_REGION_BITS                         (4)

#define MPU_ATTR                                             *((volatile uint32_t *)0xE000EDA0u)
#define MPU_ATTR_REG                                         *((volatile uint32_t *)0xE000EDA0u)
#define MPU_ATTR_ADDR                                        (0xE000EDA0u)
#define MPU_ATTR_RESET                                       (0x00000000u)
        /* XN field */
        #define MPU_ATTR_XN                                  (0x10000000u)
        #define MPU_ATTR_XN_MASK                             (0x10000000u)
        #define MPU_ATTR_XN_BIT                              (28)
        #define MPU_ATTR_XN_BITS                             (1)
        /* AP field */
        #define MPU_ATTR_AP                                  (0x07000000u)
        #define MPU_ATTR_AP_MASK                             (0x07000000u)
        #define MPU_ATTR_AP_BIT                              (24)
        #define MPU_ATTR_AP_BITS                             (3)
        /* TEX field */
        #define MPU_ATTR_TEX                                 (0x00380000u)
        #define MPU_ATTR_TEX_MASK                            (0x00380000u)
        #define MPU_ATTR_TEX_BIT                             (19)
        #define MPU_ATTR_TEX_BITS                            (3)
        /* S field */
        #define MPU_ATTR_S                                   (0x00040000u)
        #define MPU_ATTR_S_MASK                              (0x00040000u)
        #define MPU_ATTR_S_BIT                               (18)
        #define MPU_ATTR_S_BITS                              (1)
        /* C field */
        #define MPU_ATTR_C                                   (0x00020000u)
        #define MPU_ATTR_C_MASK                              (0x00020000u)
        #define MPU_ATTR_C_BIT                               (17)
        #define MPU_ATTR_C_BITS                              (1)
        /* B field */
        #define MPU_ATTR_B                                   (0x00010000u)
        #define MPU_ATTR_B_MASK                              (0x00010000u)
        #define MPU_ATTR_B_BIT                               (16)
        #define MPU_ATTR_B_BITS                              (1)
        /* SRD field */
        #define MPU_ATTR_SRD                                 (0x0000FF00u)
        #define MPU_ATTR_SRD_MASK                            (0x0000FF00u)
        #define MPU_ATTR_SRD_BIT                             (8)
        #define MPU_ATTR_SRD_BITS                            (8)
        /* SIZE field */
        #define MPU_ATTR_SIZE                                (0x0000003Eu)
        #define MPU_ATTR_SIZE_MASK                           (0x0000003Eu)
        #define MPU_ATTR_SIZE_BIT                            (1)
        #define MPU_ATTR_SIZE_BITS                           (5)
        /* ENABLE field */
        #define MPU_ATTR_ENABLE                              (0x00000001u)
        #define MPU_ATTR_ENABLE_MASK                         (0x00000001u)
        #define MPU_ATTR_ENABLE_BIT                          (0)
        #define MPU_ATTR_ENABLE_BITS                         (1)

#define MPU_BASE1                                            *((volatile uint32_t *)0xE000EDA4u)
#define MPU_BASE1_REG                                        *((volatile uint32_t *)0xE000EDA4u)
#define MPU_BASE1_ADDR                                       (0xE000EDA4u)
#define MPU_BASE1_RESET                                      (0x00000000u)
        /* ADDRESS field */
        #define MPU_BASE1_ADDRESS                            (0xFFFFFFE0u)
        #define MPU_BASE1_ADDRESS_MASK                       (0xFFFFFFE0u)
        #define MPU_BASE1_ADDRESS_BIT                        (5)
        #define MPU_BASE1_ADDRESS_BITS                       (27)
        /* VALID field */
        #define MPU_BASE1_VALID                              (0x00000010u)
        #define MPU_BASE1_VALID_MASK                         (0x00000010u)
        #define MPU_BASE1_VALID_BIT                          (4)
        #define MPU_BASE1_VALID_BITS                         (1)
        /* REGION field */
        #define MPU_BASE1_REGION                             (0x0000000Fu)
        #define MPU_BASE1_REGION_MASK                        (0x0000000Fu)
        #define MPU_BASE1_REGION_BIT                         (0)
        #define MPU_BASE1_REGION_BITS                        (4)

#define MPU_ATTR1                                            *((volatile uint32_t *)0xE000EDA8u)
#define MPU_ATTR1_REG                                        *((volatile uint32_t *)0xE000EDA8u)
#define MPU_ATTR1_ADDR                                       (0xE000EDA8u)
#define MPU_ATTR1_RESET                                      (0x00000000u)
        /* XN field */
        #define MPU_ATTR1_XN                                 (0x10000000u)
        #define MPU_ATTR1_XN_MASK                            (0x10000000u)
        #define MPU_ATTR1_XN_BIT                             (28)
        #define MPU_ATTR1_XN_BITS                            (1)
        /* AP field */
        #define MPU_ATTR1_AP                                 (0x07000000u)
        #define MPU_ATTR1_AP_MASK                            (0x07000000u)
        #define MPU_ATTR1_AP_BIT                             (24)
        #define MPU_ATTR1_AP_BITS                            (3)
        /* TEX field */
        #define MPU_ATTR1_TEX                                (0x00380000u)
        #define MPU_ATTR1_TEX_MASK                           (0x00380000u)
        #define MPU_ATTR1_TEX_BIT                            (19)
        #define MPU_ATTR1_TEX_BITS                           (3)
        /* S field */
        #define MPU_ATTR1_S                                  (0x00040000u)
        #define MPU_ATTR1_S_MASK                             (0x00040000u)
        #define MPU_ATTR1_S_BIT                              (18)
        #define MPU_ATTR1_S_BITS                             (1)
        /* C field */
        #define MPU_ATTR1_C                                  (0x00020000u)
        #define MPU_ATTR1_C_MASK                             (0x00020000u)
        #define MPU_ATTR1_C_BIT                              (17)
        #define MPU_ATTR1_C_BITS                             (1)
        /* B field */
        #define MPU_ATTR1_B                                  (0x00010000u)
        #define MPU_ATTR1_B_MASK                             (0x00010000u)
        #define MPU_ATTR1_B_BIT                              (16)
        #define MPU_ATTR1_B_BITS                             (1)
        /* SRD field */
        #define MPU_ATTR1_SRD                                (0x0000FF00u)
        #define MPU_ATTR1_SRD_MASK                           (0x0000FF00u)
        #define MPU_ATTR1_SRD_BIT                            (8)
        #define MPU_ATTR1_SRD_BITS                           (8)
        /* SIZE field */
        #define MPU_ATTR1_SIZE                               (0x0000003Eu)
        #define MPU_ATTR1_SIZE_MASK                          (0x0000003Eu)
        #define MPU_ATTR1_SIZE_BIT                           (1)
        #define MPU_ATTR1_SIZE_BITS                          (5)
        /* ENABLE field */
        #define MPU_ATTR1_ENABLE                             (0x00000001u)
        #define MPU_ATTR1_ENABLE_MASK                        (0x00000001u)
        #define MPU_ATTR1_ENABLE_BIT                         (0)
        #define MPU_ATTR1_ENABLE_BITS                        (1)

#define MPU_BASE2                                            *((volatile uint32_t *)0xE000EDACu)
#define MPU_BASE2_REG                                        *((volatile uint32_t *)0xE000EDACu)
#define MPU_BASE2_ADDR                                       (0xE000EDACu)
#define MPU_BASE2_RESET                                      (0x00000000u)
        /* ADDRESS field */
        #define MPU_BASE2_ADDRESS                            (0xFFFFFFE0u)
        #define MPU_BASE2_ADDRESS_MASK                       (0xFFFFFFE0u)
        #define MPU_BASE2_ADDRESS_BIT                        (5)
        #define MPU_BASE2_ADDRESS_BITS                       (27)
        /* VALID field */
        #define MPU_BASE2_VALID                              (0x00000010u)
        #define MPU_BASE2_VALID_MASK                         (0x00000010u)
        #define MPU_BASE2_VALID_BIT                          (4)
        #define MPU_BASE2_VALID_BITS                         (1)
        /* REGION field */
        #define MPU_BASE2_REGION                             (0x0000000Fu)
        #define MPU_BASE2_REGION_MASK                        (0x0000000Fu)
        #define MPU_BASE2_REGION_BIT                         (0)
        #define MPU_BASE2_REGION_BITS                        (4)

#define MPU_ATTR2                                            *((volatile uint32_t *)0xE000EDB0u)
#define MPU_ATTR2_REG                                        *((volatile uint32_t *)0xE000EDB0u)
#define MPU_ATTR2_ADDR                                       (0xE000EDB0u)
#define MPU_ATTR2_RESET                                      (0x00000000u)
        /* XN field */
        #define MPU_ATTR2_XN                                 (0x10000000u)
        #define MPU_ATTR2_XN_MASK                            (0x10000000u)
        #define MPU_ATTR2_XN_BIT                             (28)
        #define MPU_ATTR2_XN_BITS                            (1)
        /* AP field */
        #define MPU_ATTR2_AP                                 (0x1F000000u)
        #define MPU_ATTR2_AP_MASK                            (0x1F000000u)
        #define MPU_ATTR2_AP_BIT                             (24)
        #define MPU_ATTR2_AP_BITS                            (5)
        /* TEX field */
        #define MPU_ATTR2_TEX                                (0x00380000u)
        #define MPU_ATTR2_TEX_MASK                           (0x00380000u)
        #define MPU_ATTR2_TEX_BIT                            (19)
        #define MPU_ATTR2_TEX_BITS                           (3)
        /* S field */
        #define MPU_ATTR2_S                                  (0x00040000u)
        #define MPU_ATTR2_S_MASK                             (0x00040000u)
        #define MPU_ATTR2_S_BIT                              (18)
        #define MPU_ATTR2_S_BITS                             (1)
        /* C field */
        #define MPU_ATTR2_C                                  (0x00020000u)
        #define MPU_ATTR2_C_MASK                             (0x00020000u)
        #define MPU_ATTR2_C_BIT                              (17)
        #define MPU_ATTR2_C_BITS                             (1)
        /* B field */
        #define MPU_ATTR2_B                                  (0x00010000u)
        #define MPU_ATTR2_B_MASK                             (0x00010000u)
        #define MPU_ATTR2_B_BIT                              (16)
        #define MPU_ATTR2_B_BITS                             (1)
        /* SRD field */
        #define MPU_ATTR2_SRD                                (0x0000FF00u)
        #define MPU_ATTR2_SRD_MASK                           (0x0000FF00u)
        #define MPU_ATTR2_SRD_BIT                            (8)
        #define MPU_ATTR2_SRD_BITS                           (8)
        /* SIZE field */
        #define MPU_ATTR2_SIZE                               (0x0000003Eu)
        #define MPU_ATTR2_SIZE_MASK                          (0x0000003Eu)
        #define MPU_ATTR2_SIZE_BIT                           (1)
        #define MPU_ATTR2_SIZE_BITS                          (5)
        /* ENABLE field */
        #define MPU_ATTR2_ENABLE                             (0x00000003u)
        #define MPU_ATTR2_ENABLE_MASK                        (0x00000003u)
        #define MPU_ATTR2_ENABLE_BIT                         (0)
        #define MPU_ATTR2_ENABLE_BITS                        (2)

#define MPU_BASE3                                            *((volatile uint32_t *)0xE000EDB4u)
#define MPU_BASE3_REG                                        *((volatile uint32_t *)0xE000EDB4u)
#define MPU_BASE3_ADDR                                       (0xE000EDB4u)
#define MPU_BASE3_RESET                                      (0x00000000u)
        /* ADDRESS field */
        #define MPU_BASE3_ADDRESS                            (0xFFFFFFE0u)
        #define MPU_BASE3_ADDRESS_MASK                       (0xFFFFFFE0u)
        #define MPU_BASE3_ADDRESS_BIT                        (5)
        #define MPU_BASE3_ADDRESS_BITS                       (27)
        /* VALID field */
        #define MPU_BASE3_VALID                              (0x00000010u)
        #define MPU_BASE3_VALID_MASK                         (0x00000010u)
        #define MPU_BASE3_VALID_BIT                          (4)
        #define MPU_BASE3_VALID_BITS                         (1)
        /* REGION field */
        #define MPU_BASE3_REGION                             (0x0000000Fu)
        #define MPU_BASE3_REGION_MASK                        (0x0000000Fu)
        #define MPU_BASE3_REGION_BIT                         (0)
        #define MPU_BASE3_REGION_BITS                        (4)

#define MPU_ATTR3                                            *((volatile uint32_t *)0xE000EDBCu)
#define MPU_ATTR3_REG                                        *((volatile uint32_t *)0xE000EDBCu)
#define MPU_ATTR3_ADDR                                       (0xE000EDBCu)
#define MPU_ATTR3_RESET                                      (0x00000000u)
        /* XN field */
        #define MPU_ATTR3_XN                                 (0x10000000u)
        #define MPU_ATTR3_XN_MASK                            (0x10000000u)
        #define MPU_ATTR3_XN_BIT                             (28)
        #define MPU_ATTR3_XN_BITS                            (1)
        /* AP field */
        #define MPU_ATTR3_AP                                 (0x1F000000u)
        #define MPU_ATTR3_AP_MASK                            (0x1F000000u)
        #define MPU_ATTR3_AP_BIT                             (24)
        #define MPU_ATTR3_AP_BITS                            (5)
        /* TEX field */
        #define MPU_ATTR3_TEX                                (0x00380000u)
        #define MPU_ATTR3_TEX_MASK                           (0x00380000u)
        #define MPU_ATTR3_TEX_BIT                            (19)
        #define MPU_ATTR3_TEX_BITS                           (3)
        /* S field */
        #define MPU_ATTR3_S                                  (0x00040000u)
        #define MPU_ATTR3_S_MASK                             (0x00040000u)
        #define MPU_ATTR3_S_BIT                              (18)
        #define MPU_ATTR3_S_BITS                             (1)
        /* C field */
        #define MPU_ATTR3_C                                  (0x00020000u)
        #define MPU_ATTR3_C_MASK                             (0x00020000u)
        #define MPU_ATTR3_C_BIT                              (17)
        #define MPU_ATTR3_C_BITS                             (1)
        /* B field */
        #define MPU_ATTR3_B                                  (0x00010000u)
        #define MPU_ATTR3_B_MASK                             (0x00010000u)
        #define MPU_ATTR3_B_BIT                              (16)
        #define MPU_ATTR3_B_BITS                             (1)
        /* SRD field */
        #define MPU_ATTR3_SRD                                (0x0000FF00u)
        #define MPU_ATTR3_SRD_MASK                           (0x0000FF00u)
        #define MPU_ATTR3_SRD_BIT                            (8)
        #define MPU_ATTR3_SRD_BITS                           (8)
        /* SIZE field */
        #define MPU_ATTR3_SIZE                               (0x0000003Eu)
        #define MPU_ATTR3_SIZE_MASK                          (0x0000003Eu)
        #define MPU_ATTR3_SIZE_BIT                           (1)
        #define MPU_ATTR3_SIZE_BITS                          (5)
        /* ENABLE field */
        #define MPU_ATTR3_ENABLE                             (0x00000003u)
        #define MPU_ATTR3_ENABLE_MASK                        (0x00000003u)
        #define MPU_ATTR3_ENABLE_BIT                         (0)
        #define MPU_ATTR3_ENABLE_BITS                        (2)

#define DEBUG_HCSR                                           *((volatile uint32_t *)0xE000EDF0u)
#define DEBUG_HCSR_REG                                       *((volatile uint32_t *)0xE000EDF0u)
#define DEBUG_HCSR_ADDR                                      (0xE000EDF0u)
#define DEBUG_HCSR_RESET                                     (0x00000000u)
        /* S_RESET_ST field */
        #define DEBUG_HCSR_S_RESET_ST                        (0x02000000u)
        #define DEBUG_HCSR_S_RESET_ST_MASK                   (0x02000000u)
        #define DEBUG_HCSR_S_RESET_ST_BIT                    (25)
        #define DEBUG_HCSR_S_RESET_ST_BITS                   (1)
        /* S_RETIRE_ST field */
        #define DEBUG_HCSR_S_RETIRE_ST                       (0x01000000u)
        #define DEBUG_HCSR_S_RETIRE_ST_MASK                  (0x01000000u)
        #define DEBUG_HCSR_S_RETIRE_ST_BIT                   (24)
        #define DEBUG_HCSR_S_RETIRE_ST_BITS                  (1)
        /* S_LOCKUP field */
        #define DEBUG_HCSR_S_LOCKUP                          (0x00080000u)
        #define DEBUG_HCSR_S_LOCKUP_MASK                     (0x00080000u)
        #define DEBUG_HCSR_S_LOCKUP_BIT                      (19)
        #define DEBUG_HCSR_S_LOCKUP_BITS                     (1)
        /* S_SLEEP field */
        #define DEBUG_HCSR_S_SLEEP                           (0x00040000u)
        #define DEBUG_HCSR_S_SLEEP_MASK                      (0x00040000u)
        #define DEBUG_HCSR_S_SLEEP_BIT                       (18)
        #define DEBUG_HCSR_S_SLEEP_BITS                      (1)
        /* S_HALT field */
        #define DEBUG_HCSR_S_HALT                            (0x00020000u)
        #define DEBUG_HCSR_S_HALT_MASK                       (0x00020000u)
        #define DEBUG_HCSR_S_HALT_BIT                        (17)
        #define DEBUG_HCSR_S_HALT_BITS                       (1)
        /* S_REGRDY field */
        #define DEBUG_HCSR_S_REGRDY                          (0x00010000u)
        #define DEBUG_HCSR_S_REGRDY_MASK                     (0x00010000u)
        #define DEBUG_HCSR_S_REGRDY_BIT                      (16)
        #define DEBUG_HCSR_S_REGRDY_BITS                     (1)
        /* DBGKEY field */
        #define DEBUG_HCSR_DBGKEY                            (0xFFFF0000u)
        #define DEBUG_HCSR_DBGKEY_MASK                       (0xFFFF0000u)
        #define DEBUG_HCSR_DBGKEY_BIT                        (16)
        #define DEBUG_HCSR_DBGKEY_BITS                       (16)
        /* C_SNAPSTALL field */
        #define DEBUG_HCSR_C_SNAPSTALL                       (0x00000020u)
        #define DEBUG_HCSR_C_SNAPSTALL_MASK                  (0x00000020u)
        #define DEBUG_HCSR_C_SNAPSTALL_BIT                   (5)
        #define DEBUG_HCSR_C_SNAPSTALL_BITS                  (1)
        /* C_MASKINTS field */
        #define DEBUG_HCSR_C_MASKINTS                        (0x00000008u)
        #define DEBUG_HCSR_C_MASKINTS_MASK                   (0x00000008u)
        #define DEBUG_HCSR_C_MASKINTS_BIT                    (3)
        #define DEBUG_HCSR_C_MASKINTS_BITS                   (1)
        /* C_STEP field */
        #define DEBUG_HCSR_C_STEP                            (0x00000004u)
        #define DEBUG_HCSR_C_STEP_MASK                       (0x00000004u)
        #define DEBUG_HCSR_C_STEP_BIT                        (2)
        #define DEBUG_HCSR_C_STEP_BITS                       (1)
        /* C_HALT field */
        #define DEBUG_HCSR_C_HALT                            (0x00000002u)
        #define DEBUG_HCSR_C_HALT_MASK                       (0x00000002u)
        #define DEBUG_HCSR_C_HALT_BIT                        (1)
        #define DEBUG_HCSR_C_HALT_BITS                       (1)
        /* C_DEBUGEN field */
        #define DEBUG_HCSR_C_DEBUGEN                         (0x00000001u)
        #define DEBUG_HCSR_C_DEBUGEN_MASK                    (0x00000001u)
        #define DEBUG_HCSR_C_DEBUGEN_BIT                     (0)
        #define DEBUG_HCSR_C_DEBUGEN_BITS                    (1)

#define DEBUG_CRSR                                           *((volatile uint32_t *)0xE000EDF4u)
#define DEBUG_CRSR_REG                                       *((volatile uint32_t *)0xE000EDF4u)
#define DEBUG_CRSR_ADDR                                      (0xE000EDF4u)
#define DEBUG_CRSR_RESET                                     (0x00000000u)
        /* REGWnR field */
        #define DEBUG_CRSR_REGWnR                            (0x00010000u)
        #define DEBUG_CRSR_REGWnR_MASK                       (0x00010000u)
        #define DEBUG_CRSR_REGWnR_BIT                        (16)
        #define DEBUG_CRSR_REGWnR_BITS                       (1)
        /* REGSEL field */
        #define DEBUG_CRSR_REGSEL                            (0x0000001Fu)
        #define DEBUG_CRSR_REGSEL_MASK                       (0x0000001Fu)
        #define DEBUG_CRSR_REGSEL_BIT                        (0)
        #define DEBUG_CRSR_REGSEL_BITS                       (5)

#define DEBUG_CRDR                                           *((volatile uint32_t *)0xE000EDF8u)
#define DEBUG_CRDR_REG                                       *((volatile uint32_t *)0xE000EDF8u)
#define DEBUG_CRDR_ADDR                                      (0xE000EDF8u)
#define DEBUG_CRDR_RESET                                     (0x00000000u)
        /* DBGTMP field */
        #define DEBUG_CRDR_DBGTMP                            (0xFFFFFFFFu)
        #define DEBUG_CRDR_DBGTMP_MASK                       (0xFFFFFFFFu)
        #define DEBUG_CRDR_DBGTMP_BIT                        (0)
        #define DEBUG_CRDR_DBGTMP_BITS                       (32)

#define DEBUG_EMCR                                           *((volatile uint32_t *)0xE000EDFCu)
#define DEBUG_EMCR_REG                                       *((volatile uint32_t *)0xE000EDFCu)
#define DEBUG_EMCR_ADDR                                      (0xE000EDFCu)
#define DEBUG_EMCR_RESET                                     (0x00000000u)
        /* TRCENA field */
        #define DEBUG_EMCR_TRCENA                            (0x01000000u)
        #define DEBUG_EMCR_TRCENA_MASK                       (0x01000000u)
        #define DEBUG_EMCR_TRCENA_BIT                        (24)
        #define DEBUG_EMCR_TRCENA_BITS                       (1)
        /* MON_REQ field */
        #define DEBUG_EMCR_MON_REQ                           (0x00080000u)
        #define DEBUG_EMCR_MON_REQ_MASK                      (0x00080000u)
        #define DEBUG_EMCR_MON_REQ_BIT                       (19)
        #define DEBUG_EMCR_MON_REQ_BITS                      (1)
        /* MON_STEP field */
        #define DEBUG_EMCR_MON_STEP                          (0x00040000u)
        #define DEBUG_EMCR_MON_STEP_MASK                     (0x00040000u)
        #define DEBUG_EMCR_MON_STEP_BIT                      (18)
        #define DEBUG_EMCR_MON_STEP_BITS                     (1)
        /* MON_PEND field */
        #define DEBUG_EMCR_MON_PEND                          (0x00020000u)
        #define DEBUG_EMCR_MON_PEND_MASK                     (0x00020000u)
        #define DEBUG_EMCR_MON_PEND_BIT                      (17)
        #define DEBUG_EMCR_MON_PEND_BITS                     (1)
        /* MON_EN field */
        #define DEBUG_EMCR_MON_EN                            (0x00010000u)
        #define DEBUG_EMCR_MON_EN_MASK                       (0x00010000u)
        #define DEBUG_EMCR_MON_EN_BIT                        (16)
        #define DEBUG_EMCR_MON_EN_BITS                       (1)
        /* VC_HARDERR field */
        #define DEBUG_EMCR_VC_HARDERR                        (0x00000400u)
        #define DEBUG_EMCR_VC_HARDERR_MASK                   (0x00000400u)
        #define DEBUG_EMCR_VC_HARDERR_BIT                    (10)
        #define DEBUG_EMCR_VC_HARDERR_BITS                   (1)
        /* VC_INTERR field */
        #define DEBUG_EMCR_VC_INTERR                         (0x00000200u)
        #define DEBUG_EMCR_VC_INTERR_MASK                    (0x00000200u)
        #define DEBUG_EMCR_VC_INTERR_BIT                     (9)
        #define DEBUG_EMCR_VC_INTERR_BITS                    (1)
        /* VC_BUSERR field */
        #define DEBUG_EMCR_VC_BUSERR                         (0x00000100u)
        #define DEBUG_EMCR_VC_BUSERR_MASK                    (0x00000100u)
        #define DEBUG_EMCR_VC_BUSERR_BIT                     (8)
        #define DEBUG_EMCR_VC_BUSERR_BITS                    (1)
        /* VC_STATERR field */
        #define DEBUG_EMCR_VC_STATERR                        (0x00000080u)
        #define DEBUG_EMCR_VC_STATERR_MASK                   (0x00000080u)
        #define DEBUG_EMCR_VC_STATERR_BIT                    (7)
        #define DEBUG_EMCR_VC_STATERR_BITS                   (1)
        /* VC_CHKERR field */
        #define DEBUG_EMCR_VC_CHKERR                         (0x00000040u)
        #define DEBUG_EMCR_VC_CHKERR_MASK                    (0x00000040u)
        #define DEBUG_EMCR_VC_CHKERR_BIT                     (6)
        #define DEBUG_EMCR_VC_CHKERR_BITS                    (1)
        /* VC_NOCPERR field */
        #define DEBUG_EMCR_VC_NOCPERR                        (0x00000020u)
        #define DEBUG_EMCR_VC_NOCPERR_MASK                   (0x00000020u)
        #define DEBUG_EMCR_VC_NOCPERR_BIT                    (5)
        #define DEBUG_EMCR_VC_NOCPERR_BITS                   (1)
        /* VC_MMERR field */
        #define DEBUG_EMCR_VC_MMERR                          (0x00000010u)
        #define DEBUG_EMCR_VC_MMERR_MASK                     (0x00000010u)
        #define DEBUG_EMCR_VC_MMERR_BIT                      (4)
        #define DEBUG_EMCR_VC_MMERR_BITS                     (1)
        /* VC_CORERESET field */
        #define DEBUG_EMCR_VC_CORERESET                      (0x00000001u)
        #define DEBUG_EMCR_VC_CORERESET_MASK                 (0x00000001u)
        #define DEBUG_EMCR_VC_CORERESET_BIT                  (0)
        #define DEBUG_EMCR_VC_CORERESET_BITS                 (1)

#define NVIC_STIR                                            *((volatile uint32_t *)0xE000EF00u)
#define NVIC_STIR_REG                                        *((volatile uint32_t *)0xE000EF00u)
#define NVIC_STIR_ADDR                                       (0xE000EF00u)
#define NVIC_STIR_RESET                                      (0x00000000u)
        /* INTID field */
        #define NVIC_STIR_INTID                              (0x000003FFu)
        #define NVIC_STIR_INTID_MASK                         (0x000003FFu)
        #define NVIC_STIR_INTID_BIT                          (0)
        #define NVIC_STIR_INTID_BITS                         (10)

#define NVIC_PERIPHID4                                       *((volatile uint32_t *)0xE000EFD0u)
#define NVIC_PERIPHID4_REG                                   *((volatile uint32_t *)0xE000EFD0u)
#define NVIC_PERIPHID4_ADDR                                  (0xE000EFD0u)
#define NVIC_PERIPHID4_RESET                                 (0x00000004u)
        /* PERIPHID field */
        #define NVIC_PERIPHID4_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID4_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID4_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID4_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID5                                       *((volatile uint32_t *)0xE000EFD4u)
#define NVIC_PERIPHID5_REG                                   *((volatile uint32_t *)0xE000EFD4u)
#define NVIC_PERIPHID5_ADDR                                  (0xE000EFD4u)
#define NVIC_PERIPHID5_RESET                                 (0x00000000u)
        /* PERIPHID field */
        #define NVIC_PERIPHID5_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID5_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID5_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID5_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID6                                       *((volatile uint32_t *)0xE000EFD8u)
#define NVIC_PERIPHID6_REG                                   *((volatile uint32_t *)0xE000EFD8u)
#define NVIC_PERIPHID6_ADDR                                  (0xE000EFD8u)
#define NVIC_PERIPHID6_RESET                                 (0x00000000u)
        /* PERIPHID field */
        #define NVIC_PERIPHID6_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID6_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID6_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID6_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID7                                       *((volatile uint32_t *)0xE000EFDCu)
#define NVIC_PERIPHID7_REG                                   *((volatile uint32_t *)0xE000EFDCu)
#define NVIC_PERIPHID7_ADDR                                  (0xE000EFDCu)
#define NVIC_PERIPHID7_RESET                                 (0x00000000u)
        /* PERIPHID field */
        #define NVIC_PERIPHID7_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID7_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID7_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID7_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID0                                       *((volatile uint32_t *)0xE000EFE0u)
#define NVIC_PERIPHID0_REG                                   *((volatile uint32_t *)0xE000EFE0u)
#define NVIC_PERIPHID0_ADDR                                  (0xE000EFE0u)
#define NVIC_PERIPHID0_RESET                                 (0x00000000u)
        /* PERIPHID field */
        #define NVIC_PERIPHID0_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID0_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID0_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID0_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID1                                       *((volatile uint32_t *)0xE000EFE4u)
#define NVIC_PERIPHID1_REG                                   *((volatile uint32_t *)0xE000EFE4u)
#define NVIC_PERIPHID1_ADDR                                  (0xE000EFE4u)
#define NVIC_PERIPHID1_RESET                                 (0x000000B0u)
        /* PERIPHID field */
        #define NVIC_PERIPHID1_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID1_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID1_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID1_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID2                                       *((volatile uint32_t *)0xE000EFE8u)
#define NVIC_PERIPHID2_REG                                   *((volatile uint32_t *)0xE000EFE8u)
#define NVIC_PERIPHID2_ADDR                                  (0xE000EFE8u)
#define NVIC_PERIPHID2_RESET                                 (0x0000001Bu)
        /* PERIPHID field */
        #define NVIC_PERIPHID2_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID2_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID2_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID2_PERIPHID_BITS                 (32)

#define NVIC_PERIPHID3                                       *((volatile uint32_t *)0xE000EFECu)
#define NVIC_PERIPHID3_REG                                   *((volatile uint32_t *)0xE000EFECu)
#define NVIC_PERIPHID3_ADDR                                  (0xE000EFECu)
#define NVIC_PERIPHID3_RESET                                 (0x00000000u)
        /* PERIPHID field */
        #define NVIC_PERIPHID3_PERIPHID                      (0xFFFFFFFFu)
        #define NVIC_PERIPHID3_PERIPHID_MASK                 (0xFFFFFFFFu)
        #define NVIC_PERIPHID3_PERIPHID_BIT                  (0)
        #define NVIC_PERIPHID3_PERIPHID_BITS                 (32)

#define NVIC_PCELLID0                                        *((volatile uint32_t *)0xE000EFF0u)
#define NVIC_PCELLID0_REG                                    *((volatile uint32_t *)0xE000EFF0u)
#define NVIC_PCELLID0_ADDR                                   (0xE000EFF0u)
#define NVIC_PCELLID0_RESET                                  (0x0000000Du)
        /* PCELLID field */
        #define NVIC_PCELLID0_PCELLID                        (0xFFFFFFFFu)
        #define NVIC_PCELLID0_PCELLID_MASK                   (0xFFFFFFFFu)
        #define NVIC_PCELLID0_PCELLID_BIT                    (0)
        #define NVIC_PCELLID0_PCELLID_BITS                   (32)

#define NVIC_PCELLID1                                        *((volatile uint32_t *)0xE000EFF4u)
#define NVIC_PCELLID1_REG                                    *((volatile uint32_t *)0xE000EFF4u)
#define NVIC_PCELLID1_ADDR                                   (0xE000EFF4u)
#define NVIC_PCELLID1_RESET                                  (0x000000E0u)
        /* PCELLID field */
        #define NVIC_PCELLID1_PCELLID                        (0xFFFFFFFFu)
        #define NVIC_PCELLID1_PCELLID_MASK                   (0xFFFFFFFFu)
        #define NVIC_PCELLID1_PCELLID_BIT                    (0)
        #define NVIC_PCELLID1_PCELLID_BITS                   (32)

#define NVIC_PCELLID2                                        *((volatile uint32_t *)0xE000EFF8u)
#define NVIC_PCELLID2_REG                                    *((volatile uint32_t *)0xE000EFF8u)
#define NVIC_PCELLID2_ADDR                                   (0xE000EFF8u)
#define NVIC_PCELLID2_RESET                                  (0x00000005u)
        /* PCELLID field */
        #define NVIC_PCELLID2_PCELLID                        (0xFFFFFFFFu)
        #define NVIC_PCELLID2_PCELLID_MASK                   (0xFFFFFFFFu)
        #define NVIC_PCELLID2_PCELLID_BIT                    (0)
        #define NVIC_PCELLID2_PCELLID_BITS                   (32)

#define NVIC_PCELLID3                                        *((volatile uint32_t *)0xE000EFFCu)
#define NVIC_PCELLID3_REG                                    *((volatile uint32_t *)0xE000EFFCu)
#define NVIC_PCELLID3_ADDR                                   (0xE000EFFCu)
#define NVIC_PCELLID3_RESET                                  (0x000000B1u)
        /* PCELLID field */
        #define NVIC_PCELLID3_PCELLID                        (0xFFFFFFFFu)
        #define NVIC_PCELLID3_PCELLID_MASK                   (0xFFFFFFFFu)
        #define NVIC_PCELLID3_PCELLID_BIT                    (0)
        #define NVIC_PCELLID3_PCELLID_BITS                   (32)

/* TPIU block */
#define DATA_TPIU_BASE                                       (0xE0040000u)
#define DATA_TPIU_END                                        (0xE0040EF8u)
#define DATA_TPIU_SIZE                                       (DATA_TPIU_END - DATA_TPIU_BASE + 1)

#define TPIU_SPS                                             *((volatile uint32_t *)0xE0040000u)
#define TPIU_SPS_REG                                         *((volatile uint32_t *)0xE0040000u)
#define TPIU_SPS_ADDR                                        (0xE0040000u)
#define TPIU_SPS_RESET                                       (0x00000000u)
        /* SPS_04 field */
        #define TPIU_SPS_SPS_04                              (0x00000008u)
        #define TPIU_SPS_SPS_04_MASK                         (0x00000008u)
        #define TPIU_SPS_SPS_04_BIT                          (3)
        #define TPIU_SPS_SPS_04_BITS                         (1)
        /* SPS_03 field */
        #define TPIU_SPS_SPS_03                              (0x00000004u)
        #define TPIU_SPS_SPS_03_MASK                         (0x00000004u)
        #define TPIU_SPS_SPS_03_BIT                          (2)
        #define TPIU_SPS_SPS_03_BITS                         (1)
        /* SPS_02 field */
        #define TPIU_SPS_SPS_02                              (0x00000002u)
        #define TPIU_SPS_SPS_02_MASK                         (0x00000002u)
        #define TPIU_SPS_SPS_02_BIT                          (1)
        #define TPIU_SPS_SPS_02_BITS                         (1)
        /* SPS_01 field */
        #define TPIU_SPS_SPS_01                              (0x00000001u)
        #define TPIU_SPS_SPS_01_MASK                         (0x00000001u)
        #define TPIU_SPS_SPS_01_BIT                          (0)
        #define TPIU_SPS_SPS_01_BITS                         (1)

#define TPIU_CPS                                             *((volatile uint32_t *)0xE0040004u)
#define TPIU_CPS_REG                                         *((volatile uint32_t *)0xE0040004u)
#define TPIU_CPS_ADDR                                        (0xE0040004u)
#define TPIU_CPS_RESET                                       (0x00000001u)
        /* CPS_04 field */
        #define TPIU_CPS_CPS_04                              (0x00000008u)
        #define TPIU_CPS_CPS_04_MASK                         (0x00000008u)
        #define TPIU_CPS_CPS_04_BIT                          (3)
        #define TPIU_CPS_CPS_04_BITS                         (1)
        /* CPS_03 field */
        #define TPIU_CPS_CPS_03                              (0x00000004u)
        #define TPIU_CPS_CPS_03_MASK                         (0x00000004u)
        #define TPIU_CPS_CPS_03_BIT                          (2)
        #define TPIU_CPS_CPS_03_BITS                         (1)
        /* CPS_02 field */
        #define TPIU_CPS_CPS_02                              (0x00000002u)
        #define TPIU_CPS_CPS_02_MASK                         (0x00000002u)
        #define TPIU_CPS_CPS_02_BIT                          (1)
        #define TPIU_CPS_CPS_02_BITS                         (1)
        /* CPS_01 field */
        #define TPIU_CPS_CPS_01                              (0x00000001u)
        #define TPIU_CPS_CPS_01_MASK                         (0x00000001u)
        #define TPIU_CPS_CPS_01_BIT                          (0)
        #define TPIU_CPS_CPS_01_BITS                         (1)

#define TPIU_COSD                                            *((volatile uint32_t *)0xE0040010u)
#define TPIU_COSD_REG                                        *((volatile uint32_t *)0xE0040010u)
#define TPIU_COSD_ADDR                                       (0xE0040010u)
#define TPIU_COSD_RESET                                      (0x00000000u)
        /* PRESCALER field */
        #define TPIU_COSD_PRESCALER                          (0x00001FFFu)
        #define TPIU_COSD_PRESCALER_MASK                     (0x00001FFFu)
        #define TPIU_COSD_PRESCALER_BIT                      (0)
        #define TPIU_COSD_PRESCALER_BITS                     (13)

#define TPIU_SPP                                             *((volatile uint32_t *)0xE00400F0u)
#define TPIU_SPP_REG                                         *((volatile uint32_t *)0xE00400F0u)
#define TPIU_SPP_ADDR                                        (0xE00400F0u)
#define TPIU_SPP_RESET                                       (0x00000001u)
        /* PROTOCOL field */
        #define TPIU_SPP_PROTOCOL                            (0x00000003u)
        #define TPIU_SPP_PROTOCOL_MASK                       (0x00000003u)
        #define TPIU_SPP_PROTOCOL_BIT                        (0)
        #define TPIU_SPP_PROTOCOL_BITS                       (2)

#define TPIU_FFS                                             *((volatile uint32_t *)0xE0040300u)
#define TPIU_FFS_REG                                         *((volatile uint32_t *)0xE0040300u)
#define TPIU_FFS_ADDR                                        (0xE0040300u)
#define TPIU_FFS_RESET                                       (0x00000008u)
        /* FTNONSTOP field */
        #define TPIU_FFS_FTNONSTOP                           (0x00000008u)
        #define TPIU_FFS_FTNONSTOP_MASK                      (0x00000008u)
        #define TPIU_FFS_FTNONSTOP_BIT                       (3)
        #define TPIU_FFS_FTNONSTOP_BITS                      (1)
        /* TCPRESENT field */
        #define TPIU_FFS_TCPRESENT                           (0x00000004u)
        #define TPIU_FFS_TCPRESENT_MASK                      (0x00000004u)
        #define TPIU_FFS_TCPRESENT_BIT                       (2)
        #define TPIU_FFS_TCPRESENT_BITS                      (1)
        /* FTSTOPPED field */
        #define TPIU_FFS_FTSTOPPED                           (0x00000002u)
        #define TPIU_FFS_FTSTOPPED_MASK                      (0x00000002u)
        #define TPIU_FFS_FTSTOPPED_BIT                       (1)
        #define TPIU_FFS_FTSTOPPED_BITS                      (1)
        /* FLINPROG field */
        #define TPIU_FFS_FLINPROG                            (0x00000001u)
        #define TPIU_FFS_FLINPROG_MASK                       (0x00000001u)
        #define TPIU_FFS_FLINPROG_BIT                        (0)
        #define TPIU_FFS_FLINPROG_BITS                       (1)

#define TPIU_FFC                                             *((volatile uint32_t *)0xE0040304u)
#define TPIU_FFC_REG                                         *((volatile uint32_t *)0xE0040304u)
#define TPIU_FFC_ADDR                                        (0xE0040304u)
#define TPIU_FFC_RESET                                       (0x00000102u)
        /* TRIGIN field */
        #define TPIU_FFC_TRIGIN                              (0x00000100u)
        #define TPIU_FFC_TRIGIN_MASK                         (0x00000100u)
        #define TPIU_FFC_TRIGIN_BIT                          (8)
        #define TPIU_FFC_TRIGIN_BITS                         (1)
        /* ENFCONT field */
        #define TPIU_FFC_ENFCONT                             (0x00000002u)
        #define TPIU_FFC_ENFCONT_MASK                        (0x00000002u)
        #define TPIU_FFC_ENFCONT_BIT                         (1)
        #define TPIU_FFC_ENFCONT_BITS                        (1)

#define TPIU_FSC                                             *((volatile uint32_t *)0xE0040308u)
#define TPIU_FSC_REG                                         *((volatile uint32_t *)0xE0040308u)
#define TPIU_FSC_ADDR                                        (0xE0040308u)
#define TPIU_FSC_RESET                                       (0x00000000u)
        /* FSC field */
        #define TPIU_FSC_FSC                                 (0xFFFFFFFFu)
        #define TPIU_FSC_FSC_MASK                            (0xFFFFFFFFu)
        #define TPIU_FSC_FSC_BIT                             (0)
        #define TPIU_FSC_FSC_BITS                            (32)

#define TPIU_ITATBCTR2                                       *((volatile uint32_t *)0xE0040EF0u)
#define TPIU_ITATBCTR2_REG                                   *((volatile uint32_t *)0xE0040EF0u)
#define TPIU_ITATBCTR2_ADDR                                  (0xE0040EF0u)
#define TPIU_ITATBCTR2_RESET                                 (0x00000000u)
        /* ATREADY1 field */
        #define TPIU_ITATBCTR2_ATREADY1                      (0x00000001u)
        #define TPIU_ITATBCTR2_ATREADY1_MASK                 (0x00000001u)
        #define TPIU_ITATBCTR2_ATREADY1_BIT                  (0)
        #define TPIU_ITATBCTR2_ATREADY1_BITS                 (1)

#define TPIU_ITATBCTR0                                       *((volatile uint32_t *)0xE0040EF8u)
#define TPIU_ITATBCTR0_REG                                   *((volatile uint32_t *)0xE0040EF8u)
#define TPIU_ITATBCTR0_ADDR                                  (0xE0040EF8u)
#define TPIU_ITATBCTR0_RESET                                 (0x00000000u)
        /* ATREADY1 field */
        #define TPIU_ITATBCTR0_ATREADY1                      (0x00000001u)
        #define TPIU_ITATBCTR0_ATREADY1_MASK                 (0x00000001u)
        #define TPIU_ITATBCTR0_ATREADY1_BIT                  (0)
        #define TPIU_ITATBCTR0_ATREADY1_BITS                 (1)

/* ETM block */
#define DATA_ETM_BASE                                        (0xE0041000u)
#define DATA_ETM_END                                         (0xE0041FFFu)
#define DATA_ETM_SIZE                                        (DATA_ETM_END - DATA_ETM_BASE + 1)

/* ROM_TAB block */
#define DATA_ROM_TAB_BASE                                    (0xE00FF000u)
#define DATA_ROM_TAB_END                                     (0xE00FFFFFu)
#define DATA_ROM_TAB_SIZE                                    (DATA_ROM_TAB_END - DATA_ROM_TAB_BASE + 1)

#define ROM_SCS                                              *((volatile uint32_t *)0xE00FF000u)
#define ROM_SCS_REG                                          *((volatile uint32_t *)0xE00FF000u)
#define ROM_SCS_ADDR                                         (0xE00FF000u)
#define ROM_SCS_RESET                                        (0xFFF0F003u)
        /* ADDR_OFF field */
        #define ROM_SCS_ADDR_OFF                             (0xFFFFF000u)
        #define ROM_SCS_ADDR_OFF_MASK                        (0xFFFFF000u)
        #define ROM_SCS_ADDR_OFF_BIT                         (12)
        #define ROM_SCS_ADDR_OFF_BITS                        (20)
        /* FORMAT field */
        #define ROM_SCS_FORMAT                               (0x00000002u)
        #define ROM_SCS_FORMAT_MASK                          (0x00000002u)
        #define ROM_SCS_FORMAT_BIT                           (1)
        #define ROM_SCS_FORMAT_BITS                          (1)
        /* ENTRY_PRES field */
        #define ROM_SCS_ENTRY_PRES                           (0x00000001u)
        #define ROM_SCS_ENTRY_PRES_MASK                      (0x00000001u)
        #define ROM_SCS_ENTRY_PRES_BIT                       (0)
        #define ROM_SCS_ENTRY_PRES_BITS                      (1)

#define ROM_DWT                                              *((volatile uint32_t *)0xE00FF004u)
#define ROM_DWT_REG                                          *((volatile uint32_t *)0xE00FF004u)
#define ROM_DWT_ADDR                                         (0xE00FF004u)
#define ROM_DWT_RESET                                        (0xFFF02003u)
        /* ADDR_OFF field */
        #define ROM_DWT_ADDR_OFF                             (0xFFFFF000u)
        #define ROM_DWT_ADDR_OFF_MASK                        (0xFFFFF000u)
        #define ROM_DWT_ADDR_OFF_BIT                         (12)
        #define ROM_DWT_ADDR_OFF_BITS                        (20)
        /* FORMAT field */
        #define ROM_DWT_FORMAT                               (0x00000002u)
        #define ROM_DWT_FORMAT_MASK                          (0x00000002u)
        #define ROM_DWT_FORMAT_BIT                           (1)
        #define ROM_DWT_FORMAT_BITS                          (1)
        /* ENTRY_PRES field */
        #define ROM_DWT_ENTRY_PRES                           (0x00000001u)
        #define ROM_DWT_ENTRY_PRES_MASK                      (0x00000001u)
        #define ROM_DWT_ENTRY_PRES_BIT                       (0)
        #define ROM_DWT_ENTRY_PRES_BITS                      (1)

#define ROM_FPB                                              *((volatile uint32_t *)0xE00FF008u)
#define ROM_FPB_REG                                          *((volatile uint32_t *)0xE00FF008u)
#define ROM_FPB_ADDR                                         (0xE00FF008u)
#define ROM_FPB_RESET                                        (0xFFF03003u)
        /* ADDR_OFF field */
        #define ROM_FPB_ADDR_OFF                             (0xFFFFF000u)
        #define ROM_FPB_ADDR_OFF_MASK                        (0xFFFFF000u)
        #define ROM_FPB_ADDR_OFF_BIT                         (12)
        #define ROM_FPB_ADDR_OFF_BITS                        (20)
        /* FORMAT field */
        #define ROM_FPB_FORMAT                               (0x00000002u)
        #define ROM_FPB_FORMAT_MASK                          (0x00000002u)
        #define ROM_FPB_FORMAT_BIT                           (1)
        #define ROM_FPB_FORMAT_BITS                          (1)
        /* ENTRY_PRES field */
        #define ROM_FPB_ENTRY_PRES                           (0x00000001u)
        #define ROM_FPB_ENTRY_PRES_MASK                      (0x00000001u)
        #define ROM_FPB_ENTRY_PRES_BIT                       (0)
        #define ROM_FPB_ENTRY_PRES_BITS                      (1)

#define ROM_ITM                                              *((volatile uint32_t *)0xE00FF00Cu)
#define ROM_ITM_REG                                          *((volatile uint32_t *)0xE00FF00Cu)
#define ROM_ITM_ADDR                                         (0xE00FF00Cu)
#define ROM_ITM_RESET                                        (0xFFF01003u)
        /* ADDR_OFF field */
        #define ROM_ITM_ADDR_OFF                             (0xFFFFF000u)
        #define ROM_ITM_ADDR_OFF_MASK                        (0xFFFFF000u)
        #define ROM_ITM_ADDR_OFF_BIT                         (12)
        #define ROM_ITM_ADDR_OFF_BITS                        (20)
        /* FORMAT field */
        #define ROM_ITM_FORMAT                               (0x00000002u)
        #define ROM_ITM_FORMAT_MASK                          (0x00000002u)
        #define ROM_ITM_FORMAT_BIT                           (1)
        #define ROM_ITM_FORMAT_BITS                          (1)
        /* ENTRY_PRES field */
        #define ROM_ITM_ENTRY_PRES                           (0x00000001u)
        #define ROM_ITM_ENTRY_PRES_MASK                      (0x00000001u)
        #define ROM_ITM_ENTRY_PRES_BIT                       (0)
        #define ROM_ITM_ENTRY_PRES_BITS                      (1)

#define ROM_TPIU                                             *((volatile uint32_t *)0xE00FF010u)
#define ROM_TPIU_REG                                         *((volatile uint32_t *)0xE00FF010u)
#define ROM_TPIU_ADDR                                        (0xE00FF010u)
#define ROM_TPIU_RESET                                       (0xFFF0F003u)
        /* ADDR_OFF field */
        #define ROM_TPIU_ADDR_OFF                            (0xFFFFF000u)
        #define ROM_TPIU_ADDR_OFF_MASK                       (0xFFFFF000u)
        #define ROM_TPIU_ADDR_OFF_BIT                        (12)
        #define ROM_TPIU_ADDR_OFF_BITS                       (20)
        /* FORMAT field */
        #define ROM_TPIU_FORMAT                              (0x00000002u)
        #define ROM_TPIU_FORMAT_MASK                         (0x00000002u)
        #define ROM_TPIU_FORMAT_BIT                          (1)
        #define ROM_TPIU_FORMAT_BITS                         (1)
        /* ENTRY_PRES field */
        #define ROM_TPIU_ENTRY_PRES                          (0x00000001u)
        #define ROM_TPIU_ENTRY_PRES_MASK                     (0x00000001u)
        #define ROM_TPIU_ENTRY_PRES_BIT                      (0)
        #define ROM_TPIU_ENTRY_PRES_BITS                     (1)

#define ROM_ETM                                              *((volatile uint32_t *)0xE00FF014u)
#define ROM_ETM_REG                                          *((volatile uint32_t *)0xE00FF014u)
#define ROM_ETM_ADDR                                         (0xE00FF014u)
#define ROM_ETM_RESET                                        (0xFFF0F002u)
        /* ADDR_OFF field */
        #define ROM_ETM_ADDR_OFF                             (0xFFFFF000u)
        #define ROM_ETM_ADDR_OFF_MASK                        (0xFFFFF000u)
        #define ROM_ETM_ADDR_OFF_BIT                         (12)
        #define ROM_ETM_ADDR_OFF_BITS                        (20)
        /* FORMAT field */
        #define ROM_ETM_FORMAT                               (0x00000002u)
        #define ROM_ETM_FORMAT_MASK                          (0x00000002u)
        #define ROM_ETM_FORMAT_BIT                           (1)
        #define ROM_ETM_FORMAT_BITS                          (1)
        /* ENTRY_PRES field */
        #define ROM_ETM_ENTRY_PRES                           (0x00000001u)
        #define ROM_ETM_ENTRY_PRES_MASK                      (0x00000001u)
        #define ROM_ETM_ENTRY_PRES_BIT                       (0)
        #define ROM_ETM_ENTRY_PRES_BITS                      (1)

#define ROM_END                                              *((volatile uint32_t *)0xE00FF018u)
#define ROM_END_REG                                          *((volatile uint32_t *)0xE00FF018u)
#define ROM_END_ADDR                                         (0xE00FF018u)
#define ROM_END_RESET                                        (0x00000000u)
        /* END field */
        #define ROM_END_END                                  (0xFFFFFFFFu)
        #define ROM_END_END_MASK                             (0xFFFFFFFFu)
        #define ROM_END_END_BIT                              (0)
        #define ROM_END_END_BITS                             (32)

#define ROM_MEMTYPE                                          *((volatile uint32_t *)0xE00FFFCCu)
#define ROM_MEMTYPE_REG                                      *((volatile uint32_t *)0xE00FFFCCu)
#define ROM_MEMTYPE_ADDR                                     (0xE00FFFCCu)
#define ROM_MEMTYPE_RESET                                    (0x00000001u)
        /* MEMTYPE field */
        #define ROM_MEMTYPE_MEMTYPE                          (0x00000001u)
        #define ROM_MEMTYPE_MEMTYPE_MASK                     (0x00000001u)
        #define ROM_MEMTYPE_MEMTYPE_BIT                      (0)
        #define ROM_MEMTYPE_MEMTYPE_BITS                     (1)

#define ROM_PID4                                             *((volatile uint32_t *)0xE00FFFD0u)
#define ROM_PID4_REG                                         *((volatile uint32_t *)0xE00FFFD0u)
#define ROM_PID4_ADDR                                        (0xE00FFFD0u)
#define ROM_PID4_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID4_PID                                 (0x0000000Fu)
        #define ROM_PID4_PID_MASK                            (0x0000000Fu)
        #define ROM_PID4_PID_BIT                             (0)
        #define ROM_PID4_PID_BITS                            (4)

#define ROM_PID5                                             *((volatile uint32_t *)0xE00FFFD4u)
#define ROM_PID5_REG                                         *((volatile uint32_t *)0xE00FFFD4u)
#define ROM_PID5_ADDR                                        (0xE00FFFD4u)
#define ROM_PID5_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID5_PID                                 (0x0000000Fu)
        #define ROM_PID5_PID_MASK                            (0x0000000Fu)
        #define ROM_PID5_PID_BIT                             (0)
        #define ROM_PID5_PID_BITS                            (4)

#define ROM_PID6                                             *((volatile uint32_t *)0xE00FFFD8u)
#define ROM_PID6_REG                                         *((volatile uint32_t *)0xE00FFFD8u)
#define ROM_PID6_ADDR                                        (0xE00FFFD8u)
#define ROM_PID6_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID6_PID                                 (0x0000000Fu)
        #define ROM_PID6_PID_MASK                            (0x0000000Fu)
        #define ROM_PID6_PID_BIT                             (0)
        #define ROM_PID6_PID_BITS                            (4)

#define ROM_PID7                                             *((volatile uint32_t *)0xE00FFFDCu)
#define ROM_PID7_REG                                         *((volatile uint32_t *)0xE00FFFDCu)
#define ROM_PID7_ADDR                                        (0xE00FFFDCu)
#define ROM_PID7_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID7_PID                                 (0x0000000Fu)
        #define ROM_PID7_PID_MASK                            (0x0000000Fu)
        #define ROM_PID7_PID_BIT                             (0)
        #define ROM_PID7_PID_BITS                            (4)

#define ROM_PID0                                             *((volatile uint32_t *)0xE00FFFE0u)
#define ROM_PID0_REG                                         *((volatile uint32_t *)0xE00FFFE0u)
#define ROM_PID0_ADDR                                        (0xE00FFFE0u)
#define ROM_PID0_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID0_PID                                 (0x0000000Fu)
        #define ROM_PID0_PID_MASK                            (0x0000000Fu)
        #define ROM_PID0_PID_BIT                             (0)
        #define ROM_PID0_PID_BITS                            (4)

#define ROM_PID1                                             *((volatile uint32_t *)0xE00FFFE4u)
#define ROM_PID1_REG                                         *((volatile uint32_t *)0xE00FFFE4u)
#define ROM_PID1_ADDR                                        (0xE00FFFE4u)
#define ROM_PID1_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID1_PID                                 (0x0000000Fu)
        #define ROM_PID1_PID_MASK                            (0x0000000Fu)
        #define ROM_PID1_PID_BIT                             (0)
        #define ROM_PID1_PID_BITS                            (4)

#define ROM_PID2                                             *((volatile uint32_t *)0xE00FFFE8u)
#define ROM_PID2_REG                                         *((volatile uint32_t *)0xE00FFFE8u)
#define ROM_PID2_ADDR                                        (0xE00FFFE8u)
#define ROM_PID2_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID2_PID                                 (0x0000000Fu)
        #define ROM_PID2_PID_MASK                            (0x0000000Fu)
        #define ROM_PID2_PID_BIT                             (0)
        #define ROM_PID2_PID_BITS                            (4)

#define ROM_PID3                                             *((volatile uint32_t *)0xE00FFFECu)
#define ROM_PID3_REG                                         *((volatile uint32_t *)0xE00FFFECu)
#define ROM_PID3_ADDR                                        (0xE00FFFECu)
#define ROM_PID3_RESET                                       (0x00000000u)
        /* PID field */
        #define ROM_PID3_PID                                 (0x0000000Fu)
        #define ROM_PID3_PID_MASK                            (0x0000000Fu)
        #define ROM_PID3_PID_BIT                             (0)
        #define ROM_PID3_PID_BITS                            (4)

#define ROM_CID0                                             *((volatile uint32_t *)0xE00FFFF0u)
#define ROM_CID0_REG                                         *((volatile uint32_t *)0xE00FFFF0u)
#define ROM_CID0_ADDR                                        (0xE00FFFF0u)
#define ROM_CID0_RESET                                       (0x0000000Du)
        /* CID field */
        #define ROM_CID0_CID                                 (0x000000FFu)
        #define ROM_CID0_CID_MASK                            (0x000000FFu)
        #define ROM_CID0_CID_BIT                             (0)
        #define ROM_CID0_CID_BITS                            (8)

#define ROM_CID1                                             *((volatile uint32_t *)0xE00FFFF4u)
#define ROM_CID1_REG                                         *((volatile uint32_t *)0xE00FFFF4u)
#define ROM_CID1_ADDR                                        (0xE00FFFF4u)
#define ROM_CID1_RESET                                       (0x00000010u)
        /* CID field */
        #define ROM_CID1_CID                                 (0x000000FFu)
        #define ROM_CID1_CID_MASK                            (0x000000FFu)
        #define ROM_CID1_CID_BIT                             (0)
        #define ROM_CID1_CID_BITS                            (8)

#define ROM_CID2                                             *((volatile uint32_t *)0xE00FFFF8u)
#define ROM_CID2_REG                                         *((volatile uint32_t *)0xE00FFFF8u)
#define ROM_CID2_ADDR                                        (0xE00FFFF8u)
#define ROM_CID2_RESET                                       (0x00000005u)
        /* CID field */
        #define ROM_CID2_CID                                 (0x000000FFu)
        #define ROM_CID2_CID_MASK                            (0x000000FFu)
        #define ROM_CID2_CID_BIT                             (0)
        #define ROM_CID2_CID_BITS                            (8)

#define ROM_CID3                                             *((volatile uint32_t *)0xE00FFFFCu)
#define ROM_CID3_REG                                         *((volatile uint32_t *)0xE00FFFFCu)
#define ROM_CID3_ADDR                                        (0xE00FFFFCu)
#define ROM_CID3_RESET                                       (0x000000B1u)
        /* CID field */
        #define ROM_CID3_CID                                 (0x000000FFu)
        #define ROM_CID3_CID_MASK                            (0x000000FFu)
        #define ROM_CID3_CID_BIT                             (0)
        #define ROM_CID3_CID_BITS                            (8)

/* VENDOR block */
#define DATA_VENDOR_BASE                                     (0xE0100000u)
#define DATA_VENDOR_END                                      (0xFFFFFFFFu)
#define DATA_VENDOR_SIZE                                     (DATA_VENDOR_END - DATA_VENDOR_BASE + 1)


#endif  /*REGS_H_*/
