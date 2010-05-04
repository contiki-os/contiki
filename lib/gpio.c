#include <mc1322x.h>
#include <types.h>

inline void gpio_pad_dir(volatile uint64_t data) 
{
	*GPIO_PAD_DIR0 = (data & 0xffffffff);
	*GPIO_PAD_DIR1 = (data >> 32);
}

inline void gpio_data(volatile uint64_t data) 
{
	*GPIO_DATA0 = (data & 0xffffffff);
	*GPIO_DATA1 = (data >> 32);
}

inline uint64_t gpio_data_get(volatile uint64_t bits) {
	uint64_t rdata = 0;

	rdata = *GPIO_DATA0 & (bits & 0xffffffff);
	rdata |= (*GPIO_DATA1 & (bits >> 32)) << 32;

	return rdata;
}

inline void gpio_pad_pu_en(volatile uint64_t data) 
{
	*GPIO_PAD_PU_EN0 = (data & 0xffffffff);
	*GPIO_PAD_PU_EN1 = (data >> 32);
}

inline void gpio_data_sel(volatile uint64_t data) 
{
	*GPIO_DATA_SEL0 = (data & 0xffffffff);
	*GPIO_DATA_SEL1 = (data >> 32);
}

inline void gpio_pad_pu_sel(volatile uint64_t data) 
{
	*GPIO_PAD_PU_SEL0 = (data & 0xffffffff);
	*GPIO_PAD_PU_SEL1 = (data >> 32);
}

inline void gpio_data_set(volatile uint64_t data) 
{
	*GPIO_DATA_SET0 = (data & 0xffffffff);
	*GPIO_DATA_SET1 = (data >> 32);
}

inline void gpio_data_reset(volatile uint64_t data) 
{
	*GPIO_DATA_RESET0 = (data & 0xffffffff);
	*GPIO_DATA_RESET1 = (data >> 32);
}

inline void gpio_pad_dir_set(volatile uint64_t data) 
{
	*GPIO_PAD_DIR_SET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_SET1 = (data >> 32);
}

inline void gpio_pad_dir_reset(volatile uint64_t data) 
{
	*GPIO_PAD_DIR_RESET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_RESET1 = (data >> 32);
}
