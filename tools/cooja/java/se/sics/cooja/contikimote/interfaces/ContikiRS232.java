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
 * $Id: ContikiRS232.java,v 1.9 2009/05/26 14:24:20 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.Vector;

import org.apache.log4j.Logger;
import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMote;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.dialogs.SerialUI;
import se.sics.cooja.interfaces.PolledAfterActiveTicks;

/**
 * Contiki mote serial port and log interfaces.
 * Not fully implemented yet: does not support writeArray and writeByte
 *
 * Contiki variables:
 * <ul>
 * <li>char simSerialReceivingFlag (1=mote has incoming serial data)
 * <li>int simSerialReceivingLength
 * <li>byte[] simSerialReceivingData
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>rs232_interface
 * </ul>
 * <p>
 *
 * This observable notifies observers when a serial message is sent from the mote.
 *
 * @see #getSerialMessages()
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial port")
public class ContikiRS232 extends SerialUI implements ContikiMoteInterface, PolledAfterActiveTicks {
  private static Logger logger = Logger.getLogger(ContikiRS232.class);

  private ContikiMote mote = null;
  private SectionMoteMemory moteMem = null;

  /**
   * Approximate energy consumption of every sent character over RS232 (mQ).
   */
  public final double ENERGY_CONSUMPTION_PER_CHAR_mQ;

  private double myEnergyConsumption = 0.0;

  /**
   * Creates an interface to the RS232 at mote.
   *
   * @param mote
   *          RS232's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiRS232(Mote mote) {
    ENERGY_CONSUMPTION_PER_CHAR_mQ =
      mote.getType().getConfig().getDoubleValue(ContikiRS232.class, "CONSUMPTION_PER_CHAR_mQ");

    this.mote = (ContikiMote) mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"rs232_interface", "simlog_interface" };
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simLoggedFlag") == 1) {
      int len = moteMem.getIntValueOf("simLoggedLength");
      byte[] bytes = moteMem.getByteArray("simLoggedData", len);

      myEnergyConsumption = ENERGY_CONSUMPTION_PER_CHAR_mQ * len;

      moteMem.setByteValueOf("simLoggedFlag", (byte) 0);
      moteMem.setIntValueOf("simLoggedLength", 0);

      for (byte b: bytes) {
        dataReceived(b);
      }
    } else {
      myEnergyConsumption = 0.0;
    }
  }

  public void writeString(String message) {
    final byte[] dataToAppend = message.getBytes();

    TimeEvent writeStringEvent = new MoteTimeEvent(mote, 0) {
      public void execute(long t) {
        /* Append to existing buffer */
        int oldSize = moteMem.getIntValueOf("simSerialReceivingLength");
        int newSize = oldSize + dataToAppend.length;
        moteMem.setIntValueOf("simSerialReceivingLength", newSize);

        byte[] oldData = moteMem.getByteArray("simSerialReceivingData", oldSize);
        byte[] newData = new byte[newSize];

        System.arraycopy(oldData, 0, newData, 0, oldData.length);
        System.arraycopy(dataToAppend, 0, newData, oldSize, dataToAppend.length);

        moteMem.setByteArray("simSerialReceivingData", newData);

        moteMem.setByteValueOf("simSerialReceivingFlag", (byte) 1);
        mote.scheduleImmediateWakeup();
      }
    };
    mote.getSimulation().scheduleEvent(
        writeStringEvent,
        mote.getSimulation().getSimulationTime()
    );
  }

  public double energyConsumption() {
    return myEnergyConsumption;
  }

  public Mote getMote() {
    return mote;
  }

  private TimeEvent pendingBytesEvent = null;
  private Vector<Byte> pendingBytes = new Vector<Byte>();
  public void writeArray(byte[] s) {
    for (byte b: s) {
      pendingBytes.add(b);
    }

    mote.scheduleImmediateWakeup();
    if (pendingBytesEvent != null) {
      /* Event is already scheduled, no need to reschedule */
      return;
    }

    pendingBytesEvent = new MoteTimeEvent(mote, 0) {
      public void execute(long t) {
        ContikiRS232.this.pendingBytesEvent = null;
        if (pendingBytes.isEmpty()) {
          return;
        }

        /* Move bytes from synchronized vector to Contiki buffer */
        int nrBytes = pendingBytes.size();
        byte[] dataToAppend = new byte[nrBytes];
        for (int i=0; i < nrBytes; i++) {
          dataToAppend[i] = pendingBytes.firstElement();
          pendingBytes.remove(0);
        }

        /* Append to existing buffer */
        int oldSize = moteMem.getIntValueOf("simSerialReceivingLength");
        int newSize = oldSize + dataToAppend.length;
        moteMem.setIntValueOf("simSerialReceivingLength", newSize);

        byte[] oldData = moteMem.getByteArray("simSerialReceivingData", oldSize);
        byte[] newData = new byte[newSize];

        System.arraycopy(oldData, 0, newData, 0, oldData.length);
        System.arraycopy(dataToAppend, 0, newData, oldSize, dataToAppend.length);

        moteMem.setByteArray("simSerialReceivingData", newData);

        moteMem.setByteValueOf("simSerialReceivingFlag", (byte) 1);

        /* Reschedule us if more bytes are available */
        mote.getSimulation().scheduleEvent(this, t);
      }
    };
    mote.getSimulation().scheduleEvent(
        pendingBytesEvent,
        mote.getSimulation().getSimulationTime()
    );
  }

  public void writeByte(final byte b) {
    pendingBytes.add(b);

    mote.scheduleImmediateWakeup();
    if (pendingBytesEvent != null) {
      /* Event is already scheduled, no need to reschedule */
      return;
    }

    pendingBytesEvent = new MoteTimeEvent(mote, 0) {
      public void execute(long t) {
        ContikiRS232.this.pendingBytesEvent = null;
        if (pendingBytes.isEmpty()) {
          return;
        }

        /* Move bytes from synchronized vector to Contiki buffer */
        int nrBytes = pendingBytes.size();
        byte[] dataToAppend = new byte[nrBytes];
        for (int i=0; i < nrBytes; i++) {
          dataToAppend[i] = pendingBytes.firstElement();
          pendingBytes.remove(0);
        }

        /* Append to existing buffer */
        int oldSize = moteMem.getIntValueOf("simSerialReceivingLength");
        int newSize = oldSize + dataToAppend.length;
        moteMem.setIntValueOf("simSerialReceivingLength", newSize);

        byte[] oldData = moteMem.getByteArray("simSerialReceivingData", oldSize);
        byte[] newData = new byte[newSize];

        System.arraycopy(oldData, 0, newData, 0, oldData.length);
        System.arraycopy(dataToAppend, 0, newData, oldSize, dataToAppend.length);

        moteMem.setByteArray("simSerialReceivingData", newData);

        moteMem.setByteValueOf("simSerialReceivingFlag", (byte) 1);

        /* Reschedule us if more bytes are available */
        mote.getSimulation().scheduleEvent(this, t);
      }
    };
    mote.getSimulation().scheduleEvent(
        pendingBytesEvent,
        mote.getSimulation().getSimulationTime()
    );
  }

}
