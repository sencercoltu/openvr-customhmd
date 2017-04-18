package net.speleomaniac.customhmddisplay;

import android.annotation.SuppressLint;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DisplayActivity extends AppCompatActivity
        implements SensorEventListener {

    private static final int SERVERPORT = 1974;
    private ServerSocket serverSocket;
    private Socket clientSocket;
    private InputStream clientInputStream;
    private OutputStream clientOutputStream;
    private SensorManager sensorManager;
    //float[] rotationData = new float[4];

    private static final boolean AUTO_HIDE = true;
    private static final int AUTO_HIDE_DELAY_MILLIS = 3000;
    private static final int UI_ANIMATION_DELAY = 300;
    private final Handler mHideHandler = new Handler();
    private boolean mVisible;

    private DisplaySurface surface;

    private int surfaceWidth;
    private int surfaceHeight;

    byte[] rotationBytes = new byte[16];
    ByteBuffer rotationByteBuffer;

    private final Runnable mHidePart2Runnable = new Runnable() {
        @SuppressLint("InlinedApi")
        @Override
        public void run() {
            surface.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        }
    };

    private final Runnable mShowPart2Runnable = new Runnable() {
        @Override
        public void run() {
            ActionBar actionBar = getSupportActionBar();
            if (actionBar != null) {
                actionBar.show();
            }
        }
    };

    private final Runnable mHideRunnable = new Runnable() {
        @Override
        public void run() {
            hide();
        }
    };

    private final View.OnTouchListener mDelayHideTouchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            if (AUTO_HIDE) {
                delayedHide(AUTO_HIDE_DELAY_MILLIS);
            }
            return false;
        }
    };



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

        surface = new DisplaySurface(this);
        // Set the Renderer for drawing on the GLSurfaceView
        surface.mRenderer = new DisplayRenderer(this);
        surface.setRenderer(surface.mRenderer);

        // Render the view only when there is a change in the drawing data
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);


        setContentView(surface);

        rotationByteBuffer = ByteBuffer.wrap(rotationBytes);
        rotationByteBuffer.order(ByteOrder.LITTLE_ENDIAN);


        new Thread() {
            public void run () {
                try {
                    serverSocket = new ServerSocket(SERVERPORT);
                    serverSocket.setReuseAddress(true);
                } catch (IOException e) {
                    //e.printStackTrace();
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e1) {
                        //e1.printStackTrace();
                    }
                }
                while (!Thread.currentThread().isInterrupted()) {
                    try {
                        clientSocket = serverSocket.accept();
                    } catch (IOException e) {
                        //e.printStackTrace();
                    }
                    new Thread() {
                        public void run() {
                            final byte[] jpgData = new byte[8];
                            ByteBuffer js;

                            int inputBufferSize = 3840*2160*4;
                            byte[] inputBuffer = new byte[inputBufferSize];


                            try {
                                clientInputStream = clientSocket.getInputStream();
                                clientOutputStream = clientSocket.getOutputStream();
                            }
                            catch (IOException e) {
                                //e.printStackTrace();
                                try {
                                    Thread.sleep(100);
                                } catch (InterruptedException e1) {
                                    //e1.printStackTrace();
                                }
                                clientInputStream = null;
                                clientOutputStream = null;
                                try {
                                    clientSocket.close();
                                } catch (IOException e1) {
                                    //e1.printStackTrace();
                                }
                                clientSocket = null;
                                return;
                            }

                            //Surface surf = new Surface(renderer.getVideoTexture());

                            try {
                                while (!Thread.currentThread().isInterrupted()) {
                                    int read = clientInputStream.read(jpgData);
                                    if (read < 0)
                                        break;

                                    js = ByteBuffer.wrap(jpgData);
                                    js.order(ByteOrder.LITTLE_ENDIAN);
                                    int eye = js.getInt();
                                    int size = js.getInt();
                                    int pos = 0;
                                    int remain = size;

                                    while (remain > 0) {
                                        read = clientInputStream.read(inputBuffer, pos, Math.min(remain, 32768));
                                        if (read < 0)
                                            break;
                                        pos += read;
                                        remain -= read;
                                    }
                                    //Log.d("HMD", "Got picture:" + eye + " of " + size);
                                    Bitmap image = BitmapFactory.decodeByteArray(inputBuffer, 0, size);
                                    Bitmap oldBmp = null;
                                    if (eye == 0) {
                                        synchronized (surface.mRenderer.imageLock) {
                                            oldBmp =surface.mRenderer.left_Image;
                                            surface.mRenderer.left_Image = image;
                                        }
                                    }
                                    else if (eye == 1) {
                                        synchronized (surface.mRenderer.imageLock) {
                                            oldBmp = surface.mRenderer.right_Image;
                                            surface.mRenderer.right_Image = image;
                                        }
                                    }
                                    if (oldBmp != null)
                                        oldBmp.recycle();
                                }
                            }
                            catch (IOException e) {
                                //e.printStackTrace();
                                try {
                                    clientSocket.close();
                                    clientInputStream = null;
                                    clientOutputStream = null;

                                } catch (IOException e1) {
                                    //e1.printStackTrace();
                                }
                                try {
                                    Thread.sleep(100);
                                } catch (InterruptedException e1) {
                                    //e1.printStackTrace();
                                }
                            }
                            //surf.release();
                        }
                    }.start();
                }

                try {
                    if (serverSocket != null)
                        serverSocket.close();

                } catch (IOException e1) {
                    //e1.printStackTrace();
                }
            }
        }.start();



        mVisible = true;

        // Set up the user interaction to manually show or hide the system UI.
        surface.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                toggle();
            }
        });

        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);


    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {

            rotationByteBuffer.putFloat(0, event.values[3]);
            rotationByteBuffer.putFloat(4, event.values[1]);
            rotationByteBuffer.putFloat(8, event.values[2]);
            rotationByteBuffer.putFloat(12, event.values[0]);

            if (clientOutputStream != null) {
                try {
                    clientOutputStream.write(rotationBytes);
                } catch (IOException e) {
                    //clientOutputStream = null;
                    //e.printStackTrace();
                }
            }

        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    protected void onPause() {
        sensorManager.unregisterListener(this);
        surface.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        surface.onResume();
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        // Trigger the initial hide() shortly after the activity has been
        // created, to briefly hint to the user that UI controls
        // are available.
        delayedHide(100);
    }

    private void toggle() {
        if (mVisible) {
            hide();
        } else {
            show();
        }
    }

    private void hide() {
        // Hide UI first
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.hide();
        }
        mVisible = false;

        // Schedule a runnable to remove the status and navigation bar after a delay
        mHideHandler.removeCallbacks(mShowPart2Runnable);
        mHideHandler.postDelayed(mHidePart2Runnable, UI_ANIMATION_DELAY);
    }

    @SuppressLint("InlinedApi")
    private void show() {
        // Show the system bar
        surface.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        mVisible = true;

        // Schedule a runnable to display UI elements after a delay
        mHideHandler.removeCallbacks(mHidePart2Runnable);
        mHideHandler.postDelayed(mShowPart2Runnable, UI_ANIMATION_DELAY);
    }

    private void delayedHide(int delayMillis) {
        mHideHandler.removeCallbacks(mHideRunnable);
        mHideHandler.postDelayed(mHideRunnable, delayMillis);
    }
}
