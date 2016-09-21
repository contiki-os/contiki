/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: M25P80.java 177 2008-03-11 15:32:12Z nifi $
 *
 * -----------------------------------------------------------------
 *
 * ExtFlash
 *
 * Author  : Joakim Eriksson, Fredrik Osterlind
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date: 2008-03-11 16:32:12 +0100 (ti, 11 mar 2008) $
 *           $Revision: 177 $
 */

package se.sics.mspsim.chip;
import java.io.IOException;
import se.sics.mspsim.core.*;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.Utils;

public class M25P80 extends ExternalFlash implements USARTListener, PortListener, Memory {

  public static final int WRITE_STATUS = 0x01;
  public static final int PAGE_PROGRAM = 0x02;
  public static final int READ_DATA = 0x03;
  public static final int WRITE_DISABLE = 0x04;
  public static final int READ_STATUS = 0x05;
  public static final int WRITE_ENABLE = 0x06;
  public static final int READ_DATA_FAST = 0x0b;
  public static final int READ_IDENT = 0x9f;
  public static final int SECTOR_ERASE = 0xd8;
  public static final int BULK_ERASE = 0xc7;
  public static final int DEEP_POWER_DOWN = 0xb9;
  public static final int WAKE_UP = 0xab;

  public static final int STATUS_MASK = 0x9C;

  public static final int MEMORY_SIZE = 1024 * 1024;
  
  public static final int CHIP_SELECT = 0x10;

  private static final double PROGRAM_PAGE_MILLIS = 1.0; // 0.8 - 5 ms
  private static final double SECTOR_ERASE_MILLIS = 800; // 800 - 3 000 ms 

  private int state = 0;
  private boolean chipSelect;

  private int pos;
  private int status = 0;

  private boolean writeEnable = false;
  private boolean writing = false;

  private int[] identity = new int[] {
      0x20,0x20,0x14,0x10,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };
  private int readAddress;
  private int loadedAddress = -1;
  private int blockWriteAddress;
  private byte[] readMemory = new byte[256];
  private byte[] buffer = new byte[256];

  private TimeEvent writeEvent = new TimeEvent(0, "M25P80 Writing") {
    public void execute(long t) {
      writing = false;
    }};

  protected M25P80(String id, MSP430Core cpu) {
      super(id, "External Flash", cpu);
  }

  public M25P80(MSP430Core cpu) {
      super("M25P80", "External Flash", cpu);
  }

  @Override
  public void notifyReset() {
    writing = false;
  }

  public int getStatus() {
    return status | (writeEnable ? 0x02 : 0x00) | (writing ? 0x01 : 0x00);
  }

  public void stateChanged(int state) {
  }

  public void dataReceived(USARTSource source, int data) {
    if (chipSelect) {
      if (DEBUG) {
        log("byte received: " + data);
      }
      switch(state) {
      case READ_STATUS:
          if (DEBUG) {
            log("Read status => " + getStatus() + " from $" + Utils.hex(cpu.getPC(), 4));
          }
          source.byteReceived(getStatus());
          return;
      case READ_IDENT:
        source.byteReceived(identity[pos]);
        pos++;
        if (pos >= identity.length) {
          pos = 0;
        }
        return;
      case WRITE_STATUS:
        status = data & STATUS_MASK;
        source.byteReceived(0);
        return;
      case READ_DATA:
        if (pos < 3) {
          readAddress = (readAddress << 8) + data;
          source.byteReceived(0);
          pos++;
          if (DEBUG && pos == 3) {
            log("reading from $" + Integer.toHexString(readAddress));
          }
        } else {
          source.byteReceived(readMemory(readAddress++));
          if (readAddress > 0xfffff) {
             readAddress = 0;
          }
        }
        return;
      case SECTOR_ERASE:
        if (pos < 3) {
          readAddress = (readAddress << 8) + data;
          source.byteReceived(0);
          pos++;
          if (pos == 3) {
            // Clear buffer
            sectorErase(readAddress);
          }
        }
        return;
      case PAGE_PROGRAM:
        if (pos < 3) {
          readAddress = (readAddress << 8) + data;
          source.byteReceived(0);
          pos++;
          if (pos == 3) {
            // Clear buffer
            for (int i = 0; i < buffer.length; i++) {
              buffer[i] = (byte) 0xff;
            }
            blockWriteAddress = readAddress & 0xfff00;
            if (DEBUG) {
              log("programming at $" + Integer.toHexString(readAddress));
            }
          }
        } else {
          // Do the programming!!!
          source.byteReceived(0);
          writeBuffer((readAddress++) & 0xff, data);
        }
        return;
      }
      if (DEBUG) {
        log("new command: " + data);
      }
      switch (data) {
      case WRITE_ENABLE:
        if (DEBUG) {
          log("Write Enable");
        }
        writeEnable = true;
        break;
      case WRITE_DISABLE:
        if (DEBUG) {
          log("Write Disable");
        }
        writeEnable = false;
        break;
      case READ_IDENT:
        if (DEBUG) {
          log("Read ident.");
        }
        state = READ_IDENT;
        pos = 0;
        source.byteReceived(0);
        return;
      case READ_STATUS:
        state = READ_STATUS;
        source.byteReceived(0);
        return;
      case WRITE_STATUS:
        if (DEBUG) {
          log("Write status");
        }
        state = WRITE_STATUS;
        break;
      case READ_DATA:
        if (DEBUG) {
          log("Read Data");
        }
        state = READ_DATA;
        pos = readAddress = 0;
        break;
      case PAGE_PROGRAM:
        if (DEBUG) {
          log("Page Program");
        }
        state = PAGE_PROGRAM;
        pos = readAddress = 0;
        break;
      case SECTOR_ERASE:
        if (DEBUG) {
          log("Sector Erase");
        }
        state = SECTOR_ERASE;
        pos = 0;
        break;
      case BULK_ERASE:
        log("Bulk Erase");
        break;
      }
      source.byteReceived(0);
    }
  }

  /***********************************************
   * Memory interface methods
   ***********************************************/
  public int readByte(int address) {
    byte[] data = new byte[256];
    try {
      loadMemory(address, data);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return ~data[address & 0xff];
  }
  
  public void writeByte(int address, int data) {
    byte[] mem = new byte[256];
    try {
      loadMemory(address, mem);
      mem[address & 0xff] = (byte) ~data;
      writeBack(address, mem);
    } catch (IOException e) {
      e.printStackTrace();
    }
    if (loadedAddress >= 0
        && ((loadedAddress & 0xfff00) == (address & 0xfff00))) {
      loadedAddress = -1;
    }
  }

  @Override
  public int getSize() {
    return MEMORY_SIZE;
  }
  
  // Should return correct data!
  private int readMemory(int address) {
    if (DEBUG) {
      log("Reading memory address: " + Integer.toHexString(address));
    }
    ensureLoaded(address);
    return readMemory[address & 0xff];
  }

  private void writeBuffer(int address, int data) {
    buffer[address] = (byte) data;
  }

  private void ensureLoaded(int address) {
    if (loadedAddress < 0
	|| ((loadedAddress & 0xfff00) != (address & 0xfff00))) {
      try {
        if (DEBUG) {
          log("Loading memory: " + (address & 0xfff00));
        }
        loadMemory(address, readMemory);
      } catch (IOException e) {
        e.printStackTrace();
      }
      loadedAddress = address & 0xfff00;
    }
  }

  private void loadMemory(int address, byte[] readMemory) throws IOException {
    getStorage().read(address & 0xfff00, readMemory);
    for (int i = 0; i < readMemory.length; i++) {
      readMemory[i] = (byte) (~readMemory[i] & 0xff);
    }
  }
  
  public boolean getChipSelect() {
    return chipSelect;
  }
  
  public void portWrite(IOPort source, int data) {
    // Chip select = active low...
    if (chipSelect && (data & CHIP_SELECT) != 0) {
      // Chip select will go "off"
      switch(state) {
      case PAGE_PROGRAM:
        programPage();
        break;
      }
    }
    chipSelect = (data & CHIP_SELECT) == 0;
//    if (DEBUG) log("write to Port4: " +
//		       Integer.toString(data, 16)
//		       + " CS:" + chipSelect);
    state = 0;
  }

  private void writeStatus(double time) {
      writing = true;
      cpu.scheduleTimeEventMillis(writeEvent, time);
  }

  private void programPage() {
      if (writing) logw(WarningType.EXECUTION, "Can not set program page while already writing... from $" + Utils.hex(cpu.getPC(), 4));
      writeStatus(PROGRAM_PAGE_MILLIS);
      ensureLoaded(blockWriteAddress);
      for (int i = 0; i < readMemory.length; i++) {
          readMemory[i] &= buffer[i];
      }
      writeBack(blockWriteAddress, readMemory);
  }

  private void sectorErase(int address) {
    writeStatus(SECTOR_ERASE_MILLIS);
    int sectorAddress = address & 0xf0000;
    loadedAddress = -1;
    for (int i = 0; i < buffer.length; i++) {
      buffer[i] = (byte)0xff;
    }
    // Erase a complete sector
    blockWriteAddress = sectorAddress;
    for (int i = 0; i < 0x100; i++) {
      if (DEBUG) {
        log("erasing at $" + Integer.toHexString(blockWriteAddress));
      }
      writeBack(blockWriteAddress, buffer);
      blockWriteAddress += 0x100;
    }
  }


  private void writeBack(int address, byte[] data) {
    try {
      byte[] tmp = new byte[data.length];
      if (DEBUG) {
        log("Writing data to disk at $" + Integer.toHexString(address));
      }
      for (int i = 0; i < data.length; i++) {
        tmp[i] = (byte) (~data[i] & 0xff);
      }
      getStorage().write(address & 0xfff00, tmp);
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  @Override
  public int getModeMax() {
    return 0;
  }

  @Override
  public String info() {
      return "  Status: " + getStatus() + "  Write Enabled: " + writeEnable
      + "  Write in Progress: " + writing + "  Chip Select: " + chipSelect
      + "\n  " + getStorage().info();
  }

} // M25P80
