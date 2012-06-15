/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
import java.util.Arrays;
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

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.MoteMemory.MemoryEventType;
import se.sics.cooja.Plugin;
import se.sics.cooja.PluginType;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.TableColumnAdjuster;
import se.sics.cooja.dialogs.UpdateAggregator;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.motes.AbstractEmulatedMote;
import se.sics.cooja.util.ArrayQueue;
import se.sics.cooja.util.StringUtils;

/**
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Buffer view")
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
    registerBufferType(CustomVariableBuffer.class);
    registerBufferType(CustomIntegerBuffer.class);
    registerBufferType(CustomPointerBuffer.class);
  }

  private Parser parser = null;
  private Buffer buffer = null;
  public void startPlugin() {
    super.startPlugin();
    if (parser == null) {
      setParser(ByteArrayParser.class);
    }
    if (buffer == null) {
      Buffer b = createBufferInstance(PacketbufBuffer.class);
      if (b != null) {
        if (b.configure(BufferListener.this)) {
          setBuffer(b);
        }
      }
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

  private boolean withStackTrace = false;
  private JCheckBoxMenuItem withStackTraceCheckbox;

  private JMenu bufferMenu = new JMenu("Buffer");
  private JMenu parserMenu = new JMenu("Show as");

  private ArrayList<Mote> motes = new ArrayList<Mote>();
  private ArrayList<SegmentMemoryMonitor> memoryMonitors = new ArrayList<SegmentMemoryMonitor>();

  private TimeEvent hourTimeEvent = new TimeEvent(0) {
    public void execute(long t) {
      hasHours = true;
      repaintTimeColumn();
      hourTimeEvent = null;
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

    if (simulation.getSimulationTime() > TIME_HOUR) {
      hasHours = true;
      hourTimeEvent = null;
    } else {
      simulation.scheduleEvent(hourTimeEvent, TIME_HOUR);
    }

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
          return log.getTime(formatTimeString, hasHours);
        } else if (col == COLUMN_FROM) {
          return log.getID();
        } else if (col == COLUMN_TYPE) {
          return log.getType();
        } else if (col == COLUMN_DATA) {
          return parser.parse(log);
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
        if (rowIndex < 0 || colIndex < 0) {
          return super.getToolTipText(e);
        }
        int row = convertRowIndexToModel(rowIndex);
        int column = convertColumnIndexToModel(colIndex);
        if (row < 0 || column < 0) {
          return super.getToolTipText(e);
        }

        if (column == COLUMN_SOURCE) {
          BufferAccess ba = logs.get(row);
          if (ba.stackTrace != null) {
            return
            "<html><pre>" +
            ba.stackTrace +
            "</pre></html>";
          }
          return "No stack trace (enable in popup menu)";
        }
        if (column == COLUMN_DATA) {
          BufferAccess ba = logs.get(row);
          return
          "<html><pre>" +
          "Address: " + (ba.address==0?"null":String.format("%04x\n", ba.address)) +
          StringUtils.hexDump(ba.mem, 4, 4) +
          "</pre></html>";
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
    logTable.getColumnModel().getColumn(COLUMN_DATA).setWidth(400);

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
    hideReadsCheckbox = new JCheckBoxMenuItem("Hide READs", hideReads);
    popupMenu.add(hideReadsCheckbox);
    hideReadsCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        hideReads = hideReadsCheckbox.isSelected();
        setFilter(getFilter());
        repaint();
      }
    });

    withStackTraceCheckbox = new JCheckBoxMenuItem("Capture stack traces", withStackTrace);
    popupMenu.add(withStackTraceCheckbox);
    withStackTraceCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        withStackTrace = withStackTraceCheckbox.isSelected();
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
        adjuster.setAdjustColumn(COLUMN_DATA, false);
      }
    });

    logUpdateAggregator.start();
    simulation.getEventCentral().addMoteCountListener(logOutputListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        /* Update title */
        try {
          startMonitoring(mote);
        } catch (Exception e) {
          logger.warn("Could not monitor buffer on: " + mote, e);
        }
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

  private boolean startMonitoring(Mote mote) throws Exception {
    /* If this is a pointer buffer,
     * we must observe both the pointer itself, and the pointed to memory */

    SegmentMemoryMonitor mm = buffer.createMemoryMonitor(this, mote);
    memoryMonitors.add(mm);
    if (!motes.contains(mote)) {
      motes.add(mote);
    }
    updateTitle();
    return true;
  }

  public enum MemoryMonitorType { SEGMENT, POINTER, CONSTPOINTER };

  static class PointerMemoryMonitor extends SegmentMemoryMonitor {
    private SegmentMemoryMonitor segmentMonitor = null;
    private int lastSegmentAddress = -1;
    private final int pointerAddress;
    private final int pointerSize;

    public PointerMemoryMonitor(
        BufferListener bl, Mote mote,
        int pointerAddress, int pointerSize, int segmentSize)
    throws Exception {
      super(bl, mote, pointerAddress, pointerSize);
      this.pointerAddress = pointerAddress;
      this.pointerSize = pointerSize;

      registerSegmentMonitor(segmentSize, false);
    }

    private void registerSegmentMonitor(int size, boolean notify) throws Exception {
      byte[] pointerValue = mote.getMemory().getMemorySegment(pointerAddress, pointerSize);
      int segmentAddress = mote.getMemory().parseInt(pointerValue);

      segmentMonitor = new SegmentMemoryMonitor(bl, mote, segmentAddress, size);
      if (notify) {
        segmentMonitor.memoryChanged(mote.getMemory(), MemoryEventType.WRITE, -1);
      }
      lastSegmentAddress = segmentAddress;
    }

    final public void memoryChanged(MoteMemory memory,
        se.sics.cooja.MoteMemory.MemoryEventType type, int address) {
      if (type == MemoryEventType.READ) {
        return;
      }

      byte[] pointerValue = mote.getMemory().getMemorySegment(pointerAddress, pointerSize);
      int segmentAddress = mote.getMemory().parseInt(pointerValue);
      if (segmentAddress == lastSegmentAddress) {
        return;
      }

      /* Pointer changed - we need to create new segment monitor */
      segmentMonitor.dispose();
      try {
        registerSegmentMonitor(segmentMonitor.getSize(), true);
      } catch (Exception e) {
        logger.warn("Could not re-register memory monitor on: " + mote, e);
      }
    }

    public MemoryMonitorType getType() {
      return MemoryMonitorType.POINTER;
    }

    public void dispose() {
      super.dispose();
      segmentMonitor.dispose();
    }
  }

  static class SegmentMemoryMonitor implements se.sics.cooja.MoteMemory.MemoryMonitor {
    protected final BufferListener bl;
    protected final Mote mote;

    private final int address;
    private final int size;

    private byte[] oldData = null;

    public SegmentMemoryMonitor(BufferListener bl, Mote mote, int address, int size)
    throws Exception {
      this.bl = bl;
      this.mote = mote;
      this.address = address;
      this.size = size;

      if (address != 0) {
        if (!mote.getMemory().addMemoryMonitor(address, size, this)) {
          throw new Exception("Could not register memory monitor on: " + mote);
        }
      }
    }

    public Mote getMote() {
      return mote;
    }
    public int getAddress() {
      return address;
    }
    public int getSize() {
      return size;
    }
    public MemoryMonitorType getType() {
      return MemoryMonitorType.SEGMENT;
    }

    public void dispose() {
      if (address != 0) {
        mote.getMemory().removeMemoryMonitor(address, size, this);
      }
    }

    public void memoryChanged(MoteMemory memory, MemoryEventType type, int address) {
      byte[] newData = getAddress()==0?null:mote.getMemory().getMemorySegment(getAddress(), getSize());
      addBufferAccess(bl, mote, oldData, newData, type, this.address);
      oldData = newData;
    }

    void addBufferAccess(BufferListener bl, Mote mote, byte[] oldData, byte[] newData, MemoryEventType type, int address) {
      BufferAccess ba = new BufferAccess(
          mote,
          mote.getSimulation().getSimulationTime(),
          address,
          newData,
          oldData,
          type,
          bl.withStackTrace
      );
      bl.logUpdateAggregator.add(ba);
    }
  }

  private void stopObserving(Mote mote) {
    for (SegmentMemoryMonitor mm: memoryMonitors.toArray(new SegmentMemoryMonitor[0])) {
      if (mm.getMote() == mote) {
        mm.dispose();
        memoryMonitors.remove(mm);
      }
    }

    motes.remove(mote);
    updateTitle();
  }

  private void repaintTimeColumn() {
    logTable.getColumnModel().getColumn(COLUMN_TIME).setHeaderValue(
        logTable.getModel().getColumnName(COLUMN_TIME));
    repaint();
  }

  private void updateTitle() {
    if (buffer != null) {
      String status = buffer.getStatusString();
      setTitle("Buffer Listener - " +
          ((status!=null)?status:GUI.getDescriptionOf(buffer)) + " " +
          "- " + memoryMonitors.size() + " buffers on " + motes.size() + " motes");
    }
  }

  public void closePlugin() {
    if (hourTimeEvent != null) hourTimeEvent.remove();

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
    if (withStackTrace) {
      element = new Element("stacktrace");
      config.add(element);
    }
    element = new Element("parser");
    element.setText(parser.getClass().getName());
    config.add(element);

    element = new Element("buffer");
    element.setText(buffer.getClass().getName());
    buffer.writeConfig(element);
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
      } else if ("stacktrace".equals(name)) {
        withStackTrace = true;
        withStackTraceCheckbox.setSelected(true);
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
        Class<? extends Buffer> btClass =
          simulation.getGUI().tryLoadClass(this, Buffer.class, bufferClassname);
        if (btClass == null) {
          logger.warn("Could not create buffer parser: could not find class: " + bufferClassname);
        } else {
          Buffer b = createBufferInstance(btClass);
          if (b != null) {
            b.applyConfig(element);
            setBuffer(b);
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
            if (logs.get(row).type == MemoryEventType.READ) {
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

  public static class BufferAccess {
    public static final byte[] NULL_DATA = new byte[0];

    public final Mote mote;
    public final long time;

    public final byte[] mem;
    private boolean[] accessedBitpattern = null;

    public final MemoryEventType type;
    public final String sourceStr;
    public final String stackTrace;
    public final int address;

    public BufferAccess(
        Mote mote, long time, int address, byte[] newData, byte[] oldData, MemoryEventType type, boolean withStackTrace) {
      this.mote = mote;
      this.time = time;
      this.mem = newData==null?NULL_DATA:newData;
      this.type = type;
      this.address = address;

      /* Generate diff bit pattern */
      if (newData != null && oldData != null) {
        accessedBitpattern = new boolean[newData.length];
        for (int i=0; i < newData.length; i++) {
          accessedBitpattern[i] = (oldData[i] != mem[i]);
        }
      } else if (newData != null) {
        accessedBitpattern = new boolean[newData.length];
        Arrays.fill(accessedBitpattern, true);
      }

      if (mote instanceof AbstractEmulatedMote) {
        String s = ((AbstractEmulatedMote)mote).getPCString();
        sourceStr = s==null?"[unknown]":s;
        if (withStackTrace) {
          this.stackTrace = ((AbstractEmulatedMote)mote).getStackTrace();
        } else {
          this.stackTrace = null;
        }
      } else {
        this.sourceStr = "[unknown]";
        this.stackTrace = null;
      }
    }

    public String getAsHex() {
      return String.format("%04x", address) + ":" + StringUtils.toHex(mem);
    }

    public boolean[] getAccessedBitpattern() {
      return accessedBitpattern;
    }

    public String getType() {
      return type.toString();
    }

    public Object getSource() {
      return sourceStr;
    }

    public String getID() {
      return "ID:" + mote.getID();
    }

    public String getTime(boolean formatTimeString, boolean hasHours) {
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
            sb.append(ba.getAsHex());
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
          sb.append(ba.getAsHex());
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
          sb.append(ba.getAsHex());
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

      Buffer b = createBufferInstance(btClass);
      if (b != null) {
        if (b.configure(BufferListener.this)) {
          setBuffer(b);
        }
      }
    }
  };
  private void updateBufferMenu() {
    bufferMenu.removeAll();

    for (Class<? extends Buffer> btClass: bufferTypes) {
      if (btClass == CustomVariableBuffer.class) {
        bufferMenu.addSeparator();
      }
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
    logTable.getColumnModel().getColumn(COLUMN_DATA).setHeaderValue(GUI.getDescriptionOf(bp));

    repaint();
  }

  private static class BufferInput {
    private JPanel mainPanel = new JPanel();
    private JTextField textName = new JTextField();
    private JTextField textSize = new JTextField();
    private JTextField textOffset = new JTextField();

    public BufferInput(String name, String size, String offset) {
      mainPanel.setLayout(new GridLayout(3, 2, 5, 5));

      if (name != null) {
        textName.setText(name);
        mainPanel.add(new JLabel("Symbol:"));
        mainPanel.add(textName);
      }
      if (size != null) {
        textSize.setText(size);
        mainPanel.add(new JLabel("Size (1-" + MAX_BUFFER_SIZE + "):"));
        mainPanel.add(textSize);
      }
      if (size != null) {
        textOffset.setText(offset);
        mainPanel.add(new JLabel("Offset"));
        mainPanel.add(textOffset);
      }
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

  private static Buffer createBufferInstance(Class<? extends Buffer> btClass) {
    try {
      return btClass.newInstance();
    } catch (InstantiationException e) {
      logger.warn("Could not create buffer type: " + e.getMessage(), e);
      return null;
    } catch (IllegalAccessException e) {
      logger.warn("Could not create buffer type: " + e.getMessage(), e);
      return null;
    }
  }

  private void setBuffer(Buffer buffer) {
    if (buffer == null) {
      return;
    }

    this.buffer = buffer;

    /* Reregister memory monitors */
    for (Mote m: simulation.getMotes()) {
      stopObserving(m);
    }
    for (Mote m: simulation.getMotes()) {
      try {
        startMonitoring(m);
      } catch (Exception e) {
        logger.warn("Could not monitor buffer on: " + m, e);
      }
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
    public Object parse(BufferAccess ba);
  }
  public static abstract class GraphicalParser implements Parser {
    BufferAccess ba = null;
    public Object parse(BufferAccess ba) {
      this.ba = ba;
      return ba;
    }
    public abstract void paintComponent(Graphics g, JComponent c);
    public abstract int getUnscaledWidth();
  }

  public static abstract class StringParser implements Parser {
    public Object parse(BufferAccess ba) {
      return parseString(ba);
    }

    public abstract String parseString(BufferAccess ba);
  }

  public static interface Buffer {
    public int getAddress(Mote mote);
    public int getSize(Mote mote);

    public String getStatusString();

    public SegmentMemoryMonitor createMemoryMonitor(BufferListener bl, Mote mote)
    throws Exception;

    /*
     * Called when buffer is created by user to allow user input (AWT thread)
     */
    public boolean configure(BufferListener bl);

    /*
     * Called when buffer is created from config
     */
    public void applyConfig(Element element);

    public void writeConfig(Element element);
  }
  public static abstract class AbstractBuffer implements Buffer {
    public String getStatusString() {
      return null;
    }
    public void writeConfig(Element element) {
    }
    public void applyConfig(Element element) {
    }
    public boolean configure(BufferListener bl) {
      return true;
    }
  }

  public static abstract class PointerBuffer extends AbstractBuffer {
    public abstract int getPointerAddress(Mote mote);

    public SegmentMemoryMonitor createMemoryMonitor(BufferListener bl, Mote mote)
    throws Exception {
      return new PointerMemoryMonitor(
          bl,
          mote,
          getPointerAddress(mote),
          mote.getMemory().getIntegerLength(),
          getSize(mote)
      );
    }
  }
  public static abstract class SegmentBuffer extends AbstractBuffer {
    public SegmentMemoryMonitor createMemoryMonitor(BufferListener bl, Mote mote)
    throws Exception {
      return new SegmentMemoryMonitor(
          bl,
          mote,
          getAddress(mote),
          getSize(mote)
      );
    }
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
    public String parseString(BufferAccess ba) {
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
    public String parseString(BufferAccess ba) {
      StringBuilder sb = new StringBuilder();

      int intLen = ba.mote.getMemory().getIntegerLength();
      sb.append("<html>");
      for (int i=0; i < ba.mem.length/intLen; i++) {
        byte[] mem = Arrays.copyOfRange(ba.mem, i*intLen,(i+1)*intLen);
        boolean[] diff = Arrays.copyOfRange(ba.getAccessedBitpattern(), i*intLen,(i+1)*intLen);
        int val = ba.mote.getMemory().parseInt(mem);

        boolean red = false;
        for (boolean changed: diff) {
          if (changed) red = true;
        }

        if (red) {
          sb.append("<font color=\"red\">");
        }
        sb.append(val + " ");
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
    public String parseString(BufferAccess ba) {
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
    public String parseString(BufferAccess ba) {
      /* TODO Diff? */
      return new String(ba.mem).replaceAll("[^\\p{Print}]", "");
    }
  }

  @ClassDescription("IPv6 address")
  public static class IPv6AddressParser extends StringParser {
    public String parseString(BufferAccess ba) {
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
    public String parseString(BufferAccess ba) {
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
        int v = 0xff&ba.mem[x];
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
  public static class NodeIDBuffer extends SegmentBuffer {
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists("node_id")) {
        return -1;
      }
      return mote.getMemory().getVariableAddress("node_id");
    }
    public int getSize(Mote mote) {
      return mote.getMemory().getIntegerLength();
    }

  }

  @ClassDescription("Queuebuf 0 RAM")
  public static class Queuebuf0Buffer extends SegmentBuffer {
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists("buframmem")) {
        return -1;
      }
      int offset = 0;
      return mote.getMemory().getVariableAddress("buframmem") + offset;
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }

  @ClassDescription("packetbuf_aligned")
  public static class PacketbufBuffer extends SegmentBuffer {
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists("packetbuf_aligned")) {
        return -1;
      }
      return mote.getMemory().getVariableAddress("packetbuf_aligned");
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }

  @ClassDescription("*packetbufptr")
  public static class PacketbufPointerBuffer extends PointerBuffer {
    public int getPointerAddress(Mote mote) {
      if (!mote.getMemory().variableExists("packetbufptr")) {
        return -1;
      }
      return mote.getMemory().getVariableAddress("packetbufptr");
    }
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists("packetbufptr")) {
        return -1;
      }
      return mote.getMemory().getIntValueOf("packetbufptr");
    }
    public int getSize(Mote mote) {
      return 128;
    }
  }

  @ClassDescription("Pointer...")
  public static class CustomPointerBuffer extends PointerBuffer {
    public String variable;
    public int size;
    public int offset;
    public int getPointerAddress(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return mote.getMemory().getVariableAddress(variable);
    }
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return mote.getMemory().getIntValueOf(variable)+offset;
    }
    public int getSize(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return size;
    }

    public String getStatusString() {
      if (offset > 0) {
        return "Pointer *" + variable + "[" + offset + "] (" + size + ")";
      } else {
        return "Pointer *" + variable + " (" + size + ")";
      }
    }

    public void writeConfig(Element element) {
      element.setAttribute("variable", variable);
      element.setAttribute("size", "" + size);
      element.setAttribute("offset", "" + offset);
    }
    public void applyConfig(Element element) {
      variable = element.getAttributeValue("variable");
      size = Integer.parseInt(element.getAttributeValue("size"));
      offset = Integer.parseInt(element.getAttributeValue("offset"));
    }
    public boolean configure(BufferListener bl) {
      String suggestName = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARNAME", "node_id");
      String suggestSize = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "2");
      String suggestOffset = GUI.getExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "0");
      BufferInput infoComponent =
        new BufferInput(suggestName, suggestSize, suggestOffset);

      int result = JOptionPane.showConfirmDialog(bl,
          infoComponent.getComponent(),
          "Symbol info",
          JOptionPane.OK_CANCEL_OPTION);
      if (result != JOptionPane.OK_OPTION) {
        /* Abort */
        return false;
      }
      variable = infoComponent.getName();
      if (variable == null) {
        return false;
      }
      try {
        size = Integer.parseInt(infoComponent.getSize());
        if (size < 1 || size > MAX_BUFFER_SIZE) {
          /* Abort */
          logger.fatal("Bad buffer size " + infoComponent.getSize() + ": min 1, max " + MAX_BUFFER_SIZE);
          return false;
        }
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer size " + infoComponent.getSize() + ": " + e.getMessage(), e);
        return false;
      }
      try {
        offset = Integer.parseInt(infoComponent.getOffset());
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer offset " + infoComponent.getOffset() + ": " + e.getMessage(), e);
        /* Abort */
        return false;
      }

      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARNAME", variable);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "" + size);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "" + offset);
      return true;
    }
  }

  @ClassDescription("Symbol...")
  public static class CustomVariableBuffer extends SegmentBuffer {
    public String variable;
    public int size;
    public int offset;
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return mote.getMemory().getVariableAddress(variable)+offset;
    }
    public int getSize(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return size;
    }

    public String getStatusString() {
      if (offset > 0) {
        return "Symbol &" + variable + "[" + offset + "] (" + size + ")";
      } else {
        return "Symbol " + variable + " (" + size + ")";
      }
    }

    public void writeConfig(Element element) {
      element.setAttribute("variable", variable);
      element.setAttribute("size", "" + size);
      element.setAttribute("offset", "" + offset);
    }
    public void applyConfig(Element element) {
      variable = element.getAttributeValue("variable");
      size = Integer.parseInt(element.getAttributeValue("size"));
      offset = Integer.parseInt(element.getAttributeValue("offset"));
    }
    public boolean configure(BufferListener bl) {
      String suggestName = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARNAME", "node_id");
      String suggestSize = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "2");
      String suggestOffset = GUI.getExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "0");
      BufferInput infoComponent =
        new BufferInput(suggestName, suggestSize, suggestOffset);

      int result = JOptionPane.showConfirmDialog(bl,
          infoComponent.getComponent(),
          "Symbol info",
          JOptionPane.OK_CANCEL_OPTION);
      if (result != JOptionPane.OK_OPTION) {
        /* Abort */
        return false;
      }
      variable = infoComponent.getName();
      if (variable == null) {
        return false;
      }
      try {
        size = Integer.parseInt(infoComponent.getSize());
        if (size < 1 || size > MAX_BUFFER_SIZE) {
          /* Abort */
          logger.fatal("Bad buffer size " + infoComponent.getSize() + ": min 1, max " + MAX_BUFFER_SIZE);
          return false;
        }
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer size " + infoComponent.getSize() + ": " + e.getMessage(), e);
        return false;
      }
      try {
        offset = Integer.parseInt(infoComponent.getOffset());
      } catch (RuntimeException e) {
        logger.fatal("Failed parsing buffer offset " + infoComponent.getOffset() + ": " + e.getMessage(), e);
        /* Abort */
        return false;
      }
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARNAME", variable);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARSIZE", "" + size);
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VAROFFSET", "" + offset);
      return true;
    }
  }

  @ClassDescription("Integer...")
  public static class CustomIntegerBuffer extends SegmentBuffer {
    public String variable;
    public int getAddress(Mote mote) {
      if (!mote.getMemory().variableExists(variable)) {
        return -1;
      }
      return mote.getMemory().getVariableAddress(variable);
    }
    public int getSize(Mote mote) {
      return mote.getMemory().getIntegerLength();
    }

    public String getStatusString() {
      return "Integer " + variable;
    }

    public void writeConfig(Element element) {
      element.setAttribute("variable", variable);
    }
    public void applyConfig(Element element) {
      variable = element.getAttributeValue("variable");
    }
    public boolean configure(BufferListener bl) {
      String suggestName = GUI.getExternalToolsSetting("BUFFER_LISTENER_VARNAME", "node_id");
      BufferInput infoComponent =
        new BufferInput(suggestName, null, null);

      int result = JOptionPane.showConfirmDialog(bl,
          infoComponent.getComponent(),
          "Symbol info",
          JOptionPane.OK_CANCEL_OPTION);
      if (result != JOptionPane.OK_OPTION) {
        /* Abort */
        return false;
      }
      variable = infoComponent.getName();
      if (variable == null) {
        return false;
      }
      GUI.setExternalToolsSetting("BUFFER_LISTENER_VARNAME", variable);
      return true;
    }
  }
}
