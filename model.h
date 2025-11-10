#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<int>> faces_;
    std::vector<Vec2f> uvs_;
    std::vector<std::vector<int>> uvfaces_;
    TGAImage diffusemap_;

public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f vert(int i);
    std::vector<int> face(int idx);

    Vec2f uv(int i);
    std::vector<int> uvface(int idx);
    bool load_texture(const char* filename);
    TGAColor diffuse(const Vec2f& uvf);
};

#endif
