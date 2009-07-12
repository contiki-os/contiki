#ifndef __NVIC_H__LE94F5JS4D__
#define __NVIC_H__LE94F5JS4D__
#include <stm32f10x_map.h>
#include <stm32f10x_nvic.h>

#define NVIC_ENABLE_INT(i) WRITE_REG(NVIC->ISER[(i)/32], 1<<((i) & 0x1f))
#define NVIC_DISABLE_INT(i) WRITE_REG(NVIC->ICER[(i)/32], 1<<((i) & 0x1f))
#define NVIC_SET_PENDING(i) WRITE_REG(NVIC->ISPR[(i)/32], 1<<((i) & 0x1f))
#define NVIC_CLEAR_PENDING(i) WRITE_REG(NVIC->ICPR[(i)/32], 1<<((i) & 0x1f))

#define NVIC_SET_PRIORITY(i,p) \
MODIFY_REG(NVIC->IPR[(i)/4], 0xf<<(((i)&3)*8), (p)<<(((i)&3)*8))

#define NVIC_SET_SYSTICK_PRI(p) MODIFY_REG(SCB->SHPR[2],  0xf<<24, (p)<<24)

#endif /* __NVIC_H__LE94F5JS4D__ */
