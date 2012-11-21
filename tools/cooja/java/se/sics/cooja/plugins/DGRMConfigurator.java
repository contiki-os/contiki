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

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Observable;
import java.util.Observer;

import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.SupportedArguments;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.radiomediums.AbstractRadioMedium;
import se.sics.cooja.radiomediums.DGRMDestinationRadio;
import se.sics.cooja.radiomediums.DirectedGraphMedium;
import se.sics.cooja.radiomediums.DirectedGraphMedium.Edge;
import se.sics.cooja.util.StringUtils;

/**
 * Simple user interface for configuring edges for the Directed Graph
 * Radio Medium (DGRM).
 *
 * @see DirectedGraphMedium
 * @author Fredrik Osterlind
 */
@ClassDescription("DGRM Links")
@PluginType(PluginType.SIM_PLUGIN)
@SupportedArguments(radioMediums = {DirectedGraphMedium.class})
public class DGRMConfigurator extends VisPlugin {
	private static final long serialVersionUID = 4769638341635882051L;
	private static Logger logger = Logger.getLogger(DGRMConfigurator.class);

  private final static int IDX_SRC = 0;
  private final static int IDX_DST = 1;
  private final static int IDX_RATIO = 2;
  private final static int IDX_SIGNAL = 3;
  private final static int IDX_LQI = 4;
  private final static int IDX_DELAY = 5;

  private final static String[] COLUMN_NAMES = new String[] {
    "Source", "Destination", "RX Ratio", "RSSI","LQI", "Delay"
  };

  private GUI gui = null;
  private DirectedGraphMedium radioMedium = null;
  private Observer radioMediumObserver;
  private JTable graphTable = null;
  private JComboBox combo = new JComboBox();
	private JButton removeButton;

  public DGRMConfigurator(Simulation sim, GUI gui) {
    super("DGRM Configurator", gui);
    this.gui = gui;
    radioMedium = (DirectedGraphMedium) sim.getRadioMedium();

    /* Listen for graph updates */
    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        model.fireTableDataChanged();
      }
    });

    /* Represent directed graph by table */
    graphTable = new JTable(model) {
			private static final long serialVersionUID = -4680013510092815210L;
			public TableCellEditor getCellEditor(int row, int column) {
				combo.removeAllItems();
        if (column == IDX_RATIO) {
          for (double d=1.0; d >= 0.0; d -= 0.1) {
            combo.addItem(d);
          }
        } else if (column == IDX_SIGNAL) {
          for (double d=AbstractRadioMedium.SS_STRONG; d >= AbstractRadioMedium.SS_WEAK; d -= 1) {
            combo.addItem((int) d);
          }
        } else if (column == IDX_LQI) {
            for (int d = 110; d > 50; d -= 5) {
              combo.addItem((int) d);
            }
        } else if (column == IDX_DELAY) {
          for (double d=0; d <= 5; d++) {
            combo.addItem(d);
          }
        }
        return super.getCellEditor(row, column);
      }
    };
    graphTable.setFillsViewportHeight(true);
    combo.setEditable(true);

    graphTable.getColumnModel().getColumn(IDX_RATIO).setCellRenderer(new DefaultTableCellRenderer() {
			private static final long serialVersionUID = 4470088575039698508L;
			public void setValue(Object value) {
        if (!(value instanceof Double)) {
          setText(value.toString());
          return;
        }
        double v = ((Double) value).doubleValue();
        setText(String.format("%1.1f%%", 100*v));
      }
    });
    graphTable.getColumnModel().getColumn(IDX_SIGNAL).setCellRenderer(new DefaultTableCellRenderer() {
			private static final long serialVersionUID = -7170745293267593460L;
			public void setValue(Object value) {
        if (!(value instanceof Long)) {
          setText(value.toString());
          return;
        }
        double v = ((Double) value).doubleValue();
        setText(String.format("%1.1f dBm", v));
      }
    });
    graphTable.getColumnModel().getColumn(IDX_LQI).setCellRenderer(new DefaultTableCellRenderer() {
		private static final long serialVersionUID = -4669897764928372246L;
		public void setValue(Object value) {
	    if (!(value instanceof Long)) {
	      setText(value.toString());
	      return;
	    }
    	long v = ((Long) value).longValue();
    	setText(String.valueOf(v));
		}
    });
    graphTable.getColumnModel().getColumn(IDX_DELAY).setCellRenderer(new DefaultTableCellRenderer() {
			private static final long serialVersionUID = -4669897764928372246L;
			public void setValue(Object value) {
        if (!(value instanceof Long)) {
          setText(value.toString());
          return;
        }
        long v = ((Long) value).longValue();
        setText(v + " ms");
      }
    });
    graphTable.getColumnModel().getColumn(IDX_RATIO).setCellEditor(new DefaultCellEditor(combo));
    graphTable.getColumnModel().getColumn(IDX_SIGNAL).setCellEditor(new DefaultCellEditor(combo));
    graphTable.getColumnModel().getColumn(IDX_LQI).setCellEditor(new DefaultCellEditor(combo));
    graphTable.getColumnModel().getColumn(IDX_DELAY).setCellEditor(new DefaultCellEditor(combo));

    graphTable.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
    graphTable.getSelectionModel().setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    JPanel southPanel = new JPanel(new GridLayout(1, 3));
    JButton button = new JButton("Add");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        doAddLink();
      }
    });
    southPanel.add(button);
    button = new JButton("Remove");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
      	doRemoveSelectedLink();
      }
    });
    removeButton = button;
    removeButton.setEnabled(false);
    southPanel.add(button);
    button = new JButton("Import");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
      	doImportFromFile();
      }
    });
    southPanel.add(button);

    getContentPane().setLayout(new BorderLayout());
    add(BorderLayout.CENTER, new JScrollPane(graphTable));
    add(BorderLayout.SOUTH, southPanel);

    graphTable.getSelectionModel().addListSelectionListener(new ListSelectionListener() {
    	public void valueChanged(ListSelectionEvent e) {
    		ListSelectionModel lsm = (ListSelectionModel)e.getSource();
    		if (e.getValueIsAdjusting()) {
    			return;
    		}
    		removeButton.setEnabled(!lsm.isSelectionEmpty());
    	}
    });

    model.fireTableDataChanged();
    setSize(400, 300);
  }

  private void doAddLink() {
    JComboBox source = new JComboBox();
    JComboBox dest = new JComboBox();
    for (Mote m: gui.getSimulation().getMotes()) {
      source.addItem(m);
      dest.addItem(m);
    }

    /* User input */
    Object description[] = {
        COLUMN_NAMES[0],
        source,
        COLUMN_NAMES[1],
        dest
    };
    JOptionPane optionPane = new JOptionPane();
    optionPane.setMessage(description);
    optionPane.setMessageType(JOptionPane.QUESTION_MESSAGE);
    String options[] = new String[] {"Cancel", "Add"};
    optionPane.setOptions(options);
    optionPane.setInitialValue(options[1]);
    JDialog dialog = optionPane.createDialog(this, title);
    dialog.setTitle("Add graph edge");
    dialog.setVisible(true);
    if (optionPane.getValue() == null || !optionPane.getValue().equals("Add")) {
      return;
    }

    /* Register new edge with radio medium */
    DirectedGraphMedium.Edge newEdge = new DirectedGraphMedium.Edge(
    		((Mote) source.getSelectedItem()).getInterfaces().getRadio(),
    		new DGRMDestinationRadio(
    				((Mote) dest.getSelectedItem()).getInterfaces().getRadio()
    		)
    );
    radioMedium.addEdge(newEdge);
    model.fireTableDataChanged();
  }

  private void doRemoveLink(DirectedGraphMedium.Edge edge) {
    radioMedium.removeEdge(edge);
    model.fireTableDataChanged();
  }
	private void doRemoveSelectedLink() {
    int firstIndex = graphTable.getSelectedRow();
		if (firstIndex < 0) {
			return;
		}

		doRemoveLink(radioMedium.getEdges()[firstIndex]);
	}
	private void doImportFromFile() {
		/* Delete existing edges */
    if (radioMedium.getEdges().length > 0) {
      String[] options = new String[] { "Remove", "Cancel" };
      int n = JOptionPane.showOptionDialog(
          GUI.getTopParentContainer(),
          "Importing edges will remove all your existing edges.",
          "Clear edge table?", JOptionPane.YES_NO_OPTION,
          JOptionPane.WARNING_MESSAGE, null, options, options[0]);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
      for (DirectedGraphMedium.Edge e: radioMedium.getEdges()) {
      	radioMedium.removeEdge(e);
      }
    }

		/* Select file to import edges from */
    JFileChooser fc = new JFileChooser();
    File suggest = new File(GUI.getExternalToolsSetting("DGRM_IMPORT_LINKS_FILE", "cooja_dgrm_links.dat"));
    fc.setSelectedFile(suggest);
    int returnVal = fc.showOpenDialog(GUI.getTopParentContainer());
    if (returnVal != JFileChooser.APPROVE_OPTION) {
      return;
    }
    File file = fc.getSelectedFile();
    if (file == null || !file.exists() || !file.canRead()) {
      logger.fatal("No read access to file: " + file);
      return;
    }
    GUI.setExternalToolsSetting("DGRM_IMPORT_LINKS_FILE", file.getPath());

    /* Parse and import edges */
    try {
    	importEdges(parseDGRMLinksFile(file, gui.getSimulation()));
    } catch (Exception e) {
    	GUI.showErrorDialog(this, "Error when importing DGRM links from " + file.getName(), e, false);
    }
	}

	private void importEdges(DirectedGraphMedium.Edge[] edges) {
		Arrays.sort(edges, new Comparator<DirectedGraphMedium.Edge>() {
			public int compare(Edge o1, Edge o2) {
				return o1.source.getMote().getID() - o2.source.getMote().getID();
			}
		});
		for (DirectedGraphMedium.Edge e: edges) {
			radioMedium.addEdge(e);
		}
		logger.info("Imported " + edges.length + " DGRM edges");
	}

	static final int INDEX_SRC = 0;
	static final int INDEX_DST = 1;
	static final int INDEX_PRR = 2;
	static final int INDEX_PRR_CI = 3;
	static final int INDEX_NUM_TX = 4;
	static final int INDEX_NUM_RX = 5;
	static final int INDEX_RSSI_MEDIAN = 6;
	static final int INDEX_RSSI_MIN = 7;
	static final int INDEX_RSSI_MAX = 8;
	public static DirectedGraphMedium.Edge[] parseDGRMLinksFile(File file, Simulation simulation) {
		String fileContents = StringUtils.loadFromFile(file);
		ArrayList<DirectedGraphMedium.Edge> edges = new ArrayList<DirectedGraphMedium.Edge>();

		/* format: # [src] [dst] [prr] [prr_ci] [num_tx] [num_rx] [rssi] [rssi_min] [rssi_max] */
		for (String l: fileContents.split("\n")) {
			l = l.trim();
			if (l.startsWith("#")) {
				continue;
			}

			Mote m;
			String[] arr = l.split(" ");
			int source = Integer.parseInt(arr[INDEX_SRC]);
			m = simulation.getMoteWithID(source);
			if (m == null) {
				throw new RuntimeException("No simulation mote with ID " + source);
			}
			Radio sourceRadio = m.getInterfaces().getRadio();
			int dst = Integer.parseInt(arr[INDEX_DST]);
			m = simulation.getMoteWithID(dst);
			if (m == null) {
				throw new RuntimeException("No simulation mote with ID " + dst);
			}
			DGRMDestinationRadio destRadio = new DGRMDestinationRadio(m.getInterfaces().getRadio());
			double prr = Double.parseDouble(arr[INDEX_PRR]);
			/*double prrConfidence = Double.parseDouble(arr[INDEX_PRR_CI]);*/
			/*int numTX <- INDEX_NUM_TX;*/
			/*int numRX <- INDEX_NUM_RX;*/
			double rssi = Double.parseDouble(arr[INDEX_RSSI_MEDIAN]);
			/*int rssiMin <- INDEX_RSSI_MIN;*/
			/*int rssiMax <- INDEX_RSSI_MAX;*/

			DirectedGraphMedium.Edge edge = new DirectedGraphMedium.Edge(sourceRadio, destRadio);
			destRadio.delay = 0;
			destRadio.ratio = prr;
			/*destRadio.prrConfidence = prrConfidence;*/
			destRadio.signal = rssi;
			edges.add(edge);
		}
		return edges.toArray(new DirectedGraphMedium.Edge[0]);
	}

  final AbstractTableModel model = new AbstractTableModel() {
		private static final long serialVersionUID = 9101118401527171218L;
		public String getColumnName(int column) {
      if (column < 0 || column >= COLUMN_NAMES.length) {
        return "";
      }
      return COLUMN_NAMES[column];
    }
    public int getRowCount() {
      return radioMedium.getEdges().length;
    }
    public int getColumnCount() {
      return COLUMN_NAMES.length;
    }
    public Object getValueAt(int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        return "";
      }
      if (column < 0 || column >= COLUMN_NAMES.length) {
        return "";
      }
      DirectedGraphMedium.Edge edge = radioMedium.getEdges()[row];
      if (column == IDX_SRC) {
        return edge.source.getMote();
      }
      if (column == IDX_DST) {
        return edge.superDest.radio.getMote();
      }
      if (column == IDX_RATIO) {
        return ((DGRMDestinationRadio)edge.superDest).ratio;
      }
      if (column == IDX_SIGNAL) {
        return ((DGRMDestinationRadio)edge.superDest).signal;
      }
      if (column == IDX_LQI) {
          return ((DGRMDestinationRadio)edge.superDest).lqi;
        }
      if (column == IDX_DELAY) {
        return ((DGRMDestinationRadio)edge.superDest).delay / Simulation.MILLISECOND;
      }
      return "";
    }
    public void setValueAt(Object value, int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        return;
      }
      if (column < 0 || column >= COLUMN_NAMES.length) {
        return;
      }

      DirectedGraphMedium.Edge edge = radioMedium.getEdges()[row];
      try {
      	if (column == IDX_RATIO) {
      		((DGRMDestinationRadio)edge.superDest).ratio = ((Number)value).doubleValue();
      	} else if (column == IDX_SIGNAL) {
      		((DGRMDestinationRadio)edge.superDest).signal = ((Number)value).doubleValue();
      	} else if (column == IDX_DELAY) {
      		((DGRMDestinationRadio)edge.superDest).delay =
      			((Number)value).longValue() * Simulation.MILLISECOND;
      	} else if (column == IDX_LQI) {
  			((DGRMDestinationRadio)edge.superDest).lqi = ((Number)value).intValue();
      	} 
      	else {
          super.setValueAt(value, row, column);
      	}
      	radioMedium.requestEdgeAnalysis();
      } catch (ClassCastException e) {
      }
    }

    public boolean isCellEditable(int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        return false;
      }

      Mote sourceMote = radioMedium.getEdges()[row].source.getMote();
      if (column == IDX_SRC) {
        gui.signalMoteHighlight(sourceMote);
        return false;
      }
      if (column == IDX_DST) {
      	gui.signalMoteHighlight(radioMedium.getEdges()[row].superDest.radio.getMote());
        return false;
      }
      if (column == IDX_RATIO) {
        return true;
      }
      if (column == IDX_SIGNAL) {
        return true;
      }
      if (column == IDX_LQI) {
        return true;
      }
      if (column == IDX_DELAY) {
        return true;
      }
      return false;
    }

    public Class<? extends Object> getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }
  };

  public void closePlugin() {
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
  }

}
