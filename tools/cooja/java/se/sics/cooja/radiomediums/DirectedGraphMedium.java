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

package se.sics.cooja.radiomediums;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;
import java.util.Random;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.DGRMConfigurator;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.skins.DGRMVisualizerSkin;

/**
 * Directed Graph Radio Medium.
 * 
 * Can be used both stand-alone as a radio medium, and 
 * as a basis for other radio medium implementations.
 * 
 * The stand-alone radio medium supports propagation delays and
 * and per-link transmission success ratio/RSSI.
 * 
 * @see AbstractRadioMedium
 * @author Fredrik Osterlind
 */
@ClassDescription("Directed Graph Radio Medium (DGRM)")
public class DirectedGraphMedium extends AbstractRadioMedium {
  private static Logger logger = Logger.getLogger(DirectedGraphMedium.class);

  private Simulation simulation;
  private Random random;

  private ArrayList<Edge> edges = new ArrayList<Edge>();
  private boolean edgesDirty = true;

  /* Used for optimizing lookup time */
  private Hashtable<Radio,DGRMDestinationRadio[]> edgesTable = new Hashtable<Radio,DGRMDestinationRadio[]>();

  public DirectedGraphMedium() {
    /* Do not initialize radio medium: use only for hash table */
    super(null);
    Visualizer.registerVisualizerSkin(DGRMVisualizerSkin.class);
  }

  public DirectedGraphMedium(Simulation simulation) {
    super(simulation);
    this.simulation = simulation;
    random = simulation.getRandomGenerator();

    requestEdgeAnalysis();

    /* Register plugin and visualizer skin */
    simulation.getGUI().registerPlugin(DGRMConfigurator.class);
    Visualizer.registerVisualizerSkin(DGRMVisualizerSkin.class);
  }

  public void removed() {
    super.removed();

    /* Unregister plugin and visualizer skin */
    simulation.getGUI().unregisterPlugin(DGRMConfigurator.class);
    Visualizer.unregisterVisualizerSkin(DGRMVisualizerSkin.class);
  }

  public void addEdge(Edge e) {
    edges.add(e);
    requestEdgeAnalysis();

    ((AbstractRadioMedium.RadioMediumObservable)
        this.getRadioMediumObservable()).setRadioMediumChangedAndNotify();
  }

  public void removeEdge(Edge edge) {
    if (!edges.contains(edge)) {
      logger.fatal("Cannot remove edge: " + edge);
      return;
    }
    edges.remove(edge);
    requestEdgeAnalysis();

    ((AbstractRadioMedium.RadioMediumObservable)
        this.getRadioMediumObservable()).setRadioMediumChangedAndNotify();
  }

  public void clearEdges() {
    edges.clear();
    requestEdgeAnalysis();

    ((AbstractRadioMedium.RadioMediumObservable)
        this.getRadioMediumObservable()).setRadioMediumChangedAndNotify();
  }

  public Edge[] getEdges() {
    return edges.toArray(new Edge[0]);
  }

  /**
   * Signal that the configuration changed, and needs to be re-analyzed
   * before used.
   */
  public void requestEdgeAnalysis() {
    edgesDirty = true;
  }

  public boolean needsEdgeAnalysis() {
    return edgesDirty;
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    super.unregisterRadioInterface(radio, sim);

    for (Edge edge: getEdges()) {
      if (edge.source == radio || edge.superDest.radio == radio) {
        removeEdge(edge);
        requestEdgeAnalysis();
      }
    }
  }

  public void updateSignalStrengths() {

    /* Reset signal strengths */
    for (Radio radio : getRegisteredRadios()) {
      radio.setCurrentSignalStrength(SS_NOTHING);
    }

    /* Set signal strengths */
    RadioConnection[] conns = getActiveConnections();
    for (RadioConnection conn : conns) {
      /* When sending RSSI is Strong!
       * TODO: is this reasonable
       */
      if (conn.getSource().getCurrentSignalStrength() < SS_STRONG) {
        conn.getSource().setCurrentSignalStrength(SS_STRONG);
      }
      //Maximum reception signal of all possible radios received
      DGRMDestinationRadio dstRadios[] =  getPotentialDestinations(conn.getSource());
      if (dstRadios == null) continue; 
      for (DGRMDestinationRadio dstRadio : dstRadios) {
        if (dstRadio.radio.getCurrentSignalStrength() < dstRadio.signal) {
          dstRadio.radio.setCurrentSignalStrength(dstRadio.signal);
        }
        /* We can set this without further checks, as it will only be read
         * if a packet is actually received. In that case it is set to the
         * correct value */
        dstRadio.radio.setLQI(dstRadio.lqi);
      }
    
      
    } 
  }


  /**
   * Generates hash table using current edges for efficient lookup.
   */
  protected void analyzeEdges() {
    Hashtable<Radio,ArrayList<DGRMDestinationRadio>> listTable =
      new Hashtable<Radio,ArrayList<DGRMDestinationRadio>>();

    /* Fill edge hash table with all edges */
    for (Edge edge: getEdges()) {
      ArrayList<DGRMDestinationRadio> destRadios;
      if (!listTable.containsKey(edge.source)) {
        destRadios = new ArrayList<DGRMDestinationRadio>();
      } else {
        destRadios = listTable.get(edge.source);
      }

      destRadios.add(edge.superDest);
      listTable.put(edge.source, destRadios);
    }

    /* Convert to arrays */
    Hashtable<Radio,DGRMDestinationRadio[]> arrTable =  new Hashtable<Radio,DGRMDestinationRadio[]>();
    Enumeration<Radio> sources = listTable.keys();
    while (sources.hasMoreElements()) {
      Radio source = sources.nextElement();
      DGRMDestinationRadio[] arr = listTable.get(source).toArray(new DGRMDestinationRadio[0]);
      arrTable.put(source, arr);
    }

    this.edgesTable = arrTable;
    edgesDirty = false;
  }

  /**
   * Returns all potential destination radios, i.e. all radios "within reach".
   * Does not consider radio channels, transmission success ratios etc.
   *  
   * @param source Source radio
   * @return All potential destination radios
   */
  public DGRMDestinationRadio[] getPotentialDestinations(Radio source) {
    if (edgesDirty) {
      analyzeEdges();
    }
    return edgesTable.get(source);
  }

  public RadioConnection createConnections(Radio source) {
    if (edgesDirty) {
      analyzeEdges();
    }
    if (edgesDirty) {
      logger.fatal("Error when analyzing edges, aborting new radio connection");
      return new RadioConnection(source);
    }

    /* Create new radio connection using edge hash table */
    RadioConnection newConn = new RadioConnection(source);
    DGRMDestinationRadio[] destinations = getPotentialDestinations(source);
    if (destinations == null || destinations.length == 0) {
      /* No destinations */
      /*logger.info(sendingRadio + ": No dest");*/
      return newConn;
    }

    /*logger.info(source + ": " + destinations.length + " potential destinations");*/
    for (DGRMDestinationRadio dest: destinations) {
      
      if (dest.radio == source) {
        /* Fail: cannot receive our own transmission */
        /*logger.info(source + ": Fail, receiver is sender");*/
        continue;
      }


      if (!dest.radio.isRadioOn()) {
        /* Fail: radio is off */
        /*logger.info(source + ": Fail, off");*/
        newConn.addInterfered(dest.radio);
        continue;
      }
      
      if (dest.radio.isInterfered()) {
        /* Fail: radio is interfered in another connection */
        /*logger.info(source + ": Fail, interfered");*/
        newConn.addInterfered(dest.radio);
        continue;
      }

      int srcc = source.getChannel();
      int dstc = dest.radio.getChannel(); 
      if ( srcc >= 0 && dstc >= 0 && srcc != dstc) {
    	/* Fail: radios are on different (but configured) channels */
        continue;
      }
      
      if (dest.radio.isReceiving()) {
         /* Fail: radio is already actively receiving */
         /*logger.info(source + ": Fail, receiving");*/
         newConn.addInterfered(dest.radio);

         /* We will also interfere with the other connection */
         dest.radio.interfereAnyReception();
         
         // Find connection, that is sending to that radio
         // and mark the destination as interfered
         for (RadioConnection conn : getActiveConnections()) {
           for (Radio dstRadio : conn.getDestinations()) {
             if (dstRadio == dest.radio) {
               conn.addInterfered(dest.radio);;
               break;
             }
           }
         }        
         continue;
      }
            
      if (dest.ratio < 1.0 && random.nextDouble() > dest.ratio) {
    	/* Fail: Reception ratio */
        /*logger.info(source + ": Fail, randomly");*/
        newConn.addInterfered(dest.radio);
        continue;
      }

      /* Success: radio starts receiving */
      /*logger.info(source + ": OK: " + dest.radio);*/
      newConn.addDestination(dest.radio, dest.delay);
    }

    return newConn;
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    for (Edge edge: getEdges()) {
      element = new Element("edge");
      element.addContent(edge.getConfigXML());
      config.add(element);
    }

    return config;
  }

  private Collection<Element> delayedConfiguration = null;
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    random = simulation.getRandomGenerator();

    /* Wait until simulation has been loaded */
    delayedConfiguration = configXML;
    return true;
  }
  
public void simulationFinishedLoading() {
    if (delayedConfiguration == null) {
      return;
    }

    boolean oldConfig = false;
    for (Element element : delayedConfiguration) {
      if (element.getName().equals("edge")) {
        @SuppressWarnings("unchecked")
		Collection<Element> edgeConfig = element.getChildren();
        Radio source = null;
        DGRMDestinationRadio dest = null;
        for (Element edgeElement : edgeConfig) {
          if (edgeElement.getName().equals("src")) {
            oldConfig = true;

            /* Old config: lookup source mote */
            for (Mote m: simulation.getMotes()) {
              if (m.toString().equals(edgeElement.getText())) {
                logger.info("Old config: mapping '" + edgeElement.getText() + "' to node " + m.getID());
                source = m.getInterfaces().getRadio();
                break;
              }
            }
          } else if (edgeElement.getName().equals("source")) {
            source = simulation.getMoteWithID(
                Integer.parseInt(edgeElement.getText())).getInterfaces().getRadio();
          } else if (oldConfig && edgeElement.getName().equals("ratio")) {
            /* Old config: parse link ratio */
            double ratio = Double.parseDouble(edgeElement.getText());
            dest.ratio = ratio;
          } else if (edgeElement.getName().equals("dest")) {
            if (oldConfig) {
              /* Old config: create simple destination link */
              Radio destRadio = null;
              for (Mote m: simulation.getMotes()) {
                if (m.toString().equals(edgeElement.getText())) {
                  logger.info("Old config: mapping '" + edgeElement.getText() + "' to node " + m.getID());
                  destRadio = m.getInterfaces().getRadio();
                  break;
                }
              }
              dest = new DGRMDestinationRadio(destRadio);
            } else {
              String destClassName = edgeElement.getText().trim();
              if (destClassName == null || destClassName.isEmpty()) {
                continue;
              }
              Class<? extends DGRMDestinationRadio> destClass =
                simulation.getGUI().tryLoadClass(this, DGRMDestinationRadio.class, destClassName);
              if (destClass == null) {
                throw new RuntimeException("Could not load class: " + destClassName);
              }
              try {
                dest = destClass.newInstance();
                @SuppressWarnings("unchecked")
				List<Element> children = edgeElement.getChildren();
				dest.setConfigXML(children, simulation);
              } catch (Exception e) {
                throw (RuntimeException) 
                new RuntimeException("Unknown class: " + destClassName).initCause(e);
              }
            }
          }
        }
        if (source == null || dest == null) {
          logger.fatal("Failed loading DGRM links, aborting");
          return;
        } else {
          addEdge(new Edge(source, dest));
        }
      }
    }
    requestEdgeAnalysis();
    delayedConfiguration = null;
  }

  public static class Edge {
    public Radio source = null;
    public DGRMDestinationRadio superDest = null;

    public Edge(Radio source, DGRMDestinationRadio dest) {
      this.source = source;
      this.superDest = dest;
    }

    private Collection<Element> getConfigXML() {
      ArrayList<Element> config = new ArrayList<Element>();
      Element element;

      element = new Element("source");
      element.setText("" + source.getMote().getID());
      config.add(element);

      element = new Element("dest");
      element.setText(superDest.getClass().getName());
      Collection<Element> destConfig = superDest.getConfigXML();
      if (destConfig != null) {
        element.addContent(destConfig);
        config.add(element);
      }

      return config;
    }
  }
}
