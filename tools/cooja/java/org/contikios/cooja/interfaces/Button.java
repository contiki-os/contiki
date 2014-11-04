/*
 * Copyright (c) 2014, TU Braunschweig.
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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

package org.contikios.cooja.interfaces;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.Collection;
import javax.swing.JButton;
import javax.swing.JPanel;
import org.contikios.cooja.*;
import org.jdom.Element;

/**
 * A Button represents a mote button. An implementation should notify all
 * observers when the button changes state, and may simulate external interrupts
 * by waking up a mote if button state changes.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Button")
public abstract class Button extends MoteInterface {

  private final Simulation sim;

  private final MoteTimeEvent pressButtonEvent;
  private final MoteTimeEvent releaseButtonEvent;

  public Button(Mote mote) {
    sim = mote.getSimulation();

    pressButtonEvent = new MoteTimeEvent(mote, 0) {
      @Override
      public void execute(long t) {
        doPressButton();
      }
    };
    releaseButtonEvent = new MoteTimeEvent(mote, 0) {
      @Override
      public void execute(long t) {
        doReleaseButton();
      }
    };
  }

  /**
   * Clicks button. Button will be pressed for some time and then automatically
   * released.
   */
  public void clickButton() {
    sim.invokeSimulationThread(new Runnable() {
      @Override
      public void run() {
        sim.scheduleEvent(pressButtonEvent, sim.getSimulationTime());
        sim.scheduleEvent(releaseButtonEvent, sim.getSimulationTime() + Simulation.MILLISECOND);
      }      
    });
  }

  /**
   * Presses button.
   */
  public void pressButton() {
    sim.invokeSimulationThread(new Runnable() {
      @Override
      public void run() {
        sim.scheduleEvent(pressButtonEvent, sim.getSimulationTime());
      }      
    });
  }

  /**
   * Node-type dependent implementation of pressing a button.
   */
  protected abstract void doPressButton();

  /**
   * Releases button.
   */
  public void releaseButton() {
    sim.invokeSimulationThread(new Runnable() {
      @Override
      public void run() {
        sim.scheduleEvent(releaseButtonEvent, sim.getSimulationTime());
      }
    });
  }

  /**
   * Node-type dependent implementation of releasing a button.
   */
  protected abstract void doReleaseButton();

  /**
   * @return True if button is pressed
   */
  public abstract boolean isPressed();

  @Override
  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JButton clickButton = new JButton("Click button");

    panel.add(clickButton);
    
    clickButton.addMouseListener(new MouseAdapter() {
      @Override
      public void mousePressed(MouseEvent e) {
        sim.invokeSimulationThread(new Runnable() {

          @Override
          public void run() {
            doPressButton();
          }
        });
      }

      @Override
      public void mouseReleased(MouseEvent e) {
        sim.invokeSimulationThread(new Runnable() {

          @Override
          public void run() {
            doReleaseButton();
          }
        });
      }
    });

    clickButton.addKeyListener(new KeyAdapter() {
      @Override
      public void keyPressed(KeyEvent e) {
        sim.invokeSimulationThread(new Runnable() {

          @Override
          public void run() {
            doPressButton();
          }
        });
      }

      @Override
      public void keyReleased(KeyEvent e) {
        sim.invokeSimulationThread(new Runnable() {

          @Override
          public void run() {
            doReleaseButton();
          }
        });
      }
    });

    return panel;
  }

  @Override
  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  @Override
  public Collection<Element> getConfigXML() {
    // The button state will not be saved!
    return null;
  }

  @Override
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    // The button state will not be saved!
  }
}
