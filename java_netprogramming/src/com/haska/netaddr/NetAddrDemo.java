package com.haska.netaddr;

import java.net.*;

public class NetAddrDemo {
    public static void testInetAddressByAddr()
    {
        byte[] ips = new byte[]{ (byte)192, (byte)168,1,101};
        try {
            InetAddress addr = InetAddress.getByAddress(ips);
            System.out.println("getByAddress addr=" + addr.toString());

            InetAddress addr2 = InetAddress.getByAddress("www.example.com", ips);
            System.out.println("getByAddress with host addr=" + addr2.toString());
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
    }
    public static void testInetAddressByName(String host){
        try {
            InetAddress addr = InetAddress.getByName(host);
            System.out.println("getByName addr=" + addr.toString());

            InetAddress[] addrs = InetAddress.getAllByName(host);
            for (InetAddress a: addrs){
                System.out.println("getAllByName addr=" + a.toString());
            }
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
    }
    public static void main(String[] args) {
        testInetAddressByName("www.imooc.com");
        testInetAddressByName("www.imooc1.com");
        testInetAddressByName("www.imoocabc.com");
        //InetSocketAddress
        //java.net.Inet4Address
        testInetAddressByAddr();
    }
}
