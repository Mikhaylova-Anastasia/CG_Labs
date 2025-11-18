#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "geometry.h"

class Camera {
private:
    Vec3f eye;
    Vec3f center;
    Vec3f up;

public:
    Camera(const Vec3f& eye_ = Vec3f(1.5f, 1.2f, 2.0f),
        const Vec3f& center_ = Vec3f(0.0f, 0.0f, 0.0f),
        const Vec3f& up_ = Vec3f(0.0f, 1.0f, 0.0f));

    
    const Vec3f& getEye() const { return eye; }
    const Vec3f& getCenter() const { return center; }
    const Vec3f& getUp() const { return up; }

    
    void setEye(const Vec3f& e) { eye = e; }
    void setCenter(const Vec3f& c) { center = c; }
    void setUp(const Vec3f& u) { up = u; }

   
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
};

#endif // __CAMERA_H__

