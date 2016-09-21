package se.sics.mspsim.core;

import java.util.Arrays;
import se.sics.mspsim.util.Utils;

public class IOSegment implements Memory {

    private final MSP430Core core;
    private final IOUnit[] mem;
    private final IOUnit voidIO;

    IOSegment(MSP430Core core, int maxMemIO, IOUnit voidIO) {
        this.core = core;
        this.voidIO = voidIO;
        this.mem = new IOUnit[maxMemIO];
        Arrays.fill(mem, voidIO);
    }

    void setIORange(int address, int range, IOUnit io) {
//        System.out.println("IO: 0x" + Utils.hex(address, 4) + "-"
//                + Utils.hex(address + range - 1, 4) + ": " + io.id);
        for (int i = 0; i < range; i++) {
//            if (mem[address + i] != voidIO) {
//                System.err.println("Warning, IO unit already set: 0x"
//                        + Utils.hex(address + i, 4)
//                        + " (" + io.id + " => " + mem[address + i].id + ')');
//                throw new IllegalStateException("IO unit already set: 0x"
//                        + Utils.hex(address + i, 4)
//                        + " (" + io.id + " => " + mem[address + i].id + ')');
//            }
            mem[address + i] = io;
        }
    }

    @Override
    public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
        boolean word = mode != AccessMode.BYTE;
        // Only word reads at 0x1fe which is highest address...
        int val = mem[address].read(address, word, core.cycles);
        if (mode == AccessMode.WORD20) {
            val |= mem[address + 2].read(address, word, core.cycles) << 16;
        }
        return val;
    }

    @Override
    public void write(int dstAddress, int data, AccessMode mode) throws EmulationException {
        boolean word = mode != AccessMode.BYTE;

        if (!word) data &= 0xff;
        mem[dstAddress].write(dstAddress, data & 0xffff, word, core.cycles);
        if (mode == AccessMode.WORD20) {
            mem[dstAddress].write(dstAddress + 2, data >> 16, word, core.cycles);
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

    public String info() {
        StringBuilder sb = new StringBuilder();
        sb.append("IOSegment[0x0000-").append(Utils.hex(mem.length, 4)).append("]\n");

        int start = 0;
        for (int i = 1; i < mem.length; i++) {
            if (mem[i] != mem[i - 1]) {
                sb.append(" 0x").append(Utils.hex(start, 4))
                .append("-").append(Utils.hex(i - 1, 4))
                .append(": ").append(mem[i - 1].id).append(" (")
                .append(mem[i - 1].name).append(")\n");
                start = i;
            }
        }
        return sb.toString();
    }
}
