#ifndef GPIO_H_LK7NAD1HN8__
#define GPIO_H_LK7NAD1HN8__
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

#define GPIO_CM0  0x000000000000000fLL
#define GPIO_CM1  0x00000000000000f0LL
#define GPIO_CM2  0x0000000000000f00LL
#define GPIO_CM3  0x000000000000f000LL
#define GPIO_CM4  0x00000000000f0000LL
#define GPIO_CM5  0x0000000000f00000LL
#define GPIO_CM6  0x000000000f000000LL
#define GPIO_CM7  0x00000000f0000000LL
#define GPIO_CM8  0x0000000f00000000LL
#define GPIO_CM9  0x000000f000000000LL
#define GPIO_CM10 0x00000f0000000000LL
#define GPIO_CM11 0x0000f00000000000LL
#define GPIO_CM12 0x000f000000000000LL
#define GPIO_CM13 0x00f0000000000000LL
#define GPIO_CM14 0x0f00000000000000LL
#define GPIO_CM15 0xf000000000000000LL

#define _GPIO_CONF_INPUT_MASK(port, mask , mode)	\
  MODIFY_REG(GPIO##port ->CRH,((mask)>>32), (mode & ((mask)>>32)));	\
  MODIFY_REG(GPIO##port ->CRL,((mask)&0xffffffff), (mode & ((mask)&0xffffffff)))

#define GPIO_CONF_INPUT_ANALOG(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x00000000)

#define GPIO_CONF_INPUT_FLOATING(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x44444444)

#define GPIO_CONF_INPUT_PU_PD(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x88888888)

#define GPIO_CONF_OUTPUT_PUSH_PULL_50(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x22222222)
#define GPIO_CONF_OUTPUT_PUSH_PULL_10(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x33333333)
#define GPIO_CONF_OUTPUT_PUSH_PULL_2(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x11111111)

#define GPIO_CONF_OUTPUT_OPEN_DRAIN_50(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x77777777)
#define GPIO_CONF_OUTPUT_OPEN_DRAIN_10(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x55555555)
#define GPIO_CONF_OUTPUT_OPEN_DRAIN_2(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x66666666)

#define GPIO_CONF_OUTPUT_ALT_PUSH_PULL_50(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0xbbbbbbbb)
#define GPIO_CONF_OUTPUT_ALT_PUSH_PULL_10(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0x99999999)
#define GPIO_CONF_OUTPUT_ALT_PUSH_PULL_2(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0xaaaaaaaa)

#define GPIO_CONF_OUTPUT_ALT_OPEN_DRAIN_50(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0xffffffff)
#define GPIO_CONF_OUTPUT_ALT_OPEN_DRAIN_10(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0xdddddddd)
#define GPIO_CONF_OUTPUT_ALT_OPEN_DRAIN_2(port, mask) \
  _GPIO_CONF_INPUT_MASK(port, mask, 0xeeeeeeee)



#endif /* GPIO_H_LK7NAD1HN8__ */
