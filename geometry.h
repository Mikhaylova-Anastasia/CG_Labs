#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>

template <class t> struct Vec2 {
    union { struct { t u, v; }; struct { t x, y; }; t raw[2]; };
    Vec2() : u(0), v(0) {}
    Vec2(t _u, t _v) : u(_u), v(_v) {}
    template <class U> Vec2(const Vec2<U>& o) : u((t)o.u), v((t)o.v) {}
    inline Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(u + V.u, v + V.v); }
    inline Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(u - V.u, v - V.v); }
    inline Vec2<t> operator *(float f)          const { return Vec2<t>(u * f, v * f); }
    template <class> friend std::ostream& operator<<(std::ostream& s, const Vec2<t>& v);
};

template <class t> struct Vec3 {
    union { struct { t x, y, z; }; struct { t ivert, iuv, inorm; }; t raw[3]; };
    Vec3() : x(0), y(0), z(0) {}
    Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class U> Vec3(const Vec3<U>& o) : x((t)o.x), y((t)o.y), z((t)o.z) {}
    inline Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    inline Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
    inline Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
    inline Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }
    inline t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }
    template <class> friend std::ostream& operator<<(std::ostream& s, const Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <class t> std::ostream& operator<<(std::ostream& s, const Vec2<t>& v) { s << "(" << v.x << ", " << v.y << ")\n"; return s; }
template <class t> std::ostream& operator<<(std::ostream& s, const Vec3<t>& v) { s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n"; return s; }


struct Vec4f {
    float x, y, z, w;
    Vec4f() :x(0), y(0), z(0), w(1) {}
    Vec4f(float X, float Y, float Z, float W = 1) :x(X), y(Y), z(Z), w(W) {}
};

struct Mat4 {
    float m[4][4];
    Mat4() { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = (i == j ? 1.f : 0.f); }
    static Mat4 identity() { return Mat4(); }

    static Mat4 viewport(float x, float y, float w, float h, float depth) {
        Mat4 V;
        V.m[0][0] = w / 2.f; V.m[0][3] = x + w / 2.f;
        V.m[1][1] = h / 2.f; V.m[1][3] = y + h / 2.f;
        V.m[2][2] = depth / 2.f; V.m[2][3] = depth / 2.f;
        return V;
    }


    static Mat4 perspective(float fov_deg, float aspect, float znear, float zfar) {
        Mat4 P;

        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) P.m[i][j] = 0.f;
        float f = 1.f / std::tan((fov_deg * 3.1415926535f / 180.f) * 0.5f);
        P.m[0][0] = f / aspect;
        P.m[1][1] = f;
        P.m[2][2] = (zfar + znear) / (znear - zfar);
        P.m[2][3] = (2.f * zfar * znear) / (znear - zfar);
        P.m[3][2] = -1.f;

        return P;
    }

    static Mat4 lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
        Vec3f z = (eye - center); z.normalize();
        Vec3f x = (up ^ z); x.normalize();
        Vec3f y = (z ^ x);
        Mat4 M;
        M.m[0][0] = x.x; M.m[0][1] = x.y; M.m[0][2] = x.z; M.m[0][3] = -(x * eye);
        M.m[1][0] = y.x; M.m[1][1] = y.y; M.m[1][2] = y.z; M.m[1][3] = -(y * eye);
        M.m[2][0] = z.x; M.m[2][1] = z.y; M.m[2][2] = z.z; M.m[2][3] = -(z * eye);
        return M;
    }
};

inline Mat4 operator*(const Mat4& A, const Mat4& B) {
    Mat4 R;
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) {
        R.m[i][j] = 0;
        for (int k = 0; k < 4; k++) R.m[i][j] += A.m[i][k] * B.m[k][j];
    }
    return R;
}

inline Vec4f operator*(const Mat4& A, const Vec4f& v) {
    float r[4]; float x[4] = { v.x,v.y,v.z,v.w };
    for (int i = 0; i < 4; i++) { r[i] = 0; for (int k = 0; k < 4; k++) r[i] += A.m[i][k] * x[k]; }
    return Vec4f(r[0], r[1], r[2], r[3]);
}

#endif //__GEOMETRY_H__
