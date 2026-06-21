#include <SDL2/SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "linalg.h"
#include "mesh.h"
#include "renderer.h"

static const int W = 800;
static const int H = 600;

// Built-in cube so the program works with no arguments
static Mesh makeCube() {
    Vec3 v[8] = {
        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f},
        { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f},
        { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    };
    int quads[6][4] = {
        {0,1,2,3}, {5,4,7,6}, {4,0,3,7},
        {1,5,6,2}, {3,2,6,7}, {4,5,1,0}
    };
    Mesh m;
    for (auto& q : quads) {
        Vec3 e1 = v[q[1]] - v[q[0]];
        Vec3 e2 = v[q[2]] - v[q[0]];
        Vec3 n  = e1.cross(e2).norm();
        m.faces.push_back({{v[q[0]], v[q[1]], v[q[2]]}, {n, n, n}});
        m.faces.push_back({{v[q[0]], v[q[2]], v[q[3]]}, {n, n, n}});
    }
    return m;
}

// Rescale mesh to fit within a unit sphere centered at origin
static void normalizeMesh(Mesh& mesh) {
    Vec3 mn = { 1e9f,  1e9f,  1e9f};
    Vec3 mx = {-1e9f, -1e9f, -1e9f};

    for (auto& f : mesh.faces)
        for (int i = 0; i < 3; i++) {
            mn.x = std::min(mn.x, f.pos[i].x);
            mn.y = std::min(mn.y, f.pos[i].y);
            mn.z = std::min(mn.z, f.pos[i].z);
            mx.x = std::max(mx.x, f.pos[i].x);
            mx.y = std::max(mx.y, f.pos[i].y);
            mx.z = std::max(mx.z, f.pos[i].z);
        }

    Vec3  center = {(mn.x+mx.x)*0.5f, (mn.y+mx.y)*0.5f, (mn.z+mx.z)*0.5f};
    float extent = std::max({mx.x-mn.x, mx.y-mn.y, mx.z-mn.z});
    float s      = (extent > 0.f) ? (2.f / extent) : 1.f;

    for (auto& f : mesh.faces)
        for (int i = 0; i < 3; i++)
            f.pos[i] = (f.pos[i] - center) * s;
}

int main(int argc, char* argv[]) {
    Mesh mesh;
    if (argc > 1) {
        if (!mesh.loadOBJ(argv[1])) return 1;
        normalizeMesh(mesh);
        // Recompute normals if missing (flat shading fallback)
        bool allZero = true;
        for (auto& f : mesh.faces)
            if (!(f.nrm[0] == Vec3{}) ) { allZero = false; break; }
        if (allZero) mesh.computeNormals();
    } else {
        std::cout << "Usage: rasterizer <model.obj>\n"
                  << "No file given — rendering built-in cube.\n";
        mesh = makeCube();
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window*   win = SDL_CreateWindow("Software Rasterizer",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            W, H, 0);
    SDL_Renderer* sdl = SDL_CreateRenderer(win, -1,
                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture*  tex = SDL_CreateTexture(sdl,
                            SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            W, H);

    if (!win || !sdl || !tex) {
        std::cerr << "SDL setup failed: " << SDL_GetError() << "\n";
        return 1;
    }

    Renderer rast(W, H);

    Mat4 proj = Mat4::perspective(60.f * (3.14159265f / 180.f),
                                  (float)W / H, 0.1f, 100.f);
    Mat4 view = Mat4::lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0});

    Vec3 lightDir = Vec3{0.6f, 1.0f, 0.8f}.norm();
    Vec3 viewDir  = Vec3{0.f, 0.f, 1.f};   // approximate: camera along +z

    float angle = 0.f;
    bool  running = true;
    Uint64 last = SDL_GetPerformanceCounter();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float  dt  = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
        last = now;
        angle += dt * 0.8f;

        Mat4 model = Mat4::rotateY(angle) * Mat4::rotateX(0.3f);
        Mat4 mvp   = proj * view * model;

        rast.clear();

        for (auto& face : mesh.faces) {
            DrawVert dv[3];
            bool skip = false;

            for (int i = 0; i < 3; i++) {
                Vec3& p  = face.pos[i];
                Vec4  clip = mvp * Vec4{p.x, p.y, p.z, 1.f};

                if (clip.w <= 0.f || clip.z < -clip.w || clip.z > clip.w) {
                    skip = true;
                    break;
                }

                float invW = 1.f / clip.w;
                float nx   =  clip.x * invW;
                float ny   =  clip.y * invW;
                float nz   =  clip.z * invW;

                dv[i].screen = {
                    (nx * 0.5f + 0.5f) * W,
                    (1.f - (ny * 0.5f + 0.5f)) * H,
                    nz
                };
                dv[i].invW = invW;

                // Rotate normal by the model matrix (upper-left 3x3 only)
                Vec3& n = face.nrm[i];
                dv[i].normal = Vec3{
                    model.d[0][0]*n.x + model.d[0][1]*n.y + model.d[0][2]*n.z,
                    model.d[1][0]*n.x + model.d[1][1]*n.y + model.d[1][2]*n.z,
                    model.d[2][0]*n.x + model.d[2][1]*n.y + model.d[2][2]*n.z
                }.norm();
            }

            if (!skip)
                rast.drawTriangle(dv, lightDir, viewDir);
        }

        SDL_UpdateTexture(tex, nullptr, rast.pixels.data(), W * sizeof(uint32_t));
        SDL_RenderCopy(sdl, tex, nullptr, nullptr);
        SDL_RenderPresent(sdl);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(sdl);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
