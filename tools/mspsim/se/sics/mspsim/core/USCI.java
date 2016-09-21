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
 * -----------------------------------------------------------------
 *
 * USCI Module for the MSP430xf2xxx series.
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;


public class USCI extends IOUnit implements SFRModule, DMATrigger, USARTSource {

  // USART 0/1 register offset (0x60 / 0xD0)
  public static final int UAxCTL0 = 0;
  public static final int UAxCTL1 = 1;
  public static final int UAxBR0 = 2;
  public static final int UAxBR1 = 3;
  public static final int UAxMCTL = 4;
  public static final int UAxSTAT = 5;
  public static final int UAxRXBUF = 6;
  public static final int UAxTXBUF = 7;
  
  // SPI related registers
  public static final int UBxCTL0 = 8;
  public static final int UBxCTL1 = 9;
  public static final int UBxBR0 = 0xa;
  public static final int UBxBR1 = 0xb;
  public static final int UBxSTAT = 0xd;
  public static final int UBxRXBUF = 0xe;
  public static final int UBxTXBUF = 0xf;
  
  // IrDA related (negative offset ??)
  public static final int UABCTL = -3; /* very odd... */
  public static final int UIRTCTL = -2;
  public static final int UIRRCTL = -1;
  
  // Interrupt flags...
  public static final int UTXIFG0 = 0x80;
  public static final int URXIFG0 = 0x40;

  public static final int UTXIFG1 = 0x20;
  public static final int URXIFG1 = 0x10;

  // USART SRF mod enable registers (absolute + 1)
  public static final int ME1 = 4;
  public static final int IE1 = 0;
  public static final int IFG1 = 2;

  private final int uartID;

  // Flags.
  public static final int UTCTL_TXEMPTY = 0x01;
  public static final int UTCTL_URXSE = 0x08;
  public static final int USCI_BUSY = 0x01;
  
  private USARTListener usartListener;

  private int utxifg;
  private int urxifg;
  private int rxVector;

  private int clockSource = 0;
  private int baudRate = 0;
  private int tickPerByte = 1000;
  private long nextTXReady = -1;
  private int nextTXByte = -1;
  private int txShiftReg = -1;
  private boolean transmitting = false;

  private final SFR sfr;
  private final int sfrAddress;

  /* ifg and ie if not in sfr... - assume IE in sfraddr and IFG in addr + 1*/
  private int ifgAddress = 0;
  private int ieAddress = 0;

  private int uctl0;
  private int uctl1;
  private int umctl;
  private int ubr0;
  private int ubr1;
  private int urxbuf;
  private int utxbuf;
  private int ustat;
  private int txbit;
  
  private boolean txEnabled = false;
  private boolean rxEnabled = false;
  private boolean spiMode = false;
  
  /* DMA controller that needs to be called at certain times */
  private DMA dma;
  private int dmaIndex;
  
  private boolean usciA = true; /* if this is an USCI A or B */
  
  private TimeEvent txTrigger = new TimeEvent(0) {
    public void execute(long t) {
        // Ready to transmit new byte!
        handleTransmit(t);
    }
  };
  
  /**
   * Creates a new <code>USART</code> instance.
   *
   */
  public USCI(MSP430Core cpu, int uartID, int[] memory, MSP430Config config) {
    super(config.uartConfig[uartID].name, cpu, memory, config.uartConfig[uartID].offset);
    this.uartID = uartID;
    MSP430Config.UARTConfig uartConfig = config.uartConfig[uartID];

    // Initialize - transmit = ok...
    // and set which interrupts are used
    if (uartConfig.sfrAddr < 2) {
        sfr = cpu.getSFR();
        sfrAddress = uartConfig.sfrAddr;
        sfr.registerSFDModule(uartConfig.sfrAddr, uartConfig.rxBit, this, uartConfig.rxVector);
        sfr.registerSFDModule(uartConfig.sfrAddr, uartConfig.txBit, this, uartConfig.txVector);
    } else {
        sfr = null;
        sfrAddress = 0;
        ieAddress = uartConfig.sfrAddr;
        ifgAddress = uartConfig.sfrAddr + 1;
    }
    utxifg = 1 << uartConfig.txBit;
    urxifg = 1 << uartConfig.rxBit;
    txbit = uartConfig.txBit;
    rxVector = uartConfig.rxVector;
    usciA = uartConfig.usciA;
    reset(0);
  }

  public void setDMA(DMA dma) {
      this.dma = dma;
  }

  
  public void reset(int type) {
    nextTXReady = cpu.cycles + 100;
    txShiftReg = nextTXByte = -1;
    transmitting = false;
    clrBitIFG(urxifg);
    setBitIFG(utxifg); /* empty at start! */
    ustat &= ~USCI_BUSY;
    txEnabled = true; //false;
    rxEnabled = true; //false;
  }

  public void enableChanged(int reg, int bit, boolean enabled) {
    if (DEBUG) log("enableChanged: " + reg + " bit: " + bit +
        " enabled = " + enabled + " txBit: " + txbit);
    if (bit == txbit) {
      txEnabled = enabled;
    } else {
      rxEnabled = enabled;
    }
  }
  
  private void setBitIFG(int bits) {
//    if ((bits) > 0) {
//        System.out.println(getName() + " Set utxifg = " + utxifg +
//                " sfrA: " + sfrAddress + " bits: " + bits);
//    }
    if (sfr != null) {
        sfr.setBitIFG(sfrAddress, bits);
//        System.out.println("SFR =>" + sfr.getIFG(sfrAddress));
    } else {
        memory[ifgAddress] |= bits;
    }
    if (dma != null) {
        /* set bit first, then trigger DMA transfer - this should
         * be made via a 1 cycle or so delayed action */
        if ((bits & urxifg) > 0) dma.trigger(this, 0);
        if ((bits & utxifg) > 0) dma.trigger(this, 1);
    }
  }

  private void clrBitIFG(int bits) {
//      if ((bits & utxifg) > 0) {
//          System.out.println(getName() + " Clear utxifg");
//      }
      if (sfr != null) {
          sfr.clrBitIFG(sfrAddress, bits);
      } else {
          memory[ifgAddress] &= ~bits;
      }
  }

  private int getIFG() {
      if (sfr != null)
          return sfr.getIFG(sfrAddress);
      return memory[ifgAddress];
  }

  private boolean isIEBitsSet(int bits) {
      if (sfr != null)
          return sfr.isIEBitsSet(sfrAddress, bits);
      return (memory[ieAddress] & bits) > 0;
  }

  /* reuse USART listener API for USCI */
  @Override
  public synchronized void addUSARTListener(USARTListener listener) {
      usartListener = USARTListener.Proxy.INSTANCE.add(usartListener, listener);
  }

  @Override
  public synchronized void removeUSARTListener(USARTListener listener) {
      usartListener = USARTListener.Proxy.INSTANCE.remove(usartListener, listener);
  }

  // Only 8 bits / read!
  public void write(int address, int data, boolean word, long cycles) {
    if (address == ieAddress || address == ifgAddress) {
        memory[address] = data;
    }
    address = address - offset;

    // Indicate ready to write!!! - this should not be done here...
//    System.out.println(">>>> Write to " + getName() + " at " +
//            address + " = " + data);
    switch (address) {
    case UAxCTL0:
    case UBxCTL0:
      uctl0 = data;
      spiMode = (data & 0x01) > 0;
      if (DEBUG) log(" write to UAxCTL0 " + data);
      break;
    case UAxCTL1:
    case UBxCTL1:
        /* emulate the reset */
        if ((uctl1 & 1) == 1 && (data & 1) == 0)
            reset(0);
      uctl1 = data;
      if (DEBUG) log(" write to UAxCTL1 " + data);

      if (((data >> 6) & 3) == 1) {
        clockSource = MSP430Constants.CLK_ACLK;
        if (DEBUG) {
          log(" Selected ACLK as source");
        }
      } else {
        clockSource = MSP430Constants.CLK_SMCLK;
        if (DEBUG) {
          log(" Selected SMCLK as source");
        }
      }
      updateBaudRate();
      break;
    case UAxMCTL:
      umctl = data;
      if (DEBUG) log(" write to UMCTL " + data);
      break;
    case UAxBR0:
    case UBxBR0:
      ubr0 = data;
      updateBaudRate();
      break;
    case UAxBR1:
    case UBxBR1:
      ubr1 = data;
      updateBaudRate();
      break;
    case UAxSTAT:
    case UBxSTAT:
        //ustat = data;
      break;
    case UAxTXBUF:
    case UBxTXBUF:
      if (DEBUG) log(": USART_UTXBUF:" + (char) data + " = " + data + "\n");
      if (txEnabled || (spiMode && rxEnabled)) {
        // Interruptflag not set!
        clrBitIFG(utxifg);
        /* the TX is no longer empty ! */
        ustat |= USCI_BUSY;
        /* should the interrupt be flagged off here ? - or only the flags */
        if (DEBUG) log(" flagging off transmit interrupt");
        //      cpu.flagInterrupt(transmitInterrupt, this, false);

        // Schedule on cycles here
        // TODO: adding 3 extra cycles here seems to give
        // slightly better timing in some test...

        nextTXByte = data;
        if (!transmitting) {
            /* how long time will the copy from the TX_BUF to the shift reg take? */
            /* assume 3 cycles? */
            nextTXReady = cycles + 1; //tickPerByte + 3;
            cpu.scheduleCycleEvent(txTrigger, nextTXReady);
        }
      } else {
        log("Ignoring UTXBUF data since TX not active...");
      }
      utxbuf = data;
      break;
    }
  }

  public int read(int address, boolean word, long cycles) {
      if (address == ieAddress || address == ifgAddress) {
          return memory[address];
      }
    address = address - offset;
//    System.out.println(">>>>> Read from " + getName() + " at " +
//            address);
    
    switch (address) {
    case UAxCTL0:
    case UBxCTL0:
      if (DEBUG) log(" read from UCTL");
      return uctl0;
    case UAxCTL1:
    case UBxCTL1:
      if (DEBUG) log(" read from UTCTL: " + uctl1);
      return uctl1;
    case UAxMCTL:
      return umctl;
    case UAxBR0:
    case UBxBR0:
      return ubr0;
    case UAxBR1:
    case UBxBR1:
      return ubr1;
    case UAxTXBUF:
    case UBxTXBUF:
      return utxbuf;
    case UAxSTAT:
    case UBxSTAT:
//      System.out.println(getName() + " Reading STAT: " + ustat);
      return ustat;
    case UAxRXBUF:
    case UBxRXBUF:
      int tmp = urxbuf;
      // When byte is read - the interruptflag is cleared!
      // and error status should also be cleared later...
      // is this cleared also on the MSP430x2xx series???
      if (MSP430Constants.DEBUGGING_LEVEL > 0) {
          log(" clearing rx interrupt flag " + cpu.getPC() + " byte: " + tmp);
      }
      clrBitIFG(urxifg);
      /* This should be changed to a state rather than an "event" */
      /* Force callback since this is not used as a state */
      stateChanged(USARTListener.RXFLAG_CLEARED, true);
      return tmp;
    }
    return 0;
  }

  private void updateBaudRate() {
    int div = ubr0 + (ubr1 << 8);
    if (div == 0) {
      div = 1;
    }
    if (clockSource == MSP430Constants.CLK_ACLK) {
      if (DEBUG) {
        log(" Baud rate is (bps): " + cpu.aclkFrq / div + " div = " + div);
      }
      baudRate = cpu.aclkFrq / div;
    } else {
      if (DEBUG) {     
        log(" Baud rate is (bps): " + cpu.smclkFrq / div + " div = " + div);
      }
      baudRate = cpu.smclkFrq / div;
    }
    if (baudRate == 0) baudRate = 1;
    // Is this correct??? Is it the DCO or smclkFRQ we should have here???
    tickPerByte = (8 * cpu.smclkFrq) / baudRate;
    if (DEBUG) {
      log(" Ticks per byte: " + tickPerByte);
    }
  }

  // We should add "Interrupt serviced..." to indicate that its latest
  // Interrupt was serviced...
  public void interruptServiced(int vector) {
    /* NOTE: this is handled by SFR : clear IFG bit if interrupt is serviced */
//      System.out.println(getName() + " SFR irq " + vector + " " + txShiftReg + " " + getIFG());
  }

  private void handleTransmit(long cycles) {
    if (cpu.getMode() >= MSP430Core.MODE_LPM3) {
      logw(WarningType.EXECUTION, "Warning: USART transmission during LPM!!! " + nextTXByte);
    }

    if (transmitting) {
        /* in this case we have shifted out the last character */
        USARTListener listener = this.usartListener;
        if (listener != null && txShiftReg != -1) {
            listener.dataReceived(this, txShiftReg);
        }
        /* nothing more to transmit after this - stop transmission */
        if (nextTXByte == -1) {
            /* ~BUSY - nothing more to send - and last data already in RX */
            ustat &= ~USCI_BUSY;
            transmitting = false;
            txShiftReg = -1;
        }
    }

    /* any more chars to transmit? */
    if (nextTXByte != -1) {
        txShiftReg = nextTXByte;
        nextTXByte = -1;
        /* txbuf always empty after this */
        setBitIFG(utxifg);
        transmitting = true;
        nextTXReady = cycles + tickPerByte + 1;
        cpu.scheduleCycleEvent(txTrigger, nextTXReady);
    }

    if (DEBUG) {
      if (isIEBitsSet(utxifg)) {
        log(" flagging on transmit interrupt");
      }
      log(" Ready to transmit next at: " + cycles);
    }
  }


  public boolean isReceiveFlagCleared() {
    return (getIFG() & urxifg) == 0;
  }

  // A byte have been received!
  // This needs to be complemented with a method for checking if the USART
  // is ready for next byte (readyForReceive) that respects the current speed
  public void byteReceived(int b) {
      //System.out.println(getName() + " byte received: " + b + " enabled:" + rxEnabled);
      if (!rxEnabled) return;
    
    if (DEBUG || true) {
      log(" byteReceived: " + b + " " + (char) b);
    }
    urxbuf = b & 0xff;
    // Indicate interrupt also!
    setBitIFG(urxifg);

    // Check if the IE flag is enabled! - same as the IFlag to indicate!
    if (isIEBitsSet(urxifg)) {
      if (DEBUG) {
        log(" flagging receive interrupt ");
      }
    }
  }
  
  public String info() {
      return "UTXIE: " + isIEBitsSet(utxifg) + "  URXIE:" + isIEBitsSet(urxifg) + "\n" +
      "UTXIFG: " + ((getIFG() & utxifg) > 0) + "  URXIFG:" + ((getIFG() & urxifg) > 0);
  }

  public boolean getDMATriggerState(int index) {
      if (index == 0) {
          return (getIFG() & urxifg) > 0;
      } else {
          return (getIFG() & utxifg) > 0;
      }
  }
  
  public void clearDMATrigger(int index) {
//      System.out.println("UART clearing DMA " + index);
      if (index == 0) {
          /* clear RX - might be different in different modes... */
//          System.out.println("UART clearing read bit!");
          clrBitIFG(urxifg);
          stateChanged(USARTListener.RXFLAG_CLEARED, true);
      } else {
          /* clear TX - might be different in different modes... */
          clrBitIFG(utxifg);
      }
  }
}