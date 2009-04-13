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

#include <stdio.h>

static const int sector_len = 128 * 1024;

/* Initialize a MC1322x (ARM7)  */
struct mc1322x_state_s *mc1322x_init(void)
{
    struct mc1322x_state_s *s;
    int index;
    FILE *ram, *rom;
    ram_addr_t ramoff, romoff;

    s = (struct mc1322x_state_s *) qemu_mallocz(sizeof(struct mc1322x_state_s));

    s->env = cpu_init("mc1322x");
    if (!s->env) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    register_savevm("cpu", 0, ARM_CPU_SAVE_VERSION, cpu_save, cpu_load,
                    s->env);

    /* should probably allocate memory for all the cpu registers also */

    romoff = qemu_ram_alloc(MC1322X_ROMSIZE);
    cpu_register_physical_memory(MC1322X_ROMBASE, MC1322X_ROMSIZE,
                    romoff | IO_MEM_RAM);
    ramoff = qemu_ram_alloc(MC1322X_RAMSIZE);
    cpu_register_physical_memory(MC1322X_RAMBASE, MC1322X_RAMSIZE,
                    ramoff | IO_MEM_RAM);

    /* need to add a way to specify these images from the command line */

    if(rom = fopen("rom.img", "r")) {
	    fread(phys_ram_base,1,MC1322X_ROMSIZE,rom);
    }

    if(ram = fopen("ram.img", "r")) {
	    fprintf(stderr, "loading ram image\n");
	    fread(phys_ram_base+ramoff,1,MC1322X_RAMSIZE,ram);
    }

    s->env->regs[15] = 0x00400000;

    return s;
}

QEMUMachine mc1322x_machine = {
    "mc1322x",
    "mc1322x board",
    mc1322x_init,
};
