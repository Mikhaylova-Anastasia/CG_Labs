#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

Model::Model(const char* filename)
    : verts_(), norms_(), uv_(),
    faces_(), uv_idx_(), norm_idx_(),
    diffusemap_(), normalmap_(), specularmap_() {

    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Cannot open OBJ file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        char trash;

        
        if (!line.compare(0, 2, "v ")) {
            iss >> trash; 
            Vec3f v;
            iss >> v.x >> v.y >> v.z;
            verts_.push_back(v);
        }
        
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash; 
            Vec3f n;
            iss >> n.x >> n.y >> n.z;
            norms_.push_back(n);
        }
       
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash; 
            Vec2f t;
            iss >> t.x >> t.y;
            uv_.push_back(t);
        }
        
        else if (!line.compare(0, 2, "f ")) {
            iss >> trash; 

            std::vector<int> v_idx;
            std::vector<int> t_idx;
            std::vector<int> n_idx;

            std::string token;
            while (iss >> token) {
                int vi = 0, ti = -1, ni = -1;

                
                size_t p1 = token.find('/');
                if (p1 == std::string::npos) {
                    
                    vi = std::stoi(token);
                }
                else {
                    
                    vi = std::stoi(token.substr(0, p1));
                    size_t p2 = token.find('/', p1 + 1);

                    if (p2 == std::string::npos) {
                        
                        std::string tstr = token.substr(p1 + 1);
                        if (!tstr.empty())
                            ti = std::stoi(tstr);
                    }
                    else {
                        
                        if (p2 > p1 + 1) {
                            
                            std::string tstr = token.substr(p1 + 1, p2 - p1 - 1);
                            if (!tstr.empty())
                                ti = std::stoi(tstr);
                        }
                        
                        if (p2 + 1 < token.size()) {
                            std::string nstr = token.substr(p2 + 1);
                            if (!nstr.empty())
                                ni = std::stoi(nstr);
                        }
                    }
                }

                
                vi = vi - 1;
                if (ti > 0) ti = ti - 1;
                if (ni > 0) ni = ni - 1;

                v_idx.push_back(vi);
                t_idx.push_back(ti);
                n_idx.push_back(ni);
            }

            
            int nv = (int)v_idx.size();
            if (nv < 3) continue;

            
            if (uv_.empty()) {
                uv_.push_back(Vec2f(0.f, 0.f)); 
            }

            auto add_triangle = [&](int a, int b, int c) {
                std::vector<int> f(3), fu(3), fn(3);
                f[0] = v_idx[a]; f[1] = v_idx[b]; f[2] = v_idx[c];

                for (int k = 0; k < 3; ++k) {
                    int idxU = t_idx[(k == 0 ? a : (k == 1 ? b : c))];
                    int idxN = n_idx[(k == 0 ? a : (k == 1 ? b : c))];
                    
                    fu[k] = (idxU >= 0 ? idxU : 0);
                    fn[k] = idxN; 
                }

                faces_.push_back(f);
                uv_idx_.push_back(fu);
                norm_idx_.push_back(fn);
                };

            if (nv == 3) {
                add_triangle(0, 1, 2);
            }
            else if (nv == 4) {
                
                add_triangle(0, 1, 2);
                add_triangle(0, 2, 3);
            }
            else {
                
                for (int i = 1; i < nv - 1; ++i) {
                    add_triangle(0, i, i + 1);
                }
            }
        }
    }

    
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);

    std::cerr << "# v " << verts_.size()
        << " f " << faces_.size()
        << " vt " << uv_.size()
        << " vn " << norms_.size() << std::endl;
}

Model::~Model() {}

int Model::nverts() { return (int)verts_.size(); }
int Model::nfaces() { return (int)faces_.size(); }

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert]];
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec2f Model::uv(int iface, int nthvert) {
    int idx = uv_idx_[iface][nthvert];
    if (idx < 0 || idx >= (int)uv_.size()) return Vec2f(0.f, 0.f);
    return uv_[idx];
}

Vec3f Model::normal(int iface, int nthvert) {
    int idx = norm_idx_[iface][nthvert];
    if (idx < 0 || idx >= (int)norms_.size()) return Vec3f(0.f, 0.f, 1.f);
    return norms_[idx];
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        if (img.read_tga_file(texfile.c_str())) {
            img.flip_vertically();
            std::cerr << "texture file " << texfile << " loading ok\n";
        }
        else {
            std::cerr << "texture file " << texfile << " loading failed\n";
        }
    }
}

TGAColor Model::diffuse(Vec2f uvf) {
    if (diffusemap_.get_width() == 0 || diffusemap_.get_height() == 0) {
        
        return TGAColor(255, 255, 255);
    }
    Vec2i uv(
        int(uvf.x * diffusemap_.get_width()),
        int(uvf.y * diffusemap_.get_height()));
    return diffusemap_.get(uv.x, uv.y);
}

Vec3f Model::normal(Vec2f uvf) {
    if (normalmap_.get_width() == 0 || normalmap_.get_height() == 0) {
        return Vec3f(0.f, 0.f, 1.f);
    }
    Vec2i uv(
        int(uvf.x * normalmap_.get_width()),
        int(uvf.y * normalmap_.get_height()));
    TGAColor c = normalmap_.get(uv.x, uv.y);
    Vec3f res;
    for (int i = 0; i < 3; i++)
        res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
    return res;
}

float Model::specular(Vec2f uvf) {
    if (specularmap_.get_width() == 0 || specularmap_.get_height() == 0) {
        return 0.f;
    }
    Vec2i uv(
        int(uvf.x * specularmap_.get_width()),
        int(uvf.y * specularmap_.get_height()));
    return specularmap_.get(uv.x, uv.y)[0] / 1.f;
}
