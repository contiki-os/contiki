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
 * $Id: EventpointEvaluator.java,v 1.1 2009/02/26 13:47:38 fros4943 Exp $
 */

package se.sics.chakana;

import java.util.Observable;
import java.util.Observer;
import java.util.Vector;
import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.Simulation;
import se.sics.chakana.eventpoints.*;

/**
 * Keeps track of all active eventpoints. The control method blocks until any of
 * the active eventpoints trigger.
 * 
 * @author Fredrik Osterlind
 */
public class EventpointEvaluator {
  private static Logger logger = Logger.getLogger(EventpointEvaluator.class);

  private GUI myGUI;
  private Vector<Eventpoint> allEventpoints;
  private Eventpoint lastTriggeredEventpoint;
  private Simulation sim = null;

  private int counterEventpointID = 0;
  
  private Observer tickObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      // Evaluate active eventpoints
      for (Eventpoint eventpoint: allEventpoints) {
        if (eventpoint.evaluate()) {
          lastTriggeredEventpoint = eventpoint;
          sim.stopSimulation();
          logger.info("Eventpoint triggered: " + eventpoint);
        }
      }
    }
  };
  
  /**
   * Creates a new eventpoint evaluator.
   * 
   * @param gui GUI with simulation
   */
  public EventpointEvaluator(GUI gui) {
    myGUI = gui;
    allEventpoints = new Vector<Eventpoint>();
  }
  
  
  /**
   * Blocks until an active eventpoint triggers.
   * 
   * @return Triggered eventpoint
   * @throws EventpointException At evenpoint setup errors
   */
  public Eventpoint resumeSimulation() throws EventpointException {
    if (myGUI.getSimulation() == null)
      throw new EventpointException("No simulation to observe");
    if (allEventpoints == null || allEventpoints.isEmpty())
      throw new EventpointException("No eventpoints exist");

    // Make sure tick observer is observing the current simulation
    myGUI.getSimulation().deleteTickObserver(tickObserver);
    myGUI.getSimulation().addTickObserver(tickObserver);
    
    // Evaluate active eventpoints before starting simulation
    for (Eventpoint eventpoint: allEventpoints) {
      if (eventpoint.evaluate()) {
        lastTriggeredEventpoint = eventpoint;
        logger.info("Eventpoint triggered (EARLY): " + eventpoint);
        return lastTriggeredEventpoint;
      }
    }

    // Reset last triggered eventpoint and start simulation
    lastTriggeredEventpoint = null;
    sim = myGUI.getSimulation();
    sim.startSimulation();

    // Block until tickobserver stops simulation
    while (lastTriggeredEventpoint == null || myGUI.getSimulation().isRunning()) {
      Thread.yield();
    }
    
    if (lastTriggeredEventpoint == null)
      throw new EventpointException("Simulation was stopped without eventpoint triggering");

    return lastTriggeredEventpoint;
  }

  public Eventpoint getTriggeredEventpoint() {
    return lastTriggeredEventpoint;
  }
  
  public int getLastTriggeredEventpointID() {
    return lastTriggeredEventpoint.getID();
  }
  
  public Eventpoint getEventpoint(int id) {
    for (Eventpoint eventpoint: allEventpoints) {
      if (eventpoint.getID() == id) {
        return eventpoint;
      }
    }
    return null;
  }
  
  public void addEventpoint(Eventpoint eventpoint) {
    eventpoint.setID(counterEventpointID++);
    allEventpoints.add(eventpoint);
  }
  
  public void clearAllEventpoints() {
    allEventpoints.clear();
  }
  
  public void deleteEventpoint(int id) {
    Eventpoint eventpointToDelete = null;
    for (Eventpoint eventpoint: allEventpoints) {
      if (eventpoint.getID() == id) {
        eventpointToDelete = eventpoint;
        break;
      }
    }
    
    if (eventpointToDelete != null)
      allEventpoints.remove(eventpointToDelete);
  }
  
  class EventpointException extends Exception {
    public EventpointException(String message) {
      super(message);
    }
  }
  
}
