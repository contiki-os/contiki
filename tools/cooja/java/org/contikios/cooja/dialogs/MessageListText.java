package org.contikios.cooja.dialogs;

import java.io.OutputStream;

public class MessageListText implements MessageList {

    public MessageListText() {
    }
    
    @Override
    public void addMessage(String string, int type) {
        System.out.println("Message:" + string);
    }

    @Override
    public MessageContainer[] getMessages() {
        // TODO Auto-generated method stub
        return new MessageContainer[0];
    }

    @Override
    public void clearMessages() {
        // TODO Auto-generated method stub
    }

    @Override
    public void addMessage(String string) {
        // TODO Auto-generated method stub
        addMessage(string, MessageList.NORMAL);
    }

    @Override
    public OutputStream getInputStream(int type) {
        // TODO Auto-generated method stub
        return System.out;
    }

    
    
}
