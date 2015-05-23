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

package org.contikios.cooja.mspmote.interfaces;

import java.io.IOException;
import org.contikios.coffee.CoffeeConfiguration;
import org.contikios.coffee.CoffeeImage;
import se.sics.mspsim.chip.MemoryStorage;

public class CoojaExternalFlash extends MemoryStorage implements CoffeeImage {

    private final String target;
    private CoffeeConfiguration coffeeConfiguration;

    public CoojaExternalFlash(String target) {
        this.target = target;
    }

    /**
     * XXX Coffee specific: uses start offset
     * @see org.contikios.coffee.CoffeeImage#erase(int, int)
     */
    public void erase(int size, int offset) throws IOException {
        byte[] buffer = new byte[size];
        write(getConfiguration().startOffset + offset, buffer);
    }

    /**
     * XXX Coffee specific: uses start offset
     * @see org.contikios.coffee.CoffeeImage#getConfiguration()
     */
    public CoffeeConfiguration getConfiguration() {
        if (coffeeConfiguration == null) {
            try {
                coffeeConfiguration = new CoffeeConfiguration(target + ".properties");
            } catch (Exception e) {
                // Failed to retrieve coffee configuration
                throw new IllegalStateException("no Coffee configuration available for platform " + target);
            }
        }
        return coffeeConfiguration;
    }

    /**
     * XXX Coffee specific: uses start offset
     * @see org.contikios.coffee.CoffeeImage#read(byte[], int, int)
     */
    public void read(byte[] bytes, int size, int offset) throws IOException {
        read(getConfiguration().startOffset + offset, bytes, 0, size);
    }

    /**
     * XXX Coffee specific: uses start offset
     * @see org.contikios.coffee.CoffeeImage#write(byte[], int, int)
     */
    public void write(byte[] bytes, int size, int offset) throws IOException {
        read(getConfiguration().startOffset + offset, bytes, 0, size);
    }

}
