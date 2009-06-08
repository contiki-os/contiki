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
 * $Id: DirectedGraphMedium.java,v 1.2 2009/06/08 12:42:10 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.*;

import org.jdom.Element;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.DGRMConfigurator;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Directed Graph Radio Medium (DGRM)")
public class DirectedGraphMedium extends AbstractRadioMedium {
  private static Logger logger = Logger.getLogger(DirectedGraphMedium.class);

  /* Signal strengths in dBm.
   * Approx. values measured on TmoteSky */
  public static final double SS_NOTHING = -100;
  public static final double SS_STRONG = -10;
  public static final double SS_WEAK = -95;

  private Simulation simulation;
  private boolean edgesDirty = false;
  private Random random = null;

  private DirectedGraphMedium.Edge edges[] = new DirectedGraphMedium.Edge[0];

  public DirectedGraphMedium(Simulation simulation) {
    super(simulation);
    random = simulation.getRandomGenerator();

    setEdgesDirty();

    /* Register visualizer plugin */
    simulation.getGUI().registerTemporaryPlugin(DGRMConfigurator.class);

    this.simulation = simulation;
  }

  public void addEdge(Edge e) {
    DirectedGraphMedium.Edge newEdges[] = new DirectedGraphMedium.Edge[edges.length+1];
    System.arraycopy(edges, 0, newEdges, 0, edges.length);

    newEdges[newEdges.length-1] = e;
    edges = newEdges;
    setEdgesDirty();

    ((AbstractRadioMedium.RadioMediumObservable)
        this.getRadioMediumObservable()).setRadioMediumChangedAndNotify();
  }

  public void removeEdge(Edge edge) {
    ArrayList<Edge> list = new ArrayList<Edge>();
    for (DirectedGraphMedium.Edge e: edges) {
      list.add(e);
    }
    if (!list.contains(edge)) {
      logger.fatal("Cannot remove edge: " + edge);
      return;
    }
    list.remove(edge);
    DirectedGraphMedium.Edge newEdges[] = new DirectedGraphMedium.Edge[list.size()];
    list.toArray(newEdges);
    edges = newEdges;
    setEdgesDirty();

    ((AbstractRadioMedium.RadioMediumObservable)
        this.getRadioMediumObservable()).setRadioMediumChangedAndNotify();
  }

  public void setEdgesDirty() {
    edgesDirty = true;
  }

  public Edge[] getEdges() {
    return edges;
  }

  public void registerMote(Mote mote, Simulation sim) {
    super.registerMote(mote, sim);

    for (Edge edge: edges) {
      if (edge.delayedLoadConfig == null) {
        continue;
      }

      /* Try to configure edge now */
      if (edge.setConfigXML(edge.delayedLoadConfig, sim)) {
        edge.delayedLoadConfig = null;
      }
    }

    setEdgesDirty();
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    super.unregisterMote(mote, sim);

    for (Edge edge: edges) {
      if (edge.source == mote || edge.dest == mote) {
        removeEdge(edge);
      }
    }

    setEdgesDirty();
  }

  private class DestinationRadio {
    Radio radio;
    double ratio;
    long delay; /* us */

    public DestinationRadio(Radio dest, double ratio, long delay) {
      this.radio = dest;

      this.ratio = ratio;
      this.delay = delay;
    }

    public String toString() {
      return radio.getMote().toString();
    }
  }

  /* Used for optimizing lookup time */
  private Hashtable<Radio,DestinationRadio[]> edgesTable = new Hashtable<Radio,DestinationRadio[]>();

  private void analyzeEdges() {
    Hashtable<Radio,ArrayList<DestinationRadio>> newTable =
      new Hashtable<Radio,ArrayList<DestinationRadio>>();

    /* Fill edge hash table with all edges */
    for (Edge edge: edges) {
      if (edge.source == null) {
        return; /* Still dirty, wait until all edges are loaded */
      }

      ArrayList<DestinationRadio> destRadios;
      if (!newTable.containsKey(edge.source.getInterfaces().getRadio())) {
        /* Create new source */
        destRadios = new ArrayList<DestinationRadio>();
      } else {
        /* Extend source radio with another destination */
        destRadios = newTable.get(edge.source.getInterfaces().getRadio());
      }

      DestinationRadio destRadio;
      if (edge.dest == null) {
        /* All radios */
        Vector<Radio> allRadios = getRegisteredRadios();
        for (Radio r: allRadios) {
          destRadio = new DestinationRadio(r, edge.successRatio, edge.delay);
          destRadios.add(destRadio);
        }
      } else {
        destRadio = new DestinationRadio(edge.dest.getInterfaces().getRadio(), edge.successRatio, edge.delay);
        destRadios.add(destRadio);
      }

      newTable.put(edge.source.getInterfaces().getRadio(), destRadios);
    }

    /* Convert to arrays */
    Hashtable<Radio,DestinationRadio[]> newTable2 = new Hashtable<Radio,DestinationRadio[]>();
    Enumeration<Radio> sources = newTable.keys();
    while (sources.hasMoreElements()) {
      Radio source = sources.nextElement();
      ArrayList<DestinationRadio> list = newTable.get(source);
      DestinationRadio[] arr = new DestinationRadio[list.size()];
      list.toArray(arr);
      newTable2.put(source, arr);
    }

    this.edgesTable = newTable2;
    edgesDirty = false;
  }

  public RadioConnection createConnections(Radio source) {
    if (edgesDirty) {
      analyzeEdges();
    }

    /* Create new radio connection using edge hash table */
    DestinationRadio[] destinations = edgesTable.get(source);
    if (destinations == null || destinations.length == 0) {
      /* No destinations */
      /*logger.info(sendingRadio + ": No dest");*/
      return new RadioConnection(source);
    }

    /*logger.info(source + ": " + destinations.length + " potential destinations");*/
    RadioConnection newConn = new RadioConnection(source);
    for (DestinationRadio dest: destinations) {
      if (dest.radio == source) {
        /* Fail: cannot receive our own transmission */
        /*logger.info(source + ": Fail, receiver is sender");*/
        continue;
      }

      if (dest.ratio < 1.0 && random.nextDouble() > dest.ratio) {
        /*logger.info(source + ": Fail, randomly");*/
        continue;
      }

      if (dest.radio.isReceiving()) {
        /* Fail: radio is already actively receiving */
        /*logger.info(source + ": Fail, receiving");*/
        newConn.addInterfered(dest.radio);

        /* We will also interfere with the other connection */
        dest.radio.interfereAnyReception();
        RadioConnection otherConnection = null;
        for (RadioConnection conn : getActiveConnections()) {
          for (Radio dstRadio : conn.getDestinations()) {
            if (dstRadio == dest.radio) {
              otherConnection = conn;
              break;
            }
          }
        }
        if (otherConnection != null) {
          otherConnection.removeDestination(dest.radio);
          otherConnection.addInterfered(dest.radio);
        }
        continue;
      }

      if (dest.radio.isInterfered()) {
        /* Fail: radio is interfered in another connection */
        /*logger.info(source + ": Fail, interfered");*/
        newConn.addInterfered(dest.radio);
        continue;
      }

      /* Success: radio starts receiving */
      /*logger.info(source + ": OK: " + dest.radio);*/
      newConn.addDestination(dest.radio, dest.delay);
    }

    return newConn;
  }

  public void updateSignalStrengths() {
    if (edgesDirty) {
      analyzeEdges();
    }

    for (Radio radio : getRegisteredRadios()) {
      radio.setCurrentSignalStrength(SS_NOTHING);
    }

    for (RadioConnection conn : getActiveConnections()) {
      conn.getSource().setCurrentSignalStrength(SS_STRONG);
      for (Radio dstRadio : conn.getDestinations()) {
        dstRadio.setCurrentSignalStrength(SS_STRONG);
      }
    }

    for (RadioConnection conn : getActiveConnections()) {
      for (Radio intfRadio : conn.getInterfered()) {
        intfRadio.setCurrentSignalStrength(SS_WEAK);
        if (!intfRadio.isInterfered()) {
          intfRadio.interfereAnyReception();
        }
      }
    }

  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    for (Edge edge: edges) {
      element = new Element("edge");
      element.addContent(edge.getConfigXML());
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(final Collection<Element> configXML, boolean visAvailable) {
    random = simulation.getRandomGenerator();

    for (Element element : configXML) {
      if (element.getName().equals("edge")) {
        Edge edge = new Edge(null, null, 0, 0);
        edge.delayedLoadConfig = element.getChildren();
        addEdge(edge);
      }
    }

    setEdgesDirty();
    return true;
  }

  public static class Edge {
    public Mote source = null;
    public Mote dest = null; /* null: all motes*/
    public double successRatio = 1.0; /* Link success ratio (per packet). */
    public long delay = 0; /* Propagation delay (us). */

    public Edge(Mote source, double ratio, long delay) {
      this.source = source;
      this.successRatio = ratio;
      this.delay = delay;

      this.dest = null;
    }

    public Edge(Mote source, Mote dest, double ratio, long delay) {
      this.source = source;
      this.successRatio = ratio;
      this.delay = delay;

      this.dest = dest;
    }

    public Collection<Element> delayedLoadConfig = null; /* Used for restoring edges from config */
    public Collection<Element> getConfigXML() {
      Vector<Element> config = new Vector<Element>();
      Element element;

      element = new Element("src");
      element.setText(source.toString());
      config.add(element);

      element = new Element("dest");
      if (dest == null) {
        element.setText("ALL");
      } else {
        element.setText(dest.toString());
      }
      config.add(element);

      element = new Element("ratio");
      element.setText("" + successRatio);
      config.add(element);

      element = new Element("delay");
      element.setText("" + delay);
      config.add(element);

      return config;
    }
    public boolean setConfigXML(Collection<Element> configXML, Simulation simulation) {
      for (Element element : configXML) {
        if (element.getName().equals("src")) {
          String moteDescription = element.getText();

          boolean foundMote = false;
          for (Mote m: simulation.getMotes()) {
            if (moteDescription.equals(m.toString())) {
              foundMote = true;
              source = m;
              break;
            }
          }

          if (!foundMote) {
            return false;
          }
        }

        if (element.getName().equals("dest")) {
          String moteDescription = element.getText();

          if (moteDescription.equals("ALL")) {
            dest = null; /* ALL */
          } else {
            boolean foundMote = false;
            for (Mote m: simulation.getMotes()) {
              if (moteDescription.equals(m.toString())) {
                foundMote = true;
                dest = m;
                break;
              }
            }
            if (!foundMote) {
              return false;
            }
          }
        }

        if (element.getName().equals("ratio")) {
          successRatio = Double.parseDouble(element.getText());
        }

        if (element.getName().equals("delay")) {
          delay = Long.parseLong(element.getText());
        }

      }
      return true;
    }
  }

}
