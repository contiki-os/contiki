#ifndef NVM_H
#define NVM_H

#include "types.h"

typedef enum
{
	gNvmType_NoNvm_c,
	gNvmType_SST_c,
	gNvmType_ST_c,
	gNvmType_ATM_c,
	gNvmType_Max_c
} nvmType_t;


typedef enum
{
	gNvmErrNoError_c = 0,
	gNvmErrInvalidInterface_c,
	gNvmErrInvalidNvmType_c,
	gNvmErrInvalidPointer_c,
	gNvmErrWriteProtect_c,
	gNvmErrVerifyError_c,
	gNvmErrAddressSpaceOverflow_c,
	gNvmErrBlankCheckError_c,
	gNvmErrRestrictedArea_c,
	gNvmErrMaxError_c
} nvmErr_t;

typedef enum
{
	gNvmInternalInterface_c,
	gNvmExternalInterface_c,
	gNvmInterfaceMax_c
} nvmInterface_t;

/* ROM code seems to be THUMB */
/* need to be in a THUMB block before calling them */
extern nvmErr_t (*nvm_detect)(nvmInterface_t nvmInterface,nvmType_t* pNvmType);
extern nvmErr_t (*nvm_read)(nvmInterface_t nvmInterface , nvmType_t nvmType , void *pDest, uint32_t address, uint32_t numBytes);
extern nvmErr_t (*nvm_write)(nvmInterface_t nvmInterface, nvmType_t nvmType ,void *pSrc, uint32_t address, uint32_t numBytes);
/* sector bit field selects which sector to erase */
/* SST flash has 32 sectors 4096 bytes each */
/* bit 0 is the first sector, bit 31 is the last */
extern nvmErr_t (*nvm_erase)(nvmInterface_t nvmInterface, nvmType_t nvmType ,uint32_t sectorBitfield);
extern void(*nvm_setsvar)(uint32_t zero_for_awesome);
#endif //NVM_H
