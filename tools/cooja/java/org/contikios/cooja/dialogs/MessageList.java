package org.contikios.cooja.dialogs;

import java.io.OutputStream;

public interface MessageList {

    public static final int NORMAL = 0;
    public static final int WARNING = 1;
    public static final int ERROR = 2;
    
    public void addMessage(String string, int normal);

    public MessageContainer[] getMessages();

    public void clearMessages();

    public void addMessage(String string);

    public OutputStream getInputStream(int type);

}
