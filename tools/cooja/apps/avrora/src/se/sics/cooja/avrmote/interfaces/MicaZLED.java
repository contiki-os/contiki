/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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

package se.sics.cooja.avrmote.interfaces;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.avrmote.MicaZMote;
import se.sics.cooja.interfaces.LED;
import avrora.sim.FiniteStateMachine;
import avrora.sim.platform.MicaZ;

/**
 * @author Joakim Eriksson
 */
@ClassDescription("MicaZ LED")
public class MicaZLED extends LED {
  private static Logger logger = Logger.getLogger(MicaZLED.class);

  private boolean blueOn = false;
  private boolean greenOn = false;
  private boolean redOn = false;

  private static final Color DARK_BLUE = new Color(0, 0, 100);
  private static final Color DARK_GREEN = new Color(0, 100, 0);
  private static final Color DARK_RED = new Color(100, 0, 0);
  private static final Color BLUE = new Color(0, 0, 255);
  private static final Color GREEN = new Color(0, 255, 0);
  private static final Color RED = new Color(255, 0, 0);

  public MicaZLED(Mote mote) {
    MicaZ micaZ = ((MicaZMote) mote).getMicaZ();

    avrora.sim.platform.LED.LEDGroup leds =
      (avrora.sim.platform.LED.LEDGroup) micaZ.getDevice("leds");

    leds.leds[0].getFSM().insertProbe(new FiniteStateMachine.Probe() {
      public void fireAfterTransition(int old, int newstate) {
        redOn = newstate > 0;
        setChanged();
        notifyObservers();
      }
      public void fireBeforeTransition(int arg0, int arg1) {
      }
    });
    leds.leds[1].getFSM().insertProbe(new FiniteStateMachine.Probe() {
      public void fireAfterTransition(int old, int newstate) {
        blueOn = newstate > 0;
        setChanged();
        notifyObservers();
      }
      public void fireBeforeTransition(int arg0, int arg1) {
      }
    });
    leds.leds[2].getFSM().insertProbe(new FiniteStateMachine.Probe() {
      public void fireAfterTransition(int old, int newstate) {
        greenOn = newstate > 0;
        setChanged();
        notifyObservers();
      }
      public void fireBeforeTransition(int arg0, int arg1) {
      }
    });    
  }

  public boolean isAnyOn() {
    return blueOn || greenOn || redOn;
  }

  public boolean isGreenOn() {
    return greenOn;
  }

  public boolean isYellowOn()  {
    return blueOn; /* Returning blue */
  }

  public boolean isRedOn() {
    return redOn;
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
          g.setColor(BLUE);
          g.fillOval(x, y, d, d);
          g.setColor(Color.BLACK);
          g.drawOval(x, y, d, d);
        } else {
          g.setColor(DARK_BLUE);
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

