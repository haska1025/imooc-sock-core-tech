package com.haska.sockoption;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;


public class SockOptions {
    public static void main(String[] args) {
        ServerSocket ss = null;
        try {
            ss = new ServerSocket();
            ss.setReuseAddress(true);
            ss.bind(new InetSocketAddress(8022));
            Socket sock = ss.accept();
            sock.setKeepAlive(true);
            sock.setSoLinger(true, 20);
            sock.setSendBufferSize(16384);
            sock.setReceiveBufferSize(16384);
            sock.setOOBInline(true);
            sock.setTcpNoDelay(true);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (ss != null){
                try {
                    ss.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
