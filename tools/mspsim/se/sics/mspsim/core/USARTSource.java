package se.sics.mspsim.core;

public interface USARTSource {

    public void addUSARTListener(USARTListener listener);
    public void removeUSARTListener(USARTListener listener);

    public void addStateChangeListener(StateChangeListener listener);
    public void removeStateChangeListener(StateChangeListener listener);

    /* for input into this UART */
    public boolean isReceiveFlagCleared();
    public void byteReceived(int b);
    
}
