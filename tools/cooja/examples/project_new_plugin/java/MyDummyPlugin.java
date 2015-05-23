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

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.TimeEvent;
import org.contikios.cooja.VisPlugin;

/**
 * This is a simple example COOJA plugin.
 * It is a simulation plugin (SIM_PLUGIN): it depends on a single simulation.
 * 
 * This plugin demonstrates user interaction, scheduling time events, 
 * and saving plugin configurations.
 * 
 * This project must be loaded in COOJA before the plugin can be used:
 * Menu>Settings>Manage project directories>Browse>..>OK
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Example Plugin") /* Description shown in menu */
@PluginType(PluginType.SIM_PLUGIN)
public class MyDummyPlugin extends VisPlugin {
  private static final long serialVersionUID = 4368807123350830772L;
  private static Logger logger = Logger.getLogger(MyDummyPlugin.class);

  private Simulation sim;
  private Observer msObserver;
  private JTextField textField;

  /**
   * @param simulation Simulation object
   * @param gui GUI object 
   */
  public MyDummyPlugin(Simulation simulation, Cooja gui) {
    super("Example plugin title", gui);
    this.sim = simulation;

    /* Button */
    JButton button = new JButton("button");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        logger.info("Button clicked");
      }
    });
    add(BorderLayout.NORTH, button);

    /* Text field */
    textField = new JTextField("enter text to save");
    add(BorderLayout.SOUTH, textField);

    /* Register as millisecond observer: print time every millisecond */
    simulation.addMillisecondObserver(msObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        logger.info("Millisecond observer: simulation time is now: " + sim.getSimulationTimeMillis() + " ms");
      }
    });

    /* Register self-repeating event in simulation thread */
    simulation.invokeSimulationThread(new Runnable() {
      public void run() {
        /* This is called from the simulation thread; we can safely schedule events */
        sim.scheduleEvent(repeatEvent, sim.getSimulationTime());
      }
    });

    setSize(300,100);
  }

  private TimeEvent repeatEvent = new TimeEvent(0) {
    public void execute(long t) {
      logger.info("Repeat event: simulation time is now: " + sim.getSimulationTimeMillis() + " ms");

      /* This is called from the simulation thread; we can safely schedule events */
      sim.scheduleEvent(this, t + 10*Simulation.MILLISECOND);
    }
  };

  public void closePlugin() {
    /* Clean up plugin resources */
    logger.info("Deleting millisecond observer");
    sim.deleteMillisecondObserver(msObserver);

    logger.info("Unschedule repeat event");
    repeatEvent.remove();
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    /* Save text field */
    element = new Element("textfield");
    element.setText(textField.getText());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("textfield")) {
        final String text = element.getText();
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            textField.setText(text);
          }
        });
      }
    }
    return true;
  }

}
