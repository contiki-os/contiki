/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: elfloader.c,v 1.1 2006/06/17 22:41:18 adamdunkels Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/unistd.h>

#include "contiki.h"

#include "loader/elfloader.h"
#include "loader/symtab.h"

#include "lib/malloc.h"
#include "dev/rom.h"
#include "dev/xmem.h"

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define IMAX(a, b) (((a) > (b)) ? (a) : (b))

static char *datamemory;

#define TEXTADDRESS \
    (((uintptr_t)&_etext + (uintptr_t)&_edata - (uintptr_t)&__data_start \
     + ROM_ERASE_UNIT_SIZE) & ~(ROM_ERASE_UNIT_SIZE - 1))

#define EI_NIDENT 16

typedef unsigned long  elf32_word;
typedef   signed long  elf32_sword;
typedef unsigned short elf32_half;
typedef unsigned long  elf32_off;
typedef unsigned long  elf32_addr;


struct elf32_ehdr {
  unsigned char e_ident[EI_NIDENT];    /* ident bytes */
  elf32_half e_type;                   /* file type */ 
  elf32_half e_machine;                /* target machine */
  elf32_word e_version;                /* file version */
  elf32_addr e_entry;                  /* start address */
  elf32_off e_phoff;                   /* phdr file offset */
  elf32_off e_shoff;                   /* shdr file offset */
  elf32_word e_flags;                  /* file flags */
  elf32_half e_ehsize;                 /* sizeof ehdr */
  elf32_half e_phentsize;              /* sizeof phdr */
  elf32_half e_phnum;                  /* number phdrs */
  elf32_half e_shentsize;              /* sizeof shdr */
  elf32_half e_shnum;                  /* number shdrs */
  elf32_half e_shstrndx;               /* shdr string index */
};

/* Values for e_type. */
#define ET_NONE         0       /* Unknown type. */
#define ET_REL          1       /* Relocatable. */
#define ET_EXEC         2       /* Executable. */
#define ET_DYN          3       /* Shared object. */
#define ET_CORE         4       /* Core file. */

struct elf32_shdr {
  elf32_word sh_name; 		/* section name */
  elf32_word sh_type; 		/* SHT_... */
  elf32_word sh_flags; 	        /* SHF_... */
  elf32_addr sh_addr; 		/* virtual address */
  elf32_off sh_offset; 	        /* file offset */
  elf32_word sh_size; 		/* section size */
  elf32_word sh_link; 		/* misc info */
  elf32_word sh_info; 		/* misc info */
  elf32_word sh_addralign; 	/* memory alignment */
  elf32_word sh_entsize; 	/* entry size if table */
};

/* sh_type */
#define SHT_NULL        0               /* inactive */
#define SHT_PROGBITS    1               /* program defined information */
#define SHT_SYMTAB      2               /* symbol table section */
#define SHT_STRTAB      3               /* string table section */
#define SHT_RELA        4               /* relocation section with addends*/
#define SHT_HASH        5               /* symbol hash table section */
#define SHT_DYNAMIC     6               /* dynamic section */
#define SHT_NOTE        7               /* note section */
#define SHT_NOBITS      8               /* no space section */
#define SHT_REL         9               /* relation section without addends */
#define SHT_SHLIB       10              /* reserved - purpose unknown */
#define SHT_DYNSYM      11              /* dynamic symbol table section */
#define SHT_LOPROC      0x70000000      /* reserved range for processor */
#define SHT_HIPROC      0x7fffffff      /* specific section header types */
#define SHT_LOUSER      0x80000000      /* reserved range for application */
#define SHT_HIUSER      0xffffffff      /* specific indexes */

struct elf32_rel {
  elf32_addr      r_offset;       /* Location to be relocated. */
  elf32_word      r_info;         /* Relocation type and symbol index. */
};

struct elf32_rela {
  elf32_addr      r_offset;       /* Location to be relocated. */
  elf32_word      r_info;         /* Relocation type and symbol index. */
  elf32_sword     r_addend;       /* Addend. */
};

struct elf32_sym {
  elf32_word      st_name;        /* String table index of name. */
  elf32_addr      st_value;       /* Symbol value. */
  elf32_word      st_size;        /* Size of associated object. */
  unsigned char   st_info;        /* Type and binding information. */
  unsigned char   st_other;       /* Reserved (not used). */
  elf32_half      st_shndx;       /* Section index of symbol. */
};

#define ELF32_R_SYM(info)       ((info) >> 8)
#define ELF32_R_TYPE(info)      ((unsigned char)(info))

struct relevant_section {
  unsigned char number;
  //off_t offset;
  char *address;
};

char elfloader_unknown[30];	/* Name that caused link error. */

static struct relevant_section bss, data, text;

/*static unsigned char bss_sectionno, data_sectionno, text_sectionno;*/

static const unsigned char elf_magic_header[] =
  {0x7f, 0x45, 0x4c, 0x46,  /* 0x7f, 'E', 'L', 'F' */
   0x01,                    /* Only 32-bit objects. */
   0x01,                    /* Only LSB data. */
   0x01,                    /* Only ELF version 1. */
  };

/*---------------------------------------------------------------------------*/
static int
relocate_section(off_t section, unsigned short size,
		 off_t sectionaddr,
		 off_t strs,
		 off_t strtab,
		 off_t symtab,
		 char *mem)
{
  struct elf32_rela rela;
  struct elf32_sym s;
  off_t a;
  char name[30];
  char *addr;
  struct relevant_section *sect;

  
  for(a = section; a < section + size; a += sizeof(struct elf32_rela)) {
    xmem_pread(&rela, sizeof(rela), a);
    xmem_pread(&s, sizeof(s),
	       symtab + sizeof(struct elf32_sym) * ELF32_R_SYM(rela.r_info));
    if(s.st_name == 0) {	/* Local symbol (static) */
      if(s.st_shndx == bss.number) {	
	sect = &bss;
      } else if(s.st_shndx == data.number) {
	sect = &data;
      } else if(s.st_shndx == text.number) {
	sect = &text;
      } else {
	return ELFLOADER_SEGMENT_NOT_FOUND;
      }
      addr = sect->address + rela.r_addend;
    } else {
      xmem_pread(name, sizeof(name), strtab + s.st_name);
      addr = (char *)symtab_lookup(name);
      if(addr != NULL) {	/* Global symbol */
	addr += rela.r_addend;
      } else {			/* Exported symbol */
	if(s.st_shndx == bss.number) {	
	  sect = &bss;
	} else if(s.st_shndx == data.number) {
	  sect = &data;
	} else if(s.st_shndx == text.number) {
	  sect = &text;
	} else {
	  PRINTF("elfloader unkown name: '%30s'\n", name);
	  memcpy(elfloader_unknown, name, sizeof(elfloader_unknown));
	  elfloader_unknown[sizeof(elfloader_unknown) - 1] = 0;
	  return ELFLOADER_SYMBOL_NOT_FOUND;
	}
	addr = sect->address + s.st_value + rela.r_addend;
      }
    }
    memcpy(&mem[rela.r_offset], &addr, 2); /* Write reloc */
  }
  return ELFLOADER_OK;
}
/*---------------------------------------------------------------------------*/
typedef void (*PFV)(void);

struct process *elfloader_loaded_process;
void (*elfloader_fini)(void);

static PFV
find_init_and_fini(off_t symtab, unsigned short size, off_t strtab)
{
  struct elf32_sym s;
  off_t a;
  char name[30];
  void (*elfloader_init)(void);

  elfloader_init = NULL;

  for(a = symtab; a < symtab + size; a += sizeof(s)) {
    xmem_pread(&s, sizeof(s), a);

    if(s.st_name != 0) {
      xmem_pread(name, sizeof(name), strtab + s.st_name);
      if(strcmp(name, "process_load") == 0) {
	elfloader_loaded_process =
	  *(struct process **)&data.address[s.st_value];
      } else if (strcmp(name, "_init") == 0) {
	/* XXX Check segment == text. */
	elfloader_init = (PFV)&text.address[s.st_value];
      } else if (strcmp(name, "_fini") == 0) {
	/* XXX Check segment == text. */
	elfloader_fini = (PFV)&text.address[s.st_value];
      }
    }
  }
  return elfloader_init;
}
/*---------------------------------------------------------------------------*/
int
elfloader_load(off_t eepromaddr)
{
  struct elf32_ehdr ehdr;
  struct elf32_shdr shdr;
  struct elf32_shdr strtable;
  off_t strs;
  off_t shdrptr; 
  off_t nameptr;
  char name[12];
  
  int i;
  unsigned short shdrnum, shdrsize;

  unsigned short textoff = 0, textsize, textrelaoff = 0, textrelasize;
  unsigned short dataoff = 0, datasize, datarelaoff = 0, datarelasize;
  unsigned short symtaboff = 0, symtabsize;
  unsigned short strtaboff = 0, strtabsize;
  unsigned short bsssize = 0;

  int ret;

  void (*elfloader_init)(void);
  elfloader_unknown[0] = 0;

  /* The ELF header is located at the start of the buffer. */
  xmem_pread(&ehdr, sizeof(ehdr), eepromaddr);

  /* Make sure that we have a correct and compatible ELF header. */
  if(memcmp(ehdr.e_ident, elf_magic_header, sizeof(elf_magic_header)) != 0) {
    return ELFLOADER_BAD_ELF_HEADER;
  }

  /* Grab the section header. */
  shdrptr = eepromaddr + ehdr.e_shoff;
  xmem_pread(&shdr, sizeof(shdr), shdrptr);
  
  /* Get the size and number of entries of the section header. */
  shdrsize = ehdr.e_shentsize;
  shdrnum = ehdr.e_shnum;

  /* The string table section: holds the names of the sections. */
  xmem_pread(&strtable, sizeof(strtable),
	     eepromaddr + ehdr.e_shoff + shdrsize * ehdr.e_shstrndx);

  /* Get a pointer to the actual table of strings. This table holds
     the names of the sections, not the names of other symbols in the
     file (these are in the sybtam section). */
  strs = eepromaddr + strtable.sh_offset;

  /* Go through all sections and pick out the relevant ones. The
     ".text" segment holds the actual code from the ELF file, the
     ".data" segment contains initialized data, the ".bss" segment
     holds the size of the unitialized data segment. The ".rela.text"
     and ".rela.data" segments contains relocation information for the
     contents of the ".text" and ".data" segments, respectively. The
     ".symtab" segment contains the symbol table for this file. The
     ".strtab" segment points to the actual string names used by the
     symbol table.

     In addition to grabbing pointers to the relevant sections, we
     also save the section number for resolving addresses in the
     relocator code.
  */


  /* Initialize the segment sizes to zero so that we can check if
     their sections was found in the file or not. */
  textsize = textrelasize = datasize = datarelasize =
    symtabsize = strtabsize = 0;

  bss.number = data.number = text.number = 0;
		
  shdrptr = eepromaddr + ehdr.e_shoff; 
  for(i = 0; i < shdrnum; ++i) {

    xmem_pread(&shdr, sizeof(shdr), shdrptr);
    
    /* The name of the section is contained in the strings table. */
    nameptr = strs + shdr.sh_name;
    xmem_pread(name, sizeof(name), nameptr);

    /* Match the name of the section with a predefined set of names
       (.text, .data, .bss, .rela.text, .rela.data, .symtab, and
       .strtab). */
    
    if(strncmp(name, ".text", 5) == 0) {
      textoff = shdr.sh_offset;
      textsize = shdr.sh_size;
      text.number = i;
      //text.offset = eepromaddr + textoff;
    } else if(strncmp(name, ".rela.text", 10) == 0) {
      textrelaoff = shdr.sh_offset;
      textrelasize = shdr.sh_size;
    } else if(strncmp(name, ".data", 5) == 0) {
      dataoff = shdr.sh_offset;
      datasize = shdr.sh_size;
      data.number = i;
      //data.offset = eepromaddr + dataoff;
    } else if(strncmp(name, ".rela.data", 10) == 0) {
      datarelaoff = shdr.sh_offset;
      datarelasize = shdr.sh_size;
    } else if(strncmp(name, ".symtab", 7) == 0) {
      symtaboff = shdr.sh_offset;
      symtabsize = shdr.sh_size;
    } else if(strncmp(name, ".strtab", 7) == 0) {
      strtaboff = shdr.sh_offset;
      strtabsize = shdr.sh_size;
    } else if(strncmp(name, ".bss", 4) == 0) {
      bsssize = shdr.sh_size;
      bss.number = i;
      //bss.offset = 0;
    } else {
      PRINTF("elfloader: unknown '%12s' %d\n", name, shdrsize);
    }

    /* Move on to the next section header. */
    shdrptr += shdrsize;
  }


  if(symtabsize == 0) {
    return ELFLOADER_NO_SYMTAB;
  }
  if(strtabsize == 0) {
    return ELFLOADER_NO_STRTAB;
  }
  if(textsize == 0) {
    return ELFLOADER_NO_TEXT;
  }

  if(datamemory != NULL) {
    free(datamemory);
  }

  /* We are making semi-permanent allocations, first compact heap! */
  malloc_compact();
  datamemory = malloc(IMAX(textsize, datasize + bsssize));
  if(datamemory == NULL) {
    return ELFLOADER_DATA_TO_LARGE; /* XXX or text to large */
  }

  bss.address = (char *)datamemory;
  data.address = (char *)datamemory + bsssize;
  text.address = (char *)TEXTADDRESS;

  PRINTF("elfloader: copy text segment to RAM %p %p\n",
	 datamemory, datamemory + textsize);
  xmem_pread(datamemory, textsize, eepromaddr + textoff); 
  if(textrelasize > 0) {
    PRINTF("elfloader: relocate text in RAM\n");
    ret = relocate_section(eepromaddr + textrelaoff, textrelasize,
			   eepromaddr + textoff,
			   strs,
			   eepromaddr + strtaboff,
			   eepromaddr + symtaboff,
			   datamemory);
    if(ret != ELFLOADER_OK) {
      return ret;
    }
  }
  PRINTF("elfloader: copy text segment to ROM %p %p\n",
	 text.address, text.address + textsize);
  rom_erase((textsize + ROM_ERASE_UNIT_SIZE) & ~(ROM_ERASE_UNIT_SIZE - 1),
	    (uintptr_t)text.address);
  rom_pwrite(datamemory, textsize, (uintptr_t)text.address);

  PRINTF("elfloader: copy data segment to RAM %p %p\n",
	 data.address, data.address + datasize);
  xmem_pread(data.address, datasize, eepromaddr + dataoff); 
  if(datarelasize > 0) {
    PRINTF("elfloader: relocate data segment\n");
    ret = relocate_section(eepromaddr + datarelaoff, datarelasize,
			   eepromaddr + dataoff,
			   strs,
			   eepromaddr + strtaboff,
			   eepromaddr + symtaboff,
			   data.address);
    if(ret != ELFLOADER_OK) {
      return ret;
    }
  }

  PRINTF("elfloader: zero bss %p %p\n", bss.address, bss.address + bsssize);
  memset(bss.address, 0, bsssize);
  datamemory = realloc(datamemory, datasize + bsssize);
  if(datamemory != bss.address) {
    free(bss.address);
    return ELFLOADER_BSS_TO_LARGE; /* XXX realloc moved data */
  }

  /* Find _init, _fini, and loaded_process. */
  elfloader_loaded_process = NULL;
  elfloader_fini = NULL;
  elfloader_init = find_init_and_fini(eepromaddr + symtaboff,
				      symtabsize,
				      eepromaddr + strtaboff);

  if(elfloader_init != NULL) {
    PRINTF("init=%p fini=%p\n", elfloader_init, elfloader_fini);
    (*elfloader_init)();
    elfloader_loaded_process = NULL;
    return ELFLOADER_OK;
  }

  if(elfloader_loaded_process != NULL) {
    PRINTF("elfloader: launch program\n");
    process_start(elfloader_loaded_process, NULL);
    elfloader_fini = NULL;
    return ELFLOADER_OK;
  } else {
    return ELFLOADER_NO_STARTPOINT;
  }
}
/*---------------------------------------------------------------------------*/
void
elfloader_unload(void)
{
  if(elfloader_fini != NULL) {
    (*elfloader_fini)();
    elfloader_fini = NULL;
  } else if(elfloader_loaded_process != NULL) {
    process_exit(elfloader_loaded_process);
    elfloader_loaded_process = NULL;
  }
  if(datamemory != NULL) {
    free(datamemory);
    datamemory = NULL;
  }
}
