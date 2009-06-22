#ifndef LED_H
#define LED_H

#define LED_RED   ((1 << 23) | (1 << 8))
#define LED_GREEN ((1 << 24) | (1 << 9))
#define LED_BLUE  ((1 << 25) | (1 << 10))

#define LED_YELLOW (LED_RED  | LED_GREEN           )
#define LED_PURPLE (LED_RED  |             LED_BLUE)
#define LED_CYAN   (           LED_GREEN | LED_BLUE)
#define LED_WHITE  (LED_RED  | LED_GREEN | LED_BLUE)

#endif
