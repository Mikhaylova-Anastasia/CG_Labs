#include <vector>
#include <limits>
#include <iostream>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "my_gl.h"
#include "Camera.h"

Model* model = nullptr;
const int width = 800;
const int height = 800;


Vec3f light_dir(1.0f, 2.0f, 1.0f);


Camera camera(
    Vec3f(1.0f, 0.0f, 2.0f), // eye
    Vec3f(0.0f, 0.0f, 0.0f), // center
    Vec3f(0.0f, 1.0f, 0.0f)  // up
);


struct GouraudPhongShader : public IShader {
    mat<2, 3, float> varying_uv;
    Vec3f           varying_intensity;

    Matrix uniform_M;
    Matrix uniform_MIT;
    Matrix uniform_P;
    Vec3f  uniform_light_dir;

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec3f v_obj = model->vert(iface, nthvert);
        Vec3f n_obj = model->normal(iface, nthvert);
        Vec2f uv = model->uv(iface, nthvert);

        varying_uv.set_col(nthvert, uv);


        Vec4f v_cam4 = uniform_M * embed<4>(v_obj, 1.f);
        Vec3f v_cam = proj<3>(v_cam4);


        Vec3f n = proj<3>(uniform_MIT * embed<4>(n_obj, 0.f)).normalize();

        Vec3f l = uniform_light_dir;
        Vec3f v = (v_cam * -1.0f).normalize();

        float diff = std::max(0.0f, n * l);

        Vec3f r = (n * (2.0f * (n * l)) - l).normalize();
        float spec_pow = model->specular(uv);
        float spec = std::pow(std::max(0.0f, r * v), spec_pow);

        float ambient = 0.1f;
        float kd = 0.9f;
        float ks = 0.5f;

        float I = ambient + kd * diff + ks * spec;
        varying_intensity[nthvert] = I;


        Vec4f gl_Vertex = uniform_P * v_cam4;
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec2f uv = varying_uv * bar;
        float I = varying_intensity * bar;

        TGAColor c = model->diffuse(uv);
        for (int i = 0; i < 3; i++) {
            float v = c[i] * I;
            c[i] = (unsigned char)std::min(255.0f, v);
        }
        color = c;
        return false;
    }
};

int main(int argc, char** argv) {
    if (argc > 1) {
        model = new Model(argv[1]);
    }
    else {
        std::cout << "No args provided. Using default model obj/head.obj\n";
        model = new Model("obj/head.obj");
    }

    TGAImage frame(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);


    Matrix ModelView = camera.getModelView();
    Matrix Projection = camera.getProjection();
    Matrix Viewport = camera.getViewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    light_dir.normalize();

    Matrix MIT = ModelView.invert_transpose();


    Vec3f L_cam = proj<3>(ModelView * embed<4>(light_dir, 0.f));
    L_cam.normalize();

    GouraudPhongShader shader;
    shader.uniform_M = ModelView;
    shader.uniform_MIT = MIT;
    shader.uniform_P = Projection;
    shader.uniform_light_dir = L_cam;

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
