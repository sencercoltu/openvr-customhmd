package net.speleomaniac.customhmddisplay;


import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;


class TcpClient extends Thread {

    interface PacketReceiveListener {
        void onPacketReceived(byte[] data, int size);
    }

    private PacketReceiveListener packetReceiveListener = null;
    private final int Port;
    private final String Server;

    private Socket driverSocket = null;
    private InputStream input = null;
    private OutputStream output = null;

    private long LastPacketReceive = 0;
    public boolean IsConnected;

    TcpClient(PacketReceiveListener listener, String server, int port) {
        IsConnected = false;
        Server = server;
        Port = port;
        packetReceiveListener = listener;
    }

    private UsbPacket packetToSend = null;
    private final Object socketLock = new Object();

    void sendPacket(UsbPacket usbPacket) {
        synchronized (socketLock) {
            packetToSend = usbPacket;
        }
    }

    private void closeDriverSocket() {
        if (driverSocket != null) {
            try {
                packetReceiveListener.onPacketReceived(null, 0);
                driverSocket.close();
                input.close();
                output.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        driverSocket = null;
        input = null;
        output = null;
        IsConnected = false;
    }

    private boolean _reconnect = false;

    void reconnect() {
        _reconnect = true;
    }

    void disconnect() {
        if (isInterrupted())
            return;
        interrupt();
        //isRunning  = false;
        closeDriverSocket();
    }

    //boolean isRunning = false;

    @Override
    public synchronized void start() {
        if (isAlive()) return;
        //if (isRunning)
        //   return;
        //isRunning = true;
        super.start();
    }

    @Override
    public void run() {
        byte[] header = new byte[4]; //size only
        ByteBuffer headerBuffer = ByteBuffer.wrap(header);
        headerBuffer.order(ByteOrder.LITTLE_ENDIAN);
        byte[] buffer = new byte[1024 * 1024 * 10]; //10M
        byte[] outBuffer;
        while (!Thread.interrupted()) {
            try {
                _reconnect = false;
                LastPacketReceive = System.currentTimeMillis();
                InetAddress serverAddr = InetAddress.getByName(Server);
                driverSocket = new Socket(serverAddr, Port);
                input = driverSocket.getInputStream();
                output = driverSocket.getOutputStream();
                IsConnected = true;

                while (!Thread.interrupted() && !_reconnect && IsConnected && driverSocket != null) {
                    synchronized (socketLock) {
                        if (packetToSend != null) {
                            output.write(packetToSend.Buffer, 0, 32);
                            packetToSend = null;
                        }
                    }

                    int size = 0;
                    int avail = 0;
                        avail = input.available();
                    if (avail > 4) {
                        int read = input.read(header);
                        headerBuffer.rewind();
                        size = headerBuffer.getInt();
                    }

                    if (size == 0) {
                        //check disconnect (adb forward or reverse mode keeps connection )
                        if (System.currentTimeMillis() - LastPacketReceive > 2000) {
                            closeDriverSocket();
                            continue;
                        }
                    }

                    int remain = size;
                    int pos = 0;
                    while (!Thread.interrupted() && IsConnected && remain > 0) {
                        avail = 0;
                        if (input == null)
                            break;
                        avail = input.available();
                        if (avail >= 0) {
                            avail = Math.min(remain, avail);
                            int read = 0;
                            read = input.read(buffer, pos, avail);
                            if (read < 0) {
                                closeDriverSocket();
                                break;
                            }
                            pos += read;
                            remain -= read;
                            if (remain == 0) {
                                LastPacketReceive = System.currentTimeMillis();
                                packetReceiveListener.onPacketReceived(buffer, size);
                            }
                        } else {
                            try {
                                Thread.sleep(1);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            closeDriverSocket();
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}