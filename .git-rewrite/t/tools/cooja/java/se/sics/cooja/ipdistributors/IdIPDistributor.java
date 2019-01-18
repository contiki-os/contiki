/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: IdIPDistributor.java,v 1.1 2006/08/21 12:13:06 fros4943 Exp $
 */

package se.sics.cooja.ipdistributors;
import java.util.Vector;
import se.sics.cooja.*;

/**
 * Generates IP addresses on the form 10.[id/256 mod 256*256].[id mod 256].1.
 * 
 * Observe!
 * - ID must be set before this is called (otherwise IP=0.0.0.0).
 * - Only supports 256*256 motes, (IPs will wrap if above).
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("From ID (10.id.id.1)")
public class IdIPDistributor extends IPDistributor {
  private Vector<String> generatedIPAddresses;

  /**
   * Creates a Id IP distributor.
   * @param newMotes All motes which later will be assigned IP numbers.
   */
  public IdIPDistributor(Vector<Mote> newMotes) {
    generatedIPAddresses = new Vector<String>();

    for (int i=0; i < newMotes.size(); i++) {
      if (newMotes.get(i).getInterfaces().getMoteID() != null) {
        int moteId = newMotes.get(i).getInterfaces().getMoteID().getMoteID();
        generatedIPAddresses.add("10." + 
            (moteId / 256 % (256*256))
            + "." + 
            (moteId % 256)
            + ".1");
      } else
        generatedIPAddresses.add("0.0.0.0");
    }

  }

  public String getNextIPAddress() {
    if (generatedIPAddresses.size() > 0)
      return generatedIPAddresses.remove(0);
    else
      return "0.0.0.0";
  }

}
