/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 *
 */

package org.contikios.cooja.mspmote;
import java.io.File;
import org.apache.log4j.Logger;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.mspmote.interfaces.CoojaM25P80;
import se.sics.mspsim.platform.z1.Z1Node;

/**
 * @author Fredrik Osterlind, Niclas Finne
 */
public class Z1Mote extends MspMote {

    private static Logger logger = Logger.getLogger(Z1Mote.class);

    public Z1Mote(MspMoteType moteType, Simulation sim) {
        super(moteType, sim);
    }

    @Override
    protected boolean initEmulator(File fileELF) {
        try {
            Z1Node z1Node = new Z1Node();
            registry = z1Node.getRegistry();
            z1Node.setFlash(new CoojaM25P80(z1Node.getCPU()));

            prepareMote(fileELF, z1Node);
        } catch (Exception e) {
            logger.fatal("Error when creating Z1 mote: ", e);
            return false;
        }
        return true;
    }

    @Override
    public String toString() {
        return "Z1 " + getID();
    }

}
