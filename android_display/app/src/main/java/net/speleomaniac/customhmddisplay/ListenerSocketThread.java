package net.speleomaniac.customhmddisplay;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

class ListenerSocketThread implements Runnable {

    private static final int SERVERPORT = 1974;
    private ServerSocket serverSocket;

    public void run () {
        try {
            serverSocket = new ServerSocket(SERVERPORT);
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        while (!Thread.currentThread().isInterrupted()) {

            try {
                Socket socket = serverSocket.accept();
                serverSocket.close();
                new Thread(new ClientSocketThread(socket)).start();
                return;
            }
            catch (IOException e){
                e.printStackTrace();
            }
        }

        try {
            serverSocket.close();
        } catch (IOException e1) {
            e1.printStackTrace();
        }
        serverSocket = null;
    }
}
