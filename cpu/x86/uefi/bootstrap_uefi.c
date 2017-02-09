/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Uefi.h>
#include <Protocol/LoadedImage.h>

#define MAX_MEM_DESC 128

void start(void);

/* The section attribute below is copied from ATTR_BOOT_CODE in prot-domains.h.
 * prot-domains.h includes stdlib.h which defines NULL.  The UEFI headers also
 * define NULL, which induces a warning when the compiler detects the conflict.
 * To avoid that, we avoid including prot-domains.h from this file.
 */
EFI_STATUS EFIAPI __attribute__((section(".boot_text")))
uefi_start(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_MEMORY_DESCRIPTOR mem_map[MAX_MEM_DESC];
  UINTN mem_map_len = sizeof(mem_map);
  UINTN mem_map_key;
  UINTN mem_map_desc_sz;
  UINT32 mem_map_rev;

  EFI_STATUS res;

  res = SystemTable->BootServices->GetMemoryMap(&mem_map_len,
                                                mem_map,
                                                &mem_map_key,
                                                &mem_map_desc_sz,
                                                &mem_map_rev);
  if(res != EFI_SUCCESS) {
    return EFI_ABORTED;
  }

  res = SystemTable->BootServices->ExitBootServices(ImageHandle, mem_map_key);
  if(res != EFI_SUCCESS) {
    return EFI_ABORTED;
  }

  start();

  /* Should not be reachable: */
  return EFI_SUCCESS;
}
