/*
 * Copyright (c) 2015, Indian Institute of Science
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
 */

/**
 * \file
 *    MSP430x elfloader.
 * \author
 *    Sumankumar Panchal <suman@ece.iisc.ernet.in>
 *
 */

#include "contiki.h"
#include "loader/elfloader.h"
#include "loader/elfloader-arch.h"
#include "cfs/cfs.h"
#include "loader/symtab.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "dev/flash.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while(0)
#endif

#define EI_NIDENT 16

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
  elf32_word sh_name;     /* section name */
  elf32_word sh_type;     /* SHT_... */
  elf32_word sh_flags;          /* SHF_... */
  elf32_addr sh_addr;     /* virtual address */
  elf32_off sh_offset;          /* file offset */
  elf32_word sh_size;     /* section size */
  elf32_word sh_link;     /* misc info */
  elf32_word sh_info;     /* misc info */
  elf32_word sh_addralign;  /* memory alignment */
  elf32_word sh_entsize;  /* entry size if table */
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
  elf32_addr r_offset;            /* Location to be relocated. */
  elf32_word r_info;              /* Relocation type and symbol index. */
};

struct elf32_sym {
  elf32_word st_name;             /* String table index of name. */
  elf32_addr st_value;            /* Symbol value. */
  elf32_word st_size;             /* Size of associated object. */
  unsigned char st_info;          /* Type and binding information. */
  unsigned char st_other;         /* Reserved (not used). */
  elf32_half st_shndx;            /* Section index of symbol. */
};

#define ELF32_R_SYM(info)       ((info) >> 8)

struct relevant_section {
  unsigned char number;
  unsigned int offset;
  char *address;
};

char elfloader_unknown[30]; /* Name that caused link error. */

struct process *const *elfloader_autostart_processes;

static struct relevant_section bss, data, rodata, rodatafar, text, textfar;

static const unsigned char elf_magic_header[] =
{ 0x7f, 0x45, 0x4c, 0x46, /* 0x7f, 'E', 'L', 'F' */
  0x01,                    /* Only 32-bit objects. */
  0x01,                    /* Only LSB data. */
  0x01,                    /* Only ELF version 1. */
};

/* relocation type */
#define R_MSP430_NONE     0
#define R_MSP430_32         1
#define R_MSP430_10_PCREL         2
#define R_MSP430_16               3
#define R_MSP430_16_PCREL         4
#define R_MSP430_16_BYTE    5
#define R_MSP430_16_PCREL_BYTE    6
#define R_MSP430_2X_PCREL   7
#define R_MSP430_RL_PCREL   8
#define R_MSP430X_SRC_BYTE    9
#define R_MSP430X_SRC             10
#define R_MSP430X_DST_BYTE        11
#define R_MSP430X_DST             12
#define R_MSP430X_DST_2ND_BYTE    13
#define R_MSP430X_DST_2ND   14
#define R_MSP430X_PCREL_SRC_BYTE  15
#define R_MSP430X_PCREL_SRC   16
#define R_MSP430X_PCREL_DST_BYTE  17
#define R_MSP430X_PCREL_DST   18
#define R_MSP430X_PCREL_DST_2ND   19
#define R_MSP430X_PCREL_DST_2ND_BYTE  20
#define R_MSP430X_S_BYTE    21
#define R_MSP430X_S     22
#define R_MSP430X_D_BYTE    23
#define R_MSP430X_D     24
#define R_MSP430X_PCREL_D   25
#define R_MSP430X_INDXD     26
#define R_MSP430X_PCREL_INDXD   27
#define R_MSP430_10     28

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

static uint16_t datamemory_aligned[ELFLOADER_DATAMEMORY_SIZE / 2 + 1];
static uint8_t *datamemory = (uint8_t *)datamemory_aligned;
#if ELFLOADER_CONF_TEXT_IN_ROM
static const char textmemory[ELFLOADER_TEXTMEMORY_SIZE] = { 0 };
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
static char textmemory[ELFLOADER_TEXTMEMORY_SIZE];
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */

/*---------------------------------------------------------------------------*/
static void
seek_read(int fd, unsigned int offset, char *buf, int len)
{
  cfs_seek(fd, offset, CFS_SEEK_SET);
  cfs_read(fd, buf, len);
#if DEBUG
  {
    int i;
    PRINTF("seek_read: Read len %d from offset %d\n",
           len, offset);
    for(i = 0; i < len; ++i) {
      PRINTF("%02x ", buf[i]);
    }
    printf("\n");
  }
#endif /* DEBUG */
}
/*---------------------------------------------------------------------------*/
static void *
find_local_symbol(int fd, const char *symbol,
                  unsigned int symtab, unsigned short symtabsize,
                  unsigned int strtab)
{
  struct elf32_sym s;
  unsigned int a;
  char name[30];
  struct relevant_section *sect;

  for(a = symtab; a < symtab + symtabsize; a += sizeof(s)) {
    seek_read(fd, a, (char *)&s, sizeof(s));
    if(s.st_name != 0) {
      seek_read(fd, strtab + s.st_name, name, sizeof(name));
      if(strcmp(name, symbol) == 0) {
        if(s.st_shndx == bss.number) {
          sect = &bss;
        } else if(s.st_shndx == data.number) {
          sect = &data;
        } else if(s.st_shndx == rodatafar.number) {
          sect = &rodatafar;
        } else if(s.st_shndx == textfar.number) {
          sect = &textfar;
        } else {
          return NULL;
        }
        return &(sect->address[s.st_value]);
      }
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static int
relocate_section(int fd,
                 unsigned int section, unsigned short size,
                 unsigned int sectionaddr,
                 char *sectionbase,
                 unsigned int strs,
                 unsigned int strtab,
                 unsigned int symtab, unsigned short symtabsize,
                 unsigned char using_relas)
{
  /*
   * sectionbase added; runtime start address of current section
   */
  struct elf32_rela rela; /* Now used both for rel and rela data! */
  int rel_size = 0;
  struct elf32_sym s;
  unsigned int a;
  char name[30];
  char *addr;
  struct relevant_section *sect;

  /* determine correct relocation entry sizes */
  if(using_relas) {
    rel_size = sizeof(struct elf32_rela);
  } else {
    rel_size = sizeof(struct elf32_rel);
  }

  for(a = section; a < section + size; a += rel_size) {
    seek_read(fd, a, (char *)&rela, rel_size);
    seek_read(fd,
              symtab + sizeof(struct elf32_sym) * ELF32_R_SYM(rela.r_info),
              (char *)&s, sizeof(s));
    if(s.st_name != 0) {
      seek_read(fd, strtab + s.st_name, name, sizeof(name));
      PRINTF("name: %s\n", name);
      addr = (char *)symtab_lookup(name);
      if(addr == NULL) {
        PRINTF("name not found in global: %s\n", name);
        addr = find_local_symbol(fd, name, symtab, symtabsize, strtab);
        PRINTF("found address %p\n", addr);
      }
      if(addr == NULL) {
        if(s.st_shndx == bss.number) {
          sect = &bss;
        } else if(s.st_shndx == data.number) {
          sect = &data;
        } else if(s.st_shndx == rodatafar.number) {
          sect = &rodatafar;
        } else if(s.st_shndx == textfar.number) {
          sect = &textfar;
        } else {
          PRINTF("elfloader unknown name: '%30s'\n", name);
          memcpy(elfloader_unknown, name, sizeof(elfloader_unknown));
          elfloader_unknown[sizeof(elfloader_unknown) - 1] = 0;
          return ELFLOADER_SYMBOL_NOT_FOUND;
        }

        addr = sect->address;
      }
    } else {
      if(s.st_shndx == bss.number) {
        sect = &bss;
      } else if(s.st_shndx == data.number) {
        sect = &data;
      } else if(s.st_shndx == rodatafar.number) {
        sect = &rodatafar;
      } else if(s.st_shndx == textfar.number) {
        sect = &textfar;
      } else {
        return ELFLOADER_SEGMENT_NOT_FOUND;
      }

      addr = sect->address;
    }

    if(!using_relas) {
      /* copy addend to rela structure */
      seek_read(fd, sectionaddr + rela.r_offset, (char *)&rela.r_addend, 4);
    }

    elfloader_arch_relocate(fd, sectionaddr, sectionbase, &rela, addr);
  }

  return ELFLOADER_OK;
}
/*---------------------------------------------------------------------------*/
static void *
find_program_processes(int fd,
                       unsigned int symtab, unsigned short size,
                       unsigned int strtab)
{
  struct elf32_sym s;
  unsigned int a;
  char name[30];

  for(a = symtab; a < symtab + size; a += sizeof(s)) {
    seek_read(fd, a, (char *)&s, sizeof(s));

    if(s.st_name != 0) {
      seek_read(fd, strtab + s.st_name, name, sizeof(name));
      if(strcmp(name, "autostart_processes") == 0) {
        return &data.address[s.st_value];
      }
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
elfloader_init(void)
{
  elfloader_autostart_processes = NULL;
}
/*---------------------------------------------------------------------------*/
int
elfloader_load(int fd)
{
  struct elf32_ehdr ehdr;
  struct elf32_shdr shdr;
  struct elf32_shdr strtable;
  unsigned int strs;
  unsigned int shdrptr;
  unsigned int nameptr;
  char name[17];

  int i;
  unsigned short shdrnum, shdrsize;

  unsigned char using_relas = -1;
  unsigned short textoff = 0, textfaroff = 0, textsize, textfarsize,
                 textrelaoff = 0, textrelasize, textfarrelaoff = 0, textfarrelasize;
  unsigned short dataoff = 0, datasize, datarelaoff = 0, datarelasize;
  unsigned short rodataoff = 0, rodatafaroff = 0, rodatasize, rodatafarsize,
                 rodatarelaoff = 0, rodatarelasize, rodatafarrelaoff = 0,
                 rodatafarrelasize;
  unsigned short symtaboff = 0, symtabsize;
  unsigned short strtaboff = 0, strtabsize;
  unsigned short bsssize = 0;

  struct process **process;
  int ret;

  elfloader_unknown[0] = 0;

  /* The ELF header is located at the start of the buffer. */
  seek_read(fd, 0, (char *)&ehdr, sizeof(ehdr));

  /* Make sure that we have a correct and compatible ELF header. */
  if(memcmp(ehdr.e_ident, elf_magic_header, sizeof(elf_magic_header)) != 0) {
    PRINTF("ELF header problems\n");
    return ELFLOADER_BAD_ELF_HEADER;
  }

  /* Grab the section header. */
  shdrptr = ehdr.e_shoff;
  seek_read(fd, shdrptr, (char *)&shdr, sizeof(shdr));

  /* Get the size and number of entries of the section header. */
  shdrsize = ehdr.e_shentsize;
  shdrnum = ehdr.e_shnum;

  PRINTF("Section header: size %d num %d\n", shdrsize, shdrnum);

  /* The string table section: holds the names of the sections. */
  seek_read(fd, ehdr.e_shoff + shdrsize * ehdr.e_shstrndx,
            (char *)&strtable, sizeof(strtable));

  /*
   * Get a pointer to the actual table of strings. This table holds
   * the names of the sections, not the names of other symbols in the
   * file (these are in the sybtam section).
   */
  strs = strtable.sh_offset;

  PRINTF("Strtable offset %d\n", strs);

  /*
   *  Go through all sections and pick out the relevant ones. The
   *  ".text" and ".far.text" segments holds the actual code from
   *  the ELF file. The ".data" segment contains initialized data.
   *  The ".bss" segment holds the size of the unitialized data segment.
   *  The ".rodata" and ".far.rodata" segments contains constant data.
   *  The ".rela[a].text" and ".rela[a].far.text" segments contains
   *  relocation information for the contents of the ".text" and
   *  ".far.text" segments, respectively. The ".rela[a].rodata" and
   *  ".rela[a].far.rodata" segments contains relocation information
   *  for the contents of the ".rodata" and ".far.rodata" segments,
   *  respectively. The ".rela[a].data" segment contains relocation
   *  information for the contents of the ".data" segment. The ".symtab"
   *  segment contains the symbol table for this file. The ".strtab"
   *  segment points to the actual string names used by the symbol table.
   *
   *  In addition to grabbing pointers to the relevant sections, we
   *  also save the section number for resolving addresses in the
   *  relocator code.
   */

  /*
   * Initialize the segment sizes to zero so that we can check if
   * their sections was found in the file or not.
   */
  textsize = textfarsize = textrelasize = textfarrelasize =
          datasize = datarelasize = rodatasize = rodatafarsize =
                  rodatarelasize = rodatafarrelasize = symtabsize = strtabsize = 0;

  bss.number = data.number = rodata.number = rodatafar.number =
          text.number = textfar.number = -1;

  shdrptr = ehdr.e_shoff;
  for(i = 0; i < shdrnum; ++i) {
    seek_read(fd, shdrptr, (char *)&shdr, sizeof(shdr));

    /* The name of the section is contained in the strings table. */
    nameptr = strs + shdr.sh_name;
    seek_read(fd, nameptr, name, sizeof(name));
    PRINTF("Section shdrptr 0x%x, %d + %d type %d\n",
           shdrptr,
           strs, shdr.sh_name,
           (int)shdr.sh_type);
    /*
     * Match the name of the section with a predefined set of names
     * (.text, .far.text, .data, .bss, .rodata, .far.rodata, .rela.text, .rela.far.text,
     * .rela.data, .rela.rodata, .rela.far.rodata, .symtab, and .strtab).
     */

    if(shdr.sh_type == SHT_SYMTAB) {
      PRINTF("symtab\n");
      symtaboff = shdr.sh_offset;
      symtabsize = shdr.sh_size;
    } else if(shdr.sh_type == SHT_STRTAB) {
      PRINTF("strtab\n");
      strtaboff = shdr.sh_offset;
      strtabsize = shdr.sh_size;
    } else if(strncmp(name, ".text", 5) == 0) {
      textoff = shdr.sh_offset;
      textsize = shdr.sh_size;
      text.number = i;
      text.offset = textoff;
    } else if(strncmp(name, ".far.text", 9) == 0) {
      textfaroff = shdr.sh_offset;
      textfarsize = shdr.sh_size;
      textfar.number = i;
      textfar.offset = textfaroff;
    } else if(strncmp(name, ".rel.text", 9) == 0) {
      using_relas = 0;
      textrelaoff = shdr.sh_offset;
      textrelasize = shdr.sh_size;
    } else if(strncmp(name, ".rela.text", 10) == 0) {
      using_relas = 1;
      textrelaoff = shdr.sh_offset;
      textrelasize = shdr.sh_size;
    } else if(strncmp(name, ".rela.far.text", 14) == 0) {
      using_relas = 1;
      textfarrelaoff = shdr.sh_offset;
      textfarrelasize = shdr.sh_size;
    } else if(strncmp(name, ".data", 5) == 0) {
      dataoff = shdr.sh_offset;
      datasize = shdr.sh_size;
      data.number = i;
      data.offset = dataoff;
    } else if(strncmp(name, ".rodata", 7) == 0) {
      /* read-only data handled the same way as regular text section */
      rodataoff = shdr.sh_offset;
      rodatasize = shdr.sh_size;
      rodata.number = i;
      rodata.offset = rodataoff;
    } else if(strncmp(name, ".far.rodata", 11) == 0) {
      rodatafaroff = shdr.sh_offset;
      rodatafarsize = shdr.sh_size;
      rodatafar.number = i;
      rodatafar.offset = rodataoff;
    } else if(strncmp(name, ".rel.rodata", 11) == 0) {
      /* using elf32_rel instead of rela */
      using_relas = 0;
      rodatarelaoff = shdr.sh_offset;
      rodatarelasize = shdr.sh_size;
    } else if(strncmp(name, ".rela.rodata", 12) == 0) {
      using_relas = 1;
      rodatarelaoff = shdr.sh_offset;
      rodatarelasize = shdr.sh_size;
    } else if(strncmp(name, ".rela.far.rodata", 16) == 0) {
      using_relas = 1;
      rodatafarrelaoff = shdr.sh_offset;
      rodatafarrelasize = shdr.sh_size;
    } else if(strncmp(name, ".rel.data", 9) == 0) {
      /* using elf32_rel instead of rela */
      using_relas = 0;
      datarelaoff = shdr.sh_offset;
      datarelasize = shdr.sh_size;
    } else if(strncmp(name, ".rela.data", 10) == 0) {
      using_relas = 1;
      datarelaoff = shdr.sh_offset;
      datarelasize = shdr.sh_size;
    } else if(strncmp(name, ".bss", 4) == 0) {
      bsssize = shdr.sh_size;
      bss.number = i;
      bss.offset = 0;
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
  if(textfarsize == 0) {
    return ELFLOADER_NO_TEXT;
  }

  PRINTF("before allocate ram\n");
  bss.address = (char *)elfloader_arch_allocate_ram(bsssize + datasize);
  data.address = (char *)bss.address + bsssize;
  PRINTF("before allocate rom\n");
  textfar.address = (char *)elfloader_arch_allocate_rom(textfarsize + rodatafarsize);
  rodatafar.address = (char *)textfar.address + textfarsize;

  PRINTF("bss base address: bss.address = 0x%08x\n", bss.address);
  PRINTF("data base address: data.address = 0x%08x\n", data.address);
  PRINTF("textfar base address: textfar.address = 0x%08x\n", textfar.address);
  PRINTF("rodatafar base address: rodatafar.address = 0x%08x\n", rodatafar.address);

  /* If we have text segment relocations, we process them. */
  PRINTF("elfloader: relocate textfar\n");
  if(textfarrelasize > 0) {
    ret = relocate_section(fd,
                           textfarrelaoff, textfarrelasize,
                           textfaroff,
                           textfar.address,
                           strs,
                           strtaboff,
                           symtaboff, symtabsize, using_relas);
    if(ret != ELFLOADER_OK) {
      return ret;
    }
  }

  /* If we have any rodata segment relocations, we process them too. */
  PRINTF("elfloader: relocate rodata\n");
  if(rodatafarrelasize > 0) {
    ret = relocate_section(fd,
                           rodatafarrelaoff, rodatafarrelasize,
                           rodatafaroff,
                           rodatafar.address,
                           strs,
                           strtaboff,
                           symtaboff, symtabsize, using_relas);
    if(ret != ELFLOADER_OK) {
      PRINTF("elfloader: data failed\n");
      return ret;
    }
  }

  /* If we have any data segment relocations, we process them too. */
  PRINTF("elfloader: relocate data\n");
  if(datarelasize > 0) {
    ret = relocate_section(fd,
                           datarelaoff, datarelasize,
                           dataoff,
                           data.address,
                           strs,
                           strtaboff,
                           symtaboff, symtabsize, using_relas);
    if(ret != ELFLOADER_OK) {
      PRINTF("elfloader: data failed\n");
      return ret;
    }
  }

  /* Write text and rodata segment into flash and data segment into RAM. */
  elfloader_arch_write_rom(fd, textfaroff, textfarsize, textfar.address);
  elfloader_arch_write_rom(fd, rodatafaroff, rodatafarsize, rodatafar.address);

  memset(bss.address, 0, bsssize);
  seek_read(fd, dataoff, data.address, datasize);

  PRINTF("elfloader: autostart search\n");
  process = (struct process **)find_local_symbol(fd, "autostart_processes",
                                                 symtaboff, symtabsize, strtaboff);
  if(process != NULL) {
    PRINTF("elfloader: autostart found\n");
    elfloader_autostart_processes = process;
    return ELFLOADER_OK;
  } else {
    PRINTF("elfloader: no autostart\n");
    process = (struct process **)find_program_processes(fd, symtaboff,
                                                        symtabsize, strtaboff);
    if(process != NULL) {
      PRINTF("elfloader: FOUND PRG\n");
    }
    return ELFLOADER_NO_STARTPOINT;
  }
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
  return datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
#if ELFLOADER_CONF_TEXT_IN_ROM
  /* Return an 512-byte aligned pointer. */
  return (char *)
         ((unsigned long)&textmemory[0] & 0xfffffe00) +
         (((unsigned long)&textmemory[0] & 0x1ff) == 0 ? 0 : 0x200);
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
  return textmemory;
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
}
/*---------------------------------------------------------------------------*/
#define READSIZE 32
void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
#if ELFLOADER_CONF_TEXT_IN_ROM
  int i;
  unsigned int ptr;
  unsigned short *flashptr;

  flash_setup();

  flashptr = (unsigned short *)mem;

  cfs_seek(fd, textoff, CFS_SEEK_SET);
  for(ptr = 0; ptr < size; ptr += READSIZE) {

    /* Read data from file into RAM. */
    cfs_read(fd, (unsigned char *)datamemory, READSIZE);

    /* Clear flash page on 512 byte boundary. */
    if((((unsigned short)flashptr) & 0x01ff) == 0) {
      flash_clear(flashptr);
    }

    /*
     * Burn data from RAM into flash ROM. Flash is burned one 16-bit
     * word at a time, so we need to be careful when incrementing
     * pointers. The flashptr is already a short pointer, so
     * incrementing it by one will actually increment the address by
     * two.
     */
    for(i = 0; i < READSIZE / 2; ++i) {
      flash_write(flashptr, ((unsigned short *)datamemory)[i]);
      ++flashptr;
    }
  }

  flash_done();
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  cfs_read(fd, (unsigned char *)mem, size);
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
}
/*---------------------------------------------------------------------------*/
/* Relocate an MSP430X ELF section. */
void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
                        char *sectionaddr,
                        struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  unsigned char instr[2];

  type = ELF32_R_TYPE(rela->r_info);
  addr += rela->r_addend;

  switch(type) {
  case R_MSP430_16:
  case R_MSP430_16_PCREL:
  case R_MSP430_16_BYTE:
  case R_MSP430_16_PCREL_BYTE:
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430_32:
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_S:
  case R_MSP430X_S_BYTE:
    /* src(19:16) located at positions 11:8 of opcode */
    /* src(15:0) located just after opcode */
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_read(fd, instr, 2);
    instr[1] = (int)(instr[1]) & 0xf0 | (((long int)addr >> 8) & 0x0f00);
    instr[0] = (int)(instr[0]) & 0xff;
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, instr, 2);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_D:
  case R_MSP430X_PCREL_D:
  case R_MSP430X_D_BYTE:
    /* dst(19:16) located at positions 3:0 of opcode */
    /* dst(15:0) located just after opcode */
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_read(fd, instr, 2);
    instr[1] = (int)(instr[1]) & 0xff;
    instr[0] = (int)(instr[0]) & 0xf0 | (((long int)addr >> 16) & 0x000f);
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, instr, 2);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_PCREL_SRC_BYTE:
  case R_MSP430X_SRC_BYTE:
  case R_MSP430X_PCREL_SRC:
  case R_MSP430X_SRC:
    /* src(19:16) located at positions 10:7 of extension word */
    /* src(15:0) located just after opcode */
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_read(fd, instr, 2);
    /* 4 most-significant bits */
    instr[1] = (int)(instr[1]) & 0xf8 | (((long int)addr >> 9) & 0x0780);
    instr[0] = (int)(instr[0]) & 0x7f | (((long int)addr >> 9) & 0x0780);
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, instr, 2);
    /* 16 least-significant bits */
    cfs_seek(fd, sectionoffset + rela->r_offset + 0x04, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_DST_BYTE:
  case R_MSP430X_PCREL_DST_BYTE:
  case R_MSP430X_DST:
  case R_MSP430X_PCREL_DST:
    /* dst(19:16) located at positions 3:0 of extension word */
    /* dst(15:0) located just after opcode */
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_read(fd, instr, 2);
    instr[1] = (int)(instr[1]) & 0xff;
    instr[0] = (int)(instr[0]) & 0xf0 | (((long int)addr >> 16) & 0x000f);
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, instr, 2);
    cfs_seek(fd, sectionoffset + rela->r_offset + 0x04, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_DST_2ND:
  case R_MSP430X_PCREL_DST_2ND:
  case R_MSP430X_DST_2ND_BYTE:
  case R_MSP430X_PCREL_DST_2ND_BYTE:
    /* dst(19:16) located at positions 3:0 of extension word */
    /* dst(15:0) located after src(15:0) */
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_read(fd, instr, 2);
    instr[1] = (int)(instr[1]) & 0xff;
    instr[0] = (int)(instr[0]) & 0xf0 | (((long int)addr >> 16) & 0x000f);
    cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
    cfs_write(fd, instr, 2);
    cfs_seek(fd, sectionoffset + rela->r_offset + 0x06, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  case R_MSP430X_INDXD:
  case R_MSP430X_PCREL_INDXD:
    cfs_seek(fd, sectionoffset + rela->r_offset + 0x02, CFS_SEEK_SET);
    cfs_write(fd, (char *)&addr, 2);
    break;
  default:
    PRINTF("Unknown relocation type!\n");
    break;
  }
}
/*---------------------------------------------------------------------------*/
