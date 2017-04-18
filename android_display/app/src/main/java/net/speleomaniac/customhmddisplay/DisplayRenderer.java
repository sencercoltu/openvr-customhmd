package net.speleomaniac.customhmddisplay;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class DisplayRenderer implements GLSurfaceView.Renderer {

    public final Object imageLock = new Object();

    // Our matrices
    private final float[] mtrxProjection = new float[16];
    private final float[] mtrxView = new float[16];
    private final float[] mtrxProjectionAndView = new float[16];

    // Geometric variables
    public static float left_vertices[];
    public static float right_vertices[];
    public static short indices[];
    public FloatBuffer left_vertexBuffer;
    public FloatBuffer right_vertexBuffer;
    public ShortBuffer drawListBuffer;

    public Bitmap left_Image = null;
    public static float left_uvs[];
    public FloatBuffer left_uvBuffer;
    public Bitmap right_Image = null;
    //public static float right_uvs[];
    //public FloatBuffer right_uvBuffer;

    // Our screenresolution
    float mScreenWidth = 1920;
    float mScreenHeight = 1080;

    // Misc
    Context mContext;
    //long mLastTime;
    int mProgram;

    public DisplayRenderer(Context c) {
        mContext = c;
        //mLastTime = System.currentTimeMillis() + 100;


    }

    public void onPause() {
        /* Do stuff to pause the renderer */
    }

    public void onResume() {
        /* Do stuff to resume the renderer */
        //mLastTime = System.currentTimeMillis();
    }

    @Override
    public void onDrawFrame(GL10 unused) {
        Render(mtrxProjectionAndView);
    }

    int[] texturenames = new int[2];

    public void PrepareTexture()
    {
        // Create our UV coordinates.
        left_uvs = new float[] {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
                1.0f, 0.0f
        };

//        right_uvs = new float[] {
//                0.5f, 0.0f,
//                0.5f, 1.0f,
//                1.0f, 1.0f,
//                1.0f, 0.0f
//        };

        // The texture buffer
        ByteBuffer left_bb = ByteBuffer.allocateDirect(left_uvs.length * 4);
        left_bb.order(ByteOrder.nativeOrder());
        left_uvBuffer = left_bb.asFloatBuffer();
        left_uvBuffer.put(left_uvs);
        left_uvBuffer.position(0);

//        ByteBuffer right_bb = ByteBuffer.allocateDirect(right_uvs.length * 4);
//        right_bb.order(ByteOrder.nativeOrder());
//        right_uvBuffer = right_bb.asFloatBuffer();
//        right_uvBuffer.put(right_uvs);
//        right_uvBuffer.position(0);

        // Generate Textures, if more needed, alter these numbers.

        GLES20.glGenTextures(2, texturenames, 0);

        // Bind texture to texturename
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[0]);

        // Set filtering
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        // Set wrapping mode
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[1]);

        // Set filtering
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        // Set wrapping mode
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);

    }

    private void setLeftImage() {
        Bitmap bmp;
        // Prepare the triangle coordinate data
        GLES20.glVertexAttribPointer(DisplayShader.v_Position, 3, GLES20.GL_FLOAT, false, 0, left_vertexBuffer);
        // Prepare the texturecoordinates
        GLES20.glUniform1i ( DisplayShader.s_texture, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        if (left_Image != null) {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[0]);
            synchronized (imageLock) {
                bmp = left_Image;
                left_Image = null;
            }
            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0);
            bmp.recycle();
        }
        else
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[0]);
    }

    private void setRightImage() {
        Bitmap bmp;
        // Prepare the triangle coordinate data
        GLES20.glVertexAttribPointer(DisplayShader.v_Position, 3, GLES20.GL_FLOAT, false, 0, right_vertexBuffer);
        // Prepare the texturecoordinates
        GLES20.glUniform1i ( DisplayShader.s_texture, 1);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        if (right_Image != null) {
            synchronized (imageLock) {
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[1]);
                bmp = right_Image;
                right_Image = null;
            }
            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0);
            bmp.recycle();
        }
        else
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texturenames[1]);

    }


    private void Render(float[] m) {

        // clear Screen and Depth Buffer,
        // we have set the clear color as black.
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        // Enable generic vertex attribute array
        GLES20.glEnableVertexAttribArray(DisplayShader.v_Position);
        // Enable generic vertex attribute array
        GLES20.glEnableVertexAttribArray (DisplayShader.a_texCoord);
        // Apply the projection and view transformation
        GLES20.glUniformMatrix4fv(DisplayShader.u_MVPMatrix, 1, false, m, 0);

        GLES20.glVertexAttribPointer ( DisplayShader.a_texCoord, 2, GLES20.GL_FLOAT, false, 0, left_uvBuffer);


        setLeftImage();
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, indices.length, GLES20.GL_UNSIGNED_SHORT, drawListBuffer);


        setRightImage();
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, indices.length, GLES20.GL_UNSIGNED_SHORT, drawListBuffer);


        // Disable vertex array
        GLES20.glDisableVertexAttribArray(DisplayShader.v_Position);
        GLES20.glDisableVertexAttribArray(DisplayShader.a_texCoord);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

        // We need to know the current width and height.
        mScreenWidth = width;
        mScreenHeight = height;

        // Redo the Viewport, making it fullscreen.
        GLES20.glViewport(0, 0, (int) mScreenWidth, (int) mScreenHeight);

        // Clear our matrices
        for (int i = 0; i < 16; i++) {
            mtrxProjection[i] = 0.0f;
            mtrxView[i] = 0.0f;
            mtrxProjectionAndView[i] = 0.0f;
        }

        // Setup our screen width and height for normal sprite translation.
        Matrix.orthoM(mtrxProjection, 0, 0f, 1.0f, 0.0f, 1.0f, 0, 50);

        // Set the camera position (View matrix)
        Matrix.setLookAtM(mtrxView, 0, 0f, 0f, 1f, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

        // Calculate the projection and view transformation
        Matrix.multiplyMM(mtrxProjectionAndView, 0, mtrxProjection, 0, mtrxView, 0);

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        // Create the triangles
        SetupTriangle();
        // Create the image information
        PrepareTexture();

        // Set the clear color to black
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1);

        DisplayShader.compileAll();
    }

    public void SetupTriangle() {
        // We have create the vertices of our view.
        // We have to create the vertices of our triangle.
        left_vertices = new float[]
                {  0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 0.0f,
                   0.5f, 0.0f, 0.0f,
                   0.5f, 1.0f, 0.0f
                };

        right_vertices = new float[]
                {  0.5f, 1.0f, 0.0f,
                   0.5f, 0.0f, 0.0f,
                   1.0f, 0.0f, 0.0f,
                   1.0f, 1.0f, 0.0f
                };


        indices = new short[] {0, 1, 2, 0, 2, 3}; // The order of vertexrendering.

        // The vertex buffer.
        ByteBuffer left_bb = ByteBuffer.allocateDirect(left_vertices.length * 4);
        left_bb.order(ByteOrder.nativeOrder());
        left_vertexBuffer = left_bb.asFloatBuffer();
        left_vertexBuffer.put(left_vertices);
        left_vertexBuffer.position(0);

        ByteBuffer right_bb = ByteBuffer.allocateDirect(right_vertices.length * 4);
        right_bb.order(ByteOrder.nativeOrder());
        right_vertexBuffer = right_bb.asFloatBuffer();
        right_vertexBuffer.put(right_vertices);
        right_vertexBuffer.position(0);

        // initialize byte buffer for the draw list
        ByteBuffer dlb = ByteBuffer.allocateDirect(indices.length * 2);
        dlb.order(ByteOrder.nativeOrder());
        drawListBuffer = dlb.asShortBuffer();
        drawListBuffer.put(indices);
        drawListBuffer.position(0);
    }
}