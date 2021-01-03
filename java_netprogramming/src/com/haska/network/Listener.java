package com.haska.network;

import java.util.EventListener;

public interface Listener extends EventListener {
    public void process(CustomEventObject eo);
}
