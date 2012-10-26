/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package se.sics.cooja.plugins;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.contikimote.ContikiMoteType;
import se.sics.cooja.interfaces.Button;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.PIR;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;

/**
 * Allows a user to observe several different parts of the simulator, stopping a
 * simulation whenever an object changes.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Breakpoints")
@PluginType(PluginType.SIM_PLUGIN)
public class EventListener extends VisPlugin {
  private static final long serialVersionUID = 1L;

  private static Logger logger = Logger.getLogger(EventListener.class);

  private Simulation mySimulation;

  private Vector<EventObserver> allObservers = new Vector<EventObserver>();

  private EventListener myPlugin;

  private JLabel messageLabel = null;

  private JButton actionButton = null;

  private JPanel interfacePanel = null;

  private JPanel generalPanel = null;

  protected abstract class EventObserver implements Observer {
    protected Observable myObservation = null;

    protected EventListener myParent = null;

    public EventObserver(EventListener parent, Observable objectToObserve) {
      myParent = parent;
      myObservation = objectToObserve;
      objectToObserve.addObserver(this);
    }

    /**
     * Stop observing object (for cleaning up).
     */
    public void detachFromObject() {
      myObservation.deleteObserver(this);
    }

    /**
     * @return Object being observed.
     */
    public Observable getObservable() {
      return myObservation;
    }
  };

  protected class InterfaceEventObserver extends EventObserver {
    private Mote myMote = null;

    public InterfaceEventObserver(EventListener parent, Mote mote,
        Observable objectToObserve) {
      super(parent, objectToObserve);
      myMote = mote;
    }

    public void update(Observable obs, Object obj) {
      final MoteInterface moteInterface = (MoteInterface) obs;
      int moteID = myMote.getID();

      myParent.actOnChange("'" + GUI.getDescriptionOf(moteInterface.getClass())
          + "'" + " of mote '" + (moteID > 0 ? Integer.toString(moteID) : "?")
          + "'" + " changed at time "
          + myParent.mySimulation.getSimulationTime(), new AbstractAction(
          "View interface visualizer") {
        public void actionPerformed(ActionEvent e) {
          MoteInterfaceViewer plugin =
            (MoteInterfaceViewer) mySimulation.getGUI().tryStartPlugin(
                MoteInterfaceViewer.class, mySimulation.getGUI(), mySimulation, myMote);
          plugin.setSelectedInterface(GUI.getDescriptionOf(moteInterface.getClass()));
        }
      });
    }
  }

  protected class GeneralEventObserver extends EventObserver {
    public GeneralEventObserver(EventListener parent, Observable objectToObserve) {
      super(parent, objectToObserve);
    }

    public void update(Observable obs, Object obj) {
      myParent.actOnChange("'" + GUI.getDescriptionOf(obs.getClass()) + "'"
          + " changed at time " + myParent.mySimulation.getSimulationTime(),
          null);
    }
  }

  /**
   * @param simulationToControl
   *          Simulation to control
   */
  public EventListener(Simulation simulationToControl, GUI gui) {
    super("Event Listener", gui);

    mySimulation = simulationToControl;
    myPlugin = this;

    /* Create selectable interfaces list (only supports Contiki mote types) */
    Vector<Class<? extends MoteInterface>> allInterfaces = new Vector<Class<? extends MoteInterface>>();
    Vector<Class<? extends MoteInterface>> allInterfacesDups = new Vector<Class<? extends MoteInterface>>();

    // Add standard interfaces
    allInterfacesDups.add(Button.class);
    allInterfacesDups.add(LED.class);
    allInterfacesDups.add(Log.class);
    allInterfacesDups.add(PIR.class);
    allInterfacesDups.add(Position.class);
    allInterfacesDups.add(Radio.class);

    for (MoteType moteType : simulationToControl.getMoteTypes()) {
      if (moteType instanceof ContikiMoteType) {
        Class<? extends MoteInterface>[] arr = ((ContikiMoteType) moteType).getMoteInterfaceClasses();
        for (Class<? extends MoteInterface> intf : arr) {
          allInterfacesDups.add(intf);
        }
      }
    }
    for (Class<? extends MoteInterface> moteTypeClass : allInterfacesDups) {
      if (!allInterfaces.contains(moteTypeClass)) {
        allInterfaces.add(moteTypeClass);
      }
    }

    interfacePanel = new JPanel();
    interfacePanel.setLayout(new BoxLayout(interfacePanel, BoxLayout.Y_AXIS));
    interfacePanel.setBorder(BorderFactory.createEmptyBorder(10, 5, 10, 5));
    for (Class<? extends MoteInterface> interfaceClass : allInterfaces) {
      JCheckBox checkBox = new JCheckBox(GUI.getDescriptionOf(interfaceClass),
          false);
      checkBox.setToolTipText(interfaceClass.getName());
      checkBox.putClientProperty("interface_class", interfaceClass);
      checkBox.addActionListener(interfaceCheckBoxListener);

      interfacePanel.add(checkBox);
    }
    if (allInterfaces.isEmpty()) {
      interfacePanel.add(new JLabel("No used interface classes detected"));
    }

    // Create general selectable list
    generalPanel = new JPanel();
    generalPanel.setLayout(new BoxLayout(generalPanel, BoxLayout.Y_AXIS));
    generalPanel.setBorder(BorderFactory.createEmptyBorder(10, 5, 10, 5));

    JCheckBox simCheckBox = new JCheckBox("Simulation event", false);
    simCheckBox.putClientProperty("observable", mySimulation);
    simCheckBox.addActionListener(generalCheckBoxListener);
    generalPanel.add(simCheckBox);

    JCheckBox radioMediumCheckBox = new JCheckBox("Radio medium event", false);
    radioMediumCheckBox.putClientProperty("observable", mySimulation
        .getRadioMedium().getRadioMediumObservable());
    radioMediumCheckBox.addActionListener(generalCheckBoxListener);
    generalPanel.add(radioMediumCheckBox);

    // Add components
    JPanel mainPanel = new JPanel();
    mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
    setContentPane(mainPanel);

    mainPanel.add(new JLabel("Break on general changes:"));
    mainPanel.add(generalPanel);

    mainPanel.add(new JLabel("Break on mote interface changes:"));
    mainPanel.add(new JScrollPane(interfacePanel));

    messageLabel = new JLabel("[no change detected yet]");
    actionButton = new JButton("[no action available]");
    actionButton.setEnabled(false);
    mainPanel.add(new JLabel("Last message:"));
    mainPanel.add(messageLabel);
    mainPanel.add(actionButton);

    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  private void actOnChange(final String message, final Action action) {
    if (!mySimulation.isRunning()) {
      return;
    }

    mySimulation.stopSimulation();

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        messageLabel.setText(message);
        actionButton.setAction(action);
        actionButton.setVisible(action != null);
      }
    });
  }

  private ActionListener interfaceCheckBoxListener = new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      Class<? extends MoteInterface> interfaceClass = (Class<? extends MoteInterface>) ((JCheckBox) e
          .getSource()).getClientProperty("interface_class");
      boolean shouldObserve = ((JCheckBox) e.getSource()).isSelected();

      if (!shouldObserve) {
        // Remove existing observers
        for (EventObserver obs : allObservers.toArray(new EventObserver[0])) {
          Class<? extends Observable> objClass = obs.getObservable().getClass();
          if (objClass == interfaceClass ||
              interfaceClass.isAssignableFrom(objClass)) {
            obs.detachFromObject();
            allObservers.remove(obs);
          }
        }
      } else {
        // Register new observers
        for (int i = 0; i < mySimulation.getMotesCount(); i++) {
          MoteInterface moteInterface = mySimulation.getMote(i).getInterfaces()
              .getInterfaceOfType(interfaceClass);
          if (moteInterface != null) {
            allObservers.add(new InterfaceEventObserver(myPlugin, mySimulation
                .getMote(i), moteInterface));
          }
        }
      }
    }
  };

  private ActionListener generalCheckBoxListener = new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      Observable observable = (Observable) ((JCheckBox) e.getSource())
          .getClientProperty("observable");
      boolean shouldObserve = ((JCheckBox) e.getSource()).isSelected();

      if (!shouldObserve) {
        // Remove existing observers
        for (EventObserver obs : allObservers.toArray(new EventObserver[0])) {
          if (obs.getObservable() == observable) {
            obs.detachFromObject();
            allObservers.remove(obs);
          }
        }
      } else {
        // Register new observers
        allObservers.add(new GeneralEventObserver(myPlugin, observable));
      }
    }
  };

  public void closePlugin() {
    // Remove all existing observers
    for (EventObserver obs : allObservers) {
      obs.detachFromObject();
    }
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    /* Save general observers */
    for (Component comp: generalPanel.getComponents()) {
      if (comp instanceof JCheckBox) {
        JCheckBox checkBox = (JCheckBox) comp;
        if (checkBox.isSelected()) {
          element = new Element("general");
          element.setText(checkBox.getText());
          config.add(element);
        }
      }
    }

    /* Save interface observers */
    for (Component comp: interfacePanel.getComponents()) {
      if (comp instanceof JCheckBox) {
        JCheckBox checkBox = (JCheckBox) comp;
        if (checkBox.isSelected()) {
          element = new Element("interface");
          element.setText(checkBox.getText());
          config.add(element);
        }
      }
    }

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {

    /* Load general observers */
    for (Element element : configXML) {
      if (element.getName().equals("general")) {
        for (Component comp: generalPanel.getComponents()) {
          if (comp instanceof JCheckBox) {
            JCheckBox checkBox = (JCheckBox) comp;
            if (checkBox.getText().equals(element.getText())) {
              checkBox.setSelected(true);
              generalCheckBoxListener.actionPerformed(new ActionEvent(checkBox, ActionEvent.ACTION_PERFORMED, ""));
            }
          }
        }
      }

      /* Load interface observers */
      else if (element.getName().equals("interface")) {
        for (Component comp: interfacePanel.getComponents()) {
          if (comp instanceof JCheckBox) {
            JCheckBox checkBox = (JCheckBox) comp;
            if (checkBox.getText().equals(element.getText())) {
              checkBox.setSelected(true);
              interfaceCheckBoxListener.actionPerformed(new ActionEvent(checkBox, ActionEvent.ACTION_PERFORMED, ""));
            }
          }
        }
      }

      else {
        return false;
      }
    }

    return true;
  }

}
