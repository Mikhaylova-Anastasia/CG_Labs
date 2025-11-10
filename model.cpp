#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(), uvs_(), uvfaces_(), diffusemap_() {
    std::ifstream in(filename);
    if (in.fail()) return;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            iss >> uv.u >> uv.v;
            uvs_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f ")) {
            iss >> trash;
            std::vector<int> f;
            std::vector<int> fuv;
            int vIdx, vtIdx, vnIdx;
            char slash;
            while (iss >> vIdx >> slash >> vtIdx >> slash >> vnIdx) {
                f.push_back(vIdx - 1);
                fuv.push_back(vtIdx - 1);
            }
            faces_.push_back(f);
            uvfaces_.push_back(fuv);
        }
    }
    std::cerr << "# v " << verts_.size()
        << " f " << faces_.size()
        << " vt " << uvs_.size() << std::endl;
}

Model::~Model() {}

int Model::nverts() { return (int)verts_.size(); }
int Model::nfaces() { return (int)faces_.size(); }
std::vector<int> Model::face(int idx) { return faces_[idx]; }
Vec3f Model::vert(int i) { return verts_[i]; }
Vec2f Model::uv(int i) { return uvs_[i]; }
std::vector<int> Model::uvface(int idx) { return uvfaces_[idx]; }

bool Model::load_texture(const char* filename) {
    bool ok = diffusemap_.read_tga_file(filename);
    if (ok) diffusemap_.flip_vertically();
    return ok;
}

TGAColor Model::diffuse(const Vec2f& uvf) {
    int w = diffusemap_.get_width();
    int h = diffusemap_.get_height();
    int x = std::max(0, std::min(w - 1, (int)(uvf.u * w)));
    int y = std::max(0, std::min(h - 1, (int)(uvf.v * h)));
    return diffusemap_.get(x, y);
}
