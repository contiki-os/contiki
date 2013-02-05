#include <contiki.h>
#include "net/rime.h"

#include <stdio.h>
#include <fcntl.h>
#include <native-nvm.h>

#define NVM_BASE 0x1E000
#define NVM_SIZE 4096
static uint8_t nvm_mem[NVM_SIZE];
char const *  nvm_file = NULL;
const rimeaddr_t addr_ff = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

nvmErr_t nvm_detect(nvmInterface_t nvmInterface,nvmType_t* pNvmType)
{
	pNvmType = gNvmType_Max_c;
	return gNvmErrNoError_c;
}

nvmErr_t nvm_read(nvmInterface_t nvmInterface , nvmType_t nvmType , void *pDest, uint32_t address, uint32_t numBytes)
{
	printf("Opening nvm file '%s'\n", nvm_file);
	int s = open(nvm_file, O_RDONLY);
	if ( s > 0 )
	{
		read(s, nvm_mem, NVM_SIZE, 0);
		close(s);
	}
	else
	{
		printf("Warning: could not open nvm file\n");
		memset(nvm_mem, 0xff, NVM_SIZE);
	}
	memcpy(pDest, nvm_mem+(address-NVM_BASE), numBytes);
	return gNvmErrNoError_c;
}

nvmErr_t nvm_write(nvmInterface_t nvmInterface, nvmType_t nvmType ,void *pSrc, uint32_t address, uint32_t numBytes)
{
	memcpy(nvm_mem+(address-NVM_BASE), pSrc, numBytes);
	printf("Opening nvm file '%s'\n", nvm_file);
	int s = open(nvm_file, O_WRONLY | O_TRUNC | O_CREAT, 0644 );
	if ( s > 0 )
	{
		write(s, nvm_mem, NVM_SIZE, 0);
		close(s);
	} else {
		printf("Error: could not open nvm file\n");
	}
	return gNvmErrNoError_c;
}

nvmErr_t nvm_erase(nvmInterface_t nvmInterface, nvmType_t nvmType ,uint32_t sectorBitfield)
{
	memset(nvm_mem, 0xff, NVM_SIZE);
	return gNvmErrNoError_c;
}
