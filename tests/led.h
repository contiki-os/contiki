#ifndef LED_H
#define LED_H

#define LED_YELLOW ((1ULL << LED_RED)  | (1ULL << LED_GREEN)                     )
#define LED_PURPLE ((1ULL << LED_RED)  |                       (1ULL << LED_BLUE))
#define LED_CYAN   (                     (1ULL << LED_GREEN) | (1ULL << LED_BLUE))
#define LED_WHITE  ((1ULL << LED_RED)  | (1ULL << LED_GREEN) | (1ULL << LED_BLUE))

#endif
