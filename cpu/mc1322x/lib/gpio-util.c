
#include <mc1322x.h>
#include <board.h>

#include "gpio-util.h"

void gpio_select_function(uint8_t gpio, uint8_t func) {
	uint32_t mask = 3;
	uint8_t major, minor, shift;
	volatile uint32_t *base = GPIO_FUNC_SEL0;
	uint32_t value;
	major = gpio >> 4;
	minor = gpio & 0xF;
	shift = 2 * minor;

	value = base[major];
	value &= ~(mask << shift);
	value |= (func << shift);
	base[major] = value;
}

void gpio_reg_set(volatile uint32_t* reg, uint8_t bit) {
	uint8_t major, minor;
	major = bit / 32;
	minor = bit % 32;
	*(reg + major) |= (1UL << minor);
}

void gpio_reg_clear(volatile uint32_t* reg, uint8_t bit) {
	uint8_t major, minor;
	major = bit / 32;
	minor = bit % 32;
	*(reg + major) &= ~(1UL << minor);
}

void gpio_set_pad_dir(uint8_t gpio, uint8_t dir) {
	uint8_t major, minor;
	major = gpio / 32;
	minor = gpio % 32;
	if (dir) gpio_reg_set(GPIO_PAD_DIR0 + major, minor);
	else     gpio_reg_clear(GPIO_PAD_DIR0 + major, minor);
}

void gpio_set(uint8_t gpio) {
	uint8_t major, minor;
	major = gpio / 32;
	minor = gpio % 32;
	*(GPIO_DATA_SET0 + major) = (1UL << minor);
}

void gpio_reset(uint8_t gpio) {
	uint8_t major, minor;
	major = gpio / 32;
	minor = gpio % 32;
	*(GPIO_DATA_RESET0 + major) = (1UL << minor);
}

bool gpio_read(uint8_t gpio) {
	uint8_t major, minor;
	major = gpio / 32;
	minor = gpio % 32;
	return (*(GPIO_DATA0 + major) >> minor) & 1;
}
