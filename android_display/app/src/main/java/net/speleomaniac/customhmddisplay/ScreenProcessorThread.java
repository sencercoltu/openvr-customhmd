package net.speleomaniac.customhmddisplay;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.util.Log;
import android.view.Display;

import java.io.ByteArrayInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

class ScreenProcessorThread implements Runnable {
Bitmap _bitmap;


    ScreenProcessorThread() {
        compressed = new byte[3840 * 2160 * 4]; //2K rgba
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
                    _bitmap = BitmapFactory.decodeStream(new ByteArrayInputStream(compressed));
                    _newImage = false;
                    _size = 0;
                    if (_bitmap != null && _bitmap.getWidth() > 0)
                        DisplayActivity._screenProcessor._activity.TriggerEye(this);
                    Log.d("Image Process", (System.currentTimeMillis()-start) + " ms.");
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