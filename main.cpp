#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = nullptr;
float* zbuffer = nullptr;
Vec3f  light_dir(0.f, 0.f, -1.f);

static Vec3f barycentric(const Vec3f* pts, const Vec2f& p) {
    Vec3f a = pts[1] - pts[0];
    Vec3f b = pts[2] - pts[0];
    Vec3f s = Vec3f(a.x, b.x, pts[0].x - p.u) ^ Vec3f(a.y, b.y, pts[0].y - p.v);
    if (std::fabs(s.z) < 1e-2f) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (s.x + s.y) / s.z, s.x / s.z, s.y / s.z);
}

static void triangle_textured_persp(const Vec4f(&clip)[3],
    const Vec2f(&uvs)[3],
    float intensity,
    TGAImage& image,
    float* zbuffer,
    const Mat4& Viewport,
    Model* model)
{

    if (clip[0].w <= 0 && clip[1].w <= 0 && clip[2].w <= 0) return;

    Vec3f scr[3];
    float invw[3];
    for (int i = 0; i < 3; i++) {
        invw[i] = 1.f / clip[i].w;
        Vec4f ndc(clip[i].x * invw[i], clip[i].y * invw[i], clip[i].z * invw[i], 1.f);
        Vec4f sv = Viewport * ndc;            // z òåïåðü â [0..depth], ãäå ÌÅÍÜØÅ = ÁËÈÆÅ
        scr[i] = Vec3f(sv.x, sv.y, sv.z);
    }

    int minx = std::max(0, (int)std::floor(std::min({ scr[0].x, scr[1].x, scr[2].x })));
    int maxx = std::min(width - 1, (int)std::ceil(std::max({ scr[0].x, scr[1].x, scr[2].x })));
    int miny = std::max(0, (int)std::floor(std::min({ scr[0].y, scr[1].y, scr[2].y })));
    int maxy = std::min(height - 1, (int)std::ceil(std::max({ scr[0].y, scr[1].y, scr[2].y })));

    Vec3f pts[3] = { scr[0], scr[1], scr[2] };
    Vec2f uv_over_w[3];
    for (int i = 0; i < 3; i++) uv_over_w[i] = uvs[i] * invw[i];

    float shade = std::max(0.f, std::min(1.f, intensity));

    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            Vec3f bc = barycentric(pts, Vec2f((float)x, (float)y));
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;


            float z = pts[0].z * bc.x + pts[1].z * bc.y + pts[2].z * bc.z;

            int idx = x + y * width;
            if (z >= zbuffer[idx]) continue;
            zbuffer[idx] = z;


            float invw_p = invw[0] * bc.x + invw[1] * bc.y + invw[2] * bc.z;
            Vec2f uvw = uv_over_w[0] * bc.x + uv_over_w[1] * bc.y + uv_over_w[2] * bc.z;
            Vec2f uv = uvw * (1.f / invw_p);

            TGAColor tex = model->diffuse(uv);
            image.set(x, y, TGAColor((unsigned char)(tex.r * shade),
                (unsigned char)(tex.g * shade),
                (unsigned char)(tex.b * shade), 255));
        }
    }
}

int main(int argc, char** argv) {
    model = (argc == 2) ? new Model(argv[1]) : new Model("obj/head.obj");
    model->load_texture("obj/head_diffuse.tga");
    light_dir.normalize();


    float fov_deg = 60.f;
    float aspect = (float)width / (float)height;
    float znear = 0.2f;
    float zfar = 10.f;

    Vec3f eye(0, 0, 2.0f), center(0, 0, 0), up(0, 1, 0);

    Mat4 ModelView = Mat4::lookat(eye, center, up);
    Mat4 Projection = Mat4::perspective(fov_deg, aspect, znear, zfar);
    Mat4 Viewport = Mat4::viewport(0, 0, (float)width, (float)height, (float)depth);
    Mat4 MVP = Projection * ModelView;


    zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) zbuffer[i] = std::numeric_limits<float>::infinity();

    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> f = model->face(i);
        std::vector<int> fuv = model->uvface(i);

        Vec4f clip[3];
        Vec3f world[3];
        Vec2f uvs[3];

        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(f[j]);
            world[j] = v;
            uvs[j] = model->uv(fuv[j]);
            clip[j] = MVP * Vec4f(v.x, v.y, v.z, 1.f);
        }

        Vec3f n = (world[2] - world[0]) ^ (world[1] - world[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity <= 0.f) continue;

        triangle_textured_persp(clip, uvs, intensity, image, zbuffer, Viewport, model);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
