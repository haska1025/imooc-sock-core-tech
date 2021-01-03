package com.haska.network.impl;

import com.haska.network.IOAdapter;

public class AbstractAdapter implements IOAdapter {
	protected SocketHandler sockhandler = null;
	@Override
	public void onConnected() {
		// TODO Auto-generated method stub

	}

	@Override
	public IOAdapter onAccept() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public void onRead() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onWrite() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onClose() {
		// TODO Auto-generated method stub
		
	}

	public void setSocketHandler(SocketHandler sh){
		sockhandler = sh;
	}
}
