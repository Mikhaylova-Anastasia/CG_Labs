#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>

template <class t> struct Vec2 {
    union {
        struct { t u, v; };
        struct { t x, y; };
        t raw[2];
    };

    Vec2() : u(0), v(0) {}
    Vec2(t _u, t _v) : u(_u), v(_v) {}
    template <class U> Vec2(const Vec2<U>& o) : u((t)o.u), v((t)o.v) {}

    inline Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(u + V.u, v + V.v); }
    inline Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(u - V.u, v - V.v); }
    inline Vec2<t> operator *(float f) const { return Vec2<t>(u * f, v * f); }
    inline Vec2<t> operator /(float f) const { return Vec2<t>(u / f, v / f); }
    inline Vec2<t>& operator +=(const Vec2<t>& V) { u += V.u; v += V.v; return *this; }
    inline Vec2<t>& operator -=(const Vec2<t>& V) { u -= V.u; v -= V.v; return *this; }
    inline Vec2<t>& operator *=(float f) { u *= f; v *= f; return *this; }
    inline Vec2<t>& operator /=(float f) { u /= f; v /= f; return *this; }

    template <class> friend std::ostream& operator<<(std::ostream& s, const Vec2<t>& v);
};

template <class t>
std::ostream& operator<<(std::ostream& s, const Vec2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")";
    return s;
}

template <class t> struct Vec3 {
    union {
        struct { t x, y, z; };
        struct { t ivert, iuv, inorm; };
        t raw[3];
    };

    Vec3() : x(0), y(0), z(0) {}
    Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    template <class U> Vec3(const Vec3<U>& o) : x((t)o.x), y((t)o.y), z((t)o.z) {}

    inline Vec3<t> operator ^(const Vec3<t>& v) const {
        return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    inline Vec3<t> operator +(const Vec3<t>& v) const {
        return Vec3<t>(x + v.x, y + v.y, z + v.z);
    }
    inline Vec3<t> operator -(const Vec3<t>& v) const {
        return Vec3<t>(x - v.x, y - v.y, z - v.z);
    }
    inline Vec3<t> operator *(float f) const {
        return Vec3<t>(x * f, y * f, z * f);
    }
    inline Vec3<t> operator /(float f) const {
        return Vec3<t>(x / f, y / f, z / f);
    }
    inline t operator *(const Vec3<t>& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    inline Vec3<t>& operator +=(const Vec3<t>& v) {
        x += v.x; y += v.y; z += v.z; return *this;
    }
    inline Vec3<t>& operator -=(const Vec3<t>& v) {
        x -= v.x; y -= v.y; z -= v.z; return *this;
    }
    inline Vec3<t>& operator *=(float f) {
        x *= f; y *= f; z *= f; return *this;
    }
    inline Vec3<t>& operator /=(float f) {
        x /= f; y /= f; z /= f; return *this;
    }

    float norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    Vec3<t>& normalize(t l = 1) {
        *this = (*this) * (l / norm());
        return *this;
    }

    template <class> friend std::ostream& operator<<(std::ostream& s, const Vec3<t>& v);
};

template <class t>
std::ostream& operator<<(std::ostream& s, const Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return s;
}

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

class Vec4f {
public:
    float x, y, z, w;

    Vec4f() : x(0), y(0), z(0), w(1) {}
    Vec4f(float X, float Y, float Z, float W = 1) : x(X), y(Y), z(Z), w(W) {}
    Vec4f(const Vec3f& v, float W = 1) : x(v.x), y(v.y), z(v.z), w(W) {}

    inline Vec4f operator +(const Vec4f& v) const {
        return Vec4f(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    inline Vec4f operator -(const Vec4f& v) const {
        return Vec4f(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    inline Vec4f operator *(float f) const {
        return Vec4f(x * f, y * f, z * f, w * f);
    }
    inline Vec4f operator /(float f) const {
        return Vec4f(x / f, y / f, z / f, w / f);
    }

    Vec3f to_vec3() const {
        return Vec3f(x, y, z);
    }
};

class Mat4 {
public:
    float m[4][4];

    Mat4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = (i == j ? 1.f : 0.f);
    }

    static Mat4 identity() {
        return Mat4();
    }

    static Mat4 viewport(float x, float y, float w, float h, float depth) {
        Mat4 V;
        V.m[0][0] = w / 2.f;
        V.m[0][3] = x + w / 2.f;
        V.m[1][1] = h / 2.f;
        V.m[1][3] = y + h / 2.f;
        V.m[2][2] = depth / 2.f;
        V.m[2][3] = depth / 2.f;
        V.m[3][3] = 1.f;
        return V;
    }

    static Mat4 perspective(float fov_deg, float aspect, float znear, float zfar) {
        Mat4 P;
        float f = 1.f / std::tan((fov_deg * 3.1415926535f / 180.f) * 0.5f);
        P.m[0][0] = f / aspect;
        P.m[1][1] = f;
        P.m[2][2] = (zfar + znear) / (znear - zfar);
        P.m[2][3] = (2.f * zfar * znear) / (znear - zfar);
        P.m[3][2] = -1.f;
        P.m[3][3] = 0.f;
        return P;
    }

    static Mat4 lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
        Vec3f z = (eye - center);
        z.normalize();
        Vec3f x = (up ^ z);
        x.normalize();
        Vec3f y = (z ^ x);
        y.normalize();

        Mat4 M;
        M.m[0][0] = x.x; M.m[0][1] = x.y; M.m[0][2] = x.z; M.m[0][3] = -(x * eye);
        M.m[1][0] = y.x; M.m[1][1] = y.y; M.m[1][2] = y.z; M.m[1][3] = -(y * eye);
        M.m[2][0] = z.x; M.m[2][1] = z.y; M.m[2][2] = z.z; M.m[2][3] = -(z * eye);
        M.m[3][3] = 1.f;
        return M;
    }

    static Mat4 rotate_x(float angle_rad) {
        Mat4 R = Mat4::identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        R.m[1][1] = c;  R.m[1][2] = -s;
        R.m[2][1] = s;  R.m[2][2] = c;
        return R;
    }

    static Mat4 rotate_y(float angle_rad) {
        Mat4 R = Mat4::identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        R.m[0][0] = c;  R.m[0][2] = s;
        R.m[2][0] = -s; R.m[2][2] = c;
        return R;
    }

    static Mat4 rotate_z(float angle_rad) {
        Mat4 R = Mat4::identity();
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        R.m[0][0] = c;  R.m[0][1] = -s;
        R.m[1][0] = s;  R.m[1][1] = c;
        return R;
    }

    static Mat4 translate(float tx, float ty, float tz) {
        Mat4 T = Mat4::identity();
        T.m[0][3] = tx;
        T.m[1][3] = ty;
        T.m[2][3] = tz;
        return T;
    }

    static Mat4 scale(float sx, float sy, float sz) {
        Mat4 S = Mat4::identity();
        S.m[0][0] = sx;
        S.m[1][1] = sy;
        S.m[2][2] = sz;
        return S;
    }
};

inline Mat4 operator*(const Mat4& A, const Mat4& B) {
    Mat4 R;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            R.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                R.m[i][j] += A.m[i][k] * B.m[k][j];
            }
        }
    }
    return R;
}

inline Vec4f operator*(const Mat4& A, const Vec4f& v) {
    return Vec4f(
        A.m[0][0] * v.x + A.m[0][1] * v.y + A.m[0][2] * v.z + A.m[0][3] * v.w,
        A.m[1][0] * v.x + A.m[1][1] * v.y + A.m[1][2] * v.z + A.m[1][3] * v.w,
        A.m[2][0] * v.x + A.m[2][1] * v.y + A.m[2][2] * v.z + A.m[2][3] * v.w,
        A.m[3][0] * v.x + A.m[3][1] * v.y + A.m[3][2] * v.z + A.m[3][3] * v.w
    );
}

inline Vec3f operator*(const Mat4& A, const Vec3f& v) {
    Vec4f result = A * Vec4f(v);
    return result.to_vec3();
}

#endif