package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;

public class RAMSegment implements Memory {

    private final MSP430Core core;
    private final int memory[];

    public RAMSegment(MSP430Core core) {
        this.core = core;
        this.memory = core.memory;
    }

    @Override
    public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
        int val = memory[address] & 0xff;
        if (mode != AccessMode.BYTE) {
            val |= (memory[address + 1] << 8);
            if ((address & 1) != 0) {
                core.printWarning(WarningType.MISALIGNED_READ, address);
            }
            if (mode == AccessMode.WORD20) {
                /* will the read really get data from the full word? CHECK THIS */
                val |= (memory[address + 2] << 16) | (memory[address + 3] << 24);
                val &= 0xfffff;
            } else {
                val &= 0xffff;
            }
        }
        return val;
    }

    @Override
    public void write(int dstAddress, int dst, AccessMode mode) throws EmulationException {
        // assume RAM
        memory[dstAddress] = dst & 0xff;
        if (mode != AccessMode.BYTE) {
            memory[dstAddress + 1] = (dst >> 8) & 0xff;
            if ((dstAddress & 1) != 0) {
                core.printWarning(WarningType.MISALIGNED_WRITE, dstAddress);
            }
            if (mode != AccessMode.WORD) {
                memory[dstAddress + 2] = (dst >> 16) & 0xff; /* should be 0x0f ?? */
                memory[dstAddress + 3] = (dst >> 24) & 0xff; /* will be only zeroes*/
            }
        }
    }

    @Override
    public int get(int address, AccessMode mode) {
        return read(address, mode, AccessType.READ);
    }

    @Override
    public void set(int address, int data, AccessMode mode) {
        write(address, data, mode);
    }

}
