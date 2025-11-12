#include "new_raster.h"

Mat4 g_Model = Mat4::identity();
Mat4 g_View = Mat4::identity();
Mat4 g_Projection = Mat4::identity();
Mat4 g_Viewport = Mat4::identity();

Mat4 lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    g_View = Mat4::lookat(eye, center, up);
    return g_View;
}

Mat4 perspective(float fov_deg, float aspect, float znear, float zfar) {
    g_Projection = Mat4::perspective(fov_deg, aspect, znear, zfar);
    return g_Projection;
}

Mat4 viewport(float x, float y, float w, float h, float depth) {
    g_Viewport = Mat4::viewport(x, y, w, h, depth);
    return g_Viewport;
}
