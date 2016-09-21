/**
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson
 */

package se.sics.mspsim.config;

import java.util.ArrayList;

import se.sics.mspsim.core.ClockSystem;
import se.sics.mspsim.core.GenericUSCI;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.MSP430Config;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.Multiplier32;
import se.sics.mspsim.core.PMM;
import se.sics.mspsim.core.PMMDummy;
import se.sics.mspsim.core.SysReg;
import se.sics.mspsim.core.Timer;
import se.sics.mspsim.core.UnifiedClockSystem;
import se.sics.mspsim.util.Utils;
import se.sics.mspsim.core.RF1A;

public class CC430f5137Config extends MSP430Config {

    // NOTE: this MCU also needs to configure 
    // - positions of all timers (A0, A1, B)
    // - memory configuration
    // - 
    private static final String portConfig[] = {
            "P1=200,IN 00,OUT 02,DIR 04,REN 06,DS 08,SEL 0A,IV_L 0E,IV_H 0F,IES 18,IE 1A,IFG 1C",
            "P2=200,IN 01,OUT 03,DIR 05,REN 07,DS 09,SEL 0B,IV_L 1E,IV_H 1F,IES 19,IE 1B,IFG 1D",
            "P3=220,IN 00,OUT 02,DIR 04,REN 06,DS 08,SEL 0A",
            "P4=220,IN 01,OUT 03,DIR 05,REN 07,DS 09,SEL 0B",
            "P5=240,IN 00,OUT 02,DIR 04,REN 06,DS 08,SEL 0A",
            "P6=240,IN 01,OUT 03,DIR 05,REN 07,DS 09,SEL 0B",
            "P7=260,IN 00,OUT 02,DIR 04,REN 06,DS 08,SEL 0A",
            "P8=260,IN 01,OUT 03,DIR 05,REN 07,DS 09,SEL 0B",
            "P9=280,IN 00,OUT 02,DIR 04,REN 06,DS 08,SEL 0A",
            "P:=280,IN 01,OUT 03,DIR 05,REN 07,DS 09,SEL 0B",
    };
    
    public CC430f5137Config() {
        /* 64 vectors for the MSP430f54xx series */
        maxInterruptVector = 63;
        MSP430XArch = true;
        flashControllerOffset = 0x140;
        sfrOffset = 0x100;
        
        /* configuration for the timers - need to set-up new source maps!!! */
        
        /* XXX Adam: the ints after TimerConfig(X, Y) seem to be the interrupt vectors, i.e. i = (vector_addresss - 0xff80)/2 */
        
        //#define TIMER1_A1_VECTOR    (0x0064) /* 0xFFE4 Timer1_A3 CC1-2, TA1 */
        //#define TIMER1_A0_VECTOR    (0x0066) /* 0xFFE6 Timer1_A3 CC0 */
        //#define DMA_VECTOR          (0x0068) /* 0xFFE8 DMA */
        //#define CC1101_VECTOR       (0x006A) /* 0xFFEA CC1101 Radio Interface */
        //#define TIMER0_A1_VECTOR    (0x006C) /* 0xFFEC Timer0_A5 CC1-4, TA */
        //#define TIMER0_A0_VECTOR    (0x006E) /* 0xFFEE Timer0_A5 CC0 */
  
  
        TimerConfig timerA0 = new TimerConfig(0x6e / 2, 0x6c / 2, 5, 0x340, Timer.TIMER_Bx149, "TimerA0", 0x340 + 0x2e);
        TimerConfig timerA1 = new TimerConfig(0x66 / 2, 0x64 / 2, 3, 0x380, Timer.TIMER_Ax149, "TimerA1", 0x380 + 0x2e);
        
        timerConfig = new TimerConfig[] {timerA0, timerA1};
        
        uartConfig = new UARTConfig[] {
                new UARTConfig("USCI A0", 57, 0x5c0),
                new UARTConfig("USCI B0", 56, 0x5e0),
                new UARTConfig("USCI A1", 46, 0x600),
                new UARTConfig("USCI B1", 45, 0x620),
        };

        /* configure memory */
        infoMemConfig(0x1800, 128 * 4);
        ramConfig(0x1c00, 4 * 1024);
        mainFlashConfig(0x8000, 32 * 1024);
        
        ioMemSize(0x1000); /* 4 KB of IO Memory */
        
        watchdogOffset = 0x15c;
        // bsl, IO, etc at a later stage...
    }

    public int setup(MSP430Core cpu, ArrayList<IOUnit> ioUnits) {
    
        Multiplier32 mp = new Multiplier32(cpu, cpu.memory, 0x4c0);
        cpu.setIORange(0x4c0, 0x2e, mp);

        /* this code should be slightly more generic... and be somewhere else... */
        for (int i = 0, n = uartConfig.length; i < n; i++) {
            GenericUSCI usci = new GenericUSCI(cpu, i, cpu.memory, this);
            /* setup 0 - 1f as IO addresses */
            cpu.setIORange(uartConfig[i].offset, 0x20, usci);
//            System.out.println("Adding IOUnit USCI: " + usci.getName());
            ioUnits.add(usci);
        }
        IOPort last = null;
        ioUnits.add(last = IOPort.parseIOPort(cpu, 47, portConfig[0], last));
        ioUnits.add(last = IOPort.parseIOPort(cpu, 42, portConfig[1], last));
        
        for (int i = 2; i < portConfig.length; i++) {
            ioUnits.add(last = IOPort.parseIOPort(cpu, 0, portConfig[i], last));
        }

		/* XXX: Stub IO units: Sysreg and PMM */
		SysReg sysreg = new SysReg(cpu, cpu.memory);
		cpu.setIORange(SysReg.ADDRESS, SysReg.SIZE, sysreg);
		ioUnits.add(sysreg);

        PMMDummy pmmDummy = new PMMDummy(cpu, cpu.memory, 0x110);
        cpu.setIORange(0x110, PMMDummy.SIZE, pmmDummy);
        ioUnits.add(pmmDummy);
        
		PMM pmm = new PMM(cpu, cpu.memory, 0x120);
		cpu.setIORange(0x120, PMM.SIZE, pmm);
		ioUnits.add(pmm);

		RF1A rf1a = new RF1A(cpu, cpu.memory);
		cpu.setIORange(RF1A.ADDRESS, RF1A.SIZE, rf1a);
		ioUnits.add(rf1a);

		
        return portConfig.length + uartConfig.length;
    }

    public String getAddressAsString(int addr) {
        return Utils.hex20(addr);
    }

    public ClockSystem createClockSystem(MSP430Core cpu, int[] memory, Timer[] timers) {
        return new UnifiedClockSystem(cpu, memory, 0, timers);
    }

}
