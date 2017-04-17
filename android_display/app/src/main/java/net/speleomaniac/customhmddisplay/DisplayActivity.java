package net.speleomaniac.customhmddisplay;

import android.annotation.SuppressLint;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DisplayActivity extends AppCompatActivity implements SensorEventListener, SurfaceHolder.Callback {

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

    SurfaceView surfaceView;
    SurfaceHolder surfaceHolder;

    byte[] rotationBytes = new byte[16];
    ByteBuffer bb;

    private final Runnable mHidePart2Runnable = new Runnable() {
        @SuppressLint("InlinedApi")
        @Override
        public void run() {
            surfaceView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
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

        int numCodec = MediaCodecList.getCodecCount();
        Log.i("MMM", "We got " + numCodec + " Codecs");

        for (int i = 0; i < numCodec; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);

            if (codecInfo.isEncoder()) {
                continue;
            }

            String[] type = codecInfo.getSupportedTypes();
            for (int j = 0; j < type.length; j++) {
                Log.i("MMM", "We got type " + type[j]);
            }
        }


        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_display);

        bb = ByteBuffer.wrap(rotationBytes);
        bb.order(ByteOrder.LITTLE_ENDIAN);

        new Thread() {
            public void run () {
                try {
                    serverSocket = new ServerSocket(SERVERPORT);
                }
                catch (IOException e) {
                    e.printStackTrace();
                }

                while (!Thread.currentThread().isInterrupted()) {

                    try {
                        clientSocket = serverSocket.accept();
                        serverSocket.close();
                        new Thread() {
                            public void run() {
                                try {
                                    clientInputStream = clientSocket.getInputStream();
                                    clientOutputStream = clientSocket.getOutputStream();
                                }
                                catch (IOException e) {
                                    e.printStackTrace();
                                }

                                final byte[] jpgData = new byte[8];
                                ByteBuffer js;

                                int inputBufferSize = 3840*2160*4;
                                byte[] inputBuffer = new byte[inputBufferSize];

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
                                        Log.d("HMD", "Got picture:" + eye + " of " + size);

                                    }
                                }
                                catch (IOException e) {
                                        e.printStackTrace();
                                    try {
                                        clientSocket.close();
                                    } catch (IOException e1) {
                                        e1.printStackTrace();
                                    }
                                    clientSocket = null;
                                    try {
                                        serverSocket = new ServerSocket(SERVERPORT);
                                    } catch (IOException e1) {
                                        e1.printStackTrace();
                                    }
                                }
                            }

                        }.start();
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
        }.start();

        mVisible = true;
        surfaceView = new SurfaceView(this);
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
        setContentView(surfaceView);

        // Set up the user interaction to manually show or hide the system UI.
        surfaceView.setOnClickListener(new View.OnClickListener() {
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

            bb.putFloat(0, event.values[3]);
            bb.putFloat(4, event.values[1]);
            bb.putFloat(8, event.values[2]);
            bb.putFloat(12, event.values[0]);

            if (clientOutputStream != null) {
                try {
                    clientOutputStream.write(rotationBytes);
                } catch (IOException e) {
                    clientOutputStream = null;
                    e.printStackTrace();
                }
            }

        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    protected void onPause() {
        super.onPause();
        sensorManager.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
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
        surfaceView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        mVisible = true;

        // Schedule a runnable to display UI elements after a delay
        mHideHandler.removeCallbacks(mHidePart2Runnable);
        mHideHandler.postDelayed(mShowPart2Runnable, UI_ANIMATION_DELAY);
    }

    private void delayedHide(int delayMillis) {
        mHideHandler.removeCallbacks(mHideRunnable);
        mHideHandler.postDelayed(mHideRunnable, delayMillis);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
