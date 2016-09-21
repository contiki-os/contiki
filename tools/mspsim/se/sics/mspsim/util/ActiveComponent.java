package se.sics.mspsim.util;

/* Active components are always started when added to registry */
public interface ActiveComponent {
  public void init(String name, ComponentRegistry registry);
  public void start();
}
