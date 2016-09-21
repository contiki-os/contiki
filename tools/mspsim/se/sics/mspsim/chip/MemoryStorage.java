/**
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 */
package se.sics.mspsim.chip;

import java.io.IOException;
import java.util.Arrays;

/**
 * @author Niclas Finne
 */
public class MemoryStorage implements Storage {

    private byte[] data;
    private int maxSize;

    private void ensureCapacity(int size) throws IOException {
        if (data == null) {
            data = new byte[size];
        } else if (data.length < size) {
            data = Arrays.copyOf(data, size);
        }
    }

    @Override
    public int read(long pos, byte[] b) throws IOException {
        return read(pos, b, 0, b.length);
    }

    @Override
    public int read(long storagePos, byte[] buffer, int offset, int len) throws IOException {
        if (maxSize > 0 && storagePos + len > maxSize) {
            throw new IOException("outside storage");
        }
        if (data == null) {
            Arrays.fill(buffer, offset, offset + len, (byte)0);
        } else {
            int pos = (int) storagePos;
            if (pos + len > data.length) {
                System.arraycopy(data, pos, buffer, offset, data.length - pos);
                Arrays.fill(buffer, offset + data.length - pos, offset + len, (byte) 0);
            } else {
                System.arraycopy(data, pos, buffer, offset, len);
            }
        }
        return len;
    }

    @Override
    public void write(long storagePos, byte[] buffer) throws IOException {
        write(storagePos, buffer, 0, buffer.length);
    }

    @Override
    public void write(long storagePos, byte[] buffer, int offset, int len) throws IOException {
        int pos = (int) storagePos;
        if (maxSize > 0 && pos + len > maxSize) {
            throw new IOException("outside storage");
        }
        ensureCapacity(pos + len);
        System.arraycopy(buffer, offset, data, pos, len);
    }

    @Override
    public long getMaxSize() {
        return maxSize;
    }

    @Override
    public void setMaxSize(long size) {
        this.maxSize = (int) size;
        if (maxSize > 0 && data != null && data.length > maxSize) {
            data = Arrays.copyOf(data, maxSize);
        }
    }

    @Override
    public void close() {
        // Nothing to close
    }

    @Override
    public String info() {
        if (maxSize > 0) {
            return "MemoryStorage(" + data.length + "/" + maxSize + " bytes)";
        }
        return "MemoryStorage(" + data.length + " bytes allocated)";
    }
}
