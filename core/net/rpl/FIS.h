
#ifndef FIS_H
#define FIS_H
#include <stdint.h>

uint8_t qos(uint16_t etx, uint16_t delay, uint16_t hc);
uint8_t quality(uint16_t q, uint16_t e);

//etx
#define etx_short(etx) first_t_norm(3, 6, etx)
#define etx_avg(etx) t_norm(3, 6, 9, 12, etx)
#define etx_long(etx) last_t_norm(9, 12, etx)

//delay
#define dly_small(dly) first_t_norm(600, 1200, dly)
#define dly_avg(dly) t_norm(600, 1200, 1800, 2400, dly)
#define dly_high(dly) last_t_norm(1800, 2400, dly)

// hc
#define hc_near(hc) first_t_norm(10, 20, hc)
#define hc_avg(hc) t_norm(10, 20, 30, 40, hc)
#define hc_far(hc) last_t_norm(30, 40, hc)

//energy
#define energy_low(eng) first_t_norm(51, 102, (eng))
#define energy_medium(eng) t_norm(51, 102, 153, 205, (eng))
#define energy_full(eng) last_t_norm(153, 205, (eng))


/*QoS = FUZZY_COMBINATION(ETX,DELAY)*/

#define QoS_B1 15
#define QoS_B2 25
#define QoS_B3 35
#define QoS_B4 45
#define QoS_B5 55
#define QoS_B6 65
#define QoS_B7 75
#define QoS_B8 85

#define qos_very_slow(x) first_t_norm(QoS_B1, QoS_B2,x)
#define qos_slow(x) t_norm(QoS_B1, QoS_B2, QoS_B3, QoS_B4, x)
#define qos_avg(x) t_norm(QoS_B3, QoS_B4, QoS_B5, QoS_B6, x)
#define qos_fast(x) t_norm(QoS_B5, QoS_B6, QoS_B7, QoS_B8, x)
#define qos_very_fast(x) last_t_norm(QoS_B7, QoS_B8, x)

#endif
