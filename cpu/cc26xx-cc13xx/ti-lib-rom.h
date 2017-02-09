/*
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-ti-lib
 * @{
 *
 * \file
 * Header file with CC13xxware/CC26xxware ROM API.
 */
/*---------------------------------------------------------------------------*/
#ifndef TI_LIB_ROM_H_
#define TI_LIB_ROM_H_
/*---------------------------------------------------------------------------*/
/* rom.h */
#include "driverlib/rom.h"

/* AON API */
#define ti_lib_rom_aon_event_mcu_wake_up_set          ROM_AONEventMcuWakeUpSet
#define ti_lib_rom_aon_event_mcu_wake_up_get          ROM_AONEventMcuWakeUpGet
#define ti_lib_rom_aon_event_aux_wake_up_set          ROM_AONEventAuxWakeUpSet
#define ti_lib_rom_aon_event_aux_wake_up_get          ROM_AONEventAuxWakeUpGet
#define ti_lib_rom_aon_event_mcu_set                  ROM_AONEventMcuSet
#define ti_lib_rom_aon_event_mcu_get                  ROM_AONEventMcuGet

/* AON_WUC API */
#define ti_lib_rom_aon_wuc_aux_reset                  ROM_AONWUCAuxReset
#define ti_lib_rom_aon_wuc_recharge_ctrl_config_set   ROM_AONWUCRechargeCtrlConfigSet
#define ti_lib_rom_aon_wuc_osc_config                 ROM_AONWUCOscConfig

/* AUX_TDC API */
#define ti_lib_rom_aux_tdc_config_set                 ROM_AUXTDCConfigSet
#define ti_lib_rom_aux_tdc_measurement_done           ROM_AUXTDCMeasurementDone

/* AUX_WUC API */
#define ti_lib_rom_aux_wuc_clock_enable               ROM_AUXWUCClockEnable
#define ti_lib_rom_aux_wuc_clock_disable              ROM_AUXWUCClockDisable
#define ti_lib_rom_aux_wuc_clock_status               ROM_AUXWUCClockStatus
#define ti_lib_rom_aux_wuc_power_ctrl                 ROM_AUXWUCPowerCtrl

/* FLASH API */
#define ti_lib_rom_flash_power_mode_get               ROM_FlashPowerModeGet
#define ti_lib_rom_flash_protection_set               ROM_FlashProtectionSet
#define ti_lib_rom_flash_protection_get               ROM_FlashProtectionGet
#define ti_lib_rom_flash_protection_save              ROM_FlashProtectionSave
#define ti_lib_rom_flash_efuse_read_row               ROM_FlashEfuseReadRow
#define ti_lib_rom_flash_disable_sectors_for_write    ROM_FlashDisableSectorsForWrite

/* I2C API */
#define ti_lib_rom_i2c_master_init_exp_clk            ROM_I2CMasterInitExpClk
#define ti_lib_rom_i2c_master_err                     ROM_I2CMasterErr

/* INTERRUPT API */
#define ti_lib_rom_int_priority_grouping_set          ROM_IntPriorityGroupingSet
#define ti_lib_rom_int_priority_grouping_get          ROM_IntPriorityGroupingGet
#define ti_lib_rom_int_priority_set                   ROM_IntPrioritySet
#define ti_lib_rom_int_priority_get                   ROM_IntPriorityGet
#define ti_lib_rom_int_enable                         ROM_IntEnable
#define ti_lib_rom_int_disable                        ROM_IntDisable
#define ti_lib_rom_int_pend_set                       ROM_IntPendSet
#define ti_lib_rom_int_pend_get                       ROM_IntPendGet
#define ti_lib_rom_int_pend_clear                     ROM_IntPendClear

/* IOC API */
#define ti_lib_rom_ioc_port_configure_set             ROM_IOCPortConfigureSet
#define ti_lib_rom_ioc_port_configure_get             ROM_IOCPortConfigureGet
#define ti_lib_rom_ioc_io_shutdown_set                ROM_IOCIOShutdownSet
#define ti_lib_rom_ioc_io_mode_set                    ROM_IOCIOModeSet
#define ti_lib_rom_ioc_io_int_set                     ROM_IOCIOIntSet
#define ti_lib_rom_ioc_io_port_pull_set               ROM_IOCIOPortPullSet
#define ti_lib_rom_ioc_io_hyst_set                    ROM_IOCIOHystSet
#define ti_lib_rom_ioc_io_input_set                   ROM_IOCIOInputSet
#define ti_lib_rom_ioc_io_slew_ctrl_set               ROM_IOCIOSlewCtrlSet
#define ti_lib_rom_ioc_io_drv_strength_set            ROM_IOCIODrvStrengthSet
#define ti_lib_rom_ioc_io_port_id_set                 ROM_IOCIOPortIdSet
#define ti_lib_rom_ioc_int_enable                     ROM_IOCIntEnable
#define ti_lib_rom_ioc_int_disable                    ROM_IOCIntDisable
#define ti_lib_rom_ioc_pin_type_gpio_input            ROM_IOCPinTypeGpioInput
#define ti_lib_rom_ioc_pin_type_gpio_output           ROM_IOCPinTypeGpioOutput
#define ti_lib_rom_ioc_pin_type_uart                  ROM_IOCPinTypeUart
#define ti_lib_rom_ioc_pin_type_ssi_master            ROM_IOCPinTypeSsiMaster
#define ti_lib_rom_ioc_pin_type_ssi_slave             ROM_IOCPinTypeSsiSlave
#define ti_lib_rom_ioc_pin_type_i2c                   ROM_IOCPinTypeI2c
#define ti_lib_rom_ioc_pin_type_aux                   ROM_IOCPinTypeAux

/* PRCM API */
#define ti_lib_rom_prcm_inf_clock_configure_set       ROM_PRCMInfClockConfigureSet
#define ti_lib_rom_prcm_inf_clock_configure_get       ROM_PRCMInfClockConfigureGet
#define ti_lib_rom_prcm_audio_clock_config_set        ROM_PRCMAudioClockConfigSet
#define ti_lib_rom_prcm_power_domain_on               ROM_PRCMPowerDomainOn
#define ti_lib_rom_prcm_power_domain_off              ROM_PRCMPowerDomainOff
#define ti_lib_rom_prcm_peripheral_run_enable         ROM_PRCMPeripheralRunEnable
#define ti_lib_rom_prcm_peripheral_run_disable        ROM_PRCMPeripheralRunDisable
#define ti_lib_rom_prcm_peripheral_sleep_enable       ROM_PRCMPeripheralSleepEnable
#define ti_lib_rom_prcm_peripheral_sleep_disable      ROM_PRCMPeripheralSleepDisable
#define ti_lib_rom_prcm_peripheral_deep_sleep_enable  ROM_PRCMPeripheralDeepSleepEnable
#define ti_lib_rom_prcm_peripheral_deep_sleep_disable ROM_PRCMPeripheralDeepSleepDisable
#define ti_lib_rom_prcm_power_domain_status           ROM_PRCMPowerDomainStatus
#define ti_lib_rom_prcm_deep_sleep                    ROM_PRCMDeepSleep

/* SMPH API */
#define ti_lib_rom_smph_acquire                       ROM_SMPHAcquire

/* SSI API */
#define ti_lib_rom_ssi_config_set_exp_clk             ROM_SSIConfigSetExpClk
#define ti_lib_rom_ssi_data_put                       ROM_SSIDataPut
#define ti_lib_rom_ssi_data_put_non_blocking          ROM_SSIDataPutNonBlocking
#define ti_lib_rom_ssi_data_get                       ROM_SSIDataGet
#define ti_lib_rom_ssi_data_get_non_blocking          ROM_SSIDataGetNonBlocking

/* TIMER API */
#define ti_lib_rom_timer_configure                    ROM_TimerConfigure
#define ti_lib_rom_timer_level_control                ROM_TimerLevelControl
#define ti_lib_rom_timer_stall_control                ROM_TimerStallControl
#define ti_lib_rom_timer_wait_on_trigger_control      ROM_TimerWaitOnTriggerControl

/* TRNG API */
#define ti_lib_rom_trng_number_get                    ROM_TRNGNumberGet

/* UART API */
#define ti_lib_rom_uart_fifo_level_get                ROM_UARTFIFOLevelGet
#define ti_lib_rom_uart_config_set_exp_clk            ROM_UARTConfigSetExpClk
#define ti_lib_rom_uart_config_get_exp_clk            ROM_UARTConfigGetExpClk
#define ti_lib_rom_uart_disable                       ROM_UARTDisable
#define ti_lib_rom_uart_char_get_non_blocking         ROM_UARTCharGetNonBlocking
#define ti_lib_rom_uart_char_get                      ROM_UARTCharGet
#define ti_lib_rom_uart_char_put_non_blocking         ROM_UARTCharPutNonBlocking
#define ti_lib_rom_uart_char_put                      ROM_UARTCharPut

/* UDMA API */
#define ti_lib_rom_udma_channel_attribute_enable      ROM_uDMAChannelAttributeEnable
#define ti_lib_rom_udma_channel_attribute_disable     ROM_uDMAChannelAttributeDisable
#define ti_lib_rom_udma_channel_attribute_get         ROM_uDMAChannelAttributeGet
#define ti_lib_rom_udma_channel_control_set           ROM_uDMAChannelControlSet
#define ti_lib_rom_udma_channel_transfer_set          ROM_uDMAChannelTransferSet
#define ti_lib_rom_udma_channel_scatter_gather_set    ROM_uDMAChannelScatterGatherSet
#define ti_lib_rom_udma_channel_size_get              ROM_uDMAChannelSizeGet
#define ti_lib_rom_udma_channel_mode_get              ROM_uDMAChannelModeGet

/* VIMS API */
#define ti_lib_rom_vims_configure                     ROM_VIMSConfigure
#define ti_lib_rom_vims_mode_set                      ROM_VIMSModeSet

/* HAPI */
#define ti_lib_hapi_crc32(a, b, c)              HapiCrc32(a, b, c)
#define ti_lib_hapi_get_flash_size()            HapiGetFlashSize()
#define ti_lib_hapi_get_chip_id()               HapiGetChipId()
#define ti_lib_hapi_sector_erase(a)             HapiSectorErase(a)
#define ti_lib_hapi_program_flash(a, b, c)      HapiProgramFlash(a, b, c)
#define ti_lib_hapi_reset_device()              HapiResetDevice()
#define ti_lib_hapi_fletcher32(a, b, c)         HapiFletcher32(a, b, c)
#define ti_lib_hapi_min_value(a, b)             HapiMinValue(a,b)
#define ti_lib_hapi_max_value(a, b)             HapiMaxValue(a,b)
#define ti_lib_hapi_mean_value(a, b)            HapiMeanValue(a,b)
#define ti_lib_hapi_stand_deviation_value(a, b) HapiStandDeviationValue(a,b)
#define ti_lib_hapi_hf_source_safe_switch()     HapiHFSourceSafeSwitch()
#define ti_lib_hapi_select_comp_a_input(a)      HapiSelectCompAInput(a)
#define ti_lib_hapi_select_comp_a_ref(a)        HapiSelectCompARef(a)
#define ti_lib_hapi_select_adc_comp_b_input(a)  HapiSelectADCCompBInput(a)
#define ti_lib_hapi_select_comp_b_ref(a)        HapiSelectCompBRef(a)
/*---------------------------------------------------------------------------*/
#endif /* TI_LIB_ROM_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
