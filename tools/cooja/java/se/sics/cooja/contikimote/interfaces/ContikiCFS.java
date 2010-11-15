/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: ContikiCFS.java,v 1.12 2010/11/15 21:44:38 adamdunkels Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.PolledAfterActiveTicks;

/**
 * Contiki FileSystem (CFS) interface (such as external flash).
 *
 * Contiki variables:
 * <ul>
 * <li>char[] simCFSData
 * <li>char simCFSChanged (1=filesystem has been altered)
 * <li>int simCFSRead (bytes read from filesystem)
 * <li>int simCFSWritten (bytes written to filesystem)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>cfs_interface
 * </ul>
 * <p>
 * This observable notifies when the filesystem is used (read/write).
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Filesystem (CFS)")
public class ContikiCFS extends MoteInterface implements ContikiMoteInterface, PolledAfterActiveTicks {
  private static Logger logger = Logger.getLogger(ContikiCFS.class);

  public int FILESYSTEM_SIZE = 4000; /* Configure CFS size here and in cfs-cooja.c */
  private Mote mote = null;
  private SectionMoteMemory moteMem = null;

  private int lastRead = 0;
  private int lastWritten = 0;

  /**
   * Creates an interface to the filesystem at mote.
   *
   * @param mote Mote
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiCFS(Mote mote) {
    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"cfs_interface"};
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simCFSChanged") == 1) {
      lastRead = moteMem.getIntValueOf("simCFSRead");
      lastWritten = moteMem.getIntValueOf("simCFSWritten");

      moteMem.setIntValueOf("simCFSRead", 0);
      moteMem.setIntValueOf("simCFSWritten", 0);
      moteMem.setByteValueOf("simCFSChanged", (byte) 0);

      this.setChanged();
      this.notifyObservers(mote);
    }
  }

  /**
   * Set filesystem data.
   *
   * @param data Data
   * @return True if operation successful
   */
  public boolean setFilesystemData(byte[] data) {
    if (data.length > FILESYSTEM_SIZE) {
      logger.fatal("Error. Filesystem data too large, skipping");
      return false;
    }

    moteMem.setByteArray("simCFSData", data);
    return true;
  }

  /**
   * Get filesystem data.
   *
   * @return Filesystem data
   */
  public byte[] getFilesystemData() {
    return moteMem.getByteArray("simCFSData", FILESYSTEM_SIZE);
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

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel lastTimeLabel = new JLabel("Last change at: ?");
    final JLabel lastReadLabel = new JLabel("Last change read bytes: 0");
    final JLabel lastWrittenLabel = new JLabel("Last change wrote bytes: 0");
    final JButton uploadButton = new JButton("Upload binary file");
    panel.add(lastTimeLabel);
    panel.add(lastReadLabel);
    panel.add(lastWrittenLabel);
    panel.add(uploadButton);

    uploadButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        byte[] fileData = readDialogFileBytes(null);

        // Write file data to CFS
        if (fileData != null) {
          if (setFilesystemData(fileData)) {
            logger.info("Done! (" + fileData.length + " bytes written to CFS)");
          }
        }
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        long currentTime = mote.getSimulation().getSimulationTime();
        lastTimeLabel.setText("Last change at time: " + currentTime);
        lastReadLabel.setText("Last change read bytes: " + getLastReadCount());
        lastWrittenLabel.setText("Last change wrote bytes: " + getLastWrittenCount());
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    panel.setMinimumSize(new Dimension(140, 60));
    panel.setPreferredSize(new Dimension(140, 60));

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
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  /**
   * Opens a file dialog and returns the contents of the selected file or null if dialog aborted.
   *
   * @param parent Dialog parent, may be null
   * @return Binary contents of user selected file
   */
  public static byte[] readDialogFileBytes(Component parent) {
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
