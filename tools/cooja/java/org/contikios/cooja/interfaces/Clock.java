/*
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

import java.awt.GridLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Collection;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.contikios.cooja.*;
import org.jdom.Element;

/**
 * Represents a mote's internal clock. Notice that the overall
 * simulation time and the mote's own time may differ.
 *
 * This observable never notifies.
 *
 * @author Fredrik Osterlind
 *         Andreas Löscher
 */
@ClassDescription("Clock")
public abstract class Clock extends MoteInterface {
  /**
   * Set mote's time to given time.
   *
   * @param newTime Time
   */
  public abstract void setTime(long newTime);

  /**
   * @return Current time
   */
  public abstract long getTime();

  /**
   * Set time drift.
   *
   * @param timeDrift Time drift
   */
  public abstract void setDrift(long timeDrift);

  /**
   * The clock drift provides information about the mote's internal time,
   * and can the used to calculate for instance its startup time.
   * 
   * The startup time is the negative drift time.
   * 
   * The mote internal time can be calculated by:
   * [current simulation time] + [mote drift].
   * 
   * @see Simulation#getSimulationTime()
   * @return Time drift
   */
  public abstract long getDrift();


  /**
   * The clock deviation is a factor that represents with how much speed the
   * mote progresses through the simulation in relation to the simulation speed.
   *
   * A value of 1.0 results in the mote being simulated with the same speed
   * as the simulation. A value of 0.5 results in the mote being simulation
   * at half of the simulation speed.
   *
   *  @param deviation Deviation factor
   */
  public abstract void setDeviation(double deviation);

  /**
   * Get deviation factor
   */
  public abstract double getDeviation();
  
  @Override
  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    GridLayout layout = new GridLayout(0,2);
    
    /* elements */
    final JLabel timeLabel = new JLabel("Time (ms)");
    final JTextField timeField = new JTextField(String.valueOf(getTime() / 1000));
    final JLabel deviationLabel = new JLabel("Deviation Factor");
    final JTextField deviationField = new JTextField(String.valueOf(getDeviation()));
    final JButton readButton = new JButton("Read Clock Values");
    final JButton updateButton = new JButton("Write Clock Values");
    /* set layout */
    panel.setLayout(layout);
    /* add components */
    panel.add(timeLabel);
    panel.add(timeField);
    panel.add(deviationLabel);
    panel.add(deviationField);
    panel.add(readButton);
    panel.add(updateButton);
    
    readButton.addMouseListener(new MouseAdapter() {      
      @Override
      public void mouseClicked(MouseEvent ev) {
        if (ev.getButton()==1) {
          timeField.setText(String.valueOf(getTime() / 1000));
          deviationField.setText(String.valueOf(getDeviation()));
        }
      }
    });
    
    updateButton.addMouseListener(new MouseAdapter() {      
      @Override
      public void mouseClicked(MouseEvent ev) {
        if (ev.getButton()==1) {
          setTime(Long.parseLong(timeField.getText()) * 1000);
          setDeviation(Double.parseDouble(deviationField.getText()));
        }
      }
    });
    
    return panel;
  }
  
  @Override
  public void releaseInterfaceVisualizer(JPanel panel) {
  }
 
  @Override
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element = new Element("deviation");
    element.setText(String.valueOf(getDeviation()));
    config.add(element);
    return config;
  }

  @Override
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("deviation")) {
        setDeviation(Double.parseDouble(element.getText()));
      }
    }
  }
}
