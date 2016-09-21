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
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author Niclas Finne
 */
public class FileStorage implements Storage {

    private String filename;
    private String currentFilename;
    private RandomAccessFile file;
    private FileChannel fileChannel;
    private FileLock fileLock;
    private long maxSize = 0;

    public FileStorage() {
        this("flash.bin");
    }

    public FileStorage(String filename) {
        this.filename = filename;
    }

    private boolean ensureOpen(boolean write) throws IOException {
        if (fileChannel != null) {
            return true;
        }
        if (!write) {
            File fp = new File(filename);
            if (!fp.exists()) {
                // File does not exist and only trying to read. Delay file creation until first write
                return false;
            }
        }

        // Open flash file for R/W
        if (!openFile(filename)) {
            // Failed to open/lock the specified file. Add a counter and try with next filename.
            Matcher m = Pattern.compile("(.+?)(\\d*)(\\.[^.]+)").matcher(filename);
            if (m.matches()) {
                String baseName = m.group(1);
                String c = m.group(2);
                String extName = m.group(3);
                int count = 1;
                if (c != null && c.length() > 0) {
                    count = Integer.parseInt(c) + 1;
                }
                for (int i = 0; !openFile(baseName + count + extName) && i < 100; i++, count++);
            }
        }
        if (fileLock == null) {
            // Failed to open flash file
            if (write) {
                throw new IOException("failed to open storage file '" + filename + '\'');
            }
            return false;
        }
        return true;
    }

    private boolean openFile(String filename) {
        // Open flash file for R/W
        try {
            currentFilename = filename;
            file = new RandomAccessFile(filename, "rw");
            fileChannel = file.getChannel();
            fileLock = fileChannel.tryLock();
            if (fileLock != null) {
                // The file is now locked for use
//                if (DEBUG) log("using flash file '" + filename + '\'');
                return true;
            }
            fileChannel.close();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            close();
            return false;
        }
    }

    public String getFilename() {
        return filename;
    }

    public void setFilename(String filename) {
        this.filename = filename;
        close();
    }

    public File getCurrentFile() {
        if (file != null) {
            return new File(currentFilename);
        }
        return null;
    }

    @Override
    public int read(long pos, byte[] b) throws IOException {
        return read(pos, b, 0, b.length);
    }

    @Override
    public int read(long pos, byte[] b, int offset, int len) throws IOException {
        if (maxSize > 0 && pos + len > maxSize) {
            throw new IOException("outside storage");
        }
        if (file != null || ensureOpen(false)) {
            file.seek(pos);
            return file.read(b, offset, len);
        }
        Arrays.fill(b, (byte) 0);
        return len;
    }

    @Override
    public void write(long pos, byte[] b) throws IOException {
        write(pos, b, 0, b.length);
    }

    public void write(long pos, byte[] b, int offset, int len) throws IOException {
        if (maxSize > 0 && pos + len > maxSize) {
            throw new IOException("outside storage");
        }
        if (file != null || ensureOpen(true)) {
            file.seek(pos);
            file.write(b, offset, len);
        }
    }

    @Override
    public long getMaxSize() {
        return maxSize;
    }

    @Override
    public void setMaxSize(long size) {
        this.maxSize = size;
        if (size > 0 && file != null) {
            try {
                if (file.length() > size) {
                    file.setLength(size);
                }
            } catch (IOException e) {
                // Ignore
            }
        }
    }

    @Override
    public void close() {
        try {
            file = null;
            if (fileLock != null) {
                fileLock.release();
                fileLock = null;
            }
            if (fileChannel != null) {
                fileChannel.close();
                fileChannel = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public String info() {
        File fp = getCurrentFile();
        StringBuilder sb = new StringBuilder();
        sb.append("FileStorage(");
        if (file != null) {
            try {
                long len = file.length();
                sb.append(len);
            } catch (Exception e) {
                // Ignore
                sb.append('0');
            }
        }
        if (maxSize > 0) {
            sb.append('/').append(maxSize);
        }
        sb.append(" bytes): ");
        if (fp != null) {
            sb.append(fp.getAbsolutePath());
        } else {
            sb.append(filename);
        }
        return sb.toString();
    }
}
