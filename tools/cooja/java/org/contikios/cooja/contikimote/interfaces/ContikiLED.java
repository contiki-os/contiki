/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

package org.contikios.cooja.contikimote.interfaces;

import java.awt.*;
import java.util.*;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.LED;
import org.contikios.cooja.interfaces.PolledAfterActiveTicks;
import org.contikios.cooja.mote.memory.VarMemory;

/**
 * LEDs mote interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simLedsValue
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>leds_interface
 * </ul>
 * <p>
 *
 * This observable notifies when any LED changes.
 *
 * @author Fredrik Osterlind
 */
public class ContikiLED extends LED implements ContikiMoteInterface, PolledAfterActiveTicks {
  private static Logger logger = Logger.getLogger(ContikiLED.class);

  private Mote mote = null;
  private VarMemory moteMem = null;
  private byte currentLedValue = 0;

  private static final byte LEDS_GREEN = 1;
  private static final byte LEDS_YELLOW = 2;
  private static final byte LEDS_RED = 4;

  private static final Color DARK_GREEN = new Color(0, 50, 0);
  private static final Color DARK_YELLOW = new Color(50, 50, 0);
  private static final Color DARK_RED = new Color(50, 0, 0);
  private static final Color GREEN = new Color(0, 255, 0);
  private static final Color YELLOW = new Color(255, 255, 0);
  private static final Color RED = new Color(255, 0, 0);

  public ContikiLED() {
  }

  /**
   * Creates an interface to LEDs at mote.
   *
   * @param mote Mote
   *
   * @see Mote
   * @see org.contikios.cooja.MoteInterfaceHandler
   */
  public ContikiLED(Mote mote) {
    this.mote = mote;
    this.moteMem = new VarMemory(mote.getMemory());
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

  public void doActionsAfterTick() {
    boolean ledChanged;

    byte newLedsValue = moteMem.getByteValueOf("simLedsValue");
    if (newLedsValue != currentLedValue) {
      ledChanged = true;
    } else {
      ledChanged = false;
    }

    currentLedValue = newLedsValue;
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

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
