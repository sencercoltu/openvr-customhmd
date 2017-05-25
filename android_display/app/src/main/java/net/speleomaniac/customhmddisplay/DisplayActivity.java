package net.speleomaniac.customhmddisplay;

import android.Manifest;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.opengl.Matrix;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;

import static net.speleomaniac.customhmddisplay.DisplayActivity.VirtualPacketTypes.*;


public class DisplayActivity
        extends Activity
        implements SurfaceHolder.Callback,
                   TcpClient.PacketReceiveListener,
                   CameraGrabber.CameraFrameReceiveListener,
                   SensorEventListener {

    enum VirtualPacketTypes {
        Invalid (0),
        VrFrameInit (1),
        VrFrame (2),
        Rotation (3),
        CameraFrameInit (4),
        CameraFrame (5),
        CameraAction (6)
        ;


        private int value;
        private static Map map = new HashMap<>();
        VirtualPacketTypes(int value) {
            this.value = value;
        }

        static {
            for (VirtualPacketTypes type : values()) {
                map.put(type.value, type);
            }
        }

        public static VirtualPacketTypes valueOf(int type) {
            return (VirtualPacketTypes) map.get(type);
        }

        public int getValue() {
            return value;
        }
    };

    private TcpClient tcpClient = null;
    private MediaCodec codec = null;
    private Surface surface = null;

    private SensorManager sensorManager;
    private UsbPacket usbPacket = new UsbPacket();

    CameraGrabber camera = null;

    float Q[] = new float[4];
    float m_RotMatrix[] = new float[16];
    float m_SensorToDisplay[] = new float[16];
    float m_EkfToHeadTracker[] = new float[16];
    float m_TmpHeadView[] = new float[16];
    float m_LastRotation = Float.NaN;
    private Display m_Display;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(DisplayActivity.this, new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }

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


        m_Display = getWindowManager().getDefaultDisplay();
        Matrix.setIdentityM(m_SensorToDisplay, 0);

        camera = new CameraGrabber(this);


        usbPacket.Header.Type = (byte)(UsbPacket.HMD_SOURCE | UsbPacket.ROTATION_DATA);
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("1","b");
        surface = holder.getSurface();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d("2","b");
        surface = holder.getSurface();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("3","b");
    }

    public void getQuaternion(final float[] quaternion, final float[] m) {

        final float t = m[0] + m[5] + m[10];
        float w;
        float x;
        float y;
        float z;
        if (t >= 0.0f) {
            float s = (float)Math.sqrt(t + 1.0f);
            w = 0.5f * s;
            s = 0.5f / s;
            x = (m[9] - m[6]) * s;
            y = (m[2] - m[8]) * s;
            z = (m[4] - m[1]) * s;
        }
        else if (m[0] > m[5] && m[0] > m[10]) {
            float s = (float)Math.sqrt(1.0f + m[0] - m[5] - m[10]);
            x = s * 0.5f;
            s = 0.5f / s;
            y = (m[4] + m[1]) * s;
            z = (m[2] + m[8]) * s;
            w = (m[9] - m[6]) * s;
        }
        else if (m[5] > m[10]) {
            float s = (float)Math.sqrt(1.0f + m[5] - m[0] - m[10]);
            y = s * 0.5f;
            s = 0.5f / s;
            x = (m[4] + m[1]) * s;
            z = (m[9] + m[6]) * s;
            w = (m[2] - m[8]) * s;
        }
        else {
            float s = (float)Math.sqrt(1.0f + m[10] - m[0] - m[5]);
            z = s * 0.5f;
            s = 0.5f / s;
            x = (m[2] + m[8]) * s;
            y = (m[9] + m[6]) * s;
            w = (m[4] - m[1]) * s;
        }
        quaternion[0] = w;
        quaternion[1] = x;
        quaternion[2] = y;
        quaternion[3] = z;
    }


    long m_LastPoseTime = 0;
    float m_PosePeriod = 1000.0f / 60.0f;

    boolean hasMag = false;
    boolean hasAcc = false;
    boolean hasGyro = false;

    SensorFusion sensorFusion = new SensorFusion(0.01f);
    float s_accel[] = new float[3];
    float s_mag[] = new float[3];
    float s_gyro[] = new float[3];
    long lastGyroTime = 0;
    float gyroDiff = 0f;

    private static final float NS2S = 1.0f / 1000000000.0f;

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (State == 0)
            return;

        //if (now - m_LastPoseTime < m_PosePeriod)
        //   return;
        //m_LastPoseTime = now;
        //send max 60 times per second

        if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
            if (tcpClient != null && tcpClient.IsConnected) {
                SensorManager.getRotationMatrixFromVector(m_RotMatrix, event.values);

                float rotation = 0.0f;
                switch (m_Display.getRotation()) {
                    case 0: {
                        rotation = 0.0f;
                        break;
                    }
                    case 1: {
                        rotation = 90.0f;
                        break;
                    }
                    case 2: {
                        rotation = 180.0f;
                        break;
                    }
                    case 3: {
                        rotation = 270.0f;
                        break;
                    }
                }

                if (rotation != m_LastRotation ) {
                    m_LastRotation = rotation;
                    Matrix.setRotateEulerM(m_SensorToDisplay, 0, 0.0f, 0.0f, -rotation);
                    Matrix.setRotateEulerM(m_EkfToHeadTracker, 0, -90.0f, 0.0f, rotation);
                }

                Matrix.multiplyMM(m_TmpHeadView, 0, m_SensorToDisplay, 0, m_RotMatrix, 0);
                Matrix.multiplyMM(m_RotMatrix, 0, m_TmpHeadView, 0, m_EkfToHeadTracker, 0);

                getQuaternion(Q, m_RotMatrix);

                usbPacket.Rotation.w = Q[0];
                usbPacket.Rotation.x = Q[1];
                usbPacket.Rotation.y = Q[2];
                usbPacket.Rotation.z = Q[3];
                usbPacket.buildRotationPacket();
                tcpClient.sendPacket(Rotation, usbPacket.Buffer, 32);
            }
            return;
        }
        else if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            hasAcc = true;
            System.arraycopy(event.values, 0, s_accel, 0, 3);
        }
        else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
            hasMag = true;
            System.arraycopy(event.values, 0, s_mag, 0, 3);
        }
        else if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE) {
            if (lastGyroTime != 0 && hasMag && hasAcc) {
                gyroDiff = (event.timestamp - lastGyroTime) * NS2S ;
                hasGyro = true;
            }
            System.arraycopy(event.values, 0, s_gyro, 0, 3);
            lastGyroTime = event.timestamp;
        }

        if (gyroDiff >= 0.001) {
            sensorFusion.MadgwickQuaternionUpdateAHRS(gyroDiff, s_accel[0], s_accel[1], s_accel[2], s_gyro[0], s_gyro[1], s_gyro[2], s_mag[0], s_mag[1], s_mag[2]);

            Q[3] = sensorFusion.m_RotQuat.w;
            Q[0] = sensorFusion.m_RotQuat.x;
            Q[1] = sensorFusion.m_RotQuat.y;
            Q[2] = sensorFusion.m_RotQuat.z;


            SensorManager.getRotationMatrixFromVector(m_RotMatrix, Q);

            float rotation = 0.0f;
            switch (m_Display.getRotation()) {
                case Surface.ROTATION_0: {
                    rotation = 0.0f;
                    break;
                }
                case Surface.ROTATION_90: {
                    rotation = 90.0f;
                    break;
                }
                case Surface.ROTATION_180: {
                    rotation = 180.0f;
                    break;
                }
                case Surface.ROTATION_270: {
                    rotation = 270.0f;
                    break;
                }
            }

            if (rotation != m_LastRotation ) {
                m_LastRotation = rotation;
                Matrix.setRotateEulerM(m_SensorToDisplay, 0, 0.0f, 0.0f, -rotation);
                Matrix.setRotateEulerM(m_EkfToHeadTracker, 0, -90.0f, 0.0f, rotation);
            }

            Matrix.multiplyMM(m_TmpHeadView, 0, m_SensorToDisplay, 0, m_RotMatrix, 0);
            Matrix.multiplyMM(m_RotMatrix, 0, m_TmpHeadView, 0, m_EkfToHeadTracker, 0);

            getQuaternion(Q, m_RotMatrix);

            usbPacket.Rotation.w = Q[0];
            usbPacket.Rotation.x = Q[1];
            usbPacket.Rotation.y = Q[2];
            usbPacket.Rotation.z = Q[3];

            usbPacket.buildRotationPacket();
            tcpClient.sendPacket(Rotation, usbPacket.Buffer, 32);

            gyroDiff = 0;
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }


    @Override
    protected void onPause() {
        camera.disconnect();
        sensorManager.unregisterListener(this);
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
            tcpClient = new TcpClient(this, "127.0.0.1", 1974);
            //tcpClient = new TcpClient(this, "192.168.0.10", 1974);
            tcpClient.start();
        }

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        //setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        //sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST);
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_FASTEST);
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_FASTEST);
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), SensorManager.SENSOR_DELAY_FASTEST);
    }

    private byte[] InfoPacket = new byte[4 + 4 + 4 + 4]; //magic width height freq
    private int State = 0;
    private Thread OutputRunner;

    private int InFrameCount = 0;
    private int OutFrameCount = 0;
    private long FrameTime = 0;
    private boolean isOutputRunning = false;



    @Override
    public void onPacketReceived(VirtualPacketTypes type, byte[] frameData, int len) {
        try {
            if (surface == null || tcpClient == null || !tcpClient.IsConnected || frameData == null)
            {
                State = 0;
                return;
            }

            switch(State)
            {
                case 0: {
                    if (type == VrFrameInit &&
                            len == 16 &&
                            frameData[0] == 'H' &&
                            frameData[1] == 'M' &&
                            frameData[2] == 'D' &&
                            frameData[3] == 'D') {
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
                        int freq = bb.getInt();

                        if (width > 0 && height > 0) {
                            String codecName = "video/avc";
                            codec = MediaCodec.createDecoderByType(codecName);
                            MediaFormat format = MediaFormat.createVideoFormat(codecName, width, height);
                            format.setInteger(MediaFormat.KEY_WIDTH, width);
                            format.setInteger(MediaFormat.KEY_HEIGHT, height);
                            format.setInteger(MediaFormat.KEY_MAX_WIDTH, width);
                            format.setInteger(MediaFormat.KEY_MAX_HEIGHT, height);
                            format.setInteger(MediaFormat.KEY_FRAME_RATE, freq);
                            format.setInteger(MediaFormat.KEY_OPERATING_RATE, freq);
                            format.setInteger(MediaFormat.KEY_PRIORITY, 0);
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
                    switch(type) {
                        case VrFrame:
                            {
                                InFrameCount++;
                                int inIndex = codec.dequeueInputBuffer(10);
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
                        case CameraAction:
                        {
                            boolean status = (frameData[0] != 0);
                            if (camera != null) {
                                if (status)
                                    camera.start();
                                else
                                    camera.disconnect();
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onCameraFrameReceived(byte[] data, int size, boolean initial) {
        //Log.i("Camera", "Frame");
        if (tcpClient != null && tcpClient.IsConnected) {
            tcpClient.sendPacket(initial? CameraFrameInit : CameraFrame, data, size);
        }
    }
}
