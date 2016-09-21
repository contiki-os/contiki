package se.sics.mspsim.util;

/* Service component that can be stopped and is not autostarted when
 * registered (unless it also implements ActiveComponent)
 */
public interface ServiceComponent {
  public static enum Status {STARTED, STOPPED, ERROR};
  public String getName();
  public Status getStatus();
  public void init(String name, ComponentRegistry registry);
  public void start();
  public void stop();
}
