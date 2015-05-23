package org.contikios.cooja.interfaces;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.interfaces.ContikiLED;

public class ApplicationLED extends LED {
    private static Logger logger = Logger.getLogger(ContikiLED.class);

    private Mote mote = null;
    private byte currentLedValue = 0;

    public static final byte LEDS_GREEN = 1;
    public static final byte LEDS_YELLOW = 2;
    public static final byte LEDS_RED = 4;
    
    private static final Color DARK_GREEN = new Color(0, 50, 0);
    private static final Color DARK_YELLOW = new Color(50, 50, 0);
    private static final Color DARK_RED = new Color(50, 0, 0);
    private static final Color GREEN = new Color(0, 255, 0);
    private static final Color YELLOW = new Color(255, 255, 0);
    private static final Color RED = new Color(255, 0, 0);

     public ApplicationLED(Mote mote) {
       this.mote = mote;
     }

     public static String[] getCoreInterfaceDependencies() {
       return new String[]{"leds_interface"};
     }

     public boolean isAnyOn() {
       return currentLedValue > 0;
     }

     public boolean isGreenOn() {
       return (currentLedValue & LEDS_GREEN) > 0;
     }

     public boolean isYellowOn() {
       return (currentLedValue & LEDS_YELLOW) > 0;
     }

     public boolean isRedOn() {
       return (currentLedValue & LEDS_RED) > 0;
     }

     public void setLED(int led) {
       boolean ledChanged;
       ledChanged = led != currentLedValue;

       currentLedValue = (byte) led;
       if (ledChanged) {
         this.setChanged();
         this.notifyObservers(mote);
       }
     }

     public JPanel getInterfaceVisualizer() {
       final JPanel panel = new JPanel() {
         public void paintComponent(Graphics g) {
           super.paintComponent(g);

           int x = 20;
           int y = 25;
           int d = 25;

           if (isGreenOn()) {
             g.setColor(GREEN);
             g.fillOval(x, y, d, d);
             g.setColor(Color.BLACK);
             g.drawOval(x, y, d, d);
           } else {
             g.setColor(DARK_GREEN);
             g.fillOval(x + 5, y + 5, d-10, d-10);
           }

           x += 40;

           if (isRedOn()) {
             g.setColor(RED);
             g.fillOval(x, y, d, d);
             g.setColor(Color.BLACK);
             g.drawOval(x, y, d, d);
           } else {
             g.setColor(DARK_RED);
             g.fillOval(x + 5, y + 5, d-10, d-10);
           }

           x += 40;

           if (isYellowOn()) {
             g.setColor(YELLOW);
             g.fillOval(x, y, d, d);
             g.setColor(Color.BLACK);
             g.drawOval(x, y, d, d);
           } else {
             g.setColor(DARK_YELLOW);
             g.fillOval(x + 5, y + 5, d-10, d-10);
           }
         }
       };

       Observer observer;
       this.addObserver(observer = new Observer() {
         public void update(Observable obs, Object obj) {
           panel.repaint();
         }
       });

       // Saving observer reference for releaseInterfaceVisualizer
       panel.putClientProperty("intf_obs", observer);

       panel.setMinimumSize(new Dimension(140, 60));
       panel.setPreferredSize(new Dimension(140, 60));

       return panel;
     }

     public void releaseInterfaceVisualizer(JPanel panel) {
       Observer observer = (Observer) panel.getClientProperty("intf_obs");
       if (observer == null) {
         logger.fatal("Error when releasing panel, observer is null");
         return;
       }

       this.deleteObserver(observer);
     }

     public Collection<Element> getConfigXML() {
       return null;
     }

     @Override
     public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
        // TODO Auto-generated method stub
    }
   }