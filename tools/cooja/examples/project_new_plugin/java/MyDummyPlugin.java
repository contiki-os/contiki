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
 * $Id: MyDummyPlugin.java,v 1.3 2009/05/26 14:34:43 fros4943 Exp $
 */

import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;

/**
 * This is a simple example plugin.
 * It is a simulation plugin (SIM_PLUGIN): it depends on a single simulation.
 *
 * @author Fredrik Österlind
 */
@ClassDescription("Dummy Plugin") /* Description shown in menu */
@PluginType(PluginType.SIM_PLUGIN)
public class MyDummyPlugin extends VisPlugin {
  private static Logger logger = Logger.getLogger(MyDummyPlugin.class);

  private Simulation mySimulation;
  private Observer millisecondObserver;

  /**
   * Creates a new dummy plugin.
   *
   * @param simulationToVisualize Simulation to visualize
   */
  public MyDummyPlugin(Simulation simulationToVisualize, GUI gui) {
    super("Example plugin title", gui);
    mySimulation = simulationToVisualize;

    // Create and add a button
    JButton button = new JButton("dummy button");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        logger.info("Button clicked");
      }
    });
    add(button);

    /* Register as millisecond observer */
    mySimulation.addMillisecondObserver(millisecondObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        logger.info("Another millisecond passed - simulation time is now " + mySimulation.getSimulationTimeMillis() + " ms");
      }
    });
    
    /* Register self-repeating event */
    repeatEvent.execute(mySimulation.getSimulationTime());    

    setSize(300,100); // Set an initial size of this plugin
  }

  private TimeEvent repeatEvent = new TimeEvent(0) {
    public void execute(long t) {
      logger.info("Event executed - simulation time is now " + mySimulation.getSimulationTimeMillis() + " ms");

      mySimulation.scheduleEvent(this, t+10*Simulation.MILLISECOND);
    }
  };

  public void closePlugin() {
    /* Clean up plugin resources */
    mySimulation.deleteMillisecondObserver(millisecondObserver);
    repeatEvent.remove();
  }

}
