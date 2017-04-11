package net.speleomaniac.customhmddisplay;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

class ClientSocketThread implements Runnable {
    private Socket clientSocket;
    private InputStream input;
    private OutputStream output;

    ClientSocketThread(Socket clientSocket) {
        this.clientSocket = clientSocket;
        try {
            this.input = this.clientSocket.getInputStream();
            this.output = this.clientSocket.getOutputStream();
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void run() {

        byte[] screenInfo = new byte[16];
        byte[] screenPartInfo = new byte[8];

        DisplayActivity.outputStream = this.output;

        try {
            byte[] trash = new byte[3840 * 2160 * 4]; //4k rgba çüş

            DisplayActivity.EyeProcessors[0]._processor = new EyeProcessorThread(0);
            DisplayActivity.EyeProcessors[0]._thread = new Thread(DisplayActivity.EyeProcessors[0]._processor);
            DisplayActivity.EyeProcessors[0]._thread.start();

            DisplayActivity.EyeProcessors[1]._processor = new EyeProcessorThread(1);
            DisplayActivity.EyeProcessors[1]._thread = new Thread(DisplayActivity.EyeProcessors[1]._processor);
            DisplayActivity.EyeProcessors[1]._thread.start();

            while (!Thread.currentThread().isInterrupted()) {

                int size = input.read(screenPartInfo);
                if (size != 8)
                    throw new IOException();

                ScreenPartInfo spi = new ScreenPartInfo(screenPartInfo);
                int compSize = spi.Size;
                //spi.Compressed = new byte[compSize];
                size = 0;

                EyeProcessorThread p = DisplayActivity.EyeProcessors[spi.Eye]._processor;
                if (p.isReady()) {
                    while (size < compSize) {
                        int read = input.read( p.compressed, size, compSize - size);
                        if (read <= 0)
                            throw new IOException();
                        size += read;
                    }
                    if (size == compSize)
                        p.invalidateImage(compSize);
                }
                else {
                    while (size < compSize) {
                        int read = input.read(trash, size, compSize - size);
                        if (read <= 0)
                            throw new IOException();
                        size += read;
                    }
                }
                if (size != compSize)
                    throw new IOException();
            }
        }
        catch (IOException e) {

            DisplayActivity.outputStream = null;

            if (DisplayActivity.EyeProcessors[0]._thread != null) DisplayActivity.EyeProcessors[0]._thread.interrupt();
            if (DisplayActivity.EyeProcessors[1]._thread != null) DisplayActivity.EyeProcessors[1]._thread.interrupt();

            e.printStackTrace();
            try {
                clientSocket.close();
            } catch (IOException e1) {
                e1.printStackTrace();
            }
            clientSocket = null;
            DisplayActivity._listenerSocketThread = new Thread(new ListenerSocketThread());
            DisplayActivity._listenerSocketThread.start();
        }
    }
}