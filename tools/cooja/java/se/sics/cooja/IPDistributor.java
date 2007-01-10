/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: IPDistributor.java,v 1.2 2007/01/10 14:57:42 fros4943 Exp $
 */

package se.sics.cooja;

import java.lang.reflect.Constructor;
import java.util.Vector;
import org.apache.log4j.Logger;

/**
 * A IP distributor is used for determining IP addresses of newly created motes.
 * 
 * @see se.sics.cooja.Positioner
 * @author Fredrik Osterlind
 */
public abstract class IPDistributor {
  private static Logger logger = Logger.getLogger(IPDistributor.class);

  /**
   * This method creates an instance of the given class with the given vector as
   * constructor argument. Instead of calling the constructors directly this
   * method may be used.
   * 
   * @param ipDistClass
   *          Class
   * @param newMotes
   *          All motes that later should be assigned IP numbers
   * @return IP distributor instance
   */
  public static final IPDistributor generateIPDistributor(
      Class<? extends IPDistributor> ipDistClass, Vector<Mote> newMotes) {
    try {
      // Generating IP distributor
      Constructor constr = ipDistClass
          .getConstructor(new Class[] { Vector.class });
      return (IPDistributor) constr.newInstance(new Object[] { newMotes });
    } catch (Exception e) {
      logger.fatal("Exception when creating " + ipDistClass + ": " + e);
      return null;
    }
  }

  /**
   * Returns the next mote IP address.
   * 
   * @return IP Address
   */
  public abstract String getNextIPAddress();

}
