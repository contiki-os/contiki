/**
 * Copyright (c) 2007-2012, Swedish Institute of Computer Science.
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
 * USART
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;

public class USART extends IOUnit implements SFRModule, DMATrigger, USARTSource {

  // USART 0/1 register offset (0x70 / 0x78)
  public static final int UCTL = 0;
  public static final int UTCTL = 1;
  public static final int URCTL = 2;
  public static final int UMCTL = 3;
  public static final int UBR0 = 4;
  public static final int UBR1 = 5;
  public static final int URXBUF = 6;
  public static final int UTXBUF = 7;

  public static final int UTXIFG0 = 0x80;
  public static final int URXIFG0 = 0x40;

  public static final int UTXIFG1 = 0x20;
  public static final int URXIFG1 = 0x10;

  // USART SRF mod enable registers (absolute + 1)
  public static final int ME1 = 4;
  public static final int IE1 = 0;
  public static final int IFG1 = 2;

  private final int uartID;

  public static final int USART0_RX_VEC = 9;
  public static final int USART0_TX_VEC = 8;
  public static final int USART0_RX_BIT = 6;
  public static final int USART0_TX_BIT = 7;
  
  public static final int USART1_RX_VEC = 3;
  public static final int USART1_TX_VEC = 2;
  public static final int USART1_RX_BIT = 4;
  public static final int USART1_TX_BIT = 5;

  
  
  // Flags.
  public static final int UTCTL_TXEMPTY = 0x01;
  public static final int UTCTL_URXSE = 0x08;

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
  private int nextRXByte = -1;
  private boolean receiving = false;

  private final SFR sfr;

  private int uctl;
  private int utctl;
  private int urctl;
  private int umctl;
  private int ubr0;
  private int ubr1;
  private int urxbuf;
  private int utxbuf;
  private int txbit;
  
  private boolean txEnabled = false;
  private boolean rxEnabled = false;
  private boolean spiMode = false;
  
  /* DMA controller that needs to be called at certain times */
  private DMA dma;
  
  private TimeEvent txTrigger = new TimeEvent(0) {
    public void execute(long t) {
        // Ready to transmit new byte!
        handleTransmit(t);
    }
  };

  private TimeEvent rxTrigger = new TimeEvent(0) {
      public void execute(long t) {
          handleReceive();
      }
  };

  /**
   * Creates a new <code>USART</code> instance.
   *
   */
  public USART(MSP430Core cpu, int uartID, int[] memory, int offset) {
    super("USART" + uartID, "USART " + uartID, cpu, memory, offset);
    this.uartID = uartID;
    sfr = cpu.getSFR();

    // Initialize - transmit = ok...
    // and set which interrupts are used
    if (uartID == 0) {
      sfr.registerSFDModule(0, USART0_RX_BIT, this, USART0_RX_VEC);
      sfr.registerSFDModule(0, USART0_TX_BIT, this, USART0_TX_VEC);
      utxifg = UTXIFG0;
      urxifg = URXIFG0;
      txbit = USART0_TX_BIT;
      rxVector = USART0_RX_VEC;
    } else {
      sfr.registerSFDModule(1, USART1_RX_BIT, this, USART1_RX_VEC);
      sfr.registerSFDModule(1, USART1_TX_BIT, this, USART1_TX_VEC);
      utxifg = UTXIFG1;
      urxifg = URXIFG1;
      txbit = USART1_TX_BIT;
      rxVector = USART1_RX_VEC;
    }
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
    utctl |= UTCTL_TXEMPTY;
    txEnabled = false;
    rxEnabled = false;
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
//    if ((bits & utxifg) > 0) {
//        System.out.println(getName() + " Set utxifg");
//    }
    if (dma != null) {
        sfr.setBitIFG(uartID, bits);
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
    sfr.clrBitIFG(uartID, bits);
  }

  private int getIFG() {
    return sfr.getIFG(uartID);
  }

  private boolean isIEBitsSet(int bits) {
    return sfr.isIEBitsSet(uartID, bits);
  }

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
    address = address - offset;

    // Indicate ready to write!!! - this should not be done here...
//     if (uartID == 0) memory[IFG1] |= 0x82;
//     else memory[IFG1 + 1] |= 0x20;

//     System.out.println(">>>> Write to " + getName() + " at " +
// 		       address + " = " + data);
    switch (address) {
    case UCTL:
      uctl = data;
      spiMode = (data & 0x04) > 0;
      if (DEBUG) log(" write to UCTL " + data);
      break;
    case UTCTL:
      utctl = data;
      if (DEBUG) log(" write to UTCTL " + data);

      if (((data >> 4) & 3) == 1) {
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
      if ((data & UTCTL_URXSE) == UTCTL_URXSE) {
          sfr.setAutoclear(rxVector, false);
      } else {
          sfr.setAutoclear(rxVector, true);
      }
      updateBaudRate();
      break;
    case URCTL:
      urctl = data;
      break;
    case UMCTL:
      umctl = data;
      if (DEBUG) log(" write to UMCTL " + data);
      break;
    case UBR0:
      ubr0 = data;
      updateBaudRate();
      break;
    case UBR1:
      ubr1 = data;
      updateBaudRate();
      break;
    case UTXBUF:
      if (DEBUG) log(" USART_UTXBUF: " + data + " " + (data > 32 ? (char)data : '.'));
      if (txEnabled || (spiMode && rxEnabled)) {
        // Interruptflag not set!
        clrBitIFG(utxifg);
        /* the TX is no longer empty ! */
        utctl &= ~UTCTL_TXEMPTY;
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
    address = address - offset;
//     System.out.println(">>>>> Read from " + getName() + " at " +
// 		       address + " = " + memory[address]);
    
    switch (address) {
    case UCTL:
      if (DEBUG) log(" read from UCTL");
      return uctl;
    case UTCTL:
      if (DEBUG) log(" read from UTCTL: " + utctl);
      return utctl;
    case URCTL:
      return urctl;
    case UMCTL:
      return umctl;
    case UBR0:
      return ubr0;
    case UBR1:
      return ubr1;
    case UTXBUF:
      return utxbuf;
    case URXBUF:
      int tmp = urxbuf;
      // When byte is read - the interruptflag is cleared!
      // and error status should also be cleared later...
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
            /* TXEMPTY means both TXBUF and shiftreg empty */
            utctl |= UTCTL_TXEMPTY;
            transmitting = false;
            txShiftReg = -1;
        }
    }

    /* any more chars to transmit? */
    if (nextTXByte != -1) {
        txShiftReg = nextTXByte;
        nextTXByte = -1;
        transmitting = true;
        /* txbuf always empty after this */
        setBitIFG(utxifg);
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
    return !receiving && (getIFG() & urxifg) == 0;
  }

  // A byte have been received!
  // This needs to be complemented with a method for checking if the USART
  // is ready for next byte (readyForReceive) that respects the current speed
  public void byteReceived(int b) {
    if (!rxEnabled) return;
    if (DEBUG) {
      log(" byteReceived: " + b + " " + (b > 32 ? (char)b : '.'));
    }
    nextRXByte = b & 0xff;
    if (!receiving) {
      receiving = true;
      cpu.scheduleCycleEvent(rxTrigger, cpu.cycles + 1);
    }
  }

  private void handleReceive() {
    receiving = false;
    urxbuf = nextRXByte;

    // Indicate interrupt!
    setBitIFG(urxifg);

    if (DEBUG) {
      // Check if the IE flag is enabled! - same as the IFlag to indicate!
      if (isIEBitsSet(urxifg)) {
        log(" flagging receive interrupt ");
      }
    }
  }

  public String info() {
      return "  UTXIE: " + isIEBitsSet(utxifg) + "  URXIE: " + isIEBitsSet(urxifg) + "\n" +
      "  UTXIFG: " + ((getIFG() & utxifg) > 0) + "  URXIFG: " + ((getIFG() & urxifg) > 0) + "\n" +
      "  Baudrate: " + baudRate + " bps  Cycles per byte: " + tickPerByte;
  }

  public boolean getDMATriggerState(int index) {
      if (index == 0) {
          return (getIFG() & urxifg) > 0;
      } else {
          return (getIFG() & utxifg) > 0;
      }
  }

  public void clearDMATrigger(int index) {
      if (index == 0) {
          /* clear RX - might be different in different modes... */
          if (DEBUG) {
              log("clearing DMA read bit!");
          }
          clrBitIFG(urxifg);
          stateChanged(USARTListener.RXFLAG_CLEARED, true);
      } else {
          if (DEBUG) {
              log("clearing DMA " + index);
          }
          /* clear TX - might be different in different modes... */
          clrBitIFG(utxifg);
      }
  }
}
