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
Vec3f light_dir(0.f, 0.f, -1.f);

static Vec3f barycentric(const Vec3f* pts, const Vec2f& p) {
    Vec3f a = pts[1] - pts[0];
    Vec3f b = pts[2] - pts[0];
    Vec3f s = Vec3f(a.x, b.x, pts[0].x - p.u) ^ Vec3f(a.y, b.y, pts[0].y - p.v);
    if (std::fabs(s.z) < 1e-2f) return Vec3f(-1.f, 1.f, 1.f);
    return Vec3f(1.f - (s.x + s.y) / s.z, s.x / s.z, s.y / s.z);
}

static void triangle_textured(const Vec3i(&screen)[3],
    const Vec2f(&uvs)[3],
    float intensity,
    TGAImage& image,
    float* zbuffer,
    Model* model) {
    Vec3f pts[3] = {
        Vec3f((float)screen[0].x, (float)screen[0].y, (float)screen[0].z),
        Vec3f((float)screen[1].x, (float)screen[1].y, (float)screen[1].z),
        Vec3f((float)screen[2].x, (float)screen[2].y, (float)screen[2].z)
    };

    int minx = std::max(0, (int)std::floor(std::min({ screen[0].x, screen[1].x, screen[2].x })));
    int miny = std::max(0, (int)std::floor(std::min({ screen[0].y, screen[1].y, screen[2].y })));
    int maxx = std::min(width - 1, (int)std::ceil(std::max({ screen[0].x, screen[1].x, screen[2].x })));
    int maxy = std::min(height - 1, (int)std::ceil(std::max({ screen[0].y, screen[1].y, screen[2].y })));

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            Vec3f bc = barycentric(pts, Vec2f((float)x, (float)y));
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;

            float z = pts[0].z * bc.x + pts[1].z * bc.y + pts[2].z * bc.z;
            int idx = x + y * width;
            if (zbuffer[idx] >= z) continue;
            zbuffer[idx] = z;

            Vec2f uv = uvs[0] * bc.x + uvs[1] * bc.y + uvs[2] * bc.z;
            TGAColor tex = model->diffuse(uv);

            float shade = std::max(0.f, std::min(1.f, intensity));
            unsigned char r = (unsigned char)((float)tex.r * shade);
            unsigned char g = (unsigned char)((float)tex.g * shade);
            unsigned char b = (unsigned char)((float)tex.b * shade);
            image.set(x, y, TGAColor(r, g, b, 255));
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 2) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/head.obj");
    }
    model->load_texture("obj/head_diffuse.tga");

    zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> fuv = model->uvface(i);

        Vec3i screen[3];
        Vec3f world[3];
        Vec2f uvs[3];

        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            screen[j] = Vec3i(
                (int)((v.x + 1.f) * width * 0.5f),
                (int)((v.y + 1.f) * height * 0.5f),
                (int)((v.z + 1.f) * depth * 0.5f)
            );
            world[j] = v;
            uvs[j] = model->uv(fuv[j]);
        }

        Vec3f n = (world[2] - world[0]) ^ (world[1] - world[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity <= 0.f) continue;

        triangle_textured(screen, uvs, intensity, image, zbuffer, model);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float z = zbuffer[x + y * width];
            unsigned char v = (z <= -std::numeric_limits<float>::max() / 2.f)
                ? 0
                : (unsigned char)std::max(0.f, std::min(255.f, z));
            zbimage.set(x, y, TGAColor(v, 1));
        }
    }
    zbimage.flip_vertically();
    zbimage.write_tga_file("zbuffer.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
