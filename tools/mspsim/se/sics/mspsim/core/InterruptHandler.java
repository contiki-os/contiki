/**
 * 
 */
package se.sics.mspsim.core;

/**
 * @author joakim
 *
 */
public interface InterruptHandler {
  // We should add "Interrupt serviced..." to indicate that its latest
  // Interrupt was serviced...
  public void interruptServiced(int vector);
  public String getName();
}
