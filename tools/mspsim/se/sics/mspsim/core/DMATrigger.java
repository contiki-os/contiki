package se.sics.mspsim.core;

public interface DMATrigger {
    public void setDMA(DMA dma);
    public boolean getDMATriggerState(int index);
    public void clearDMATrigger(int index);
}
