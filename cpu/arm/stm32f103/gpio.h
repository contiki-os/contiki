#ifndef __GPIO_H__LK7NAD1HN8__
#define __GPIO_H__LK7NAD1HN8__
#include <stm32f10x_map.h>

#define _GPIO_INPUT_ANALOG(h,l) 0

#define _GPIO_INPUT_FLOATING(h,l) (l)

#define _GPIO_INPUT_PU_PD(h,l) (h)

#define _GPIO_OUTPUT_PUSH_PULL(h,l) 0 

#define _GPIO_OUTPUT_OPEN_DRAIN(h,l) (l)

#define _GPIO_OUTPUT_ALT_PUSH_PULL(h,l) (h) 
#define _GPIO_OUTPUT_ALT_OPEN_DRAIN(h,l) ((h)|(l))

#define _GPIO_OUTPUT_SPEED_10(h,l) (l)
#define _GPIO_OUTPUT_SPEED_2(h,l) (h)
#define _GPIO_OUTPUT_SPEED_50(h,l) ((h)|(l))

#define _GPIO_INPUT_CNF(bit,mode)  \
_GPIO_INPUT_##mode(_GPIO_CONF_BIT_REG_##bit(GPIO_,_CNF##bit##_1),\
		   _GPIO_CONF_BIT_REG_##bit(GPIO_,_CNF##bit##_0))

#define _GPIO_OUTPUT_CNF(bit,mode)  \
_GPIO_OUTPUT_##mode(_GPIO_CONF_BIT_REG_##bit(GPIO_,_CNF##bit##_1),\
		   _GPIO_CONF_BIT_REG_##bit(GPIO_,_CNF##bit##_0))

#define _GPIO_OUTPUT_SPEED(bit,speed)  \
_GPIO_OUTPUT_SPEED_##speed(_GPIO_CONF_BIT_REG_##bit(GPIO_,_MODE##bit##_1),\
		   _GPIO_CONF_BIT_REG_##bit(GPIO_,_MODE##bit##_0))

#define _GPIO_CONF_BIT_REG_0(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_1(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_2(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_3(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_4(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_5(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_6(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_7(a,c) a##CRL##c
#define _GPIO_CONF_BIT_REG_8(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_9(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_10(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_11(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_12(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_13(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_14(a,c) a##CRH##c
#define _GPIO_CONF_BIT_REG_15(a,c) a##CRH##c

#define _GPIO_CONF_BIT_REG(b) _GPIO_CONF_BIT_REG_##b(,)


#define _GPIO_CONF_MASK(bit) (_GPIO_CONF_BIT_REG_##bit(GPIO_,_CNF##bit) \
|_GPIO_CONF_BIT_REG_##bit(GPIO_,_MODE##bit))

#define GPIO_CONF_INPUT_PORT(port,bit,mode) MODIFY_REG(GPIO##port -> _GPIO_CONF_BIT_REG(bit),_GPIO_CONF_MASK(bit), _GPIO_INPUT_CNF(bit,mode))


#define GPIO_CONF_OUTPUT_PORT(port,bit,mode,speed) MODIFY_REG(GPIO##port -> _GPIO_CONF_BIT_REG(bit),_GPIO_CONF_MASK(bit), _GPIO_OUTPUT_CNF(bit,mode) | _GPIO_OUTPUT_SPEED(bit,speed))

#ifndef AFIO_MAPR_SWJ_CFG_VALUE
#define AFIO_MAPR_SWJ_CFG_VALUE AFIO_MAPR_SWJ_CFG_RESET
#endif

#define AFIO_REMAP(mask,value) \
MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG | mask, AFIO_MAPR_SWJ_CFG_VALUE | value);

#endif /* __GPIO_H__LK7NAD1HN8__ */
