package se.sics.mspsim.ui;

import java.awt.Component;

public interface ManagedWindow {

  public void setSize(int width, int height);
  public void setBounds(int x, int y, int width, int height);
  public void pack();

  public void add(Component component);
  public void removeAll();

  public boolean isVisible();

  public void setVisible(boolean b);

  public String getTitle();

  public void setTitle(String string);

}
