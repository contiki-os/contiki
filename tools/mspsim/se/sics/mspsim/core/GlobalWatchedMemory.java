package se.sics.mspsim.core;

public class GlobalWatchedMemory implements Memory {

    private final Memory wrappedMemory;
    private MemoryMonitor monitor;

    GlobalWatchedMemory(Memory wrapped) {
        this.wrappedMemory = wrapped;
    }

    public Memory getWatchedMemory() {
        return wrappedMemory;
    }

    public boolean hasGlobalMonitor() {
        return monitor != null;
    }

    public synchronized void addGlobalMonitor(MemoryMonitor mon) {
        monitor = MemoryMonitor.Proxy.INSTANCE.add(monitor, mon);
    }

    public synchronized void removeGlobalMonitor(MemoryMonitor mon) {
        monitor = MemoryMonitor.Proxy.INSTANCE.remove(monitor, mon);
    }

    @Override
    public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
        final MemoryMonitor mon = monitor;
        if (mon != null) {
            mon.notifyReadBefore(address, mode, type);

            int val = wrappedMemory.read(address, mode, type);

            mon.notifyReadAfter(address, mode, type);
            return val;
        }
        return wrappedMemory.read(address, mode, type);
    }

    @Override
    public void write(int dstAddress, int data, AccessMode mode) throws EmulationException {
        MemoryMonitor mon = monitor;
        if (mon != null) {
            mon.notifyWriteBefore(dstAddress, data, mode);
            wrappedMemory.write(dstAddress, data, mode);
            mon.notifyWriteAfter(dstAddress, data, mode);
        } else {
            wrappedMemory.write(dstAddress, data, mode);
        }
    }

    @Override
    public int get(int address, AccessMode mode) {
        return wrappedMemory.get(address, mode);
    }

    @Override
    public void set(int address, int data, AccessMode mode) {
        wrappedMemory.set(address, data, mode);
    }

}
