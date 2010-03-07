#ifndef LED_H
#define LED_H

#define LED_YELLOW ((1 << LED_RED)  | (1 << LED_GREEN)                  )
#define LED_PURPLE ((1 << LED_RED)  |                    (1 << LED_BLUE))
#define LED_CYAN   (                  (1 << LED_GREEN) | (1 << LED_BLUE))
#define LED_WHITE  ((1 << LED_RED)  | (1 << LED_GREEN) | (1 << LED_BLUE))

#endif
