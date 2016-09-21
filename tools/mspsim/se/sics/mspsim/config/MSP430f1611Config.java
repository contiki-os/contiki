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

import se.sics.mspsim.core.ADC12;
import se.sics.mspsim.core.DMA;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.InterruptMultiplexer;
import se.sics.mspsim.core.MSP430Config;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.Multiplier;
import se.sics.mspsim.core.Timer;
import se.sics.mspsim.core.USART;

public class MSP430f1611Config extends MSP430Config {

    
    public MSP430f1611Config() {
        maxInterruptVector = 15;

        /* configuration for the timers */
        TimerConfig timerA = new TimerConfig(6, 5, 3, 0x160, Timer.TIMER_Ax149, "TimerA", Timer.TAIV);
        TimerConfig timerB = new TimerConfig(13, 12, 7, 0x180, Timer.TIMER_Bx149, "TimerB", Timer.TBIV);
 
        timerConfig = new TimerConfig[] {timerA, timerB};
        
        /* configure memory */
        infoMemConfig(0x1000, 128 * 2);
        mainFlashConfig(0x4000, 48 * 1024);
        ramConfig(0x1100, 10 * 1024);
        ramMirrorConfig(0x200, 2 * 1024, 0x1100);
    }
    

    public int setup(MSP430Core cpu, ArrayList<IOUnit> ioUnits) {
        USART usart0 = new USART(cpu, 0, cpu.memory, 0x70);
        USART usart1 = new USART(cpu, 1, cpu.memory, 0x78);
        cpu.setIORange(0x70, 8, usart0);
        cpu.setIORange(0x78, 8, usart1);

        Multiplier mp = new Multiplier(cpu, cpu.memory, 0);
        cpu.setIORange(0x130, 0x0f, mp);

        // Usarts
        ioUnits.add(usart0);
        ioUnits.add(usart1);

        DMA dma = new DMA("dma", cpu, cpu.memory, 0);
        cpu.setIORange(0x1e0, 24, dma);

        /* DMA Ctl */
        cpu.setIORange(0x122, 1, dma);
        cpu.setIORange(0x124, 1, dma);

        /* configure the DMA */
        dma.setDMATrigger(DMA.URXIFG0, usart0, 0);
        dma.setDMATrigger(DMA.UTXIFG0, usart0, 1);
        dma.setDMATrigger(DMA.URXIFG1, usart1, 0);
        dma.setDMATrigger(DMA.UTXIFG1, usart1, 1);
        dma.setInterruptMultiplexer(new InterruptMultiplexer(cpu, 0));

        ioUnits.add(dma);
        
        // Add port 1,2 with interrupt capability!
        // IOPorts will add themselves to the CPU
        ioUnits.add(new IOPort(cpu, 1, 4, cpu.memory, 0x20));
        ioUnits.add(new IOPort(cpu, 2, 1, cpu.memory, 0x28));

        // Add port 3,4 & 5,6
        for (int i = 0, n = 2; i < n; i++) {
            ioUnits.add(new IOPort(cpu, (3 + i), 0, cpu.memory, 0x18 + i * 4));
            ioUnits.add(new IOPort(cpu, (5 + i), 0, cpu.memory, 0x30 + i * 4));
        }

        ADC12 adc12 = new ADC12(cpu);
        ioUnits.add(adc12);
        cpu.setIORange(0x080, 16, adc12);
        cpu.setIORange(0x140, 16, adc12);
        cpu.setIORange(0x150, 16, adc12);
        cpu.setIORange(0x1a0,  8, adc12);

        return 3 + 6;
    }
}
