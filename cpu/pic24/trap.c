
#include <stdint.h>
#include <stdio.h>
#include <p33Fxxxx.h>

extern uint16_t _SP_init;
extern uint16_t _SPLIM_init;

uint16_t *
print_frame(uint16_t *frame)
{
  uint32_t pc = (uint32_t)(frame[-3]) | ((uint32_t)(frame[-2]) << 16);
  printf("f = %04x, caller = %08lx\n", (uint16_t)frame, pc);
  return (uint16_t *)(frame[-1]);
}
void
handle_trap(uint16_t *fp, uint16_t *frame)
{
  int i;
  uint32_t pc = (uint32_t)frame[0] | (((uint32_t)frame[1] & 0x7f) << 16);
  printf("BOOM: %08lx\n", pc);

  printf("INTCON1 INTCON2 IFS1 IFS2 IFS3 IFS4\n");
  printf("%04x    %04x    %04x %04x %04x %04x\n", INTCON1, INTCON2, IFS1, IFS2, IFS3, IFS4);

  printf("RCNT  W0   W1   W2   W3   W4   W5   W6   W7\n");
  for(i = 2; i < 11; ++i) {
    printf("%04x ", frame[i]);
  }
  printf("\n W8   W9   W10  W11  W12  W13\n");
  for(i = 11; i < 17; ++i) {
    printf("%04x ", frame[i]);
  }
  printf("\n");

  uint16_t *stk = frame + 5;
  printf("W14 = %04x, W15 = %04x\n", (uint16_t)fp, (uint16_t)stk);

  /* Dump the stack */
  while(stk >= (uint16_t *)&_SP_init) {
    printf("%04x -> %04x\n", (uint16_t)stk, *stk);
    --stk;
  }

  /* Walk backwards until FP = stack beginning */
  while(fp >= (uint16_t *)&_SP_init && fp < (uint16_t *)&_SPLIM_init) {
    fp = print_frame(fp);
  }

  /* Now dump all memory - skip the registers, they're toast! */
  uint16_t *addy = (uint16_t *)0x0020;
  uint16_t rcnt = 0;
  while(addy != (uint16_t *)0x4800) {
    if(!rcnt) {
      rcnt = 8;
      printf("\n%04x : ", (uint16_t)addy);
    } else {
      printf(" ");
    } printf("%04x", *addy);
    --rcnt;
    ++addy;
  }

  while(1) ;  /* Let the watchdog clean this mess up if enabled */
}
