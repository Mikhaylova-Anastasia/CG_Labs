
#ifndef __MY_GL_H__
#define __MY_GL_H__

#include "tgaimage.h"
#include "geometry.h"

struct IShader {
    virtual ~IShader() {}

    virtual Vec4f vertex(int iface, int nthvert) = 0;

    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};


void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

#endif // __MY_GL_H__
