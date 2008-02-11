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
 * $Id: VariableWatcher.java,v 1.6 2008/02/11 14:03:19 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.text.NumberFormat;
import java.util.Collection;
import java.util.Vector;
import javax.swing.*;
import org.jdom.Element;
import se.sics.cooja.*;
import se.sics.cooja.AddressMemory.UnknownVariableException;

/**
 * Variable Watcher enables a user to watch mote variables during a simulation.
 * Variables can be read or written either as bytes, integers or byte arrays.
 *
 * User can also see which variables seems to be available on the selected node.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Variable Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
public class VariableWatcher extends VisPlugin {
  private static final long serialVersionUID = 1L;

  private AddressMemory moteMemory;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private final static int BYTE_INDEX = 0;
  private final static int INT_INDEX = 1;
  private final static int ARRAY_INDEX = 2;

  private JPanel lengthPane;
  private JPanel valuePane;
  private JComboBox varName;
  private JComboBox varType;
  private JFormattedTextField[] varValues;
  private JFormattedTextField varLength;
  private JButton writeButton;

  private NumberFormat integerFormat;

  /**
   * Create a variable watcher window.
   *
   * @param moteToView Mote to view
   */
  public VariableWatcher(Mote moteToView, Simulation simulation, GUI gui) {
    super("Variable Watcher (" + moteToView + ")", gui);

    moteMemory = (AddressMemory) moteToView.getMemory();

    JLabel label;
    integerFormat = NumberFormat.getIntegerInstance();
    JPanel mainPane = new JPanel();
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;

    // Variable name
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Variable name");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);

    varName = new JComboBox();
    varName.setEditable(true);
    varName.setSelectedItem("[enter or pick name]");

    String[] allPotentialVarNames = moteMemory.getVariableNames();
    for (String aVarName: allPotentialVarNames) {
      varName.addItem(aVarName);
    }

    varName.addKeyListener(new KeyListener() {
      public void keyPressed(KeyEvent e) {
        writeButton.setEnabled(false);
      }
      public void keyTyped(KeyEvent e) {
        writeButton.setEnabled(false);
      }
      public void keyReleased(KeyEvent e) {
        writeButton.setEnabled(false);
      }
    });

    smallPane.add(BorderLayout.EAST, varName);
    mainPane.add(smallPane);

    // Variable type
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Variable type");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);

    varType = new JComboBox();
    varType.addItem("Byte (1 byte)"); // BYTE_INDEX = 0
    varType.addItem("Integer (" + moteMemory.getIntegerLength() + " bytes)"); // INT_INDEX = 1
    varType.addItem("Byte array (x bytes)"); // ARRAY_INDEX = 2

    varType.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (varType.getSelectedIndex() == ARRAY_INDEX) {
          lengthPane.setVisible(true);
          setNumberOfValues(((Number) varLength.getValue()).intValue());
        } else {
          lengthPane.setVisible(false);
          setNumberOfValues(1);
        }
        pack();
      }
    });

    smallPane.add(BorderLayout.EAST, varType);
    mainPane.add(smallPane);

    // Variable length
    lengthPane = new JPanel(new BorderLayout());
    label = new JLabel("Variable length");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    lengthPane.add(BorderLayout.WEST, label);

    varLength = new JFormattedTextField(integerFormat);
    varLength.setValue(new Integer(1));
    varLength.setColumns(4);
    varLength.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        setNumberOfValues(((Number) varLength.getValue()).intValue());
      }
    });

    lengthPane.add(BorderLayout.EAST, varLength);
    mainPane.add(lengthPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    lengthPane.setVisible(false);

    // Variable value label
    label = new JLabel("Variable value");
    label.setAlignmentX(JLabel.CENTER_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    mainPane.add(label);

    // Variable value(s)
    valuePane = new JPanel();
    valuePane.setLayout(new BoxLayout(valuePane, BoxLayout.X_AXIS));

    varValues = new JFormattedTextField[1];
    varValues[0] = new JFormattedTextField(integerFormat);
    varValues[0].setValue(new Integer(0));
    varValues[0].setColumns(3);
    varValues[0].setText("?");

    for (JFormattedTextField varValue: varValues) {
      valuePane.add(varValue);

    }

    mainPane.add(valuePane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    // Read/write buttons
    smallPane = new JPanel(new BorderLayout());
    JButton button = new JButton("Read");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (varType.getSelectedIndex() == BYTE_INDEX) {
          try {
            byte val = moteMemory.getByteValueOf((String) varName.getSelectedItem());
            varValues[0].setValue(new Integer(val));
            varName.setBackground(Color.WHITE);
            writeButton.setEnabled(true);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
            writeButton.setEnabled(false);
          }
        } else if (varType.getSelectedIndex() == INT_INDEX) {
          try {
            int val = moteMemory.getIntValueOf((String) varName.getSelectedItem());
            varValues[0].setValue(new Integer(val));
            varName.setBackground(Color.WHITE);
            writeButton.setEnabled(true);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
            writeButton.setEnabled(false);
          }
        } else if (varType.getSelectedIndex() == ARRAY_INDEX) {
          try {
            int length = ((Number) varLength.getValue()).intValue();
            byte[] vals = moteMemory.getByteArray((String) varName.getSelectedItem(), length);
            for (int i=0; i < length; i++) {
              varValues[i].setValue(new Integer(vals[i]));
            }
            varName.setBackground(Color.WHITE);
            writeButton.setEnabled(true);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
            writeButton.setEnabled(false);
          }
        }
      }
    });
    smallPane.add(BorderLayout.WEST, button);

    button = new JButton("Write");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (varType.getSelectedIndex() == BYTE_INDEX) {
          try {
            byte val = (byte) ((Number) varValues[0].getValue()).intValue();
            moteMemory.setByteValueOf((String) varName.getSelectedItem(), val);
            varName.setBackground(Color.WHITE);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
          }
        } else if (varType.getSelectedIndex() == INT_INDEX) {
          try {
            int val = ((Number) varValues[0].getValue()).intValue();
            moteMemory.setIntValueOf((String) varName.getSelectedItem(), val);
            varName.setBackground(Color.WHITE);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
          }
        } else if (varType.getSelectedIndex() == ARRAY_INDEX) {
          try {
            int length = ((Number) varLength.getValue()).intValue();
            byte[] vals = new byte[length];
            for (int i=0; i < length; i++) {
              vals[i] = (byte) ((Number) varValues[i].getValue()).intValue();
            }

            moteMemory.setByteArray((String) varName.getSelectedItem(), vals);
            varName.setBackground(Color.WHITE);
            writeButton.setEnabled(true);
          } catch (UnknownVariableException ex) {
            varName.setBackground(Color.RED);
            writeButton.setEnabled(false);
          }
        }
      }
    });
    smallPane.add(BorderLayout.EAST, button);
    button.setEnabled(false);
    writeButton = button;


    mainPane.add(smallPane);
    mainPane.add(Box.createRigidArea(new Dimension(0,25)));

    this.setContentPane(new JScrollPane(mainPane,
					JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
					JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED));
    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  private void setNumberOfValues(int nr) {
    valuePane.removeAll();

    if (nr > 0) {
      varValues = new JFormattedTextField[nr];
      for (int i=0; i < nr; i++) {
        varValues[i] = new JFormattedTextField(integerFormat);
        varValues[i] .setValue(new Integer(0));
        varValues[i] .setColumns(3);
        varValues[i] .setText("?");
        valuePane.add(varValues[i]);
      }
    }
    pack();
  }

  public void closePlugin() {
  }

  public Collection<Element> getConfigXML() {
    // Return currently watched variable and type
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Selected variable name
    element = new Element("varname");
    element.setText((String) varName.getSelectedItem());
    config.add(element);

    // Selected variable type
    if (varType.getSelectedIndex() == BYTE_INDEX) {
      element = new Element("vartype");
      element.setText("byte");
      config.add(element);
    } else if (varType.getSelectedIndex() == INT_INDEX) {
      element = new Element("vartype");
      element.setText("int");
      config.add(element);
    } else if (varType.getSelectedIndex() == ARRAY_INDEX) {
      element = new Element("vartype");
      element.setText("array");
      config.add(element);
      element = new Element("array_length");
      element.setText(varLength.getValue().toString());
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    lengthPane.setVisible(false);
    setNumberOfValues(1);
    varLength.setValue(1);

    for (Element element : configXML) {
      if (element.getName().equals("varname")) {
        varName.setSelectedItem(element.getText());
      } else if (element.getName().equals("vartype")) {
        if (element.getText().equals("byte")) {
          varType.setSelectedIndex(BYTE_INDEX);
        } else if (element.getText().equals("int")) {
          varType.setSelectedIndex(INT_INDEX);
        } else if (element.getText().equals("array")) {
          varType.setSelectedIndex(ARRAY_INDEX);
          lengthPane.setVisible(true);
        }
      } else if (element.getName().equals("array_length")) {
        int nrValues = Integer.parseInt(element.getText());
        setNumberOfValues(nrValues);
        varLength.setValue(nrValues);
      }
    }

    return true;
  }

}
