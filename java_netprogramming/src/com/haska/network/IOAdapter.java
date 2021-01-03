package com.haska.network;

import com.haska.network.impl.SocketHandler;

public interface IOAdapter {
    public void onConnected();
    public IOAdapter onAccept();
    public void setSocketHandler(SocketHandler sh);
    public void onRead();
    public void onWrite();
    public void onClose();
}
