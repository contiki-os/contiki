package org.contikios.cooja.dialogs;

public class MessageListText implements MessageList {

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

}
