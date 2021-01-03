package com.haska.network.impl;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;

import com.haska.network.IOAdapter;

public class TcpHandler extends SocketHandler {
    public TcpHandler(IOThread io_thr, IOAdapter io_adp){
    	super(io_thr, io_adp);
    }
	public boolean connect(String hostname, int port) throws IOException{		
		sc_ = SocketChannel.open();
		sc_.configureBlocking(false);
		if (sc_.connect(new InetSocketAddress(hostname, port))){
			return true;
		}
		
		// Register non-blocking connect events
		registerEvent(SelectionKey.OP_CONNECT);
		return false;
    }

	public void handle_connected() {
		try {
			if (sc_.finishConnect()) {
				io_adp_.onConnected();
				registerEvent(SelectionKey.OP_READ);
			}
		}catch(ClosedChannelException e){
			e.printStackTrace();
			io_adp_.onClose();
		}catch(IOException e){
			e.printStackTrace();
			io_adp_.onClose();
		}
	}
	
    public void handle_read()
    {
    	// Read data
        io_adp_.onRead();    
    }
    public void handle_write()
    {
    	try{
    		cancelEvent(SelectionKey.OP_WRITE);
    		io_adp_.onWrite();
    	}catch(ClosedChannelException e){
			e.printStackTrace();
			io_adp_.onClose();
		}
    }
    
	public void accept(SocketChannel sc) throws IOException,ClosedChannelException {
		sc_ = sc;
		registerEvent(SelectionKey.OP_READ);
	}
    public int send(ByteBuffer buff)throws IOException{
    	return sc_.write(buff);
    }
    public int recv(ByteBuffer buff)throws IOException{
    	return sc_.read(buff);
    }
}
