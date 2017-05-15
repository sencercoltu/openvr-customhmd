package net.speleomaniac.customhmddisplay;

import android.util.FloatMath;

class Quaternion {
    public float w, x, y, z;

    // create a new object with the given components
    public Quaternion(float w, float x, float y, float z) {
        this.w = w;
        this.x = x;
        this.y = y;
        this.z = z;
    }

    // normalize
    public Quaternion normalize () {
        float len = norm();
        if (len != 0.f) {
            len = (float)Math.sqrt(len);
            float w = this.w / len;
            float x = this.x / len;
            float y = this.y / len;
            float z = this.z / len;
            return new Quaternion(w, x, y, z);
       }
       else
            return new Quaternion(this.w, this.x, this.y, this.z);
    }

    // return the quaternion norm
    public float norm() {
        return (float)Math.sqrt(w * w + x * x + y * y + z * z);
    }

    // return the quaternion conjugate
    public Quaternion conjugate() {
        return new Quaternion(w, -x, -y, -z);
    }

    // return a new Quaternion whose value is (this + b)
    public Quaternion add(Quaternion b) {
        Quaternion a = this;
        return new Quaternion(a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z);
    }


    // return a new Quaternion whose value is (this * b)
    public Quaternion multiply(Quaternion b) {
        Quaternion a = this;
        float w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
        float x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
        float y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
        float z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
        return new Quaternion(w, x, y, z);
    }

    // return a new Quaternion whose value is the inverse of this
    public Quaternion inverse() {
        float d = w * w + x * x + y * y + z * z;
        return new Quaternion(w / d, -x / d, -y / d, -z / d);
    }


    // return a / b
    // we use the definition a * b^-1 (as opposed to b^-1 a)
    public Quaternion divide(Quaternion b) {
        Quaternion a = this;
        return a.multiply(b.inverse());
    }

    public Quaternion rotateByAngleX(float angle) {
        final float halfAngle = 0.5f * angle;
        final float sin = (float) Math.sin(halfAngle);
        final float cos = (float) Math.cos(halfAngle);
        return new Quaternion(
                -x * sin + w * cos,
                x * cos + w * sin,
                y * cos + z * sin,
                -y * sin + z * cos
        );
    }

    public Quaternion rotateByAngleY(final float angle) {
        final float halfAngle = 0.5f * angle;
        final float sin = (float) Math.sin(halfAngle);
        final float cos = (float) Math.cos(halfAngle);
        return new Quaternion(
                -y * sin + w * cos,
                x * cos - z * sin,
                y * cos + w * sin,
                x * sin + z * cos
        );
    }

    public Quaternion rotateByAngleZ(final float angle) {
        final float halfAngle = 0.5f * angle;
        final float sin = (float) Math.sin(halfAngle);
        final float cos = (float) Math.cos(halfAngle);
        return new Quaternion(
                -z * sin + w * cos,
                x * cos + y * sin,
                -x * sin + y * cos,
                z * cos + w * sin
        );
    }
}