#ifndef CPU_RF_CORE_RF_RAT_H_
#define CPU_RF_CORE_RF_RAT_H_

/*
 * rfcore-rat.h
 *
 *  Created on: 21 èþë. 2017 ã.
 *      Author: alexrayne
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup rf-core
 * @{
 *
 * \defgroup rf-core-rat CC26xx/CC13xx RAT synchronisation monitor
 * \brief it provides monitor for RAT oveflow monitoring, and RAT timeStamps evaluate
 * @{
 *
 * \file
 * Header file for the CC26xx/CC13xx RAT timer handling.
 */

#include <stdint.h>
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/**
 * RF RAT Front-End usage selection
 * 0:           time stamps not implements, it just retrieved as is
 * 1 (Default): fully implements RAT timestamps at RTTimer domain
 * Not Defined: same
 */
#ifdef RF_RAT_CONF_STYLE
#define RF_RAT_STYLE RF_RAT_CONF_STYLE
#else
#define RF_RAT_STYLE 1
#endif
/*---------------------------------------------------------------------------*/
struct rf_rat_controler{
    uint8_t (*is_on)(void);
    int     (*on)(void);
    int     (*off)(void);
};

typedef uint32_t rf_rat_time_t;

rf_rat_time_t rf_rat_now();

#if RF_RAT_STYLE > 0
/**
 * \brief explicit RAT timer overflow check.
 *  MUST call with powered RF - core
 */
uint_fast8_t rf_rat_check_overflow(bool first_time);

/**
 * \brief Initates and start RAT timer overflow monitor
 * \param rf       - provide an interface for RFcore enabling/disabling
 *  MUST call with powered RF - core
 */
void rf_rat_monitor_init(const struct rf_rat_controler* rf);

/**
 * \brief prepare RAT timer time, for later timestamp evaluate rf_rat_calc_last_packet_rttime
 * \param rat_timestamp - RAT timer value - time of last RF operation
 *  MUST call with powered RF - core
 */
void     rf_rat_last_timestamp(rf_rat_time_t rat_timestamp);

/**
 * \brief evaluates timestamp of prepared RAT value at last rf_rat_last_packet_timestamp
 * \return - timestamp in RTTimer ticks
 */
uint32_t rf_rat_calc_last_rttime(void);

/**
 * \brief just debug prints RAT internals
 */
void rf_rat_debug_dump(void);

#else
#define rf_rat_monitor_init(...)
#define rf_rat_check_overflow(...)
#define rf_rat_debug_dump(...)

extern rf_rat_time_t rf_rat_timestamp;

static inline
void     rf_rat_last_timestamp(rf_rat_time_t t){
    rf_rat_timestamp = t;
}

static inline
uint32_t rf_rat_calc_last_rttime(void){
    return rf_rat_timestamp;
}

#endif

/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

#endif /* CPU_RF_CORE_RF_RAT_H_ */
