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
 * $Id: TimeLine.java,v 1.11 2009/07/02 12:05:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.Watchpoint;
import se.sics.cooja.WatchpointMote;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.interfaces.Radio.RadioEvent;

/**
 * Shows events such as mote logs, LEDs, and radio transmissions, in a timeline.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Timeline")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class TimeLine extends VisPlugin {

  public static final int LED_PIXEL_HEIGHT = 2;
  public static final int EVENT_PIXEL_HEIGHT = 4;
  public static final int TIME_MARKER_PIXEL_HEIGHT = 6;
  public static final int FIRST_MOTE_PIXEL_OFFSET = TIME_MARKER_PIXEL_HEIGHT + EVENT_PIXEL_HEIGHT;

  private static final Color COLOR_BACKGROUND = Color.WHITE;
  private static final boolean PAINT_ZERO_WIDTH_EVENTS = true;
  private static final int TIMELINE_UPDATE_INTERVAL = 100;

  private static int currentPixelDivisor = 200;

  private static final int[] ZOOM_LEVELS = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000 }; 

  private int zoomLevel = 9;

  private static Logger logger = Logger.getLogger(TimeLine.class);

  private int paintedMoteHeight = EVENT_PIXEL_HEIGHT;

  private Simulation simulation;
  private MoteCountListener newMotesListener;

  private JScrollPane timelineScrollPane;
  private MoteRuler timelineMoteRuler;
  private JComponent timeline;
  private Box eventCheckboxes;
  private JSplitPane splitPane;
  
  private ArrayList<MoteObservation> activeMoteObservers = new ArrayList<MoteObservation>();

  private ArrayList<MoteEvents> allMoteEvents = new ArrayList<MoteEvents>();

  private boolean showRadioRXTX = true;
  private boolean showRadioChannels = false;
  private boolean showRadioHW = true;
  private boolean showLEDs = true;
  private boolean showLogOutputs = false;
  private boolean showWatchpoints = false;

  /**
   * @param simulation Simulation
   * @param gui GUI
   */
  public TimeLine(final Simulation simulation, final GUI gui) {
    super("Timeline (Add motes to observe by clicking +)", gui);
    this.simulation = simulation;
    
    currentPixelDivisor = ZOOM_LEVELS[zoomLevel];
    
    /* Box: events to observe */
    eventCheckboxes = Box.createVerticalBox();
    JCheckBox eventCheckBox;
    eventCheckBox = createEventCheckbox("Radio RX/TX", "Show radio transmissions, receptions, and collisions");
    eventCheckBox.setSelected(showRadioRXTX);
    eventCheckBox.setName("showRadioRXTX");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioRXTX = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("Radio channels", "Show different radio channels");
    eventCheckBox.setSelected(showRadioChannels);
    eventCheckBox.setName("showRadioChannels");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioChannels = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    /*eventCheckboxes.add(eventCheckBox);*/
    eventCheckBox = createEventCheckbox("Radio ON/OFF", "Show radio hardware state");
    eventCheckBox.setSelected(showRadioHW);
    eventCheckBox.setName("showRadioHW");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioHW = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("LEDs", "Show LED state");
    eventCheckBox.setSelected(showLEDs);
    eventCheckBox.setName("showLEDs");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLEDs = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("Log output", "Show mote log output, such as by printf()'s");
    eventCheckBox.setSelected(showLogOutputs);
    eventCheckBox.setName("showLogOutput");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLogOutputs = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    /*eventCheckboxes.add(eventCheckBox);*/
    eventCheckBox = createEventCheckbox("Watchpoints", "Show code watchpoints (for MSPSim-based motes)");
    eventCheckBox.setSelected(showWatchpoints);
    eventCheckBox.setName("showWatchpoints");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showWatchpoints = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);

    /* Panel: timeline canvas w. scroll pane and add mote button */
    timeline = new Timeline();
    timelineScrollPane = new JScrollPane(
        timeline,
        JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
        JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

    JButton timelineAddMoteButton = new JButton(addMoteAction);
    timelineAddMoteButton.setText("+");
    timelineAddMoteButton.setToolTipText("Add mote");
    timelineAddMoteButton.setBorderPainted(false);
    timelineAddMoteButton.setFont(new Font("SansSerif", Font.PLAIN, 11));

    timelineMoteRuler = new MoteRuler();
    timelineScrollPane.setRowHeaderView(timelineMoteRuler);
    timelineScrollPane.setCorner(JScrollPane.LOWER_LEFT_CORNER, timelineAddMoteButton);
    timelineScrollPane.setBackground(Color.WHITE);

    splitPane = new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        new JScrollPane(eventCheckboxes),
        timelineScrollPane
    );
    splitPane.setOneTouchExpandable(true);

    getContentPane().add(splitPane);

    pack();
    setSize(gui.getDesktopPane().getWidth(), 150);
    setLocation(0, gui.getDesktopPane().getHeight() - 150);

    numberMotesWasUpdated();

    /* Automatically add/delete motes */
    simulation.getEventCentral().addMoteCountListener(newMotesListener = new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        addMote(mote);
      }
      public void moteWasRemoved(Mote mote) {
        removeMote(mote);
      }
    });
    for (Mote m: simulation.getMotes()) {
      addMote(m);
    }
    
    /* Update timeline for the duration of the plugin */
    repaintTimelineTimer.start();

    /* TODO Register Integer overflow time event */
  }

  private JCheckBox createEventCheckbox(String text, String tooltip) {
    JCheckBox checkBox = new JCheckBox(text, true);
    checkBox.setToolTipText(tooltip);
    return checkBox;
  }

  private Action removeMoteAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      JComponent b = (JComponent) e.getSource();
      Mote m = (Mote) b.getClientProperty("mote");
      removeMote(m);
    }
  };
  private Action addMoteAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {

      JComboBox source = new JComboBox();
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
      String options[] = new String[] {"Cancel", "Add"};
      optionPane.setOptions(options);
      optionPane.setInitialValue(options[1]);
      JDialog dialog = optionPane.createDialog(GUI.getTopParentContainer(), "Add mote to timeline");
      dialog.setVisible(true);

      if (optionPane.getValue() == null || !optionPane.getValue().equals("Add")) {
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

  private Action zoomInAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      Rectangle r = timeline.getVisibleRect();
      final long centerTime = (r.x + r.width/2)*currentPixelDivisor;

      if (zoomLevel > 0) {
        zoomLevel--;
      }
      currentPixelDivisor = ZOOM_LEVELS[zoomLevel];
      logger.info("Zoom level: " + currentPixelDivisor + " microseconds/pixel " + ((zoomLevel==0)?"(MIN)":""));
      
      lastRepaintSimulationTime = -1; /* Force repaint */
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          Rectangle r = new Rectangle((int)(centerTime/currentPixelDivisor)-1, 0, 2, 1);
          timeline.scrollRectToVisible(r);
        }
      });
    }
  };

  private Action zoomOutAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      Rectangle r = timeline.getVisibleRect();
      final long centerTime = (r.x + r.width/2)*currentPixelDivisor;

      if (zoomLevel < ZOOM_LEVELS.length-1) {
        zoomLevel++;
      }
      currentPixelDivisor = ZOOM_LEVELS[zoomLevel];
      logger.info("Zoom level: " + currentPixelDivisor + " microseconds/pixel " + ((zoomLevel==ZOOM_LEVELS.length-1)?"(MAX)":""));

      lastRepaintSimulationTime = -1; /* Force repaint */
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          Rectangle r = new Rectangle((int)(centerTime/currentPixelDivisor)-1, 0, 2, 1);
          timeline.scrollRectToVisible(r);
        }
      });
    }
  };
  
  /**
   * Save logged raw data to file for post-processing. 
   */
  private Action saveDataAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      JFileChooser fc = new JFileChooser();
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

  private void numberMotesWasUpdated() {
    /* Plugin title */
    if (allMoteEvents.isEmpty()) {
      setTitle("Timeline (Add motes to observe by clicking +)");
    } else {
      setTitle("Timeline (" + allMoteEvents.size() + " motes)");
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
    private ActionListener watchpointListener; /* XXX */
    
    public MoteObservation(Mote mote, Observable observable, Observer observer) {
      this.mote = mote;
      this.observable = observable;
      this.observer = observer;
    }

    /* XXX Special case, should be generalized */
    public MoteObservation(Mote mote, WatchpointMote watchpointMote, ActionListener listener) {
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

  private void addMoteObservers(Mote mote, final MoteEvents moteEvents) {
    /* TODO Log: final Log moteLog = mote.getInterfaces().getLog(); */
    /* TODO Unknown state event */
    
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

    /* Radio HW, RXTX */
    final Radio moteRadio = mote.getInterfaces().getRadio();
    if (moteRadio != null) {
      RadioHWEvent startupHW = new RadioHWEvent(
          simulation.getSimulationTime(), moteRadio.isReceiverOn());
      moteEvents.addRadioHW(startupHW);
      RadioRXTXEvent startupRXTX = new RadioRXTXEvent(
          simulation.getSimulationTime(), RadioEvent.UNKNOWN);
      moteEvents.addRadioRXTX(startupRXTX);
      Observer observer = new Observer() {
        public void update(Observable o, Object arg) {
          /* Radio HW events */
          if (moteRadio.getLastEvent() == RadioEvent.HW_ON ||
              moteRadio.getLastEvent() == RadioEvent.HW_OFF) {
            RadioHWEvent ev = new RadioHWEvent(
                simulation.getSimulationTime(), moteRadio.getLastEvent()==RadioEvent.HW_ON);

            moteEvents.addRadioHW(ev);
            return;
          }

          /* Radio RXTX events */
          if (moteRadio.getLastEvent() == RadioEvent.TRANSMISSION_STARTED ||
              moteRadio.getLastEvent() == RadioEvent.TRANSMISSION_FINISHED ||
              moteRadio.getLastEvent() == RadioEvent.RECEPTION_STARTED ||
              moteRadio.getLastEvent() == RadioEvent.RECEPTION_INTERFERED ||
              moteRadio.getLastEvent() == RadioEvent.RECEPTION_FINISHED) {
            RadioRXTXEvent ev = new RadioRXTXEvent(
                simulation.getSimulationTime(), moteRadio.getLastEvent());

            moteEvents.addRadioRXTX(ev);
            return;
          }

        }
      };

      moteRadio.addObserver(observer);
      activeMoteObservers.add(new MoteObservation(mote, moteRadio, observer));
    }
    
    /* XXX Experimental: Watchpoints */
    if (mote instanceof WatchpointMote) {
      final WatchpointMote watchpointMote = ((WatchpointMote)mote);
      ActionListener listener = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          if (watchpointMote.getLastWatchpoint() == null) {
            return;
          }
          WatchpointEvent ev = new WatchpointEvent(
              simulation.getSimulationTime(),
              watchpointMote.getLastWatchpoint()
          );

          moteEvents.addWatchpoint(ev);
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
    if (showRadioHW) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showLEDs) {
      h += 3*LED_PIXEL_HEIGHT;
    }
    if (showLogOutputs) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showWatchpoints) {
      h += EVENT_PIXEL_HEIGHT;
    }
    paintedMoteHeight = h;
    timelineMoteRuler.repaint();
    timeline.repaint();
  }

  public void closePlugin() {
    /* Remove repaint timer */
    repaintTimelineTimer.stop();

    simulation.getEventCentral().removeMoteCountListener(newMotesListener);
    
    /* Remove active mote interface observers */
    for (MoteObservation o: activeMoteObservers) {
      o.dispose();
    }
    activeMoteObservers.clear();
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
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
    if (showRadioHW) {
      element = new Element("showRadioHW");
      config.add(element);
    }
    if (showLEDs) {
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

    element = new Element("split");
    element.addContent("" + splitPane.getDividerLocation());
    config.add(element);

    element = new Element("zoom");
    element.addContent("" + zoomLevel);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    showRadioRXTX = false;
    showRadioChannels = false;
    showRadioHW = false;
    showLEDs = false;
    showLogOutputs = false;
    showWatchpoints = false;

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
        showRadioHW = true;
      } else if ("showLEDs".equals(name)) {
        showLEDs = true;
      } else if ("showLogOutput".equals(name)) {
        showLogOutputs = true;
      } else if ("showWatchpoints".equals(name)) {
        showWatchpoints = true;
      } else if ("split".equals(name)) {
        splitPane.setDividerLocation(Integer.parseInt(element.getText()));
      } else if ("zoom".equals(name)) {
        zoomLevel = Integer.parseInt(element.getText())-1;
        zoomOutAction.actionPerformed(null);
      }
    }

    /* XXX HACK: Update checkboxes according to config */
    for (Component c: eventCheckboxes.getComponents()) {
      if (c.getName() == "showRadioRXTX") {
        ((JCheckBox)c).setSelected(showRadioRXTX);
      } else if (c.getName() == "showRadioChannels") {
        ((JCheckBox)c).setSelected(showRadioChannels);
      } else if (c.getName() == "showRadioHW") {
        ((JCheckBox)c).setSelected(showRadioHW);
      } else if (c.getName() == "showLEDs") {
        ((JCheckBox)c).setSelected(showLEDs);
      } else if (c.getName() == "showLogOutput") {
        ((JCheckBox)c).setSelected(showLogOutputs);
      } else if (c.getName() == "showWatchpoints") {
        ((JCheckBox)c).setSelected(showWatchpoints);
      }
    }
    recalculateMoteHeight();

    return true;
  }

  class Timeline extends JComponent {
    private int mousePixelPositionX = -1;
    private int mousePixelPositionY = -1;

    public Timeline() {
      setLayout(null);
      setToolTipText("");
      setBackground(COLOR_BACKGROUND);
      
      addMouseListener(mouseAdapter);
      addMouseMotionListener(mouseAdapter);

      /* Popup menu */
      final JPopupMenu popupMenu = new JPopupMenu();
      JMenuItem addMoteItem = new JMenuItem(addMoteAction);
      addMoteItem.setText("Add motes to timeline");
      popupMenu.add(addMoteItem);

      popupMenu.addSeparator();

      JMenuItem zoomInItem = new JMenuItem(zoomInAction);
      zoomInItem.setText("Zoom in");
      popupMenu.add(zoomInItem);

      JMenuItem zoomOutItem = new JMenuItem(zoomOutAction);
      zoomOutItem.setText("Zoom out");
      popupMenu.add(zoomOutItem);

      popupMenu.addSeparator();

      JMenuItem saveItem = new JMenuItem(saveDataAction);
      saveItem.setText("Save raw data to file");
      popupMenu.add(saveItem);

      addMouseListener(new MouseAdapter() {
        public void mouseClicked(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }
        }
        public void mousePressed(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }
        }
        public void mouseReleased(MouseEvent e) {
          if (e.isPopupTrigger()) {
            popupMenu.show(Timeline.this, e.getX(), e.getY());
          }
        }
      });
    }
    
    private MouseAdapter mouseAdapter = new MouseAdapter() {
      public void mouseDragged(MouseEvent e) {
        super.mouseDragged(e);
        if (mousePixelPositionX >= 0) {
          mousePixelPositionX = e.getX();
          mousePixelPositionY = e.getY();
          repaint();
        }
      }
      public void mousePressed(MouseEvent e) {
        if (e.getPoint().getY() < FIRST_MOTE_PIXEL_OFFSET) {
          mousePixelPositionX = e.getX();
          mousePixelPositionY = e.getY();
          repaint();
        }
      }
      public void mouseReleased(MouseEvent e) {
        super.mouseReleased(e);
        mousePixelPositionX = -1;
        repaint();
      }
    };

    private final Color SEPARATOR_COLOR = new Color(220, 220, 220);
    public void paintComponent(Graphics g) {
      Rectangle bounds = g.getClipBounds();
      /*logger.info("Clip bounds: " + bounds);*/
      long intervalStart = bounds.x*currentPixelDivisor;
      long intervalEnd = intervalStart + bounds.width*currentPixelDivisor;

      if (intervalEnd > simulation.getSimulationTime()) {
        intervalEnd = simulation.getSimulationTime();
      }

      /*logger.info("Painting interval: " + intervalStart + " -> " + intervalEnd);*/
      if (bounds.x > Integer.MAX_VALUE - 1000) {
        /* TODO Strange bounds */
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
        if (showRadioHW) {
          MoteEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioHWEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showLEDs) {
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
              (int) time/currentPixelDivisor, (int)0, 
              (int) time/currentPixelDivisor, (int)TIME_MARKER_PIXEL_HEIGHT);
        } else {
          g.drawLine(
              (int) time/currentPixelDivisor, (int)0, 
              (int) time/currentPixelDivisor, (int)TIME_MARKER_PIXEL_HEIGHT/2);
        }          
        time += (10*Simulation.MILLISECOND);
      }
    }

    private void drawMouseTime(Graphics g, long start, long end) {
      if (mousePixelPositionX >= 0) {
        String str = "Time (ms): " + ((double)mousePixelPositionX*currentPixelDivisor/Simulation.MILLISECOND);
        int h = g.getFontMetrics().getHeight();
        int w = g.getFontMetrics().stringWidth(str) + 6;
        int y=mousePixelPositionY<getHeight()/2?0:getHeight()-h;
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

    public String getToolTipText(MouseEvent event) {
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
      long time = event.getPoint().x*(long)currentPixelDivisor;
      tooltip += "Time (ms): " + (double)(time/Simulation.MILLISECOND) + "<br>";

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
      if (showRadioHW) {
        if (evMatched == evMouse) {
          events = allMoteEvents.get(mote).radioHWEvents;
        }
        evMatched++;
      }
      if (showLEDs) {
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
        }
      }

      tooltip += "</html>";
      return tooltip;
    }
  }

  class MoteRuler extends JPanel {
    public MoteRuler() {
      setPreferredSize(new Dimension(35, 1));
      setToolTipText("");
      setBackground(COLOR_BACKGROUND);

      final JPopupMenu popupMenu = new JPopupMenu();
      final JMenuItem removeItem = new JMenuItem(removeMoteAction);
      removeItem.setText("Remove from timeline");
      popupMenu.add(removeItem);

      addMouseListener(new MouseAdapter() {
        public void mouseClicked(MouseEvent e) {
          Mote m = getMote(e.getPoint());
          if (m == null) {
            return;
          }
          removeItem.setText("Remove from timeline: " + m);
          removeItem.putClientProperty("mote", m);
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
        String str = "??";
        if (moteLog.mote.getInterfaces().getMoteID() != null) {
          str = "" + moteLog.mote.getInterfaces().getMoteID().getMoteID();
        }
        int w = g.getFontMetrics().stringWidth(str) + 1;

        /*g.drawRect(0, y, getWidth()-1, paintedMoteHeight);*/
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
          w = (int) (ev.next.time - ev.time)/currentPixelDivisor;
        } else {
          w = (int) (end - ev.time)/currentPixelDivisor; /* No more events */
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
  class RadioRXTXEvent extends MoteEvent {
    RadioEvent state = null;
    public RadioRXTXEvent(long time, RadioEvent ev) {
      super(time);
      this.state = ev;
    }
    public Color getEventColor() {
      if (state == RadioEvent.TRANSMISSION_STARTED) {
        return Color.BLUE;
      } else if (state == RadioEvent.RECEPTION_STARTED) {
        return Color.GREEN;
      } else if (state == RadioEvent.RECEPTION_INTERFERED) {
        return Color.RED;
      } else {
        return null;
      }
    }
    public String toString() {
      if (state == RadioEvent.TRANSMISSION_STARTED) {
        return "Radio TX started at " + time + "<br>";
      } else if (state == RadioEvent.TRANSMISSION_FINISHED) {
        return "Radio TX finished at " + time + "<br>";
      } else if (state == RadioEvent.RECEPTION_STARTED) {
        return "Radio RX started at " + time + "<br>";
      } else if (state == RadioEvent.RECEPTION_FINISHED) {
        return "Radio RX finished at " + time + "<br>";
      } else if (state == RadioEvent.RECEPTION_INTERFERED) {
        return "Radio reception was interfered at " + time + "<br>";
      }
      return "Unknown event<br>";
    }
  }
  class RadioChannelEvent extends MoteEvent {
    public RadioChannelEvent(long time) {
      super(time);
    }
    public Color getEventColor() {
      return Color.GRAY; /* TODO Implement me */
    }
  }
  class RadioHWEvent extends MoteEvent {
    boolean on;
    public RadioHWEvent(long time, boolean on) {
      super(time);
      this.on = on;
    }
    public Color getEventColor() {
      return on?Color.GRAY:null;
    }
    public String toString() {
      return "Radio HW was turned " + (on?"on":"off") + " at time " + time + "<br>";
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
          w = (int) (ev.next.time - ev.time)/currentPixelDivisor;
        } else {
          w = (int) (end - ev.time)/currentPixelDivisor; /* No more events */
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
    public LogEvent(long time) {
      super(time);
    }
    public Color getEventColor() {
      return Color.GRAY; /* TODO Implement me */
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

      /* TODO XXX Requires MSPSim changes */
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
      Rectangle visibleRectangle = timeline.getVisibleRect(); 
      boolean isTracking = visibleRectangle.x + visibleRectangle.width >= timeline.getWidth();

      /* Only set new size if simulation time has changed */
      if (simulation.getSimulationTime() == lastRepaintSimulationTime) {
        return;
      }
      lastRepaintSimulationTime = simulation.getSimulationTime(); 

      /* Update timeline size */
      int newWidth = (int) (simulation.getSimulationTime()/currentPixelDivisor);
      int newHeight = (int) (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size()); 
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
            newWidth-1, 0, 
            1, 1);
        timeline.scrollRectToVisible(r);
      }
    }
  });

}
