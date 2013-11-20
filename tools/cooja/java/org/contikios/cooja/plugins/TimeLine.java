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
 */

package org.contikios.cooja.plugins;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JCheckBox;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JToolTip;
import javax.swing.KeyStroke;
import javax.swing.Popup;
import javax.swing.PopupFactory;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.HasQuickHelp;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Plugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.SimEventCentral.LogOutputEvent;
import org.contikios.cooja.SimEventCentral.LogOutputListener;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.Watchpoint;
import org.contikios.cooja.WatchpointMote;
import org.contikios.cooja.WatchpointMote.WatchpointListener;
import org.contikios.cooja.interfaces.LED;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.interfaces.Radio.RadioEvent;
import org.contikios.cooja.motes.AbstractEmulatedMote;

/**
 * Shows events such as mote logs, LEDs, and radio transmissions, in a timeline.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Timeline")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class TimeLine extends VisPlugin implements HasQuickHelp {
  private static final long serialVersionUID = -883154261246961973L;
  public static final int LED_PIXEL_HEIGHT = 2;
  public static final int EVENT_PIXEL_HEIGHT = 4;
  public static final int TIME_MARKER_PIXEL_HEIGHT = 6;
  public static final int FIRST_MOTE_PIXEL_OFFSET = TIME_MARKER_PIXEL_HEIGHT + EVENT_PIXEL_HEIGHT;

  private static final Color COLOR_BACKGROUND = Color.WHITE;
  private static final boolean PAINT_ZERO_WIDTH_EVENTS = true;
  private static final int TIMELINE_UPDATE_INTERVAL = 100;

  private double currentPixelDivisor = 200;

  private static final long[] ZOOM_LEVELS = {
  	1, 2, 5, 10,
  	20, 50, 100, 200, 500, 1000,
  	2000, 5000, 10000, 20000, 50000, 100000 };

  private boolean needZoomOut = false;

  private static Logger logger = Logger.getLogger(TimeLine.class);

  private int paintedMoteHeight = EVENT_PIXEL_HEIGHT;

  private Simulation simulation;
  private LogOutputListener newMotesListener;
  
  /* Expermental features: Use currently active plugin to filter Timeline Log outputs */
  private LogListener logEventFilterPlugin = null;

  private JScrollPane timelineScrollPane;
  private MoteRuler timelineMoteRuler;
  private JComponent timeline;

  private Observer moteHighlightObserver = null;
  private ArrayList<Mote> highlightedMotes = new ArrayList<Mote>();
  private final static Color HIGHLIGHT_COLOR = Color.CYAN;

  private ArrayList<MoteObservation> activeMoteObservers = new ArrayList<MoteObservation>();

  private ArrayList<MoteEvents> allMoteEvents = new ArrayList<MoteEvents>();

  private boolean showRadioRXTX = true;
  private boolean showRadioChannels = false;
  private boolean showRadioOnoff = true;
  private boolean showLeds = true;
  private boolean showLogOutputs = false;
  private boolean showWatchpoints = false;

  private Point popupLocation = null;

  private JCheckBox showWatchpointsCheckBox;
  private JCheckBox showLogsCheckBox;
  private JCheckBox showLedsCheckBox;
  private JCheckBox showRadioOnoffCheckbox;
  private JCheckBox showRadioChannelsCheckbox;
  private JCheckBox showRadioTXRXCheckbox;

  /**
   * @param simulation Simulation
   * @param gui GUI
   */
  public TimeLine(final Simulation simulation, final Cooja gui) {
    super("Timeline", gui);
    this.simulation = simulation;

    currentPixelDivisor = ZOOM_LEVELS[ZOOM_LEVELS.length/2];

    /* Menus */
    JMenuBar menuBar = new JMenuBar();
    JMenu fileMenu = new JMenu("File");
    JMenu editMenu = new JMenu("Edit");
    JMenu motesMenu = new JMenu("Motes");
    JMenu eventsMenu = new JMenu("Events");
    JMenu viewMenu = new JMenu("View");
    JMenu zoomMenu = new JMenu("Zoom");

    menuBar.add(fileMenu);
    menuBar.add(editMenu);
    menuBar.add(viewMenu);
    menuBar.add(zoomMenu);
    menuBar.add(eventsMenu);
    menuBar.add(motesMenu);

    this.setJMenuBar(menuBar);

    motesMenu.add(new JMenuItem(addMoteAction));
    zoomMenu.add(new JMenuItem(zoomInAction));
    zoomMenu.add(new JMenuItem(zoomOutAction));
    zoomMenu.add(new JMenuItem(zoomSliderAction));
    viewMenu.add(new JCheckBoxMenuItem(executionDetailsAction) {
	    private static final long serialVersionUID = 8314556794750277113L;
	    public boolean isSelected() {
      		return executionDetails;
	    }
    });

    fileMenu.add(new JMenuItem(saveDataAction));
    fileMenu.add(new JMenuItem(statisticsAction));
    editMenu.add(new JMenuItem(clearAction));

    showRadioTXRXCheckbox = createEventCheckbox("Radio traffic", "Show radio transmissions, receptions, and collisions");
    showRadioTXRXCheckbox.setName("showRadioRXTX");
    showRadioTXRXCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioRXTX = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showRadioTXRXCheckbox);
    showRadioOnoffCheckbox = createEventCheckbox("Radio on/off", "Show radio hardware state");
    showRadioOnoffCheckbox.setSelected(showRadioOnoff);
    showRadioOnoffCheckbox.setName("showRadioHW");
    showRadioOnoffCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioOnoff = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showRadioOnoffCheckbox);
    showRadioChannelsCheckbox = createEventCheckbox("Radio channel", "Show different radio channels");
    showRadioChannelsCheckbox.setSelected(showRadioChannels);
    showRadioChannelsCheckbox.setName("showRadioChannels");
    showRadioChannelsCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioChannels = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showRadioChannelsCheckbox);
    showLedsCheckBox = createEventCheckbox("LEDs", "Show LED state");
    showLedsCheckBox.setSelected(showLeds);
    showLedsCheckBox.setName("showLEDs");
    showLedsCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLeds = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showLedsCheckBox);
    showLogsCheckBox = createEventCheckbox("Log output", "Show mote log output, such as printf()'s");
    showLogsCheckBox.setSelected(showLogOutputs);
    showLogsCheckBox.setName("showLogOutput");
    showLogsCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLogOutputs = ((JCheckBox) e.getSource()).isSelected();
        
        /* Check whether there is an active log listener that is used to filter logs */
        logEventFilterPlugin = (LogListener) simulation.getCooja().getPlugin(
            LogListener.class.getName());
        if (showLogOutputs) {
          if (logEventFilterPlugin != null) {
            logger.info("Filtering shown log outputs by use of " + Cooja.getDescriptionOf(LogListener.class) + " plugin");
          } else {
            logger.info("No active " + Cooja.getDescriptionOf(LogListener.class) + " plugin, not filtering log outputs");
          }
        }
        
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showLogsCheckBox);
    showWatchpointsCheckBox = createEventCheckbox("Watchpoints", "Show code watchpoints (for emulated motes)");
    showWatchpointsCheckBox.setSelected(showWatchpoints);
    showWatchpointsCheckBox.setName("showWatchpoints");
    showWatchpointsCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showWatchpoints = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventsMenu.add(showWatchpointsCheckBox);

    /* Box: events to observe */

    /* Panel: timeline canvas w. scroll pane and add mote button */
    timeline = new Timeline();
    timelineScrollPane = new JScrollPane(
        timeline,
        JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
        JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    timelineScrollPane.getHorizontalScrollBar().setUnitIncrement(50);

    timelineMoteRuler = new MoteRuler();
    timelineScrollPane.setRowHeaderView(timelineMoteRuler);
    timelineScrollPane.setBackground(Color.WHITE);

    /* Zoom in/out via keyboard*/
    getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_PLUS, KeyEvent.CTRL_DOWN_MASK), "zoomIn");
    getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_EQUALS, KeyEvent.SHIFT_DOWN_MASK | KeyEvent.CTRL_DOWN_MASK), "zoomIn");
    getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_ADD, KeyEvent.CTRL_DOWN_MASK), "zoomIn");
    getActionMap().put("zoomIn", zoomInAction);
    getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_MINUS, KeyEvent.CTRL_DOWN_MASK), "zoomOut");
    getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_SUBTRACT, KeyEvent.CTRL_DOWN_MASK), "zoomOut");
    getActionMap().put("zoomOut", zoomOutAction);

    /*    getContentPane().add(splitPane);*/
    getContentPane().add(timelineScrollPane);

    recalculateMoteHeight();
    pack();

    numberMotesWasUpdated();

    /* Automatically add/delete motes.
     * This listener also observes mote log outputs. */
    simulation.getEventCentral().addLogOutputListener(newMotesListener = new LogOutputListener() {
      public void moteWasAdded(Mote mote) {
        addMote(mote);
      }
      public void moteWasRemoved(Mote mote) {
        removeMote(mote);
      }
      public void removedLogOutput(LogOutputEvent ev) {
      }
      public void newLogOutput(LogOutputEvent ev) {
        /* Log output */
        Mote mote = ev.getMote();
        LogEvent logEvent = new LogEvent(ev);
        
        /* TODO Optimize */
        for (MoteEvents moteEvents: allMoteEvents) {
          if (moteEvents.mote == mote) {
            moteEvents.addLog(logEvent);
            break;
          }
        }
      }
    });
    for (Mote m: simulation.getMotes()) {
      addMote(m);
    }

    /* Update timeline for the duration of the plugin */
    repaintTimelineTimer.start();

    gui.addMoteHighlightObserver(moteHighlightObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!(obj instanceof Mote)) {
          return;
        }

        final Timer timer = new Timer(100, null);
        final Mote mote = (Mote) obj;
        timer.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            /* Count down */
            if (timer.getDelay() < 90) {
              timer.stop();
              highlightedMotes.remove(mote);
              repaint();
              return;
            }

            /* Toggle highlight state */
            if (highlightedMotes.contains(mote)) {
              highlightedMotes.remove(mote);
            } else {
              highlightedMotes.add(mote);
            }
            timer.setDelay(timer.getDelay()-1);
            repaint();
          }
        });
        timer.start();
      }
    });

    /* XXX HACK: here we set the position and size of the window when it appears on a blank simulation screen. */
    this.setLocation(0, gui.getDesktopPane().getHeight() - 166);
    this.setSize(gui.getDesktopPane().getWidth(), 166);
  }

  public void startPlugin() {
      super.startPlugin();
      
      showWatchpointsCheckBox.setSelected(showWatchpoints);
      showLogsCheckBox.setSelected(showLogOutputs);
      showLedsCheckBox.setSelected(showLeds);
      showRadioOnoffCheckbox.setSelected(showRadioOnoff);
      showRadioChannelsCheckbox.setSelected(showRadioChannels);
      showRadioTXRXCheckbox.setSelected(showRadioRXTX);
  }

  private JCheckBox createEventCheckbox(String text, String tooltip) {
    JCheckBox checkBox = new JCheckBox(text, true);
    checkBox.setToolTipText(tooltip);
    return checkBox;
  }

  private Action removeMoteAction = new AbstractAction() {
    private static final long serialVersionUID = 2924285037480429045L;
    public void actionPerformed(ActionEvent e) {
      JComponent b = (JComponent) e.getSource();
      Mote m = (Mote) b.getClientProperty("mote");
      removeMote(m);
    }
  };
  private Action removeAllOtherMotesAction = new AbstractAction() {
  	private static final long serialVersionUID = 2924285037480429045L;
  	public void actionPerformed(ActionEvent e) {
  		JComponent b = (JComponent) e.getSource();
  		Mote m = (Mote) b.getClientProperty("mote");
  		MoteEvents[] mes = allMoteEvents.toArray(new MoteEvents[0]);
  		for (MoteEvents me: mes) {
  			if (me.mote == m) {
  				continue;
  			}
  			removeMote(me.mote);
  		}
  	}
  };
  private Action sortMoteAction = new AbstractAction() {
    private static final long serialVersionUID = 621116674700872058L;
    public void actionPerformed(ActionEvent e) {
      JComponent b = (JComponent) e.getSource();
      Mote m = (Mote) b.getClientProperty("mote");

      /* Sort by distance */
      ArrayList<MoteEvents> sortedMoteEvents = new ArrayList<MoteEvents>();
      for (MoteEvents me: allMoteEvents.toArray(new MoteEvents[0])) {
        double d = me.mote.getInterfaces().getPosition().getDistanceTo(m);

        int i=0;
        for (i=0; i < sortedMoteEvents.size(); i++) {
          double d2 = m.getInterfaces().getPosition().getDistanceTo(sortedMoteEvents.get(i).mote);
          if (d < d2) {
            break;
          }
        }
        sortedMoteEvents.add(i, me);

      }
      allMoteEvents = sortedMoteEvents;
      numberMotesWasUpdated();
    }
  };
  private Action topMoteAction = new AbstractAction() {
		private static final long serialVersionUID = 4683178751482241843L;
		public void actionPerformed(ActionEvent e) {
  		JComponent b = (JComponent) e.getSource();
  		Mote m = (Mote) b.getClientProperty("mote");

  		/* Sort by distance */
  		MoteEvents mEvent = null;
  		for (MoteEvents me: allMoteEvents.toArray(new MoteEvents[0])) {
  			if (me.mote == m) {
  				mEvent = me;
  				break;
  			}
  		}
  		allMoteEvents.remove(mEvent);
  		allMoteEvents.add(0, mEvent);
  		numberMotesWasUpdated();
  	}
  };
  private Action addMoteAction = new AbstractAction("Show motes...") {
    private static final long serialVersionUID = 7546685285707302865L;
    public void actionPerformed(ActionEvent e) {

      JComboBox<Object> source = new JComboBox<Object>();
      source.addItem("All motes");
      for (Mote m: simulation.getMotes()) {
        source.addItem(m);
      }
      Object description[] = {
          source
      };
      JOptionPane optionPane = new JOptionPane();
      optionPane.setMessage(description);
      optionPane.setMessageType(JOptionPane.QUESTION_MESSAGE);
      String options[] = new String[] {"Cancel", "Show"};
      optionPane.setOptions(options);
      optionPane.setInitialValue(options[1]);
      JDialog dialog = optionPane.createDialog(Cooja.getTopParentContainer(), "Show mote in timeline");
      dialog.setVisible(true);

      if (optionPane.getValue() == null || !optionPane.getValue().equals("Show")) {
        return;
      }

      if (source.getSelectedItem().equals("All motes")) {
        for (Mote m: simulation.getMotes()) {
          addMote(m);
        }
      } else {
        addMote((Mote) source.getSelectedItem());
      }
    }
  };

  private void forceRepaintAndFocus(final long focusTime, final double focusCenter) {
    forceRepaintAndFocus(focusTime, focusCenter, true);
  }

  private void forceRepaintAndFocus(final long focusTime, final double focusCenter, final boolean mark) {
    lastRepaintSimulationTime = -1; /* Force repaint */
    repaintTimelineTimer.getActionListeners()[0].actionPerformed(null); /* Force size update*/
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        int w = timeline.getVisibleRect().width;

        /* centerPixel-leftPixel <=> focusCenter*w; */
        int centerPixel = (int) (focusTime/currentPixelDivisor);
        int leftPixel = (int) (focusTime/currentPixelDivisor - focusCenter*w);

        Rectangle r = new Rectangle(
            leftPixel, 0,
            w, 1
        );

        timeline.scrollRectToVisible(r);

        /* Time ruler */
        if (mark) {
          mousePixelPositionX = centerPixel;
          mouseDownPixelPositionX = centerPixel;
          mousePixelPositionY = timeline.getHeight();
        }
      }
    });
  }

  private int zoomGetLevel (final double zoomDivisor) {
    int zoomLevel = 0;
    while (zoomLevel < ZOOM_LEVELS.length) {
      if (zoomDivisor <= ZOOM_LEVELS[zoomLevel]) break;
      zoomLevel++;
    }
    return zoomLevel;
  }
  private int zoomGetLevel () {
    return zoomGetLevel(currentPixelDivisor);
  }

  private double zoomLevelToDivisor (int zoomLevel) {
    if (0 > zoomLevel) {
      zoomLevel = 0;
    } else if (ZOOM_LEVELS.length <= zoomLevel) {
      zoomLevel = ZOOM_LEVELS.length - 1;
    }
    return ZOOM_LEVELS[zoomLevel];
  }

  private void zoomFinish (final double zoomDivisor,
                           final long focusTime,
                           final double focusCenter) {
    currentPixelDivisor = zoomDivisor;
    String note = "";
    if (ZOOM_LEVELS[0] >= zoomDivisor) {
      currentPixelDivisor = ZOOM_LEVELS[0];
      note = " (MIN)";
    } else if (ZOOM_LEVELS[ZOOM_LEVELS.length-1] <= zoomDivisor) {
      currentPixelDivisor = ZOOM_LEVELS[ZOOM_LEVELS.length-1];
      note = " (MAX)";
    }
    if (zoomDivisor != currentPixelDivisor) {
      logger.info("Zoom level: adjusted out-of-range " + zoomDivisor + " us/pixel");
    }
    logger.info("Zoom level: " + currentPixelDivisor + " microseconds/pixel " + note);

    forceRepaintAndFocus(focusTime, focusCenter);
  }

  private void zoomFinishLevel (final int zoomLevel,
                                final long focusTime,
                                final double focusCenter) {
    final double cpd = zoomLevelToDivisor(zoomLevel);
    zoomFinish(cpd, focusTime, focusCenter);
  }

  private void zoomIn (final long focusTime,
                       final double focusCenter) {
    zoomFinishLevel(zoomGetLevel()-1, focusTime, focusCenter);
  }

  private void zoomOut (final long focusTime,
                        final double focusCenter) {
    zoomFinishLevel(zoomGetLevel()+1, focusTime, focusCenter);
  }

  private Action zoomInAction = new AbstractAction("Zoom in (Ctrl+)") {
    private static final long serialVersionUID = -2592452356547803615L;
    public void actionPerformed(ActionEvent e) {
      Rectangle r = timeline.getVisibleRect();
      int pixelX = r.x + r.width/2;
      if (popupLocation != null) {
        pixelX = popupLocation.x;
        popupLocation = null;
      }
      if (mousePixelPositionX > 0) {
        pixelX = mousePixelPositionX;
      }
      final long centerTime = (long) (pixelX*currentPixelDivisor);
      zoomIn(centerTime, 0.5);
    }
  };

  private Action zoomOutAction = new AbstractAction("Zoom out (Ctrl-)") {
    private static final long serialVersionUID = 6837091379835151725L;
    public void actionPerformed(ActionEvent e) {
      Rectangle r = timeline.getVisibleRect();
      int pixelX = r.x + r.width/2;
      if (popupLocation != null) {
        pixelX = popupLocation.x;
        popupLocation = null;
      }
      if (mousePixelPositionX > 0) {
        pixelX = mousePixelPositionX;
      }
      final long centerTime = (long) (pixelX*currentPixelDivisor);
      zoomOut(centerTime, 0.5);
    }
  };

  private Action zoomSliderAction = new AbstractAction("Zoom slider (Ctrl+Mouse)") {
    private static final long serialVersionUID = -4288046377707363837L;
    public void actionPerformed(ActionEvent e) {
      final int zoomLevel = zoomGetLevel();
      final JSlider zoomSlider = new JSlider(JSlider.VERTICAL, 0, ZOOM_LEVELS.length-1, zoomLevel);
      zoomSlider.setInverted(true);
      zoomSlider.setPaintTicks(true);
      zoomSlider.setPaintLabels(false);

      final long centerTime = (long) (popupLocation.x*currentPixelDivisor);

      zoomSlider.addChangeListener(new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
          final int zoomLevel = zoomSlider.getValue();
          zoomFinishLevel(zoomLevel, centerTime, 0.5);
        }
      });

      final JPopupMenu zoomPopup = new JPopupMenu();
      zoomPopup.add(zoomSlider);
      zoomPopup.show(TimeLine.this, TimeLine.this.getWidth()/2, 0);
      zoomSlider.requestFocus();
    }
  };

  /**
   * Save logged raw data to file for post-processing.
   */
  private Action saveDataAction = new AbstractAction("Save to file...") {
    private static final long serialVersionUID = 975176793514425718L;
    public void actionPerformed(ActionEvent e) {
      JFileChooser fc = new JFileChooser();
      int returnVal = fc.showSaveDialog(Cooja.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        return;
      }
      File saveFile = fc.getSelectedFile();

      if (saveFile.exists()) {
        String s1 = "Overwrite";
        String s2 = "Cancel";
        Object[] options = { s1, s2 };
        int n = JOptionPane.showOptionDialog(
            Cooja.getTopParentContainer(),
            "A file with the same name already exists.\nDo you want to remove it?",
            "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      if (saveFile.exists() && !saveFile.canWrite()) {
        logger.fatal("No write access to file");
        return;
      }

      try {
        BufferedWriter outStream = new BufferedWriter(
            new OutputStreamWriter(
                new FileOutputStream(
                    saveFile)));

        /* Output all events (sorted per mote) */
        for (MoteEvents moteEvents: allMoteEvents) {
          for (MoteEvent ev: moteEvents.ledEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
          for (MoteEvent ev: moteEvents.logEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
          for (MoteEvent ev: moteEvents.radioChannelEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
          for (MoteEvent ev: moteEvents.radioHWEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
          for (MoteEvent ev: moteEvents.radioRXTXEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
          for (MoteEvent ev: moteEvents.watchpointEvents) {
            outStream.write(moteEvents.mote + "\t" + ev.time + "\t" + ev.toString() + "\n");
          }
        }

        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }

    }
  };

  private Action clearAction = new AbstractAction("Clear all timeline data") {
    private static final long serialVersionUID = -4592530582786872403L;
    public void actionPerformed(ActionEvent e) {
      if (simulation.isRunning()) {
        simulation.invokeSimulationThread(new Runnable() {
          public void run() {
            clear();
          }
        });
      } else {
        clear();
      }
    }
  };

  public void clear() {
    for (MoteEvents me : allMoteEvents) {
      me.clear();
    }
    repaint();
  }


  private class MoteStatistics {
    Mote mote;
    long onTimeRedLED = 0, onTimeGreenLED = 0, onTimeBlueLED = 0;
    int nrLogs = 0;
    long radioOn = 0;
    long onTimeRX = 0, onTimeTX = 0, onTimeInterfered = 0;

    public String toString() {
      return toString(true, true, true, true);
    }
    public String toString(boolean logs, boolean leds, boolean radioHW, boolean radioRXTX) {
      long duration = simulation.getSimulationTime(); /* XXX */
      StringBuilder sb = new StringBuilder();
      String moteDesc = (mote!=null?"" + mote.getID():"AVERAGE") + " ";
      if (logs) {
        sb.append(moteDesc + "nr_logs " + nrLogs + "\n");
      }
      if (leds) {
        sb.append(moteDesc + "led_red " + onTimeRedLED + " us " + 100.0*((double)onTimeRedLED/duration) + " %\n");
        sb.append(moteDesc + "led_green " + onTimeGreenLED + " us " + 100.0*((double)onTimeGreenLED/duration) + " %\n");
        sb.append(moteDesc + "led_blue " + onTimeBlueLED + " us " + 100.0*((double)onTimeBlueLED/duration) + " %\n");
      }
      if (radioHW) {
        sb.append(moteDesc + "radio_on " + radioOn + " us " + 100.0*((double)radioOn/duration) + " %\n");
      }
      if (radioRXTX) {
        sb.append(moteDesc + "radio_tx " + onTimeTX + " us " + 100.0*((double)onTimeTX/duration) + " %\n");
        sb.append(moteDesc + "radio_rx " + onTimeRX + " us " + 100.0*((double)onTimeRX/duration) + " %\n");
        sb.append(moteDesc + "radio_int " + onTimeInterfered + " us " + 100.0*((double)onTimeInterfered/duration) + " %\n");
      }
      return sb.toString();
    }
  }
  private Action statisticsAction = new AbstractAction("Print statistics to console") {
    private static final long serialVersionUID = 8671605486913497397L;
    public void actionPerformed(ActionEvent e) {
      if (simulation.isRunning()) {
        simulation.stopSimulation();
      }
      logger.info(extractStatistics());
    }
  };

  public String extractStatistics() {
    return extractStatistics(true, true, true, true);
  }
  public synchronized String extractStatistics(
      boolean logs, boolean leds, boolean radioHW, boolean radioRXTX) {
    StringBuilder output = new StringBuilder();

    /* Process all events (per mote basis) */
    ArrayList<MoteStatistics> allStats = new ArrayList<MoteStatistics>();
    for (MoteEvents moteEvents: allMoteEvents) {
      MoteStatistics stats = new MoteStatistics();
      allStats.add(stats);
      stats.mote = moteEvents.mote;

      if (leds) {
        for (MoteEvent ev: moteEvents.ledEvents) {
          if (!(ev instanceof LEDEvent)) continue;
          LEDEvent ledEvent = (LEDEvent) ev;

          /* Red */
          if (ledEvent.red) {
            /* LED is on, add time interval */
            if (ledEvent.next == null) {
              stats.onTimeRedLED += (simulation.getSimulationTime() - ledEvent.time);
            } else {
              stats.onTimeRedLED += (ledEvent.next.time - ledEvent.time);
            }
          }

          /* Green */
          if (ledEvent.green) {
            /* LED is on, add time interval */
            if (ledEvent.next == null) {
              stats.onTimeGreenLED += (simulation.getSimulationTime() - ledEvent.time);
            } else {
              stats.onTimeGreenLED += (ledEvent.next.time - ledEvent.time);
            }
          }

          /* Blue */
          if (ledEvent.blue) {
            /* LED is on, add time interval */
            if (ledEvent.next == null) {
              stats.onTimeBlueLED += (simulation.getSimulationTime() - ledEvent.time);
            } else {
              stats.onTimeBlueLED += (ledEvent.next.time - ledEvent.time);
            }
          }
        }
      }

      if (logs) {
        for (MoteEvent ev: moteEvents.logEvents) {
          if (!(ev instanceof LogEvent)) continue;
          stats.nrLogs++;
        }
      }

      if (radioHW) {
        for (MoteEvent ev: moteEvents.radioHWEvents) {
          if (!(ev instanceof RadioHWEvent)) continue;
          RadioHWEvent hwEvent = (RadioHWEvent) ev;
          if (hwEvent.on) {
            /* HW is on */
            if (hwEvent.next == null) {
              stats.radioOn += (simulation.getSimulationTime() - hwEvent.time);
            } else {
              stats.radioOn += (hwEvent.next.time - hwEvent.time);
            }
          }
        }
      }

      if (radioRXTX) {
        for (MoteEvent ev: moteEvents.radioRXTXEvents) {
          if (!(ev instanceof RadioRXTXEvent)) continue;
          RadioRXTXEvent rxtxEvent = (RadioRXTXEvent) ev;
          if (rxtxEvent.state == RXTXRadioEvent.IDLE) {
            continue;
          }

          long diff;
          if (rxtxEvent.next == null) {
            diff = (simulation.getSimulationTime() - rxtxEvent.time);
          } else {
            diff = (rxtxEvent.next.time - rxtxEvent.time);
          }

          if (rxtxEvent.state == RXTXRadioEvent.TRANSMITTING) {
            stats.onTimeTX += diff;
            continue;
          }
          if (rxtxEvent.state == RXTXRadioEvent.INTERFERED) {
            stats.onTimeInterfered += diff;
            continue;
          }
          if (rxtxEvent.state == RXTXRadioEvent.RECEIVING) {
            stats.onTimeRX += diff;
            continue;
          }
        }
      }

      output.append(stats.toString(logs, leds, radioHW, radioRXTX));
    }

    if (allStats.size() == 0) {
      return output.toString();
    }

    /* Average */
    MoteStatistics average = new MoteStatistics();
    for (MoteStatistics stats: allStats) {
      average.onTimeRedLED += stats.onTimeRedLED;
      average.onTimeGreenLED += stats.onTimeGreenLED;
      average.onTimeBlueLED += stats.onTimeBlueLED;
      average.radioOn += stats.radioOn;
      average.onTimeRX += stats.onTimeRX;
      average.onTimeTX += stats.onTimeTX;
      average.onTimeInterfered += stats.onTimeInterfered;
    }
    average.onTimeBlueLED /= allStats.size();
    average.onTimeGreenLED /= allStats.size();
    average.onTimeBlueLED /= allStats.size();
    average.radioOn /= allStats.size();
    average.onTimeRX /= allStats.size();
    average.onTimeTX /= allStats.size();
    average.onTimeInterfered /= allStats.size();

    output.append(average.toString(logs, leds, radioHW, radioRXTX));
    return output.toString();
  }

  public void trySelectTime(final long toTime) {
    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        /* Mark selected time in time ruler */
        final int toPixel = (int) (toTime / currentPixelDivisor);
        mousePixelPositionX = toPixel;
        mouseDownPixelPositionX = toPixel;
        mousePixelPositionY = timeline.getHeight();

        /* Check if time is already visible */
        Rectangle vis = timeline.getVisibleRect();
        if (toPixel >= vis.x && toPixel < vis.x + vis.width) {
          repaint();
          return;
        }

        forceRepaintAndFocus(toTime, 0.5, false);
      }
    });
  }

  private Action radioLoggerAction = new AbstractAction("Show in " + Cooja.getDescriptionOf(RadioLogger.class)) {
    private static final long serialVersionUID = 7690116136861949864L;
    public void actionPerformed(ActionEvent e) {
      if (popupLocation == null) {
        return;
      }
      long time = (long) (popupLocation.x*currentPixelDivisor);

      Plugin[] plugins = simulation.getCooja().getStartedPlugins();
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
  private Action logListenerAction = new AbstractAction("Show in " + Cooja.getDescriptionOf(LogListener.class)) {
    private static final long serialVersionUID = -8626118368774023257L;
    public void actionPerformed(ActionEvent e) {
      if (popupLocation == null) {
        return;
      }
      long time = (long) (popupLocation.x*currentPixelDivisor);

      Plugin[] plugins = simulation.getCooja().getStartedPlugins();
      for (Plugin p: plugins) {
      	if (!(p instanceof LogListener)) {
      		continue;
      	}

        /* Select simulation time */
        LogListener plugin = (LogListener) p;
        plugin.trySelectTime(time);
      }
    }
  };

  private Action showInAllAction = new AbstractAction("Show in " + Cooja.getDescriptionOf(LogListener.class) + " and " + Cooja.getDescriptionOf(RadioLogger.class)) {
    private static final long serialVersionUID = -2458733078524773995L;
    public void actionPerformed(ActionEvent e) {
      logListenerAction.actionPerformed(null);
      radioLoggerAction.actionPerformed(null);
    }
  };

  private boolean executionDetails = false;
  private Action executionDetailsAction = new AbstractAction("Show execution details in tooltips") {
    private static final long serialVersionUID = -8626118368774023257L;
    public void actionPerformed(ActionEvent e) {
    	executionDetails = !executionDetails;
    }
  };

  private void numberMotesWasUpdated() {
    /* Plugin title */
    if (allMoteEvents.isEmpty()) {
      setTitle("Timeline");
    } else {
      setTitle("Timeline showing " + allMoteEvents.size() + " motes");
    }
    timelineMoteRuler.revalidate();
    timelineMoteRuler.repaint();
    timeline.revalidate();
    timeline.repaint();
  }

  /* XXX Keeps track of observed mote interfaces */
  class MoteObservation {
    private Observer observer;
    private Observable observable;
    private Mote mote;

    private WatchpointMote watchpointMote; /* XXX */
    private WatchpointListener watchpointListener; /* XXX */

    public MoteObservation(Mote mote, Observable observable, Observer observer) {
      this.mote = mote;
      this.observable = observable;
      this.observer = observer;
    }

    /* XXX Special case, should be generalized */
    public MoteObservation(Mote mote, WatchpointMote watchpointMote, WatchpointListener listener) {
      this.mote = mote;
      this.watchpointMote = watchpointMote;
      this.watchpointListener = listener;
    }

    public Mote getMote() {
      return mote;
    }

    /**
     * Disconnect observer from observable (stop observing) and clean up resources (remove pointers).
     */
    public void dispose() {
      if (observable != null) {
        observable.deleteObserver(observer);
        mote = null;
        observable = null;
        observer = null;
      }

      /* XXX */
      if (watchpointMote != null) {
        watchpointMote.removeWatchpointListener(watchpointListener);
        watchpointMote = null;
        watchpointListener = null;
      }
    }
  }

  private void addMoteObservers(final Mote mote, final MoteEvents moteEvents) {
    /* LEDs */
    final LED moteLEDs = mote.getInterfaces().getLED();
    if (moteLEDs != null) {
      LEDEvent startupEv = new LEDEvent(
          simulation.getSimulationTime(),
          moteLEDs.isRedOn(),
          moteLEDs.isGreenOn(),
          moteLEDs.isYellowOn()
      );
      moteEvents.addLED(startupEv);
      Observer observer = new Observer() {
        public void update(Observable o, Object arg) {
          LEDEvent ev = new LEDEvent(
              simulation.getSimulationTime(),
              moteLEDs.isRedOn(),
              moteLEDs.isGreenOn(),
              moteLEDs.isYellowOn()
          );

          moteEvents.addLED(ev);
        }
      };

      moteLEDs.addObserver(observer);
      activeMoteObservers.add(new MoteObservation(mote, moteLEDs, observer));
    }

    /* Radio OnOff, RXTX, and channels */
    final Radio moteRadio = mote.getInterfaces().getRadio();
    if (moteRadio != null) {
      RadioChannelEvent startupChannel = new RadioChannelEvent(
          simulation.getSimulationTime(), moteRadio.getChannel(), moteRadio.isRadioOn());
      moteEvents.addRadioChannel(startupChannel);
      RadioHWEvent startupHW = new RadioHWEvent(
          simulation.getSimulationTime(), moteRadio.isRadioOn());
      moteEvents.addRadioHW(startupHW);
      RadioRXTXEvent startupRXTX = new RadioRXTXEvent(
          simulation.getSimulationTime(), RXTXRadioEvent.IDLE);
      moteEvents.addRadioRXTX(startupRXTX);
      Observer observer = new Observer() {
        int lastChannel = -1;
        public void update(Observable o, Object arg) {
          RadioEvent radioEv = moteRadio.getLastEvent();

          String details = null;
          if (executionDetails && mote instanceof AbstractEmulatedMote) {
            details = ((AbstractEmulatedMote) mote).getExecutionDetails();
            if (details != null) {
              details = "<br>" + details.replace("\n", "<br>");
            }
          }

          /* Radio channel */
          int nowChannel = moteRadio.getChannel();
          if (nowChannel != lastChannel) {
            lastChannel = nowChannel;
            RadioChannelEvent ev = new RadioChannelEvent(
                simulation.getSimulationTime(), nowChannel, moteRadio.isRadioOn());
            moteEvents.addRadioChannel(ev);

            ev.details = details;
          }
          
          if (radioEv == RadioEvent.HW_ON ||
              radioEv == RadioEvent.HW_OFF) {
            RadioHWEvent ev = new RadioHWEvent(
                simulation.getSimulationTime(), moteRadio.isRadioOn());
            moteEvents.addRadioHW(ev);

            ev.details = details;

            /* Also create another channel event here */
            lastChannel = nowChannel;
            RadioChannelEvent ev2 = new RadioChannelEvent(
                simulation.getSimulationTime(), nowChannel, moteRadio.isRadioOn());
            ev2.details = details;
            moteEvents.addRadioChannel(ev2);
          }

          /* Radio RXTX events */
          if (radioEv == RadioEvent.TRANSMISSION_STARTED ||
              radioEv == RadioEvent.TRANSMISSION_FINISHED ||
              radioEv == RadioEvent.RECEPTION_STARTED ||
              radioEv == RadioEvent.RECEPTION_INTERFERED ||
              radioEv == RadioEvent.RECEPTION_FINISHED) {

            RadioRXTXEvent ev;
            /* Override events, instead show state */
            if (moteRadio.isTransmitting()) {
              ev = new RadioRXTXEvent(
                  simulation.getSimulationTime(), RXTXRadioEvent.TRANSMITTING);
            } else if (!moteRadio.isRadioOn()) {
              ev = new RadioRXTXEvent(
                  simulation.getSimulationTime(), RXTXRadioEvent.IDLE);
            } else if (moteRadio.isInterfered()) {
              ev = new RadioRXTXEvent(
                  simulation.getSimulationTime(), RXTXRadioEvent.INTERFERED);
            } else if (moteRadio.isReceiving()) {
              ev = new RadioRXTXEvent(
                  simulation.getSimulationTime(), RXTXRadioEvent.RECEIVING);
            } else {
              ev = new RadioRXTXEvent(
                  simulation.getSimulationTime(), RXTXRadioEvent.IDLE);
            }

            moteEvents.addRadioRXTX(ev);

            ev.details = details;
          }

        }
      };

      moteRadio.addObserver(observer);
      activeMoteObservers.add(new MoteObservation(mote, moteRadio, observer));
    }

    /* Watchpoints */
    if (mote instanceof WatchpointMote) {
      final WatchpointMote watchpointMote = ((WatchpointMote)mote);
      WatchpointListener listener = new WatchpointListener() {
        public void watchpointTriggered(Watchpoint watchpoint) {
          WatchpointEvent ev = new WatchpointEvent(simulation.getSimulationTime(), watchpoint);

          if (executionDetails && mote instanceof AbstractEmulatedMote) {
            String details = ((AbstractEmulatedMote) mote).getExecutionDetails();
            if (details != null) {
              details = "<br>" + details.replace("\n", "<br>");
              ev.details = details;
            }
          }

          moteEvents.addWatchpoint(ev);
        }
        public void watchpointsChanged() {
        }
      };

      watchpointMote.addWatchpointListener(listener);
      activeMoteObservers.add(new MoteObservation(mote, watchpointMote, listener));
    }

  }

  private void addMote(Mote newMote) {
    if (newMote == null) {
      return;
    }
    for (MoteEvents moteEvents: allMoteEvents) {
      if (moteEvents.mote == newMote) {
        return;
      }
    }

    MoteEvents newMoteLog = new MoteEvents(newMote);
    allMoteEvents.add(newMoteLog);
    addMoteObservers(newMote, newMoteLog);

    numberMotesWasUpdated();
  }

  private void removeMote(Mote mote) {
    MoteEvents remove = null;
    for (MoteEvents moteEvents: allMoteEvents) {
      if (moteEvents.mote == mote) {
        remove = moteEvents;
        break;
      }
    }
    if (remove == null) {
      logger.warn("No such observed mote: " + mote);
      return;
    }
    allMoteEvents.remove(remove);

    /* Remove mote observers */
    MoteObservation[] moteObservers = activeMoteObservers.toArray(new MoteObservation[0]);
    for (MoteObservation o: moteObservers) {
      if (o.getMote() == mote) {
        o.dispose();
        activeMoteObservers.remove(o);
      }
    }

    numberMotesWasUpdated();
  }

  private void recalculateMoteHeight() {
    int h = EVENT_PIXEL_HEIGHT;
    if (showRadioRXTX) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showRadioChannels) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showRadioOnoff) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showLeds) {
      h += 3*LED_PIXEL_HEIGHT;
    }
    if (showLogOutputs) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showWatchpoints) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (h != paintedMoteHeight) {
      paintedMoteHeight = h;
      timelineMoteRuler.repaint();
      timeline.repaint();
    }
  }

  public void closePlugin() {
    /* Remove repaint timer */
    repaintTimelineTimer.stop();

    if (moteHighlightObserver != null) {
      simulation.getCooja().deleteMoteHighlightObserver(moteHighlightObserver);
    }

    simulation.getEventCentral().removeMoteCountListener(newMotesListener);

    /* Remove active mote interface observers */
    for (MoteObservation o: activeMoteObservers) {
      o.dispose();
    }
    activeMoteObservers.clear();
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    /* Remember observed motes */
    Mote[] allMotes = simulation.getMotes();
    for (MoteEvents moteEvents: allMoteEvents) {
      element = new Element("mote");
      for (int i=0; i < allMotes.length; i++) {
        if (allMotes[i] == moteEvents.mote) {
          element.setText("" + i);
          config.add(element);
          break;
        }
      }
    }

    if (showRadioRXTX) {
      element = new Element("showRadioRXTX");
      config.add(element);
    }
    if (showRadioChannels) {
      element = new Element("showRadioChannels");
      config.add(element);
    }
    if (showRadioOnoff) {
      element = new Element("showRadioHW");
      config.add(element);
    }
    if (showLeds) {
      element = new Element("showLEDs");
      config.add(element);
    }
    if (showLogOutputs) {
      element = new Element("showLogOutput");
      config.add(element);
    }
    if (showWatchpoints) {
      element = new Element("showWatchpoints");
      config.add(element);
    }

    if (executionDetails) {
      element = new Element("executionDetails");
      config.add(element);
    }

    element = new Element("zoomfactor");
    element.addContent("" + currentPixelDivisor);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    showRadioRXTX = false;
    showRadioChannels = false;
    showRadioOnoff = false;
    showLeds = false;
    showLogOutputs = false;
    showWatchpoints = false;

    executionDetails = false;

    /* Remove already registered motes */
    MoteEvents[] allMoteEventsArr = allMoteEvents.toArray(new MoteEvents[0]);
    for (MoteEvents moteEvents: allMoteEventsArr) {
      removeMote(moteEvents.mote);
    }

    for (Element element : configXML) {
      String name = element.getName();
      if ("mote".equals(name)) {
        int index = Integer.parseInt(element.getText());
        addMote(simulation.getMote(index));
      } else if ("showRadioRXTX".equals(name)) {
        showRadioRXTX = true;
      } else if ("showRadioChannels".equals(name)) {
        showRadioChannels = true;
      } else if ("showRadioHW".equals(name)) {
        showRadioOnoff = true;
      } else if ("showLEDs".equals(name)) {
        showLeds = true;
      } else if ("showLogOutput".equals(name)) {
        showLogOutputs = true;
      } else if ("showWatchpoints".equals(name)) {
        showWatchpoints = true;

      } else if ("executionDetails".equals(name)) {
      	executionDetails = true;
      } else if ("zoom".equals(name)) {
        /* NB: Historically this is a one-based not zero-based index */
        final int zl = Integer.parseInt(element.getText())-1;
        zoomFinishLevel(zl, 0, 0);
      } else if ("zoomfactor".equals(name)) {
        /* NB: Historically no validation on this option */
        final double cpd = Double.parseDouble(element.getText());
        zoomFinish(cpd, 0, 0);
      }
    }

    recalculateMoteHeight();

    return true;
  }


  private int mousePixelPositionX = -1;
  private int mousePixelPositionY = -1;
  private int mouseDownPixelPositionX = -1;
  class Timeline extends JComponent {
    private static final long serialVersionUID = 2206491823778169359L;
    public Timeline() {
      setLayout(null);
      setToolTipText(null);
      setBackground(COLOR_BACKGROUND);

      addMouseListener(mouseAdapter);
      addMouseMotionListener(mouseAdapter);
      addMouseWheelListener(mouseAdapter);

      /* Popup menu */
      final JPopupMenu popupMenu = new JPopupMenu();

      popupMenu.add(new JMenuItem(showInAllAction));
      popupMenu.add(new JMenuItem(logListenerAction));
      popupMenu.add(new JMenuItem(radioLoggerAction));

      JMenu advancedMenu = new JMenu("Advanced");
      advancedMenu.add(new JCheckBoxMenuItem(executionDetailsAction) {
				private static final long serialVersionUID = 8314556794750277113L;
				public boolean isSelected() {
      		return executionDetails;
      	}
      });

      addMouseListener(new MouseAdapter() {
      	long lastClick = -1;
        public void mouseClicked(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupLocation = new Point(e.getX(), e.getY());
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }

          /* Focus on double-click */
          if (System.currentTimeMillis() - lastClick < 250) {
            popupLocation = e.getPoint();
            showInAllAction.actionPerformed(null);

            long time = (long) (popupLocation.x*currentPixelDivisor);
            Plugin[] plugins = simulation.getCooja().getStartedPlugins();
            for (Plugin p: plugins) {
            	if (!(p instanceof TimeLine)) {
            		continue;
            	}
            	if (p == TimeLine.this) {
            		continue;
            	}
              /* Select simulation time */
            	TimeLine plugin = (TimeLine) p;
              plugin.trySelectTime(time);
            }

          }
          lastClick = System.currentTimeMillis();
        }
        public void mousePressed(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupLocation = new Point(e.getX(), e.getY());
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }
        }
        public void mouseReleased(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupLocation = new Point(e.getX(), e.getY());
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }
        }
      });
    }

    private MouseAdapter mouseAdapter = new MouseAdapter() {
      private Popup popUpToolTip = null;
      private double zoomInitialPixelDivisor;
      private int zoomInitialMouseY;
      private long zoomCenterTime = -1;
      private double zoomCenter = -1;
      public void mouseDragged(MouseEvent e) {
        super.mouseDragged(e);
        if (e.isControlDown()) {
          /* Zoom with mouse */
          if (zoomCenterTime < 0) {
            return;
          }

          double factor = 0.01*(e.getY() - zoomInitialMouseY);
          factor = Math.exp(factor);

          final double cpd = zoomInitialPixelDivisor * factor;
          zoomFinish(cpd, zoomCenterTime, zoomCenter);
          return;
        }
        if (e.isAltDown()) {
          /* Pan with mouse */
          if (zoomCenterTime < 0) {
            return;
          }

          zoomCenter = (double) (e.getX() - timeline.getVisibleRect().x) / timeline.getVisibleRect().width;
          forceRepaintAndFocus(zoomCenterTime, zoomCenter);
          return;
        }

        if (mousePixelPositionX >= 0) {
          mousePixelPositionX = e.getX();
          mousePixelPositionY = e.getY();
          repaint();
        }
      }
      public void mousePressed(MouseEvent e) {
        if (e.isControlDown()) {
          /* Zoom with mouse */
          zoomInitialMouseY = e.getY();
          zoomInitialPixelDivisor = currentPixelDivisor;
          zoomCenterTime = (long) (e.getX()*currentPixelDivisor);
          zoomCenter = (double) (e.getX() - timeline.getVisibleRect().x) / timeline.getVisibleRect().width;
          return;
        }
        if (e.isAltDown()) {
          /* Pan with mouse */
          zoomCenterTime = (long) (e.getX()*currentPixelDivisor);
          return;
        }

        if (popUpToolTip != null) {
          popUpToolTip.hide();
          popUpToolTip = null;
        }
        if (e.getPoint().getY() < FIRST_MOTE_PIXEL_OFFSET) {
          mousePixelPositionX = e.getX();
          mouseDownPixelPositionX = e.getX();
          mousePixelPositionY = e.getY();
          repaint();
        } else {
          /* Trigger tooltip */
          JToolTip t = timeline.createToolTip();
          t.setTipText(Timeline.this.getMouseToolTipText(e));
          if (t.getTipText() == null || t.getTipText().equals("")) {
            return;
          }
          t.validate();

          /* Check tooltip width */
          Rectangle screenBounds = timeline.getGraphicsConfiguration().getBounds();
          int x;
          {
            int tooltip = e.getLocationOnScreen().x + t.getPreferredSize().width;
            int screen = screenBounds.x + screenBounds.width;
            if (tooltip > screen) {
              x = e.getLocationOnScreen().x - (tooltip-screen);
            } else {
              x = e.getLocationOnScreen().x;
            }
          }

          /* Check tooltip height */
          int y;
          {
            int tooltip = e.getLocationOnScreen().y + t.getPreferredSize().height;
            int screen = screenBounds.y + screenBounds.height;
            if (tooltip > screen) {
              y = e.getLocationOnScreen().y - (tooltip-screen);
            } else {
              y = e.getLocationOnScreen().y;
            }
          }

          popUpToolTip = PopupFactory.getSharedInstance().getPopup(null, t, x, y);
          popUpToolTip.show();
        }
      }
      public void mouseReleased(MouseEvent e) {
        zoomCenterTime = -1;
        if (popUpToolTip != null) {
          popUpToolTip.hide();
          popUpToolTip = null;
        }
        super.mouseReleased(e);
        mousePixelPositionX = -1;
        repaint();
      }
      public void mouseWheelMoved(MouseWheelEvent e) {
        if (e.isControlDown()) {
          final int nticks = e.getWheelRotation();
          final int zoomLevel = zoomGetLevel() + nticks;
          final long zct = (long) (e.getX()*currentPixelDivisor);
          final double zc = (double) (e.getX() - timeline.getVisibleRect().x) / timeline.getVisibleRect().width;
          zoomFinishLevel(zoomLevel, zct, zc);
          return;
        }
      }
    };

    private final Color SEPARATOR_COLOR = new Color(220, 220, 220);
    public void paintComponent(Graphics g) {
      Rectangle bounds = g.getClipBounds();
      /*logger.info("Clip bounds: " + bounds);*/

      if (needZoomOut) {
        /* Need zoom out */
        g.setColor(Color.RED);
        g.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);

        Rectangle vis = timeline.getVisibleRect();
        g.setColor(Color.WHITE);
        String msg = "Zoom out";
        FontMetrics fm = g.getFontMetrics();
        int msgWidth = fm.stringWidth(msg);
        int msgHeight = fm.getHeight();
        g.drawString(msg,
            vis.x + vis.width/2 - msgWidth/2,
            vis.y + vis.height/2 + msgHeight/2);
        return;
      }

      long intervalStart = (long)(bounds.x*currentPixelDivisor);
      long intervalEnd = (long) (intervalStart + bounds.width*currentPixelDivisor);

      if (intervalEnd > simulation.getSimulationTime()) {
        intervalEnd = simulation.getSimulationTime();
      }

      /*logger.info("Painting interval: " + intervalStart + " -> " + intervalEnd);*/
      if (bounds.x > Integer.MAX_VALUE - 1000) {
        /* Strange bounds */
        return;
      }

      g.setColor(COLOR_BACKGROUND);
      g.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);

      drawTimeRule(g, intervalStart, intervalEnd);

      /* Paint mote events */
      int lineHeightOffset = FIRST_MOTE_PIXEL_OFFSET;
      boolean dark = true;
      for (int mIndex = 0; mIndex < allMoteEvents.size(); mIndex++) {

        /* Mote separators */
        if (dark) {
          g.setColor(SEPARATOR_COLOR);
          g.fillRect(
              0, lineHeightOffset-2,
              getWidth(), paintedMoteHeight
          );
        }
        dark = !dark;

        if (showRadioRXTX) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioRXTXEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showRadioChannels) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioChannelEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showRadioOnoff) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioHWEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showLeds) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).ledEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += 3*LED_PIXEL_HEIGHT;
        }
        if (showLogOutputs) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).logEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showWatchpoints) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).watchpointEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }

        lineHeightOffset += EVENT_PIXEL_HEIGHT;
      }

      /* Draw vertical time marker (if mouse is dragged) */
      drawMouseTime(g, intervalStart, intervalEnd);
    }

    private <T extends MoteEvent> T getFirstIntervalEvent(ArrayList<T> events, long time) {
      /* TODO IMPLEMENT ME: Binary search */
      int nrEvents = events.size();
      if (nrEvents == 0) {
        return null;
      }
      if (nrEvents == 1) {
        events.get(0);
      }

      int ev = 0;
      while (ev < nrEvents && events.get(ev).time < time) {
        ev++;
      }
      ev--;
      if (ev < 0) {
        ev = 0;
      }

      if (ev >= events.size()) {
        return events.get(events.size()-1);
      }
      return events.get(ev);
    }

    private void drawTimeRule(Graphics g, long start, long end) {
      long time;

      /* Paint 10ms and 100 ms markers */
      g.setColor(Color.GRAY);

      time = start - (start % (100*Simulation.MILLISECOND));
      while (time <= end) {
        if (time % (100*Simulation.MILLISECOND) == 0) {
          g.drawLine(
              (int) (time/currentPixelDivisor), 0,
              (int) (time/currentPixelDivisor), TIME_MARKER_PIXEL_HEIGHT);
        } else {
          g.drawLine(
              (int) (time/currentPixelDivisor), 0,
              (int) (time/currentPixelDivisor), TIME_MARKER_PIXEL_HEIGHT/2);
        }
        time += (10*Simulation.MILLISECOND);
      }
    }

    private void drawMouseTime(Graphics g, long start, long end) {
      if (mousePixelPositionX >= 0) {
        long time = (long) (mousePixelPositionX*currentPixelDivisor);
        long diff = (long) (Math.abs(mouseDownPixelPositionX-mousePixelPositionX)*currentPixelDivisor);
        String str =
        	"Time (ms): " + (double)time/Simulation.MILLISECOND +
        	" (" + (double)diff/Simulation.MILLISECOND + ")";

        int h = g.getFontMetrics().getHeight();
        int w = g.getFontMetrics().stringWidth(str) + 6;
        int y= mousePixelPositionY<getHeight()/2?0:getHeight()-h;
        int delta = mousePixelPositionX + w > end/currentPixelDivisor?w:0; /* Don't write outside visible area */

        /* Line */
        g.setColor(Color.GRAY);
        g.drawLine(
            mousePixelPositionX, 0,
            mousePixelPositionX, getHeight());

        /* Text box */
        g.setColor(Color.DARK_GRAY);
        g.fillRect(
            mousePixelPositionX-delta, y,
            w, h);
        g.setColor(Color.BLACK);
        g.drawRect(
            mousePixelPositionX-delta, y,
            w, h);
        g.setColor(Color.WHITE);
        g.drawString(str,
            mousePixelPositionX+3-delta,
            y+h-1);
      }
    }

    public String getMouseToolTipText(MouseEvent event) {
      if (event.getPoint().y <= TIME_MARKER_PIXEL_HEIGHT) {
        return "<html>Click to display time marker</html>";
      }
      if (event.getPoint().y <= FIRST_MOTE_PIXEL_OFFSET) {
        return null;
      }

      /* Mote */
      int mote = (event.getPoint().y-FIRST_MOTE_PIXEL_OFFSET)/paintedMoteHeight;
      if (mote < 0 || mote >= allMoteEvents.size()) {
        return null;
      }
      String tooltip = "<html>Mote: " + allMoteEvents.get(mote).mote + "<br>";

      /* Time */
      long time = (long) (event.getPoint().x*currentPixelDivisor);
      tooltip += "Time (ms): " + (double)time/Simulation.MILLISECOND + "<br>";

      /* Event */
      ArrayList<? extends MoteEvent> events = null;
      int evMatched = 0;
      int evMouse = ((event.getPoint().y-FIRST_MOTE_PIXEL_OFFSET) % paintedMoteHeight) / EVENT_PIXEL_HEIGHT;
      if (showRadioRXTX) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).radioRXTXEvents;
        }
        evMatched++;
      }
      if (showRadioChannels) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).radioChannelEvents;
        }
        evMatched++;
      }
      if (showRadioOnoff) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).radioHWEvents;
        }
        evMatched++;
      }
      if (showLeds) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).ledEvents;
        }
        evMatched++;
      }
      if (showLogOutputs) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).logEvents;
        }
        evMatched++;
      }
      if (showWatchpoints) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).watchpointEvents;
        }
        evMatched++;
      }
      if (events != null) {
        MoteEvent ev = getFirstIntervalEvent(events, time);
        if (ev != null && time >= ev.time) {
          tooltip += ev + "<br>";

        	if (ev.details != null) {
        		tooltip += "Details:<br>" + ev.details;
        	}
        }
      }

      tooltip += "</html>";
      return tooltip;
    }
  }

  class MoteRuler extends JPanel {
    private static final long serialVersionUID = -5555627354526272220L;

    public MoteRuler() {
      setPreferredSize(new Dimension(35, 1));
      setToolTipText(null);
      setBackground(COLOR_BACKGROUND);

      final JPopupMenu popupMenu = new JPopupMenu();
      final JMenuItem topItem = new JMenuItem(topMoteAction);
      topItem.setText("Move to top");
      popupMenu.add(topItem);
      final JMenuItem sortItem = new JMenuItem(sortMoteAction);
      sortItem.setText("Sort by distance");
      popupMenu.add(sortItem);
      final JMenuItem removeItem = new JMenuItem(removeMoteAction);
      removeItem.setText("Remove from timeline");
      popupMenu.add(removeItem);
      final JMenuItem keepMoteOnlyItem = new JMenuItem(removeAllOtherMotesAction);
      keepMoteOnlyItem.setText("Remove all motes from timeline but");
      popupMenu.add(keepMoteOnlyItem);

      addMouseListener(new MouseAdapter() {
        public void mouseClicked(MouseEvent e) {
          Mote m = getMote(e.getPoint());
          if (m == null) {
            return;
          }
          simulation.getCooja().signalMoteHighlight(m);

          sortItem.setText("Sort by distance: " + m);
          sortItem.putClientProperty("mote", m);
          topItem.setText("Move to top: " + m);
          topItem.putClientProperty("mote", m);
          removeItem.setText("Remove from timeline: " + m);
          removeItem.putClientProperty("mote", m);
          keepMoteOnlyItem.setText("Remove all motes from timeline but: " + m);
          keepMoteOnlyItem.putClientProperty("mote", m);
          popupMenu.show(MoteRuler.this, e.getX(), e.getY());
        }
      });
    }

    private Mote getMote(Point p) {
      if (p.y < FIRST_MOTE_PIXEL_OFFSET) {
        return null;
      }
      int m = (p.y-FIRST_MOTE_PIXEL_OFFSET)/paintedMoteHeight;
      if (m < allMoteEvents.size()) {
        return allMoteEvents.get(m).mote;
      }
      return null;
    }

    protected void paintComponent(Graphics g) {
      g.setColor(COLOR_BACKGROUND);
      g.fillRect(0, 0, getWidth(), getHeight());
      g.setColor(Color.BLACK);

      g.setFont(new Font("SansSerif", Font.PLAIN, paintedMoteHeight));
      int y = FIRST_MOTE_PIXEL_OFFSET-EVENT_PIXEL_HEIGHT/2+paintedMoteHeight;
      for (MoteEvents moteLog: allMoteEvents) {
        String str = "" + moteLog.mote.getID();
        int w = g.getFontMetrics().stringWidth(str) + 1;

        if (highlightedMotes.contains(moteLog.mote)) {
        	g.setColor(HIGHLIGHT_COLOR);
          g.fillRect(0, y-paintedMoteHeight, getWidth()-1, paintedMoteHeight);
          g.setColor(Color.BLACK);
        }
        g.drawString(str, getWidth() - w, y);
        y += paintedMoteHeight;
      }
    }

    public String getToolTipText(MouseEvent event) {
      Point p = event.getPoint();
      Mote m = getMote(p);
      if (m == null)
        return null;

      return "<html>" + m + "<br>Click mote for options</html>";
    }
  }

  /* Event classes */
  abstract class MoteEvent {
    MoteEvent prev = null;
    MoteEvent next = null;
    String details = null;
    long time;
    public MoteEvent(long time) {
      this.time = time;
    }

    /**
     * Used by the default paint method to color events.
     * The event is not painted if the returned color is null.
     *
     * @see #paintInterval(Graphics, int, long)
     * @return Event color or null
     */
    public abstract Color getEventColor();

    /* Default paint method */
    public void paintInterval(Graphics g, int lineHeightOffset, long end) {
      MoteEvent ev = this;
      while (ev != null && ev.time < end) {
        int w; /* Pixel width */

        /* Calculate event width */
        if (ev.next != null) {
          w = (int) ((ev.next.time - ev.time)/currentPixelDivisor);
        } else {
          w = (int) ((end - ev.time)/currentPixelDivisor); /* No more events */
        }

        /* Handle zero pixel width events */
        if (w == 0) {
          if (PAINT_ZERO_WIDTH_EVENTS) {
            w = 1;
          } else {
            ev = ev.next;
            continue;
          }
        }

        Color color = ev.getEventColor();
        if (color == null) {
          /* Skip painting event */
          ev = ev.next;
          continue;
        }
        g.setColor(color);

        g.fillRect(
            (int)(ev.time/currentPixelDivisor), lineHeightOffset,
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
      }
    }
  }
  class NoHistoryEvent extends MoteEvent {
    public NoHistoryEvent(long time) {
      super(time);
    }
    public Color getEventColor() {
      return Color.CYAN;
    }
    public String toString() {
      return "No events has been captured yet";
    }
  }
  public enum RXTXRadioEvent {
    IDLE, RECEIVING, TRANSMITTING, INTERFERED
  }
  class RadioRXTXEvent extends MoteEvent {
    RXTXRadioEvent state = null;
    public RadioRXTXEvent(long time, RXTXRadioEvent ev) {
      super(time);
      this.state = ev;
    }
    public RadioRXTXEvent(long time, RXTXRadioEvent ev, String details) {
      this(time, ev);
      this.details = details;
    }
    public Color getEventColor() {
      if (state == RXTXRadioEvent.IDLE) {
        return null;
      } else if (state == RXTXRadioEvent.TRANSMITTING) {
        return Color.BLUE;
      } else if (state == RXTXRadioEvent.RECEIVING) {
        return Color.GREEN;
      } else if (state == RXTXRadioEvent.INTERFERED) {
        return Color.RED;
      } else {
        logger.fatal("Unknown RXTX event");
        return null;
      }
    }
    public String toString() {
      if (state == RXTXRadioEvent.IDLE) {
        return "Radio idle from " + time + "<br>";
      } else if (state == RXTXRadioEvent.TRANSMITTING) {
      	return "Radio transmitting from " + time + "<br>";
      } else if (state == RXTXRadioEvent.RECEIVING) {
        return "Radio receiving from " + time + "<br>";
      } else if (state == RXTXRadioEvent.INTERFERED) {
        return "Radio interfered from " + time + "<br>";
      } else {
        return "Unknown event<br>";
      }
    }
  }

  private final static Color[] CHANNEL_COLORS = new Color[] {
    Color.decode("0x008080"), Color.decode("0x808080"), Color.decode("0xC00000"),
    Color.decode("0x000020"), Color.decode("0x202000"), Color.decode("0x200020"),
    Color.decode("0x002020"), Color.decode("0x202020"), Color.decode("0x006060"),
    Color.decode("0x606060"), Color.decode("0xA00000"), Color.decode("0x00A000"),
    Color.decode("0x0000A0"), Color.decode("0x400040"), Color.decode("0x004040"),
    Color.decode("0x404040"), Color.decode("0x200000"), Color.decode("0x002000"),
    Color.decode("0xA0A000"), Color.decode("0xA000A0"), Color.decode("0x00A0A0"),
    Color.decode("0xA0A0A0"), Color.decode("0xE00000"), Color.decode("0x600000"),
    Color.decode("0x000040"), Color.decode("0x404000"), Color.decode("0xFF0000"),
    Color.decode("0x00FF00"), Color.decode("0x0000FF"), Color.decode("0xFFFF00"),
    Color.decode("0xFF00FF"), Color.decode("0x808000"), Color.decode("0x800080"),
  };
  class RadioChannelEvent extends MoteEvent {
    int channel;
    boolean radioOn;
    public RadioChannelEvent(long time, int channel, boolean radioOn) {
      super(time);
      this.channel = channel;
      this.radioOn = radioOn;
    }
    public Color getEventColor() {
      if (channel >= 0) {
        if (!radioOn) {
          return null;
        }
        Color c = CHANNEL_COLORS[channel % CHANNEL_COLORS.length];
        return c;
      }
      return null;
    }
    public String toString() {
      String str = "Radio channel " + channel + "<br>";
      return str;
    }
  }

  class RadioHWEvent extends MoteEvent {
    boolean on;
    public RadioHWEvent(long time, boolean on) {
      super(time);
      this.on = on;
    }
    public RadioHWEvent(long time, boolean on, int channel) {
    	this(time, on);
    }
    public Color getEventColor() {
    	if (on) {
    	    return Color.GRAY;
    	}
    	return null;
    }
    public String toString() {
      String str = "Radio HW was turned " + (on?"on":"off") + "<br>";
      return str;
    }
  }
  class LEDEvent extends MoteEvent {
    boolean red;
    boolean green;
    boolean blue;
    Color color;
    public LEDEvent(long time, boolean red, boolean green, boolean blue) {
      super(time);
      this.red = red;
      this.green = green;
      this.blue = blue;
      this.color = new Color(red?255:0, green?255:0, blue?255:0);
    }
    public Color getEventColor() {
      if (!red && !green && !blue) {
        return null;
      } else if (red && green && blue) {
        return Color.LIGHT_GRAY;
      } else {
        return color;
      }
    }
    /* LEDs are painted in three lines */
    public void paintInterval(Graphics g, int lineHeightOffset, long end) {
      MoteEvent ev = this;
      while (ev != null && ev.time < end) {
        int w; /* Pixel width */

        /* Calculate event width */
        if (ev.next != null) {
          w = (int) ((ev.next.time - ev.time)/currentPixelDivisor);
        } else {
          w = (int) ((end - ev.time)/currentPixelDivisor); /* No more events */
        }

        /* Handle zero pixel width events */
        if (w == 0) {
          if (PAINT_ZERO_WIDTH_EVENTS) {
            w = 1;
          } else {
            ev = ev.next;
            continue;
          }
        }

        Color color = ev.getEventColor();
        if (color == null) {
          /* Skip painting event */
          ev = ev.next;
          continue;
        }
        if (color.getRed() > 0) {
          g.setColor(new Color(color.getRed(), 0, 0));
          g.fillRect(
              (int)(ev.time/currentPixelDivisor), lineHeightOffset,
              w, LED_PIXEL_HEIGHT
          );
        }
        if (color.getGreen() > 0) {
          g.setColor(new Color(0, color.getGreen(), 0));
          g.fillRect(
              (int)(ev.time/currentPixelDivisor), lineHeightOffset+LED_PIXEL_HEIGHT,
              w, LED_PIXEL_HEIGHT
          );
        }
        if (color.getBlue() > 0) {
          g.setColor(new Color(0, 0, color.getBlue()));
          g.fillRect(
              (int)(ev.time/currentPixelDivisor), lineHeightOffset+2*LED_PIXEL_HEIGHT,
              w, LED_PIXEL_HEIGHT
          );
        }
        ev = ev.next;
      }
    }
    public String toString() {
      return
      "LED state:<br>" +
      "Red = " + (red?"ON":"OFF") + "<br>" +
      "Green = " + (green?"ON":"OFF") + "<br>" +
      "Blue = " + (blue?"ON":"OFF") + "<br>";
    }
  }
  class LogEvent extends MoteEvent {
    LogOutputEvent logEvent;
    public LogEvent(LogOutputEvent ev) {
      super(ev.getTime());
      this.logEvent = ev;
    }
    public Color getEventColor() {
      if (logEventFilterPlugin != null) {
        /* Ask log listener for event color to use */
        return logEventFilterPlugin.getColorOfEntry(logEvent);
      }
      return Color.GRAY;
    }
    /* Default paint method */
    public void paintInterval(Graphics g, int lineHeightOffset, long end) {
      LogEvent ev = this;
      while (ev != null && ev.time < end) {
        /* Ask active log listener whether this should be filtered  */
        
        if (logEventFilterPlugin != null) {
          boolean show = logEventFilterPlugin.filterWouldAccept(ev.logEvent);
          if (!show) {
            /* Skip painting event */
            ev = (LogEvent) ev.next;
            continue;
          }
        }

        Color color = ev.getEventColor();
        if (color == null) {
          /* Skip painting event */
          ev = (LogEvent) ev.next;
          continue;
        }

        g.setColor(color);
        g.fillRect(
            (int)(ev.time/currentPixelDivisor), lineHeightOffset,
            4, EVENT_PIXEL_HEIGHT
        );
        g.setColor(Color.BLACK);
        g.fillRect(
            (int)(ev.time/currentPixelDivisor), lineHeightOffset,
            1, EVENT_PIXEL_HEIGHT
        );

        ev = (LogEvent) ev.next;
      }
    }
    public String toString() {
      return "Mote " + logEvent.getMote() + " says:<br>" + logEvent.getMessage() + "<br>";
    }
  }
  class WatchpointEvent extends MoteEvent {
    Watchpoint watchpoint;
    public WatchpointEvent(long time, Watchpoint watchpoint) {
      super(time);
      this.watchpoint = watchpoint;
    }
    public Color getEventColor() {
      Color c = watchpoint.getColor();
      if (c == null) {
        return Color.BLACK;
      }
      return c;
    }
    public String toString() {
      String desc = watchpoint.getDescription();
      desc = desc.replace("\n", "<br>");
      return
      "Watchpoint triggered at time (ms): " +  time/Simulation.MILLISECOND + ".<br>"
      + desc + "<br>";
    }

    /* Default paint method */
    public void paintInterval(Graphics g, int lineHeightOffset, long end) {
      MoteEvent ev = this;
      while (ev != null && ev.time < end) {
        int w = 2; /* Watchpoints are always two pixels wide */

        Color color = ev.getEventColor();
        if (color == null) {
          /* Skip painting event */
          ev = ev.next;
          continue;
        }
        g.setColor(color);

        g.fillRect(
            (int)(ev.time/currentPixelDivisor), lineHeightOffset,
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
      }
    }
  }
  class MoteEvents {
    Mote mote;
    ArrayList<MoteEvent> radioRXTXEvents;
    ArrayList<MoteEvent> radioChannelEvents;
    ArrayList<MoteEvent> radioHWEvents;
    ArrayList<MoteEvent> ledEvents;
    ArrayList<MoteEvent> logEvents;
    ArrayList<MoteEvent> watchpointEvents;

    private MoteEvent lastRadioRXTXEvent = null;
    private MoteEvent lastRadioChannelEvent = null;
    private MoteEvent lastRadioHWEvent = null;
    private MoteEvent lastLEDEvent = null;
    private MoteEvent lastLogEvent = null;
    private MoteEvent lastWatchpointEvent = null;

    public MoteEvents(Mote mote) {
      this.mote = mote;
      this.radioRXTXEvents = new ArrayList<MoteEvent>();
      this.radioChannelEvents = new ArrayList<MoteEvent>();
      this.radioHWEvents = new ArrayList<MoteEvent>();
      this.ledEvents = new ArrayList<MoteEvent>();
      this.logEvents = new ArrayList<MoteEvent>();
      this.watchpointEvents = new ArrayList<MoteEvent>();

      if (mote.getSimulation().getSimulationTime() > 0) {
        /* Create no history events */
        lastRadioRXTXEvent = new NoHistoryEvent(0);
        lastRadioChannelEvent = new NoHistoryEvent(0);
        lastRadioHWEvent = new NoHistoryEvent(0);
        lastLEDEvent = new NoHistoryEvent(0);
        lastLogEvent = new NoHistoryEvent(0);
        lastWatchpointEvent = new NoHistoryEvent(0);
        radioRXTXEvents.add(lastRadioRXTXEvent);
        radioChannelEvents.add(lastRadioChannelEvent);
        radioHWEvents.add(lastRadioHWEvent);
        ledEvents.add(lastLEDEvent);
        logEvents.add(lastLogEvent);
        watchpointEvents.add(lastWatchpointEvent);
      }
    }

    protected void clear() {
      this.radioRXTXEvents.clear();
      this.radioChannelEvents.clear();
      this.radioHWEvents.clear();
      this.ledEvents.clear();
      this.logEvents.clear();
      this.watchpointEvents.clear();

      if (mote.getSimulation().getSimulationTime() > 0) {
        /* Create no history events */
        lastRadioRXTXEvent = new NoHistoryEvent(0);
        lastRadioChannelEvent = new NoHistoryEvent(0);
        lastRadioHWEvent = new NoHistoryEvent(0);
        lastLEDEvent = new NoHistoryEvent(0);
        lastLogEvent = new NoHistoryEvent(0);
        lastWatchpointEvent = new NoHistoryEvent(0);
        radioRXTXEvents.add(lastRadioRXTXEvent);
        radioChannelEvents.add(lastRadioChannelEvent);
        radioHWEvents.add(lastRadioHWEvent);
        ledEvents.add(lastLEDEvent);
        logEvents.add(lastLogEvent);
        watchpointEvents.add(lastWatchpointEvent);
      }
    }

    public void addRadioRXTX(RadioRXTXEvent ev) {
      /* Link with previous events */
      if (lastRadioRXTXEvent != null) {
        ev.prev = lastRadioRXTXEvent;
        lastRadioRXTXEvent.next = ev;
      }
      lastRadioRXTXEvent = ev;

      radioRXTXEvents.add(ev);
    }
    public void addRadioChannel(RadioChannelEvent ev) {
      /* Link with previous events */
      if (lastRadioChannelEvent != null) {
        ev.prev = lastRadioChannelEvent;
        lastRadioChannelEvent.next = ev;
      }
      lastRadioChannelEvent = ev;

      radioChannelEvents.add(ev);
    }
    public void addRadioHW(RadioHWEvent ev) {
      /* Link with previous events */
      if (lastRadioHWEvent != null) {
        ev.prev = lastRadioHWEvent;
        lastRadioHWEvent.next = ev;
      }
      lastRadioHWEvent = ev;

      radioHWEvents.add(ev);
    }
    public void addLED(LEDEvent ev) {
      /* Link with previous events */
      if (lastLEDEvent != null) {
        ev.prev = lastLEDEvent;
        lastLEDEvent.next = ev;
      }
      lastLEDEvent = ev;

      ledEvents.add(ev);
    }
    public void addLog(LogEvent ev) {
      /* Link with previous events */
      if (lastLogEvent != null) {
        ev.prev = lastLogEvent;
        lastLogEvent.next = ev;
      }
      lastLogEvent = ev;

      logEvents.add(ev);
    }
    public void addWatchpoint(WatchpointEvent ev) {
      /* Link with previous events */
      if (lastWatchpointEvent != null) {
        ev.prev = lastWatchpointEvent;
        lastWatchpointEvent.next = ev;
      }
      lastWatchpointEvent = ev;

      watchpointEvents.add(ev);
    }
  }

  private long lastRepaintSimulationTime = -1;
  private Timer repaintTimelineTimer = new Timer(TIMELINE_UPDATE_INTERVAL, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      /* Only set new size if simulation time has changed */
      long now = simulation.getSimulationTime();
      if (now == lastRepaintSimulationTime) {
        return;
      }
      lastRepaintSimulationTime = now;

      /* Update timeline size */
      int newWidth;
      if (now/currentPixelDivisor > Integer.MAX_VALUE) {
        /* Need zoom out */
        newWidth = 1;
        needZoomOut = true;
      } else {
        newWidth = (int) (now/currentPixelDivisor);
        needZoomOut = false;
      }

      Rectangle visibleRectangle = timeline.getVisibleRect();
      boolean isTracking = visibleRectangle.x + visibleRectangle.width >= timeline.getWidth();

      int newHeight = (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size());
      timeline.setPreferredSize(new Dimension(
          newWidth,
          newHeight
      ));
      timelineMoteRuler.setPreferredSize(new Dimension(
          35,
          newHeight
      ));
      timeline.revalidate();
      timeline.repaint();

      /* Update visible rectangle */
      if (isTracking) {
        Rectangle r = new Rectangle(
            newWidth-1, visibleRectangle.y,
            1, 1);
        timeline.scrollRectToVisible(r);
      }
    }
  });

  public String getQuickHelp() {
    return
        "<b>Timeline</b>" +
        "<p>The timeline shows simulation events over time. " +
        "The timeline can be used to inspect activities of individual nodes as well as interactions between nodes." +
        "<p>For each mote, simulation events are shown on a colored line. Different colors correspond to different events. For more information about a particular event, mouse click it." +
        "<p>The <i>Events</i> menu control what event types are shown in the timeline. " +
        "Currently, six event types are supported (see below). " +
        "<p>All motes are by default shown in the timeline. Motes can be removed from the timeline by right-clicking the node ID on the left." +
        "<p>To display a vertical time marker on the timeline, press and hold the mouse on the time ruler (top)." +
        "<p>For more options for a given event, right-click the mouse for a popup menu." +
        "<p><b>Radio traffic</b>" +
        "<br>Shows radio traffic events. Transmissions are painted blue, receptions are green, and interfered radios are red." +
        "<p><b>Radio channel</b>" +
        "<br>Shows the current radio channel by colors." +
        "<p><b>Radio on/off</b>" +
        "<br>Shows whether the mote radio is on or off. When gray, the radio is on." +
        "<p><b>LEDs</b>" +
        "<br>Shows LED state: red, green, and blue. (Assumes all mote types have exactly three LEDs.)" +
        "<p><b>Log outputs</b>" +
        "<br>Shows log outputs, as also shown in " + Cooja.getDescriptionOf(LogListener.class) +
        "<p><b>Watchpoints</b>" +
        "<br>Shows triggered watchpoints, currently only supported by emulated motes. To add watchpoints, use the Msp Code Watcher plugin.";
  }
}
