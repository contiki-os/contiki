#ifndef TESTS_H
#define TESTS_H

#include "put.h"
#include "led.h"

#define NL "\033[K\r\n"

void print_welcome(char* testname);
void dump_regs(uint32_t base, uint32_t len);
void print_packet(volatile packet_t *p);

#endif
