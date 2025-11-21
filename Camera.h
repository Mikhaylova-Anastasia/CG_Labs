
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "geometry.h"

class Camera {
public:
    Camera(const Vec3f& eye_, const Vec3f& center_, const Vec3f& up_);

    const Vec3f& getEye() const { return eye; }
    const Vec3f& getCenter() const { return center; }
    const Vec3f& getUp() const { return up; }

    void setEye(const Vec3f& e) { eye = e; }
    void setCenter(const Vec3f& c) { center = c; }
    void setUp(const Vec3f& u) { up = u; }



    Matrix getModelView() const;


    Matrix getProjection() const;


    Matrix getViewport(int x, int y, int w, int h, float depth = 255.f) const;

private:
    Vec3f eye;
    Vec3f center;
    Vec3f up;
};

#endif // __CAMERA_H__
