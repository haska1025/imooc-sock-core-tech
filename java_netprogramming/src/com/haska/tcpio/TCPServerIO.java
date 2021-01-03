package com.haska.tcpio;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;

public class TCPServerIO {
    private static final int PORT =56002;

    public static void main(String[] args) {
        ServerSocket ss = null;
        try {
            // 创建一个服务器 Socket
            ss = new ServerSocket(PORT);
            // 监听新的连接请求
            Socket conn = ss.accept();
            System.out.println("Accept a new connection:" + conn.getRemoteSocketAddress().toString());

            // 读取客户端数据
            DataInputStream in = new DataInputStream(
                    new BufferedInputStream(conn.getInputStream()));
            int msgLen = in.readInt();
            byte[] inMessage = new byte[msgLen];
            in.read(inMessage);
            System.out.println("Recv from client:" + new String(inMessage) + "length:" + msgLen);

            // 向客户端发送数据
            String rsp = "Hello Client!\n";

            DataOutputStream out = new DataOutputStream(
                    new BufferedOutputStream(conn.getOutputStream()));
            out.writeInt(rsp.getBytes().length);
            out.write(rsp.getBytes());
            out.flush();
            System.out.println("Send to client:" + rsp + " length:" + rsp.getBytes().length);
            conn.close();
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

        System.out.println("Server exit!");
    }
}
