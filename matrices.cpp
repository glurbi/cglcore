#include <matrices.h>

void ortho(matrice44 m, float left, float right, float bottom, float top, float near, float far) {
    m[0] = 2 / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 2 / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 2 / (far - near);
    m[11] = 0.0f;
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

/*
    public static Matrix44 identity() {
        Matrix44 matrix = new Matrix44();
        matrix.m[0] = 1.0f;
        matrix.m[5] = 1.0f;
        matrix.m[10] = 1.0f;
        matrix.m[15] = 1.0f;
        return matrix;
    }

    public Matrix44 mult(Matrix44 that) {
        Matrix44 matrix = new Matrix44();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                matrix.m[i+j*4] =
                    this.m[i+0] * that.m[j*4+0] +
                    this.m[i+4] * that.m[j*4+1] +
                    this.m[i+8] * that.m[j*4+2] +
                    this.m[i+12] * that.m[j*4+3];
            }
        }
        return matrix;
    }

    public float[] mult(float[] vec4) {
        float[] result = new float[4];
        for (int i = 0; i < 4; i++) {
            result[i] =
                this.m[i+0] * vec4[0] +
                this.m[i+4] * vec4[1] +
                this.m[i+8] * vec4[2] +
                this.m[i+12] * vec4[3];
        }
        return result;
    }

    public Matrix44 frustum(float left, float right, float bottom, float top, float near, float far) {
        // Generate a perspective projection matrix as defined at:
        // http://www.opengl.org/sdk/docs/man/xhtml/glFrustum.xml
        Matrix44 matrix = new Matrix44();
        matrix.m[0] = 2 * near / (right - left);
        matrix.m[1] = 0.0f;
        matrix.m[2] = 0.0f;
        matrix.m[3] = 0.0f;
        matrix.m[4] = 0.0f;
        matrix.m[5] = 2 * near / (top - bottom);
        matrix.m[6] = 0.0f;
        matrix.m[7] = 0.0f;
        matrix.m[8] = (right + left) / (right - left);
        matrix.m[9] = (top + bottom) / (top - bottom);
        matrix.m[10] = - (far + near) / (far - near);
        matrix.m[11] = -1.0f;
        matrix.m[12] = 0.0f;
        matrix.m[13] = 0.0f;
        matrix.m[14] = -2.0f * far * near / (far - near);
        matrix.m[15] = 0.0f;
        return mult(matrix);
    }

    public Matrix44 translate(float x, float y, float z) {
        // Generate a translation matrix as defined at:
        // http://www.opengl.org/sdk/docs/man/xhtml/glTranslate.xml
        Matrix44 matrix = new Matrix44();
        matrix.m[0] = 1.0f;
        matrix.m[1] = 0.0f;
        matrix.m[2] = 0.0f;
        matrix.m[3] = 0.0f;
        matrix.m[4] = 0.0f;
        matrix.m[5] = 1.0f;
        matrix.m[6] = 0.0f;
        matrix.m[7] = 0.0f;
        matrix.m[8] = 0.0f;
        matrix.m[9] = 0.0f;
        matrix.m[10] = 1.0f;
        matrix.m[11] = 0.0f;
        matrix.m[12] = x;
        matrix.m[13] = y;
        matrix.m[14] = z;
        matrix.m[15] = 1.0f;
        return mult(matrix);
    }

    public Matrix44 rotate(float a, float x, float y, float z) {
        // Generate a rotation matrix as defined at:
        // http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
        Matrix44 matrix = new Matrix44();
        float c = (float) Math.cos(Math.toRadians(a));
        float s = (float) Math.sin(Math.toRadians(a));
        matrix.m[0] = x * x * (1 - c) + c;
        matrix.m[1] = y * x * (1 - c) + z * s;
        matrix.m[2] = x * z * (1 - c) - y * s;
        matrix.m[3] = 0.0f;
        matrix.m[4] = y * x * (1 - c) - z * s;
        matrix.m[5] = y * y * (1 - c) + c;
        matrix.m[6] = y * z * (1 - c) + x * s;
        matrix.m[7] = 0.0f;
        matrix.m[8] = x * z * (1 - c) + y * s;
        matrix.m[9] = y * z * (1 - c) - x * s;
        matrix.m[10] = z * z * (1 - c) + c;
        matrix.m[11] = 0.0f;
        matrix.m[12] = 0.0f;
        matrix.m[13] = 0.0f;
        matrix.m[14] = 0.0f;
        matrix.m[15] = 1.0f;
        return mult(matrix);
    }

    public float[] raw() {
        return m;
    }

    @Override
    public String toString() {
        return "" + m[0] + "\t" + m[4] + "\t" + m[8] + "\t" + m[12] + "\n"
                  + m[1] + "\t" + m[5] + "\t" + m[9] + "\t" + m[13] + "\n"
                  + m[2] + "\t" + m[6] + "\t" + m[10] + "\t" + m[14] + "\n"
                  + m[3] + "\t" + m[7] + "\t" + m[11] + "\t" + m[15] + "\n";
    }
    */
