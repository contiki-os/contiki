package se.sics.mspsim.core;

public class NoMemSegment implements Memory {

    private final MSP430Core core;

    NoMemSegment(MSP430Core core) {
        this.core = core;
    }

    @Override
    public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
        throw new EmulationException("Illegal read - out of bounds at $" + core.config.getAddressAsString(address));
//        core.printWarning(MSP430Constants.ADDRESS_OUT_OF_BOUNDS_READ, address);
//        return 0;
    }

    @Override
    public void write(int dstAddress, int dst, AccessMode mode) throws EmulationException {
        throw new EmulationException("Illegal write - out of bounds at $" + core.config.getAddressAsString(dstAddress));
        // core.printWarning(MSP430Constants.ADDRESS_OUT_OF_BOUNDS_WRITE, dstAddress);
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
