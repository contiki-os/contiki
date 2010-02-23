#ifndef LED_H
#define LED_H

#define LED_YELLOW (LED_RED  | LED_GREEN           )
#define LED_PURPLE (LED_RED  |             LED_BLUE)
#define LED_CYAN   (           LED_GREEN | LED_BLUE)
#define LED_WHITE  (LED_RED  | LED_GREEN | LED_BLUE)

#endif
