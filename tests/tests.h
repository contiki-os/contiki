#ifndef TESTS_H
#define TESTS_H

#include "put.h"
#include "led.h"

#define NL "\033[K\r\n"

void uart1_init(uint16_t inc, uint16_t mod);
void print_welcome(char* testname);
void dump_regs(uint32_t base, uint32_t len);
void print_packet(packet_t *p);

#endif
