/*
 * Copyright (c) 2012, Thingsquare.
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
 */

package se.sics.mspsim.core;

import se.sics.mspsim.chip.CC1101;
import se.sics.mspsim.chip.CC1101.GDOListener;
import se.sics.mspsim.core.IOPort.PinState;

public class RF1A extends IOUnit implements InterruptHandler {
    private boolean DEBUG = true;
    
    public static int ADDRESS = 0xf00;
    public static int SIZE = 64;

    private static final int RF1AIFCTL0 = 0x00;
    private static final int RF1AIFCTL1 = 0x02;
    private static final int RF1AIFCTL2 = 0x04;
    private static final int RF1AIFERR = 0x06;
    private static final int RF1AIFERRV = 0x0c;
    private static final int RF1AIFIV = 0x0e;
    private static final int RF1AINSTRW = 0x10;
    private static final int RF1AINSTR1W = 0x12;
    private static final int RF1AINSTR2B = 0x14;
    private static final int RF1ADINW = 0x16;
    private static final int RF1ADOUTB = 0x20;
    private static final int RF1ASTATB = 0x21;
    private static final int RF1ASTAT1W = 0x22;
    private static final int RF1ASTAT2W = 0x24;
    private static final int RF1ADOUT0W = 0x28;
    private static final int RF1ADOUT1W = 0x2a;
    private static final int RF1ADOUT2W = 0x2c;
    private static final int RF1AIN = 0x30;
    private static final int RF1AIFG = 0x32;
    private static final int RF1AIES = 0x34;
    private static final int RF1AIE = 0x36;
    private static final int RF1AIV = 0x38;
    private static final int RF1ARXFIFO = 0x3c;
    private static final int RF1ATXFIFO = 0x3e;

    private CC1101 cc1101;

    private boolean interruptOnCC1101GDO0 = false;
    private boolean gdo0IsHigh = false;

    public RF1A(final MSP430Core cpu, int[] memory) {
        super("RF1A", "RF1A", cpu, memory, ADDRESS);
        cc1101 = new CC1101(cpu);
        cc1101.setGDO0Listener(new GDOListener() {
          public void event(PinState state) {
            /* cc430f5137 datasheet p. 17 */
            gdo0IsHigh = (state == PinState.HI);
            cpu.flagInterrupt(53, RF1A.this, gdo0IsHigh);
          }
        });

        reset(0);
    }    

    public void reset(int type) {
        /* Initial values */
        memory[ADDRESS + 0x00] = 0x0000; /* RF1AIFCTL0 */
        memory[ADDRESS + 0x02] = 0x0000; /* RF1AIFCTL1 */
        memory[ADDRESS + 0x04] = 0x0000; /* RF1AIFCTL2 */
        memory[ADDRESS + 0x06] = 0x0000; /* RF1AIFERR */
        memory[ADDRESS + 0x0c] = 0x0000; /* RF1AIFERRV */
        memory[ADDRESS + 0x0e] = 0x0000; /* RF1AIFIV */
        memory[ADDRESS + 0x10] = 0x0000; /* RF1AINSTRW */

        memory[ADDRESS + 0x12] = 0x0000; /* RF1AINSTR1W */
        memory[ADDRESS + 0x14] = 0x0000; /* RF1AINSTR2B */
        memory[ADDRESS + 0x16] = 0x0000; /* RF1ADINW */

        memory[ADDRESS + 0x20] = 0x0000; /* RF1ASTAT0W */
        memory[ADDRESS + 0x22] = 0x0000; /* RF1ASTAT1W */
        memory[ADDRESS + 0x24] = 0x0000; /* RF1ASTAT2W */
        memory[ADDRESS + 0x28] = 0x0000; /* RF1ADOUT0W */

        memory[ADDRESS + 0x2a] = 0x0000; /* RF1ADOUT1W */
        memory[ADDRESS + 0x2c] = 0x0000; /* RF1ADOUT2W */

        memory[ADDRESS + 0x30] = 0x0000; /* RF1AIN */
        memory[ADDRESS + 0x32] = 0x0000; /* RF1AIFG */
        memory[ADDRESS + 0x34] = 0x0000; /* RF1AIES */
        memory[ADDRESS + 0x36] = 0x0000; /* RF1AIE */
        memory[ADDRESS + 0x38] = 0x0000; /* RF1AIV */

        memory[ADDRESS + 0x3c] = 0x0000; /* RF1ARXFIFO */
        memory[ADDRESS + 0x3e] = 0x0000; /* RF1ATXFIFO */

        interruptOnCC1101GDO0 = false;

        ioAddress = -1;
        ioBurst = false;
        ioRead = false;
        ioWrite = false;
        expectingDummyWrite = false;
    }

    private int ioAddress = -1;
    private boolean ioBurst = false;
    private boolean ioRead = false;
    private boolean ioWrite = false;
    private boolean expectingDummyWrite = false;
    public void write(int address, int value, boolean word, long cycles) {
        /*if (DEBUG) {
            System.out.printf(this.getName() + ": Write to 0x%04x: 0x%04x\n", address, value);
        }*/

        if (address == ADDRESS + RF1AIE) {
          /* Configure interrupts */
          interruptOnCC1101GDO0 = (value & 1) != 0;
        } else if (address == ADDRESS + RF1AINSTRW + 1) {
            /* Store address. Read/write? Burst/single? */
            expectingDummyWrite = false;
            ioBurst = CC1101.spiIsBurst(value);
            if (!ioBurst && CC1101.spiIsStrobe(value)) {
              /* Execute strobe command */
              cc1101.setLastInstructionWasRead(false); /* This is probably wrong! */
              ioBurst = false;
              ioRead = false;
              ioWrite = false;
              expectingDummyWrite = true;
              cc1101.strobe(value & (~CC1101.SPI_READ_BIT) & (~CC1101.SPI_BURST_BIT));
            } else {
              /* Store address */
              ioRead = CC1101.spiIsRead(value);
              ioWrite = !ioRead;
              cc1101.setLastInstructionWasRead(ioRead);
              ioAddress = value & (~CC1101.SPI_READ_BIT) & (~CC1101.SPI_BURST_BIT);
              if (DEBUG) {
                if (ioAddress == CC1101.CC1101_RXFIFO) {
                } else if (ioAddress == CC1101.CC1101_MARCSTATE) {
                } else if (ioAddress == CC1101.CC1101_RXBYTES) {
                } else if (ioAddress == CC1101.CC1101_TXBYTES) {
                } else {
                  System.out.printf("IO address 0x%04x (" + (ioRead?"read":"write") + ", " + (ioBurst?"burst":"single") + ")\n", ioAddress);
                }
              }
              if (ioRead) {
                expectingDummyWrite = true;
              }
            }
        } else if (address == ADDRESS + RF1AINSTRW) {
            if (expectingDummyWrite) {
                /* Ignore dummy write */
                expectingDummyWrite = false;
                return;
            }
            
            if (!ioRead && !ioWrite) {
                System.out.printf("Error: Unknown data mode for write 0x%04x\n", address);
                return;
            } else if (ioRead) {
                System.out.printf("Error: writing in read mode: 0x%04x\n", address);
                return;
            }
            
            /* Write CC1101 register */
            cc1101.setReg(ioAddress, value);
            if (ioBurst) {
                if (ioAddress != CC1101.CC1101_TXFIFO && ioAddress != CC1101.CC1101_PATABLE) {
                    ioAddress++;
                }
            } else {
                ioWrite = false;
            }
        }
    }

    public int read(int address, boolean word, long cycles) {
        /*if (DEBUG) {
            System.out.printf(this.getName() + ": Read from 0x%04x\n", address);
        }*/

        if (address == ADDRESS + RF1AIV) {
          if (interruptOnCC1101GDO0 && gdo0IsHigh) {
            return 2; /* RF1AIV_RFIFG0 */
          }
          return 0;
        } else if (address == ADDRESS + RF1AIFCTL1) {
            /* XXX TODO: CTL1 always return 0xFF */
            return 0xff;
        } else if (address == ADDRESS + RF1ASTAT1W + 1) {
          /* Marcstate */
          return cc1101.getMarcstate();
        } else if (!word && address == ADDRESS + RF1ASTATB) {
            /* Status byte */
            return cc1101.getStatusByte();
        } else if (address == ADDRESS + RF1ADOUTB) {
            /* Return last received byte from emulated SPI interface */
            
            if (!ioRead && !ioWrite) {
                System.out.printf("Error: Unknown data mode for read 0x%04x\n", address);
                return -1;
            } else if (ioWrite) {
                System.out.printf("Error: reading in write mode: 0x%04x\n", address);
                return -1;
            }

            /* Read CC1101 register */
            int val = cc1101.getReg(ioAddress);

            if (ioBurst) {
                if (ioAddress != CC1101.CC1101_TXFIFO && ioAddress != CC1101.CC1101_PATABLE) {
                    ioAddress++;
                }
            } else {
                ioRead = false;
            }

            return val;
        }
        return -1;
    }

    public void interruptServiced(int vector) {
        if (DEBUG) {
            System.out.println(this.getName() + ": Interrupt services vector: " + vector);
        }
    }
}
