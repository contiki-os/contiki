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
 */

package org.contikios.cooja.plugins;

import java.awt.BorderLayout;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JScrollPane;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;

/**
 * Shows a summary of all mote types.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote Type Information")
@PluginType(PluginType.SIM_PLUGIN)
public class MoteTypeInformation extends VisPlugin {
  private static Logger logger = Logger.getLogger(MoteTypeInformation.class);

  private Simulation simulation;
  private Observer simObserver;
  private int nrMotesTypes = -1;

  /**
   * @param simulation Simulation
   * @param gui Cooja
   */
  public MoteTypeInformation(Simulation simulation, Cooja gui) {
    super("Mote Type Information", gui);
    this.simulation = simulation;

    this.getContentPane().add(BorderLayout.CENTER,
        new JScrollPane(createPanel(),
        JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
        JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
    pack();
    setSize(Math.min(getWidth(), 600), Math.min(getHeight(), 600));
    nrMotesTypes = simulation.getMoteTypes().length;

    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (MoteTypeInformation.this.simulation.getMoteTypes().length == nrMotesTypes) {
          return;
        }
        nrMotesTypes = MoteTypeInformation.this.simulation.getMoteTypes().length;
        MoteTypeInformation.this.getContentPane().removeAll();
        MoteTypeInformation.this.getContentPane().add(BorderLayout.CENTER,
            new JScrollPane(createPanel(),
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
        revalidate();
        repaint();
      }
    });
  }

  private JComponent createPanel() {
    Box box = Box.createVerticalBox();
    box.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    /* Mote types */
    for (MoteType moteType: simulation.getMoteTypes()) {
      String moteTypeString =
        Cooja.getDescriptionOf(moteType) +": " +
        "ID=" + moteType.getIdentifier() +
        ", \"" + moteType.getDescription() + "\"";

      JComponent moteTypeVisualizer = moteType.getTypeVisualizer();
      if (moteTypeVisualizer == null) {
        moteTypeVisualizer = new JLabel("[no information available]");
      }
      moteTypeVisualizer.setAlignmentX(Box.LEFT_ALIGNMENT);
      moteTypeVisualizer.setBorder(BorderFactory.createTitledBorder(moteTypeString));
      box.add(moteTypeVisualizer);
      box.add(Box.createVerticalStrut(15));
    }
    return box;
  }


  public void closePlugin() {
    simulation.deleteObserver(simObserver);
  }

}
