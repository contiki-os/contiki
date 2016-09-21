package se.sics.mspsim.core;


public interface SFRModule extends InterruptHandler {
  
  public void enableChanged(int reg, int bit, boolean enabled);
  
//  public void ieChanged(int reg, int bit, boolean enabled);

  /* public void interruptServiced(int reg, int bit); */
}
