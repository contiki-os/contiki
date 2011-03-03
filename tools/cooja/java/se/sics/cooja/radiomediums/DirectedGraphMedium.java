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
 * $Id: DirectedGraphMedium.java,v 1.8 2010/12/02 15:25:50 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Random;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
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
	private Hashtable<Radio,DestinationRadio[]> edgesTable = new Hashtable<Radio,DestinationRadio[]>();

	public DirectedGraphMedium() {
		/* Do not initialize radio medium: use only for hash table */
		super(null);
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
			if (conn.getSource().getCurrentSignalStrength() < SS_STRONG) {
				conn.getSource().setCurrentSignalStrength(SS_STRONG);
			}
			for (Radio dstRadio : conn.getDestinations()) {
				if (dstRadio.getCurrentSignalStrength() < SS_STRONG) {
					dstRadio.setCurrentSignalStrength(SS_STRONG);
				}
			}
		}

		/* Set signal strength to weak on interfered */
		for (RadioConnection conn : conns) {
			for (Radio intfRadio : conn.getInterfered()) {
				if (intfRadio.getCurrentSignalStrength() < SS_STRONG) {
					intfRadio.setCurrentSignalStrength(SS_STRONG);
				}

				if (!intfRadio.isInterfered()) {
					/*logger.warn("Radio was not interfered");*/
					intfRadio.interfereAnyReception();
				}
			}
		}
	}



	/**
	 * Generates hash table using current edges for efficient lookup.
	 */
	protected void analyzeEdges() {
		Hashtable<Radio,ArrayList<DestinationRadio>> listTable =
			new Hashtable<Radio,ArrayList<DestinationRadio>>();

		/* Fill edge hash table with all edges */
		for (Edge edge: getEdges()) {
			ArrayList<DestinationRadio> destRadios;
			if (!listTable.containsKey(edge.source)) {
				destRadios = new ArrayList<DestinationRadio>();
			} else {
				destRadios = listTable.get(edge.source);
			}

			destRadios.add(edge.superDest);
			listTable.put(edge.source, destRadios);
		}

		/* Convert to arrays */
		Hashtable<Radio,DestinationRadio[]> arrTable =  new Hashtable<Radio,DestinationRadio[]>();
		Enumeration<Radio> sources = listTable.keys();
		while (sources.hasMoreElements()) {
			Radio source = sources.nextElement();
			DestinationRadio[] arr = listTable.get(source).toArray(new DestinationRadio[0]);
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
	public DestinationRadio[] getPotentialDestinations(Radio source) {
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
		DestinationRadio[] destinations = getPotentialDestinations(source);
		if (destinations == null || destinations.length == 0) {
			/* No destinations */
			/*logger.info(sendingRadio + ": No dest");*/
			return newConn;
		}

		/*logger.info(source + ": " + destinations.length + " potential destinations");*/
		for (DestinationRadio d: destinations) {
			DGRMDestinationRadio dest = (DGRMDestinationRadio) d;
			if (dest.radio == source) {
				/* Fail: cannot receive our own transmission */
				/*logger.info(source + ": Fail, receiver is sender");*/
				continue;
			}

			/* Fail if radios are on different (but configured) channels */ 
			if (source.getChannel() >= 0 &&
					dest.radio.getChannel() >= 0 &&
					source.getChannel() != dest.radio.getChannel()) {
				continue;
			}

			if (!dest.radio.isReceiverOn()) {
				/* Fail: radio is off */
				/*logger.info(source + ": Fail, off");*/
				newConn.addInterfered(dest.radio);
				continue;
			}

			if (dest.ratio < 1.0 && random.nextDouble() > dest.ratio) {
				/*logger.info(source + ": Fail, randomly");*/
				/* TODO Interfere now? */
				newConn.addInterfered(dest.radio);

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
					otherConnection.addInterfered(dest.radio);
				}
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

		boolean warnedOldConfig = false;
		for (Element element : delayedConfiguration) {
			if (element.getName().equals("edge")) {
				Collection<Element> edgeConfig = element.getChildren();
				Radio source = null;
				DestinationRadio dest = null;
				for (Element edgeElement : edgeConfig) {
					if (edgeElement.getName().equals("src")) {
						/* Old version, ignore edge */
						if (!warnedOldConfig) {
							logger.fatal("Old simulation config detected: DGRM links will not be imported");
							warnedOldConfig = true;
						}
						return;
					} else if (edgeElement.getName().equals("source")) {
						source = simulation.getMoteWithID(
								Integer.parseInt(edgeElement.getText())).getInterfaces().getRadio();
					} else if (edgeElement.getName().equals("dest")) {
						String destClassName = edgeElement.getText().trim();
						if (destClassName == null || destClassName.isEmpty()) {
							continue;
						}
						Class<? extends DestinationRadio> destClass =
							simulation.getGUI().tryLoadClass(this, DestinationRadio.class, destClassName);
						if (destClass == null) {
							throw new RuntimeException("Could not load class: " + destClassName);
						}
						try {
							dest = destClass.newInstance();
							dest.setConfigXML(edgeElement.getChildren(), simulation);
						} catch (Exception e) {
							throw (RuntimeException) 
							new RuntimeException("Unknown class: " + destClassName).initCause(e);
						}
					}
				}
				if (source == null || dest == null) {
					if (!warnedOldConfig) {
						logger.fatal("Old simulation config detected: DGRM links will not be imported");
						warnedOldConfig = true;
					}
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
		public DestinationRadio superDest = null;

		public Edge(Radio source, DestinationRadio dest) {
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
