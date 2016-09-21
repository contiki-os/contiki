package se.sics.mspsim.core;

import java.util.ArrayList;

import se.sics.mspsim.util.Utils;

public abstract class MSP430Config {
    
    public static class TimerConfig {
        public final int ccr0Vector;
        public final int ccrXVector;
        public final int ccrCount;
        public final int offset;
        public final String name;
        public final int[] srcMap;
        public final int timerIVAddr;
        
        public TimerConfig(int ccr0Vec, int ccrXVec, int ccrCount, int offset,
                int[] srcMap, String name, int tiv) {
            ccr0Vector = ccr0Vec;
            ccrXVector = ccrXVec;
            this.ccrCount = ccrCount;
            this.name = name;
            this.offset = offset;
            this.srcMap = srcMap;
            this.timerIVAddr = tiv;
        }
    }

    public static class UARTConfig {
        private static final int USCI_2 = 1;
        private static final int USCI_5 = 2;
        
        public final int txVector;
        public final int rxVector;
        public final int offset;
        public final String name;
        public final int txBit;
        public final int rxBit;
        public final int sfrAddr;
        public final boolean usciA;
        public final int type;
        
        public UARTConfig(String name, int vector, int offset) {
            type = USCI_5;
            txVector = rxVector = vector;
            this.txBit = this.rxBit = 0;
            this.offset = offset;
            this.name = name;
            this.usciA = false;
            this.sfrAddr = 0;
        }

        public UARTConfig(int txVector, int rxVector, int txBit, int rxBit, int sftAddr, int offset,
                    String name, boolean usciA) {
            this.type = USCI_2;
            this.txVector = txVector;
            this.rxVector = rxVector;
            this.txBit = txBit;
            this.rxBit = rxBit;
            this.offset = offset;
            this.name = name;
            this.usciA = usciA;
            this.sfrAddr = sftAddr;
        }
    }

    public UARTConfig[] uartConfig;
    
    /* default for the 149/1611 */
    public TimerConfig[] timerConfig = {
            new TimerConfig(6, 5, 3, 0x160, Timer.TIMER_Ax149, "TimerA", Timer.TAIV),
            new TimerConfig(13, 12, 7, 0x180, Timer.TIMER_Bx149, "TimerB", Timer.TBIV)
    };
    
    /* Memory configuration */
    public int maxMemIO = 0x200;
    public int maxMem = 64*1024;
    public int maxInterruptVector = 15;
    
    public int mainFlashStart = 0x0000;
    public int mainFlashSize = 48 * 1024;

    public int infoMemStart = 0x0000;
    public int infoMemSize = 2 * 128;

    public int ramStart = 0x2000;
    public int ramSize = 2048;

    public int ramMirrorStart = 0x0;
    public int ramMirrorSize = 0x0;
    public int ramMirrorAddress = 0x0;
    
    public int flashControllerOffset = 0x128;
    
    public boolean MSP430XArch = false;

    public int sfrOffset = 0;

    public int watchdogOffset = 0x120;
    
    public abstract int setup(MSP430Core cpu, ArrayList<IOUnit> ioUnits);

    
    public boolean isRAM(int address) {
        return address >= ramStart && address < ramStart + ramSize;
    }

    public boolean isRAMMirror(int address) {
        return ramMirrorSize > 0 && address >= ramMirrorStart && address < ramMirrorStart + ramMirrorSize;
    }

    public boolean isFlash(int address) {
        return address >= mainFlashStart && address < mainFlashStart + mainFlashSize;
    }

    public boolean isInfoMem(int address) {
        return address >= infoMemStart && address < infoMemStart + infoMemSize;
    }

    public boolean isIO(int address) {
        return address < maxMemIO;
    }
    
    public String getAddressAsString(int addr) {
        return Utils.hex16(addr);
    }


    public void infoMemConfig(int start, int size) {
        infoMemStart = start;
        infoMemSize = size;
    }
    
    public void mainFlashConfig(int start, int size) {
        mainFlashStart = start;
        mainFlashSize = size;
        if (maxMem < start + size) {
            maxMem = start + size;
        }
    }
    
    /* ignored for now */
    public void ramConfig(int start, int size) {
       ramStart = start;
       ramSize = size;
    }

    public void ramMirrorConfig(int start, int size, int address) {
        ramMirrorStart = start;
        ramMirrorSize = size;
        ramMirrorAddress = address;
    }

    public void ioMemSize(int size) {
        maxMemIO = size;
    }

    public ClockSystem createClockSystem(MSP430Core cpu, int[] memory, Timer[] timers) {
        return new BasicClockModule(cpu, memory, 0, timers);
    }

}
