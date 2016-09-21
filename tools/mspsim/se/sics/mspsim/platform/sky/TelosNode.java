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
 * $Id: TelosNode.java 304 2008-09-06 20:04:45Z joxe $
 *
 * -----------------------------------------------------------------
 *
 * TelosNode
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date: 2008-09-06 22:04:45 +0200 (Sat, 06 Sep 2008) $
 *           $Revision: 304 $
 */

package se.sics.mspsim.platform.sky;
import java.io.IOException;
import se.sics.mspsim.chip.AT45DB;
import se.sics.mspsim.chip.FileStorage;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.USARTSource;
import se.sics.mspsim.util.ArgumentManager;

/**
 * Emulation of Telos Mote (old version of Sky Node)
 * 
 * TODO: Cleanup the MoteIVNode, SkyNode and TelosNode
 */
public class TelosNode extends MoteIVNode {

  // P4.4 - Output: SPI Flash Chip Select
  public static final int FLASH_RESET = (1<<3);
  public static final int FLASH_CS = (1<<4);
  
  private AT45DB flash;

  /**
   * Creates a new <code>TelosNode</code> instance.
   *
   */
  public TelosNode() {
    super("Telos");
  }

  public AT45DB getFlash() {
    return flash;
  }

  public void setFlash(AT45DB flash) {
    this.flash = flash;
  }

  @Override
  protected void flashWrite(IOPort source, int data) {
    flash.setReset((data & FLASH_RESET) == 0);
    flash.setChipSelect((data & FLASH_CS) == 0);
  }

  // USART Listener
  public void dataReceived(USARTSource source, int data) {
    radio.dataReceived(source, data);
    flash.dataReceived(source, data);
  }

  public void setupNodePorts() {
    super.setupNodePorts();
    if (getFlash() == null) {
        setFlash(new AT45DB(cpu));
    }
    if (flashFile != null) {
      getFlash().setStorage(new FileStorage(flashFile));
    }
  }

  public static void main(String[] args) throws IOException {
    TelosNode node = new TelosNode();
    ArgumentManager config = new ArgumentManager();
    config.handleArguments(args);
    node.setupArgs(config);
  }

}
