#ifndef UTILS_H
#define UTILS_H

#define mem32(x) ((volatile uint32_t *)(x))
#define mem16(x) ((volatile uint16_t *)(x))

#define CAT2(x, y, z)  x##y##z

#define STR(x) #x
#define STR2(x) STR(x)

#define bit(bit) (1 << bit)
#define bit_is_set(val, bit) (((val & (1 << bit)) >> bit) == 1)
#define clear_bit(val, bit)  (val = (val & ~(1 << bit)))
#define set_bit(val, bit)  (val = (val | (1 << bit)))

#define ones(num) ( (1ULL << num) - 1 )
#define bit_mask(length, shift) (ones(length) << shift)
#define get_field(val, field) ((val & field##_MASK) >> field)
//#define bitfield(name, length, shift) ( #define #name length  #define #name##_MASK bit_mask(length, shift))
#define bitfield(name, length, shift) ( define #name length )

#endif /* UTILS_H */
