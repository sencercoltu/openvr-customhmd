package net.speleomaniac.customhmddisplay;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DisplayActivity
        extends Activity
        implements SurfaceHolder.Callback,
                   TcpClient.PacketReceiveListener,
                    SensorEventListener {

    private TcpClient tcpClient = null;
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
    private final String CodecName = "video/avc"; //"video/x-vnd.on2.vp8"; //"video/avc"
    private int State = 0;
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
                                codec = MediaCodec.createDecoderByType(CodecName);
                                MediaFormat format = MediaFormat.createVideoFormat(CodecName, width, height);
                                format.setInteger(MediaFormat.KEY_MAX_WIDTH, width);
                                format.setInteger(MediaFormat.KEY_MAX_HEIGHT, height);
                                codec.configure(format, surface, null, 0);
                                codec.start();
                            State = 1;
                        }
                    }
                    else {
                        tcpClient.reconnect();
                    }

                }
                break;
                case 1: {
                    int inIndex = codec.dequeueInputBuffer(10000);
                    if (inIndex >= 0) {
                        ByteBuffer inputBuffer = codec.getInputBuffer(inIndex);
                        inputBuffer.clear();
                        inputBuffer.put(frameData, 0, len);
                        codec.queueInputBuffer(inIndex, 0, len, 1, 0);
                    }

                    MediaCodec.BufferInfo buffInfo = new MediaCodec.BufferInfo();
                    int outIndex = codec.dequeueOutputBuffer(buffInfo, 10000);

                    switch (outIndex) {
                        case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                            break;
                        case MediaCodec.INFO_TRY_AGAIN_LATER:
                            break;
                        case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                            break;
                        default:
                            codec.releaseOutputBuffer(outIndex, true);
                            break;
                    }
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //Quaternion rotate = new Quaternion(1, 0, 0, 0).rotateByAngleX(0);
    @Override
    public void onSensorChanged(SensorEvent event) {
        if (State == 0)
            return;
        if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
            Quaternion q = new Quaternion(event.values[3], event.values[0], event.values[1], event.values[2]);
            //q = q.multiply(rotate);

            usbPacket.Rotation.w = q.w;
            usbPacket.Rotation.x = q.x;
            usbPacket.Rotation.y = q.y;
            usbPacket.Rotation.z = q.z;
            if (tcpClient != null && tcpClient.IsConnected) {
                usbPacket.buildRotationPacket();
                tcpClient.sendPacket(usbPacket);
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    protected void onPause() {
        sensorManager.unregisterListener(this);
        if (tcpClient != null) {
            tcpClient.disconnect();
            try {
                tcpClient.join();
            } catch (Exception e) {
                e.printStackTrace();
            }
            tcpClient = null;
        }
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
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
        sensorManager.registerListener(this, sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR), SensorManager.SENSOR_DELAY_FASTEST);
    }


}
