/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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

package org.contikios.cooja.contikimote.interfaces;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.xml.bind.DatatypeConverter;
import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.PolledAfterActiveTicks;
import org.contikios.cooja.mote.memory.VarMemory;

/**
 * Contiki EEPROM interface
 *
 * Contiki variables:
 * <ul>
 * <li>char[] simEEPROMData
 * <li>char simEEPROMChanged (1=EEPROM has been altered)
 * <li>int simEEPROMRead (bytes read from EEPROM)
 * <li>int simEEPROMWritten (bytes written to EEPROM)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>eeprom_interface
 * </ul>
 * <p>
 * This observable notifies when the eeprom is used (read/write).
 *
 * @author Claes Jakobsson (based on ContikiCFS by Fredrik Osterlind)
 */
@ClassDescription("EEPROM")
public class ContikiEEPROM extends MoteInterface implements ContikiMoteInterface, PolledAfterActiveTicks {
  private static Logger logger = Logger.getLogger(ContikiEEPROM.class);

  public int EEPROM_SIZE = 1024; /* Configure EEPROM size here and in eeprom.c. Should really be multiple of 16 */
  private Mote mote = null;
  private VarMemory moteMem = null;

  private int lastRead = 0;
  private int lastWritten = 0;

  /**
   * Creates an interface to the EEPROM at mote.
   *
   * @param mote Mote
   * @see Mote
   * @see org.contikios.cooja.MoteInterfaceHandler
   */
  public ContikiEEPROM(Mote mote) {
    this.mote = mote;
    this.moteMem = new VarMemory(mote.getMemory());
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"eeprom_interface"};
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simEEPROMChanged") == 1) {
      lastRead = moteMem.getIntValueOf("simEEPROMRead");
      lastWritten = moteMem.getIntValueOf("simEEPROMWritten");

      moteMem.setIntValueOf("simEEPROMRead", 0);
      moteMem.setIntValueOf("simEEPROMWritten", 0);
      moteMem.setByteValueOf("simEEPROMChanged", (byte) 0);

      this.setChanged();
      this.notifyObservers(mote);
    }
  }

  /**
   * Set EEPROM data.
   *
   * @param data Data
   * @return True if operation successful
   */
  public boolean setEEPROMData(byte[] data) {
    if (data.length > EEPROM_SIZE) {
      logger.fatal("Error. EEPROM data too large, skipping");
      return false;
    }

    moteMem.setByteArray("simEEPROMData", data);
    return true;
  }

  /**
   * Get EEPROM data.
   *
   * @return Filesystem data
   */
  public byte[] getEEPROMData() {
    return moteMem.getByteArray("simEEPROMData", EEPROM_SIZE);
  }

  /**
   * @return Read bytes count last change.
   */
  public int getLastReadCount() {
    return lastRead;
  }

  /**
   * @return Written bytes count last change.
   */
  public int getLastWrittenCount() {
    return lastWritten;
  }

  String byteArrayToPrintableCharacters(byte[] data, int offset, int length) {
      StringBuilder sb = new StringBuilder();
      for (int i = offset; i < offset + length; i++) {
        sb.append(data[i] > 31 && data[i] < 128 ? (char) data[i] : '.');
      }
      return sb.toString();
  }
  
  String byteArrayToHexList(byte[] data, int offset, int length) {
      StringBuilder sb = new StringBuilder();
            
      for (int i = 0; i < length; i++) {
          byte h = (byte) ((int) data[offset + i] >> 4);
          byte l = (byte) ((int) data[offset + i] & 0xf);
          sb.append((char)(h < 10 ? 0x30 + h : 0x61 + h - 10));
          sb.append((char)(l < 10 ? 0x30 + l : 0x61 + l - 10));
          sb.append(' ');
          if (i % 8 == 7 && i != length - 1) {
              sb.append(' ');
          }
      }
    
      return sb.toString();
  }
  
  void redrawDataView(JTextArea textArea) {
      StringBuilder sb = new StringBuilder();
      Formatter fmt = new Formatter(sb);
      byte[] data = getEEPROMData();
      
      for (int i = 0; i < EEPROM_SIZE; i+= 16) {
          fmt.format("%04d  %s | %s |\n", i, byteArrayToHexList(data, i, 16), byteArrayToPrintableCharacters(data, i, 16));
      }
      
      textArea.setText(sb.toString());
      textArea.setCaretPosition(0);
  }
  
  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel lastTimeLabel = new JLabel("Last change at: ?");
    final JLabel lastReadLabel = new JLabel("Last change read bytes: 0");
    final JLabel lastWrittenLabel = new JLabel("Last change wrote bytes: 0");
    final JButton uploadButton = new JButton("Upload binary file");
    final JButton clearButton = new JButton("Reset EEPROM to zero");
    final JTextArea dataViewArea = new JTextArea();
    final JScrollPane dataViewScrollPane = new JScrollPane(dataViewArea);
    
    panel.add(lastTimeLabel);
    panel.add(lastReadLabel);
    panel.add(lastWrittenLabel);
    panel.add(uploadButton);
    panel.add(clearButton);
    
    panel.add(dataViewScrollPane);
    
    uploadButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        byte[] eepromData = readDialogEEPROMBytes(null);

        // Write file data to EEPROM
        if (eepromData != null) {
          if (setEEPROMData(eepromData)) {
            logger.info("Done! (" + eepromData.length + " bytes written to EEPROM)");
          }
          
          redrawDataView(dataViewArea);
        }
      }
    });

    clearButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        byte[] eepromData = new byte[EEPROM_SIZE];        

        if (setEEPROMData(eepromData)) {
            logger.info("Done! (EEPROM reset to zero)");
        }
          
        redrawDataView(dataViewArea);
      }
    });
    
    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        long currentTime = mote.getSimulation().getSimulationTime();
        lastTimeLabel.setText("Last change at time: " + currentTime);
        lastReadLabel.setText("Last change read bytes: " + getLastReadCount());
        lastWrittenLabel.setText("Last change wrote bytes: " + getLastWrittenCount());
        
        redrawDataView(dataViewArea);        
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    panel.setMinimumSize(new Dimension(140, 60));
    panel.setPreferredSize(new Dimension(140, 60));

    dataViewArea.setLineWrap(false);
    dataViewArea.setEditable(false);
    dataViewArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
    dataViewScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    dataViewScrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    
    redrawDataView(dataViewArea);
    
    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Collection<Element> getConfigXML() {
      Vector<Element> config = new Vector<Element>();
      Element element;

      // Infinite boolean
      element = new Element("eeprom");
      element.setText(DatatypeConverter.printBase64Binary(getEEPROMData()));
      config.add(element);

      return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
      for (Element element : configXML) {
        if (element.getName().equals("eeprom")) {
          setEEPROMData(DatatypeConverter.parseBase64Binary(element.getText()));
        }
      }
  }

  /**
   * Opens a file dialog and returns the contents of the selected file or null if dialog aborted.
   *
   * @param parent Dialog parent, may be null
   * @return Binary contents of user selected file
   */
  public static byte[] readDialogEEPROMBytes(Component parent) {
    // Choose file
    File file = null;
    JFileChooser fileChooser = new JFileChooser();
    fileChooser.setCurrentDirectory(new java.io.File("."));
    fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
    fileChooser.setDialogTitle("Select binary data");

    if (fileChooser.showOpenDialog(parent) == JFileChooser.APPROVE_OPTION) {
      file = fileChooser.getSelectedFile();
    } else {
      return null;
    }

    // Read file data
    long fileSize = file.length();
    byte[] fileData = new byte[(int) fileSize];

    FileInputStream fileIn;
    DataInputStream dataIn;
    int offset = 0;
    int numRead = 0;
    try {
      fileIn = new FileInputStream(file);
      dataIn = new DataInputStream(fileIn);
      while (offset < fileData.length
          && (numRead = dataIn.read(fileData, offset, fileData.length - offset)) >= 0) {
        offset += numRead;
      }

      dataIn.close();
      fileIn.close();
    } catch (Exception ex) {
      logger.debug("Exception ex: " + ex);
      return null;
    }

    return fileData;
  }

}
