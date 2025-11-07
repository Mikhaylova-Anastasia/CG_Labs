#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Model* model = nullptr;
const int width = 800;
const int height = 800;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static inline Vec3f rotateX(const Vec3f& v, float a) {
    float ca = std::cos(a), sa = std::sin(a);
    return Vec3f(v.x, ca * v.y - sa * v.z, sa * v.y + ca * v.z);
}
static inline Vec3f rotateY(const Vec3f& v, float a) {
    float ca = std::cos(a), sa = std::sin(a);
    return Vec3f(ca * v.x + sa * v.z, v.y, -sa * v.x + ca * v.z);
}
static inline Vec3f rotateZ(const Vec3f& v, float a) {
    float ca = std::cos(a), sa = std::sin(a);
    return Vec3f(ca * v.x - sa * v.y, sa * v.x + ca * v.y, v.z);
}


float AX = 0.0f;     
float AY = 0.0f;      
float AZ = 0.0f;      

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x) std::swap(p0, p1);

    for (int x = p0.x; x <= p1.x; x++) {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = int(p0.y * (1.f - t) + p1.y * t);
        if (steep) image.set(y, x, color);
        else       image.set(x, y, color);
    }
}


void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
    if (t0.y == t1.y && t0.y == t2.y) return;
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);
    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int  segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / std::max(1, segment_height);
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int x = A.x; x <= B.x; x++) image.set(x, t0.y + i, color);
    }
}

int main(int argc, char** argv) {
    
    if (argc == 2) model = new Model(argv[1]);
    else           model = new Model("obj/frog.obj");

    TGAImage image(width, height, TGAImage::RGB);

    
    Vec3f vmin(1e9f, 1e9f, 1e9f);
    Vec3f vmax(-1e9f, -1e9f, -1e9f);
    for (int i = 0; i < model->nverts(); ++i) {
        Vec3f v = model->vert(i);
        v = rotateX(v, AX);
        v = rotateY(v, AY);
        v = rotateZ(v, AZ);
        vmin.x = std::min(vmin.x, v.x); vmin.y = std::min(vmin.y, v.y); vmin.z = std::min(vmin.z, v.z);
        vmax.x = std::max(vmax.x, v.x); vmax.y = std::max(vmax.y, v.y); vmax.z = std::max(vmax.z, v.z);
    }

   
    Vec3f c((vmin.x + vmax.x) * 0.5f,
        (vmin.y + vmax.y) * 0.5f,
        (vmin.z + vmax.z) * 0.5f);
    float sx = std::max(1e-6f, vmax.x - vmin.x);
    float sy = std::max(1e-6f, vmax.y - vmin.y);
    float scale = 0.9f * std::min(width / sx, height / sy);   

    auto toScreen = [&](const Vec3f& w) -> Vec2i {
        
        float X = (w.x - c.x) * scale + width * 0.5f;
        float Y = (w.y - c.y) * scale + height * 0.5f;
        return Vec2i(int(X), int(Y));
        };

    
    Vec3f light_dir(0.f, 0.f, -1.f);
    light_dir.normalize();

    
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> f = model->face(i);
        if ((int)f.size() < 3) continue;

        for (int k = 1; k + 1 < (int)f.size(); ++k) {
            int tri[3] = { f[0], f[k], f[k + 1] };

            Vec2i screen[3];
            Vec3f world[3]; 
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(tri[j]);
                v = rotateX(v, AX);
                v = rotateY(v, AY);
                v = rotateZ(v, AZ);
                world[j] = v;
                screen[j] = toScreen(v);  
            }

           
            Vec3f n = (world[2] - world[0]) ^ (world[1] - world[0]);
            if (n.norm() == 0) continue;
            n.normalize();

            float intensity = n * light_dir;       
            if (intensity <= 0.f) continue;        

            unsigned char c8 = (unsigned char)(std::min(1.f, intensity) * 255.f);
            triangle(screen[0], screen[1], screen[2], image, TGAColor(c8, c8, c8, 255));
        }
    }

    image.flip_vertically(); 
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
