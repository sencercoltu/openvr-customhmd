package net.speleomaniac.customhmddisplay;

import android.Manifest;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.Image;
import android.media.ImageWriter;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.opengl.Matrix;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import org.bytedeco.javacpp.*;
import static org.bytedeco.javacpp.avcodec.*;
import static org.bytedeco.javacpp.avformat.*;
import static org.bytedeco.javacpp.avutil.*;
import static org.bytedeco.javacpp.swscale.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.HashMap;
import java.util.Map;

import static net.speleomaniac.customhmddisplay.DisplayActivity.VirtualPacketTypes.*;
import static org.bytedeco.javacpp.avformat.av_register_all;


public class DisplayActivity
        extends Activity
        implements SurfaceHolder.Callback,
                   TcpClient.PacketReceiveListener,
                   CameraGrabber.CameraFrameReceiveListener,
                   SensorEventListener {

    enum VirtualPacketTypes {
        Invalid ((short)0),
        VrFrameInit ((short)1),
        VrFrame ((short)2),
        Rotation ((short)3),
        CameraFrameInit ((short)4),
        CameraFrame ((short)5),
        CameraAction ((short)6)
        ;


        private short value;
        private static Map map = new HashMap<>();
        VirtualPacketTypes(short value) {
            this.value = value;
        }

        static {
            for (VirtualPacketTypes type : values()) {
                map.put(type.value, type);
            }
        }

        public static VirtualPacketTypes valueOf(short type) {
            return (VirtualPacketTypes) map.get(type);
        }

        public short getValue() {
            return value;
        }
    };

    private TcpClient tcpClient = null;

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

    AVCodec pCodec = null;
    AVCodecContext pCodecContext = null;
    AVFrame pFrame = null;
    AVFrame pFrameRGB = null;
    AVPacket pPacket = new avcodec.AVPacket();
    AVDictionary dict = new AVDictionary();
    private BytePointer pFrameDataPointer = null;
    SurfaceHolder pHolder = null;
    SwsContext pSwsContext = null;

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

        pHolder = sv.getHolder();
        //sv.getHolder().addCallback(this);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(sv);


        m_Display = getWindowManager().getDefaultDisplay();
        Matrix.setIdentityM(m_SensorToDisplay, 0);

        camera = new CameraGrabber(this);


        usbPacket.Header.Type = (byte)(UsbPacket.HMD_SOURCE | UsbPacket.ROTATION_DATA);
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

        av_register_all();
        av_init_packet(pPacket);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("1","b");
        //surface = holder.getSurface();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d("2","b");
        //surface = holder.getSurface();
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
            //tcpClient = new TcpClient(this, "192.168.42.178", 1974);
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

    private int InFrameCount = 0;
    private int OutFrameCount = 0;
    private long FrameTime = 0;

    // MediaCodec
    //private Thread OutputRunner;
    //private boolean isOutputRunning = false;
    //private MediaCodec codec = null;
    //private Surface surface = null;
    ImageWriter pImageWriter = null;
    int[] frameBytes;

    int frameStride = 0;
    @Override
    public void onPacketReceived(VirtualPacketTypes type, byte[] frameData, int len) {
        try {
            if (pHolder == null || tcpClient == null || !tcpClient.IsConnected || frameData == null)
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

// MediaCodec
//                        if (OutputRunner != null) {
//                            isOutputRunning = false;
//                            OutputRunner.join();
//                            OutputRunner = null;
//                        }

                        if (pImageWriter == null)
                            pImageWriter = ImageWriter.newInstance(pHolder.getSurface(), 1);

                        if (pCodecContext != null) {
                            avcodec_close(pCodecContext);
                            av_free(pCodecContext);
                            av_frame_free(pFrame);
                            av_frame_free(pFrameRGB);
                            pCodecContext = null;
                            pFrame = null;
                            pFrameRGB = null;
                            av_free(pFrameDataPointer);
                            pFrameDataPointer = null;
                        }


// MediaCodec
//
//                        if (codec != null ) {
//                            try {
//                                codec.flush();
//                                codec.stop();
//                            }
//                            catch (Exception e) {
//                                e.printStackTrace();
//                            }
//                            codec.release();
//                            codec = null;
//                        }

                        ByteBuffer bb = ByteBuffer.wrap(frameData);
                        bb.order(ByteOrder.LITTLE_ENDIAN);
                        int magic = bb.getInt();
                        int width = bb.getInt();
                        int height = bb.getInt();
                        int freq = bb.getInt();

                        if (width > 0 && height > 0) {
//                            String codecName = "video/avc";
//                            codec = MediaCodec.createDecoderByType(codecName);
//                            MediaFormat format = MediaFormat.createVideoFormat(codecName, width, height);
//                            format.setInteger(MediaFormat.KEY_WIDTH, width);
//                            format.setInteger(MediaFormat.KEY_HEIGHT, height);
//                            format.setInteger(MediaFormat.KEY_MAX_WIDTH, width);
//                            format.setInteger(MediaFormat.KEY_MAX_HEIGHT, height);
//                            format.setInteger(MediaFormat.KEY_FRAME_RATE, freq);
//                            format.setInteger(MediaFormat.KEY_OPERATING_RATE, freq);
//                            format.setInteger(MediaFormat.KEY_PRIORITY, 0);
//                            codec.configure(format, surface, null, 0);
//                            codec.start();

                            pCodec = avcodec_find_decoder(avcodec.AV_CODEC_ID_H264);
                            pCodecContext = avcodec_alloc_context3(pCodec);
                            pCodecContext.height(height);
                            pCodecContext.width(width);
                            AVRational fps = new AVRational();
                            fps.den(freq);
                            fps.num(1);
                            pCodecContext.framerate(fps);
                            pCodecContext.pix_fmt(AV_PIX_FMT_YUV420P);
                            pFrame = av_frame_alloc();
                            pFrameRGB = av_frame_alloc();
                            avcodec_open2(pCodecContext, pCodec, dict);

                            int framePacketSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 32);
                            frameBytes = new int[av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 32)];
                            pFrameDataPointer = new BytePointer(av_malloc(framePacketSize));
                            //avpicture_fill(new AVPicture(pFrameRGB), pFrameDataPointer, AV_PIX_FMT_RGB32, pCodecContext.width(), pCodecContext.height());


                            pSwsContext = sws_getContext(pCodecContext.width(), pCodecContext.height(), pCodecContext.pix_fmt(), pCodecContext.width(), pCodecContext.height(), AV_PIX_FMT_RGB32, SWS_BILINEAR, null, null, (DoublePointer)null);

                            State = 1;
//                            OutputRunner = new Thread () {
//                                @Override
//                                public void run() {
//                                    OutFrameCount = 0;
//                                    MediaCodec.BufferInfo buffInfo = new MediaCodec.BufferInfo();
//                                    while (isOutputRunning) {
//                                        int outIndex = codec.dequeueOutputBuffer(buffInfo, 1000);
//                                        switch (outIndex) {
//                                            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
//                                                break;
//                                            case MediaCodec.INFO_TRY_AGAIN_LATER:
//                                                break;
//                                            case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
//                                                break;
//                                            default:
//                                                OutFrameCount++;
//                                                codec.releaseOutputBuffer(outIndex, true);
//                                                break;
//                                        }
//                                    }
//                                }
//                            };
//                            isOutputRunning = true;
//                            OutputRunner.start();
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
                                pFrameDataPointer.put(frameData, 0, len);
                                pPacket.size(len).position(0).data(pFrameDataPointer);

                                //Log.d("SSS", "PktSize: " + pPacket.size());

                                if (0 == avcodec_send_packet(pCodecContext, pPacket)) {
                                    if (0 == avcodec_receive_frame(pCodecContext, pFrame)) {

                                        sws_scale(pSwsContext, pFrame.data(), pFrame.linesize(), 0, pCodecContext.height(), pFrameRGB.data(), pFrameRGB.linesize());
                                        //Log.d("SSS", "FrameLineSize: " + pFrame.linesize(0) + "," + pFrame.linesize(1) + "," + pFrame.linesize(2));
                                        //Log.d("SSS", "FrameFormat: " + pFrame.format());
                                        //Log.d("SSS", "FrameDur: " + pFrame.pkt_duration());
                                        //Log.d("SSS", "FrameHeight: " + pFrame.height());
                                        byte[] bb = new byte[pFrameRGB.pkt_size()];
                                        pFrameRGB.data(0).asByteBuffer().get(bb, 0, pFrameRGB.pkt_size());
                                        //decodeYUV(frameBytes, bb , pCodecContext.width(), pCodecContext.height());


                                        Canvas c = pHolder.lockCanvas(null);
                                        if (c != null) {
                                            try {
                                                Bitmap bmp = Bitmap.createBitmap(frameBytes, 0, pCodecContext.width(), pCodecContext.height(), pFrameRGB.linesize(0), Bitmap.Config.ARGB_8888);
                                                //Bitmap bmp = BitmapFactory.decodeByteArray(bb.array(), 0, pFrame.pkt_size());
                                                if (bmp != null)
                                                    c.setBitmap(bmp);
                                            }
                                            finally {
                                                pHolder.unlockCanvasAndPost(c);
                                            }
                                        }
                                        //Log.d("AVCODEC", "Decoded: " + pFrame.toString());
                                    }
                                }


//                                int inIndex = codec.dequeueInputBuffer(1000);
//                                if (inIndex >= 0) {
//                                    ByteBuffer inputBuffer = codec.getInputBuffer(inIndex);
//                                    if (inputBuffer != null) {
//                                        inputBuffer.clear();
//                                        inputBuffer.put(frameData, 0, len);
//                                        codec.queueInputBuffer(inIndex, 0, len, 1, 0);
//                                    }
//                                }
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

    public void decodeYUV(int[] out, byte[] fg, int width, int height)
            throws NullPointerException, IllegalArgumentException {
        int sz = width * height;
        if (out == null)
            throw new NullPointerException("buffer out is null");
        if (out.length < sz)
            throw new IllegalArgumentException("buffer out size " + out.length
                    + " < minimum " + sz);
        if (fg == null)
            throw new NullPointerException("buffer 'fg' is null");
        if (fg.length < sz)
            throw new IllegalArgumentException("buffer fg size " + fg.length
                    + " < minimum " + sz * 3 / 2);
        int i, j;
        int Y, Cr = 0, Cb = 0;
        for (j = 0; j < height; j++) {
            int pixPtr = j * width;
            final int jDiv2 = j >> 1;
            for (i = 0; i < width; i++) {
                Y = fg[pixPtr];
                if (Y < 0)
                    Y += 255;
                if ((i & 0x1) != 1) {
                    final int cOff = sz + jDiv2 * width + (i >> 1) * 2;
                    Cb = fg[cOff];
                    if (Cb < 0)
                        Cb += 127;
                    else
                        Cb -= 128;
                    Cr = fg[cOff + 1];
                    if (Cr < 0)
                        Cr += 127;
                    else
                        Cr -= 128;
                }
                int R = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
                if (R < 0)
                    R = 0;
                else if (R > 255)
                    R = 255;
                int G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1)
                        + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                if (G < 0)
                    G = 0;
                else if (G > 255)
                    G = 255;
                int B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
                if (B < 0)
                    B = 0;
                else if (B > 255)
                    B = 255;

                out[pixPtr++] = 0xff000000 + (B << 16) + (G << 8) + R;
            }
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
