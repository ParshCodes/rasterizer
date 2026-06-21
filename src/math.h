#pragma once
#include <cmath>

struct Vec2 {
    float x = 0, y = 0;
};

struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3 operator+(Vec3 o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(Vec3 o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    Vec3 operator-()        const { return {-x, -y, -z}; }
    bool operator==(Vec3 o) const { return x==o.x && y==o.y && z==o.z; }

    float dot(Vec3 o)   const { return x*o.x + y*o.y + z*o.z; }
    Vec3  cross(Vec3 o) const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
    float len()  const { return sqrtf(x*x + y*y + z*z); }
    Vec3  norm() const { float l = len(); return l > 0.f ? Vec3{x/l, y/l, z/l} : Vec3{}; }
};

inline Vec3 operator*(float s, Vec3 v) { return v * s; }

struct Vec4 {
    float x = 0, y = 0, z = 0, w = 1;
    Vec3 xyz() const { return {x, y, z}; }
};

struct Mat4 {
    float d[4][4] = {};

    static Mat4 identity() {
        Mat4 m;
        m.d[0][0] = m.d[1][1] = m.d[2][2] = m.d[3][3] = 1.f;
        return m;
    }

    static Mat4 perspective(float fovY, float aspect, float near, float far) {
        float f = 1.f / tanf(fovY * 0.5f);
        Mat4 m;
        m.d[0][0] = f / aspect;
        m.d[1][1] = f;
        m.d[2][2] = (far + near) / (near - far);
        m.d[2][3] = 2.f * far * near / (near - far);
        m.d[3][2] = -1.f;
        return m;
    }

    static Mat4 lookAt(Vec3 eye, Vec3 at, Vec3 up) {
        Vec3 f = (at - eye).norm();
        Vec3 r = f.cross(up).norm();
        Vec3 u = r.cross(f);
        Mat4 m = identity();
        m.d[0][0]=r.x;  m.d[0][1]=r.y;  m.d[0][2]=r.z;  m.d[0][3]=-r.dot(eye);
        m.d[1][0]=u.x;  m.d[1][1]=u.y;  m.d[1][2]=u.z;  m.d[1][3]=-u.dot(eye);
        m.d[2][0]=-f.x; m.d[2][1]=-f.y; m.d[2][2]=-f.z; m.d[2][3]= f.dot(eye);
        return m;
    }

    static Mat4 rotateY(float a) {
        Mat4 m = identity();
        m.d[0][0]= cosf(a); m.d[0][2]=sinf(a);
        m.d[2][0]=-sinf(a); m.d[2][2]=cosf(a);
        return m;
    }

    static Mat4 rotateX(float a) {
        Mat4 m = identity();
        m.d[1][1]= cosf(a); m.d[1][2]=-sinf(a);
        m.d[2][1]= sinf(a); m.d[2][2]= cosf(a);
        return m;
    }

    static Mat4 translate(Vec3 t) {
        Mat4 m = identity();
        m.d[0][3]=t.x; m.d[1][3]=t.y; m.d[2][3]=t.z;
        return m;
    }

    static Mat4 scale(float s) {
        Mat4 m = identity();
        m.d[0][0]=s; m.d[1][1]=s; m.d[2][2]=s;
        return m;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    r.d[i][j] += d[i][k] * b.d[k][j];
        return r;
    }

    Vec4 operator*(Vec4 v) const {
        return {
            d[0][0]*v.x + d[0][1]*v.y + d[0][2]*v.z + d[0][3]*v.w,
            d[1][0]*v.x + d[1][1]*v.y + d[1][2]*v.z + d[1][3]*v.w,
            d[2][0]*v.x + d[2][1]*v.y + d[2][2]*v.z + d[2][3]*v.w,
            d[3][0]*v.x + d[3][1]*v.y + d[3][2]*v.z + d[3][3]*v.w,
        };
    }
};
