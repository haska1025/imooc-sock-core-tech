package com.haska.network.impl;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

public class Poller {
    private Selector nio_selector_;
    
    public void start() throws IOException{
    	nio_selector_ = Selector.open();   
    }
    
    public SelectionKey register(SocketHandler sh, int ops) throws ClosedChannelException{
    	return sh.getSocketChannel().register(nio_selector_, ops, sh);  
    }
    public void poll() throws IOException{
		int readyChannels = nio_selector_.select();

		if (readyChannels == 0)
			return;

		Set<SelectionKey> selectedKeys = nio_selector_.selectedKeys();

		Iterator<SelectionKey> keyIterator = selectedKeys.iterator();

		while (keyIterator.hasNext()) {
			SelectionKey key = keyIterator.next();
			SocketHandler sh = (SocketHandler) key.attachment();
			if (sh != null) {
				if (key.isAcceptable()) {
					// a connection was accepted by a ServerSocketChannel.
					sh.handle_accept();
				} else if (key.isConnectable()) {
					// a connection was established with a remote server.
					sh.handle_connected();
				} else if (key.isReadable()) {
					// a channel is ready for reading
					sh.handle_read();
				} else if (key.isWritable()) {
					// a channel is ready for writing
					sh.handle_write();
				}
			}
			keyIterator.remove();
		}
	}
    
    public void wakeup(){
    	nio_selector_.wakeup();    	
    }
    
    public void close() throws IOException{
    	this.nio_selector_.close();
    }
}
