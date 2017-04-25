package net.speleomaniac.customhmddisplay;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaCodec;
import android.media.MediaDataSource;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.regex.Matcher;

public class DisplayActivity
        extends Activity
        implements SurfaceHolder.Callback,
                   UdpComm.PacketReceiveListener,
                    SensorEventListener {

    private UdpComm udpSocket = null;
    private MediaCodec codec;
    private Surface surface;

    private SensorManager sensorManager;
    private UsbPacket usbPacket = new UsbPacket();


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

        udpSocket = new UdpComm(1974);
        udpSocket.setPacketReceiveListener(this);

        usbPacket.Header.Type = (byte)(UsbPacket.HMD_SOURCE | UsbPacket.ROTATION_DATA);
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
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


    private byte[] InfoPacket = new byte[4 + 4 + 4]; //magic width height
    private boolean Initialized = false;
    private final String CodecName = "video/avc"; //"video/x-vnd.on2.vp8"; //"video/avc"

    @Override
    public void onPacketReceived(DatagramPacket packet) {
        if (surface == null) return;
        byte[] frameData = packet.getData();
        int len = packet.getLength();

//        if (!Initialized) {
//            try {
//
//                InetAddress driverAddr = packet.getAddress();
//                udpSocket.setDriverAddress(driverAddr);
//                codec = MediaCodec.createDecoderByType();
//                MediaFormat format = MediaFormat.createVideoFormat("video/avc", 1920, 1080);
//                format.setInteger(MediaFormat.KEY_MAX_WIDTH, 1920);
//                format.setInteger(MediaFormat.KEY_MAX_HEIGHT, 1080);
//
//                codec.configure(format, surface, null, 0);
//                codec.start();
//            } catch (IOException e) {
//                throw new RuntimeException(e.getMessage());
//            }
//
//            Initialized = true;
//        }



        if (len == 12 && frameData[0] == 'H' && frameData[1] == 'M' && frameData[2] == 'D' && frameData[3] == 'D') {
            Initialized = false;

            if (codec != null ) {
                codec.flush();
                codec.stop();
                codec.release();
                codec = null;
            }

            ByteBuffer bb = ByteBuffer.wrap(frameData);
            bb.order(ByteOrder.LITTLE_ENDIAN);
            int magic = bb.getInt();
            int width = bb.getInt();
            int height = bb.getInt();

            if (width > 0 && height > 0) {
                try {
                    InetAddress driverAddr = packet.getAddress();
                    udpSocket.setDriverAddress(driverAddr);
                    codec = MediaCodec.createDecoderByType(CodecName);
                    MediaFormat format = MediaFormat.createVideoFormat(CodecName, width, height);
                    format.setInteger(MediaFormat.KEY_MAX_WIDTH, width);
                    format.setInteger(MediaFormat.KEY_MAX_HEIGHT, height);

                    codec.configure(format, surface, null, 0);
                    codec.start();
                } catch (IOException e) {
                    throw new RuntimeException(e.getMessage());
                }
                Initialized = true;
            }
            return;
        }

        if (!Initialized)
            return;

        /*Edit: This part may be left out*/
        /*
        int NAL_START = 1;
        //103, 104 -> SPS, PPS  | 101 -> Data
        int id = 0;
        int dataOffset = 0;

        //Later on this will be serversided, but for now...
        //Separate the SPSPPS from the Data
        for (int i = 0; i < frameData.length - 4; i++) {
            id = frameData[i] << 24 | frameData[i + 1] << 16 | frameData[i + 2] << 8 | frameData[i + 3];
            if (id == NAL_START) {
                if (frameData[i + 4] == 101) {
                    dataOffset = i;
                }
            }
        }

        byte[] SPSPPS = Arrays.copyOfRange(frameData, 0, dataOffset);
        byte[] data = Arrays.copyOfRange(frameData, dataOffset, frameData.length);

        if (SPSPPS.length != 0) {
            int inIndex = codec.dequeueInputBuffer(100000);

            if (inIndex >= 0) {
                ByteBuffer input = codec.getInputBuffer(inIndex);
                input.clear();
                input.put(SPSPPS);
                codec.queueInputBuffer(inIndex, 0, SPSPPS.length, 16, MediaCodec.BUFFER_FLAG_CODEC_CONFIG);
            }
            inIndex = codec.dequeueInputBuffer(10000);
            if (inIndex >= 0) {
                ByteBuffer inputBuffer = codec.getInputBuffer(inIndex);
                inputBuffer.clear();
                inputBuffer.put(data);
                codec.queueInputBuffer(inIndex, 0, frameData.length, 16, 0);
            }
        }
        */
        /*Edit end*/
        //else {
            int inIndex = codec.dequeueInputBuffer(10000);
            if (inIndex >= 0) {
                ByteBuffer inputBuffer = codec.getInputBuffer(inIndex);
                inputBuffer.clear();
                inputBuffer.put(frameData);
                codec.queueInputBuffer(inIndex, 0, frameData.length, 16, 0);
            }
        //}
        MediaCodec.BufferInfo buffInfo = new MediaCodec.BufferInfo();
        int outIndex = codec.dequeueOutputBuffer(buffInfo, 10000);

        switch (outIndex) {
            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                break;
            case MediaCodec.INFO_TRY_AGAIN_LATER:
                break;
            case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED: // -3: //This solves it
                //ByteBuffer buffer = codec.getOutputBuffer(outIndex);
                break;
            default:
                codec.releaseOutputBuffer(outIndex, true);
                //codec.flush();
                break;
        }
    }

    //Quaternion rotate = new Quaternion(1, 0, 0, 0).rotateByAngleX(0);
    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
            Quaternion q = new Quaternion(event.values[3], event.values[0], event.values[1], event.values[2]);
            //q = q.multiply(rotate);

            usbPacket.Rotation.w = q.w;
            usbPacket.Rotation.x = q.x;
            usbPacket.Rotation.y = q.y;
            usbPacket.Rotation.z = q.z;
            if (udpSocket != null) {
                usbPacket.buildRotationPacket();
                udpSocket.sendPacket(usbPacket);
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    protected void onPause() {
        sensorManager.unregisterListener(this);
        udpSocket.interrupt();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        udpSocket.start();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST);
    }


}
