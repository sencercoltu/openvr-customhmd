package net.speleomaniac.customhmddisplay;


import android.os.AsyncTask;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;



class UdpComm extends Thread {

    interface PacketReceiveListener {
        public void onPacketReceived(DatagramPacket packet);
    }

    private PacketReceiveListener packetReceiveListener = null;
    private final int Port;
    private DatagramSocket driverSocket = null;
    private DatagramPacket driverPacket;
    private long LastPacketReceive = 0;

    UdpComm(int port) {
        Port = port;
    }

    void setPacketReceiveListener(PacketReceiveListener listener) {
        packetReceiveListener = listener;
    }

    UsbPacket packetToSend = null;

    public void sendPacket(UsbPacket usbPacket) {
        if (driverSocket == null)
            return;
        packetToSend = usbPacket;
    }

    private void closeDriverSocket() {
        if (driverSocket != null)
            driverSocket.close();
        driverSocket = null;
    }

    @Override
    public void run() {
        DatagramSocket clientSocket = null;
        try {
            clientSocket = new DatagramSocket(Port);
        } catch (SocketException e) {
            e.printStackTrace();
            closeDriverSocket();
            return;
        }

        Thread senderThread = new Thread() {
            @Override
            public void run() {
                while (!Thread.interrupted()) {
                    try {
                        Thread.sleep(20); //max 50 rots per sec
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    if (packetToSend == null) continue;
                    synchronized (packetToSend) {
                        byte[] packet = packetToSend.Buffer;
                        driverPacket.setData(packet, 0, 32);
                        try {
                            if (driverSocket != null)
                                driverSocket.send(driverPacket);
                            packetToSend = null;
                        } catch (IOException e) {
                            e.printStackTrace();
                            closeDriverSocket();
                        }
                    }
                }
            }
        };
        senderThread.start();

        byte[] buffer = new byte[64000];
        int len;
        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
        while (!Thread.interrupted()) {
            try {
                clientSocket.receive(packet);
                len = packet.getLength();
                LastPacketReceive = System.currentTimeMillis();
                if (len > 0 && packetReceiveListener != null)
                    packetReceiveListener.onPacketReceived(packet);
            } catch (IOException e) {
                e.printStackTrace();
                closeDriverSocket();
                break;
            }
        }
        senderThread.interrupt();
        clientSocket.close();
        closeDriverSocket();
    }

    private byte[] driverBuffer = new byte[32];
    void setDriverAddress(InetAddress addr) {
        try {
            driverSocket = new DatagramSocket();
        } catch (SocketException e) {
            e.printStackTrace();
            driverSocket = null;
            return;
        }
        driverSocket.connect(addr, Port);
        driverPacket = new DatagramPacket(driverBuffer, 32);
    }
}