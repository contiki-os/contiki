/**
 * Copyright (c) 2012 Swedish Institute of Computer Science.
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
 */
package se.sics.mspsim.chip;

public class CC2520SPI {

    private final static boolean DEBUG = false;

    private final CC2520 cc2520;
    private final SPIData spiData;
    private final SPICommand[] commands = new SPICommand[256];

    public CC2520SPI(CC2520 cc) {
        /* the SPI commands for CC2520 */
        cc2520 = cc;
        spiData = cc;
        SPICommand[] spiCommands = {
            new SPICommand("SNOP 0 0 0 0 0 0 0 0") {
                public void executeSPICommand() {}
            },
            new SPICommand("IBUFLD 0 0 0 0 0 0 1 0 i i i i i i i i") {
                public void executeSPICommand() {
                    cc2520.instructionBuffer = spiData.getSPIData(1);
                }
            },
            new SPICommand("SIBUFEX 0 0 0 0 0 0 1 1"),
            new SPICommand("SSAMPLECCA 0 0 0 0 0 1 0 0"),
            new SPICommand("SRES 0 0 0 0 1 1 1 1 - - - - - - - -"),
            new SPICommand("MEMRD 0 0 0 1 a a a a a a a a a a a a - - - - - - - - ...") {
                final BitField adr = getBitField("a");
                int cAdr = 0;
                public boolean dataReceived(int data) {
                    /* check if this is first two bytes*/
                    if (spiData.getSPIDataLen() == 2) {
                        cAdr = adr.getValue(spiData);
                    } else if (spiData.getSPIDataLen() > 2){
                        spiData.outputSPI(cc2520.readMemory(cAdr));
                        cAdr = (cAdr + 1) & 0x3ff;
                    }
                    return true;
                }
                public void executeSPICommand() {}
            },
            new SPICommand("MEMWR 0 0 1 0 a a a a a a a a a a a a d d d d d d d d ...") {
                final BitField adr = getBitField("a");
                int cAdr = -1;
                public boolean dataReceived(int data) {
                    /* check if this is first two bytes*/
                    int len = spiData.getSPIDataLen();
//                    int sdata[] = spiData.getSPIData();
                    if (len == 2) {
                        cAdr = adr.getValue(spiData);
//                        System.out.println("SPI BitValue: [" + adr.startBit + " - " +
//                                adr.endBit + "] mask:" + adr.firstMask);
//                        System.out.printf("SPI Data: %02x %02x  => adr:%x\n", sdata[0], sdata[1], cAdr);
                    } else if (len > 2){
                        cc2520.writeMemory(cAdr, data);
                        cAdr = (cAdr + 1) & 0x3ff;
                    }
                    return true;
                }
                public void executeSPICommand() {}
            },
            new SPICommand("RXBUF 0 0 1 1 0 0 0 0 - - - - - - - - ...") {
                public boolean dataReceived(int data) {
                    /* second byte is fifo data instead of status... */
                    if (spiData.getSPIDataLen() > 1) {
                        cc2520.readRXFifo();
                    }
                    return true;
                }
                public void executeSPICommand() {}
            },
            new SPICommand("RXBUFCP 0 0 1 1 1 0 0 0 0 0 0 0 a a a a a a a a a a a a - - - - - - - - ..."),
            new SPICommand("RXBUFMOV 0 0 1 1 0 0 1 p c c c c c c c c 0 0 0 0 a a a a a a a a a a a a"),
            new SPICommand("TXBUF 0 0 1 1 1 0 1 0 d d d d d d d d d d d d d d d d ...") {
                public boolean dataReceived(int data) {
                    if (spiData.getSPIDataLen() > 1) {
                        cc2520.writeTXFIFO(data);
                    }
                    return true;
                }
                public void executeSPICommand() {}
            },
            new SPICommand("TXBUFCP 0 0 1 1 1 1 1 p c c c c c c c c 0 0 0 0 a a a a a a a a a a a a"),
            new SPICommand("RANDOM 0 0 1 1 1 1 0 0 - - - - - - - - - - - - - - - - ..."),
            new SPICommand("SXOSCON 0 1 0 0 0 0 0 0") {
                public void executeSPICommand() {
                    cc2520.startOscillator();
                }
            },
            new SPICommand("STXCAL 0 1 0 0 0 0 0 1"),
            new SPICommand("SRXON 0 1 0 0 0 0 1 0") {
                public void executeSPICommand() {
                    cc2520.rxon();
                }
            },
            new SPICommand("STXON 0 1 0 0 0 0 1 1") {
                public void executeSPICommand() {
                    cc2520.stxon();
                }
            },
            new SPICommand("STXONCCA 0 1 0 0 0 1 0 0") {
                public void executeSPICommand() {
                    cc2520.stxoncca();
                }
            },
            new SPICommand("SRFOFF 0 1 0 0 0 1 0 1") {
                public void executeSPICommand() {
                    cc2520.rxtxoff();
                }
            },
            new SPICommand("SXOSCOFF 0 1 0 0 0 1 1 0") {
                public void executeSPICommand() {
                    cc2520.stopOscillator();
                }
            },
            new SPICommand("SFLUSHRX 0 1 0 0 0 1 1 1") {
                public void executeSPICommand() {
                    cc2520.flushRX();
                }
            },
            new SPICommand("SFLUSHTX 0 1 0 0 1 0 0 0") {
                public void executeSPICommand() {
                    cc2520.flushTX();
                }
            },
            new SPICommand("SACK 0 1 0 0 1 0 0 1") {
                public void executeSPICommand() {
                    cc2520.sack(false);
                }
            },
            new SPICommand("SACKPEND 0 1 0 0 1 0 1 0") {
                public void executeSPICommand() {
                    cc2520.sack(true);
                }
            },
            new SPICommand("SNACK 0 1 0 0 1 0 1 1"),
            new SPICommand("SRXMASKBITSET 0 1 0 0 1 1 0 0"),
            new SPICommand("SRXMASKBITCLR 0 1 0 0 1 1 0 1"),
            new SPICommand("RXMASKAND 0 1 0 0 1 1 1 0 d d d d d d d d d d d d d d d d"),
            new SPICommand("RXMASKOR 0 1 0 0 1 1 1 1 d d d d d d d d d d d d d d d d"),
            new SPICommand("MEMCP 0 1 0 1 0 0 0 p c c c c c c c c a a a a e e e e a a a a a a a a e e e e e e e e"),
            new SPICommand("MEMCPR 0 1 0 1 0 0 1 p c c c c c c c c a a a a e e e e a a a a a a a a e e e e e e e e"),
            new SPICommand("MEMXCP 0 1 0 1 0 1 0 p c c c c c c c c a a a a e e e e a a a a a a a a e e e e e e e e"),
            new SPICommand("MEMXWR 0 1 0 1 0 1 1 0 0 0 0 0 a a a a a a a a a a a a d d d d d d d d ..."),
            new SPICommand("BCLR 0 1 0 1 1 0 0 0 a a a a a b b b"),
            new SPICommand("BSET 0 1 0 1 1 0 0 1 a a a a a b b b"),
            new SPICommand("CTR/UCTR 0 1 1 0 0 0 0 p k k k k k k k k 0 c c c c c c c n n n n n n n n a a a a e e e e a a a a a a a a e e e e e e e e"),
            new SPICommand("CBCMAC 0 1 1 0 0 1 0 p k k k k k k k k 0 c c c c c c c a a a a e e e e a a a a a a a a e e e e e e e e 0 0 0 0 0 mmm"),
            new SPICommand("UCBCMAC 0 1 1 0 0 1 1 p k k k k k k k k 0 c c c c c c c 0 0 0 0 a a a a a a a a a a a a 0 0 0 0 0 mmm"),
            new SPICommand("CCM 0 1 1 0 1 0 0 p k k k k k k k k 0 c c c c c c c n n n n n n n n a a a a e e e e a a a a a a a a e e e e e e e e 0 f f f f f f f 0 0 0 0 0 0 mm"),
            new SPICommand("UCCM 0 1 1 0 1 0 1 p k k k k k k k k 0 c c c c c c c n n n n n n n n a a a a e e e e a a a a a a a a e e e e e e e e 0 f f f f f f f 0 0 0 0 0 0 mm"),
            new SPICommand("ECB 0 1 1 1 0 0 0 p k k k k k k k k c c c c a a a a a a a a a a a a 0 0 0 0 e e e e e e e e e e e e"),
            new SPICommand("ECBO 0 1 1 1 0 0 1 p k k k k k k k k c c c c a a a a a a a a a a a a"),
            new SPICommand("ECBX 0 1 1 1 0 1 0 p k k k k k k k k c c c c a a a a a a a a a a a a 0 0 0 0 e e e e e e e e e e e e"),
            new SPICommand("INC 0 1 1 1 1 0 0 p 0 0 c c a a a a a a a a a a a a"),
            new SPICommand("ABORT 0 1 1 1 1 1 1 1 0 0 0 0 0 0 c c"),
            new SPICommand("REGRD 1 0 a a a a a a - - - - - - - - ...") {
                final BitField adr = getBitField("a");
                int cAdr = 0;
                public boolean dataReceived(int data) {
                    /* check if this is first byte*/
                    if (spiData.getSPIDataLen() == 1) {
                        cAdr = adr.getValue(spiData);
                    } else {
                        spiData.outputSPI(cc2520.readMemory(cAdr));
                        cAdr = (cAdr + 1) & 0x7f;
                    }
                    return true;
                }
                public void executeSPICommand() {}
            },
            new SPICommand("REGWR 1 1 a a a a a a d d d d d d d d ...") {
                final BitField adr = getBitField("a");
                int cAdr = 0;
                public boolean dataReceived(int data) {
                    /* check if this is first byte*/
                    if (spiData.getSPIDataLen() == 1) {
                        cAdr = adr.getValue(spiData);
                    } else {
                        cc2520.writeMemory(cAdr, data);
                        cAdr = (cAdr + 1) & 0x7f;
                    }
                    return true;
                }
                public void executeSPICommand() {}
            }
        };

        /* set up the commands */
        for (SPICommand c : spiCommands) {
            int maxv = 1 << (8 - c.bitCount);
            int v = c.value;
            /* populate an array with the values for quick decoding */
            for (int j = 0; j < maxv; j++) {
                if (DEBUG) System.out.printf(c.name + " =>  Value: %x\n", (v + j));
                if (commands[v + j] != null) {
                    throw new IllegalStateException("ERROR: command already registered: " + commands[v + j].name);
                }
                commands[v + j] = c;
            }
        }
    }

    SPICommand getCommand(int cmd) {
        if (cmd < 256) {
            return commands[cmd];
        }
        return null;
    }

//   public static void main(String[] args) {
//       CC2520SPI spi = new CC2520SPI(null);
//       SPICommand cmd = spi.getCommand(0xff);
//       /* commands that take infinite number of bytes have the bitfield ... */
//       System.out.println("Has ... => " + cmd.getBitField("..."));
//
//       if (args.length > 0 && "all".equals(args[0])) {
//           for (int i = 0, n = spi.commands.length; i < n; i++) {
//               SPICommand c = spi.commands[i];
//               System.out.print(se.sics.mspsim.util.Utils.binary8(i));
//               if (c != null) {
//                   System.out.printf(": %14s %s/%s (%d)\n", c.name,
//                           se.sics.mspsim.util.Utils.binary8(c.value),
//                           se.sics.mspsim.util.Utils.binary8(c.mask),
//                           c.bitCount);
//               } else {
//                   System.out.println(":");
//               }
//           }
//       }
//   }

}
