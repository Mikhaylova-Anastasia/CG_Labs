#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <array>
#include <memory>
#include <functional>
#include <random>

using namespace std;

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() {}
    Vec3(float X, float Y, float Z) :x(X), y(Y), z(Z) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
};

struct Color {
    float r = 0, g = 0, b = 0;
    Color() {}
    Color(float R, float G, float B) :r(R), g(G), b(B) {}

    Color operator+(const Color& other) const { return Color(r + other.r, g + other.g, b + other.b); }
    Color operator*(float scalar) const { return Color(r * scalar, g * scalar, b * scalar); }
};

struct Vertex {
    Vec3 pos;
    Color col;

    Vertex() {}
    Vertex(const Vec3& p, const Color& c) : pos(p), col(c) {}
};

struct Triangle {
    Vertex v0, v1, v2;
    Triangle() {}
    Triangle(const Vertex& V0, const Vertex& V1, const Vertex& V2) : v0(V0), v1(V1), v2(V2) {}
};

Vec3 crossp(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

float dotp(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 normalize(const Vec3& v) {
    float L = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (L == 0) return v;
    return Vec3(v.x / L, v.y / L, v.z / L);
}

vector<Color> brightColors = {
    Color(1.0f, 0.0f, 0.0f),   // Красный
    Color(0.0f, 1.0f, 0.0f),   // Зеленый
    Color(0.0f, 0.0f, 1.0f),   // Синий
    Color(1.0f, 1.0f, 0.0f),   // Желтый
    Color(1.0f, 0.0f, 1.0f),   // Пурпурный
    Color(0.0f, 1.0f, 1.0f),   // Голубой
    Color(1.0f, 0.5f, 0.0f),   // Оранжевый
    Color(0.5f, 0.0f, 1.0f),   // Фиолетовый
    Color(1.0f, 0.0f, 0.5f),   // Розовый
    Color(0.0f, 1.0f, 0.5f)    // Бирюзовый
};

Color getRandomBrightColor() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, brightColors.size() - 1);
    return brightColors[dist(gen)];
}

class Mesh {
private:
    vector<Vertex> vertices;
    vector<array<int, 3>> faces;
    vector<Triangle> triangles;
    bool isTransformed = false;

public:
    Mesh() {}

    bool LoadFromOBJ(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) return false;

        vertices.clear();
        faces.clear();
        triangles.clear();
        isTransformed = false;

        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;

            if (line[0] == 'v' && (line.size() == 1 || isspace((unsigned char)line[1]))) {
                istringstream iss(line.substr(1));
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(Vec3(x, y, z), Color(1.0f, 1.0f, 1.0f));
            }
            else if (line[0] == 'f' && (line.size() == 1 || isspace((unsigned char)line[1]))) {
                istringstream iss(line.substr(1));
                vector<string> tokens;
                string tok;
                while (iss >> tok) tokens.push_back(tok);
                if (tokens.size() < 3) continue;

                auto parseIndex = [&](const string& s) {
                    size_t p = s.find('/');
                    string idx = (p == string::npos) ? s : s.substr(0, p);
                    return stoi(idx) - 1;
                    };

                int a = parseIndex(tokens[0]), b = parseIndex(tokens[1]), c = parseIndex(tokens[2]);
                if (a >= 0 && b >= 0 && c >= 0) {
                    faces.push_back(array<int, 3>{a, b, c});
                }
            }
        }

        BuildTriangles();
        return true;
    }

    void CreateFromData(const vector<Vertex>& verts, const vector<array<int, 3>>& indices) {
        vertices = verts;
        faces = indices;
        triangles.clear();
        isTransformed = false;
        BuildTriangles();
    }

    void Transform(const function<Vec3(const Vec3&)>& transformFunc) {
        for (auto& vertex : vertices) {
            vertex.pos = transformFunc(vertex.pos);
        }
        isTransformed = true;
        BuildTriangles();
    }

    void SetRandomColors() {
        for (auto& face : faces) {
            Color randomColor = getRandomBrightColor();
            vertices[face[0]].col = randomColor;
            vertices[face[1]].col = randomColor;
            vertices[face[2]].col = randomColor;
        }
        BuildTriangles();
    }

    void SetColorFromNormals(const Vec3& lightDir) {
        for (auto& face : faces) {
            Vec3 p0 = vertices[face[0]].pos;
            Vec3 p1 = vertices[face[1]].pos;
            Vec3 p2 = vertices[face[2]].pos;

            Vec3 e1 = p1 - p0;
            Vec3 e2 = p2 - p0;
            Vec3 normal = normalize(crossp(e1, e2));

            float intensity = max(0.0f, dotp(normal, lightDir));
            Color faceColor(intensity, intensity, intensity);

            vertices[face[0]].col = faceColor;
            vertices[face[1]].col = faceColor;
            vertices[face[2]].col = faceColor;
        }
        BuildTriangles();
    }

    const vector<Triangle>& GetTriangles() const {
        return triangles;
    }

    void GetBoundingBox(Vec3& minV, Vec3& maxV) const {
        if (vertices.empty()) return;

        minV = vertices[0].pos;
        maxV = vertices[0].pos;

        for (const auto& vertex : vertices) {
            const Vec3& v = vertex.pos;
            minV.x = min(minV.x, v.x);
            minV.y = min(minV.y, v.y);
            minV.z = min(minV.z, v.z);
            maxV.x = max(maxV.x, v.x);
            maxV.y = max(maxV.y, v.y);
            maxV.z = max(maxV.z, v.z);
        }
    }

private:
    void BuildTriangles() {
        triangles.clear();
        for (const auto& face : faces) {
            if (face[0] < vertices.size() && face[1] < vertices.size() && face[2] < vertices.size()) {
                triangles.emplace_back(
                    vertices[face[0]],
                    vertices[face[1]],
                    vertices[face[2]]
                );
            }
        }
    }
};

struct Image {
    int w = 0, h = 0;
    vector<unsigned char> data;

    Image(int W, int H) :w(W), h(H), data(W* H * 3, 0) {}

    void setPixel(int x, int y, const Color& c) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        int idx = (y * w + x) * 3;
        auto clamp = [](float v)->unsigned char {
            if (v < 0) v = 0;
            if (v > 1) v = 1;
            return (unsigned char)(v * 255.0f + 0.5f);
            };
        data[idx + 0] = clamp(c.r);
        data[idx + 1] = clamp(c.g);
        data[idx + 2] = clamp(c.b);
    }

    void writePPM(const string& filename) const {
        FILE* f = nullptr;
        fopen_s(&f, filename.c_str(), "wb");
        if (!f) { perror("fopen_s"); return; }
        fprintf(f, "P6 %d %d 255\n", w, h);
        fwrite(data.data(), 1, data.size(), f);
        fclose(f);
    }

    void Clear(const Color& clearColor = Color(0, 0, 0)) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                setPixel(x, y, clearColor);
            }
        }
    }
};

inline float edgeFunc(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

void rasterizeTriangle(Image& img, vector<float>& depth, const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    int W = img.w, H = img.h;
    float minX = floor(min({ v0.pos.x, v1.pos.x, v2.pos.x }));
    float minY = floor(min({ v0.pos.y, v1.pos.y, v2.pos.y }));
    float maxX = ceil(max({ v0.pos.x, v1.pos.x, v2.pos.x }));
    float maxY = ceil(max({ v0.pos.y, v1.pos.y, v2.pos.y }));

    int x0 = max(0, (int)minX), y0 = max(0, (int)minY);
    int x1 = min(W - 1, (int)maxX), y1 = min(H - 1, (int)maxY);

    float area = edgeFunc(v0.pos, v1.pos, v2.pos);
    if (fabs(area) < 1e-9f) return;

    float invArea = 1.0f / area;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            Vec3 p((float)x + 0.5f, (float)y + 0.5f, 0);
            float w0 = edgeFunc(v1.pos, v2.pos, p) * invArea;
            float w1 = edgeFunc(v2.pos, v0.pos, p) * invArea;
            float w2 = edgeFunc(v0.pos, v1.pos, p) * invArea;

            if (w0 >= -1e-6f && w1 >= -1e-6f && w2 >= -1e-6f) {
                float z = w0 * v0.pos.z + w1 * v1.pos.z + w2 * v2.pos.z;
                int idx = y * W + x;

                if (z < depth[idx]) {
                    depth[idx] = z;
                    Color col = v0.col * w0 + v1.col * w1 + v2.col * w2;
                    img.setPixel(x, y, col);
                }
            }
        }
    }
}

void DrawMesh(Image& img, vector<float>& depth, const Mesh& mesh) {
    const vector<Triangle>& triangles = mesh.GetTriangles();
    for (const auto& triangle : triangles) {
        rasterizeTriangle(img, depth, triangle.v0, triangle.v1, triangle.v2);
    }
}

int main(int argc, char** argv) {
    string modelFile = "model2.obj";
    if (argc >= 2) modelFile = argv[1];

    const int W = 800, H = 600;
    Image img(W, H);
    vector<float> depth(W * H, 1e9f);
    Vec3 lightDir = normalize(Vec3(0.3f, 1.0f, 0.6f));

    Mesh mesh;
    if (!mesh.LoadFromOBJ(modelFile)) {
        cerr << "Failed to open OBJ: " << modelFile << endl;
        return 1;
    }

    Vec3 minV, maxV;
    mesh.GetBoundingBox(minV, maxV);
    Vec3 center((minV.x + maxV.x) / 2, (minV.y + maxV.y) / 2, (minV.z + maxV.z) / 2);

    float scaleX = maxV.x - minV.x;
    float scaleY = maxV.y - minV.y;
    float scaleZ = maxV.z - minV.z;
    float scale = max(scaleX, max(scaleY, scaleZ)) * 0.5f;

    auto projectTop = [&](const Vec3& v)->Vec3 {
        Vec3 p;
        p.x = ((v.x - center.x) / scale * 0.5f + 0.5f) * (W - 1);
        p.y = ((v.z - center.z) / scale * 0.5f + 0.5f) * (H - 1);
        p.z = (v.y - center.y) / scale;
        return p;
        };

    mesh.Transform(projectTop);
    mesh.SetRandomColors();

    img.Clear(Color(0.0f, 0.0f, 0.0f));
    fill(depth.begin(), depth.end(), 1e9f);

    DrawMesh(img, depth, mesh);

    img.writePPM("output.ppm");
    printf("Rendered mesh with %zu triangles to output.ppm (%dx%d)\n",
        mesh.GetTriangles().size(), W, H);

    return 0;
}