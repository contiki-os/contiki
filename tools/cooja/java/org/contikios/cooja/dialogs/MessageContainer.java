package org.contikios.cooja.dialogs;

public class MessageContainer {

    public final int type;
    public final String message;

    public MessageContainer(String message, int type) {
        this.message = message;
        this.type = type;
    }

    @Override
    public String toString() {
        return message;
    }
}
