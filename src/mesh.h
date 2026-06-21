#pragma once
#include <vector>
#include <string>
#include "math.h"

struct Face {
    Vec3 pos[3];
    Vec3 nrm[3];
};

struct Mesh {
    std::vector<Face> faces;

    bool loadOBJ(const std::string& path);
    void computeNormals();
};
