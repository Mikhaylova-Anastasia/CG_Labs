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

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage& image, TGAColor color, float* zbuffer) {
    if (t0.y == t1.y && t0.y == t2.y) return;

    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);

    int total_height = t2.y - t0.y;
    if (total_height == 0) return;

    Vec3f t0f(t0), t1f(t1), t2f(t2);

    for (int i = 0; i < total_height; i++) {
        bool second_half = i > (t1.y - t0.y) || t1.y == t0.y;
        int segment_height = second_half ? (t2.y - t1.y) : (t1.y - t0.y);
        if (segment_height == 0) continue;

        float alpha = (float)i / (float)total_height;
        float beta = (float)(i - (second_half ? (t1.y - t0.y) : 0)) / (float)segment_height;

        Vec3f A = t0f + (t2f - t0f) * alpha;
        Vec3f B = second_half ? t1f + (t2f - t1f) * beta : t0f + (t1f - t0f) * beta;

        if (A.x > B.x) std::swap(A, B);

        int y = (int)A.y;
        if (y < 0 || y >= height) continue;

        int x0 = std::max(0, (int)std::ceil(A.x));
        int x1 = std::min(width - 1, (int)std::floor(B.x));

        for (int x = x0; x <= x1; x++) {
            float phi = (B.x == A.x) ? 1.f : (float)(x - A.x) / (float)(B.x - A.x);
            Vec3f P = A + (B - A) * phi;

            int idx = x + y * width;
            if (idx < 0 || idx >= width * height) continue;

            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                image.set(x, y, color);
            }
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

    zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    {
        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3i screen[3];
            Vec3f world[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(face[j]);
                screen[j] = Vec3i((v.x + 1.f) * width / 2.f,
                    (v.y + 1.f) * height / 2.f,
                    (v.z + 1.f) * depth / 2.f);
                world[j] = v;
            }
            Vec3f n = (world[2] - world[0]) ^ (world[1] - world[0]);
            n.normalize();
            float intensity = n * light_dir;
            if (intensity > 0.f) {
                TGAColor col((unsigned char)(intensity * 255),
                    (unsigned char)(intensity * 255),
                    (unsigned char)(intensity * 255), 255);
                triangle(screen[0], screen[1], screen[2], image, col, zbuffer);
            }
        }

        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    {
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float z = zbuffer[x + y * width];
                unsigned char v = (z <= -std::numeric_limits<float>::max() / 2)
                    ? 0
                    : (unsigned char)std::max(0.f, std::min(255.f, z));
                zbimage.set(x, y, TGAColor(v, 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }

    delete model;
    delete[] zbuffer;
    return 0;
}
