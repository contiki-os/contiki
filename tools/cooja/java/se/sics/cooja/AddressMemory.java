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
 * $Id: AddressMemory.java,v 1.2 2008/02/11 14:02:29 fros4943 Exp $
 */

package se.sics.cooja;

public interface AddressMemory {

  /**
   * @return All variable names known and residing in this memory
   */
  public String[] getVariableNames();

  /**
   * Checks if given variable exists in memory.
   *
   * @param varName Variable name
   * @return True if variable exists, false otherwise
   */
  public boolean variableExists(String varName);

  /**
   * Returns address of variable with given name.
   *
   * @param varName Variable name
   * @return Variable address
   * @throws UnknownVariableException Variable does not exist
   */
  public int getVariableAddress(String varName) throws UnknownVariableException;

  /**
   * Returns a value of the byte variable with the given name.
   *
   * @param varName Name of byte variable
   * @return Value of byte variable
   * @throws UnknownVariableException Variable does not exist
   */
  public byte getByteValueOf(String varName) throws UnknownVariableException;

  /**
   * Set byte value of variable with given name.
   *
   * @param varName Name of byte variable
   * @param newVal New value of byte
   * @throws UnknownVariableException Variable does not exist
   */
  public void setByteValueOf(String varName, byte newVal) throws UnknownVariableException;

  /**
   * Returns byte array of given length and with the given name.
   *
   * @param varName Name of array
   * @param length Length of array
   * @return Data of array
   * @throws UnknownVariableException Variable does not exist
   */
  public byte[] getByteArray(String varName, int length) throws UnknownVariableException;

  /**
   * Set byte array of the variable with the given name.
   *
   * @param varName Name of array
   * @param data New data of array
   * @throws UnknownVariableException Variable does not exist
   */
  public void setByteArray(String varName, byte[] data) throws UnknownVariableException;

  /**
   * @return Number of bytes in an integer
   */
  public int getIntegerLength();

  /**
   * Returns a value of the integer variable with the given name.
   *
   * @param varName Name of integer variable
   * @return Value of integer variable
   * @throws UnknownVariableException Variable does not exist
   */
  public int getIntValueOf(String varName) throws UnknownVariableException;

  /**
   * Set integer value of variable with given name.
   *
   * @param varName Name of integer variable
   * @param newVal New integer value
   * @throws UnknownVariableException Variable does not exist
   */
  public void setIntValueOf(String varName, int newVal) throws UnknownVariableException;

  /**
   * Unknown variable name exception.
   */
  public class UnknownVariableException extends RuntimeException {
    public UnknownVariableException(String varName) {
      super("Unknown variable name: " + varName);
    }
  }

}
