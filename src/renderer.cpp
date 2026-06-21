#include "renderer.h"
#include <algorithm>
#include <cmath>

static uint32_t toARGB(Vec3 c) {
    int r = (int)(std::clamp(c.x, 0.f, 1.f) * 255.f);
    int g = (int)(std::clamp(c.y, 0.f, 1.f) * 255.f);
    int b = (int)(std::clamp(c.z, 0.f, 1.f) * 255.f);
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

Renderer::Renderer(int w, int h) : width(w), height(h) {
    pixels.resize(w * h);
    zbuf.resize(w * h);
}

void Renderer::clear(uint32_t bg) {
    std::fill(pixels.begin(), pixels.end(), bg);
    std::fill(zbuf.begin(), zbuf.end(), 1.f);
}

void Renderer::writePixel(int x, int y, uint32_t color, float depth) {
    int i = y * width + x;
    if (depth < zbuf[i]) {
        zbuf[i]   = depth;
        pixels[i] = color;
    }
}

void Renderer::drawTriangle(const DrawVert v[3], Vec3 lightDir, Vec3 viewDir) {
    float minX = std::min({v[0].screen.x, v[1].screen.x, v[2].screen.x});
    float maxX = std::max({v[0].screen.x, v[1].screen.x, v[2].screen.x});
    float minY = std::min({v[0].screen.y, v[1].screen.y, v[2].screen.y});
    float maxY = std::max({v[0].screen.y, v[1].screen.y, v[2].screen.y});

    int x0 = std::max(0,          (int)minX);
    int x1 = std::min(width  - 1, (int)maxX);
    int y0 = std::max(0,          (int)minY);
    int y1 = std::min(height - 1, (int)maxY);

    if (x0 > x1 || y0 > y1) return;

    float ax = v[1].screen.x - v[0].screen.x;
    float ay = v[1].screen.y - v[0].screen.y;
    float bx = v[2].screen.x - v[0].screen.x;
    float by = v[2].screen.y - v[0].screen.y;
    float area = ax * by - ay * bx;
    if (fabsf(area) < 1e-6f) return;
    float invArea = 1.f / area;

    // Base surface color
    Vec3 albedo = {0.85f, 0.72f, 0.60f};

    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float px = x + 0.5f, py = y + 0.5f;
            float cx = px - v[0].screen.x;
            float cy = py - v[0].screen.y;

            float w1 = (cx * by - cy * bx) * invArea;
            float w2 = (ax * cy - ay * cx) * invArea;
            float w0 = 1.f - w1 - w2;

            if (w0 < 0.f || w1 < 0.f || w2 < 0.f) continue;

            float depth = w0 * v[0].screen.z + w1 * v[1].screen.z + w2 * v[2].screen.z;

            // Perspective-correct normal interpolation
            float iw = w0 * v[0].invW + w1 * v[1].invW + w2 * v[2].invW;
            Vec3 N = (v[0].normal * (w0 * v[0].invW) +
                      v[1].normal * (w1 * v[1].invW) +
                      v[2].normal * (w2 * v[2].invW));
            N = (N * (1.f / iw)).norm();

            // Phong shading
            float ambient  = 0.12f;
            float diffuse  = std::max(0.f, N.dot(lightDir)) * 0.80f;

            Vec3  R   = (N * (2.f * N.dot(lightDir)) - lightDir).norm();
            float spec = powf(std::max(0.f, R.dot(viewDir)), 32) * 0.45f;

            Vec3 color = albedo * (ambient + diffuse) + Vec3{1,1,1} * spec;
            writePixel(x, y, toARGB(color), depth);
        }
    }
}
