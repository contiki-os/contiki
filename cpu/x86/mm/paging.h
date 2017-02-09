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

#ifndef CPU_X86_MM_PAGING_H_
#define CPU_X86_MM_PAGING_H_

#include <stdint.h>

/**
 * Page table entry format for PAE mode page table.  See Intel Combined Manual,
 * Vol. 3, Section 4.4 for more details.
 */
typedef union pte {
  struct {
    uint64_t present         : 1;
    uint64_t writable        : 1;
    uint64_t user_accessible : 1;
    uint64_t pwt             : 1; /**< Specify write-through cache policy */
    uint64_t pcd             : 1; /**< Disable caching */
    uint64_t accessed        : 1;
    uint64_t dirty           : 1;
    uint64_t                 : 5;
    uint64_t addr            : 51;
    uint64_t exec_disable    : 1;
  };
  uint64_t raw;
} pte_t;

#define ENTRIES_PER_PDPT 4
#define ENTRIES_PER_PAGE_TABLE 512

typedef pte_t pdpt_t[ENTRIES_PER_PDPT];
typedef pte_t page_table_t[ENTRIES_PER_PAGE_TABLE];

#define MIN_PAGE_SIZE_SHAMT 12
#define MIN_PAGE_SIZE (1 << MIN_PAGE_SIZE_SHAMT)

#endif /* CPU_X86_MM_PAGING_H_ */
