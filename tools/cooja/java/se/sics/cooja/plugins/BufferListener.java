/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.regex.PatternSyntaxException;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.RowFilter;
import javax.swing.SwingUtilities;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableModel;
import javax.swing.table.TableRowSorter;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.AddressMemory;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.Plugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.TableColumnAdjuster;
import se.sics.cooja.dialogs.UpdateAggregator;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.motes.AbstractEmulatedMote;
import se.sics.cooja.motes.AbstractEmulatedMote.MemoryEventHandler;
import se.sics.cooja.motes.AbstractEmulatedMote.MemoryEventType;
import se.sics.cooja.motes.AbstractEmulatedMote.MemoryMonitor;
import se.sics.cooja.util.ArrayQueue;
import se.sics.cooja.util.StringUtils;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Buffer Listener")
@PluginType(PluginType.SIM_PLUGIN)
public class BufferListener extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(BufferListener.class);

  private final static int COLUMN_TIME = 0;
  private final static int COLUMN_FROM = 1;
  private final static int COLUMN_TYPE = 2;
  private final static int COLUMN_DATA = 3;
  private final static int COLUMN_SOURCE = 4;
  private final static String[] COLUMN_NAMES = {
    "Time ms",
    "Mote",
    "Access",
    "*******",
    "Source",
  };

  private static final long TIME_SECOND = 1000*Simulation.MILLISECOND;
  private static final long TIME_MINUTE = 60*TIME_SECOND;
  private static final long TIME_HOUR = 60*TIME_MINUTE;

  final static int MAX_BUFFER_SIZE = 128;

  private static ArrayList<Class<? extends Parser>> bufferParsers = 
    new ArrayList<Class<? extends Parser>>();
  static {
    registerBufferParser(ByteArrayParser.class);
    registerBufferParser(IntegerParser.class);
    registerBufferParser(TerminatedStringParser.class);
    registerBufferParser(PrintableCharactersParser.class);
    registerBufferParser(IPv4AddressParser.class);
    registerBufferParser(IPv6AddressParser.class);
    /* TODO Add parsers: ValueToWidth, AccessHeatmap, .. */
    registerBufferParser(GraphicalHeight4BitsParser.class);
    registerBufferParser(GraphicalGrayscale4BitsParser.class);
  }

  /* TODO Hide identical lines? */

  private static ArrayList<Class<? extends Buffer>> bufferTypes = 
    new ArrayList<Class<? extends Buffer>>();
  static {
    registerBufferType(PacketbufBuffer.class);
    registerBufferType(PacketbufPointerBuffer.class);
    registerBufferType(NodeIDBuffer.class);
    registerBufferType(Queuebuf0Buffer.class);
    /* TODO Add buffers: Queuebuf(1,2,3,4,..). */
    registerBufferType(CustomVariableBuffer.class); /* Special buffer: Custom Variable */
    registerBufferType(CustomPointerBuffer.class); /* Special buffer: Custom Pointer */
  }
  
  private Parser parser = null;
  private Buffer buffer = null;
  public void startPlugin() {
    super.startPlugin();
    if (parser == null) {
      setParser(ByteArrayParser.class);
    }
    if (buffer == null) {
      setBuffer(PacketbufBuffer.class);
    }
  }

  private boolean formatTimeString = false;
  private boolean hasHours = false;

  private final JTable logTable;
  private TableRowSorter<TableModel> logFilter;
  private ArrayQueue<BufferAccess> logs = new ArrayQueue<BufferAccess>();

  private Simulation simulation;

  private JTextField filterTextField = null;
  private JLabel filterLabel = new JLabel("Filter: ");
  private Color filterTextFieldBackground;

  private AbstractTableModel model;

  private MoteCountListener logOutputListener;

  private boolean backgroundColors = false;
  private JCheckBoxMenuItem colorCheckbox;

  private boolean inverseFilter = false;
  private JCheckBoxMenuItem inverseFilterCheckbox;

  private boolean hideReads = true;
  private JCheckBoxMenuItem hideReadsCheckbox;
  
  private JMenu parserMenu = new JMenu("Parser");
  private JMenu bufferMenu = new JMenu("Buffer");
  
  private ArrayList<Mote> monitoredMotes = new ArrayList<Mote>();
  private ArrayList<MemoryMonitor> monitoredMemories = new ArrayList<MemoryMonitor>();
  private MemoryEventHandler meh = new MemoryEventHandler() {
    public void event(MemoryMonitor mm, MemoryEventType type, int adr, int data) {
      MemoryMonitor observedMemory = null;
      
      if (mm.isPointer()) {
        /* Stop observing pointed memory */
        observedMemory = mm.getPointedMemory();
        if (observedMemory == null) {
          logger.fatal("No reference to pointed memory!");
        } else {
          observedMemory.stop();          
          monitoredMemories.remove(observedMemory);
        }

        /* Restart observing pointed memory */
        observedMemory = startObservingMemory(
            (AbstractEmulatedMote) observedMemory.getMote(),
            buffer.getAddress(observedMemory.getMote()),
            buffer.getSize(observedMemory.getMote())
        );
        mm.setPointer(true, observedMemory);
      } else {
        observedMemory = mm;
      }
      
      /* Display memory event */
      if (!hasHours && simulation.getSimulationTime() > TIME_HOUR) {
        hasHours = true;
        repaintTimeColumn();
      }
      
      BufferAccess previous = null;
      previous = mm.getLastBufferAccess();
      BufferAccess current = new BufferAccess(
          simulation.getSimulationTime(), 
          observedMemory,
          type,
          adr, /* XXX May be wrong if pointer, remove these */
          data,
          previous
      );
      logUpdateAggregator.add(current);
      mm.setLastBufferAccess(current);
    }
  };

  private static final int UPDATE_INTERVAL = 250;
  private UpdateAggregator<BufferAccess> logUpdateAggregator = new UpdateAggregator<BufferAccess>(UPDATE_INTERVAL) {
    private Runnable scroll = new Runnable() {
      public void run() {
        logTable.scrollRectToVisible(
            new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
      }
    };
    protected void handle(List<BufferAccess> ls) {
      boolean isVisible = true;
      if (logTable.getRowCount() > 0) {
        Rectangle visible = logTable.getVisibleRect();
        if (visible.y + visible.height < logTable.getHeight()) {
          isVisible = false;
        }
      }

      /* Add */
      int index = logs.size();
      logs.addAll(ls);
      model.fireTableRowsInserted(index, logs.size()-1);

      /* Remove old */
      int removed = 0;
      while (logs.size() > simulation.getEventCentral().getLogOutputBufferSize()) {
        logs.remove(0);
        removed++;
      }
      if (removed > 0) {
        model.fireTableRowsDeleted(0, removed-1);
      }

      if (isVisible) {
        SwingUtilities.invokeLater(scroll);
      }
    }
  };

  /**
   * @param simulation Simulation
   * @param gui GUI
   */
  public BufferListener(final Simulation simulation, final GUI gui) {
    super("Buffer Listener - " + "?" + " motes", gui);
    this.simulation = simulation;

    model = new AbstractTableModel() {
      private static final long serialVersionUID = 3065150390849332924L;
      public String getColumnName(int col) {
        if (col == COLUMN_TIME && formatTimeString) {
          return "Time";
        }
        return COLUMN_NAMES[col];
      }
      public int getRowCount() {
        return logs.size();
      }
      public int getColumnCount() {
        return COLUMN_NAMES.length;
      }
      public Object getValueAt(int row, int col) {
        BufferAccess log = logs.get(row);
        if (col == COLUMN_TIME) {
          return log.getTime();
        } else if (col == COLUMN_FROM) {
          return log.getID();
        } else if (col == COLUMN_TYPE) {
          return log.getType();
        } else if (col == COLUMN_DATA) {
          return log.getParsedData();
        } else if (col == COLUMN_SOURCE) {
          return log.getSource();
        }
        return null;
      }
    };

    logTable = new JTable(model) {
      private static final long serialVersionUID = -930616018336483196L;
      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = rowAtPoint(p);
        int colIndex = columnAtPoint(p);
        int columnIndex = convertColumnIndexToModel(colIndex);
        if (rowIndex < 0 || columnIndex < 0) {
          return super.getToolTipText(e);
        }
        Object v = getValueAt(rowIndex, columnIndex);
        if (v instanceof BufferAccess && parser instanceof GraphicalParser) {
          return
          "<html><font face=\"Verdana\">" + 
          StringUtils.hexDump(((BufferAccess)v).mem, 4, 4).replaceAll("\n", "<br>") + 
          "</html>";
        }
        if (v != null) {
          String t = v.toString();
          if (t.length() > 60) {
            StringBuilder sb = new StringBuilder();
            sb.append("<html>");
            do {
              sb.append(t.substring(0, 60)).append("<br>");
              t = t.substring(60);
            } while (t.length() > 60);
            return sb.append(t).append("</html>").toString();
          }
        }
        return super.getToolTipText(e);
      }
    };
    DefaultTableCellRenderer cellRenderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = -340743275865216182L;
      private final Color[] BG_COLORS = new Color[] {
          new Color(200, 200, 200),
          new Color(200, 200, 255),
          new Color(200, 255, 200),
          new Color(200, 255, 255),
          new Color(255, 200, 200),
          new Color(255, 255, 200),
          new Color(255, 255, 255),
          new Color(255, 220, 200),
          new Color(220, 255, 220),
          new Color(255, 200, 255),
      };
      public Component getTableCellRendererComponent(JTable table,
          Object value, boolean isSelected, boolean hasFocus, int row,
          int column) {
        if (row >= logTable.getRowCount()) {
          if (value instanceof BufferAccess && parser instanceof GraphicalParser) {
            graphicalParserPanel.update((BufferAccess) value, (GraphicalParser)parser);
            return graphicalParserPanel;
          }
          return super.getTableCellRendererComponent(
              table, value, isSelected, hasFocus, row, column);
        }

        Color bgColor = null;
        if (backgroundColors) {
          BufferAccess d = logs.get(logTable.getRowSorter().convertRowIndexToModel(row));
          char last = d.getID().charAt(d.getID().length()-1);
          if (last >= '0' && last <= '9') {
            bgColor = BG_COLORS[last - '0'];
          } else {
            bgColor = null;
          }
        } else {
          bgColor = null;
        }
        if (isSelected) {
          bgColor = table.getSelectionBackground();
        }
        
        if (value instanceof BufferAccess && parser instanceof GraphicalParser) {
          graphicalParserPanel.update((BufferAccess) value, (GraphicalParser)parser);
          graphicalParserPanel.setBackground(bgColor);
          return graphicalParserPanel;
        } else {
          setBackground(bgColor);
        }
        
        return super.getTableCellRendererComponent(
            table, value, isSelected, hasFocus, row, column);
      }
    };
    logTable.getColumnModel().getColumn(COLUMN_TIME).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_FROM).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_TYPE).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_SOURCE).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_DATA).setCellRenderer(cellRenderer);
    logTable.setFillsViewportHeight(true);
    logTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    logTable.setFont(new Font("Monospaced", Font.PLAIN, 12));
    logTable.addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_SPACE) {
          showInAllAction.actionPerformed(null);
        }
      }
    });
    logFilter = new TableRowSorter<TableModel>(model);
    for (int i = 0, n = model.getColumnCount(); i < n; i++) {
      logFilter.setSortable(i, false);
    }
    logTable.setRowSorter(logFilter);

    /* Toggle time format */
    logTable.getTableHeader().addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {
        int colIndex = logTable.columnAtPoint(e.getPoint());
        int columnIndex = logTable.convertColumnIndexToModel(colIndex);

        if (columnIndex != COLUMN_TIME) {
          return;
        }
        formatTimeString = !formatTimeString;
        repaintTimeColumn();
      }
    });
    logTable.addMouseListener(new MouseAdapter() {
      private Parser lastParser = null;
      public void mousePressed(MouseEvent e) {
        if (e.getButton() != MouseEvent.BUTTON2) {
          return;
        }
        int colIndex = logTable.columnAtPoint(e.getPoint());
        int columnIndex = logTable.convertColumnIndexToModel(colIndex);
        if (columnIndex == COLUMN_DATA) {
          /* Temporarily switch to byte parser */
          lastParser = parser;
          setParser(ByteArrayParser.class);
        }
      }
      public void mouseExited(MouseEvent e) {
        if (lastParser != null) {
          /* Switch back to previous parser */
          setParser(lastParser.getClass());
          lastParser = null;
        }
      }
      public void mouseReleased(MouseEvent e) {
        if (lastParser != null) {
          /* Switch back to previous parser */
          setParser(lastParser.getClass());
          lastParser = null;
        }
      }
      public void mouseClicked(MouseEvent e) {
        int colIndex = logTable.columnAtPoint(e.getPoint());
        int columnIndex = logTable.convertColumnIndexToModel(colIndex);
        if (columnIndex != COLUMN_FROM) {
          return;
        }

        int rowIndex = logTable.rowAtPoint(e.getPoint());
        BufferAccess d = logs.get(logTable.getRowSorter().convertRowIndexToModel(rowIndex));
        if (d == null) {
          return;
        }
        simulation.getGUI().signalMoteHighlight(d.mote);
      }
    });

    /* Automatically update column widths */
    final TableColumnAdjuster adjuster = new TableColumnAdjuster(logTable, 0);
    adjuster.packColumns();

    /* Popup menu */
    JPopupMenu popupMenu = new JPopupMenu();
    bufferMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateBufferMenu();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });
    popupMenu.add(bufferMenu);
    parserMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateParserMenu();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });
    popupMenu.add(parserMenu);
    popupMenu.addSeparator();
    JMenu copyClipboard = new JMenu("Copy to clipboard");
    copyClipboard.add(new JMenuItem(copyAllAction));
    copyClipboard.add(new JMenuItem(copyAction));
    popupMenu.add(copyClipboard);
    popupMenu.add(new JMenuItem(clearAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(saveAction));
    popupMenu.addSeparator();
    JMenu focusMenu = new JMenu("Show in");
    focusMenu.add(new JMenuItem(showInAllAction));
    focusMenu.addSeparator();
    focusMenu.add(new JMenuItem(timeLineAction));
    focusMenu.add(new JMenuItem(radioLoggerAction));
    focusMenu.add(new JMenuItem(bufferListenerAction));
    popupMenu.add(focusMenu);
    popupMenu.addSeparator();
    colorCheckbox = new JCheckBoxMenuItem("Mote-specific coloring");
    popupMenu.add(colorCheckbox);
    colorCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        backgroundColors = colorCheckbox.isSelected();
        repaint();
      }
    });
    inverseFilterCheckbox = new JCheckBoxMenuItem("Inverse filter");
    popupMenu.add(inverseFilterCheckbox);
    inverseFilterCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        inverseFilter = inverseFilterCheckbox.isSelected();
        if (inverseFilter) {
          filterLabel.setText("Exclude:");
        } else {
          filterLabel.setText("Filter:");
        }
        setFilter(getFilter());
        repaint();
      }
    });
    hideReadsCheckbox = new JCheckBoxMenuItem("Hide READs", true);
    popupMenu.add(hideReadsCheckbox);
    hideReadsCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        hideReads = hideReadsCheckbox.isSelected();
        setFilter(getFilter());
        repaint();
      }
    });


    logTable.setComponentPopupMenu(popupMenu);

    /* Column width adjustment */
    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        /* Make sure this happens *after* adding history */
        adjuster.setDynamicAdjustment(true);
      }
    });

    logUpdateAggregator.start();
    simulation.getEventCentral().addMoteCountListener(logOutputListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        /* Update title */
        startObserving(mote);
      }
      public void moteWasRemoved(Mote mote) {
        /* Update title */
        stopObserving(mote);
      }
    });

    /* UI components */
    JPanel filterPanel = new JPanel();
    filterPanel.setLayout(new BoxLayout(filterPanel, BoxLayout.X_AXIS));
    filterTextField = new JTextField("");
    filterTextFieldBackground = filterTextField.getBackground();
    filterPanel.add(Box.createHorizontalStrut(2));
    filterPanel.add(filterLabel);
    filterPanel.add(filterTextField);
    filterTextField.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        String str = filterTextField.getText();
        setFilter(str);

        /* Autoscroll */
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            int s = logTable.getSelectedRow();
            if (s < 0) {
              return;
            }

            s = logTable.getRowSorter().convertRowIndexToView(s);
            if (s < 0) {
              return;
            }

            int v = logTable.getRowHeight()*s;
            logTable.scrollRectToVisible(new Rectangle(0, v-5, 1, v+5));
          }
        });
      }
    });
    filterPanel.add(Box.createHorizontalStrut(2));

    getContentPane().add(BorderLayout.CENTER, new JScrollPane(logTable));
    getContentPane().add(BorderLayout.SOUTH, filterPanel);

    updateTitle();
    pack();
    setSize(gui.getDesktopPane().getWidth(), 150);
    setLocation(0, gui.getDesktopPane().getHeight() - 300);
  }

  private boolean startObserving(Mote mote) {
    if (!(mote instanceof AbstractEmulatedMote)) {
      logger.fatal("Failed creating memory monitor: wrong mote type");
      return false;
    }
    AbstractEmulatedMote am = (AbstractEmulatedMote) mote;
    if (!(am.getMemory() instanceof AddressMemory)) {
      logger.fatal("Failed creating memory monitor: wrong memory type");
      return false;
    }

    /* If this is a pointer buffer,
     * we must observe both the pointer itself, and the pointed to memory */

    int address = buffer.getAddress(am);
    int size = buffer.getSize(am);
    if (address < 0 || size < 0 || size > MAX_BUFFER_SIZE) {
      logger.fatal("Failed start observing memory: bad addr/size: " + address + "/" + size);
      return false;
    }

    MemoryMonitor mm = startObservingMemory(am, address, size);
    if (mm == null) {
      return false;
    }
    boolean pointer = (buffer instanceof PointerBuffer);
    if (pointer) {
      int pointerAddress = ((PointerBuffer)buffer).getPointerAddress(am);
      /* Whenever the pointer changes, we re-register this observation */
      MemoryMonitor pointerMM = startObservingMemory(am, pointerAddress, 2);
      pointerMM.setPointer(true, mm);
    }
    return true;
  }
  private MemoryMonitor startObservingMemory(AbstractEmulatedMote mote, int address, int size) {
    MemoryMonitor mm = mote.createMemoryMonitor(meh);
    if (mm == null) {
      logger.fatal("Failed creating memory monitor: not implemented");
      return null;
    }

    boolean started = mm.start(address, size);
    if (!started) {
      logger.fatal("Failed creating memory monitor: could not start monitor");
      return null;
    }

    monitoredMemories.add(mm);
    /* TODO Don't keep pointed memories in this list.
     * Instead automatically unregister them from their parent memories. */
    if (!monitoredMotes.contains(mote)) {
      monitoredMotes.add(mote);
    }
    updateTitle();
    return mm;
  }
  private void stopObserving(Mote mote) {
    if (!(mote instanceof AbstractEmulatedMote)) {
      return;
    }

    for (MemoryMonitor mm: monitoredMemories.toArray(new MemoryMonitor[0])) {
      if (mm.getMote() == mote) {
        mm.stop();
        monitoredMemories.remove(mm);
      }
    }

    monitoredMotes.remove(mote);
    updateTitle();
  }

  private void repaintTimeColumn() {
    logTable.getColumnModel().getColumn(COLUMN_TIME).setHeaderValue(
        logTable.getModel().getColumnName(COLUMN_TIME));
    repaint();
  }

  private void updateTitle() {
    if (buffer != null) {
      String bufferDesc = GUI.getDescriptionOf(buffer);
      /* Special buffer: Custom Variable/Custom Pointer */
      if (buffer.getClass() == CustomVariableBuffer.class) {
        int offset = ((CustomVariableBuffer)buffer).getOffset();
        if (offset > 0) {
          bufferDesc = "&" + ((CustomVariableBuffer)buffer).getVariable() + "[" + offset + "] (" + ((CustomVariableBuffer)buffer).getSize() + ")";
        } else {
          bufferDesc = ((CustomVariableBuffer)buffer).getVariable() + " (" + ((CustomVariableBuffer)buffer).getSize() + ")";
        }
      } else if (buffer.getClass() == CustomPointerBuffer.class) {
        int offset = ((CustomPointerBuffer)buffer).getOffset();
        bufferDesc = "*" + ((CustomPointerBuffer)buffer).getVariable() + " (" + ((CustomPointerBuffer)buffer).getSize() + ")";
        if (offset > 0) {
          bufferDesc = "*" + ((CustomPointerBuffer)buffer).getVariable() + "[" + offset + "] (" + ((CustomPointerBuffer)buffer).getSize() + ")";
        } else {
          bufferDesc = "*" + ((CustomPointerBuffer)buffer).getVariable() + " (" + ((CustomPointerBuffer)buffer).getSize() + ")";
        }
      }

      int count=0;
      for (MemoryMonitor mm: monitoredMemories) {
        if (mm.isPointer()) continue;
        count++;
      }
      setTitle("Buffer Listener - " + 
          bufferDesc + " - " + 
          count + " buffers on " + monitoredMotes.size() + " motes");
    }
  }

  public void closePlugin() {
    /* Stop observing motes */
    logUpdateAggregator.stop();
    simulation.getEventCentral().removeMoteCountListener(logOutputListener);

    for (Mote m: simulation.getMotes()) {
      stopObserving(m);
    }
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    element = new Element("filter");
    element.setText(filterTextField.getText());
    config.add(element);

    if (formatTimeString) {
      element = new Element("formatted_time");
      config.add(element);
    }
    if (backgroundColors) {
      element = new Element("coloring");
      config.add(element);
    }
    if (inverseFilter) {
      element = new Element("inversefilter");
      config.add(element);
    }
    if (!hideReads) {
      element = new Element("showreads");
      config.add(element);
    }
    element = new Element("parser");
    element.setText(parser.getClass().getName());
    config.add(element);

    element = new Element("buffer");
    element.setText(buffer.getClass().getName());
    /* Special buffer: Custom Variable/Custom Pointer */
    if (buffer.getClass() == CustomVariableBuffer.class) {
      element.setAttribute("variable", ((CustomVariableBuffer) buffer).getVariable());
      element.setAttribute("size", "" + ((CustomVariableBuffer) buffer).getSize());
      element.setAttribute("offset", "" + ((CustomVariableBuffer) buffer).getOffset());
    } else if (buffer.getClass() == CustomPointerBuffer.class) {
      element.setAttribute("variable", ((CustomPointerBuffer) buffer).getVariable());
      element.setAttribute("size", "" + ((CustomPointerBuffer) buffer).getSize());
      element.setAttribute("offset", "" + ((CustomPointerBuffer) buffer).getOffset());
    }
    config.add(element);
    
    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();
      if ("filter".equals(name)) {
        final String str = element.getText();
        EventQueue.invokeLater(new Runnable() {
          public void run() {
            setFilter(str);
          }
        });
      } else if ("coloring".equals(name)) {
        backgroundColors = true;
        colorCheckbox.setSelected(true);
      } else if ("inversefilter".equals(name)) {
        inverseFilter = true;
        inverseFilterCheckbox.setSelected(true);
      } else if ("showreads".equals(name)) {
        hideReads = false;
        hideReadsCheckbox.setSelected(false);
      } else if ("formatted_time".equals(name)) {
        formatTimeString = true;
        repaintTimeColumn();
      } else if ("parser".equals(name)) {
        String parserClassname = element.getText();
        Class<? extends Parser> parserClass = 
          simulation.getGUI().tryLoadClass(this, Parser.class, parserClassname);
        if (parserClass == null) {
          logger.warn("Could not create buffer parser: could not find class: " + parserClassname);
        } else {
          setParser(parserClass);
        }
      } else if ("buffer".equals(name)) {
        String bufferClassname = element.getText();
        Class<? extends Buffer> bufferClass = 
          simulation.getGUI().tryLoadClass(this, Buffer.class, bufferClassname);
        if (bufferClass == null) {
          logger.warn("Could not create buffer parser: could not find class: " + bufferClassname);
        } else {

          /* Special buffer: Custom Variable/Custom Pointer */
          if (bufferClass == CustomVariableBuffer.class ||
              bufferClass == CustomPointerBuffer.class) {
            String varName = element.getAttributeValue("variable");
            int varSize = Integer.parseInt(element.getAttributeValue("size"));
            int offset = Integer.parseInt(element.getAttributeValue("offset"));
            setBuffer(bufferClass, varName, varSize, offset);
          } else {
            setBuffer(bufferClass);
          }
        }
      }
    }

    return true;
  }

  public String getFilter() {
    return filterTextField.getText();
  }

  public void setFilter(String str) {
    filterTextField.setText(str);

    try {
      final RowFilter<Object,Object> regexp;
      if (str != null && str.length() > 0) {
        /* TODO Handle graphical components */
        regexp = RowFilter.regexFilter(str, COLUMN_FROM, COLUMN_TYPE, COLUMN_SOURCE, COLUMN_DATA);
      } else {
        regexp = null;
      }
      RowFilter<Object, Object> wrapped = new RowFilter<Object, Object>() {
        public boolean include(RowFilter.Entry<? extends Object, ? extends Object> entry) {
          if (hideReads) {
            int row = (Integer) entry.getIdentifier();
            if (logs.get(row).isRead) {
              return false;
            }
          }
          if (regexp != null) {
            boolean pass = regexp.include(entry);
            
            if (inverseFilter && pass) {
              return false;
            } else if (!inverseFilter && !pass) {
              return false;
            }
          }
          return true;
        }
      };
      logFilter.setRowFilter(wrapped);
      filterTextField.setBackground(filterTextFieldBackground);
      filterTextField.setToolTipText(null);
    } catch (PatternSyntaxException e) {
      logFilter.setRowFilter(null);
      filterTextField.setBackground(Color.red);
      filterTextField.setToolTipText("Syntax error in regular expression: " + e.getMessage());
    }
  }

  public void trySelectTime(final long time) {
    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        for (int i=0; i < logs.size(); i++) {
          if (logs.get(i).time < time) {
            continue;
          }

          int view = logTable.convertRowIndexToView(i);
          if (view < 0) {
            continue;
          }
          logTable.scrollRectToVisible(logTable.getCellRect(view, 0, true));
          logTable.setRowSelectionInterval(view, view);
          return;
        }
      }
    });  
  }

  public class BufferAccess {
    public final AbstractEmulatedMote mote;
    public final long time;
    
    public final boolean isRead;

    private MemoryMonitor mm;
    private int adr;
    
    public final String typeStr;
    public final String sourceStr;
    public final byte[] mem;
    
    private boolean[] accessedBitpattern = null;
    
    public BufferAccess(
        long time, MemoryMonitor mm,
        MemoryEventType type, int adr, int data,
        BufferAccess previous) {
      this.time = time;
      this.mote = (AbstractEmulatedMote) mm.getMote();
      this.mm = mm;
      this.adr = adr;
      this.isRead = (type == MemoryEventType.READ);
      this.mem = mm.getMote().getMemory().getMemorySegment(mm.getAddress(), mm.getSize());
      
      /* Apply currently written data, if write operation */
      if (type == MemoryEventType.WRITE) {
        if (adr >= mm.getAddress() && adr < mm.getAddress() + mm.getSize()) {
          int offset = adr - mm.getAddress();
          byte now = (byte)(0xff&data);
          /*byte prev = mem[offset];*/
          /*logger.debug(String.format(
              "WRITE: 0x%02x: %02x -> %02x", adr, prev, now));*/
          mem[offset] = now;
        } else {
          logger.warn(String.format(
              "Write operation outside monitored addresses: %x !@ %x - %x", 
              adr, mm.getAddress(), mm.getAddress()+mm.getSize()));
        }
      }
      
      /* Generate diff bit pattern */
      if (previous != null) {
        accessedBitpattern = new boolean[previous.mem.length];
        for (int i=0; i < previous.mem.length; i++) {
          accessedBitpattern[i] = (previous.mem[i] != mem[i]);
        }
      }
      
      typeStr = type.toString();
      String s = mote.getPCString();
      sourceStr = s==null?"[unknown]":s;
    }

    public Object getParsedData() {
      return parser.parse(mm, adr, this);
    }
    public String getParsedString() {
      return StringUtils.toHex(mem);
    }
    
    public boolean[] getAccessedBitpattern() {
      return accessedBitpattern;
    }

    public String getType() {
      return typeStr;
    }

    public Object getSource() {
      return sourceStr;
    }

    public String getID() {
      return "ID:" + mote.getID();
    }
    
    public String getTime() {
      if (formatTimeString) {
        long t = time;
        long h = (t / TIME_HOUR);
        t -= (t / TIME_HOUR)*TIME_HOUR;
        long m = (t / TIME_MINUTE);
        t -= (t / TIME_MINUTE)*TIME_MINUTE;
        long s = (t / TIME_SECOND);
        t -= (t / TIME_SECOND)*TIME_SECOND;
        long ms = t / Simulation.MILLISECOND;
        if (hasHours) {
          return String.format("%d:%02d:%02d.%03d", h,m,s,ms);
        } else {
          return String.format("%02d:%02d.%03d", m,s,ms);
        }
      } else {
        return "" + time / Simulation.MILLISECOND;
      }
    }
  }

  private Action saveAction = new AbstractAction("Save to file") {
    private static final long serialVersionUID = -4140706275748686944L;

    public void actionPerformed(ActionEvent e) {
      JFileChooser fc = new JFileChooser();
      File suggest = new File(GUI.getExternalToolsSetting("BUFFER_LISTENER_SAVEFILE", "BufferAccessLogger.txt"));
      fc.setSelectedFile(suggest);
      int returnVal = fc.showSaveDialog(GUI.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        return;
      }

      File saveFile = fc.getSelectedFile();
      if (saveFile.exists()) {
        String s1 = "Overwrite";
        String s2 = "Cancel";
        Object[] options = { s1, s2 };
        int n = JOptionPane.showOptionDialog(
            GUI.getTopParentContainer(),
            "A file with the same name already exists.\nDo you want to remove it?",
            "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      GUI.setExternalToolsSetting("BUFFER_LISTENER_SAVEFILE", saveFile.getPath());
      if (saveFile.exists() && !saveFile.canWrite()) {
        logger.fatal("No write access to file: " + saveFile);
        return;
      }

      try {
        PrintWriter outStream = new PrintWriter(new FileWriter(saveFile));

        StringBuilder sb = new StringBuilder();
        for (int i=0; i < logTable.getRowCount(); i++) {
          sb.append(logTable.getValueAt(i, COLUMN_TIME));
          sb.append("\t");
          sb.append(logTable.getValueAt(i, COLUMN_FROM));
          sb.append("\t");
          sb.append(logTable.getValueAt(i, COLUMN_TYPE));
          sb.append("\t");
          if (parser instanceof GraphicalParser) {
            BufferAccess ba = (BufferAccess) logTable.getValueAt(i, COLUMN_DATA);
            sb.append(ba.getParsedString());
          } else {
            sb.append(logTable.getValueAt(i, COLUMN_DATA));
          }
          sb.append("\t");
          sb.append(logTable.getValueAt(i, COLUMN_SOURCE));
          sb.append("\n");
        }
        outStream.print(sb.toString());
        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }
    }
  };

  private Action bufferListenerAction = new AbstractAction("in Buffer Listener") {
    private static final long serialVersionUID = -6358463434933029699L;
    public void actionPerformed(ActionEvent e) {
      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      long time = logs.get(model).time;

      Plugin[] plugins = simulation.getGUI().getStartedPlugins();
      for (Plugin p: plugins) {
        if (!(p instanceof BufferListener)) {
          continue;
        }

        /* Select simulation time */
        BufferListener plugin = (BufferListener) p;
        plugin.trySelectTime(time);
      }
    }
  };

  private Action timeLineAction = new AbstractAction("in Timeline") {
    private static final long serialVersionUID = -6358463434933029699L;
    public void actionPerformed(ActionEvent e) {
      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      long time = logs.get(model).time;
      
      Plugin[] plugins = simulation.getGUI().getStartedPlugins();
      for (Plugin p: plugins) {
        if (!(p instanceof TimeLine)) {
          continue;
        }
        
        /* Select simulation time */
        TimeLine plugin = (TimeLine) p;
        plugin.trySelectTime(time);
      }
    }
  };
  
  private Action radioLoggerAction = new AbstractAction("in Radio Logger") {
    private static final long serialVersionUID = -3041714249257346688L;
    public void actionPerformed(ActionEvent e) {
      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      long time = logs.get(model).time;

      Plugin[] plugins = simulation.getGUI().getStartedPlugins();
      for (Plugin p: plugins) {
        if (!(p instanceof RadioLogger)) {
          continue;
        }

        /* Select simulation time */
        RadioLogger plugin = (RadioLogger) p;
        plugin.trySelectTime(time);
      }
    }
  };

  private Action showInAllAction = new AbstractAction("All") {
    private static final long serialVersionUID = -8433490108577001803L;
    {
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_SPACE, 0, true));
    }

    public void actionPerformed(ActionEvent e) {
      timeLineAction.actionPerformed(null);
      radioLoggerAction.actionPerformed(null);
    }
  };

  private Action clearAction = new AbstractAction("Clear") {
    private static final long serialVersionUID = -2115620313183440224L;
    public void actionPerformed(ActionEvent e) {
      int size = logs.size();
      if (size > 0) {
        logs.clear();
        model.fireTableRowsDeleted(0, size - 1);
      }
    }
  };

  private Action copyAction = new AbstractAction("Selected") {
    private static final long serialVersionUID = -8433490108577001803L;
    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      int[] selectedRows = logTable.getSelectedRows();

      StringBuilder sb = new StringBuilder();
      for (int i: selectedRows) {
        sb.append(logTable.getValueAt(i, COLUMN_TIME));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_FROM));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_TYPE));
        sb.append("\t");
        if (parser instanceof GraphicalParser) {
          BufferAccess ba = (BufferAccess) logTable.getValueAt(i, COLUMN_DATA);
          sb.append(ba.getParsedString());
        } else {
          sb.append(logTable.getValueAt(i, COLUMN_DATA));
        }
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_SOURCE));
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

  private Action copyAllAction = new AbstractAction("All") {
    private static final long serialVersionUID = -5038884975254178373L;

    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for (int i=0; i < logTable.getRowCount(); i++) {
        sb.append(logTable.getValueAt(i, COLUMN_TIME));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_FROM));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_TYPE));
        sb.append("\t");
        if (parser instanceof GraphicalParser) {
          BufferAccess ba = (BufferAccess) logTable.getValueAt(i, COLUMN_DATA);
          sb.append(ba.getParsedString());
        } else {
          sb.append(logTable.getValueAt(i, COLUMN_DATA));
        }
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_SOURCE));
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

  private final ActionListener parserSelectedListener = new ActionListener() {
    @SuppressWarnings("unchecked")
    public void actionPerformed(ActionEvent e) {
      Class<? extends Parser> bpClass =
        (Class<? extends Parser>) 
        ((JMenuItem) e.getSource()).getClientProperty("CLASS");
      setParser(bpClass);
    }
  };
  private void updateParserMenu() {
    parserMenu.removeAll();
    
    for (Class<? extends Parser> bpClass: bufferParsers) {
      JCheckBoxMenuItem mi = new JCheckBoxMenuItem(GUI.getDescriptionOf(bpClass), bpClass==parser.getClass());
      mi.putClientProperty("CLASS", bpClass);
      mi.addActionListener(parserSelectedListener);
      parserMenu.add(mi);
    }
  }

  private final ActionListener bufferSelectedListener = new ActionListener() {
    @SuppressWarnings("unchecked")
    public void actionPerformed(ActionEvent e) {
      Class<? extends Buffer> btClass =
        (Class<? extends Buffer>) 
        ((JMenuItem) e.getSource()).getClientProperty("CLASS");
      setBuffer(btClass);
    }
  };
  private void updateBufferMenu() {
    bufferMenu.removeAll();
    
    for (Class<? extends Buffer> btClass: bufferTypes) {
      JCheckBoxMenuItem mi = new JCheckBoxMenuItem(GUI.getDescriptionOf(btClass), btClass==buffer.getClass());
      mi.putClientProperty("CLASS", btClass);
      mi.addActionListener(bufferSelectedListener);
      bufferMenu.add(mi);
    }
  }

  private void setParser(Class<? extends Parser> bpClass) {
    Parser bp = null;
    try {
      bp = bpClass.newInstance();
    } catch (InstantiationException e) {
      logger.warn("Could not create buffer parser: " + e.getMessage(), e);
      return;
    } catch (IllegalAccessException e) {
      logger.warn("Could not create buffer parser: " + e.getMessage(), e);
      return;
    }
    
    parser = bp;
    logTable.getColumnModel().getColumn(COLUMN_DATA).setHeaderValue(
        GUI.getDescriptionOf(bp));

    repaint();
  }


  private class VariableInfoComponent {
    private JPanel mainPanel = new JPanel();
    private JTextField textName = new JTextField();
    private JTextField textSize = new JTextField();
    private JTextField textOffset = new JTextField();

    public VariableInfoComponent(String name, String size, String offset) {
      textName.setText(name);
      textSize.setText(size);
      textOffset.setText(offset);

      mainPanel.setLayout(new GridLayout(3, 2, 5, 5));
      mainPanel.add(new JLabel("Variable name:"));
      mainPanel.add(textName);
      mainPanel.add(new JLabel("Buffer size (1-" + MAX_BUFFER_SIZE + "):"));
      mainPanel.add(textSize);
      mainPanel.add(new JLabel("Offset:"));
      mainPanel.add(textOffset);
    }
    public String getName() {
      return textName.getText();
    }
    public String getSize() {
      return textSize.getText();
    }
    public String getOffset() {
      return textOffset.getText();
    }
    public JComponent getComponent() {
      return mainPanel;
    }
  }
  
  private void setBuffer(Class<? extends Buffer> btClass) {
    setBuffer(btClass, null, -1, -1);
  }
  private void setBuffer(Class<? extends Buffer> btClass, String varName, int varSize, int varOffset) {
    /* Special buffer: Custom Variable/Custom Pointer */
    if ((btClass == CustomVariableBuffer.class || btClass == CustomPointerBuffer.class)
        && varName == null) {
      String d = (btClass == CustomVariableBuffer.class)?"Variable":"Pointer";

      /* Read variable name */
      String suggestName = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARNAME", "node_id");
      String suggestSize = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "8");
      String suggestOffset = GUI.getExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "0");
      VariableInfoComponent infoComponent =
        new VariableInfoComponent(suggestName, suggestSize, suggestOffset);
      
      int result = JOptionPane.showConfirmDialog(BufferListener.this,
          infoComponent.getComponent(),
          d + " info",
          JOptionPane.OK_CANCEL_OPTION);
      if (result != JOptionPane.OK_OPTION) {
        /* Abort */
        return;
      }
      varName = infoComponent.getName();
      if (varName == null) {
        /* Abort */
        return;
      }
      try {
        varSize = Integer.parseInt(infoComponent.getSize());
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer size " + infoComponent.getSize() + ": " + e.getMessage(), e);
        /* Abort */
        return;
      }
      if (varSize < 1 || varSize > MAX_BUFFER_SIZE) {
        /* Abort */
        logger.fatal("Bad buffer size " + infoComponent.getSize() + ": min 1, max " + MAX_BUFFER_SIZE);
        return;
      }
      try {
        varOffset = Integer.parseInt(infoComponent.getOffset());
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer offset " + infoComponent.getOffset() + ": " + e.getMessage(), e);
        /* Abort */
        return;
      }
    }
    
    /* Generate buffer type */
    Buffer bt = null;
    try {
      bt = btClass.newInstance();
    } catch (InstantiationException e) {
      logger.warn("Could not create buffer type: " + e.getMessage(), e);
      return;
    } catch (IllegalAccessException e) {
      logger.warn("Could not create buffer type: " + e.getMessage(), e);
      return;
    }

    /* Special buffer: Custom Variable/Custom Pointer */
    if (btClass == CustomVariableBuffer.class) {
      ((CustomVariableBuffer)bt).setInfo(varName, varSize, varOffset);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARNAME", varName);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "" + varSize);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "" + varOffset);
    } else if (btClass == CustomPointerBuffer.class) {
      ((CustomPointerBuffer)bt).setInfo(varName, varSize, varOffset);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARNAME", varName);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "" + varSize);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "" + varOffset);
    }

    buffer = bt;

    /* Reregister memory monitors */
    for (Mote m: simulation.getMotes()) {
      stopObserving(m);
    }
    for (Mote m: simulation.getMotes()) {
      /* TODO Check return values */
      startObserving(m);
    }

    /* Clear previous buffers, update gui */
    clearAction.actionPerformed(null);
    updateTitle();
    repaint();
  }
  
  public static interface Parser {
    /**
     * @param mm Memory monitor
     * @param address Address that changed. May not contain all changes
     * @param ba Buffer Access object
     * @return String or custom graphical object
     */
    public Object parse(MemoryMonitor mm, int address, BufferAccess ba);
  }
  public static abstract class GraphicalParser implements Parser {
    BufferAccess ba = null;
    public Object parse(MemoryMonitor mm, int address, BufferAccess ba) {
      this.ba = ba;
      return ba;
    }
    public abstract void paintComponent(Graphics g, JComponent c);
    public abstract int getUnscaledWidth();
  }

  public static abstract class StringParser implements Parser {
    public Object parse(MemoryMonitor mm, int address, BufferAccess ba) {
      return parseString(mm,address,ba);
    }

    /**
     * @param mm Memory monitor
     * @param address Address that changed. May not contain all changes
     * @param ba Buffer Access object
     * @return String representation
     */
    public abstract String parseString(MemoryMonitor mm, int address, BufferAccess ba);
  }
  public static interface Buffer {
    public int getAddress(Mote mote);
    public int getSize(Mote mote);
    
  }
  public static interface PointerBuffer extends Buffer {
    public int getAddress(Mote mote);
    public int getSize(Mote mote);

    public int getPointerAddress(Mote mote);
  }

  public static boolean registerBufferParser(Class<? extends Parser> bpClass) {
    if (bufferParsers.contains(bpClass)) {
      return false;
    }
    bufferParsers.add(bpClass);
    return true;
  }
  public static void unregisterBufferParser(Class<? extends Parser> bpClass) {
    bufferParsers.remove(bpClass);
  }

  public static boolean registerBufferType(Class<? extends Buffer> btClass) {
    if (bufferTypes.contains(btClass)) {
      return false;
    }
    bufferTypes.add(btClass);
    return true;
  }
  public static void unregisterBufferType(Class<? extends Buffer> btClass) {
    bufferTypes.remove(btClass);
  }
  
  @ClassDescription("Byte array")
  public static class ByteArrayParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      boolean[] diff = ba.getAccessedBitpattern();
      if (diff == null) {
        return StringUtils.toHex(ba.mem, 4); /* 00112233 00112233 .. */
      }
      StringBuilder sb = new StringBuilder();
      sb.append("<html>");
      boolean inRed = false;
      int group = 0;
      for (int i=0; i < ba.mem.length; i++) {
        if (inRed == diff[i]) {
          //logger.debug("same " + i);
          sb.append(StringUtils.toHex(ba.mem[i]));
        } else if (!inRed) {
          /* Diff begins */
          sb.append("<font color=\"red\">");
          sb.append(StringUtils.toHex(ba.mem[i]));
          inRed = true;
        } else {
          /* Diff ends */
          sb.append("</font>");
          sb.append(StringUtils.toHex(ba.mem[i]));
          inRed = false;
        }
        group++;
        if (++group >= 8) {
          group=0;
          sb.append(" ");
        }
      }
      if (inRed) {
        /* Diff ends */
        sb.append("</font>");
        inRed = false;
      }
      sb.append("</html>");
      return sb.toString();
    }
  }
  
  @ClassDescription("Integer array")
  public static class IntegerParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      StringBuilder sb = new StringBuilder();
      boolean[] diff = ba.getAccessedBitpattern();
      
      sb.append("<html>");
      for (int i=0; i+1 < ba.mem.length;) {
        int val = 0;
        boolean red = false;
        if (diff != null && (diff[i] || diff[i+1])) {
          red = true;
        }
        val += ((ba.mem[i++] & 0xFF)) << 8;
        val += ((ba.mem[i++] & 0xFF)) << 0;
        
        if (red) {
          sb.append("<font color=\"red\">");
        }
        sb.append((Integer.reverseBytes(val) >> 16) + " ");
        if (red) {
          sb.append("</font>");
        }
      }
      sb.append("</html>");
      return sb.toString();
    }
  }

  @ClassDescription("Terminated string")
  public static class TerminatedStringParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      /* TODO Diff? */
      int i;
      for (i=0; i < ba.mem.length; i++) {
        if (ba.mem[i] == '\0') {
          break;
        }
      }
      byte[] termString = new byte[i];
      System.arraycopy(ba.mem, 0, termString, 0, i);
      return new String(termString).replaceAll("[^\\p{Print}]", "");
    }
  }

  @ClassDescription("Printable characters")
  public static class PrintableCharactersParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      /* TODO Diff? */
      return new String(ba.mem).replaceAll("[^\\p{Print}]", ""); 
    }
  }

  @ClassDescription("IPv6 address")
  public static class IPv6AddressParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      /* TODO Diff? */
      if (ba.mem.length < 16) {
        return "[must monitor at least 16 bytes]";
      }
      byte[] mem;
      if (ba.mem.length > 16) {
        mem = new byte[16];
        System.arraycopy(ba.mem, 0, mem, 0, 16);
      } else {
        mem = ba.mem;
      }
      return IPAddress.compressIPv6Address(StringUtils.toHex(mem, 2).replaceAll(" ", ":"));
    }
  }

  @ClassDescription("IPv4 address")
  public static class IPv4AddressParser extends StringParser {
    public String parseString(MemoryMonitor mm, int address, BufferAccess ba) {
      /* TODO Diff? */
      if (ba.mem.length < 4) {
        return "[must monitor at least 4 bytes]";
      }
      StringBuilder sb = new StringBuilder();
      sb.append(0xff&ba.mem[0]);
      sb.append(".");
      sb.append(0xff&ba.mem[1]);
      sb.append(".");
      sb.append(0xff&ba.mem[2]);
      sb.append(".");
      sb.append(0xff&ba.mem[3]);
      return sb.toString();
    }
  }
  
  static class GrapicalParserPanel extends JPanel {
    private static final long serialVersionUID = -8375160571675638467L;
    static final int XOFFSET = 0;
    static final int HEIGHT = 16;
    private GraphicalParser parser;
    public GrapicalParserPanel() {
      super();
    }
    public void update(BufferAccess ba, GraphicalParser parser) {
      this.parser = parser;
      parser.ba = ba;
      setPreferredSize(new Dimension(parser.getUnscaledWidth() + 2*XOFFSET, HEIGHT));
    }
    public void paintComponent(Graphics g) {
      super.paintComponent(g);
      g.translate(XOFFSET, 0);
    
      if (getWidth() > getPreferredSize().width + 10 ||
          getWidth() < getPreferredSize().width - 10) {
        double scale = 1.0*getWidth()/getPreferredSize().width;
        ((Graphics2D)g).scale(scale, 1.0);
      }

      parser.paintComponent(g, this);
    }
  };
  private GrapicalParserPanel graphicalParserPanel = new GrapicalParserPanel();
  
  @ClassDescription("Graphical: Height")
  public static class GraphicalHeight4BitsParser extends GraphicalParser {
    public int getUnscaledWidth() {
      return ba.mem.length*2;
    }
    public void paintComponent(Graphics g, JComponent c) {
      g.setColor(Color.GRAY);
      boolean[] diff = ba.getAccessedBitpattern();
      for (int x=0; x < ba.mem.length; x++) {
        boolean red = false;
        if (diff != null && diff[x]) {
          red = true;
        }
        int v = (int)0xff&ba.mem[x];
        int h = Math.min(v/16, 15); /* crop */
        if (red) {
          g.setColor(Color.RED);
        }
        g.fillRect(x*2, 16-h, 2, h);
        if (red) {
          g.setColor(Color.GRAY);
        }
      }
    }
  }
  
  @ClassDescription("Graphical: Grayscale")
  public static class GraphicalGrayscale4BitsParser extends GraphicalParser {
    public int getUnscaledWidth() {
      return ba.mem.length*2;
    }
    public void paintComponent(Graphics g, JComponent c) {
      boolean[] diff = ba.getAccessedBitpattern();
      for (int x=0; x < ba.mem.length; x++) {
        boolean red = false;
        if (diff != null && diff[x]) {
          red = true;
        }
        int color = 255-(0xff&ba.mem[x]);
        if (red) {
          g.setColor(Color.RED);
        } else {
          g.setColor(new Color(color, color, color));
        }
        g.fillRect(x*2, 1, 2, 15);
      }
    }
  }
  
  @ClassDescription("Variable: node_id")
  public static class NodeIDBuffer implements Buffer {
    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists("node_id")) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getVariableAddress("node_id");
    }
    public int getSize(Mote mote) {
      return 2;
    }
  }
  
  @ClassDescription("Queuebuf 0 RAM")
  public static class Queuebuf0Buffer implements Buffer {
    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists("buframmem")) {
        return -1;
      }
      int offset = 0;
      return ((AddressMemory)mote.getMemory()).getVariableAddress("buframmem") + offset;
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }
  
  @ClassDescription("packetbuf_aligned")
  public static class PacketbufBuffer implements Buffer {
    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists("packetbuf_aligned")) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getVariableAddress("packetbuf_aligned");
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }
  
  @ClassDescription("*packetbufptr")
  public static class PacketbufPointerBuffer implements PointerBuffer {
    public int getPointerAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists("packetbufptr")) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getVariableAddress("packetbufptr");
    }
    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists("packetbufptr")) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getIntValueOf("packetbufptr");
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }
  
  @ClassDescription("Custom pointer")
  public static class CustomPointerBuffer implements PointerBuffer {
    public String variable;
    public int size;
    public int offset;
    public void setInfo(String variable, int size, int offset) {
      this.variable = variable;
      this.size = size;
      this.offset = offset;
    }
    public String getVariable() {
      return variable;
    }
    public int getSize() {
      return size;
    }
    public int getOffset() {
      return offset;
    }

    public int getPointerAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists(variable)) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getVariableAddress(variable);
    }
    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists(variable)) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getIntValueOf(variable)+offset;
    }
    public int getSize(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists(variable)) {
        return -1;
      }
      return size;
    }
  }
  
  @ClassDescription("Custom variable")
  public static class CustomVariableBuffer implements Buffer {
    public String variable;
    public int size;
    public int offset;
    public void setInfo(String variable, int size, int offset) {
      this.variable = variable;
      this.size = size;
      this.offset = offset;
    }
    public String getVariable() {
      return variable;
    }
    public int getSize() {
      return size;
    }
    public int getOffset() {
      return offset;
    }
    public boolean isPointer() {
      return false;
    }

    public int getAddress(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists(variable)) {
        return -1;
      }
      return ((AddressMemory)mote.getMemory()).getVariableAddress(variable)+offset;
    }
    public int getSize(Mote mote) {
      if (!((AddressMemory)mote.getMemory()).variableExists(variable)) {
        return -1;
      }
      return size;
    }
  }
  
}
