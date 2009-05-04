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
 * $Id: TimeLine.java,v 1.1 2009/05/04 15:38:35 fros4943 Exp $
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
import java.util.*;

import javax.swing.*;

import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.*;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.interfaces.Log;
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
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(TimeLine.class);

  private static Color COLOR_BACKGROUND = Color.WHITE;
  public static final int EVENT_PIXEL_HEIGHT = 4;
  public static final int TIME_MARKER_PIXEL_HEIGHT = 6;
  public static final int FIRST_MOTE_PIXEL_OFFSET = TIME_MARKER_PIXEL_HEIGHT + EVENT_PIXEL_HEIGHT;

  private int paintedMoteHeight = EVENT_PIXEL_HEIGHT;

  private Simulation simulation;
  private JScrollPane timelineScrollPane;
  private MoteRuler timelineMoteRuler;
  private JPanel timeline;
  private Box eventCheckboxes;

  private Observer tickObserver;
  private ArrayList<MoteObservation> activeMoteObservers = new ArrayList<MoteObservation>();

  private ArrayList<MoteEvents> allMoteEvents = new ArrayList<MoteEvents>();

  private long startTime;

  private boolean viewportTracking = true;
  private Point viewportInfinite = new Point(Integer.MAX_VALUE, 0);
  private Runnable updateViewport = new Runnable() {
    public void run() {
      if (!viewportTracking) {
        return;    
      }
      timelineScrollPane.getViewport().setViewPosition(viewportInfinite);
    }
  };

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
    startTime = simulation.getSimulationTime();

    /* Automatically repaint every tick */
    simulation.addTickObserver(tickObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (timelineScrollPane == null)
          return;

        timeline.setPreferredSize(new Dimension(
            (int) (simulation.getSimulationTime() - startTime),
            (int) (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size())
        ));
        timelineMoteRuler.setPreferredSize(new Dimension(
            35,
            (int) (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size())
        ));
        timeline.revalidate();
        timeline.repaint();
        
        if (viewportTracking) {
          SwingUtilities.invokeLater(updateViewport);
        }
      }
    });

    /* Box: events to observe */
    eventCheckboxes = Box.createVerticalBox();
    JCheckBox eventCheckBox;
    eventCheckBox = createEventCheckbox("Radio RX/TX", "Show radio transmissions, receptions, and collisions");
    eventCheckBox.setName("showRadioRXTX");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioRXTX = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("Radio channels", "Show different radio channels");
    eventCheckBox.setName("showRadioChannels");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioChannels = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    /*eventCheckboxes.add(eventCheckBox);*/
    eventCheckBox = createEventCheckbox("Radio ON/OFF", "Show radio hardware state");
    eventCheckBox.setName("showRadioHW");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showRadioHW = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("LEDs", "Show LED state");
    eventCheckBox.setName("showLEDs");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLEDs = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    eventCheckboxes.add(eventCheckBox);
    eventCheckBox = createEventCheckbox("Log output", "Show mote log output, such as by printf()'s");
    eventCheckBox.setName("showLogOutput");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showLogOutputs = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    /*eventCheckboxes.add(eventCheckBox);*/
    eventCheckBox = createEventCheckbox("Watchpoints", "Show code watchpoints configurable on MSPSim based motes");
    eventCheckBox.setName("showWatchpoints");
    eventCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        showWatchpoints = ((JCheckBox) e.getSource()).isSelected();
        recalculateMoteHeight();
      }
    });
    /*eventCheckboxes.add(eventCheckBox);*/

    /* Panel: timeline canvas w. scroll pane and add mote button */
    timeline = new Timeline();
    timelineScrollPane = new JScrollPane(
        timeline,
        JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
        JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    timelineScrollPane.getHorizontalScrollBar().addMouseMotionListener(new MouseAdapter() {
      public void mouseDragged(MouseEvent e) {
        Rectangle view = timelineScrollPane.getViewport().getViewRect();
        if (view.x + view.width + 5 > simulation.getSimulationTime()) {
          viewportInfinite.y = view.y;
          viewportTracking = true;
        } else {
          viewportTracking = false;
        }
        timeline.revalidate();
        timeline.repaint();
      }
    });
    timelineScrollPane.getVerticalScrollBar().addMouseMotionListener(new MouseAdapter() {
      public void mouseDragged(MouseEvent e) {
        Rectangle view = timelineScrollPane.getViewport().getViewRect();
        viewportInfinite.y = view.y;
        timeline.revalidate();
        timeline.repaint();
      }
    });

    JButton timelineAddMoteButton = new JButton(addMoteAction);
    timelineAddMoteButton.setText("+");
    timelineAddMoteButton.setToolTipText("Add mote");
    timelineAddMoteButton.setBorderPainted(false);
    timelineAddMoteButton.setFont(new Font("SansSerif", Font.PLAIN, 11));

    timelineMoteRuler = new MoteRuler();
    timelineScrollPane.setRowHeaderView(timelineMoteRuler);
    timelineScrollPane.setCorner(JScrollPane.LOWER_LEFT_CORNER, timelineAddMoteButton);
    timelineScrollPane.setBackground(Color.WHITE);

    JSplitPane splitPane = new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        eventCheckboxes,
        timelineScrollPane
    );
    splitPane.setOneTouchExpandable(true);
    splitPane.setResizeWeight(0.0);

    getContentPane().add(splitPane);

    pack();
    setSize(gui.getDesktopPane().getWidth(), 150);
    setLocation(0, gui.getDesktopPane().getHeight() - 150);

    numberMotesWasUpdated();
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

  private void numberMotesWasUpdated() {
    /* Timeline */
    timeline.setPreferredSize(new Dimension(
        (int) (simulation.getSimulationTime() - startTime),
        (int) (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size())
    ));
    timelineMoteRuler.setPreferredSize(new Dimension(
        35,
        (int) (FIRST_MOTE_PIXEL_OFFSET + paintedMoteHeight * allMoteEvents.size())
    ));
    timelineMoteRuler.revalidate();
    timelineMoteRuler.repaint();
    timeline.revalidate();
    timeline.repaint();

    /* Plugin title */
    if (allMoteEvents.isEmpty()) {
      setTitle("Timeline (Add motes to observe by clicking +)");
    } else {
      setTitle("Timeline (" + allMoteEvents.size() + " motes)");
    }
  }

  /* XXX Keeps track of observed mote interfaces */
  class MoteObservation {
    private Observer observer;
    private Observable observable;
    private Mote mote;

    public MoteObservation(Mote mote, Observable observable, Observer observer) {
      this.mote = mote;
      this.observable = observable;
      this.observer = observer;
    }

    public Mote getMote() {
      return mote;
    }

    /**
     * Disconnect observer from observable (stop observing) and clean up resources (remove pointers).
     */
    public void dispose() {
      observable.deleteObserver(observer);
      mote = null;
      observable = null;
      observer = null;
    }
  }

  private void addMoteObservers(Mote mote, final MoteEvents moteEvents) {
    final LED moteLEDs = mote.getInterfaces().getLED();
    final Radio moteRadio = mote.getInterfaces().getRadio();
    final Log moteLog = mote.getInterfaces().getLog();
    /* TODO Watchpoints? */

    /* LEDs */
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

    /* Radio HW */
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

  }

  private void addMote(Mote newMote) {
    if (newMote != null) {
      for (MoteEvents moteEvents: allMoteEvents) {
        if (moteEvents.mote == newMote) {
          return;
        }
      }
      
      MoteEvents newMoteLog = new MoteEvents(newMote);
      allMoteEvents.add(newMoteLog);
      addMoteObservers(newMote, newMoteLog);
    }

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
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showLogOutputs) {
      h += EVENT_PIXEL_HEIGHT;
    }
    if (showWatchpoints) {
      h += EVENT_PIXEL_HEIGHT;
    }
    paintedMoteHeight = h;
    timelineMoteRuler.revalidate();
    timelineMoteRuler.repaint();
    timeline.revalidate();
    timeline.repaint();
  }

  public void closePlugin() {
    simulation.deleteTickObserver(tickObserver);

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

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    showRadioRXTX = false;
    showRadioChannels = false;
    showRadioHW = false;
    showLEDs = false;
    showLogOutputs = false;
    showWatchpoints = false;

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

  class Timeline extends JPanel {

    private int mouseTimePositionX = -1;
    private int mouseTimePositionY = -1;

    private MouseAdapter mouseAdapter = new MouseAdapter() {
      public void mouseDragged(MouseEvent e) {
        super.mouseDragged(e);
        if (mouseTimePositionX >= 0) {
          mouseTimePositionX = e.getX();
          mouseTimePositionY = e.getY();
          repaint();
        }
      }
      public void mousePressed(MouseEvent e) {
        if (e.getPoint().getY() < FIRST_MOTE_PIXEL_OFFSET) {
          mouseTimePositionX = e.getX();
          mouseTimePositionY = e.getY();
          repaint();
        }
      }
      public void mouseReleased(MouseEvent e) {
        super.mouseReleased(e);
        mouseTimePositionX = -1;
        repaint();
      }
    };

    public Timeline() {
      setToolTipText("");
      setBackground(COLOR_BACKGROUND);

      addMouseListener(mouseAdapter);
      addMouseMotionListener(mouseAdapter);
    }

    public void paintComponent(Graphics g) {
      /*logger.info("Painting timeline: " + startTime + " -> " + (startTime + timeline.getWidth()));*/

      Rectangle bounds = g.getClipBounds();
      long intervalStart = bounds.x + startTime;
      long intervalEnd = intervalStart + bounds.width;
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
      for (int mIndex = 0; mIndex < allMoteEvents.size(); mIndex++) {
        if (showRadioRXTX) {
          RadioRXTXEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioRXTXEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showRadioChannels) {
          RadioChannelEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioChannelEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showRadioHW) {
          RadioHWEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).radioHWEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showLEDs) {
          LEDEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).ledEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showLogOutputs) {
          LogEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).logEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }
        if (showWatchpoints) {
          WatchpointEvent firstEvent = getFirstIntervalEvent(allMoteEvents.get(mIndex).watchpointEvents, intervalStart);
          if (firstEvent != null) {
            firstEvent.paintInterval(g, lineHeightOffset, (int) startTime, intervalEnd);
          }
          lineHeightOffset += EVENT_PIXEL_HEIGHT;
        }

        lineHeightOffset += EVENT_PIXEL_HEIGHT;
      }

      /* Draw vertical time marker (mouse dragged) */
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
      long millis;

      /* Paint 10ms and 100 ms markers */
      g.setColor(Color.GRAY);

      millis = start - (start % 100);
      while (millis <= end) {
        if (millis % 100 == 0) {
          g.drawLine(
              (int)(millis - startTime), (int)0, 
              (int)(millis - startTime), (int)TIME_MARKER_PIXEL_HEIGHT);
        } else {
          g.drawLine(
              (int)(millis - startTime), (int)0, 
              (int)(millis - startTime), (int)TIME_MARKER_PIXEL_HEIGHT/2);
        }          
        millis += 10;
      }
    }

    private void drawMouseTime(Graphics g, long start, long end) {
      if (mouseTimePositionX >= 0) {
        String str = "Time: " + (mouseTimePositionX + startTime);
        int h = g.getFontMetrics().getHeight();
        int w = g.getFontMetrics().stringWidth(str) + 6;
        int y=mouseTimePositionY<getHeight()/2?0:getHeight()-h;
        int delta = mouseTimePositionX + w > end?w:0; /* Don't write outside visible area */

        /* Line */
        g.setColor(Color.GRAY);
        g.drawLine(
            mouseTimePositionX, 0, 
            mouseTimePositionX, getHeight());

        /* Text box */
        g.setColor(Color.DARK_GRAY);
        g.fillRect(
            mouseTimePositionX-delta, y, 
            w, h);
        g.setColor(Color.BLACK);
        g.drawRect(
            mouseTimePositionX-delta, y, 
            w, h);
        g.setColor(Color.WHITE);
        g.drawString(str, 
            mouseTimePositionX+3-delta, 
            y+h-1);
      }
    }

    public int getWidth() {
      return (int) (simulation.getSimulationTime() - startTime); 
    }

    public String getToolTipText(MouseEvent event) {
      if (event.getPoint().y <= FIRST_MOTE_PIXEL_OFFSET) {
        return "<html>Click to display time</html>";
      }

      /* Mote */
      int mote = (event.getPoint().y-FIRST_MOTE_PIXEL_OFFSET)/paintedMoteHeight;
      if (mote < 0 || mote >= allMoteEvents.size()) {
        return null;
      }      
      String tooltip = "<html>Mote: " + allMoteEvents.get(mote).mote + "<br>";

      /* Time */
      long time = event.getPoint().x + startTime;
      tooltip += "Time: " + time + "<br>";

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
    long time;
    public MoteEvent(long time) {
      this.time = time;
    }
  }
  class RadioRXTXEvent extends MoteEvent {
    RadioRXTXEvent prev = null;
    RadioRXTXEvent next = null;
    RadioEvent state = null;
    public RadioRXTXEvent(long time, RadioEvent ev) {
      super(time);
      this.state = ev;
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      RadioRXTXEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }
        
        if (ev.state == RadioEvent.TRANSMISSION_STARTED) {
          g.setColor(Color.BLUE);
        } else if (ev.state == RadioEvent.RECEPTION_STARTED) {
          g.setColor(Color.GREEN);
        } else if (ev.state == RadioEvent.RECEPTION_INTERFERED) {
          g.setColor(Color.RED);
        } else {
          /*g.setColor(Color.LIGHT_GRAY);*/
          ev = ev.next;
          continue;
        }
        
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
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
    RadioChannelEvent prev = null;
    RadioChannelEvent next = null;
    public RadioChannelEvent(long time) {
      super(time);
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      RadioChannelEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }

        g.setColor(Color.GRAY);
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
      }
    }
  }
  class RadioHWEvent extends MoteEvent {
    RadioHWEvent prev = null;
    RadioHWEvent next = null;
    boolean on;
    public RadioHWEvent(long time, boolean on) {
      super(time);
      this.on = on;
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      RadioHWEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }

        if (!ev.on) {
          ev = ev.next;
          continue;
        }
        g.setColor(Color.GRAY);
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );
        
        ev = ev.next;
      }
    }
    public String toString() {
      return "Radio HW was turned " + (on?"on":"off") + " at time " + time + "<br>";
    }
  }
  class LEDEvent extends MoteEvent {
    LEDEvent prev = null;
    LEDEvent next = null;
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
      prev = null;
      next = null;
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      LEDEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }

        if (!ev.red && !ev.green && !ev.blue) {
          g.setColor(Color.WHITE);
        } else if (ev.red && ev.green && ev.blue) {
          g.setColor(Color.LIGHT_GRAY);
        } else {
          g.setColor(ev.color);
        }
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );

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
    LogEvent prev = null;
    LogEvent next = null;
    public LogEvent(long time) {
      super(time);
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      LogEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }

        g.setColor(Color.GREEN);
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
      }
    }

  }
  class WatchpointEvent extends MoteEvent {
    WatchpointEvent prev = null;
    WatchpointEvent next = null;
    public WatchpointEvent(long time) {
      super(time);
    }
    public void paintInterval(Graphics g, int lineHeightOffset, int startTime, long end) {
      WatchpointEvent ev = this;
      while (ev != null && ev.time < end) {
        int w;

        /* Paint until next event or end of clip */
        if (ev.next != null) {
          w = (int) (ev.next.time - ev.time);
        } else {
          w = (int) (end - ev.time); /* No more events */
        }

        /* Ignore painting events with zero width */
        if (w == 0) {
          ev = ev.next;
          continue;
        }

        g.setColor(Color.BLUE);
        g.fillRect(
            (int)(ev.time - startTime), lineHeightOffset, 
            w, EVENT_PIXEL_HEIGHT
        );

        ev = ev.next;
      }
    }

  }
  class MoteEvents {
    Mote mote;
    ArrayList<RadioRXTXEvent> radioRXTXEvents;
    ArrayList<RadioChannelEvent> radioChannelEvents;
    ArrayList<RadioHWEvent> radioHWEvents;
    ArrayList<LEDEvent> ledEvents; 
    ArrayList<LogEvent> logEvents;
    ArrayList<WatchpointEvent> watchpointEvents;

    private RadioRXTXEvent lastRadioRXTXEvent = null;
    private RadioChannelEvent lastRadioChannelEvent = null;
    private RadioHWEvent lastRadioHWEvent = null;
    private LEDEvent lastLEDEvent = null;
    private LogEvent lastLogEvent = null;
    private WatchpointEvent lastWatchpointEvent = null;

    public MoteEvents(Mote mote) {
      this.mote = mote;
      this.radioRXTXEvents = new ArrayList<RadioRXTXEvent>();
      this.radioChannelEvents = new ArrayList<RadioChannelEvent>();
      this.radioHWEvents = new ArrayList<RadioHWEvent>();
      this.ledEvents = new ArrayList<LEDEvent>();
      this.logEvents = new ArrayList<LogEvent>();
      this.watchpointEvents = new ArrayList<WatchpointEvent>();
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

}
