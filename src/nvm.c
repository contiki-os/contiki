#include "nvm.h"

volatile nvmErr_t (*nvm_detect)(nvmInterface_t nvmInterface,nvmType_t* pNvmType) = 0x00006cb9;
volatile nvmErr_t (*nvm_read)(nvmInterface_t nvmInterface , nvmType_t nvmType , void *pDest, uint32_t address, uint32_t numBytes) = 0x00006d69;
volatile nvmErr_t (*nvm_write)(nvmInterface_t nvmInterface, nvmType_t nvmType ,void *pSrc, uint32_t address, uint32_t numBytes) = 0x00006ec5;
volatile nvmErr_t (*nvm_erase)(nvmInterface_t nvmInterface, nvmType_t nvmType ,uint32_t sectorBitfield) = 0x00006e05;
volatile void(*nvm_setsvar)(uint32_t zero_for_awesome) = 0x00007085;
