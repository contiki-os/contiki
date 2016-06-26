/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-ti-lib TI CC26xxware/CC13xxware Glue
 *
 * Glue file which renames TI CC26xxware functions. Thus, for example,
 * PowerCtrlIOFreezeDisable() becomes power_ctrl_io_freeze_disable()
 *
 * This is not strictly required and a call to the former will work perfectly
 * correctly. However, in using those macros, we make the core of the Contiki
 * port match the naming convention.
 *
 * Since all functions are prefixed with ti_lib, it also becomes clear to the
 * reader that this is a call to TI driverlib's sources and not a call to a
 * function inside Contiki
 *
 * @{
 *
 * \file
 * Header file with macros which rename TI CC26xxware functions.
 */
#ifndef TI_LIB_H_
#define TI_LIB_H_
/*---------------------------------------------------------------------------*/
/* Include ROM API */
#include "ti-lib-rom.h"
/*---------------------------------------------------------------------------*/
/* aon_batmon.h */
#include "driverlib/aon_batmon.h"

#define ti_lib_aon_batmon_enable(...)                    AONBatMonEnable(__VA_ARGS__)
#define ti_lib_aon_batmon_disable(...)                   AONBatMonDisable(__VA_ARGS__)
#define ti_lib_aon_batmon_temperature_get_deg_c(...)     AONBatMonTemperatureGetDegC(__VA_ARGS__)
#define ti_lib_aon_batmon_battery_voltage_get(...)       AONBatMonBatteryVoltageGet(__VA_ARGS__)
#define ti_lib_aon_batmon_new_battery_measure_ready(...) AONBatMonNewBatteryMeasureReady(__VA_ARGS__)
#define ti_lib_aon_batmon_new_temp_measure_ready(...)    AONBatMonNewTempMeasureReady(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aon_event.h */
#include "driverlib/aon_event.h"

#define ti_lib_aon_event_mcu_wake_up_set(...) AONEventMcuWakeUpSet(__VA_ARGS__)
#define ti_lib_aon_event_mcu_wake_up_get(...) AONEventMcuWakeUpGet(__VA_ARGS__)
#define ti_lib_aon_event_aux_wake_up_set(...) AONEventAuxWakeUpSet(__VA_ARGS__)
#define ti_lib_aon_event_aux_wake_up_get(...) AONEventAuxWakeUpGet(__VA_ARGS__)
#define ti_lib_aon_event_mcu_set(...)         AONEventMcuSet(__VA_ARGS__)
#define ti_lib_aon_event_mcu_get(...)         AONEventMcuGet(__VA_ARGS__)
#define ti_lib_aon_event_rtc_set(...)         AONEventRtcSet(__VA_ARGS__)
#define ti_lib_aon_event_rtc_get(...)         AONEventRtcGet(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aon_ioc.h */
#include "driverlib/aon_ioc.h"

#define ti_lib_aon_ioc_drive_strength_set(...)    AONIOCDriveStrengthSet(__VA_ARGS__)
#define ti_lib_aon_ioc_drive_strength_get(...)    AONIOCDriveStrengthGet(__VA_ARGS__)
#define ti_lib_aon_ioc_freeze_enable(...)         AONIOCFreezeEnable(__VA_ARGS__)
#define ti_lib_aon_ioc_freeze_disable(...)        AONIOCFreezeDisable(__VA_ARGS__)
#define ti_lib_aon_ioc_32_khz_output_disable(...) AONIOC32kHzOutputDisable(__VA_ARGS__)
#define ti_lib_aon_ioc_32_khz_output_enable(...)  AONIOC32kHzOutputEnable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aon_rtc.h */
#include "driverlib/aon_rtc.h"

#define ti_lib_aon_rtc_enable(...)                    AONRTCEnable(__VA_ARGS__)
#define ti_lib_aon_rtc_disable(...)                   AONRTCDisable(__VA_ARGS__)
#define ti_lib_aon_rtc_active(...)                    AONRTCActive(__VA_ARGS__)
#define ti_lib_aon_rtc_channel_active(...)            AONRTCChannelActive(__VA_ARGS__)
#define ti_lib_aon_rtc_reset(...)                     AONRTCReset(__VA_ARGS__)
#define ti_lib_aon_rtc_delay_config(...)              AONRTCDelayConfig(__VA_ARGS__)
#define ti_lib_aon_rtc_combined_event_config(...)     AONRTCCombinedEventConfig(__VA_ARGS__)
#define ti_lib_aon_rtc_event_clear(...)               AONRTCEventClear(__VA_ARGS__)
#define ti_lib_aon_rtc_event_get(...)                 AONRTCEventGet(__VA_ARGS__)
#define ti_lib_aon_rtc_sec_get(...)                   AONRTCSecGet(__VA_ARGS__)
#define ti_lib_aon_rtc_fraction_get(...)              AONRTCFractionGet(__VA_ARGS__)
#define ti_lib_aon_rtc_sub_sec_incr_get(...)          AONRTCSubSecIncrGet(__VA_ARGS__)
#define ti_lib_aon_rtc_mode_ch1_set(...)              AONRTCModeCh1Set(__VA_ARGS__)
#define ti_lib_aon_rtc_mode_ch1_get(...)              AONRTCModeCh1Get(__VA_ARGS__)
#define ti_lib_aon_rtc_mode_ch2_set(...)              AONRTCModeCh2Set(__VA_ARGS__)
#define ti_lib_aon_rtc_mode_ch2_get(...)              AONRTCModeCh2Get(__VA_ARGS__)
#define ti_lib_aon_rtc_channel_enable(...)            AONRTCChannelEnable(__VA_ARGS__)
#define ti_lib_aon_rtc_channel_disable(...)           AONRTCChannelDisable(__VA_ARGS__)
#define ti_lib_aon_rtc_compare_value_set(...)         AONRTCCompareValueSet(__VA_ARGS__)
#define ti_lib_aon_rtc_compare_value_get(...)         AONRTCCompareValueGet(__VA_ARGS__)
#define ti_lib_aon_rtc_current_compare_value_get(...) AONRTCCurrentCompareValueGet(__VA_ARGS__)
#define ti_lib_aon_rtc_current_64_bit_value_get(...)  AONRTCCurrent64BitValueGet(__VA_ARGS__)
#define ti_lib_aon_rtc_inc_value_ch2_set(...)         AONRTCIncValueCh2Set(__VA_ARGS__)
#define ti_lib_aon_rtc_inc_value_ch2_get(...)         AONRTCIncValueCh2Get(__VA_ARGS__)
#define ti_lib_aon_rtc_capture_value_ch1_get(...)     AONRTCCaptureValueCh1Get(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aon_wuc.h */
#include "driverlib/aon_wuc.h"

#define ti_lib_aon_wuc_mcu_wake_up_config(...)        AONWUCMcuWakeUpConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_mcu_power_down_config(...)     AONWUCMcuPowerDownConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_mcu_power_off_config(...)      AONWUCMcuPowerOffConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_mcu_sram_config(...)           AONWUCMcuSRamConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_clock_config_get(...)      AONWUCAuxClockConfigGet(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_power_down_config(...)     AONWUCAuxPowerDownConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_wake_up_config(...)        AONWUCAuxWakeUpConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_sram_config(...)           AONWUCAuxSRamConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_wakeup_event(...)          AONWUCAuxWakeupEvent(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_image_valid(...)           AONWUCAuxImageValid(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_image_invalid(...)         AONWUCAuxImageInvalid(__VA_ARGS__)
#define ti_lib_aon_wuc_aux_reset(...)                 AONWUCAuxReset(__VA_ARGS__)
#define ti_lib_aon_wuc_power_status_get(...)          AONWUCPowerStatusGet(__VA_ARGS__)
#define ti_lib_aon_wuc_shut_down_enable(...)          AONWUCShutDownEnable(__VA_ARGS__)
#define ti_lib_aon_wuc_domain_power_down_enable(...)  AONWUCDomainPowerDownEnable(__VA_ARGS__)
#define ti_lib_aon_wuc_domain_power_down_disable(...) AONWUCDomainPowerDownDisable(__VA_ARGS__)
#define ti_lib_aon_wuc_mcu_reset_status_get(...)      AONWUCMcuResetStatusGet(__VA_ARGS__)
#define ti_lib_aon_wuc_mcu_reset_clear(...)           AONWUCMcuResetClear(__VA_ARGS__)
#define ti_lib_aon_wuc_recharge_ctrl_config_set(...)  AONWUCRechargeCtrlConfigSet(__VA_ARGS__)
#define ti_lib_aon_wuc_recharge_ctrl_config_get(...)  AONWUCRechargeCtrlConfigGet(__VA_ARGS__)
#define ti_lib_aon_wuc_osc_config(...)                AONWUCOscConfig(__VA_ARGS__)
#define ti_lib_aon_wuc_jtag_power_off(...)            AONWUCJtagPowerOff(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aux_adc.h */
#include "driverlib/aux_adc.h"

#define ti_lib_aux_adc_disable(...)                            AUXADCDisable(__VA_ARGS__)
#define ti_lib_aux_adc_enable_async(...)                       AUXADCEnableAsync(__VA_ARGS__)
#define ti_lib_aux_adc_enable_sync(...)                        AUXADCEnableSync(__VA_ARGS__)
#define ti_lib_aux_adc_disable_input_scaling(...)              AUXADCDisableInputScaling(__VA_ARGS__)
#define ti_lib_aux_adc_flush_fifo(...)                         AUXADCFlushFifo(__VA_ARGS__)
#define ti_lib_aux_adc_gen_manual_trigger(...)                 AUXADCGenManualTrigger(__VA_ARGS__)
#define ti_lib_aux_adc_get_fifo_status(...)                    AUXADCGetFifoStatus(__VA_ARGS__)
#define ti_lib_aux_adc_read_fifo(...)                          AUXADCReadFifo(__VA_ARGS__)
#define ti_lib_aux_adc_pop_fifo(...)                           AUXADCPopFifo(__VA_ARGS__)
#define ti_lib_aux_adc_select_input(...)                       AUXADCSelectInput(__VA_ARGS__)
#define ti_lib_aux_adc_get_adjustment_gain(...)                AUXADCGetAdjustmentGain(__VA_ARGS__)
#define ti_lib_aux_adc_get_adjustment_offset(...)              AUXADCGetAdjustmentOffset(__VA_ARGS__)
#define ti_lib_aux_adc_value_to_microvolts(...)                AUXADCValueToMicrovolts(__VA_ARGS__)
#define ti_lib_aux_adc_microvolts_to_value(...)                AUXADCMicrovoltsToValue(__VA_ARGS__)
#define ti_lib_aux_adc_adjust_value_for_gain_and_offset(...)   AUXADCAdjustValueForGainAndOffset(__VA_ARGS__)
#define ti_lib_aux_adc_unadjust_value_for_gain_and_offset(...) AUXADCUnadjustValueForGainAndOffset(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* aux_wuc.h */
#include "driverlib/aux_wuc.h"

#define ti_lib_aux_wuc_clock_enable(...)   AUXWUCClockEnable(__VA_ARGS__)
#define ti_lib_aux_wuc_clock_disable(...)  AUXWUCClockDisable(__VA_ARGS__)
#define ti_lib_aux_wuc_clock_status(...)   AUXWUCClockStatus(__VA_ARGS__)
#define ti_lib_aux_wuc_clock_freq_req(...) AUXWUCClockFreqReq(__VA_ARGS__)
#define ti_lib_aux_wuc_power_ctrl(...)     AUXWUCPowerCtrl(__VA_ARGS__)
#define ti_lib_aux_wuc_freeze_enable(...)  AUXWUCFreezeEnable(__VA_ARGS__)
#define ti_lib_aux_wuc_freeze_disable(...) AUXWUCFreezeDisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* cpu.h */
#include "driverlib/cpu.h"

#define ti_lib_cpu_cpsid(...)        CPUcpsid(__VA_ARGS__)
#define ti_lib_cpu_cpsie(...)        CPUcpsie(__VA_ARGS__)
#define ti_lib_cpu_primask(...)      CPUprimask(__VA_ARGS__)
#define ti_lib_cpu_wfi(...)          CPUwfi(__VA_ARGS__)
#define ti_lib_cpu_wfe(...)          CPUwfe(__VA_ARGS__)
#define ti_lib_cpu_sev(...)          CPUsev(__VA_ARGS__)
#define ti_lib_cpu_base_pri_get(...) CPUbasepriGet(__VA_ARGS__)
#define ti_lib_cpu_base_pri_set(...) CPUbasepriSet(__VA_ARGS__)
#define ti_lib_cpu_delay(...)        CPUdelay(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* chipinfo.h */
#include "driverlib/chipinfo.h"

#define ti_lib_chipinfo_get_supported_protocol_bv(...) ChipInfo_GetSupportedProtocol_BV(__VA_ARGS__)
#define ti_lib_chipinfo_supports_ble(...)              ChipInfo_SupportsBLE(__VA_ARGS__)
#define ti_lib_chipinfo_supports_ieee_802_15_4(...)    ChipInfo_SupportsIEEE_802_15_4(__VA_ARGS__)
#define ti_lib_chipinfo_supports_proprietary(...)      ChipInfo_SupportsPROPRIETARY(__VA_ARGS__)
#define ti_lib_chipinfo_get_package_type(...)          ChipInfo_GetPackageType(__VA_ARGS__)
#define ti_lib_chipinfo_package_type_is_4x4(...)       ChipInfo_PackageTypeIs4x4(__VA_ARGS__)
#define ti_lib_chipinfo_package_type_is_5x5(...)       ChipInfo_PackageTypeIs5x5(__VA_ARGS__)
#define ti_lib_chipinfo_package_type_is_7x7(...)       ChipInfo_PackageTypeIs7x7(__VA_ARGS__)
#define ti_lib_chipinfo_get_device_id_hw_rev_code(...) ChipInfo_GetDeviceIdHwRevCode(__VA_ARGS__)
#define ti_lib_chipinfo_get_chip_type(...)             ChipInfo_GetChipType(__VA_ARGS__)
#define ti_lib_chipinfo_get_chip_family(...)           ChipInfo_GetChipFamily(__VA_ARGS__)
#define ti_lib_chipinfo_chip_family_is_cc26xx(...)     ChipInfo_ChipFamilyIsCC26xx(__VA_ARGS__)
#define ti_lib_chipinfo_chip_family_is_cc13xx(...)     ChipInfo_ChipFamilyIsCC13xx(__VA_ARGS__)
#define ti_lib_chipinfo_get_hw_revision(...)           ChipInfo_GetHwRevision(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_1_0(...)        ChipInfo_HwRevisionIs_1_0(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_gteq_2_0(...)   ChipInfo_HwRevisionIs_GTEQ_2_0(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_2_0(...)        ChipInfo_HwRevisionIs_2_0(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_2_1(...)        ChipInfo_HwRevisionIs_2_1(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_2_2(...)        ChipInfo_HwRevisionIs_2_2(__VA_ARGS__)
#define ti_lib_chipinfo_hw_revision_is_gteq_2_2(...)   ChipInfo_HwRevisionIs_GTEQ_2_2( __VA_ARGS__ )
/*---------------------------------------------------------------------------*/
/* ddi.h */
#include "driverlib/ddi.h"

#define ti_lib_aux_adi_ddi_safe_write(...) AuxAdiDdiSafeWrite(__VA_ARGS__)
#define ti_lib_aux_adi_ddi_safe_read(...)  AuxAdiDdiSafeRead(__VA_ARGS__)
#define ti_lib_ddi_32_reg_write(...)       DDI32RegWrite(__VA_ARGS__)
#define ti_lib_ddi_32_reg_read(...)        DDI32RegRead(__VA_ARGS__)
#define ti_lib_ddi_32_bits_set(...)        DDI32BitsSet(__VA_ARGS__)
#define ti_lib_ddi_32_bits_clear(...)      DDI32BitsClear(__VA_ARGS__)
#define ti_lib_ddi_8_set_val_bit(...)      DDI8SetValBit(__VA_ARGS__)
#define ti_lib_ddi_16_set_val_bit(...)     DDI16SetValBit(__VA_ARGS__)
#define ti_lib_ddi_16_bit_write(...)       DDI16BitWrite(__VA_ARGS__)
#define ti_lib_ddi_16_bit_field_write(...) DDI16BitfieldWrite(__VA_ARGS__)
#define ti_lib_ddi_16_bit_read(...)        DDI16BitRead(__VA_ARGS__)
#define ti_lib_ddi_16_bitfield_read(...)   DDI16BitfieldRead(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* gpio.h */
#include "driverlib/gpio.h"

#define ti_lib_gpio_read_dio(...)                    GPIO_readDio(__VA_ARGS__)
#define ti_lib_gpio_read_multi_dio(...)              GPIO_readMultiDio(__VA_ARGS__)
#define ti_lib_gpio_write_dio(...)                   GPIO_writeDio(__VA_ARGS__)
#define ti_lib_gpio_write_multi_dio(...)             GPIO_writeMultiDio(__VA_ARGS__)
#define ti_lib_gpio_set_dio(...)                     GPIO_setDio(__VA_ARGS__)
#define ti_lib_gpio_set_multi_dio(...)               GPIO_setMultiDio(__VA_ARGS__)
#define ti_lib_gpio_clear_dio(...)                   GPIO_clearDio(__VA_ARGS__)
#define ti_lib_gpio_clear_multi_dio(...)             GPIO_clearMultiDio(__VA_ARGS__)
#define ti_lib_gpio_toggle_dio(...)                  GPIO_toggleDio(__VA_ARGS__)
#define ti_lib_gpio_toggle_multi_dio(...)            GPIO_toggleMultiDio(__VA_ARGS__)
#define ti_lib_gpio_get_output_enable_dio(...)       GPIO_getOutputEnableDio(__VA_ARGS__)
#define ti_lib_gpio_get_output_enable_multi_dio(...) GPIO_getOutputEnableMultiDio(__VA_ARGS__)
#define ti_lib_gpio_set_output_enable_dio(...)       GPIO_setOutputEnableDio(__VA_ARGS__)
#define ti_lib_gpio_set_output_enable_multi_dio(...) GPIO_setOutputEnableMultiDio(__VA_ARGS__)
#define ti_lib_gpio_get_event_dio(...)               GPIO_getEventDio(__VA_ARGS__)
#define ti_lib_gpio_get_event_multi_dio(...)         GPIO_getEventMultiDio(__VA_ARGS__)
#define ti_lib_gpio_clear_event_dio(...)             GPIO_clearEventDio(__VA_ARGS__)
#define ti_lib_gpio_clear_event_multi_dio(...)       GPIO_clearEventMultiDio(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* i2c.h */
#include "driverlib/i2c.h"

#define ti_lib_i2c_int_register(...)          I2CIntRegister(__VA_ARGS__)
#define ti_lib_i2c_int_unregister(...)        I2CIntUnregister(__VA_ARGS__)
#define ti_lib_i2c_master_bus_busy(...)       I2CMasterBusBusy(__VA_ARGS__)
#define ti_lib_i2c_master_busy(...)           I2CMasterBusy(__VA_ARGS__)
#define ti_lib_i2c_master_control(...)        I2CMasterControl(__VA_ARGS__)
#define ti_lib_i2c_master_data_get(...)       I2CMasterDataGet(__VA_ARGS__)
#define ti_lib_i2c_master_data_put(...)       I2CMasterDataPut(__VA_ARGS__)
#define ti_lib_i2c_master_disable(...)        I2CMasterDisable(__VA_ARGS__)
#define ti_lib_i2c_master_enable(...)         I2CMasterEnable(__VA_ARGS__)
#define ti_lib_i2c_master_err(...)            I2CMasterErr(__VA_ARGS__)
#define ti_lib_i2c_master_init_exp_clk(...)   I2CMasterInitExpClk(__VA_ARGS__)
#define ti_lib_i2c_master_int_clear(...)      I2CMasterIntClear(__VA_ARGS__)
#define ti_lib_i2c_master_int_disable(...)    I2CMasterIntDisable(__VA_ARGS__)
#define ti_lib_i2c_master_int_enable(...)     I2CMasterIntEnable(__VA_ARGS__)
#define ti_lib_i2c_master_int_status(...)     I2CMasterIntStatus(__VA_ARGS__)
#define ti_lib_i2c_master_slave_addr_set(...) I2CMasterSlaveAddrSet(__VA_ARGS__)
#define ti_lib_i2c_slave_data_get(...)        I2CSlaveDataGet(__VA_ARGS__)
#define ti_lib_i2c_slave_data_put(...)        I2CSlaveDataPut(__VA_ARGS__)
#define ti_lib_i2c_slave_disable(...)         I2CSlaveDisable(__VA_ARGS__)
#define ti_lib_i2c_slave_enable(...)          I2CSlaveEnable(__VA_ARGS__)
#define ti_lib_i2c_slave_init(...)            I2CSlaveInit(__VA_ARGS__)
#define ti_lib_i2c_slave_address_set(...)     I2CSlaveAddressSet(__VA_ARGS__)
#define ti_lib_i2c_slave_int_clear(...)       I2CSlaveIntClear(__VA_ARGS__)
#define ti_lib_i2c_slave_int_disable(...)     I2CSlaveIntDisable(__VA_ARGS__)
#define ti_lib_i2c_slave_int_enable(...)      I2CSlaveIntEnable(__VA_ARGS__)
#define ti_lib_i2c_slave_int_status(...)      I2CSlaveIntStatus(__VA_ARGS__)
#define ti_lib_i2c_slave_status(...)          I2CSlaveStatus(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* interrupt.h */
#include "driverlib/interrupt.h"

#define ti_lib_int_master_enable(...)         IntMasterEnable(__VA_ARGS__)
#define ti_lib_int_master_disable(...)        IntMasterDisable(__VA_ARGS__)
#define ti_lib_int_register(...)              IntRegister(__VA_ARGS__);
#define ti_lib_int_unregsiter(...)            IntUnregister(__VA_ARGS__)
#define ti_lib_int_priority_grouping_set(...) IntPriorityGroupingSet(__VA_ARGS__)
#define ti_lib_int_priority_grouping_get(...) IntPriorityGroupingGet(__VA_ARGS__)
#define ti_lib_int_priority_set(...)          IntPrioritySet(__VA_ARGS__)
#define ti_lib_int_priority_get(...)          IntPriorityGet(__VA_ARGS__)
#define ti_lib_int_enable(...)                IntEnable(__VA_ARGS__)
#define ti_lib_int_disable(...)               IntDisable(__VA_ARGS__)
#define ti_lib_int_pend_set(...)              IntPendSet(__VA_ARGS__)
#define ti_lib_int_pend_get(...)              IntPendGet(__VA_ARGS__)
#define ti_lib_int_pend_clear(...)            IntPendClear(__VA_ARGS__)
#define ti_lib_int_mask_set(...)              IntPriorityMaskSet(__VA_ARGS__)
#define ti_lib_int_mask_get(...)              IntPriorityMaskGet(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* ioc.h */
#include "driverlib/ioc.h"

#define ti_lib_ioc_port_configure_set(...)   IOCPortConfigureSet(__VA_ARGS__)
#define ti_lib_ioc_port_configure_get(...)   IOCPortConfigureGet(__VA_ARGS__)
#define ti_lib_ioc_io_shutdown_set(...)      IOCIOShutdownSet(__VA_ARGS__)
#define ti_lib_ioc_io_mode_set(...)          IOCIOModeSet(__VA_ARGS__)
#define ti_lib_ioc_io_port_pull_set(...)     IOCIOPortPullSet(__VA_ARGS__)
#define ti_lib_ioc_io_hyst_set(...)          IOCIOHystSet(__VA_ARGS__)
#define ti_lib_ioc_io_input_set(...)         IOCIOInputSet(__VA_ARGS__)
#define ti_lib_ioc_io_slew_ctrl_set(...)     IOCIOSlewCtrlSet(__VA_ARGS__)
#define ti_lib_ioc_io_drv_strength_set(...)  IOCIODrvStrengthSet(__VA_ARGS__)
#define ti_lib_ioc_io_port_id_set(...)       IOCIOPortIdSet(__VA_ARGS__)
#define ti_lib_ioc_io_int_set(...)           IOCIOIntSet(__VA_ARGS__)
#define ti_lib_ioc_int_register(...)         IOCIntRegister(__VA_ARGS__);
#define ti_lib_ioc_int_unregister(...)       IOCIntUnregister(__VA_ARGS__)
#define ti_lib_ioc_int_enable(...)           IOCIntEnable(__VA_ARGS__)
#define ti_lib_ioc_int_disable(...)          IOCIntDisable(__VA_ARGS__)
#define ti_lib_ioc_int_clear(...)            IOCIntClear(__VA_ARGS__)
#define ti_lib_ioc_int_status(...)           IOCIntStatus(__VA_ARGS__)
#define ti_lib_ioc_pin_type_gpio_input(...)  IOCPinTypeGpioInput(__VA_ARGS__)
#define ti_lib_ioc_pin_type_gpio_output(...) IOCPinTypeGpioOutput(__VA_ARGS__)
#define ti_lib_ioc_pin_type_uart(...)        IOCPinTypeUart(__VA_ARGS__)
#define ti_lib_ioc_pin_type_ssi_master(...)  IOCPinTypeSsiMaster(__VA_ARGS__)
#define ti_lib_ioc_pin_type_ssi_slave(...)   IOCPinTypeSsiSlave(__VA_ARGS__)
#define ti_lib_ioc_pin_type_i2c(...)         IOCPinTypeI2c(__VA_ARGS__)
#define ti_lib_ioc_pin_type_aux(...)         IOCPinTypeAux(__VA_ARGS__)
#define ti_lib_ioc_pin_type_spis(...)        IOCPinTypeSpis(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* osc.h */
#include "driverlib/osc.h"

#define ti_lib_osc_xhf_power_mode_set(...)                OSCXHfPowerModeSet(__VA_ARGS__)
#define ti_lib_osc_clock_loss_event_enable(...)           OSCClockLossEventEnable(__VA_ARGS__)
#define ti_lib_osc_clock_loss_event_disable(...)          OSCClockLossEventDisable(__VA_ARGS__)
#define ti_lib_osc_clock_source_set(...)                  OSCClockSourceSet(__VA_ARGS__)
#define ti_lib_osc_clock_source_get(...)                  OSCClockSourceGet(__VA_ARGS__)
#define ti_lib_osc_hf_source_ready(...)                   OSCHfSourceReady(__VA_ARGS__)
#define ti_lib_osc_hf_source_switch(...)                  OSCHfSourceSwitch(__VA_ARGS__)
#define ti_lib_osc_hf_get_startup_time(...)               OSCHF_GetStartupTime(__VA_ARGS__)
#define ti_lib_osc_hf_turn_on_xosc(...)                   OSCHF_TurnOnXosc(__VA_ARGS__)
#define ti_lib_osc_hf_attempt_to_switch_to_xosc(...)      OSCHF_AttemptToSwitchToXosc(__VA_ARGS__)
#define ti_lib_osc_hf_debug_get_crystal_amplitude(...)    OSCHF_DebugGetCrystalAmplitude(__VA_ARGS__)
#define ti_lib_osc_hf_debug_get_expected_average_crystal_amplitude(...) \
                                                          OSCHF_DebugGetExpectedAverageCrystalAmplitude(__VA_ARGS__)
#define ti_lib_osc_hposc_relative_frequency_offset_get(...) \
                                                          OSC_HPOSCRelativeFrequencyOffsetGet(__VA_ARGS__)
#define ti_lib_osc_hposc_relative_frequency_offset_to_rf_core_format_convert(...) \
                                                          OSC_HPOSCRelativeFrequencyOffsetToRFCoreFormatConvert(__VA_ARGS__)
#define ti_lib_osc_hf_switch_to_rc_osc_turn_off_xosc(...) OSCHF_SwitchToRcOscTurnOffXosc(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* prcm.h */
#include "driverlib/prcm.h"

#define ti_lib_prcm_inf_clock_configure_set(...)       PRCMInfClockConfigureSet(__VA_ARGS__)
#define ti_lib_prcm_inf_clock_configure_get(...)       PRCMInfClockConfigureGet(__VA_ARGS__)
#define ti_lib_prcm_mcu_power_off(...)                 PRCMMcuPowerOff(__VA_ARGS__)
#define ti_lib_prcm_mcu_power_off_cancel(...)          PRCMMcuPowerOffCancel(__VA_ARGS__)
#define ti_lib_prcm_mcu_uldo_configure(...)            PRCMMcuUldoConfigure(__VA_ARGS__)
#define ti_lib_prcm_audio_clock_enable(...)            PRCMAudioClockEnable(__VA_ARGS__)
#define ti_lib_prcm_audio_clock_disable(...)           PRCMAudioClockDisable(__VA_ARGS__)
#define ti_lib_prcm_audio_clock_config_set(...)        PRCMAudioClockConfigSet(__VA_ARGS__)
#define ti_lib_prcm_load_set(...)                      PRCMLoadSet(__VA_ARGS__)
#define ti_lib_prcm_load_get(...)                      PRCMLoadGet(__VA_ARGS__)
#define ti_lib_prcm_domain_enable(...)                 PRCMDomainEnable(__VA_ARGS__)
#define ti_lib_prcm_domain_disable(...)                PRCMDomainDisable(__VA_ARGS__)
#define ti_lib_prcm_power_domain_on(...)               PRCMPowerDomainOn(__VA_ARGS__)
#define ti_lib_prcm_power_domain_off(...)              PRCMPowerDomainOff(__VA_ARGS__)
#define ti_lib_prcm_rf_power_down_when_idle(...)       PRCMRfPowerDownWhenIdle(__VA_ARGS__)
#define ti_lib_prcm_peripheral_run_enable(...)         PRCMPeripheralRunEnable(__VA_ARGS__)
#define ti_lib_prcm_peripheral_run_disable(...)        PRCMPeripheralRunDisable(__VA_ARGS__)
#define ti_lib_prcm_peripheral_sleep_enable(...)       PRCMPeripheralSleepEnable(__VA_ARGS__)
#define ti_lib_prcm_peripheral_sleep_disable(...)      PRCMPeripheralSleepDisable(__VA_ARGS__)
#define ti_lib_prcm_peripheral_deep_sleep_enable(...)  PRCMPeripheralDeepSleepEnable(__VA_ARGS__)
#define ti_lib_prcm_peripheral_deep_sleep_disable(...) PRCMPeripheralDeepSleepDisable(__VA_ARGS__)
#define ti_lib_prcm_power_domain_status(...)           PRCMPowerDomainStatus(__VA_ARGS__)
#define ti_lib_prcm_rf_ready(...)                      PRCMRfReady(__VA_ARGS__)
#define ti_lib_prcm_sleep(...)                         PRCMSleep(__VA_ARGS__)
#define ti_lib_prcm_deep_sleep(...)                    PRCMDeepSleep(__VA_ARGS__)
#define ti_lib_prcm_cache_retention_enable(...)        PRCMCacheRetentionEnable(__VA_ARGS__)
#define ti_lib_prcm_cache_retention_disable(...)       PRCMCacheRetentionDisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* sys_ctrl.h */
#include "driverlib/pwr_ctrl.h"

#define ti_lib_pwr_ctrl_state_set(...)          PowerCtrlStateSet(__VA_ARGS__)
#define ti_lib_pwr_ctrl_source_set(...)         PowerCtrlSourceSet(__VA_ARGS__)
#define ti_lib_pwr_ctrl_source_get(...)         PowerCtrlSourceGet(__VA_ARGS__)
#define ti_lib_pwr_ctrl_reset_source_get(...)   PowerCtrlResetSourceGet(__VA_ARGS__)
#define ti_lib_pwr_ctrl_reset_source_clear(...) PowerCtrlResetSourceClear(__VA_ARGS__)
#define ti_lib_pwr_ctrl_io_freeze_enable(...)   PowerCtrlIOFreezeEnable(__VA_ARGS__)
#define ti_lib_pwr_ctrl_io_freeze_disable(...)  PowerCtrlIOFreezeDisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* sys_ctrl.h */
#include "driverlib/sys_ctrl.h"

#define ti_lib_sys_ctrl_power_everything(...)                 SysCtrlPowerEverything(__VA_ARGS__)
#define ti_lib_sys_ctrl_powerdown(...)                        SysCtrlPowerdown(__VA_ARGS__)
#define ti_lib_sys_ctrl_standby(...)                          SysCtrlStandby(__VA_ARGS__)
#define ti_lib_sys_ctrl_shutdown(...)                         SysCtrlShutdown(__VA_ARGS__)
#define ti_lib_sys_ctrl_clock_get(...)                        SysCtrlClockGet(__VA_ARGS__)
#define ti_lib_sys_ctrl_aon_sync(...)                         SysCtrlAonSync(__VA_ARGS__)
#define ti_lib_sys_ctrl_aon_update(...)                       SysCtrlAonUpdate(__VA_ARGS__)
#define ti_lib_sys_ctrl_set_recharge_before_power_down(...)   SysCtrlSetRechargeBeforePowerDown(__VA_ARGS__)
#define ti_lib_sys_ctrl_adjust_recharge_after_power_down(...) SysCtrlAdjustRechargeAfterPowerDown(__VA_ARGS__)
#define ti_lib_sys_ctrl_dcdc_voltage_conditional_control(...) SysCtrl_DCDC_VoltageConditionalControl(__VA_ARGS__)
#define ti_lib_sys_ctrl_reset_source_get(...)                 SysCtrlResetSourceGet(__VA_ARGS__)
#define ti_lib_sys_ctrl_system_reset(...)                     SysCtrlSystemReset(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* ssi.h */
#include "driverlib/ssi.h"

#define ti_lib_ssi_config_set_exp_clk(...)    SSIConfigSetExpClk(__VA_ARGS__)
#define ti_lib_ssi_enable(...)                SSIEnable(__VA_ARGS__)
#define ti_lib_ssi_disable(...)               SSIDisable(__VA_ARGS__)
#define ti_lib_ssi_data_put(...)              SSIDataPut(__VA_ARGS__)
#define ti_lib_ssi_data_put_non_blocking(...) SSIDataPutNonBlocking(__VA_ARGS__)
#define ti_lib_ssi_data_get(...)              SSIDataGet(__VA_ARGS__)
#define ti_lib_ssi_data_get_non_blocking(...) SSIDataGetNonBlocking(__VA_ARGS__)
#define ti_lib_ssi_busy(...)                  SSIBusy(__VA_ARGS__)
#define ti_lib_ssi_status(...)                SSIStatus(__VA_ARGS__)
#define ti_lib_ssi_int_register(...)          SSIIntRegister(__VA_ARGS__)
#define ti_lib_ssi_int_unregister(...)        SSIIntUnregister(__VA_ARGS__)
#define ti_lib_ssi_int_enable(...)            SSIIntEnable(__VA_ARGS__)
#define ti_lib_ssi_int_disable(...)           SSIIntDisable(__VA_ARGS__)
#define ti_lib_ssi_int_clear(...)             SSIIntClear(__VA_ARGS__)
#define ti_lib_ssi_int_status(...)            SSIIntStatus(__VA_ARGS__)
#define ti_lib_ssi_dma_enable(...)            SSIDMAEnable(__VA_ARGS__)
#define ti_lib_ssi_dma_disable(...)           SSIDMADisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* systick.h */
#include "driverlib/systick.h"

#define ti_lib_systick_enable(...)         SysTickEnable(__VA_ARGS__)
#define ti_lib_systick_disable(...)        SysTickDisable(__VA_ARGS__)
#define ti_lib_systick_int_register(...)   SysTickIntRegister(__VA_ARGS__)
#define ti_lib_systick_int_unregister(...) SysTickIntUnregister(__VA_ARGS__)
#define ti_lib_systick_int_enable(...)     SysTickIntEnable(__VA_ARGS__)
#define ti_lib_systick_int_disable(...)    SysTickIntDisable(__VA_ARGS__)
#define ti_lib_systick_period_set(...)     SysTickPeriodSet(__VA_ARGS__)
#define ti_lib_systick_period_get(...)     SysTickPeriodGet(__VA_ARGS__)
#define ti_lib_systick_value_get(...)      SysTickValueGet(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* timer.h */
#include "driverlib/timer.h"

#define ti_lib_timer_enable(...)                  TimerEnable(__VA_ARGS__)
#define ti_lib_timer_disable(...)                 TimerDisable(__VA_ARGS__)
#define ti_lib_timer_configure(...)               TimerConfigure(__VA_ARGS__)
#define ti_lib_timer_level_control(...)           TimerLevelControl(__VA_ARGS__)
#define ti_lib_timer_event_control(...)           TimerEventControl(__VA_ARGS__)
#define ti_lib_timer_stall_control(...)           TimerStallControl(__VA_ARGS__)
#define ti_lib_timer_wait_on_trigger_control(...) TimerWaitOnTriggerControl(__VA_ARGS__)
#define ti_lib_timer_rtc_enable(...)              TimerRtcEnable(__VA_ARGS__)
#define ti_lib_timer_rtc_disable(...)             TimerRtcDisable(__VA_ARGS__)
#define ti_lib_timer_prescale_set(...)            TimerPrescaleSet(__VA_ARGS__)
#define ti_lib_timer_prescale_get(...)            TimerPrescaleGet(__VA_ARGS__)
#define ti_lib_timer_prescale_match_set(...)      TimerPrescaleMatchSet(__VA_ARGS__)
#define ti_lib_timer_prescale_match_get(...)      TimerPrescaleMatchGet(__VA_ARGS__)
#define ti_lib_timer_load_set(...)                TimerLoadSet(__VA_ARGS__)
#define ti_lib_timer_load_get(...)                TimerLoadGet(__VA_ARGS__)
#define ti_lib_timer_value_get(...)               TimerValueGet(__VA_ARGS__)
#define ti_lib_timer_match_set(...)               TimerMatchSet(__VA_ARGS__)
#define ti_lib_timer_match_get(...)               TimerMatchGet(__VA_ARGS__)
#define ti_lib_timer_int_register(...)            TimerIntRegister(__VA_ARGS__)
#define ti_lib_timer_int_unregister(...)          TimerIntUnregister(__VA_ARGS__)
#define ti_lib_timer_int_enable(...)              TimerIntEnable(__VA_ARGS__)
#define ti_lib_timer_int_disable(...)             TimerIntDisable(__VA_ARGS__)
#define ti_lib_timer_int_status(...)              TimerIntStatus(__VA_ARGS__)
#define ti_lib_timer_int_clear(...)               TimerIntClear(__VA_ARGS__)
#define ti_lib_timer_synchronize(...)             TimerSynchronize(__VA_ARGS__)
#define ti_lib_timer_ccp_combine_enable(...)      TimerCcpCombineEnable(__VA_ARGS__)
#define ti_lib_timer_ccp_combine_disable(...)     TimerCcpCombineDisable(__VA_ARGS__)
#define ti_lib_timer_match_update_mode(...)       TimerMatchUpdateMode(__VA_ARGS__)
#define ti_lib_timer_interval_load_mode(...)      TimerIntervalLoadMode(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* trng.h */
#include "driverlib/trng.h"

#define ti_lib_trng_configure(...)      TRNGConfigure(__VA_ARGS__)
#define ti_lib_trng_enable(...)         TRNGEnable(__VA_ARGS__)
#define ti_lib_trng_disable(...)        TRNGDisable(__VA_ARGS__)
#define ti_lib_trng_number_get(...)     TRNGNumberGet(__VA_ARGS__)
#define ti_lib_trng_status_get(...)     TRNGStatusGet(__VA_ARGS__)
#define ti_lib_trng_reset(...)          TRNGReset(__VA_ARGS__)
#define ti_lib_trng_int_enable(...)     TRNGIntEnable(__VA_ARGS__)
#define ti_lib_trng_int_disable(...)    TRNGIntDisable(__VA_ARGS__)
#define ti_lib_trng_int_status(...)     TRNGIntStatus(__VA_ARGS__)
#define ti_lib_trng_int_clear(...)      TRNGIntClear(__VA_ARGS__)
#define ti_lib_trng_int_register(...)   TRNGIntRegister(__VA_ARGS__)
#define ti_lib_trng_int_unregister(...) TRNGIntUnregister(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* uart.h */
#include "driverlib/uart.h"

#define ti_lib_uart_parity_mode_set(...)       UARTParityModeSet(__VA_ARGS__)
#define ti_lib_uart_parity_mode_get(...)       UARTParityModeGet(__VA_ARGS__)
#define ti_lib_uart_fifo_level_set(...)        UARTFIFOLevelSet(__VA_ARGS__)
#define ti_lib_uart_fifo_level_get(...)        UARTFIFOLevelGet(__VA_ARGS__)
#define ti_lib_uart_config_set_exp_clk(...)    UARTConfigSetExpClk(__VA_ARGS__)
#define ti_lib_uart_config_get_exp_clk(...)    UARTConfigGetExpClk(__VA_ARGS__)
#define ti_lib_uart_enable(...)                UARTEnable(__VA_ARGS__)
#define ti_lib_uart_disable(...)               UARTDisable(__VA_ARGS__)
#define ti_lib_uart_fifo_enable(...)           UARTFIFOEnable(__VA_ARGS__)
#define ti_lib_uart_fifo_disable(...)          UARTFIFODisable(__VA_ARGS__)
#define ti_lib_uart_chars_avail(...)           UARTCharsAvail(__VA_ARGS__)
#define ti_lib_uart_space_avail(...)           UARTSpaceAvail(__VA_ARGS__)
#define ti_lib_uart_char_get_non_blocking(...) UARTCharGetNonBlocking(__VA_ARGS__)
#define ti_lib_uart_char_get(...)              UARTCharGet(__VA_ARGS__)
#define ti_lib_uart_char_put_non_blocking(...) UARTCharPutNonBlocking(__VA_ARGS__)
#define ti_lib_uart_char_put(...)              UARTCharPut(__VA_ARGS__)
#define ti_lib_uart_break_ctl(...)             UARTBreakCtl(__VA_ARGS__)
#define ti_lib_uart_busy(...)                  UARTBusy(__VA_ARGS__)
#define ti_lib_uart_int_register(...)          UARTIntRegister(__VA_ARGS__)
#define ti_lib_uart_int_unregister(...)        UARTIntUnregister(__VA_ARGS__)
#define ti_lib_uart_int_enable(...)            UARTIntEnable(__VA_ARGS__)
#define ti_lib_uart_int_disable(...)           UARTIntDisable(__VA_ARGS__)
#define ti_lib_uart_int_status(...)            UARTIntStatus(__VA_ARGS__)
#define ti_lib_uart_int_clear(...)             UARTIntClear(__VA_ARGS__)
#define ti_lib_uart_dma_enable(...)            UARTDMAEnable(__VA_ARGS__)
#define ti_lib_uart_dma_disable(...)           UARTDMADisable(__VA_ARGS__)
#define ti_lib_uart_rx_error_get(...)          UARTRxErrorGet(__VA_ARGS__)
#define ti_lib_uart_rx_error_clear(...)        UARTRxErrorClear(__VA_ARGS__)
#define ti_lib_uart_hw_flow_control_en(...)    UARTHwFlowControlEnable(__VA_ARGS__)
#define ti_lib_uart_hw_flow_control_dis(...)   UARTHwFlowControlDisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* vims.h */
#include "driverlib/vims.h"

#define ti_lib_vims_configure(...) VIMSConfigure(__VA_ARGS__)
#define ti_lib_vims_mode_set(...)  VIMSModeSet(__VA_ARGS__)
#define ti_lib_vims_mode_get(...)  VIMSModeGet(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/* watchdog.h */
#include "driverlib/watchdog.h"

#define ti_lib_watchdog_running(...)        WatchdogRunning(__VA_ARGS__)
#define ti_lib_watchdog_enable(...)         WatchdogEnable(__VA_ARGS__)
#define ti_lib_watchdog_reset_enable(...)   WatchdogResetEnable(__VA_ARGS__)
#define ti_lib_watchdog_reset_disable(...)  WatchdogResetDisable(__VA_ARGS__)
#define ti_lib_watchdog_lock(...)           WatchdogLock(__VA_ARGS__)
#define ti_lib_watchdog_unlock(...)         WatchdogUnlock(__VA_ARGS__)
#define ti_lib_watchdog_lock_state(...)     WatchdogLockState(__VA_ARGS__)
#define ti_lib_watchdog_reload_set(...)     WatchdogReloadSet(__VA_ARGS__)
#define ti_lib_watchdog_reload_get(...)     WatchdogReloadGet(__VA_ARGS__)
#define ti_lib_watchdog_value_get(...)      WatchdogValueGet(__VA_ARGS__)
#define ti_lib_watchdog_int_register(...)   WatchdogIntRegister(__VA_ARGS__)
#define ti_lib_watchdog_int_unregister(...) WatchdogIntUnregister(__VA_ARGS__)
#define ti_lib_watchdog_int_enable(...)     WatchdogIntEnable(__VA_ARGS__)
#define ti_lib_watchdog_int_status(...)     WatchdogIntStatus(__VA_ARGS__)
#define ti_lib_watchdog_int_clear(...)      WatchdogIntClear(__VA_ARGS__)
#define ti_lib_watchdog_int_type_set(...)   WatchdogIntTypeSet(__VA_ARGS__)
#define ti_lib_watchdog_stall_enable(...)   WatchdogStallEnable(__VA_ARGS__)
#define ti_lib_watchdog_stall_disable(...)  WatchdogStallDisable(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
#endif /* TI_LIB_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
