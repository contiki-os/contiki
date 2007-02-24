#include <malloc.h>
#include <loader/elfloader-arch-otf.h>

#if 0
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

/* Supported relocations */

#define R_ARM_ABS32	2
#define R_ARM_THM_CALL	10

/* Adapted from elfloader-avr.c */

void
elfloader_arch_relocate(int input_fd,
			struct elfloader_output *output,
			unsigned int sectionoffset,
			char *sectionaddr,
                        struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  unsigned char instr[4];

  type = ELF32_R_TYPE(rela->r_info);

  cfs_seek(input_fd, sectionoffset + rela->r_offset);

/*   PRINTF("elfloader_arch_relocate: type %d\n", type); */
/*   PRINTF("Addr: %p, Addend: %ld\n",   addr, rela->r_addend); */
  switch(type) {
  case R_ARM_ABS32:
    {
      int32_t addend;
      cfs_read(input_fd, (char*)&addend, 4);
      addr += addend;
      elfloader_output_write_segment(output,(char*) &addr, 4);
      PRINTF("%p: addr: %p\n", sectionaddr +rela->r_offset,
	     addr);
    }
    break;
  case R_ARM_THM_CALL:
    {
      int32_t offset;
      char *base;
      cfs_read(input_fd, (char*)instr, 4);
      /* Ignore the addend since it will be zero for calls to symbols,
	 and I can't think of a case when doing a relative call to
	 a non-symbol position */
      base = sectionaddr + (rela->r_offset + 4);
      if (((*(uint16_t*)(instr+2)) & 0x1800) == 0x0800) {
	addr = (char*)((((uint32_t)addr) & 0xfffffffd)
		       | (((uint32_t)base) & 0x00000002));
      }
      offset = addr -  (sectionaddr + (rela->r_offset + 4));
      if (offset < -(1<<22) || offset >= (1<<22)) {
	PRINTF("elfloader-arm.c: offset %d too large for relative call\n",
	       (int)offset);
      }
    /*   PRINTF("%p: %04x %04x  offset: %d addr: %p\n", sectionaddr +rela->r_offset, *(uint16_t*)instr, *(uint16_t*)(instr+2), (int)offset, addr);  */
      *(uint16_t*)instr = (*(uint16_t*)instr & 0xf800) | ((offset>>12)&0x07ff);
      *(uint16_t*)(instr+2) = ((*(uint16_t*)(instr+2) & 0xf800)
			       | ((offset>>1)&0x07ff));
      elfloader_output_write_segment(output, (char*)instr, 4);
  /*     PRINTF("cfs_write: %04x %04x\n",*(uint16_t*)instr, *(uint16_t*)(instr+2));  */
    }
    break;
    
  default:
    PRINTF("elfloader-arm.c: unsupported relocation type %d\n", type);
    break;
  }
}
