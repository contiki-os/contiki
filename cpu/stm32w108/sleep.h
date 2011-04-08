
/* Enter system in deep sleep 1 (core power domain is fully
 * powered down and sleep timer is active).
 * Execution is suspended for a given number of seconds.
 *
 * Pay attention! All system peripherals (including sensors) have
 * to be reinitialized before being used again. UART, LEDs and
 * real timers are automatically reinitialized. */

void sleep_seconds(int seconds);
