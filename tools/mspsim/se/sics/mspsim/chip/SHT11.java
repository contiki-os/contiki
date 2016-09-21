/**
 * Copyright (c) 2007, 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id: $
 *
 * -----------------------------------------------------------------
 *
 * SHT11 
 *
 * Author  : Joakim Eriksson, joakime@sics.se
 * Created : Sept 16 2008
 * Updated : $Date:  $
 *           $Revision: $
 */

package se.sics.mspsim.chip;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;
import se.sics.mspsim.util.Utils;

public class SHT11 extends Chip {

  private static final int IDLE = 0;
  private static final int COMMAND = 1;
  private static final int ACK_CMD = 2;
  private static final int MEASURE = 3;
  private static final int WRITE_BYTE = 4;
  private static final int ACK_WRITE = 5;

  private final int CMD_MEASURE_TEMP = 0x03;
  private final int CMD_MEASURE_HUM = 0x05;

  private final static char[] INIT_COMMAND = "CdcCDc".toCharArray();
  private int initPos = 0;
  
  
  /* Serial data pins */
  IOPort sclkPort;
  int sclkPin;
  IOPort sdataPort;
  int sdataPin;
  
  int state = IDLE;
  
  boolean clockHi = false;
  boolean dataHi = false;
  private int readData = 0;
  private int bitCnt = 0;
  private int temp = 3960 + 2400;
  private int humid = 0x1040;
  private int output[] = new int[3];
  private int writePos = 0;
  private int writeLen = 0;
  private int writeData = 0;
  
  private static int rev8bits(int v) {
    int r = 0;
    int s = 8;

    while(v > 0) {
      r = (r << 1) | v & 1;
      v = (v >> 1) & 0xff;
      s--;
    }
    r <<= s;                  /* Shift when v's highest bits are zero */
    return r & 0xff;
  }
  
  private int crc8Add(int acc, int data) {
    int i;
    acc ^= (data & 0xff);
    for(i = 0; i < 8; i++) {
      if((acc & 0x80) != 0) {
        acc = ((acc << 1) ^ 0x31) & 0xff;
      } else {
        acc <<= 1;
      }
    }
    return acc & 0xff;
  }
  
  
  private TimeEvent measureEvent = new TimeEvent(0) {
    public void execute(long t) {
      if (readData == CMD_MEASURE_TEMP) {
        output[0] = (temp >> 8) & 0xff;
        output[1] = temp & 0xff;
      } else if (readData == CMD_MEASURE_HUM) {
        output[0] = (humid >> 8) & 0xff;
        output[1] = humid & 0xff;
      } else {
        /* Something bad has happened */
        return;
      }
      
      int crc = 0;
      crc = crc8Add(crc, readData);
      crc = crc8Add(crc, output[0]);
      crc = crc8Add(crc, output[1]);
      if (DEBUG) log("CRC: " + Utils.hex8(crc) +
          " rcrc: " + Utils.hex8(rev8bits(crc)));
      output[2] = rev8bits(crc);
      
      /* finished measuring - signal with LOW! */
      sdataPort.setPinState(sdataPin, IOPort.PinState.LOW);
      state = WRITE_BYTE;
      writeData = output[0];
      writePos = 0;
      writeLen = 3;
    }};

    
  public void setTemperature(int temp) {
     this.temp = temp;
  }

  public int getTemperature() {
     return this.temp;
  }
  
  public void setHumidity(int humidity) {
      this.humid = humidity;
  }
  
  public int getHumidity() {
     return this.humid;
  }

  public SHT11(MSP430Core cpu) {
      super("SHT11", "Digital Humidity Sensor", cpu);
  }
    
  public void setDataPort(IOPort port, int bit) {
    sdataPort = port;
    sdataPin = bit;
  }
  
  public void reset(int type) {    
    clockHi = true;
    dataHi = true;
    initPos = 0;
    bitCnt = 0;
    readData = 0;
    writePos = 0;
    writeData = 0;
    state = IDLE;
    // Always set pin to high when not doing anything...
    sdataPort.setPinState(sdataPin, IOPort.PinState.HI);
  }
  
  public void clockPin(boolean high) {
    if (clockHi == high) return;

    char c = high ? 'C' : 'c';
    if (DEBUG) log("clock pin " + c);
    switch (state) {
    case IDLE:
      if (checkInit(c)) {
        state = COMMAND;
      }
      break;
    case COMMAND:
      if (c == 'c') {
        readData = (readData << 1) | (dataHi ? 1 : 0);
        bitCnt++;
        if (bitCnt == 8) {
          if (DEBUG) log("read: " + Utils.hex8(readData));
          bitCnt = 0;
          state = ACK_CMD;
          sdataPort.setPinState(sdataPin, IOPort.PinState.LOW);
        }
      }
      break;
    case ACK_CMD:
      if (c == 'c') {
        sdataPort.setPinState(sdataPin, IOPort.PinState.HI);
        if (readData == CMD_MEASURE_HUM || readData == CMD_MEASURE_TEMP) {
          state = MEASURE;
          /* schedule measurement for 20 millis */
          cpu.scheduleTimeEventMillis(measureEvent, 20);
        }
      }
      break;
    case MEASURE:
      break;
    case WRITE_BYTE:
      if (c == 'C') {
        boolean hi = (writeData & 0x80) != 0;
        sdataPort.setPinState(sdataPin, hi ? IOPort.PinState.HI : IOPort.PinState.LOW);
        bitCnt++;
        writeData = writeData << 1;
        if (bitCnt == 8) {
          // All bits are written!
          state = ACK_WRITE;
          if (DEBUG) log("Wrote byte: " + output[writePos]);
          writePos++;
        }
      }
      break;
    case ACK_WRITE:
      if (c == 'C' && dataHi) {
        if (DEBUG) log("*** NO ACK???");
        reset(0);
      }
      break;
    }
    clockHi = high;
  }
  
  public void dataPin(boolean high) {
    if (dataHi == high) return;
    char c = high ? 'D' : 'd';
    if (DEBUG) log("data pin  " + c);
    switch (state) {
    case IDLE:
      if (checkInit(c)) {
        state = COMMAND;
      }
      break;
    case ACK_WRITE:
      if (c == 'D') { // if D goes back high - then we are done here!!!
        if (DEBUG) log("ACK for byte complete...");
        if (writePos < writeLen) {
          state = WRITE_BYTE;
          writeData = output[writePos];
          bitCnt = 0;
        } else {
          reset(0);
        }
      }
      break;
    }
    dataHi = high;
  }

  private boolean checkInit(char c) {
    if (INIT_COMMAND[initPos] == c) {
      initPos++;
      if (initPos == INIT_COMMAND.length) {
        initPos = 0;
        if (DEBUG) {
          log("COMMAND signature detected!!!");
        }
        return true;
      }
    } else {
      initPos = 0;
      // If this is a correct first char => ok!
      if (c == INIT_COMMAND[0]) {
        initPos = 1;
      }
    }
    return false;
  }
  
  
  public int getModeMax() {
    return 0;
  }

  /* no configuration for the SHT11 ? */
  public int getConfiguration(int parameter) {
      return 0;
  }

}
