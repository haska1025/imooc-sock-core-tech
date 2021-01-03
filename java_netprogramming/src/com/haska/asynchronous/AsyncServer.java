package com.haska.asynchronous;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class AsyncServer {
    private static final int PORT =56002;

    public static void main(String[] args) {
        try (AsynchronousServerSocketChannel server = AsynchronousServerSocketChannel.open()){
            server.bind(new InetSocketAddress("127.0.0.1", PORT));

            Future<AsynchronousSocketChannel> acceptFuture = server.accept();
            AsynchronousSocketChannel client = acceptFuture.get(10, TimeUnit.SECONDS);

            if (client != null && client.isOpen()){
                ByteBuffer inBuffer = ByteBuffer.allocate(128);
                Future<Integer> readResult = client.read(inBuffer);
                System.out.println("Do something");
                readResult.get();

                inBuffer.flip();
                Future<Integer> writeResult = client.write(inBuffer);
                writeResult.get();
            }

            client.close();
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
