/*
 * Copyright (c) 2014, TU Braunschweig.
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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyListener;
import java.text.NumberFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultFormatterFactory;
import javax.swing.text.DocumentFilter;
import javax.swing.text.JTextComponent;
import javax.swing.text.PlainDocument;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MotePlugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.mote.memory.UnknownVariableException;
import org.contikios.cooja.mote.memory.VarMemory;
import org.jdom.Element;

/**
 * Variable Watcher enables a user to watch mote variables during a simulation.
 * Variables can be read or written either as bytes, integers or byte arrays.
 *
 * User can also see which variables seems to be available on the selected node.
 *
 * @author Fredrik Osterlind
 * @author Enrico Jorns
 */
@ClassDescription("Variable Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
public class VariableWatcher extends VisPlugin implements MotePlugin {
  private static final long serialVersionUID = 1L;

  private VarMemory moteMemory;

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private final static int BYTE_INDEX = 0;
  private final static int INT_INDEX = 1;
  private final static int ARRAY_INDEX = 2;
  private final static int CHAR_ARRAY_INDEX = 3;

  private JPanel lengthPane;
  private JPanel valuePane;
  private JComboBox varNameCombo;
  private JComboBox varTypeCombo;
  private JComboBox varFormatCombo;
  private JFormattedTextField[] varValues;
  private byte[] bufferedBytes;
  private JFormattedTextField varLength;
  private JButton writeButton;
  private JLabel debuglbl;
  private FocusListener charValueFocusListener;

  private NumberFormat integerFormat;
  private ValueFormatter hf;

  private Mote mote;

  /** 
   * Display types for variables.
   */
  public enum VarTypes {

    BYTE("byte", 1),
    SHORT("short", 2),
    INT("int", 2),
    LONG("long", 4),
    ADDR("address", 4);

    String mRep;
    int mSize;

    VarTypes(String rep, int size) {
      mRep = rep;
      mSize = size;
    }

    /**
     * Returns the number of bytes for this data type.
     *
     * @return Size in bytes
     */
    public int getBytes() {
      return mSize;
    }

    protected void setBytes(int size) {
      mSize = size;
    }

    /**
     * Returns String name of this variable type.
     *
     * @return Type name
     */
    @Override
    public String toString() {
      return mRep;
    }
  }

  /** 
   * Display formats for variables.
   */
  public enum VarFormats {

    CHAR("Char", 0),
    DEC("Decimal", 10),
    HEX("Hex", 16);

    String mRep;
    int mBase;

    VarFormats(String rep, int base) {
      mRep = rep;
      mBase = base;
    }

    /**
     * Returns String name of this variable representation.
     *
     * @return Type name
     */
    @Override
    public String toString() {
      return mRep;
    }
  }

  VarFormats[] valueFormats = {VarFormats.CHAR, VarFormats.DEC, VarFormats.HEX};
  VarTypes[] valueTypes = {VarTypes.BYTE, VarTypes.SHORT, VarTypes.INT, VarTypes.LONG, VarTypes.ADDR};

  /**
   * @param moteToView Mote
   * @param simulation Simulation
   * @param gui GUI
   */
  public VariableWatcher(Mote moteToView, Simulation simulation, Cooja gui) {
    super("Variable Watcher (" + moteToView + ")", gui);
    this.mote = moteToView;
    moteMemory = new VarMemory(moteToView.getMemory());

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

    varNameCombo = new JComboBox();
    varNameCombo.setEditable(true);
    varNameCombo.setSelectedItem("[enter or pick name]");

    List<String> allPotentialVarNames = new ArrayList<>(moteMemory.getVariableNames());
    Collections.sort(allPotentialVarNames);
    for (String aVarName: allPotentialVarNames) {
      varNameCombo.addItem(aVarName);
    }

    /* Reset variable read feedbacks if variable name was changed */
    final JTextComponent tc = (JTextComponent) varNameCombo.getEditor().getEditorComponent();
    tc.getDocument().addDocumentListener(new DocumentListener() {

      @Override
      public void insertUpdate(DocumentEvent e) {
        writeButton.setEnabled(false);
        ((JTextField) varNameCombo.getEditor().getEditorComponent()).setForeground(UIManager.getColor("TextField.foreground"));
      }

      @Override
      public void removeUpdate(DocumentEvent e) {
        writeButton.setEnabled(false);
        ((JTextField) varNameCombo.getEditor().getEditorComponent()).setForeground(UIManager.getColor("TextField.foreground"));
      }

      @Override
      public void changedUpdate(DocumentEvent e) {
        writeButton.setEnabled(false);
        ((JTextField) varNameCombo.getEditor().getEditorComponent()).setForeground(UIManager.getColor("TextField.foreground"));
      }
    });

    varNameCombo.addItemListener(new ItemListener() {

      @Override
      public void itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {
          try {
            /* invalidate byte field */
            bufferedBytes = null;
            /* calculate number of elements required to show the value in the given size */
            updateNumberOfValues();
          }
          catch (UnknownVariableException ex) {
            ((JTextField) varNameCombo.getEditor().getEditorComponent()).setForeground(Color.RED);
            writeButton.setEnabled(false);
          }
        }
      }
    });

    smallPane.add(BorderLayout.EAST, varNameCombo);
    mainPane.add(smallPane);

    // Variable type
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Variable type");
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    smallPane.add(BorderLayout.WEST, label);

    /* set correct integer and address size */
    valueTypes[2].setBytes(moteToView.getMemory().getLayout().intSize);
    valueTypes[4].setBytes(moteToView.getMemory().getLayout().addrSize);

    JPanel reprPanel = new JPanel(new BorderLayout());
    varTypeCombo = new JComboBox(valueTypes);
    varTypeCombo.addItemListener(new ItemListener() {

      @Override
      public void itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {
          hf.setType((VarTypes) e.getItem());
          updateNumberOfValues(); // number of elements should have changed
        }
      }
    });

    varFormatCombo = new JComboBox(valueFormats);
    varFormatCombo.setSelectedItem(VarFormats.HEX);
    varFormatCombo.addItemListener(new ItemListener() {

      @Override
      public void itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {

          hf.setFormat((VarFormats) e.getItem());
          refreshValues(); // format of elements should have changed
        }
      }
    });

    reprPanel.add(BorderLayout.WEST, varTypeCombo);
    reprPanel.add(BorderLayout.EAST, varFormatCombo);

    smallPane.add(BorderLayout.EAST, reprPanel);
    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0,5)));

    // Variable value label
    label = new JLabel("Variable value");
    label.setAlignmentX(JLabel.CENTER_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH,LABEL_HEIGHT));
    mainPane.add(label);

    // Variable value(s)
    valuePane = new JPanel();
    valuePane.setLayout(new BoxLayout(valuePane, BoxLayout.X_AXIS));

    hf = new ValueFormatter(
            (VarTypes) varTypeCombo.getSelectedItem(),
            (VarFormats) varFormatCombo.getSelectedItem());

    varValues = new JFormattedTextField[1];
    varValues[0] = new JFormattedTextField(integerFormat);
    varValues[0].setValue(new Integer(0));
    varValues[0].setColumns(3);
    varValues[0].setText("?");

    charValueFocusListener = new FocusListener() {
      @Override
      public void focusGained(FocusEvent arg0) {
        JTextField jtxt = (JTextField)arg0.getComponent();
        jtxt.selectAll();
      }

      @Override
      public void focusLost(FocusEvent arg0) {

      }
    };

    mainPane.add(valuePane);
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));

    debuglbl = new JLabel();
    mainPane.add(new JPanel().add(debuglbl));
    mainPane.add(Box.createRigidArea(new Dimension(0,5)));

    // Read/write buttons
    smallPane = new JPanel(new BorderLayout());
    JButton button = new JButton("Read");
    button.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        bufferedBytes = moteMemory.getByteArray(
                (String) varNameCombo.getSelectedItem(),
                moteMemory.getVariableSize((String) varNameCombo.getSelectedItem()));
        refreshValues();

      }
    });
    smallPane.add(BorderLayout.WEST, button);

    button = new JButton("Write");
    button.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        throw new UnsupportedOperationException("Writing is not supported yet.");
      }
    });
    smallPane.add(BorderLayout.EAST, button);
    button.setEnabled(false);
    writeButton = button;
    mainPane.add(smallPane);

    add(BorderLayout.NORTH, mainPane);
    pack();
  }

  /**
   * String to Value to String conversion for JFormattedTextField
   * based on selected VarTypes and VarFormats.
   */
  public class ValueFormatter extends JFormattedTextField.AbstractFormatter {

    final String TEXT_NOT_TO_TOUCH;

    private VarTypes mType;
    private VarFormats mFormat;

    public ValueFormatter(VarTypes type, VarFormats format) {
      mType = type;
      mFormat = format;
      if (mFormat == VarFormats.HEX) {
        TEXT_NOT_TO_TOUCH = "0x";
      }
      else {
        TEXT_NOT_TO_TOUCH = "";
      }
    }

    public void setType(VarTypes type) {
      mType = type;
    }

    public void setFormat(VarFormats format) {
      mFormat = format;
    }

    @Override
    public Object stringToValue(String text) throws ParseException {
      Object ret;
      switch (mFormat) {
        case CHAR:
          ret = text.charAt(0);
          break;
        case DEC:
        case HEX:
          try {
            ret = Integer.decode(text);
          }
          catch (NumberFormatException ex) {
            ret = 0;
          }
          break;
        default:
          ret = null;
      }
      return ret;
    }

    @Override
    public String valueToString(Object value) throws ParseException {
      if (value == null) {
        return "?";
      }

      switch (mFormat) {
        case CHAR:
          return String.format("%c", value);
        case DEC:
          return String.format("%d", value);
        case HEX:
          return String.format("0x%x", value);
        default:
          return "";
      }
    }

    /* Do not override TEXT_NOT_TO_TOUCH */
    @Override
    public DocumentFilter getDocumentFilter() {
      return new DocumentFilter() {

        @Override
        public void insertString(DocumentFilter.FilterBypass fb, int offset, String string, AttributeSet attr) throws BadLocationException {
          if (offset < TEXT_NOT_TO_TOUCH.length()) {
            return;
          }
          super.insertString(fb, offset, string, attr);
        }

        @Override
        public void replace(DocumentFilter.FilterBypass fb, int offset, int length, String text, AttributeSet attrs) throws BadLocationException {
          if (offset < TEXT_NOT_TO_TOUCH.length()) {
            length = Math.max(0, length - TEXT_NOT_TO_TOUCH.length());
            offset = TEXT_NOT_TO_TOUCH.length();
          }
          super.replace(fb, offset, length, text, attrs);
        }

        @Override
        public void remove(DocumentFilter.FilterBypass fb, int offset, int length) throws BadLocationException {
          if (offset < TEXT_NOT_TO_TOUCH.length()) {
            length = Math.max(0, length + offset - TEXT_NOT_TO_TOUCH.length());
            offset = TEXT_NOT_TO_TOUCH.length();
          }
          if (length > 0) {
            super.remove(fb, offset, length);
          }
        }
      };
    }
  }


  /**
   * Updates all value fields based on buffered data.
   */
  private void refreshValues() {
    int bytes = moteMemory.getVariableSize((String) varNameCombo.getSelectedItem());
    int typeSize = ((VarTypes) varTypeCombo.getSelectedItem()).getBytes();
    int elements = (int) Math.ceil((double) bytes / typeSize);

    /* Skip if we have no data to set */
    if ((bufferedBytes == null) || (bufferedBytes.length < bytes)) {
      return;
    }

    /* Set values based on buffered data */
    for (int i = 0; i < elements; i += 1) {
      int val = 0;
      for (int j = 0; j < typeSize; j++) {
        val += ((bufferedBytes[i * typeSize + j] & 0xFF) << (j * 8));
      }
      varValues[i].setValue(val);
      try {
        varValues[i].commitEdit();
      }
      catch (ParseException ex) {
        Logger.getLogger(VariableWatcher.class.getName()).log(Level.SEVERE, null, ex);
      }
    }

  }

  /**
   * Updates number of value fields.
   */
  private void updateNumberOfValues() {
    valuePane.removeAll();

    DefaultFormatterFactory defac = new DefaultFormatterFactory(hf);
    long address = moteMemory.getVariableAddress((String) varNameCombo.getSelectedItem());
    int bytes = moteMemory.getVariableSize((String) varNameCombo.getSelectedItem());
    int typeSize = ((VarTypes) varTypeCombo.getSelectedItem()).getBytes();
    int elements = (int) Math.ceil((double) bytes / typeSize);

    if (elements > 0) {
      varValues = new JFormattedTextField[elements];
      for (int i = 0; i < elements; i++) {
        varValues[i] = new JFormattedTextField(defac);
        valuePane.add(varValues[i]);
      }
    }

    refreshValues();

    pack();
  }

  @Override
  public void closePlugin() {
  }

  @Override
  public Collection<Element> getConfigXML() {
    // Return currently watched variable and type
    Vector<Element> config = new Vector<>();

    Element element;

    // Selected variable name
    element = new Element("varname");
    element.setText((String) varNameCombo.getSelectedItem());
    config.add(element);

    // Selected variable type
    element = new Element("vartype");
    element.setText(String.valueOf(varTypeCombo.getSelectedIndex()));
    config.add(element);
    
    // Selected output format
    element = new Element("varformat");
    element.setText(String.valueOf(varDispCombo.getSelectedIndex()));
    config.add(element);
    return config;
  }

  @Override
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      switch (element.getName()) {
        case "varname":
          varNameCombo.setSelectedItem(element.getText());
          break;
        case "vartype":
          varTypeCombo.setSelectedIndex(Integer.parseInt(element.getText()));
          break;
        case "varformat":
          varDispCombo.setSelectedIndex(Integer.parseInt(element.getText()));
          break;
      }
    }
    updateNumberOfValues();

    return true;
  }

  @Override
  public Mote getMote() {
    return mote;
  }
}

/* Limit JTextField input class */
class JTextFieldLimit extends PlainDocument {

  private static final long serialVersionUID = 1L;
  private int limit;
  // optional uppercase conversion
  private boolean toUppercase = false;

  JTextFieldLimit(int limit) {
    super();
    this.limit = limit;
  }

  JTextFieldLimit(int limit, boolean upper) {
    super();
    this.limit = limit;
    toUppercase = upper;
  }

  @Override
  public void insertString(int offset, String  str, AttributeSet attr)
  throws BadLocationException {
    if (str == null) {
      return;
    }

    if ((getLength() + str.length()) <= limit) {
      if (toUppercase) {
        str = str.toUpperCase();
      }
      super.insertString(offset, str, attr);
    }
  }
}
