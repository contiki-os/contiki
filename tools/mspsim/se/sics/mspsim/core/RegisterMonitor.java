package se.sics.mspsim.core;

import se.sics.mspsim.util.ProxySupport;

public interface RegisterMonitor {

    public void notifyReadBefore(int reg, Memory.AccessMode mode);
    public void notifyReadAfter(int reg, Memory.AccessMode mode);

    public void notifyWriteBefore(int reg, int data, Memory.AccessMode mode);
    public void notifyWriteAfter(int reg, int data, Memory.AccessMode mode);

    public static class Adapter implements RegisterMonitor {

        @Override
        public void notifyReadBefore(int reg, Memory.AccessMode mode) {
        }

        @Override
        public void notifyReadAfter(int reg, Memory.AccessMode mode) {
        }

        @Override
        public void notifyWriteBefore(int reg, int data, Memory.AccessMode mode) {
        }

        @Override
        public void notifyWriteAfter(int reg, int data, Memory.AccessMode mode) {
        }

    }

    public static class Proxy extends ProxySupport<RegisterMonitor> implements RegisterMonitor {
        public static final Proxy INSTANCE = new Proxy();

        @Override
        public void notifyReadBefore(int reg, Memory.AccessMode mode) {
            RegisterMonitor[] listeners = this.listeners;
            for(RegisterMonitor listener : listeners) {
                listener.notifyReadBefore(reg, mode);
            }
        }

        @Override
        public void notifyReadAfter(int reg, Memory.AccessMode mode) {
            RegisterMonitor[] listeners = this.listeners;
            for(RegisterMonitor listener : listeners) {
                listener.notifyReadAfter(reg, mode);
            }
        }

        @Override
        public void notifyWriteBefore(int reg, int data, Memory.AccessMode mode) {
            RegisterMonitor[] listeners = this.listeners;
            for(RegisterMonitor listener : listeners) {
                listener.notifyWriteBefore(reg, data, mode);
            }
        }

        @Override
        public void notifyWriteAfter(int reg, int data, Memory.AccessMode mode) {
            RegisterMonitor[] listeners = this.listeners;
            for(RegisterMonitor listener : listeners) {
                listener.notifyWriteAfter(reg, data, mode);
            }
        }

    }
}
