#include "Camera.h"


Camera::Camera(const Vec3f& eye_, const Vec3f& center_, const Vec3f& up_)
    : eye(eye_), center(center_), up(up_) {
}


void Camera::moveForward(float amount) {
    Vec3f dir = (center - eye).normalize();
    eye = eye + dir * amount;
    center = center + dir * amount;
}

void Camera::moveRight(float amount) {
    Vec3f dir = (center - eye).normalize();
    Vec3f right = cross(dir, up).normalize();
    eye = eye + right * amount;
    center = center + right * amount;
}


void Camera::moveUp(float amount) {
    eye = eye + up * amount;
    center = center + up * amount;
}
