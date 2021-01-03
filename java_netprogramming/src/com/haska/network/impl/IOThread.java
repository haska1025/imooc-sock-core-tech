package com.haska.network.impl;

import java.io.IOException;
import java.util.concurrent.ConcurrentLinkedQueue;

import com.haska.network.CustomEventObject;

public class IOThread {
    private Poller poller_ = new Poller();
    private boolean stop_ = false;
    private Worker worker = null;
    private ConcurrentLinkedQueue<CustomEventObject> listener = new ConcurrentLinkedQueue<CustomEventObject>();
    
    public Poller getPoller(){return poller_;}
    
    public void start() throws IOException{
    	poller_.start();
    	
    	// Start worker
    	worker = new Worker();
    	worker.start();
    }
    
    public void addListener(CustomEventObject e){
    	listener.offer(e);
    	poller_.wakeup();
    }
    
    public void stop(){
		try {
			stop_ = true;
			poller_.wakeup();
			worker.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
    }
    
    void dispatchListeners(){
    	while (!listener.isEmpty()){
    		CustomEventObject e = listener.poll();
    		e.listen().process(e);
    	}
    }
    class Worker extends Thread{
		@Override
		public void run() {
			while(!stop_){
				try{
					dispatchListeners();
					poller_.poll();
				} catch (IOException e){
					e.printStackTrace();
				}
			}
		}
	}
}
