#ifndef UTILS_H
#define UTILS_H

#define CAT2(x, y, z)  x##y##z

#define bit(bit) (1 << bit)
#define bit_is_set(val, bit) (((val & (1 << bit)) >> bit) == 1)
#define clear_bit(val, bit)  (val = (val & ~(1 << bit)))
#define set_bit(val, bit)  (val = (val | (1 << bit)))

#endif /* UTILS_H */
