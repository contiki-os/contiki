/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package com.thingsquare.cooja.mspsim;

import java.io.File;

import org.apache.log4j.Logger;

import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;
import se.sics.mspsim.platform.ti.CC430Node;

/**
 * @author Fredrik Osterlind
 */
public class CC430Mote extends MspMote {
    private static Logger logger = Logger.getLogger(CC430Mote.class);

    public CC430Node cc430Node = null;

    public CC430Mote(MspMoteType moteType, Simulation sim) {
        super(moteType, sim);
    }

    protected boolean initEmulator(File fileELF) {
        try {
            cc430Node = new CC430Node();
            registry = cc430Node.getRegistry();

            prepareMote(fileELF, cc430Node);
        } catch (Exception e) {
            logger.fatal("Error when creating CC430 mote: ", e);
            return false;
        }
        return true;
    }

    public void idUpdated(int newID) {
        logger.fatal("idUpdated(" + newID + ") ignored");
    }

    public String toString() {
        return "CC430 " + getID();
    }
}
