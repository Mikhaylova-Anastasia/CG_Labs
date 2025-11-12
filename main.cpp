#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "new_raster.h"

const int width = 800;
const int height = 800;
const int depth = 255;

float* zbuffer = nullptr;

class Camera {
public:
    Vec3f eye, center, up;
    float fov, aspect, znear, zfar;
    float rx, ry; // углы вращени€

    Camera()
        : eye(0, 0, 3), center(0, 0, 0), up(0, 1, 0),
        fov(45.f), aspect((float)width / height), znear(0.1f), zfar(50.f),
        rx(0.f), ry(0.f) {
    }

    void look_at(const Vec3f& target) { center = target; }
    void rotate(float dx, float dy) {
        rx = std::max(-1.5f, std::min(1.5f, rx + dx));
        ry += dy;
        float R = (eye - center).norm();
        float x = R * std::sin(ry) * std::cos(rx);
        float z = R * std::cos(ry) * std::cos(rx);
        float y = R * std::sin(rx);
        eye = center + Vec3f(x, y, z);
        Vec3f dir = (center - eye).normalize();
        Vec3f right = (dir ^ Vec3f(0, 1, 0)).normalize();
        up = (right ^ dir).normalize();
    }

    Mat4 view() const { return Mat4::lookat(eye, center, up); }
    Mat4 projection() const { return Mat4::perspective(fov, aspect, znear, zfar); }
};

static Vec3f barycentric(const Vec3f* pts, const Vec2f& p) {
    Vec3f a = pts[1] - pts[0];
    Vec3f b = pts[2] - pts[0];
    Vec3f s = Vec3f(a.x, b.x, pts[0].x - p.u) ^ Vec3f(a.y, b.y, pts[0].y - p.v);
    if (std::fabs(s.z) < 1e-2f) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (s.x + s.y) / s.z, s.x / s.z, s.y / s.z);
}

// ѕерспективно-корректный диффуз + опциональный alpha/white kill
static void triangle_textured_persp(
    const Vec4f(&clip)[3],
    const Vec2f(&uvs)[3],
    TGAImage& image,
    float* zbuffer,
    const Mat4& Viewport,
    Model* model,
    int alpha_cutoff = -1,     // <0 Ч не провер€ть альфу
    bool white_kill = false)   // true Ч отбрасывать почти белые пиксели
{
    Vec3f scr[3];
    float invw[3];
    for (int i = 0; i < 3; i++) {
        invw[i] = 1.f / clip[i].w;
        Vec4f ndc(clip[i].x * invw[i], clip[i].y * invw[i], clip[i].z * invw[i], 1.f);
        Vec4f sv = Viewport * ndc;
        scr[i] = Vec3f(sv.x, sv.y, sv.z);
    }

    int minx = std::max(0, (int)std::floor(std::min({ scr[0].x,scr[1].x,scr[2].x })));
    int maxx = std::min(width - 1, (int)std::ceil(std::max({ scr[0].x,scr[1].x,scr[2].x })));
    int miny = std::max(0, (int)std::floor(std::min({ scr[0].y,scr[1].y,scr[2].y })));
    int maxy = std::min(height - 1, (int)std::ceil(std::max({ scr[0].y,scr[1].y,scr[2].y })));

    Vec3f pts[3] = { scr[0],scr[1],scr[2] };
    Vec2f uv_over_w[3] = {
        Vec2f(uvs[0].u * invw[0], uvs[0].v * invw[0]),
        Vec2f(uvs[1].u * invw[1], uvs[1].v * invw[1]),
        Vec2f(uvs[2].u * invw[2], uvs[2].v * invw[2])
    };

    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            Vec3f bc = barycentric(pts, Vec2f((float)x, (float)y));
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

            float z = 0.f, w = 0.f;
            for (int i = 0; i < 3; i++) {
                z += scr[i].z * bc.raw[i];
                w += invw[i] * bc.raw[i];
            }
            float depth = z / w;
            int idx = x + y * width;
            if (depth >= zbuffer[idx]) continue;
            zbuffer[idx] = depth;

            Vec2f uv_w(
                uv_over_w[0].u * bc.x + uv_over_w[1].u * bc.y + uv_over_w[2].u * bc.z,
                uv_over_w[0].v * bc.x + uv_over_w[1].v * bc.y + uv_over_w[2].v * bc.z
            );
            Vec2f uv(uv_w.u / w, uv_w.v / w);

            TGAColor tex = model->diffuse(uv);

            // --- Alpha test / White kill ---
            if (alpha_cutoff >= 0 && tex.a < alpha_cutoff) continue;
            if (white_kill) {
                int sum = (int)tex.r + (int)tex.g + (int)tex.b;
                if (sum > 3 * 250) continue; // почти белый Ч отбрасываем
            }

            image.set(x, y, tex);
        }
    }
}

static void render_model(
    Model* mdl,
    const Mat4& M, const Mat4& V, const Mat4& P, const Mat4& Viewport,
    TGAImage& img, float* zbuf,
    int alpha_cutoff = -1, bool white_kill = false)
{
    Mat4 MVP = P * V * M;
    for (int i = 0; i < mdl->nfaces(); ++i) {
        auto f = mdl->face(i);
        auto fuv = mdl->uvface(i);

        Vec4f clip[3];
        Vec2f uvs[3];
        for (int j = 0; j < 3; j++) {
            Vec3f v = mdl->vert(f[j]);
            uvs[j] = mdl->uv(fuv[j]);
            clip[j] = MVP * Vec4f(v.x, v.y, v.z, 1.f);
        }
        triangle_textured_persp(clip, uvs, img, zbuf, Viewport, mdl, alpha_cutoff, white_kill);
    }
}

int main(int, char**) {
    // --- загрузка моделей + текстур
    Model head("obj/head.obj");                         head.load_texture("obj/head_diffuse.tga");
    Model eye_in("obj/african_head_eye_inner.obj");    eye_in.load_texture("obj/african_head_eye_inner_diffuse.tga");
    Model eye_out("obj/african_head_eye_outer.obj");    eye_out.load_texture("obj/african_head_eye_outer_diffuse.tga");
    Model floorM("obj/floor.obj");                       floorM.load_texture("obj/floor_diffuse.tga");

    // --- буферы
    zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) zbuffer[i] = std::numeric_limits<float>::infinity();
    TGAImage image(width, height, TGAImage::RGB);

    // --- камера
    Camera cam;
    cam.look_at(Vec3f(0, 0, 0));
    cam.rotate(0.30f, 0.50f);

    // --- матрицы
    Mat4 V = cam.view();
    Mat4 P = cam.projection();
    Mat4 VP = Mat4::viewport(0, 0, (float)width, (float)height, (float)depth);

    // --- модельные матрицы
    Mat4 M_head = Mat4::identity();
    Mat4 M_eye = Mat4::identity();
    Mat4 M_floor = Mat4::translate(0.f, -0.8f, 0.f) * Mat4::scale(1.8f, 1.f, 1.8f);

    // --- рендер: пол и голова Ч обычный; inner Ч обычный; outer Ч с маской
    render_model(&floorM, M_floor, V, P, VP, image, zbuffer);
    render_model(&head, M_head, V, P, VP, image, zbuffer);
    render_model(&eye_in, M_eye, V, P, VP, image, zbuffer);

    // ƒл€ роговицы: если в TGA есть альфа Ч хватит alpha_cutoff=128.
    // ≈сли текстура без альфы и бела€ Ч white_kill=true избавит от белых Ђлинзї.
    render_model(&eye_out, M_eye, V, P, VP, image, zbuffer, /*alpha_cutoff=*/128, /*white_kill=*/true);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zbuffer;
    return 0;
}
