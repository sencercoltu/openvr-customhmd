package net.speleomaniac.customhmddisplay;

import android.content.Context;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;

import static android.media.MediaCodec.CONFIGURE_FLAG_ENCODE;

//use old camera api, new one pain in @ss

public class CameraGrabber extends Thread {

    interface CameraFrameReceiveListener {
        void onCameraFrameReceived(byte[] data, int size);
    }

    private CameraFrameReceiveListener cameraFrameReceiveListener = null;
    private boolean isCameraRunnig = false;
    private DisplayActivity mContext = null;
    private Surface mSurface;
    private Camera mCamera;
    //private Handler mHandler = null;
    private SurfaceView mSurfaceView = null;


    public CameraGrabber(DisplayActivity context) {
        cameraFrameReceiveListener = context;
        mContext = context;
        //imageReader = ImageReader.newInstance(320, 240, ImageFormat.YUV_420_888, 1);
        //mSurface = imageReader.getSurface();
        //mHandler = new Handler(mContext.getMainLooper());
        mSurfaceView = new SurfaceView(mContext);
        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                mSurface = holder.getSurface(); //.createPersistentInputSurface();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                mSurface = holder.getSurface();
                if (mCamera != null) {
                    disconnect();
                    start();
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }

    @Override
    public synchronized void start() {
        if (isCameraRunnig)
            return;
        isCameraRunnig = true;
        super.start();
    }

    void disconnect() {
        if (!isCameraRunnig)
            return;
        isCameraRunnig = false;
        try {
            join();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    private final Object cameraCreateLock = new Object();

    private MediaCodec encoder = null;

    private Camera.PreviewCallback previewCallback = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            if (encoder != null) {
                int inIndex = encoder.dequeueInputBuffer(1);
                if (inIndex >= 0) {
                    ByteBuffer inputBuffer = encoder.getInputBuffer(inIndex);
                    if (inputBuffer != null) {
                        inputBuffer.clear();
                        inputBuffer.put(data, 0, data.length);
                        long ptsUsec = System.currentTimeMillis() * 1000;
                        encoder.queueInputBuffer(inIndex, 0, data.length, ptsUsec, CONFIGURE_FLAG_ENCODE);
                    }
                }
            }
        }
    };

    @Override
    public void run() {
        Camera.CameraInfo ci = new Camera.CameraInfo();
        for (int i=0; i<Camera.getNumberOfCameras(); i++) {
            Camera.getCameraInfo(i, ci);
            if (ci.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                mCamera = Camera.open(i);
                break;
            }
        }

        if (mCamera == null)
            return;


        try {
            Camera.Parameters parameters = mCamera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            parameters.setPreviewSize(320, 240);
            mCamera.setParameters(parameters);
            mCamera.setPreviewDisplay(mSurfaceView.getHolder());
            mCamera.setPreviewCallback(previewCallback);
        } catch (IOException e) {
            e.printStackTrace();
        }



        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        MediaFormat format = MediaFormat.createVideoFormat("video/avc", 320, 240);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        format.setInteger(MediaFormat.KEY_BIT_RATE, 300000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

        try {
            encoder = MediaCodec.createEncoderByType("video/avc");
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }


//        while (!mSurface.isValid()) {
//            try {
//                Thread.sleep(1);
//            } catch (InterruptedException e) {
//                e.printStackTrace();
//            }
//        }



        encoder.configure(format, null, null, CONFIGURE_FLAG_ENCODE);
        Surface surf = MediaCodec.createPersistentInputSurface();
        //encoder.setInputSurface(mSurface);
        encoder.start();
        mCamera.startPreview();
        if (mCamera != null) {

            while (isCameraRunnig) {
                while (true) {
                    int encoderStatus = encoder.dequeueOutputBuffer(bufferInfo, 10);
                    switch (encoderStatus) {
                        case MediaCodec.INFO_TRY_AGAIN_LATER:
                            break;
                        case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                            break;
                        case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                            MediaFormat newFormat = encoder.getOutputFormat();
                            break;
                        default:
                            if (encoderStatus < 0) //ignore
                                break;
                            ByteBuffer encodedData = encoder.getOutputBuffer(encoderStatus);
                            if (encodedData == null) {
                                throw new RuntimeException("encoderOutputBuffer " + encoderStatus + " was null");
                            }
                            byte[] bytes = new byte[encodedData.limit()];
                            encodedData.position(0);
                            encodedData.get(bytes);
                            cameraFrameReceiveListener.onCameraFrameReceived(bytes, bytes.length);
                            encoder.releaseOutputBuffer(encoderStatus, false);
                            break;
                    }
                }
            }

            mCamera.stopPreview();
            mCamera.release();
            mCamera = null;
        }

        encoder.stop();
        encoder.release();
        encoder = null;
    }

    /*
    @Override
    public void onImageAvailable(ImageReader reader) {
        Image image = null;
        try {
            image = reader.acquireLatestImage();
            ByteBuffer bufferY = image.getPlanes()[0].getBuffer();
            ByteBuffer bufferCb = image.getPlanes()[1].getBuffer();
            ByteBuffer bufferCr = image.getPlanes()[2].getBuffer();

            ByteBuffer buffer = ByteBuffer.allocate(bufferY.capacity() + bufferCb.capacity() + bufferCr.capacity());
            buffer.put(bufferY).put(bufferCb).put(bufferCr);
            byte[] bytes = new byte[buffer.capacity()];
            buffer.position(0);
            buffer.get(bytes);

            if (encoder != null) {
                int inIndex = encoder.dequeueInputBuffer(1);
                if (inIndex >= 0) {
                    ByteBuffer inputBuffer = encoder.getInputBuffer(inIndex);
                    if (inputBuffer != null) {
                        inputBuffer.clear();
                        inputBuffer.put(bytes, 0, bytes.length);
                        encoder.queueInputBuffer(inIndex, 0, bytes.length, 1, 0);
                    }
                }
            }
        }
        catch (Exception ee) {
        }
        finally {
            if(image!=null)
                image.close();
        }
    }
    */
}
