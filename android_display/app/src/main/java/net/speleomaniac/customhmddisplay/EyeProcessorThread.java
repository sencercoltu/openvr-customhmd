package net.speleomaniac.customhmddisplay;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.view.Display;

import java.io.ByteArrayInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

class EyeProcessorThread implements Runnable {

    private int Eye;
    Bitmap _bitmap;


    EyeProcessorThread(int eye, ScreenInfo info) {
        Eye = eye;
        compressed = new byte[info.Stride * info.Height];
    }

    byte[] compressed;

    final Object _lock = new Object();
    private boolean _newImage = false;
    private int _size = 0;

    boolean isReady() {
        return _newImage == false;
    }


    void invalidateImage(int size) {
        synchronized (_lock) {
            _newImage = true;
            _size = size;
        }
    }

    @Override
    public void run() {
        while (!Thread.currentThread().isInterrupted()) {
            synchronized (_lock) {
                if (_newImage && _size > 0) {
                    long start = System.currentTimeMillis();
                    _bitmap = BitmapFactory.decodeByteArray(compressed, 0, _size);
                    _newImage = false;
                    _size = 0;
                    if (_bitmap != null && _bitmap.getWidth() > 0)
                        DisplayActivity.EyeProcessors[Eye]._activity.TriggerEye(Eye, this);
                    continue;
                }
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}