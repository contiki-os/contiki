package se.sics.cooja;

import javax.swing.*;

public class CoojaApplet extends JApplet {

  public static CoojaApplet applet = null;

  public void init()
  {
    applet = this;
  }

  public void start(){
    GUI.main(new String[] { "-applet" });
  }

  public void stop(){
  }
}
