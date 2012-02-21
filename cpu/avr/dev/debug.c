
#include "compiler.h"
#include "delay.h"

#include <avr/pgmspace.h>

/*-----------------------------------------------------------------------------------*/
static void
delay(void)
{
  unsigned char i;
  for(i = 0; i < 1; ++i) {
    Delay_10ms(1);
  }
}
/*-----------------------------------------------------------------------------------*/
static char buffer[40];
static const char hextab[] PROGMEM = 
  {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
/*-----------------------------------------------------------------------------------*/
static void
print_buffer(unsigned char len)
{
  unsigned char i;

  for(i = 0; i < len; ++i) {
    delay();
    UDR0 = buffer[i];
  }
}
/*-----------------------------------------------------------------------------------*/
void
debug_print8(unsigned char v)
{
  /*  buffer[0] = v / 100 + '0';
  buffer[1] = (v / 10) % 10 + '0';
  buffer[2] = v % 10 + '0';
  buffer[3] = ' ';
  buffer[4] = PRG_RDB(hextab + (v >> 4));
  buffer[5] = PRG_RDB(hextab + (v & 0x0f));    
  buffer[6] = '\n';
  print_buffer(7);*/
}
/*-----------------------------------------------------------------------------------*/
void
debug_print16(unsigned short v)
{
  /*  buffer[0] = v / 10000 + '0';
  buffer[1] = (v / 1000) % 10 + '0';
  buffer[2] = (v / 100) % 10 + '0';
  buffer[3] = (v / 10) % 10 + '0';
  buffer[4] = v % 10 + '0';
  buffer[5] = ' ';
  buffer[6] = PRG_RDB(hextab + ((v & 0xf000) >> 12));
  buffer[7] = PRG_RDB(hextab + ((v & 0x0f00) >> 8));
  buffer[8] = PRG_RDB(hextab + ((v & 0xf0) >> 4));
  buffer[9] = PRG_RDB(hextab + (v & 0x0f));    
  buffer[10] = '\n';
  print_buffer(11);*/
}
/*-----------------------------------------------------------------------------------*/
void
debug_print(char *str)
{
  /*  unsigned char i;

  for(i = 0; PRG_RDB(str + i) != 0; ++i) {
    buffer[i] = PRG_RDB(str + i);
  }
  print_buffer(i);*/
}
/*-----------------------------------------------------------------------------------*/
