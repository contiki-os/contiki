/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

package org.contikios.cooja.radiomediums;

import java.io.PrintWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.FileInputStream;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Random;

import org.apache.log4j.Logger;
import org.jdom.Element;
import org.jdom.output.XMLOutputter;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.RadioConnection;
import org.contikios.cooja.SimEventCentral.MoteCountListener;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.skins.TerrainLOSVisualizerSkin;

/**
 * The Terrain-based Line Of Sight medium uses terrain data from the SRTM 
 * to determine which nodes have line of sight with each other.
 *
 * The model is built on top of UDGM and therefore uses the same transmission, interference,
 * signal strength, and success ratios.
 *
 * TerrainLOS uses a viewshed algorithm developed by Wang, Robinson, and White in 
 * their paper from 2000 in Photogrammetric Engineering and Remote Sensing Volume 66 Issue 1,
 * titled "Generating viewsheds without using sightlines". The algorithm is used when 
 * forming the Directed Graph Medium. Only motes that are within transmission distance
 * and have line of sight are added to the DGRM.
 *
 * @see UDGM
 * @author Sam Mansfield
 */
@ClassDescription("Terrain-based LOS")
public class TerrainLOSMedium extends AbstractRadioMedium {
  private static Logger logger = Logger.getLogger(TerrainLOSMedium.class);
  
  /* Success ratio of TX. If this fails, no radios receive the packet */
  public double SUCCESS_RATIO_TX = 1.0; 
  /* Success ratio of RX. If this fails, 
   * the single affected receiver does not receive the packet */
  public double SUCCESS_RATIO_RX = 1.0; 
  /* Transmission range. */
  public double TRANSMITTING_RANGE = 50; 
  /* Interference range. Ignored if below transmission range. */
  public double INTERFERENCE_RANGE = 100; 
  /* If true, outputs the directed acyclic graph f the network to the working directory. */
  public boolean OUTPUT_DAG = false;
  
  private DirectedGraphMedium dgrm;
  private Random random = null;

  /* By default use 
   * TERRAIN_FILEPATH = "", EAST_WIDTH = 100, SOUTH_WIDTH = 100, EAST_OFFSET = 0, 
   * SOUTH_OFFSET = 0
   */
  public Terrain map = new Terrain("", 100, 100, 0, 0);

  public TerrainLOSMedium(Simulation simulation) {
    super(simulation);
    random = simulation.getRandomGenerator();
    dgrm = new DirectedGraphMedium() {
      protected void analyzeEdges() {
        /* Create edges according to line of sight.
         * XXX May be slow for mobile networks 
         */
        clearEdges();
        for (Radio source: TerrainLOSMedium.this.getRegisteredRadios()) {
          Position sourcePos = source.getPosition();
          /* Caculate the viewshed for the source. */
          map.calculateLOS(
              map.convertLocToCord((int) sourcePos.getXCoordinate()), 
              map.convertLocToCord((int) sourcePos.getYCoordinate()));
          for (Radio dest: TerrainLOSMedium.this.getRegisteredRadios()) {
            Position destPos = dest.getPosition();
            /* Ignore ourselves */
            if (source == dest) {
              continue;
            }
            double distance = sourcePos.getDistanceTo(destPos);
            
            if (distance < Math.max(TRANSMITTING_RANGE, INTERFERENCE_RANGE)) {
              /* Add potential destination if there is line of sight. */
              if(map.isThereLOS(
                    map.convertLocToCord((int) destPos.getXCoordinate()), 
                    map.convertLocToCord((int) destPos.getYCoordinate()))) {
                addEdge(
                    new DirectedGraphMedium.Edge(source, 
                        new DGRMDestinationRadio(dest)));
              }
            }
            
          }
        }
        super.analyzeEdges();

        if (OUTPUT_DAG) {
          try {
            XMLOutputter output = new XMLOutputter();
            /* Write DAG to working directory (cooja/build). */
            PrintWriter pr = new PrintWriter("dag.xml");
            pr.write("<root>\n");
            for (Element e: dgrm.getConfigXML()) {
              pr.write(output.outputString(e) + "\n"); 
            }
            pr.write("</root>\n");
            pr.close();
          }
          catch (FileNotFoundException e) {
            logger.error("FileNotFoundException: " + e.getMessage());
          }
          catch (SecurityException e) {
            logger.error("SecurityException: " + e.getMessage());
          }
        }
      }
    };

    /* Register as position observer.
     * If any positions change, re-analyze potential receivers. 
     */
    final Observer positionObserver = new Observer() {
      public void update(Observable o, Object arg) {
        dgrm.requestEdgeAnalysis();
      }
    };
    /* Re-analyze potential receivers if radios are added/removed. */
    simulation.getEventCentral().addMoteCountListener(new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        mote.getInterfaces().getPosition().addObserver(positionObserver);
        dgrm.requestEdgeAnalysis();
      }
      public void moteWasRemoved(Mote mote) {
        mote.getInterfaces().getPosition().deleteObserver(positionObserver);
        dgrm.requestEdgeAnalysis();
      }
    });
    for (Mote mote: simulation.getMotes()) {
      mote.getInterfaces().getPosition().addObserver(positionObserver);
    }
    dgrm.requestEdgeAnalysis();

    /* Register visualizer skin */
    Visualizer.registerVisualizerSkin(TerrainLOSVisualizerSkin.class);
  }

  public void removed() {
  	super.removed();
  	
		Visualizer.unregisterVisualizerSkin(TerrainLOSVisualizerSkin.class);
  }
  
  public void setTxRange(double r) {
    TRANSMITTING_RANGE = r;
    dgrm.requestEdgeAnalysis();
  }

  public void setInterferenceRange(double r) {
    INTERFERENCE_RANGE = r;
    dgrm.requestEdgeAnalysis();
  }

  public void setTerrainFilepath(String fp) {
    map.TERRAIN_FILEPATH = fp;
    map.configureMap();
  }
  
  public void setEastWidth(int w) {
    map.EAST_WIDTH = w;
    map.configureMap();
  }

  public void setSouthWidth(int w) {
    map.SOUTH_WIDTH = w;
    map.configureMap();
  }

  public void setEastOffset(int o) {
    map.EAST_OFFSET = o;
    map.configureMap();
  }

  public void setSouthOffset(int o) {
    map.SOUTH_OFFSET = o;
    map.configureMap();
  }

  public void setOutputDAG(boolean d) {
    OUTPUT_DAG = d;
  }

  public RadioConnection createConnections(Radio sender) {
    RadioConnection newConnection = new RadioConnection(sender);

    /* Fail radio transmission randomly - no radios will hear this transmission */
    if (getTxSuccessProbability(sender) < 1.0 && 
        random.nextDouble() > getTxSuccessProbability(sender)) {
      return newConnection;
    }

    /* Calculate ranges: grows with radio output power */
    double moteTransmissionRange = TRANSMITTING_RANGE
    * ((double) sender.getCurrentOutputPowerIndicator() / 
      (double) sender.getOutputPowerIndicatorMax());
    double moteInterferenceRange = INTERFERENCE_RANGE
    * ((double) sender.getCurrentOutputPowerIndicator() / 
      (double) sender.getOutputPowerIndicatorMax());

    /* Get all potential destination radios */
    DestinationRadio[] potentialDestinations = dgrm.getPotentialDestinations(sender);
    if (potentialDestinations == null) {
      return newConnection;
    }

    /* Loop through all potential destinations */
    Position senderPos = sender.getPosition();
    for (DestinationRadio dest: potentialDestinations) {
      Radio recv = dest.radio;

      /* Fail if radios are on different (but configured) channels */ 
      if (sender.getChannel() >= 0 &&
          recv.getChannel() >= 0 &&
          sender.getChannel() != recv.getChannel()) {

        /* Add the connection in a dormant state;
         * it will be activated later when the radio will be
         * turned on and switched to the right channel. This behavior
         * is consistent with the case when receiver is turned off. 
         */
        newConnection.addInterfered(recv);

        continue;
      }
      Position recvPos = recv.getPosition();
      
      /* Remnant of code from UDGM */
      /* Fail if radio is turned off */
//      if (!recv.isReceiverOn()) {
//        /* Special case: allow connection if source is Contiki radio, 
//         * and destination is something else (byte radio).
//         * Allows cross-level communication with power-saving MACs. */
//        if (sender instanceof ContikiRadio &&
//            !(recv instanceof ContikiRadio)) {
//          /*logger.info("Special case: creating connection to turned off radio");*/
//        } else {
//          recv.interfereAnyReception();
//          continue;
//        }
//      }

      double distance = senderPos.getDistanceTo(recvPos);
      if (distance <= moteTransmissionRange) {
        /* Within transmission range */

        if (!recv.isRadioOn()) {
          newConnection.addInterfered(recv);
          recv.interfereAnyReception();
        } else if (recv.isInterfered()) {
          /* Was interfered: keep interfering */
          newConnection.addInterfered(recv);
        } else if (recv.isTransmitting()) {
          newConnection.addInterfered(recv);
        } else if (recv.isReceiving() ||
            (random.nextDouble() > getRxSuccessProbability(sender, recv))) {
          /* Was receiving, or reception failed: start interfering */
          newConnection.addInterfered(recv);
          recv.interfereAnyReception();

          /* Interfere receiver in all other active radio connections */
          for (RadioConnection conn : getActiveConnections()) {
            if (conn.isDestination(recv)) {
              conn.addInterfered(recv);
            }
          }

        } else {
          /* Success: radio starts receiving */
          newConnection.addDestination(recv);
        }
      } else if (distance <= moteInterferenceRange) {
        /* Within interference range */
        newConnection.addInterfered(recv);
        recv.interfereAnyReception();
      }
    }

    return newConnection;
  }
  
  public double getSuccessProbability(Radio source, Radio dest) {
  	return getTxSuccessProbability(source) * getRxSuccessProbability(source, dest);
  }
  public double getTxSuccessProbability(Radio source) {
    return SUCCESS_RATIO_TX;
  }
  public double getRxSuccessProbability(Radio source, Radio dest) {
    /* Get all potential destination radios */
    DestinationRadio[] potentialDestinations = 
      dgrm.getPotentialDestinations(source);
    if (potentialDestinations == null) {
      return 0.0;
    }

    /* Loop through all potential destinations. 
     * If dest is a potential destination calculate the percentage.
     */
    boolean validDest = false;
    for (DestinationRadio potentialDest: potentialDestinations) {
      if(potentialDest.radio == dest) {
        validDest = true; 
      } 
    }

    if(!validDest) {
      return 0.0;
    }

    double distance = source.getPosition().getDistanceTo(dest.getPosition());
    double distanceSquared = Math.pow(distance,2.0);
    double distanceMax = TRANSMITTING_RANGE * 
      ((double) source.getCurrentOutputPowerIndicator() / 
      (double) source.getOutputPowerIndicatorMax());
    if (distanceMax == 0.0) {
      return 0.0;
    }
    double distanceMaxSquared = Math.pow(distanceMax,2.0);
    double ratio = distanceSquared / distanceMaxSquared;
    if (ratio > 1.0) {
    	return 0.0;
    }
    return 1.0 - ratio*(1.0-SUCCESS_RATIO_RX); 
  }

  public void updateSignalStrengths() {
    /* Override: uses distance as signal strength factor */
    
    /* Reset signal strengths */
    for (Radio radio : getRegisteredRadios()) {
      radio.setCurrentSignalStrength(getBaseRssi(radio));
    }

    /* Set signal strength to below strong on destinations */
    RadioConnection[] conns = getActiveConnections();
    for (RadioConnection conn : conns) {
      if (conn.getSource().getCurrentSignalStrength() < SS_STRONG) {
        conn.getSource().setCurrentSignalStrength(SS_STRONG);
      }
      for (Radio dstRadio : conn.getDestinations()) {
        if (conn.getSource().getChannel() >= 0 &&
            dstRadio.getChannel() >= 0 &&
            conn.getSource().getChannel() != dstRadio.getChannel()) {
          continue;
        }

        double dist = conn.getSource().getPosition().getDistanceTo(dstRadio.getPosition());

        double maxTxDist = TRANSMITTING_RANGE
          * ((double) conn.getSource().getCurrentOutputPowerIndicator() / 
          (double) conn.getSource().getOutputPowerIndicatorMax());
        double distFactor = dist/maxTxDist;

        double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
        if (dstRadio.getCurrentSignalStrength() < signalStrength) {
          dstRadio.setCurrentSignalStrength(signalStrength);
        }
      }
    }

    /* Set signal strength to below weak on interfered */
    for (RadioConnection conn : conns) {
      for (Radio intfRadio : conn.getInterfered()) {
        if (conn.getSource().getChannel() >= 0 &&
            intfRadio.getChannel() >= 0 &&
            conn.getSource().getChannel() != intfRadio.getChannel()) {
          continue;
        }

        double dist = conn.getSource().getPosition().getDistanceTo(intfRadio.getPosition());

        double maxTxDist = TRANSMITTING_RANGE
          * ((double) conn.getSource().getCurrentOutputPowerIndicator() / 
          (double) conn.getSource().getOutputPowerIndicatorMax());
        double distFactor = dist/maxTxDist;

        if (distFactor < 1) {
          double signalStrength = SS_STRONG + distFactor*(SS_WEAK - SS_STRONG);
          if (intfRadio.getCurrentSignalStrength() < signalStrength) {
            intfRadio.setCurrentSignalStrength(signalStrength);
          }
        } else {
          intfRadio.setCurrentSignalStrength(SS_WEAK);
          if (intfRadio.getCurrentSignalStrength() < SS_WEAK) {
            intfRadio.setCurrentSignalStrength(SS_WEAK);
          }
        }

        if (!intfRadio.isInterfered()) {
          /*logger.warn("Radio was not interfered: " + intfRadio);*/
          intfRadio.interfereAnyReception();
        }
      }
    }
  }

  public Collection<Element> getConfigXML() {
    Collection<Element> config = super.getConfigXML();
    Element element;

    /* Transmitting range */
    element = new Element("transmitting_range");
    element.setText(Double.toString(TRANSMITTING_RANGE));
    config.add(element);

    /* Interference range */
    element = new Element("interference_range");
    element.setText(Double.toString(INTERFERENCE_RANGE));
    config.add(element);

    /* Transmission success probability */
    element = new Element("success_ratio_tx");
    element.setText("" + SUCCESS_RATIO_TX);
    config.add(element);

    /* Reception success probability */
    element = new Element("success_ratio_rx");
    element.setText("" + SUCCESS_RATIO_RX);
    config.add(element);
    
    /* Terrain filepath */
    element = new Element("terrain_filepath");
    element.setText("" + map.TERRAIN_FILEPATH);
    config.add(element);

    /* Border width */
    element = new Element("east_width");
    element.setText("" + map.EAST_WIDTH);
    config.add(element);

    /* Border height */
    element = new Element("south_width");
    element.setText("" + map.SOUTH_WIDTH);
    config.add(element);

    /* Border width offset */
    element = new Element("east_offset");
    element.setText("" + map.EAST_OFFSET);
    config.add(element);

    /* Border height offset */
    element = new Element("south_offset");
    element.setText("" + map.SOUTH_OFFSET); 
    config.add(element);
    
    /* If medium is outputting the DAG. */
    element = new Element("output_dag");
    element.setText(Boolean.toString(OUTPUT_DAG)); 
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    super.setConfigXML(configXML, visAvailable);
    for (Element element : configXML) {
      if (element.getName().equals("transmitting_range")) {
        TRANSMITTING_RANGE = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("interference_range")) {
        INTERFERENCE_RANGE = Double.parseDouble(element.getText());
      }

      /* Backwards compatibility */
      if (element.getName().equals("success_ratio")) {
        SUCCESS_RATIO_TX = Double.parseDouble(element.getText());
        logger.warn("Loading old Cooja Config, XML element \"sucess_ratio\" parsed at \"sucess_ratio_tx\"");
      }

      if (element.getName().equals("success_ratio_tx")) {
        SUCCESS_RATIO_TX = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("success_ratio_rx")) {
        SUCCESS_RATIO_RX = Double.parseDouble(element.getText());
      }
      
      if(element.getName().equals("terrain_filepath")) {
        map.TERRAIN_FILEPATH = element.getText();
      }

      if(element.getName().equals("east_width")) {
        map.EAST_WIDTH = Integer.parseInt(element.getText());
      }

      if(element.getName().equals("south_width")) {
        map.SOUTH_WIDTH = Integer.parseInt(element.getText());
      }

      if(element.getName().equals("east_offset")) {
        map.EAST_OFFSET = Integer.parseInt(element.getText());
      }

      if(element.getName().equals("south_offset")) {
        map.SOUTH_OFFSET = Integer.parseInt(element.getText());
      }
      
      if(element.getName().equals("output_dag")) {
        OUTPUT_DAG = Boolean.parseBoolean(element.getText());
      }

      map.configureMap(); 
      dgrm.requestEdgeAnalysis();
    }
    return true;
  }

  public class Terrain {
    /* The heights of the current map. */
    public double h[][];
    /* The source location. */
    private int srcX = -1;
    private int srcY = -1;
    /* The calculated viewshed based on srcX and srcY. */
    public double los[][];
    /* Used to determine color gradient. */
    public double min = 32768.0;
    public double max = -1.0;
    /* The width and height of an hgt file. Although technically it is 3601, but
     * the extra byte is dupblicated in adjacent tiles.
     */
    private int HGTWIDTH = 3600;
    private int HGTHEIGHT = 3600;
  
    /* The filepath of the terrain file. If empty a well terrain will be
     * used instead. 
     */
    public String TERRAIN_FILEPATH = "";
    /* The width (degrees) extending to the East TerrainLOS 
     * uses of the terrain file. 
     */
    public int EAST_WIDTH = 100;
    /* The width (degrees) extending to the South TerrainLOS 
     * uses of the terrain file. 
     */
    public int SOUTH_WIDTH = 100;
    /* The offset (degrees( to the East TerrainLOS will start 
     * reading the terrain file. 
     */
    public int EAST_OFFSET = 0;
    /* The offset (degrees) to the South TerrainLOS will start 
     * reading the terrain file. 
     */
    public int SOUTH_OFFSET = 0;
  
    /* A new terrain takes in a filepath, East width, South width,
     * East offset, and South Offset. If filepath is "" then a synthetic
     * well terrain is used instead.
     */
    public Terrain(String fp, int ew, int sw, int eo, int so) {
      TERRAIN_FILEPATH = fp;
      EAST_WIDTH = ew;
      SOUTH_WIDTH = sw;
      EAST_OFFSET = eo;
      SOUTH_OFFSET = so;

      configureMap();
    }
   
    /* Configures the map using the current set configurations */
    public void configureMap() {
      /* Error Checking */
      if(EAST_WIDTH < 0 || SOUTH_WIDTH < 0 || 
         EAST_OFFSET < 0 || SOUTH_OFFSET < 0) {
        logger.error("All widths and offsets must be greater than or equal to 0" +
                     " EAST_WIDTH: " + EAST_WIDTH + "SOUTH_WIDTH: " + SOUTH_WIDTH +  
                     "\nEAST_OFFSET: " + EAST_OFFSET + " SOUTH_OFFSET: " + SOUTH_OFFSET);
      }
      if((EAST_WIDTH + EAST_OFFSET) > HGTWIDTH) {
        logger.error("East width plus East offset must be less than 3600" +
                     "\nEAST_WIDTH: " + EAST_WIDTH + " EAST_OFFSET: " + EAST_OFFSET);
      }
      if((SOUTH_WIDTH + SOUTH_OFFSET) > HGTHEIGHT) {
        logger.error("South width plus South offset must be less than 3600" +
                     "\nSOUTH_WIDTH: " + SOUTH_WIDTH + " SOUTH_OFFSET: " + SOUTH_OFFSET);
      }
     
      /* TODO: Why plus 1? */
      h = new double[EAST_WIDTH][SOUTH_WIDTH];
      los = new double[EAST_WIDTH][SOUTH_WIDTH];
      
      byte buffer;
      /* Set these as the lowest respective possible values. Each height is a signed
       * two byte value, so the possbile range is -32767 to 32767. TerrainLOS currently
       * only supports positive heights so the range is 0 to 32767. */
      min = 32768.0;
      max = -1.0;
      
      if(TERRAIN_FILEPATH != "") {
        try {
          FileInputStream inputStream = new FileInputStream(TERRAIN_FILEPATH);
  
          int skip_rows = 3601 - SOUTH_WIDTH - SOUTH_OFFSET;
          int skip_cols = 3601 - EAST_WIDTH - EAST_OFFSET;
          int msb;
          int lsb;
          int xi;
          int yi;
          double elevation;
  
          inputStream.skip(SOUTH_OFFSET*2*3601);
          yi = 0;
          for(int mapY = SOUTH_OFFSET; mapY < SOUTH_WIDTH + SOUTH_OFFSET; mapY++) {
            inputStream.skip(EAST_OFFSET*2);
            xi = 0;
            for(int mapX = EAST_OFFSET; mapX < EAST_WIDTH + EAST_OFFSET; mapX++) { 
              msb = inputStream.read();
              lsb = inputStream.read();
              /* By default each byte is treated as signed, so convert back to the
               * unisnged value if negative. 
               */
              if(msb < 0) msb += 256;
              if(lsb < 0) msb += 256;
              elevation = twosComp((msb << 8) + lsb, 16);
              if(elevation > 0) {
                h[xi][yi] = elevation;
              }
              else {
                elevation = 0;
              }
              if(max < ((double) elevation)) {
                max = (double) elevation; 
              }
              if(min > ((double) elevation)) {
                min = (double) elevation;
              }
              xi++;
            }
            inputStream.skip(skip_cols*2);
            yi++;
          }
          inputStream.skip(skip_rows*2*3601);
            
          int eof = inputStream.read();
          if(eof != -1) {
            logger.error("Error: Bad format for SRTM"); 
          }
  
          inputStream.close();
        }
        catch(FileNotFoundException ex) {
          logger.error(
              "Unable to open file '" + 
              TERRAIN_FILEPATH + "'");
        }
        catch(IOException ex) {
          logger.error(
              "Error reading file '" +
              TERRAIN_FILEPATH + "'");
        }
      }
      /* Well terrain */
      else {
        max = 1000.0;
        min = 0.0;
        for(int i = 0; i < EAST_WIDTH; i++) {
          for(int j = 0; j < SOUTH_WIDTH; j++) {
	    if(i > EAST_WIDTH/6 && i < (EAST_WIDTH - EAST_WIDTH/6) &&
               j > SOUTH_WIDTH/6 && j < (SOUTH_WIDTH - SOUTH_WIDTH/6)) {
              h[i][j] = 0.0;
            }
            else {
              h[i][j] = 1000.0;
            }
          }
        }
      } 
    }

    private int twosComp(int val, int bits) {
      if((val & (1 << (bits - 1))) != 0) {
        val = val - (1 << bits);
      }
      return val;
    }
  
    private boolean inBounds(int x, int y) {
      /* Check inputs */
      if(x < 0 || y < 0)
        return false;
      else if(x >= EAST_WIDTH || y >= SOUTH_WIDTH)
        return false;
      /* Check source, it is possible that x and y
       * are also the source, but it doesn't hurt to 
       * check twice.
      */
      else if(srcX < 0 || srcY < 0)
        return false;
      else if(srcX >= EAST_WIDTH || srcY >= SOUTH_WIDTH)
        return false;
      else 
        return true;
    }
  
    public double getHeight(int x, int y) {
      return h[x][y];
    }
  
    public int getSrcX() {
      return srcX;
    }
    
    public int getSrcY() {
      return srcY;
    }
  
    /* Calculates line of sight at position (x0, y0)
     * where x0 amd y0 are coordinates, not locations
     * Must be called after constructor
    */
    public void calculateLOS(int x0, int y0) {
  
      srcX = x0;
      srcY = y0;
      
      double x0f = (double) srcX;
      double y0f = (double) srcY;
     
      /* If the source is not in bounds exit
       * the default behavior is that motes outside the range
       * cannot talk to each other.
      */
      if(!inBounds(x0, y0))
        return;
      
      /* Wang, Robinson, and White's Algorithm for finding line of sight */
      double z0 = h[x0][y0];
      /* Put source on a tripod
       * z0 += 5;
       */
  
      int x1;
      int y1;
      int x2;
      int y2;
      double z1;
      double z2;
      double xf;
      double yf;
      double visible = 0;
      double actual = 0;
      
      for(int x = x0 - 1; (x >=0) && (x < EAST_WIDTH) && x <= x0 + 1; x++) {
        for(int y = y0 - 1; (y >= 0) && (y < SOUTH_WIDTH) && y <= y0 + 1; y++) {
          los[x][y] = h[x][y];
        }
      }
      
      /* Divide into 8 octets and 8 axes */
      /* E */
      for(int x = x0 + 2; x < EAST_WIDTH; x++) {
        x1 = x - 1;
        z1 = los[x1][y0];
        xf = (double) x;
        visible = (z1 - z0)*((xf - x0f)/(xf - x0f - 1)) + z0;
        actual = h[x][y0];
        los[x][y0] = (visible > actual) ? visible : actual;
      }
      /* S */
      for(int y = y0 + 2; y < SOUTH_WIDTH; y++) {
        y1 = y - 1;
        z1 = los[x0][y1];
        yf = (double) y;
        visible = (z1 - z0)*((yf - y0f)/(yf - y0f - 1)) + z0;
        actual = h[x0][y];
        los[x0][y] = (visible > actual) ? visible : actual;
      }
      /* W */
      for(int x = x0 - 2; x >= 0; x--) {
        x1 = x + 1;
        z1 = los[x1][y0];
        xf = (double) x;
        visible = (z1 - z0)*((x0f - xf)/(x0f - xf - 1)) + z0;
        actual = h[x][y0];
        los[x][y0] = (visible > actual) ? visible : actual;
      }
      /* N */
      for(int y = y0 - 2; y >= 0; y--) {
        y1 = y + 1;
        z1 = los[x0][y1];
        yf = (double) y;
        visible = (z1 - z0)*((y0f - yf)/(y0f - yf - 1)) + z0;
        actual = h[x0][y];
        los[x0][y] = (visible > actual) ? visible : actual;
      }
      /* SE */
      for(int x = x0 + 2, y = y0 + 2; x < EAST_WIDTH && y < SOUTH_WIDTH;
          x++, y++) {
        x1 = x - 1;
        y1 = y - 1;
        z1 = los[x1][y1];
        xf = (double) x;
        visible = (z1 - z0)*((xf - x0f)/(xf - x0f - 1)) + z0;
        actual = h[x][y];
        los[x][y] = (visible > actual) ? visible : actual;
      }
      /* SW */
      for(int x = x0 - 2, y = y0 + 2; x >= 0 && y < SOUTH_WIDTH;
          x--, y++) {
        x1 = x + 1;
        y1 = y - 1;
        z1 = los[x1][y1];
        xf = (double) x;
        visible = (z1 - z0)*((x0f - xf)/(x0f - xf - 1)) + z0;
        actual = h[x][y];
        los[x][y] = (visible > actual) ? visible : actual;
      }
      /* NW */
      for(int x = x0 - 2, y = y0 - 2; x >= 0 && y >= 0;
          x--, y--) {
        x1 = x + 1;
        y1 = y + 1;
        z1 = los[x1][y1];
        xf = (double) x;
        visible = (z1 - z0)*((x0f - xf)/(x0f - xf - 1)) + z0;
        actual = h[x][y];
        los[x][y] = (visible > actual) ? visible : actual;
      }
      /* NE */
      for(int x = x0 + 2, y = y0 - 2; x < EAST_WIDTH && y >= 0;
          x++, y--) {
        x1 = x - 1;
        y1 = y + 1;
        z1 = los[x1][y1];
        xf = (double) x;
        visible = (z1 - z0)*((xf - x0f)/(xf - x0f - 1)) + z0;
        actual = h[x][y];
        los[x][y] = (visible > actual) ? visible : actual;
      }
      /* E-SE */
      for(int x = x0 + 2; x < EAST_WIDTH; x++) {
        for(int y = y0 + 1; ((y - y0) < (x - x0)) && (y < SOUTH_WIDTH); y++) {
          x1 = x - 1;
          y1 = y - 1;
          z1 = los[x1][y1];
          x2 = x - 1;
          y2 = y;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*((yf - y0f)/(xf - x0f - 1)) 
                    + (z2 - z0)*((xf - x0f - (yf - y0f))/(xf - x0f - 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* S-SE */
      for(int y = y0 + 2; y < SOUTH_WIDTH; y++) {
        for(int x = x0 + 1; ((x - x0) < (y - y0)) && x < EAST_WIDTH; x++) {
          x1 = x - 1;
          y1 = y - 1;
          z1 = los[x1][y1];
          x2 = x;
          y2 = y - 1;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*((xf - x0f)/(yf - y0f - 1)) 
                    + (z2 - z0)*((yf - y0f - (xf - x0f))/(yf - y0f - 1)) + z0; 
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* S-SW */
      for(int y = y0 + 2; y < SOUTH_WIDTH; y++) {
        for(int x = x0 - 1; ((x0 - x) < (y - y0)) && (x >= 0); x--) {
          x1 = x + 1;
          y1 = y - 1;
          z1 = los[x1][y1];
          x2 = x;
          y2 = y - 1;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*(-(xf - x0f)/(yf - y0f - 1)) 
                    + (z2 - z0)*((xf - x0f + yf - y0f)/(yf - y0f - 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* W-SW */
      for(int x = x0 - 2; x >= 0; x--) {
        for(int y = y0 + 1; ((y - y0) < (x0 - x)) && y < SOUTH_WIDTH; y++) {
          x1 = x + 1;
          y1 = y - 1;
          z1 = los[x1][y1];
          x2 = x + 1;
          y2 = y;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*(-(yf - y0f)/(xf - x0f + 1))
                    + (z2 - z0)*((xf - x0f + yf - y0f)/(xf - x0f + 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* W-NW */
      for(int x = x0 - 2; x >= 0; x--) {
        for(int y = y0 - 1; ((y0 - y) < (x0 - x)) && y >= 0; y--) {
          x1 = x + 1;
          y1 = y + 1;
          z1 = los[x1][y1];
          x2 = x + 1;
          y2 = y;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*((yf - y0f)/(xf - x0f + 1))
                    + (z2 - z0)*((xf - x0f - (yf - y0f))/(xf - x0f + 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* N-NW */
      for(int y = y0 - 2; y >= 0; y--) {
        for(int x = x0 - 1; ((x0 - x) < (y0 - y)) && (x >= 0); x--) {
          x1 = x + 1;
          y1 = y + 1;
          z1 = los[x1][y1];
          x2 = x;
          y2 = y + 1;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*((xf - x0f)/(yf - y0f + 1))
                    + (z2 - z0)*((yf - y0f - (xf - x0f))/(yf - y0f + 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* N-NE */
      for(int y = y0 - 2; y >= 0; y--) {
        for(int x = x0 + 1; (x - x0 < y0 - y) && (x < EAST_WIDTH); x++) {
          x1 = x - 1;
          y1 = y + 1;
          z1 = los[x1][y1];
          x2 = x;
          y2 = y + 1;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*(-(xf - x0f)/(yf - y0f + 1))
                    + (z2 - z0)*((xf - x0f + yf - y0f)/(yf - y0f + 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
      /* E-NE */
      for(int x = x0 + 2; x < EAST_WIDTH; x++) {
        for(int y = y0 - 1; (y0 - y < x - x0) && (y >= 0); y--) {
          x1 = x - 1;
          y1 = y + 1;
          z1 = los[x1][y1];
          x2 = x - 1;
          y2 = y;
          z2 = los[x2][y2];
          xf = (double) x;
          yf = (double) y;
          visible = (z1 - z0)*(-(yf - y0f)/(xf - x0f - 1))
                    + (z2 - z0)*((xf - x0f + yf - y0f)/(xf - x0f - 1)) + z0;
          actual = h[x][y];
          los[x][y] = (visible > actual) ? visible : actual;
        }
      }
    }
  
    public int convertLocToCord(int loc) {
      int cord;
  
      cord = loc / 33;
  
      return cord;
    }
  
    /* Using the srcX and srcY location determines whether the giben destination
     * is in LOS, where destX and destY are coordinates
    */
    public boolean isThereLOS(int dst_x, int dst_y) {
    
      /* If the destination or source is not in bounds
       * then the default behavour is that motes cannot 
       * talk to each other.
      */
      if(!inBounds(dst_x, dst_y))
        return false;
  
      if(h[dst_x][dst_y] >= los[dst_x][dst_y])
        return true;
      else
        return false;
    }
   
  }

}
