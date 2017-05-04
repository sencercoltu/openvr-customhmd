package net.speleomaniac.customhmddisplay;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import com.google.vr.sdk.base.HeadTransform;
import com.google.vr.sdk.base.sensors.HeadTracker;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DisplayActivity
        extends Activity
        implements SurfaceHolder.Callback,
                   TcpClient.PacketReceiveListener
                   //SensorEventListener
{

    private TcpClient tcpClient = null;
    private MediaCodec codec;
    private Surface surface;

    //private SensorManager sensorManager;
    private UsbPacket usbPacket = new UsbPacket();

    //private HeadTransform headTransform = new HeadTransform();
    private HeadTracker headTracker;
    private Thread trackerThread;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SurfaceView sv = new SurfaceView(this);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        sv.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

        sv.getHolder().addCallback(this);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(sv);


        headTracker = HeadTracker.createFromContext(this);
        trackerThread = new Thread(){
            @Override
            public void run() {
                float[] Q = new float[4];
                HeadTransform ht = new HeadTransform();

                while (isTrackerRunning){
                    if (tcpClient != null && tcpClient.IsConnected) {
                        headTracker.getLastHeadView(ht.getHeadView(), 0);
                        ht.getQuaternion(Q, 0);
                        usbPacket.Rotation.w = Q[3];
                        usbPacket.Rotation.x = Q[0];
                        usbPacket.Rotation.y = Q[1];
                        usbPacket.Rotation.z = Q[2];
                        usbPacket.buildRotationPacket();
                        tcpClient.sendPacket(usbPacket);
                    }
                    try {
                        long diff = System.currentTimeMillis() - FrameTime;
                        if (diff >= 1000) {
                            long inFps = (InFrameCount * 1000) / diff;
                            long outFps = (OutFrameCount * 1000) / diff;
                            InFrameCount = 0;
                            OutFrameCount = 0;
                            FrameTime = System.currentTimeMillis();
                            Log.d("FPS", "In: " + inFps + " Out: " + outFps);
                        }
                        Thread.sleep(50);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };

        usbPacket.Header.Type = (byte)(UsbPacket.HMD_SOURCE | UsbPacket.ROTATION_DATA);
        //sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        surface = holder.getSurface();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        surface = holder.getSurface();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

//    float Q[] = new float[4];
//    @Override
//    public void onSensorChanged(SensorEvent event) {
//        if (State == 0)
//            return;
//        if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
//
//            SensorManager.getQuaternionFromVector(Q, event.values);
//
//            usbPacket.Rotation.w = Q[0];
//            usbPacket.Rotation.x = Q[2];
//            usbPacket.Rotation.y = Q[1];
//            usbPacket.Rotation.z = Q[3];
//            if (tcpClient != null && tcpClient.IsConnected) {
//                usbPacket.buildRotationPacket();
//                tcpClient.sendPacket(usbPacket);
//            }
//        }
//    }
//
//    @Override
//    public void onAccuracyChanged(Sensor sensor, int accuracy) {
//
//    }

    public boolean isTrackerRunning = false;

    @Override
    protected void onPause() {
        headTracker.stopTracking();
        if (isTrackerRunning) {
            isTrackerRunning = false;
            try {
                trackerThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        //sensorManager.unregisterListener(this);
        if (tcpClient != null) {
            tcpClient.disconnect();
            tcpClient = null;
        }
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (tcpClient == null) {
            //tcpClient = new TcpClient(this, "127.0.0.1", 1974);
            tcpClient = new TcpClient(this, "192.168.0.10", 1974);
            tcpClient.start();
        }
        //setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        //sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST);
        if (!isTrackerRunning) {
            isTrackerRunning = true;
            trackerThread.start();
        }
        headTracker.startTracking();

    }

    private byte[] InfoPacket = new byte[4 + 4 + 4]; //magic width height
    private int State = 0;
    private Thread OutputRunner;

    private int InFrameCount = 0;
    private int OutFrameCount = 0;
    private long FrameTime = 0;
    private boolean isOutputRunning = false;

    @Override
    public void onPacketReceived(byte[] frameData, int len) {
        try {
            if (surface == null || tcpClient == null || !tcpClient.IsConnected || frameData == null)
            {
                State = 0;
                return;
            }

            switch(State)
            {
                case 0: {
                    if (len == 12 && frameData[0] == 'H' && frameData[1] == 'M' && frameData[2] == 'D' && frameData[3] == 'D') {
                        FrameTime = System.currentTimeMillis();
                        if (OutputRunner != null) {
                            isOutputRunning = false;
                            OutputRunner.join();
                            OutputRunner = null;
                        }
                        if (codec != null ) {
                            try {
                                codec.flush();
                                codec.stop();
                            }
                            catch (Exception e) {
                                e.printStackTrace();
                            }
                            codec.release();
                            codec = null;
                        }

                        ByteBuffer bb = ByteBuffer.wrap(frameData);
                        bb.order(ByteOrder.LITTLE_ENDIAN);
                        int magic = bb.getInt();
                        int width = bb.getInt();
                        int height = bb.getInt();

                        if (width > 0 && height > 0) {
                            String codecName = "video/avc";
                            codec = MediaCodec.createDecoderByType(codecName);
                            MediaFormat format = MediaFormat.createVideoFormat(codecName, width, height);
                            format.setInteger(MediaFormat.KEY_MAX_WIDTH, width);
                            format.setInteger(MediaFormat.KEY_MAX_HEIGHT, height);
                            format.setInteger(MediaFormat.KEY_OPERATING_RATE, Short.MAX_VALUE);
                            codec.configure(format, surface, null, 0);
                            codec.start();
                            State = 1;
                            OutputRunner = new Thread () {
                                @Override
                                public void run() {
                                    OutFrameCount = 0;
                                    MediaCodec.BufferInfo buffInfo = new MediaCodec.BufferInfo();
                                    while (isOutputRunning) {
                                        int outIndex = codec.dequeueOutputBuffer(buffInfo, 10000);
                                        switch (outIndex) {
                                            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                                                break;
                                            case MediaCodec.INFO_TRY_AGAIN_LATER:
                                                break;
                                            case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                                                break;
                                            default:
                                                OutFrameCount++;
                                                codec.releaseOutputBuffer(outIndex, true);
                                                break;
                                        }
                                    }
                                }
                            };
                            isOutputRunning = true;
                            OutputRunner.start();
                            InFrameCount = 0;
                        }
                    }
                    else {
                        tcpClient.reconnect();
                    }
                }
                break;
                case 1: {
                    InFrameCount++;
                    int inIndex = codec.dequeueInputBuffer(10000);
                    if (inIndex >= 0) {
                        ByteBuffer inputBuffer = codec.getInputBuffer(inIndex);
                        if (inputBuffer != null) {
                            inputBuffer.clear();
                            inputBuffer.put(frameData, 0, len);
                            codec.queueInputBuffer(inIndex, 0, len, 1, 0);
                        }
                    }



                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
