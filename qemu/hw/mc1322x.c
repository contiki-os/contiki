/* Freescale mc1322x support
 *
 * Copyright (c) 2009 Mariano Alvira
 * Written by Mariano Alvira <mar@devl.org>
 *
 * This code is licenced under the GPL.
 */

#include "hw.h"
#include "mc1322x.h"
#include "sysemu.h"
#include "boards.h"
#include "flash.h"
#include "block.h"

static const int sector_len = 128 * 1024;

/* Initialize a MC1322x (ARM7)  */
struct mc1322x_state_s *mc1322x_init(void)
{
    struct mc1322x_state_s *s;
    int index;

    s = (struct mc1322x_state_s *) qemu_mallocz(sizeof(struct mc1322x_state_s));

    s->env = cpu_init("mc1322x");
    if (!s->env) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    register_savevm("cpu", 0, ARM_CPU_SAVE_VERSION, cpu_save, cpu_load,
                    s->env);

    /* SDRAM & Internal Memory Storage */
    /* should probably allocate memory for all the cpu registers... I think that is where the emulation might be bombing */
    cpu_register_physical_memory(MC1322X_ROMBASE, MC1322X_ROMSIZE,
                    qemu_ram_alloc(MC1322X_ROMSIZE) | IO_MEM_RAM);
    cpu_register_physical_memory(MC1322X_RAMBASE, MC1322X_RAMSIZE,
                    qemu_ram_alloc(MC1322X_RAMSIZE) | IO_MEM_RAM);

    index = drive_get_index(IF_MTD, 0, 0);
    if(0<bdrv_read(drives_table[index].bdrv,0,phys_ram_base+MC1322X_ROMBASE,MC1322X_ROMSIZE/512)) {
	    fprintf(stderr, "qemu: Error registering rom memory.\n");
    }

    index = drive_get_index(IF_PFLASH, 0, 0);
    if (!pflash_cfi01_register(0x00400000, qemu_ram_alloc(MC1322X_RAMSIZE),
            drives_table[index].bdrv, sector_len, MC1322X_RAMSIZE / sector_len,
            2, 0, 0, 0, 0)) {
        fprintf(stderr, "qemu: Error registering flash memory.\n");
        exit(1);
    }

    s->env->regs[15] = 0x00400000;

    return s;
}

QEMUMachine mc1322x_machine = {
    "mc1322x",
    "mc1322x board",
    mc1322x_init,
};
