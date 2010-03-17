#include <mc1322x.h>
#include <types.h>

inline void gpio_pad_dir(uint64_t data) 
{
	*GPIO_PAD_DIR0 = (data & 0xffffffff);
	*GPIO_PAD_DIR1 = (data >> 32);
}

inline void gpio_data(uint64_t data) 
{
	*GPIO_DATA0 = (data & 0xffffffff);
	*GPIO_DATA1 = (data >> 32);
}

inline void gpio_pad_pu_en(uint64_t data) 
{
	*GPIO_PAD_PU_EN0 = (data & 0xffffffff);
	*GPIO_PAD_PU_EN1 = (data >> 32);
}

inline void gpio_data_sel(uint64_t data) 
{
	*GPIO_DATA_SEL0 = (data & 0xffffffff);
	*GPIO_DATA_SEL1 = (data >> 32);
}

inline void gpio_pad_pu_sel(uint64_t data) 
{
	*GPIO_PAD_PU_SEL0 = (data & 0xffffffff);
	*GPIO_PAD_PU_SEL1 = (data >> 32);
}

inline void gpio_data_set(uint64_t data) 
{
	*GPIO_DATA_SET0 = (data & 0xffffffff);
	*GPIO_DATA_SET1 = (data >> 32);
}

inline void gpio_data_reset(uint64_t data) 
{
	*GPIO_DATA_RESET0 = (data & 0xffffffff);
	*GPIO_DATA_RESET1 = (data >> 32);
}

inline void gpio_pad_dir_set(uint64_t data) 
{
	*GPIO_PAD_DIR_SET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_SET1 = (data >> 32);
}

inline void gpio_pad_dir_reset(uint64_t data) 
{
	*GPIO_PAD_DIR_RESET0 = (data & 0xffffffff);
	*GPIO_PAD_DIR_RESET1 = (data >> 32);
}
