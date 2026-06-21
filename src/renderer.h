#pragma once
#include <vector>
#include <cstdint>
#include "math.h"

struct DrawVert {
    Vec3  screen;   // x, y in pixels; z is NDC depth in [-1, 1]
    Vec3  normal;   // world-space normal for shading
    float invW;     // 1/clip.w for perspective-correct interpolation
};

class Renderer {
public:
    int width, height;
    std::vector<uint32_t> pixels;
    std::vector<float>    zbuf;

    Renderer(int w, int h);

    void clear(uint32_t bg = 0xFF111111);
    void drawTriangle(const DrawVert v[3], Vec3 lightDir, Vec3 viewDir);

private:
    void writePixel(int x, int y, uint32_t color, float depth);
};
