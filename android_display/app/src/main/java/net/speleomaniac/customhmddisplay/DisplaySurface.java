package net.speleomaniac.customhmddisplay;

import android.content.Context;
import android.opengl.GLSurfaceView;


public class DisplaySurface extends GLSurfaceView {

    public DisplayRenderer mRenderer;

    public DisplaySurface(Context context) {
        super(context);

        // Create an OpenGL ES 2.0 context.
        setEGLContextClientVersion(2);
    }

    @Override
    public void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        mRenderer.onPause();
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        mRenderer.onResume();
    }

}