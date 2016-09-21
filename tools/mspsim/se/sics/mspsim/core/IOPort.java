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
 * IOPort
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package  se.sics.mspsim.core;
import java.util.Arrays;

import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.Utils;

public class IOPort extends IOUnit {

    public enum PinState { LOW, HI };

    public enum PortReg {IN, OUT, DIR, SEL, SEL2, IFG, IES, IE, REN, DS, IV_L, IV_H};

    /* portmaps for 1611 */
    private static final PortReg[] PORTMAP_INTERRUPT = 
        {PortReg.IN, PortReg.OUT, PortReg.DIR, PortReg.IFG, PortReg.IES, PortReg.IE, PortReg.SEL}; 
    private static final PortReg[] PORTMAP_NO_INTERRUPT = 
        {PortReg.IN, PortReg.OUT, PortReg.DIR, PortReg.SEL};

    private final int port;
    private final int interrupt;

    // External pin state!
    private final PinState pinState[] = new PinState[8];

    private final PortReg[] portMap;

    private PortListener portListener = null;

    /* Registers for Digital I/O */

    private int in;
    private int out;
    private int dir;
    private int sel;
    private int sel2;
    private int ie;
    private int ifg;
    private int ies; /* edge select */
    private int ren;
    private int ds;

    private int iv; /* low / high */

    private Timer[] timerCapture = new Timer[8];

    private IOPort ioPair;
    
    /**
     * Creates a new <code>IOPort</code> instance.
     *
     */
    public IOPort(MSP430Core cpu, int port, int interrupt, int[] memory, int offset) {
        this(cpu, port, interrupt, memory, offset,
                interrupt == 0 ? PORTMAP_NO_INTERRUPT : PORTMAP_INTERRUPT);
    }

    /* Create an IOPort with a special PortMap */
    public IOPort(MSP430Core cpu, int port,
            int interrupt, int[] memory, int offset, PortReg[] portMap) {
        super("P" + port, "Port " + port, cpu, memory, offset);
        this.port = port;
        this.interrupt = interrupt;
        this.ie = 0;
        this.ifg = 0;
        this.portMap = portMap;

//        System.out.println("Port " + port + " interrupt vector: " + interrupt);
        /* register all the registers from the port-map */
        for (int i = 0; i < portMap.length; i++) {
            if (portMap[i] != null) {
//                System.out.println("  P" + port + portMap[i] + " at " + Utils.hex16(offset + i));
                cpu.setIORange(offset + i, 1, this);
            }
        }
    }
    
    public static IOPort parseIOPort(MSP430Core cpu, int interrupt, String specification, IOPort last) {
        /* Specification = Px=Offset,REG Off, ... */
        String[] specs = specification.split(",");
        int port = specs[0].charAt(1) - '0';
        int offset = Integer.parseInt(specs[0].substring(3), 16);

        PortReg[] portMap = new PortReg[0x20]; /* Worst case port-map */
        int highest = -1;
        for (int i = 1; i < specs.length; i++) {
            String[] preg = specs[i].split(" ");
            PortReg pr = PortReg.valueOf(preg[0]);
            int offs = Integer.parseInt(preg[1], 16);
            if (offs >= portMap.length) {
                portMap = Arrays.copyOf(portMap, offs + 1);
            }
            portMap[offs] = pr;
            if (offs > highest) {
                highest = offs;
            }
        }
        if (highest + 1 < portMap.length) {
            portMap = Arrays.copyOf(portMap, highest + 1);
        }
        IOPort newPort = new IOPort(cpu, port, interrupt, cpu.memory, offset, portMap);
        if (last != null && offset == last.offset && offset > 0) {
            // This port is a pair with previous port to allow 16 bits writes
            last.ioPair = newPort;
        }
        return newPort;
    }

    public int getPort() {
        return port;
    }

    public int getIn() {
        return in;
    }

    public int getOut() {
        return out;
    }

    public int getDirection() {
        return dir;
    }

    public int getSelect() {
        return sel;
    }

    public synchronized void addPortListener(PortListener newListener) {
        portListener = PortListenerProxy.addPortListener(portListener, newListener);
    }

    public synchronized void removePortListener(PortListener oldListener) {
        portListener = PortListenerProxy.removePortListener(portListener, oldListener);
    }

    public void setTimerCapture(Timer timer, int pin) {
        if (DEBUG) {
            log("Setting timer capture for pin: " + pin);
        }
        timerCapture[pin] = timer;
    }

    private void updateIV() {
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
        cpu.flagInterrupt(interrupt, this, (ifg & ie) > 0);
    }

    public int getRegister(PortReg register) {
        switch(register) {
        case DIR:
            return dir;
        case IE:
            return ie;
        case IES:
            return ies;
        case IFG:
            return ifg;
        case IN:
            return in;
        case IV_H:
            return (iv >> 8) & 0xff;
        case IV_L:
            return iv & 0xff;
        case OUT:
            return out;
        case REN:
            return ren;
        case DS:
            return ds;
        case SEL:
            return sel;
        case SEL2:
            return sel2;
        }
        return 0;
    }

    /* only byte access!!! */
    private int readPort(PortReg function, long cycles) {
        switch(function) {
        case OUT:
            return out;
        case IN:
            return in;
        case DIR:
            return dir;
        case REN:
            return ren;
        case IFG:
            return ifg;
        case IE:
            return ie;
        case IES:
            return ies;
        case SEL:
            return sel;
        case SEL2:
            return sel2;
        case DS:
            return ds;
        case IV_L: {
            int v = iv & 0xff;
            // Clear highest interrupt
            if (iv != 0) {
                if (iv > 1 && iv < 17) {
                    ifg &= ~(1 << ((iv - 2) / 2));
                }
                updateIV();
            }
            return v;
        }
        case IV_H: {
            int v = iv >> 8;
            return v;
        }
        }
        /* default is zero ??? */
        return 0;
    }

    private void writePort(PortReg function, int data, long cycles) {
        switch(function) {
        case OUT: {
            out = data;
            PortListener listener = portListener;
            if (listener != null) {
            	listener.portWrite(this, out | (~dir) & 0xff);
            }
            break;
        }
        case IN:
            logw(WarningType.ILLEGAL_IO_WRITE, "WARNING: writing to read-only " + getID() + "IN");
            throw new EmulationException("Writing to read-only " + getID() + "IN");
            //          in = data;
        case DIR: {
            dir = data;
            PortListener listener = portListener;
            if (listener != null) {
                // Any output configured pin (pin-bit = 0) should have 1 here?! 
                //              if (name.equals("1"))
                //                System.out.println(getName() + " write to IOPort via DIR reg: " + Utils.hex8(data));
                listener.portWrite(this, out | (~dir) & 0xff);
            }
            break;
        }
        case REN:
            ren = data;
            break;
        case IFG:
            if (DEBUG) {
                log("Setting IFlag: " + data);
            }
            ifg = data;
            updateIV();
            break;
        case IE:
            ie = data;
            if (DEBUG) {
                log("Setting IE: " + data);
            }
            cpu.flagInterrupt(interrupt, this, (ifg & ie) > 0);
            break;
        case IES:
            ies = data;
            break;
        case SEL:
            sel = data;
            break;
        case SEL2:
            sel2 = data;
            break;
        case DS:
            ds = data;
            break;
        case IV_L:
            // IV can not be written but highest interrupt should be cleared
            // on access.
            if (iv != 0) {
                if (iv > 1 && iv < 17) {
                    ifg &= ~(1 << ((iv - 2) / 2));
                }
                updateIV();
            }
            break;
        case IV_H:
            // IV_H can not be written
            break;
        }
    }


    public int read(int address, boolean word, long cycles) {
        PortReg reg = portMap[address - offset];
        /* only byte read allowed if not having an ioPair */
        if (word && reg == PortReg.IV_L) {
            /* Read hi first then low => update on low!!! */
            return (readPort(PortReg.IV_H, cycles) << 8) | readPort(reg, cycles);
        } else if (word && ioPair != null) {
            /* read same function from both */
            return readPort(reg, cycles) | (ioPair.readPort(reg, cycles) << 8);
        }
        /* NOTE: read of PIV might be wrong here - might be word access on IV? */
        return readPort(reg, cycles);
    }


    public void write(int address, int data, boolean word, long cycles) {
        int iAddress = address - offset;
        PortReg fun = portMap[iAddress];
        if (DEBUG) {
            log("Writing to " + getID() + fun +
                    " ($" + Utils.hex(address, 2) +
                    ") => $" + Utils.hex(data, 2) + "=#" +
                    Utils.binary8(data) + (word ? " (word)" : ""));
        }

        /* only byte write - need to convert any word write here... */
        if (word && ioPair != null) {
            writePort(fun, data & 0xff, cycles);
            ioPair.writePort(fun, data >> 8, cycles);
        } else {
            writePort(fun, data, cycles);
        }
    }

    public void interruptServiced(int vector) {
    }

    // for HW to set hi/low on the pins...
    public void setPinState(int pin, PinState state) {
        if (pinState[pin] != state) {
            pinState[pin] = state;
            int bit = 1 << pin;
            if (state == PinState.HI) {
                in |= bit;
            } else {
                in &= ~bit;
            }
            if (interrupt > 0) {
                if ((ies & bit) == 0) {
                    // LO/HI transition
                    if (state == PinState.HI) {
                        ifg |= bit;
                        if (DEBUG) {
                            log("Flagging interrupt (HI): " + bit);
                        }
                        updateIV();
                    }
                } else {
                    // HI/LO transition
                    if (state == PinState.LOW) {
                        ifg |= bit;
                        if (DEBUG) {
                            log("Flagging interrupt (LOW): " + bit);
                        }
                        updateIV();
                    }
                }
            }

            if (timerCapture[pin] != null) {
                /* should not be pin and 0 here
                 * pin might need configuration and 0 can maybe also be 1? 
                 */
                //        if (DEBUG) log("Notifying timer of changed pin value");
                timerCapture[pin].capture(pin, 0, state);
            }

        }
    }

    public void reset(int type) {
        int oldValue = out | (~dir) & 0xff;

        Arrays.fill(pinState, PinState.LOW);
        in = 0;
        dir = 0;
        ren = 0;
        ifg = 0;
        ie = 0;
        iv = 0;
        cpu.flagInterrupt(interrupt, this, (ifg & ie) > 0);

        PortListener listener = portListener;
        int newValue = out | (~dir) & 0xff;
        if (oldValue != newValue && listener != null) {
            listener.portWrite(this, newValue);
        }
    }

    public String info() {
        StringBuilder sb = new StringBuilder();
        sb.append(" $").append(Utils.hex(offset, 2)).append(':');
        for (int i = 0, n = portMap.length; i < n; i++) {
            PortReg reg = portMap[i];
            if (reg != null) {
                sb.append(' ').append(reg).append("($").append(Utils.hex(i, 2)).append("):$")
                .append(Utils.hex(0, 2));
            }
        }
        return sb.toString();
    }

}
