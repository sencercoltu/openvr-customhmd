package net.speleomaniac.customhmddisplay;

import android.util.Log;
import org.iq80.snappy.Snappy;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.zip.Checksum;

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

        //LZ4SafeDecompressor decompressor = LZ4Factory.fastestInstance().safeDecompressor();
        //Checksum checksum = XXHashFactory.fastestInstance().newStreamingHash32(OutboundTcpConnection.LZ4_HASH_SEED).asChecksum();
        //LZ4BlockInputStream compressedStream = new LZ4BlockInputStream(input);

        byte[] compressedBuffer = new byte[262144];
        byte[] decompressedBuffer = new byte[3840*2160*4];
        byte[] frameType = new byte[RemoteFrameType.ClassSize];
        byte[] frameStart = new byte[RemoteFrameStart.ClassSize];
        byte[] framePayload = new byte[RemoteFramePayload.ClassSize];
        byte[] frameEnd = new byte[RemoteFrameEnd.ClassSize];

        DisplayActivity.outputStream = this.output;

        try {
            DisplayActivity._screenProcessor._processor = new ScreenProcessorThread();
            DisplayActivity._screenProcessor._thread = new Thread(DisplayActivity._screenProcessor._processor);
            DisplayActivity._screenProcessor._thread.start();

            RemoteFrameStart rfs = null;
            RemoteFrameType rft = null;
            RemoteFramePayload rfp = null;
            RemoteFrameEnd rfe = null;

            int packedSize = 0;
            int unpackedSize = 0;
            int decompressPos = 0;

            while (!Thread.currentThread().isInterrupted()) {

                int size = input.read(frameType);
                if (size != RemoteFrameType.ClassSize)
                    throw new IOException();

                rft = new RemoteFrameType(frameType);

                switch(rft.Type)
                {
                    case 1:
                        size = input.read(frameStart);
                        if (size != RemoteFrameStart.ClassSize)
                            throw new IOException();
                        rfs = new RemoteFrameStart(frameStart);
                        packedSize = 0;
                        unpackedSize = 0;
                        decompressPos = 0;
                        break;
                    case 2:
                        size = input.read(framePayload);
                        if (size != RemoteFramePayload.ClassSize)
                            throw new IOException();
                        rfp = new RemoteFramePayload(framePayload);
                        packedSize += rfp.PartSize;

                        size = 0;
                        while (size < rfp.PartSize) {
                            int read = input.read(compressedBuffer, size, rfp.PartSize - size);
                            if (read <= 0)
                                throw new IOException();
                            size += read;
                        }

                        int decompressedSize = Snappy.uncompress(compressedBuffer, 0, rfp.PartSize, decompressedBuffer, decompressPos);

                        //int decompressedSize = decompressor.decompress(compressedBuffer, 0, rfp.PartSize, decompressedBuffer, decompressPos);
                        if (decompressedSize > 0) {
                            decompressPos += decompressedSize;
                        }
                        unpackedSize = decompressPos;
                        break;
                    case 3:
                        size = input.read(frameEnd);
                        if (size != RemoteFrameEnd.ClassSize)
                            throw new IOException();
                        rfe = new RemoteFrameEnd(frameEnd);
                        if (rfe.Size != packedSize) {
                            Log.d("P", "Size NOT matched: " + rfe.Size + " != " + packedSize);
                        } else {
                            Log.d("P", "Decompressed: " + packedSize + " to " + unpackedSize);
                        }

                        break;
                }


               /* int compSize = spi.Size;
                //spi.Compressed = new byte[compSize];
                size = 0;

                ScreenProcessorThread p = DisplayActivity._screenProcessor._processor;
                if (p.isReady()) {
                    while (size < compSize) {
                        int read = input.read( p.compressed, size, Math.min(compSize - size, trash.length));
                        if (read <= 0)
                            throw new IOException();
                        size += read;
                    }
                    //if (size == compSize)
                    //    p.invalidateImage(compSize);
                }
                else {
                    while (size < compSize) {
                        int read = input.read(trash, 0, Math.min(compSize - size, trash.length));
                        if (read <= 0)
                            throw new IOException();
                        size += read;
                    }
                }
                if (size != compSize)
                    throw new IOException();*/
            }
        }
        catch (IOException e) {

            DisplayActivity.outputStream = null;

            if (DisplayActivity._screenProcessor._thread != null) DisplayActivity._screenProcessor._thread.interrupt();


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