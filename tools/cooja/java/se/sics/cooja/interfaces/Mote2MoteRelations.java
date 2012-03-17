/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: Mote2MoteRelations.java,v 1.7 2010/12/10 17:50:49 nifi Exp $
 */

package se.sics.cooja.interfaces;

import java.awt.Color;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.SimEventCentral.MoteCountListener;

/**
 * Mote2Mote Relations is used to show mote relations in simulated
 * applications, typically for debugging or visualization purposes.
 *
 * The interface is write-only: the simulated Contiki has no knowledge of current relations
 * with other motes. The Contiki application can, however, add and remove relations.
 *
 * A Contiki application adds/removes a relation by outputting a simple messages on its log interface,
 * typically via printf()'s of the serial port.
 *
 * Syntax:
 * "<relation identifier #L> <destination mote ID> <add/remove>"
 *
 * Example, add relation between this mote and mote with ID 1
 * "#L 1 1"
 *
 * Example, remove relation between this mote and mote with ID 1
 * "#L 1 0"
 *
 * Example, remove relation between this mote and mote with ID 2
 * "#L 2 0"
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote2Mote Relations")
public class Mote2MoteRelations extends MoteInterface {
  private static Logger logger = Logger.getLogger(Mote2MoteRelations.class);
  private Mote mote = null;

  private ArrayList<Mote> relations = new ArrayList<Mote>();
  private GUI gui;

  private Observer logObserver = new Observer() {
    public void update(Observable o, Object arg) {
      String msg = ((Log) o).getLastLogMessage();
      handleNewLog(msg);
    };
  };
  
  private MoteCountListener moteCountListener;
  
  public Mote2MoteRelations(Mote mote) {
    this.mote = mote;
    this.gui = mote.getSimulation().getGUI();
  }

  public void added() {
    super.added();
    
    /* Observe log interfaces */
    for (MoteInterface mi: mote.getInterfaces().getInterfaces()) {
      if (mi instanceof Log) {
        ((Log)mi).addObserver(logObserver);
      }
    }

    /* Observe other motes: if removed, remove our relations to them too */
    mote.getSimulation().getEventCentral().addMoteCountListener(moteCountListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        /* Ignored */
      }
      public void moteWasRemoved(Mote mote) {
        /* This mote was removed - cleanup by removed() */
        if (Mote2MoteRelations.this.mote == mote) {
          return;
        }

        /* Remove mote from our relations */
        if (!relations.contains(mote)) {
          return;
        }
        relations.remove(mote);
        gui.removeMoteRelation(Mote2MoteRelations.this.mote, mote);
      }
    });
  }
  
  public void removed() {
    super.removed();

    /* Stop observing log interfaces */
    for (MoteInterface mi: mote.getInterfaces().getInterfaces()) {
      if (mi instanceof Log) {
        ((Log)mi).deleteObserver(logObserver);
      }
    }
    logObserver = null;

    /* Remove all relations to other motes */
    Mote[] relationsArr = relations.toArray(new Mote[0]);
    for (Mote m: relationsArr) {
      gui.removeMoteRelation(Mote2MoteRelations.this.mote, m);
    }
    relations.clear();

    mote.getSimulation().getEventCentral().removeMoteCountListener(moteCountListener);
  }

  private void handleNewLog(String msg) {
    if (msg == null) {
      return;
    }

    if (msg.startsWith("DEBUG: ")) {
      msg = msg.substring("DEBUG: ".length());
    }
    
    if (!msg.startsWith("#L ")) {
      return;
    }

    String colorName = null;
    int colorIndex = msg.indexOf(';');
    if (colorIndex > 0) {
       colorName = msg.substring(colorIndex + 1).trim();
       msg = msg.substring(0, colorIndex).trim();
    }
    String[] args = msg.split(" ");
    if (args.length != 3) {
      return;
    }

    int destID;
    try {
      destID = Integer.parseInt(args[1]);
    } catch (Exception e) {
      // Not a mote id
      return;
    }
    String state = args[2];

    /* Locate destination mote */
    /* TODO Use Rime address interface instead of mote ID? */
    Mote destinationMote = mote.getSimulation().getMoteWithID(destID);
    if (destinationMote == null) {
      logger.warn("No destination mote with ID: " + destID);
      return;
    }
    if (destinationMote == mote) {
      /*logger.warn("Cannot create relation with ourselves");*/
      return;
    }

    /* Change line state */
    if (state.equals("1")) {
      if (relations.contains(destinationMote)) {
        return;
      }
      relations.add(destinationMote);
      gui.addMoteRelation(mote, destinationMote, decodeColor(colorName));
    } else {
      relations.remove(destinationMote);
      gui.removeMoteRelation(mote, destinationMote);
    }

    setChanged();
    notifyObservers();
  }

  private Color decodeColor(String colorString) {
    if (colorString == null) {
      return null;
    } else if (colorString.equalsIgnoreCase("red")) {
      return Color.RED;
    } else if (colorString.equalsIgnoreCase("green")) {
      return Color.GREEN;
    } else if (colorString.equalsIgnoreCase("blue")) {
      return Color.BLUE;
    } else if (colorString.equalsIgnoreCase("orange")) {
      return Color.ORANGE;
    } else if (colorString.equalsIgnoreCase("pink")) {
      return Color.PINK;
    } else if (colorString.equalsIgnoreCase("yellow")) {
      return Color.YELLOW;
    } else if (colorString.equalsIgnoreCase("gray")) {
      return Color.GRAY;
    } else if (colorString.equalsIgnoreCase("magenta")) {
      return Color.MAGENTA;
    } else if (colorString.equalsIgnoreCase("black")) {
      return Color.BLACK;
    } else {
      try {
        return Color.decode(colorString);
      } catch (NumberFormatException e) {
      }
      return null;
    }
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel countLabel = new JLabel();
    countLabel.setText("Mote has " + relations.size() + " mote relations");
    panel.add(countLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        countLabel.setText("Mote has " + relations.size() + " mote relations");
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

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
