package net.speleomaniac.customhmddisplay;

import android.opengl.GLES20;

/**
 * Created by Sencer Coltu on 18/04/2017.
 */

public class DisplayShader {
    // Program variables
    public static int sp_SolidColor;

    /* SHADER Image
 *
 * This shader is for rendering 2D images straight from a texture
 * No additional effects.
 *
 */

    public static int a_texCoord;
    public static int v_texCoord;
    public static int s_texture;
    public static int v_Position;
    public static int u_MVPMatrix;

    public static void compileAll() {

        // Create the shaders, images
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vs_Image);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fs_Image);

        sp_Image = GLES20.glCreateProgram();
        GLES20.glAttachShader(sp_Image, vertexShader);
        GLES20.glAttachShader(sp_Image, fragmentShader);
        GLES20.glLinkProgram(sp_Image);

        // Set our shader programm
        GLES20.glUseProgram(sp_Image);

        // get handle to vertex shader's vPosition member
        v_Position = GLES20.glGetAttribLocation(DisplayShader.sp_Image, "v_Position");

        // Get handle to texture coordinates location
        a_texCoord = GLES20.glGetAttribLocation(DisplayShader.sp_Image, "a_texCoord" );

        // Get handle to shape's transformation matrix
        u_MVPMatrix = GLES20.glGetUniformLocation(DisplayShader.sp_Image, "u_MVPMatrix");

        // Get handle to textures locations
        s_texture = GLES20.glGetUniformLocation (DisplayShader.sp_Image, "s_texture" );


    }


    public static final String vs_Image =
            "uniform mat4 u_MVPMatrix;" +
            "attribute vec4 v_Position;" +
            "attribute vec2 a_texCoord;" +
            "varying vec2 v_texCoord;" +
            "void main() {" +
            "  gl_Position = u_MVPMatrix * v_Position;" +
            "  v_texCoord = a_texCoord;" +
            "}";
    public static final String fs_Image =
            "precision mediump float;" +
            "varying vec2 v_texCoord;" +
            "uniform sampler2D s_texture;" +
            "void main() {" +
            "  gl_FragColor = texture2D( s_texture, v_texCoord );" +
            "}";

    public static int sp_Image;

    public static int loadShader(int type, String shaderCode) {

        // create a vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        // add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        // return the shader
        return shader;
    }
}

