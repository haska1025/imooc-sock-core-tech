package com.haska.multithreadserver;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class TCPServerPerThread implements Runnable{
    private static final int PORT =56002;

    private Socket sock = null;

    TCPServerPerThread(Socket sock){
        this.sock = sock;
    }

    @Override
    public void run() {
        // 读取客户端数据
        try {
            while (true){
                // 读取客户端数据
                DataInputStream in = new DataInputStream(
                        new BufferedInputStream(sock.getInputStream()));
                int msgLen = in.readInt();
                byte[] inMessage = new byte[msgLen];
                in.read(inMessage);
                System.out.println("Recv from client:" + new String(inMessage) + "length:" + msgLen);

                // 向客户端发送数据
                String rsp = "Hello Client!\n";
                DataOutputStream out = new DataOutputStream(
                        new BufferedOutputStream(sock.getOutputStream()));
                out.writeInt(rsp.getBytes().length);
                out.write(rsp.getBytes());
                out.flush();
                System.out.println("Send to client:" + rsp + " length:" + rsp.getBytes().length);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (sock != null){
                try {
                    sock.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void main(String[] args) {
        ServerSocket ss = null;
        try {
            // 创建一个服务器 Socket
            ss = new ServerSocket(PORT);
            while (true){
                // 监听新的连接请求
                Socket conn = ss.accept();
                System.out.println("Accept a new connection:"
                        + conn.getRemoteSocketAddress().toString());
                Thread t = new Thread(new TCPServerPerThread(conn));
                t.start();
            }
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
