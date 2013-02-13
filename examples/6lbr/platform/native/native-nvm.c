#include <contiki.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "nvm-itf.h"
#include "nvm-config.h"
#include "native-nvm.h"

#define NVM_SIZE 0x100
static uint8_t nvm_mem[NVM_SIZE];
char const *nvm_file = NULL;

void
nvm_data_read(void)
{
  printf("Opening nvm file '%s'\n", nvm_file);
  int s = open(nvm_file, O_RDONLY);

  if(s > 0) {
    read(s, nvm_mem, NVM_SIZE);
    close(s);
  } else {
    printf("Warning: could not open nvm file\n");
    memset(nvm_mem, 0xff, NVM_SIZE);
  }
  memcpy((uint8_t *) & nvm_data, nvm_mem, sizeof(nvm_data));
}

void
nvm_data_write(void)
{
  memcpy(nvm_mem, (uint8_t *) & nvm_data, sizeof(nvm_data));
  printf("Opening nvm file '%s'\n", nvm_file);
  int s = open(nvm_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);

  if(s > 0) {
    write(s, nvm_mem, NVM_SIZE);
    close(s);
  } else {
    printf("Error: could not open nvm file\n");
  }
}
