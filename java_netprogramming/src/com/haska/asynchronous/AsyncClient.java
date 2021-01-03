package com.haska.asynchronous;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.Future;

public class AsyncClient {
    private static final int PORT =56002;
    public static void main(String[] args) {
        try (AsynchronousSocketChannel client = AsynchronousSocketChannel.open()) {
            Future<Void> result = client.connect(new InetSocketAddress("127.0.0.1", PORT));
            System.out.println("Async connect the server");
            result.get();

            String reqMessage = "Hello server!";
            ByteBuffer reqBuffer = ByteBuffer.wrap(reqMessage.getBytes());
            Future<Integer> writeResult = client.write(reqBuffer);
            System.out.println("Async send to server:" + reqMessage);
            writeResult.get();

            ByteBuffer inBuffer = ByteBuffer.allocate(128);
            Future<Integer> readResult = client.read(inBuffer);
            readResult.get();
            System.out.println("Async recv from server:" + new String(inBuffer.array()).trim());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
