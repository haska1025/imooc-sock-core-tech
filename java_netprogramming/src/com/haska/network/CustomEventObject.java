package com.haska.network;

import java.util.EventObject;

public class CustomEventObject extends EventObject{
    /**
     * 
     */
    private static final long serialVersionUID = -5732376546950130991L;

    // The string type
    protected String type;
    
    // The process listener
    protected Listener listener;
    
    public CustomEventObject(Object src, Listener l, String t){
        super(src);
    	this.type = t;
    	this.listener = l;
    }
    
    public Listener listen(){return listener;}
    public String type(){return type;}
}
