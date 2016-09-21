package se.sics.mspsim.core;

public class WatchedMemory implements Memory {

    private final int start;
    private final Memory wrappedMemory;
    private final MemoryMonitor watchPoints[] = new MemoryMonitor[Memory.SEGMENT_SIZE];

    WatchedMemory(int start, Memory wrapped) {
        this.start = start;
        this.wrappedMemory = wrapped;
    }

    @Override
    public int read(int address, AccessMode mode, AccessType type) throws EmulationException {
        final int a = address - start;
        int val;
        MemoryMonitor mon = watchPoints[a];
        if (mon != null) {
            mon.notifyReadBefore(address, mode, type);
            val = wrappedMemory.read(address, mode, type);
            mon.notifyReadAfter(address, mode, type);
        } else {
            val = wrappedMemory.read(address, mode, type);
        }
        return val;
    }

    @Override
    public void write(int dstAddress, int dst, AccessMode mode) throws EmulationException {
        final int a = dstAddress - start;
        final MemoryMonitor mon = watchPoints[a];
        if (mon != null) {
            mon.notifyWriteBefore(dstAddress, dst, mode);
            wrappedMemory.write(dstAddress, dst, mode);
            mon.notifyWriteAfter(dstAddress, dst, mode);
        } else {
            wrappedMemory.write(dstAddress, dst, mode);
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

    public boolean hasWatchPoint(int address) {
        MemoryMonitor mon = watchPoints[address - start];
        return mon != null;
    }

    public synchronized void addWatchPoint(int address, MemoryMonitor mon) {
        final int a = address - start;
        watchPoints[a] = MemoryMonitor.Proxy.INSTANCE.add(watchPoints[a], mon);
    }

    public synchronized void removeWatchPoint(int address, MemoryMonitor mon) {
        final int a = address - start;
        watchPoints[a] = MemoryMonitor.Proxy.INSTANCE.remove(watchPoints[a], mon);
    }

}
