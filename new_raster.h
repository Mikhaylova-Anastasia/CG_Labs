#pragma once
#include "geometry.h"

// √лобальные матрицы в стиле TinyRenderer (на будущее)
extern Mat4 g_Model;
extern Mat4 g_View;
extern Mat4 g_Projection;
extern Mat4 g_Viewport;

// ќбЄртки под твои Mat4::* (возвращают матрицу и обновл€ют глобальные)
Mat4 lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up);
Mat4 perspective(float fov_deg, float aspect, float znear, float zfar);
Mat4 viewport(float x, float y, float w, float h, float depth);

