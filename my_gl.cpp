#include <cmath>
#include <limits>
#include "my_gl.h"


static Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u.z) > 1e-2f)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); 
}

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }

    Vec2i P;
    TGAColor color;

    for (P.x = (int)bboxmin.x; P.x <= (int)bboxmax.x; P.x++) {
        for (P.y = (int)bboxmin.y; P.y <= (int)bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(
                proj<2>(pts[0] / pts[0][3]),
                proj<2>(pts[1] / pts[1][3]),
                proj<2>(pts[2] / pts[2][3]),
                proj<2>(P)
            );
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            float z = pts[0][2] * bc_screen.x +
                pts[1][2] * bc_screen.y +
                pts[2][2] * bc_screen.z;
            float w = pts[0][3] * bc_screen.x +
                pts[1][3] * bc_screen.y +
                pts[2][3] * bc_screen.z;

            int frag_depth = int(z / w + 0.5f);
            frag_depth = std::max(0, std::min(255, frag_depth));

            
            if (zbuffer.get(P.x, P.y)[0] > frag_depth)
                continue;

            bool discard = shader.fragment(bc_screen, color);
            if (discard) continue;

            if (shader.is_transparent) {
                
                float a = std::max(0.f, std::min(1.f, shader.alpha));

                TGAColor under = image.get(P.x, P.y);
                TGAColor out;
                for (int k = 0; k < 3; k++) {
                    float v = under[k] * (1.f - a) + color[k] * a;
                    out[k] = (unsigned char)std::max(0.f, std::min(255.f, v));
                }
                image.set(P.x, P.y, out);
            }
            else {
                
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}
