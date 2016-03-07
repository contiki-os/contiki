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
 * $Id: AngleInterval.java,v 1.2 2007/03/23 21:13:43 fros4943 Exp $
 */

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;

import javax.swing.JFileChooser;
import javax.swing.JScrollPane;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.TimeEvent;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.dialogs.MessageList;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.util.StringUtils;

@ClassDescription("Mobility")
@PluginType(PluginType.SIM_PLUGIN)
public class Mobility extends VisPlugin {
  private static final long serialVersionUID = -1087396096570660083L;
  private static Logger logger = Logger.getLogger(Mobility.class);

  private static final boolean QUIET = false;

  private final boolean WRAP_MOVES = true; /* Wrap around loaded moves forever */
  
  private Move[] entries; /* All mote moves */
  private Simulation simulation;
  private long periodStart; /* us */
  private int currentMove;

  private File filePositions = null;

  private MessageList log = new MessageList();

  public Mobility(Simulation simulation, final Cooja gui) {
    super("Mobility", gui, false);
    this.simulation = simulation;

    log.addPopupMenuItem(null, true); /* Create message list popup */
    add(new JScrollPane(log));

    if (!QUIET) {
      log.addMessage("Mobility plugin started at (ms): " + simulation.getSimulationTimeMillis());
      logger.info("Mobility plugin started at (ms): " + simulation.getSimulationTimeMillis());
    }
    setSize(500,200);
  }

  public void startPlugin() {
    super.startPlugin();

    if (filePositions != null) {
      /* Positions were already loaded */
      return;
    }

    JFileChooser fileChooser = new JFileChooser();
    File suggest = new File(Cooja.getExternalToolsSetting("MOBILITY_LAST", "positions.dat"));
    fileChooser.setSelectedFile(suggest);
    fileChooser.setDialogTitle("Select positions file");
    int reply = fileChooser.showOpenDialog(Cooja.getTopParentContainer());
    if (reply == JFileChooser.APPROVE_OPTION) {
      filePositions = fileChooser.getSelectedFile();
      Cooja.setExternalToolsSetting("MOBILITY_LAST", filePositions.getAbsolutePath());
    }
    if (filePositions == null) {
      throw new RuntimeException("No positions file");
    }
    loadPositions();
  }
  
  private void loadPositions() {
    try {
      if (!QUIET) {
        log.addMessage("Parsing position file: " + filePositions);
        logger.info("Parsing position file: " + filePositions);
      }
  
      String data = StringUtils.loadFromFile(filePositions);
      
      /* Load move by move */
      ArrayList<Move> entriesList = new ArrayList<Move>();
      for (String line: data.split("\n")) {
        if (line.trim().isEmpty() || line.startsWith("#")) {
          /* Skip header/metadata */
          continue;
        }

        String[] args = line.split(" ");
        Move e = new Move();
        e.moteIndex = Integer.parseInt(args[0]); /* XXX Mote index. Not ID */
        e.time = (long) (Double.parseDouble(args[1])*1000.0*Simulation.MILLISECOND); /* s -> us */
        e.posX = Double.parseDouble(args[2]);
        e.posY = Double.parseDouble(args[3]);

        entriesList.add(e);
      }
      Collections.sort(entriesList, new Comparator<Move>(){
        public int compare (Move m1, Move m2){
          if(m1.time < m2.time) return -1;
          else if(m1.time > m2.time) return 1;
          else return 0;
        }
      });
      entries = entriesList.toArray(new Move[0]);
      if (!QUIET) {
        log.addMessage("Loaded " + entries.length + " positions");
        logger.info("Loaded " + entries.length + " positions");
      }

      setTitle("Mobility: " + filePositions.getName());
      
      /* Execute first event - it will reschedule itself */
      simulation.invokeSimulationThread(new Runnable() {
        public void run() {
          currentMove = 0;
          periodStart = simulation.getSimulationTime();
          /*logger.debug("periodStart: " + periodStart);*/
          moveNextMoteEvent.execute(Mobility.this.simulation.getSimulationTime());
        }
      });

    } catch (Exception e) {
      log.addMessage("Error when loading positions: " + e.getMessage());
      logger.info("Error when loading positions:", e);
      entries = new Move[0];
    }
  }

  private TimeEvent moveNextMoteEvent = new TimeEvent(0) {
    public void execute(long t) {

      /* Detect early events: reschedule for later */
      if (simulation.getSimulationTime() < entries[currentMove].time + periodStart) {
        simulation.scheduleEvent(this, entries[currentMove].time + periodStart);
        return;
      }

      /* Perform a single move */
      Move move = entries[currentMove];
      if (move.moteIndex < simulation.getMotesCount()) {
        Mote mote = simulation.getMote(move.moteIndex);
        Position pos = mote.getInterfaces().getPosition();
        pos.setCoordinates(move.posX, move.posY, pos.getZCoordinate());
        /*logger.info(simulation.getSimulationTimeMillis() + ": Executing " + move);*/
      } else {
        /*log.addMessage(simulation.getSimulationTimeMillis() + ": Bad move, no mote " + move.moteIndex);
        logger.warn(simulation.getSimulationTimeMillis() + ": No such mote, skipping move " + move);*/
      }

      currentMove++;
      if (currentMove >= entries.length) {
        if (!WRAP_MOVES) {
          return;
        }
        /*log.addMessage("New mobility period at " + simulation.getSimulationTime());*/
        /*logger.info("New mobility period at " + simulation.getSimulationTime());*/
        periodStart = simulation.getSimulationTime();
        currentMove = 0;
      }

      /* Reschedule future events */
      simulation.scheduleEvent(this, entries[currentMove].time + periodStart);
    }
  };

  public void closePlugin() {
    moveNextMoteEvent.remove();
  }

  class Move {
    long time;
    int moteIndex;
    double posX, posY;

    public String toString() {
      return "MOVE: mote " + moteIndex + " -> [" + posX + "," + posY + "] @ " + time/Simulation.MILLISECOND;
    }
  }
  
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    if (filePositions != null) {
      element = new Element("positions");
      File file = simulation.getCooja().createPortablePath(filePositions);
      element.setText(file.getPath().replaceAll("\\\\", "/"));
      element.setAttribute("EXPORT", "copy");
      config.add(element);
    }

    return config;
  }
  
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("positions")) {
        filePositions = simulation.getCooja().restorePortablePath(new File(element.getText()));
        loadPositions();
      }
    }

    return true;
  }
}
