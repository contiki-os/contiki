package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.util.Observable;
import java.util.Observer;
import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SpinnerNumberModel;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.radiomediums.DirectedGraphMedium;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("DGRM Configurator")
@PluginType(PluginType.SIM_PLUGIN)
public class DGRMConfigurator extends VisPlugin {
  private static Logger logger = Logger.getLogger(DGRMConfigurator.class);

  private final static int IDX_SRC = 0;
  private final static int IDX_DST = 1;
  private final static int IDX_RATIO = 2;
  private final static int IDX_DELAY = 3;
  private final static int IDX_DEL = 4;
  private final static String[] columns = new String[] {
    "Source", "Destination", "Success Ratio (%)", "Delay (ms)", "Delete"
  };

  private GUI gui = null;
  private DirectedGraphMedium radioMedium = null;
  private Observer radioMediumObserver;
  private JTable graphTable = null;
  private JComboBox combo = new JComboBox();

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
      public TableCellEditor getCellEditor(int row, int column) {
        if (column == IDX_RATIO) {
          combo.removeAllItems();
          combo.addItem(1.0);
          combo.addItem(0.9);
          combo.addItem(0.8);
          combo.addItem(0.7);
          combo.addItem(0.6);
          combo.addItem(0.5);
          combo.addItem(0.4);
          combo.addItem(0.3);
          combo.addItem(0.2);
          combo.addItem(0.1);
          combo.addItem(0.1);
          combo.addItem(0.0);
        }
        if (column == IDX_DELAY) {
          combo.removeAllItems();
          combo.addItem(1);
          combo.addItem(2);
          combo.addItem(3);
          combo.addItem(4);
          combo.addItem(5);
        }

        return super.getCellEditor(row, column);
      }
      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int row = rowAtPoint(p);
        int col = convertColumnIndexToModel(columnAtPoint(p));

				/* TODO */
        return super.getToolTipText();
      }
    };
    combo.setEditable(true);
    graphTable.getColumnModel().getColumn(2).setCellRenderer(new DefaultTableCellRenderer() {
      public void setValue(Object value) {
        if (!(value instanceof Double)) {
          setText(value.toString());
          return;
        }
        double v = ((Double) value).doubleValue();
        setText((Math.round(v*1000.0) / 10.0) + "%");
      }
    });
    graphTable.getColumnModel().getColumn(3).setCellRenderer(new DefaultTableCellRenderer() {
      public void setValue(Object value) {
        if (!(value instanceof Long)) {
          setText(value.toString());
          return;
        }
        long v = ((Long) value).longValue();
        setText(v + " ms");
      }
    });
    graphTable.getColumnModel().getColumn(2).setCellEditor(new DefaultCellEditor(combo));
    graphTable.getColumnModel().getColumn(3).setCellEditor(new DefaultCellEditor(combo));

    graphTable.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
    graphTable.getSelectionModel().setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    JButton button = new JButton("Add");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        doAddLink();
      }
    });
    final JScrollPane scrollPane = new JScrollPane(graphTable);
    getContentPane().setLayout(new BorderLayout());
    add(BorderLayout.CENTER, scrollPane);
    add(BorderLayout.SOUTH, button);

    model.fireTableDataChanged();
  }

  private void doAddLink() {
    JComboBox source = new JComboBox();
    for (int i=0; i < gui.getSimulation().getMotesCount(); i++) {
      source.addItem(gui.getSimulation().getMote(i));
    }

    JComboBox dest = new JComboBox();
    for (int i=0; i < gui.getSimulation().getMotesCount(); i++) {
      dest.addItem(gui.getSimulation().getMote(i));
    }
    dest.addItem("ALL");

    JSpinner ratio = new JSpinner(new SpinnerNumberModel(1.0, 0.0, 1.0, 0.01));
    JSpinner delay = new JSpinner(new SpinnerNumberModel(0, 0, 100, 1));

    /* User input */
    Object description[] = {
        columns[0],
        source,
        columns[1],
        dest,
        columns[2],
        ratio,
        columns[3],
        delay
    };
    JOptionPane optionPane = new JOptionPane();
    optionPane.setMessage(description);
    optionPane.setMessageType(JOptionPane.QUESTION_MESSAGE);
    String options[] = new String[] {"Cancel", "Add"};
    optionPane.setOptions(options);
    optionPane.setInitialValue(options[1]);
    JDialog dialog = optionPane.createDialog(gui.getTopParentContainer(), title);
    dialog.setTitle("Add new link");
    dialog.setVisible(true);
    if (optionPane.getValue() == null || !optionPane.getValue().equals("Add")) {
      return;
    }

    /* Register new edge with radio medium */
    DirectedGraphMedium.Edge newEdge;
    if (dest.getSelectedItem() instanceof Mote) {
      newEdge = new DirectedGraphMedium.Edge(
          (Mote) source.getSelectedItem(),
          (Mote) dest.getSelectedItem(),
          ((Number)ratio.getValue()).doubleValue(),
          ((Number)delay.getValue()).longValue()
      );
    } else {
      newEdge = new DirectedGraphMedium.Edge(
          (Mote) source.getSelectedItem(),
          ((Number)ratio.getValue()).doubleValue(),
          ((Number)delay.getValue()).longValue()
      );
    }
    radioMedium.addEdge(newEdge);
    model.fireTableDataChanged();
  }

  private void doRemoveLink(DirectedGraphMedium.Edge edge) {
    radioMedium.removeEdge(edge);
    model.fireTableDataChanged();
  }

  final AbstractTableModel model = new AbstractTableModel() {
    public String getColumnName(int column) {
      if (column < 0 || column >= columns.length) {
        logger.fatal("Unknown column: " + column);
        return "";
      }
      return columns[column];
    }

    public int getRowCount() {
      return radioMedium.getEdges().length;
    }

    public int getColumnCount() {
      return columns.length;
    }

    public Object getValueAt(int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        logger.fatal("Unknown row: " + row);
        return "";
      }
      if (column < 0 || column >= columns.length) {
        logger.fatal("Unknown column: " + column);
        return "";
      }

      DirectedGraphMedium.Edge edge = radioMedium.getEdges()[row];
      if (column == IDX_SRC) {
        if (edge.source == null) {
          return "?";
        }
        return edge.source;
      }
      if (column == IDX_DST) {
        if (edge.dest == null) {
          return "ALL";
        }
        return edge.dest;
      }
      if (column == IDX_RATIO) {
        return edge.successRatio;
      }
      if (column == IDX_DELAY) {
        return edge.delay;
      }
      if (column == IDX_DEL) {
        return new Boolean(false);
      }

      logger.debug("Column data not implemented: " + column);
      return "?";
    }

    public void setValueAt(Object value, int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        logger.fatal("Unknown row: " + row);
        return;
      }
      if (column < 0 || column >= columns.length) {
        logger.fatal("Unknown column: " + column);
        return;
      }

      DirectedGraphMedium.Edge edge = radioMedium.getEdges()[row];
      if (column == IDX_RATIO) {
        /* Success ratio */
        edge.successRatio = ((Number)value).doubleValue();
        radioMedium.setEdgesDirty();
        return;
      }
      if (column == IDX_DELAY) {
        /* Propagation delay */
        edge.delay = ((Number)value).longValue();
        radioMedium.setEdgesDirty();
        return;
      }
      if (column == IDX_DEL) {
        /* Delete link */
        doRemoveLink(edge);
        return;
      }
      super.setValueAt(value, row, column);
    }

    public boolean isCellEditable(int row, int column) {
      if (row < 0 || row >= radioMedium.getEdges().length) {
        return false;
      }

      Mote sourceMote = radioMedium.getEdges()[row].source;
      if (column == IDX_SRC) {
        gui.signalMoteHighlight(sourceMote);
        return false;
      }
      if (column == IDX_DST) {
        if (radioMedium.getEdges()[row].dest != null) {
          gui.signalMoteHighlight(radioMedium.getEdges()[row].dest);
        }
        return false;
      }
      if (column == IDX_RATIO) {
        return true;
      }
      if (column == IDX_DELAY) {
        return true;
      }
      if (column == IDX_DEL) {
        return true;
      }
      return false;
    }

    public Class getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }
  };

  public void closePlugin() {
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
  }

}
