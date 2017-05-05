package net.speleomaniac.customhmddisplay;

import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.view.Surface;
import android.view.SurfaceView;
import android.widget.FrameLayout;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Collections;

/**
 * Created by Sencer Coltu on 05/05/2017.
 */

public class CameraGrabber extends Thread
        /*implements ImageReader.OnImageAvailableListener*/ {

    //private ImageReader imageReader = null;
    private boolean isCameraRunnig = false;
    private DisplayActivity mContext = null;
    private SurfaceView surfaceView = null;
    private Surface mSurface;
    

    public CameraGrabber(DisplayActivity context) {
        mContext = context;
        //imageReader = ImageReader.newInstance(320, 240, ImageFormat.YUV_420_888, 1);
        //mSurface = imageReader.getSurface();
        mHandler = new Handler(mContext.getMainLooper());
        surfaceView = new SurfaceView(mContext);
        mContext.addContentView(surfaceView, new FrameLayout.LayoutParams(320, 240));
        surfaceView.getHolder().setFixedSize(320, 240);
        mSurface = surfaceView.getHolder().getSurface();
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

    private CameraDevice mCameraDevice = null;
    private CaptureRequest.Builder mPreviewBuilder = null;
    private CameraCaptureSession mPreviewSession = null;
    private Handler mHandler = null;

    private CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            synchronized (cameraCreateLock) {
                cameraCreateLock.notify();
            }

            try {
                mPreviewBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            } catch (CameraAccessException e){
                e.printStackTrace();
            }

            mPreviewBuilder.addTarget(mSurface);

            try {
                mCameraDevice.createCaptureSession(Collections.singletonList(mSurface), mPreviewStateCallback, mHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            mCameraDevice = null;
            camera.close();
            cameraCreateLock.notify();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            mCameraDevice = null;
            camera.close();
            cameraCreateLock.notify();
        }
    };

    private CameraCaptureSession.StateCallback mPreviewStateCallback = new CameraCaptureSession.StateCallback() {

        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mPreviewSession = session;

            mPreviewBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
            try {
                mPreviewSession.setRepeatingRequest(mPreviewBuilder.build(), null, mHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            mPreviewSession = null;
        }
    };

    private final Object cameraCreateLock = new Object();

    private MediaCodec encoder = null;

    @Override
    public void run() {
        CameraManager manager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        String cameras[] = null;
        try {
            cameras = manager.getCameraIdList();
        } catch (CameraAccessException e) {
            e.printStackTrace();
            return;
        }

        if (cameras.length == 0)
            return;

        String cameraId = null;
        for (String camera1 : cameras) {
            CameraCharacteristics characteristics = null;
            try {
                characteristics = manager.getCameraCharacteristics(camera1);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
            if (characteristics == null)
                continue;

            int facing = characteristics.get(CameraCharacteristics.LENS_FACING);
            if (facing == CameraCharacteristics.LENS_FACING_BACK) {
                //use this one
                cameraId = camera1;
                break;
            }
        }

        if (cameraId == null)
            return;

        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        MediaFormat format = MediaFormat.createVideoFormat("video/avc", 320, 240);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
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



        encoder.configure(format, mSurface, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        //encoder.setInputSurface(mSurface);
        encoder.start();

        try {
            manager.openCamera(cameraId, mStateCallback, mHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        synchronized (cameraCreateLock) {
            try {
                cameraCreateLock.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mCameraDevice != null) {

            while (isCameraRunnig) {
                while (true) {
                    int encoderStatus = encoder.dequeueOutputBuffer(bufferInfo, 10);
                    if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {

                    } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        MediaFormat newFormat = encoder.getOutputFormat();
                    } else if (encoderStatus < 0) {
                        // let's ignore it
                    } else {
                        ByteBuffer encodedData = encoder.getOutputBuffer(encoderStatus);
                        if (encodedData == null) {
                            throw new RuntimeException("encoderOutputBuffer " + encoderStatus + " was null");
                        }

                        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                            // The codec config data was pulled out and fed to the muxer when we got
                            // the INFO_OUTPUT_FORMAT_CHANGED status.  Ignore it.
                            bufferInfo.size = 0;
                        }

                        if (bufferInfo.size != 0) {
                            // adjust the ByteBuffer values to match BufferInfo (not needed?)
                            encodedData.position(bufferInfo.offset);
                            encodedData.limit(bufferInfo.offset + bufferInfo.size);
                            //write to remote
                        }

                        encoder.releaseOutputBuffer(encoderStatus, false);

                        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                            break;      // out of while
                        }
                    }
                }
            }

            if (mPreviewSession != null)
                mPreviewSession.close();
            mPreviewSession = null;

            mCameraDevice.close();
            mCameraDevice = null;
        }
        encoder.stop();
        encoder.release();
        encoder = null;

        if (mSurface != null)
            mSurface.release();
        mSurface =  null;
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
