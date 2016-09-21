package se.sics.mspsim.util;

import se.sics.mspsim.core.EmulationException;

public class ArrayFIFO {

    public static final boolean STRICT = false;
    
    private final int memory[];
    private final int start;
    private final int size;
    private final String name;
    
    private int readPos = 0;
    private int writePos = 0;
    private int len = 0;

    private int markWritePos;
    
    public ArrayFIFO(String name, int[] mem, int start, int size) {
        memory = mem;
        this.name = name;
        this.start = start;
        this.size = size;
        len = 0;
    }

    
    public boolean write(int data) {
        memory[start + writePos++] = data;
        if (writePos >= size) {
            writePos = 0;
        }
        len++;
        return len == size;
    }
    
    public boolean isEmpty() {
        return len == 0;
    }
    
    public boolean isFull() {
        return len == size;
    }

    /* peek data as if it would be read, but leave it on the fifo */
    public int peek(int index) {
        int pos = readPos + index;
        if (pos < 0) {
            pos += size;
        } else if (pos >= size) {
            pos -= size;
        }        
        int v = memory[start + pos];
        return v & 0xff;
    }
    
    public int read() {
        if (len <= 0) {
            if (STRICT) {
                throw new EmulationException("Reading from empty FIFO");
            } else {
                /*System.out.println("Reading from empty FIFO");*/
                return 0;
            }
        }
        int v = memory[start + readPos++];
        if (readPos >= size) {
            readPos = 0;
        }
        len--;
        return v & 0xff;
    }
    
    /* get data relative the write position (typically negative input values */
    public int get(int index) {
        int pos = writePos + index;
        if (pos < 0) {
            pos += size;
        } else if (pos >= size) {
            pos -= size;
        }
        return memory[start + pos] & 0xff;
    }

    /* set data relative the write position (typically negative input values */
    public void set(int index, int data) {
        int pos = writePos + index;
        if (pos < 0) {
            pos += size;
        } else if (pos >= size) {
            pos -= size;
        }
        memory[start + pos] = data & 0xff;
    }

    
    public int length() {
        return len;
    }
    
    public void reset() {
        writePos = 0;
        readPos = 0;
        len = 0;
    }

    /* for marking a write pos - needed for supporting dropping non-matching packets */
    public void mark() {
        markWritePos = writePos;
    }
    
    public void restore() {
        writePos = markWritePos;
        len = writePos - readPos;
        if (len < 0) len += size;
    }


    public String stateToString() {
        return name + " len: " + len + " rpos: " + readPos + " wpos: " + writePos;
    }


    public boolean tailEquals(int[] data, int offset, int len) {
        return tailEquals(data, offset, len, 0);
    }


    public boolean tailEquals(int[] data, int offset, int len, int ignore) {
        for (int i = 0; i < len; i++) {
            if ((data[offset + i] & 0xff) != get(i - len - ignore)) {
                return false;
            }
        }
        return true;
    }
}
