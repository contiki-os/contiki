#include "contiki.h"
#include "contiki-lib.h"

#include "cetic-6lbr.h"
#include "nvm-config.h"
#include "nvm-itf.h"

#include <mc1322x.h>
#include "config.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define CETIC_6LBR_NVM_ADDRESS (MC1322X_CONFIG_PAGE + 0x100)

void
nvm_data_read(void)
{
  nvmType_t type = 0;
  nvmErr_t err;

  PRINTF("Reading 6LBR NVM\n");
  err = nvm_detect(gNvmInternalInterface_c, &type);
  err =
    nvm_read(gNvmInternalInterface_c, type, (uint8_t *) & nvm_data,
             CETIC_6LBR_NVM_ADDRESS, sizeof(nvm_data_t));
  PRINTF("err : %d\n", err);
}

void
nvm_data_write(void)
{
  nvmType_t type = 0;
  nvmErr_t err;

  PRINTF("Flashing 6LBR NVM\n");
  mc1322x_config_save(&mc1322x_config);
  err = nvm_detect(gNvmInternalInterface_c, &type);
  err =
    nvm_write(gNvmInternalInterface_c, type, (uint8_t *) & nvm_data,
              CETIC_6LBR_NVM_ADDRESS, sizeof(nvm_data_t));
  PRINTF("err : %d\n", err);
}
