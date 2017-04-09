package net.speleomaniac.customhmddisplay;

import android.annotation.SuppressLint;
import android.graphics.Point;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.LinearLayout;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DisplayActivity extends AppCompatActivity implements SensorEventListener {

    private SensorManager sensorManager;
    public static OutputStream outputStream;
    float[] rotationData = new float[4];

    static Handler UIHandler;
    public static Thread _listenerSocketThread = null;
    private static final boolean AUTO_HIDE = true;
    private static final int AUTO_HIDE_DELAY_MILLIS = 3000;
    private static final int UI_ANIMATION_DELAY = 300;
    private final Handler mHideHandler = new Handler();
    private LinearLayout mContentView;
    private boolean mVisible;

    public static int EyeWidth;
    public static int EyeHeight;

    static EyeProcessor[] EyeProcessors;

    byte[] rotationBytes = new byte[16];
    ByteBuffer bb;

    private ImageView[] Eyes = new ImageView[2];

    private final Runnable mHidePart2Runnable = new Runnable() {
        @SuppressLint("InlinedApi")
        @Override
        public void run() {
            mContentView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
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
        setContentView(R.layout.activity_display);

        bb = ByteBuffer.wrap(rotationBytes);
        bb.order(ByteOrder.LITTLE_ENDIAN);

        EyeProcessors = new EyeProcessor[] {
                new EyeProcessor(this),
                new EyeProcessor(this)
        };

        UIHandler = new Handler();

        _listenerSocketThread = new Thread(new ListenerSocketThread());
        _listenerSocketThread.start();

        mVisible = true;
        mContentView = (LinearLayout) findViewById(R.id.fullscreen_content);

        Eyes[0] = (ImageView) findViewById(R.id.leftEye);
        Eyes[1] = (ImageView) findViewById(R.id.rightEye);

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        EyeWidth = size.x;
        EyeHeight = size.y;

        // Set up the user interaction to manually show or hide the system UI.
        mContentView.setOnClickListener(new View.OnClickListener() {
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

            if (outputStream != null) {
                try {
                    outputStream.write(rotationBytes);
                } catch (IOException e) {
                    outputStream = null;
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
        mContentView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        mVisible = true;

        // Schedule a runnable to display UI elements after a delay
        mHideHandler.removeCallbacks(mHidePart2Runnable);
        mHideHandler.postDelayed(mShowPart2Runnable, UI_ANIMATION_DELAY);
    }

    private void delayedHide(int delayMillis) {
        mHideHandler.removeCallbacks(mHideRunnable);
        mHideHandler.postDelayed(mHideRunnable, delayMillis);
    }

    private class ImagePainter implements Runnable {

        private int Eye;
        private EyeProcessorThread Processor;

        private ImagePainter(int eye, EyeProcessorThread processor) {
            Eye = eye;
            Processor = processor;
        }

        @Override
        public void run() {
            synchronized (Processor._lock) {
                try {
                    Eyes[Eye].setImageBitmap(Processor._bitmap);
                }
                catch (Exception e){
                    e.printStackTrace();
                }
            }

        }
    }

    public void TriggerEye(int eye, EyeProcessorThread eyeProcessorThread) {
        UIHandler.post(new ImagePainter(eye, eyeProcessorThread));
        //Eyes[eye].setImageBitmap(eyeProcessorThread._bitmap);
    }
}
