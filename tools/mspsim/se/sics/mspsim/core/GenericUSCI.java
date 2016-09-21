package se.sics.mspsim.core;

import java.util.ArrayDeque;

import se.sics.mspsim.chip.I2CUnit.I2CData;


/** 
 * GenericUSCI - for newer MSP430's
 * 
 * @author Unknown
 * @author Víctor Ariño <victor.arino@tado.com>
 */
public class GenericUSCI extends IOUnit implements DMATrigger, USARTSource {

    // USCI A/Bx common register offset
    public static final int CTL0 = 1; /* Is this really correct??? */
    public static final int CTL1 = 0;
    public static final int BR0 = 6;
    public static final int BR1 = 7;
    public static final int MCTL = 8;
    public static final int STAT = 0x0a;
    public static final int RXBUF = 0x0c;
    public static final int TXBUF = 0x0e;
    
    // i2C Registers
    public static final int I2COA = 0x10;
    public static final int I2CSA = 0x12; 

    // Interrupt related
    public static final int IE = 0x1c;
    public static final int IFG = 0x1d;
    public static final int IV = 0x1e;

    // Misc flags
    public static final int RXIFG = 0x01;
    public static final int TXIFG = 0x02;
    
    public static final int USCI_BUSY = 0x01;

    public static final int SWRST = 0x01;
    
    private USARTListener usartListener;

    private int ubr0;
    private int ubr1;

    protected int ie;
    protected int ifg;
    protected int iv = 2; /* TODO Implement me! */

    private int clockSource = 0;
    private int baudRate = 0;

    private int tickPerByte = 1000;
    private long nextTXReady = -1;
    private boolean transmitting = false;

    private int ctl0;
    private int ctl1;
    private int br0;
    private int br1;
    private int mctl;
    private int rxbuf;
    private int txbuf;
    private int stat;

    private boolean syncMode = false;

    /* always on for now - but SWRST controls it */
    private boolean moduleEnabled = true;

    private final int uartIndex;
    private final int vector;

    private TimeEvent txTrigger = new TimeEvent(0) {
        public void execute(long t) {
            // Ready to transmit new byte!
            handleTransmit(t);
        }
    };
	private boolean i2cEnabled;
	private boolean i2cTransmitter;
	private int i2cSlaveAddress;
	private int i2cOwnAddress;
	private boolean readyForNextTransmit;
	private boolean stopConditionPending;
	
	private ArrayDeque<Integer> txBuffer = new ArrayDeque<Integer>(100);

    public GenericUSCI(MSP430Core cpu, int uartIndex, int[] memory, MSP430Config config) {
        super(config.uartConfig[uartIndex].name, cpu, memory, config.uartConfig[uartIndex].offset);
        /* do some stuff ? */
        
        this.uartIndex = uartIndex;
        MSP430Config.UARTConfig uartConfig = config.uartConfig[uartIndex];

        /* both vectors are the same in modern MSP430 USCIs (f5xxx) */
        vector = uartConfig.rxVector;
        reset(0);
    }

    public void reset(int type) {
        nextTXReady = cpu.cycles + tickPerByte + 100;
        transmitting = false;
        clrBitIFG(RXIFG);
        setBitIFG(TXIFG); /* empty at start! */
        stat &= ~USCI_BUSY;
        moduleEnabled = true; //false;
        updateIV();
        txBuffer.clear();
      }

    void updateIV() {
        int bitval = 0x01;
        iv = 0;
        int ie_ifg = ifg & ie;
        for (int i = 0; i < 8; i++) {
            if ((bitval & ie_ifg) > 0) {
                iv = 2 + i * 2;
                break;
            }
            bitval = bitval << 1;
        }
        //System.out.println("*** Setting IV to: " + iv + " ifg: " + ifg);
    }
    
    protected void setBitIFG(int bits) {
        ifg |= bits;

        // TODO: implement DMA... 
        //        if (dma != null) {
        //            /* set bit first, then trigger DMA transfer - this should
        //             * be made via a 1 cycle or so delayed action */
        //            if ((bits & urxifg) > 0) dma.trigger(this, 0);
        //            if ((bits & utxifg) > 0) dma.trigger(this, 1);
        //        }
        updateIV();
        if ((ifg & ie) > 0) cpu.flagInterrupt(vector, this, true);
    }

    protected void clrBitIFG(int bits) {
        ifg &= ~bits;
        /* if no more interrupts here - turn off... */
        updateIV();
        if ((ifg & ie) == 0) cpu.flagInterrupt(vector, this, false);
    }

    protected int getIFG() {
        return ifg;
    }

    private boolean isIEBitsSet(int bits) {
        return (bits & ie) != 0;
    }
    
    private void handleTransmit(long cycles) {
        if (cpu.getMode() >= MSP430Core.MODE_LPM3) {
            System.out.println(getName() + " Warning: USART transmission during LPM!!! ");
        }
        
        if (transmitting) {
            /* in this case we have shifted out the last character */
            USARTListener listener = this.usartListener;
            if (listener != null && !txBuffer.isEmpty()) {
            	int t = txBuffer.remove();
                listener.dataReceived(this, t);
                
                if (i2cEnabled) {  
                	if ((t & I2CData.START) > 0) {
                		ctl1 &= ~0x02;
                	} else if ((t & I2CData.STOP) > 0) { 
                		ctl1 &= ~0x04;
                	}
                }
            }
            /* nothing more to transmit after this - stop transmission */
            if (txBuffer.isEmpty()) {
        		/* ~BUSY - nothing more to send - and last data already in RX */
            	stat &= ~USCI_BUSY;
        		transmitting = false;
        		setBitIFG(TXIFG);
            }
        }

        /* any more chars to transmit? */
        if (!txBuffer.isEmpty()) {
            /* txbuf always empty after this */
            clrBitIFG(TXIFG);
            transmitting = true;
            nextTXReady = cycles + tickPerByte + 1;
            cpu.scheduleCycleEvent(txTrigger, nextTXReady);
        }

        if (DEBUG) {
            if (isIEBitsSet(TXIFG)) {
                log(" flagging on transmit interrupt");
            }
            log(" Ready to transmit next at: " + cycles);
        }
    }


    protected void updateBaudRate() {
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


    public void interruptServiced(int vector) {
    }

    // Only 8 bits / read!
    public void write(int address, int data, boolean word, long cycles) {
      address = address - offset;

      // Indicate ready to write!!! - this should not be done here...
//      System.out.println(">>>> Write to " + getName() + " at " +
//              address + " = " + data);
      switch (address) {
      case CTL0:
        syncMode = (data & 0x01) > 0;
        i2cEnabled = (data & 0x06) == 0x06;
        if (DEBUG) log(" write to UxxCTL0 " + data);
        break;
      case CTL1:
          /* emulate the reset */
          if ((ctl1 & SWRST) == SWRST && (data & SWRST) == 0)
              reset(0);
          moduleEnabled = (data & SWRST) == 0;
          if (DEBUG) log(" write to UxxCTL1 " + data + " => ModEn:" + moduleEnabled);

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
          
          /*
           * When in I2C mode and an start or stop condition is reached
           * just transmit the corresponding signals.
           */
          if (i2cEnabled) {
        	  if ((data & 0x04) > 0 && (ctl1 & 0x04) == 0) { // stop condition
        		  txBuffer.add(I2CData.STOP);
        	  } 
        	  if ((data & 0x02) > 0 && (ctl1 & 0x02) == 0) {
        		  i2cTransmitter = (data & 0x10) == 0x10;
        		  /* Transmit a start condition START|ADDR|R/W */
        		  int t = I2CData.START;
        		  t |= i2cSlaveAddress << 1;
        		  t |= i2cTransmitter ? 1 : 0;
        		  txBuffer.add(t);
        		  rxbuf = 0;
        	  } 
        	  clrBitIFG(TXIFG);
        	  stat |= USCI_BUSY;
              if (!transmitting) {
                  nextTXReady = cycles + 1; //tickPerByte + 3;
                  cpu.scheduleCycleEvent(txTrigger, nextTXReady);
              }
          }
          
          ctl1 = data;
          
          break;
      case MCTL:
        mctl = data;
        if (DEBUG) log(" write to UMCTL " + data);
        break;
      case BR0:
        ubr0 = data;
        updateBaudRate();
        break;
      case BR1:
        ubr1 = data;
        updateBaudRate();
        break;
      case STAT:
          //ustat = data;
        break;
      case IE:
          ie = data;
        break;
      case TXBUF:
        if (DEBUG) log(": USART_UTXBUF:" + (char) data + " = " + data + "\n");
        if (moduleEnabled) {
          // Interruptflag not set!
          clrBitIFG(TXIFG);
          /* the TX is no longer empty ! */
          stat |= USCI_BUSY;
          /* should the interrupt be flagged off here ? - or only the flags */
          if (DEBUG) log(" flagging off transmit interrupt");
          //      cpu.flagInterrupt(transmitInterrupt, this, false);

          // Schedule on cycles here
          // TODO: adding 3 extra cycles here seems to give
          // slightly better timing in some test...
          txBuffer.add(data);
          if (!transmitting) {
              /* how long time will the copy from the TX_BUF to the shift reg take? */
              /* assume 3 cycles? */
              nextTXReady = cycles + tickPerByte + 1; //tickPerByte + 3;
              cpu.scheduleCycleEvent(txTrigger, nextTXReady);
          }
        } else {
          log("Ignoring UTXBUF data since TX not active...");
        }
        txbuf = data;
        break;
        
      case I2CSA:
    	i2cSlaveAddress = data & 0x3ff;
    	break;
    	
      case I2COA:
    	i2cOwnAddress = data & 0x3ff;
    	break;
      }
    }

    public int read(int address, boolean word, long cycles) {
        int op = address - offset;
        switch (op) {
        case CTL0:
            return ctl0;
        case CTL1:
            return ctl1;
        case BR0:
            return br0;
        case BR1:
            return br1;
        case TXBUF:
            return txbuf;
        case RXBUF:
            int tmp = rxbuf;
            // When byte is read - the interruptflag is cleared!
            // and error status should also be cleared later...
            // is this cleared also on the MSP430x5xx series???
            if (MSP430Constants.DEBUGGING_LEVEL > 0) {
                log(" clearing rx interrupt flag " + cpu.getPC() + " byte: " + tmp);
            }
            clrBitIFG(RXIFG);
            /* This should be changed to a state rather than an "event" */
            /* Force callback since this is not used as a state */
            stateChanged(USARTListener.RXFLAG_CLEARED, true);
            
            /* For timing issues we have to send the ACK after reading the
             * register */
            if (i2cEnabled && !i2cTransmitter) {
        		txBuffer.add(I2CData.ACK);
        		stat |= USCI_BUSY;
                if (!transmitting) {
                    /* how long time will the copy from the TX_BUF to the shift reg take? */
                    /* assume 3 cycles? */
                    nextTXReady = cpu.cpuCycles + tickPerByte + 1; //tickPerByte + 3;
                    cpu.scheduleCycleEvent(txTrigger, nextTXReady);
                }
            }
            
            return tmp;
        case MCTL:
            return mctl;
        case STAT:
            return stat;
        case IE:
            return ie;
        case IFG:
            return ifg;
        case IV:
            return iv;
        }
        return 0;
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

    /*  default behavior assumes UART/SPI config */
    public boolean isReceiveFlagCleared() {
        return (ifg & RXIFG) == 0;
    }

    // A byte have been received!
    // This needs to be complemented with a method for checking if the USART
    // is ready for next byte (readyForReceive) that respects the current speed
    public void byteReceived(int b) {
        //System.out.println(getName() + " byte received: " + b);

        if (DEBUG) {
            log("byteReceived: " + b + " " + (char) b);
        }
        
        /* Ignore i2c ACK messages */
        if (i2cEnabled && b == I2CData.ACK) {
			return;
        }
        
        rxbuf = b & 0xff;
        
        // Indicate interrupt also!
        setBitIFG(RXIFG);

        // Check if the IE flag is enabled! - same as the IFlag to indicate!
        if (isIEBitsSet(RXIFG)) {
            if (DEBUG) {
                log(" flagging receive interrupt ");
            }
        }
    }

    /* TODO: IMPLEMENT DMA! */
    public void setDMA(DMA dma) {
    }

    public boolean getDMATriggerState(int index) {
        return false;
    }

    public void clearDMATrigger(int index) {
    }

    public String info() {
        return "UTXIE: " + isIEBitsSet(TXIFG) + "  URXIE:" + isIEBitsSet(RXIFG) + "\n" +
        "UTXIFG: " + ((getIFG() & TXIFG) > 0) + "  URXIFG:" + ((getIFG() & RXIFG) > 0);
    }
        
    public int getBaudRate() {
    	return tickPerByte;
    }
    
    protected void vlog(String msg) {
    	if (i2cEnabled) {
    		System.out.println("USCI "+msg);
    	}
    }

}
