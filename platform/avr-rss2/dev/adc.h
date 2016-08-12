/* Copyright Robert Olsson */

#define V_IN_FACTOR              0.006256  /* 6.4/1023 */
#define V_IN_FACTOR_NONE         0.001564  /* 1.6/1023 */
#define SCHOTTKY_DROP   0.29
#define V_IN_FACTOR_SCHOTTKY     0.0292 /* 30.19-SCHOTTKY_DROP/1023 schottky corr */

uint16_t
adc_read(uint8_t pin);
double
adc_read_v_in(void);
double
adc_read_a1(void);
double
adc_read_a2(void);

