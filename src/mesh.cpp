#include "mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>

struct FaceIdx { int v = -1, t = -1, n = -1; };

static FaceIdx parseVertex(const std::string& tok) {
    FaceIdx idx;
    std::istringstream ss(tok);
    std::string part;

    std::getline(ss, part, '/');
    if (!part.empty()) idx.v = std::stoi(part) - 1;

    if (std::getline(ss, part, '/') && !part.empty())
        idx.t = std::stoi(part) - 1;

    if (std::getline(ss, part, '/') && !part.empty())
        idx.n = std::stoi(part) - 1;

    return idx;
}

bool Mesh::loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Cannot open: " << path << "\n";
        return false;
    }

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tok;
        ss >> tok;

        if (tok == "v") {
            Vec3 p; ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (tok == "vn") {
            Vec3 n; ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (tok == "f") {
            std::vector<FaceIdx> fv;
            std::string vert;
            while (ss >> vert)
                fv.push_back(parseVertex(vert));

            for (int i = 1; i + 1 < (int)fv.size(); i++) {
                Face tri;
                int idx[3] = {0, i, i + 1};
                for (int j = 0; j < 3; j++) {
                    int pi = fv[idx[j]].v;
                    int ni = fv[idx[j]].n;
                    tri.pos[j] = (pi >= 0 && pi < (int)positions.size()) ? positions[pi] : Vec3{};
                    tri.nrm[j] = (ni >= 0 && ni < (int)normals.size())   ? normals[ni]  : Vec3{};
                }
                faces.push_back(tri);
            }
        }
    }

    if (normals.empty())
        computeNormals();

    std::cout << "Loaded " << faces.size() << " triangles from " << path << "\n";
    return true;
}

void Mesh::computeNormals() {
    for (auto& f : faces) {
        Vec3 e1 = f.pos[1] - f.pos[0];
        Vec3 e2 = f.pos[2] - f.pos[0];
        Vec3 n  = e1.cross(e2).norm();
        f.nrm[0] = f.nrm[1] = f.nrm[2] = n;
    }
}
