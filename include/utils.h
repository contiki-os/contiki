#ifndef UTILS_H
#define UTILS_H

#define reg32(x) (*(volatile uint32_t *)(x))
#define reg16(x) (*(volatile uint16_t *)(x))

#define bit(bit) (1<<bit)
#define bit_is_set(val,bit) (((val & (1<<bit)) >> bit) == 1)
#define clear_bit(val,bit)  (val=(val & ~(1<<bit)))
#define set_bit(val,bit)  (val=(val | (1<<bit)))

#endif
