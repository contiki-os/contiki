/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: MoteInterfaceViewer.java,v 1.5 2009/03/09 15:39:33 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.util.Collection;
import java.util.Vector;
import javax.swing.*;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * Mote Interface Viewer views information about a specific mote interface.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote Interface Viewer")
@PluginType(PluginType.MOTE_PLUGIN)
public class MoteInterfaceViewer extends VisPlugin {
  private static final long serialVersionUID = 1L;

  private Mote mote;
  private MoteInterface selectedMoteInterface = null;
  private JPanel currentInterfaceVisualizer = null;
  private JComboBox selectInterfaceComboBox = null;

  /**
   * Create a new mote interface viewer.
   *
   * @param moteToView Mote to view
   */
  public MoteInterfaceViewer(Mote moteToView, Simulation simulation, GUI gui) {
    super("Mote Interface Viewer (" + moteToView + ")", gui);
    mote = moteToView;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    mainPane.setLayout(new BorderLayout());
    JPanel smallPane;

    // Select interface combo box
    smallPane = new JPanel(new BorderLayout());

    label = new JLabel("Select interface:");

    selectInterfaceComboBox = new JComboBox();
    final JPanel interfacePanel = new JPanel();

    Vector<MoteInterface> intfs = mote.getInterfaces().getInterfaces();
    for (MoteInterface intf : intfs) {
      selectInterfaceComboBox.addItem(GUI.getDescriptionOf(intf));
    }

    selectInterfaceComboBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {

        // Release old interface visualizer if any
        if (selectedMoteInterface != null && currentInterfaceVisualizer != null) {
          selectedMoteInterface.releaseInterfaceVisualizer(currentInterfaceVisualizer);
        }

        // View selected interface if any
        interfacePanel.removeAll();
        String interfaceDescription = (String) selectInterfaceComboBox.getSelectedItem();
        selectedMoteInterface = null;
        Vector<MoteInterface> intfs = mote.getInterfaces().getInterfaces();
        for (MoteInterface intf : intfs) {
          if (GUI.getDescriptionOf(intf).equals(interfaceDescription)) {
            selectedMoteInterface = intf;
          }
        }
        currentInterfaceVisualizer = selectedMoteInterface.getInterfaceVisualizer();
        if (currentInterfaceVisualizer != null) {
          currentInterfaceVisualizer.setBorder(BorderFactory.createEtchedBorder());
          interfacePanel.add(BorderLayout.CENTER, currentInterfaceVisualizer);
          currentInterfaceVisualizer.setVisible(true);
        } else {
          interfacePanel.add(new JLabel("No interface visualizer"));
          currentInterfaceVisualizer = null;
        }
        setSize(getSize());
      }
    });
    selectInterfaceComboBox.setSelectedIndex(0);

    smallPane.add(BorderLayout.WEST, label);
    smallPane.add(BorderLayout.EAST, selectInterfaceComboBox);
    mainPane.add(BorderLayout.NORTH, smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,10)));

    // Add selected interface
    interfacePanel.setLayout(new BorderLayout());
    if (selectInterfaceComboBox.getItemCount() > 0) {
      selectInterfaceComboBox.setSelectedIndex(0);
      selectInterfaceComboBox.dispatchEvent(new ActionEvent(selectInterfaceComboBox, ActionEvent.ACTION_PERFORMED, ""));
    }

    mainPane.add(BorderLayout.CENTER, interfacePanel);

    this.setContentPane(new JScrollPane(mainPane,
					JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
					JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
    pack();
    setPreferredSize(new Dimension(350,300));
    setSize(new Dimension(350,300));

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  /**
   * Tries to select the interface with the given class name.
   * @param description Interface description
   * @return True if selected, false otherwise
   */
  public boolean setSelectedInterface(String description) {
    for (int i=0; i < selectInterfaceComboBox.getItemCount(); i++) {
      if (selectInterfaceComboBox.getItemAt(i).equals(description)) {
        selectInterfaceComboBox.setSelectedIndex(i);
        return true;
      }
    }
    return false;
  }

  public void closePlugin() {
    // Release old interface visualizer if any
    if (selectedMoteInterface != null && currentInterfaceVisualizer != null) {
      selectedMoteInterface.releaseInterfaceVisualizer(currentInterfaceVisualizer);
    }
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Selected variable name
    element = new Element("interface");
    element.setText((String) selectInterfaceComboBox.getSelectedItem());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("interface")) {
        setSelectedInterface(element.getText());
      }
    }
    return true;
  }

}
