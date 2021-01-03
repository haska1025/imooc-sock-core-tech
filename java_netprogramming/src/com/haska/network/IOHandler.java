package com.haska.network;

import java.nio.channels.spi.AbstractSelectableChannel;

public interface IOHandler {
	public void handle_read();

	public void handle_write();

	public void handle_connected();

	public void handle_accept();
	
	public AbstractSelectableChannel getSocketChannel();
}
