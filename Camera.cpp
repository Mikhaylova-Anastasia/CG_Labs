
#include "Camera.h"

Camera::Camera(const Vec3f& eye_, const Vec3f& center_, const Vec3f& up_)
    : eye(eye_), center(center_), up(up_) {
}


Matrix Camera::getModelView() const {
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();

    Matrix M = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        M[0][i] = x[i];
        M[1][i] = y[i];
        M[2][i] = z[i];

        M[i][3] = -center[i];
    }
    return M;
}


Matrix Camera::getProjection() const {
    Matrix P = Matrix::identity();
    float c = (eye - center).norm();
    float coeff = -1.f / c;
    P[3][2] = coeff;
    return P;
}


Matrix Camera::getViewport(int x, int y, int w, int h, float depth) const {
    Matrix V = Matrix::identity();
    V[0][3] = x + w / 2.f;
    V[1][3] = y + h / 2.f;
    V[2][3] = depth / 2.f;
    V[0][0] = w / 2.f;
    V[1][1] = h / 2.f;
    V[2][2] = depth / 2.f;
    return V;
}
