
#ifndef GPIO_UTIL_H
#define GPIO_UTIL_H

#include <stdbool.h>
#include <stdint.h>

void gpio_select_function(uint8_t gpio, uint8_t func);
void gpio_reg_set(volatile uint32_t* reg, uint8_t bit);
void gpio_reg_clear(volatile uint32_t* reg, uint8_t bit);

#define PAD_DIR_INPUT 0
#define PAD_DIR_OUTPUT 1
void gpio_set_pad_dir(uint8_t gpio, uint8_t dir);

#undef gpio_set
#undef gpio_reset
#undef gpio_read

//#define gpio_set gpio_set_ian
//#define gpio_reset gpio_reset_ian
//#define gpio_read gpio_read_ian

void gpio_set(uint8_t gpio);
void gpio_reset(uint8_t gpio);
bool gpio_read(uint8_t gpio);

#endif
