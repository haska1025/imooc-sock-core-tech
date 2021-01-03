package com.haska.network.impl;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.AbstractSelectableChannel;

import com.haska.network.IOAdapter;
import com.haska.network.IOHandler;

public class SocketHandler implements IOHandler{
	private SelectionKey sk_ = null;
	protected SocketChannel sc_;
    protected IOThread io_thr_;
    protected IOAdapter io_adp_;
    
    public IOThread io_thr(){return io_thr_;}
    
	public SocketHandler(IOThread io_thr, IOAdapter io_adp){
		io_thr_ = io_thr;
		io_adp_ = io_adp;
	}

	public void registerEvent(int ops) throws ClosedChannelException {
		if (sk_ == null) {
			sk_ = io_thr_.getPoller().register(this, ops);
		} else {
			sk_.interestOps(sk_.interestOps() | ops);
		}
	}

	public void cancelEvent(int ops) throws ClosedChannelException {
		if (sk_ == null)
			return;

		sk_.interestOps(sk_.interestOps() & (~ops));
	}
	
	// IOHandler's methods
    public void handle_read()
    {
    	
    }
    public void handle_write()
    {
    	
    }
    public void handle_timeout(long timerid)
    {
    	
    }
    public void handle_connected()
    {
    	
    }
    public void handle_accept()
    {
    	
    }
    
    public AbstractSelectableChannel getSocketChannel()
    {
    	return sc_;
    }
    
    // Need override by subclass
    public boolean connect(String hostname, int port) throws IOException{return false;}
    public void accept(SocketChannel sc) throws IOException,ClosedChannelException {}    
    public int send(ByteBuffer buff)throws IOException{return -1;}
    public int recv(ByteBuffer buff)throws IOException{return -1;}
    public void close(){
    	try{
    	    sk_.cancel();
    	    sc_.close();
    	}catch(IOException e){
    		;
    	}
    }
}
