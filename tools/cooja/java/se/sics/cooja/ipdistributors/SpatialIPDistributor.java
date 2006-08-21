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
 * $Id: SpatialIPDistributor.java,v 1.1 2006/08/21 12:13:07 fros4943 Exp $
 */

package se.sics.cooja.ipdistributors;
import java.util.Vector;
import se.sics.cooja.*;

/**
 * Generates spatial IP addresses on the form 10.[z-coord].[y-coord].[x-coord].
 * The smallest coordinate in each interval will be mapped onto address 1,
 * and the biggest coordinate onto address 200.
 * Nothing prevents several motes from getting the same IP number.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Spatial (10.x.y.z)")
public class SpatialIPDistributor extends IPDistributor {
  private double lowestX, biggestX, lowestY, biggestY, lowestZ, biggestZ;
  private Vector<String> generatedIPAddresses;

  /**
   * Creates a random IP distributor.
   * @param newMotes All motes which later will be assigned IP numbers.
   */
  public SpatialIPDistributor(Vector<Mote> newMotes) {
    lowestX = newMotes.get(0).getInterfaces().getPosition().getXCoordinate();
    biggestX = newMotes.get(0).getInterfaces().getPosition().getXCoordinate();
    lowestY = newMotes.get(0).getInterfaces().getPosition().getYCoordinate();
    biggestY = newMotes.get(0).getInterfaces().getPosition().getYCoordinate();
    lowestZ = newMotes.get(0).getInterfaces().getPosition().getZCoordinate();
    biggestZ = newMotes.get(0).getInterfaces().getPosition().getZCoordinate();

    for (int i=0; i < newMotes.size(); i++) {
      if (newMotes.get(i).getInterfaces().getPosition().getXCoordinate() < lowestX)
	lowestX = newMotes.get(i).getInterfaces().getPosition().getXCoordinate();
      if (newMotes.get(i).getInterfaces().getPosition().getXCoordinate() > biggestX)
	biggestX = newMotes.get(i).getInterfaces().getPosition().getXCoordinate();

      if (newMotes.get(i).getInterfaces().getPosition().getYCoordinate() < lowestY)
	lowestY = newMotes.get(i).getInterfaces().getPosition().getYCoordinate();
      if (newMotes.get(i).getInterfaces().getPosition().getYCoordinate() > biggestY)
	biggestY = newMotes.get(i).getInterfaces().getPosition().getYCoordinate();

      if (newMotes.get(i).getInterfaces().getPosition().getZCoordinate() < lowestZ)
	lowestZ = newMotes.get(i).getInterfaces().getPosition().getZCoordinate();
      if (newMotes.get(i).getInterfaces().getPosition().getZCoordinate() > biggestZ)
	biggestZ = newMotes.get(i).getInterfaces().getPosition().getZCoordinate();
    }

    generatedIPAddresses = new Vector<String>();
    for (int i=0; i < newMotes.size(); i++) {
      String ipAddress = "10.";
      int partIP;

      // Z coord
      partIP = (int) (1 + 199*(newMotes.get(i).getInterfaces().getPosition().getZCoordinate() - lowestZ) / (biggestZ - lowestZ));
      ipAddress = ipAddress.concat(partIP + ".");

      // Y coord
      partIP = (int) (1 + 199*(newMotes.get(i).getInterfaces().getPosition().getYCoordinate() - lowestY) / (biggestY - lowestY));
      ipAddress = ipAddress.concat(partIP + ".");

      // X coord
      partIP = (int) (1 + 199*(newMotes.get(i).getInterfaces().getPosition().getXCoordinate() - lowestX) / (biggestX - lowestX));
      ipAddress = ipAddress.concat(partIP + "");

      generatedIPAddresses.add(ipAddress);
    }
  }

  public String getNextIPAddress() {
    if (generatedIPAddresses.size() > 0)
      return generatedIPAddresses.remove(0);
    else
      return "0.0.0.0";
  }

}
