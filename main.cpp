#include <vector>
#include <limits>
#include <iostream>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "my_gl.h"

Model* model = nullptr;
const int width = 800;
const int height = 800;


Vec3f light_dir(0, 0, 1);
Vec3f eye(1.5, 1.2, 2.0);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);


struct PhongShader : public IShader {
    mat<2, 3, float> varying_uv;
    mat<3, 3, float> varying_nrm;

    Matrix uniform_M;
    Matrix uniform_MIT;
    Vec3f  uniform_light_dir;

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));

        Vec3f n = model->normal(iface, nthvert);
        varying_nrm.set_col(nthvert,
            proj<3>(uniform_MIT * embed<4>(n, 0.f)));

        Vec3f v = model->vert(iface, nthvert);
        Vec4f gl_Vertex = Projection * uniform_M * embed<4>(v, 1.f);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec2f uv = varying_uv * bar;

        Vec3f n = (varying_nrm * bar).normalize();
        Vec3f l = uniform_light_dir;

        float diff = std::max(0.f, n * l);

        
        Vec3f r = (n * (2.f * (n * l)) - l).normalize();
        float spec_pow = model->specular(uv);
        float spec = std::pow(std::max(r.z, 0.f), spec_pow);

        TGAColor c = model->diffuse(uv);

        float ambient = 0.1f;
        float kd = 0.9f;
        float ks = 0.5f;

        float intensity = ambient + kd * diff + ks * spec;

        for (int i = 0; i < 3; i++) {
            float v = c[i] * intensity;
            c[i] = (unsigned char)std::min(255.f, v);
        }

        color = c;
        return false;
    }
};



int main(int argc, char** argv) {
    if (argc > 1) model = new Model(argv[1]);
    else {
        std::cout << "No args provided. Using default model obj/head.obj\n";
        model = new Model("obj/head.obj");
    }

    TGAImage frame(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    lookat(eye, center, up);
    projection(-1.f / (eye - center).norm());
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    light_dir.normalize();

    PhongShader shader;
    shader.uniform_M = ModelView;
    shader.uniform_MIT = ModelView.invert_transpose();

   
    Vec3f L = proj<3>(ModelView * embed<4>(light_dir, 0.f));
    L.normalize();
    shader.uniform_light_dir = L;
    

    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f clip_verts[3];
        for (int j = 0; j < 3; j++) {
            clip_verts[j] = shader.vertex(i, j);
            clip_verts[j] = Viewport * clip_verts[j];
        }

        triangle(clip_verts, shader, frame, zbuffer);
    }

    frame.flip_vertically();
    frame.write_tga_file("output.tga");

    delete model;
    return 0;
}
