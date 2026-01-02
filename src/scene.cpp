#include "scene.h"
#include "GeomUtil.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <filesystem>

static uint64_t edge_key(uint32_t a, uint32_t b) {
    uint32_t lo = std::min(a, b);
    uint32_t hi = std::max(a, b);
    return (uint64_t(hi) << 32) | uint64_t(lo);
}

struct EdgeRef {
    int tri = -1;
    uint8_t edge = 255;
};

bool Scene::load(const std::string& filename) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << "\n";
        return false;
    }

    m_triangles.clear();
    m_debugTriangles.clear();
    m_positions.clear();
    reset_bounds();

    std::vector<uint32_t> meshBase(scene->mNumMeshes, 0);
    uint32_t totalVerts = 0;

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        meshBase[m] = totalVerts;
        totalVerts += scene->mMeshes[m]->mNumVertices;
    }

    m_positions.resize(totalVerts);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        const uint32_t base = meshBase[m];

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            const aiVector3D& p = mesh->mVertices[i];
            glm::vec3 gp{ p.x, p.y, p.z };
            m_positions[base + i] = gp;
            expand_bounds(gp);
        }
    }

    std::unordered_map<uint64_t, EdgeRef> edgeMap;
    edgeMap.reserve(size_t(totalVerts));

    auto add_edge = [&](int triId, uint8_t edgeIdx, uint32_t a, uint32_t b) {
        uint64_t key = edge_key(a, b);

        auto it = edgeMap.find(key);
        if (it == edgeMap.end()) {
            edgeMap.emplace(key, EdgeRef{ triId, edgeIdx });
            return;
        }

        const EdgeRef other = it->second;

        if (other.tri == triId) return;

        if (m_triangles[other.tri].adj[other.edge] != -1) return;

        m_triangles[other.tri].adj[other.edge] = triId;
        m_triangles[other.tri].adjEdge[other.edge] = edgeIdx;

        m_triangles[triId].adj[edgeIdx] = other.tri;
        m_triangles[triId].adjEdge[edgeIdx] = other.edge;
    };

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        const bool hasNormals = mesh->HasNormals();
        const uint32_t base = meshBase[m];

        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;

            GeomUtil::Triangle tri;

            tri.i0 = base + uint32_t(face.mIndices[0]);
            tri.i1 = base + uint32_t(face.mIndices[1]);
            tri.i2 = base + uint32_t(face.mIndices[2]);

            tri.v0 = m_positions[tri.i0];
            tri.v1 = m_positions[tri.i1];
            tri.v2 = m_positions[tri.i2];

            tri.n0 = tri.n1 = tri.n2 = glm::vec3(0.0f);
            if (hasNormals) {
                const aiVector3D& n0 = mesh->mNormals[face.mIndices[0]];
                const aiVector3D& n1 = mesh->mNormals[face.mIndices[1]];
                const aiVector3D& n2 = mesh->mNormals[face.mIndices[2]];
                tri.n0 = { n0.x, n0.y, n0.z };
                tri.n1 = { n1.x, n1.y, n1.z };
                tri.n2 = { n2.x, n2.y, n2.z };
            }

            tri.adj[0] = tri.adj[1] = tri.adj[2] = -1;
            tri.adjEdge[0] = tri.adjEdge[1] = tri.adjEdge[2] = 255;

            const int triId = (int)m_triangles.size();
            m_triangles.push_back(tri);

            add_edge(triId, 0, tri.i0, tri.i1);
            add_edge(triId, 1, tri.i1, tri.i2);
            add_edge(triId, 2, tri.i2, tri.i0);
        }
    }

    return true;
}

static void append_cylinder(std::vector<GeomUtil::Triangle>& out,
    const glm::vec3& p0,
    const glm::vec3& p1,
    float r,
    int sides) {
    glm::vec3 axis = p1 - p0;
    glm::vec3 w = GeomUtil::safe_normalize(axis);
    if (glm::dot(w, w) == 0.0f) return;

    glm::vec3 u, v;
    GeomUtil::make_orthonormal_basis(w, u, v);

    const float twoPi = 6.283185307179586f;
    std::vector<glm::vec3> ring0(sides), ring1(sides);

    for (int i = 0; i < sides; ++i) {
        float t = (float)i / (float)sides;
        float ang = twoPi * t;
        glm::vec3 offset = r * (std::cos(ang) * u + std::sin(ang) * v);
        ring0[i] = p0 + offset;
        ring1[i] = p1 + offset;
    }

    for (int i = 0; i < sides; ++i) {
        int j = (i + 1) % sides;

        GeomUtil::Triangle t0;
        t0.v0 = ring0[i];
        t0.v1 = ring1[i];
        t0.v2 = ring1[j];

        GeomUtil::Triangle t1;
        t1.v0 = ring0[i];
        t1.v1 = ring1[j];
        t1.v2 = ring0[j];

        t0.n0 = t0.n1 = t0.n2 = glm::vec3(0.0f);
        t1.n0 = t1.n1 = t1.n2 = glm::vec3(0.0f);

        out.push_back(t0);
        out.push_back(t1);
    }
}

void Scene::draw_ray(const glm::vec3& origin,
    const glm::vec3& dir,
    float length,
    const glm::vec3& color,
    float radius,
    int sides
    )
{
    glm::vec3 d = GeomUtil::safe_normalize(dir);
    if (glm::dot(d, d) == 0.0f) return;

    if (radius <= 0.0f) {
        glm::vec3 diag = m_boundsMax - m_boundsMin;
        float D = std::sqrt(glm::dot(diag, diag));
        radius = 0.001f * D;
        if (radius <= 0.0f) radius = 1e-3f;
    }

    if (sides < 3) sides = 3;

    glm::vec3 p0 = origin;
    glm::vec3 p1 = origin + length * d;

    const size_t start = m_debugTriangles.size();
    append_cylinder(m_debugTriangles, p0, p1, radius, sides);

    for (size_t i = start; i < m_debugTriangles.size(); ++i) {
        m_debugTriangles[i].color = color;
    }
}

bool Scene::export_obj(const std::string& obj_path) const {
    namespace fs = std::filesystem;

    fs::path objP(obj_path);
    fs::path dir = objP.parent_path();
    if (!dir.empty() && !fs::exists(dir)) {
        std::cerr << "export_obj: output directory does not exist: " << dir.string() << "\n";
        return false;
    }

    fs::path mtlP = objP;
    mtlP.replace_extension(".mtl");

    auto clamp01 = [](float x) { return std::max(0.0f, std::min(1.0f, x)); };

    struct ColorKey {
        uint8_t r, g, b;
        bool operator==(const ColorKey& o) const { return r == o.r && g == o.g && b == o.b; }
    };

    struct ColorKeyHash {
        size_t operator()(const ColorKey& k) const noexcept {
            return (size_t(k.r) << 16) ^ (size_t(k.g) << 8) ^ size_t(k.b);
        }
    };

    auto to_key = [&](const glm::vec3& c) -> ColorKey {
        glm::vec3 cc{ clamp01(c.x), clamp01(c.y), clamp01(c.z) };
        ColorKey k;
        k.r = (uint8_t)std::lround(cc.x * 255.0f);
        k.g = (uint8_t)std::lround(cc.y * 255.0f);
        k.b = (uint8_t)std::lround(cc.z * 255.0f);
        return k;
    };

    auto key_to_rgb = [&](const ColorKey& k) -> glm::vec3 {
        return glm::vec3(k.r / 255.0f, k.g / 255.0f, k.b / 255.0f);
    };

    auto mat_name = [&](const ColorKey& k) -> std::string {
        return "Mat_" + std::to_string((int)k.r) + "_" + std::to_string((int)k.g) + "_" + std::to_string((int)k.b);
    };

    using TriList = std::vector<const GeomUtil::Triangle*>;
    std::unordered_map<ColorKey, TriList, ColorKeyHash> groups;
    groups.reserve(m_triangles.size() / 2 + m_debugTriangles.size() + 8);

    auto add_grouped = [&](const std::vector<GeomUtil::Triangle>& tris) {
        for (const auto& t : tris) {
            groups[to_key(t.color)].push_back(&t);
        }
    };

    add_grouped(m_triangles);
    add_grouped(m_debugTriangles);

    std::ofstream mtl(mtlP, std::ios::out);
    if (!mtl) {
        std::cerr << "export_obj: failed to open MTL for writing: " << mtlP.string() << "\n";
        return false;
    }

    for (const auto& kv : groups) {
        const ColorKey k = kv.first;
        const glm::vec3 rgb = key_to_rgb(k);
        const std::string name = mat_name(k);

        mtl << "newmtl " << name << "\n";
        mtl << "Ka 0.0 0.0 0.0\n";
        mtl << "Kd " << rgb.x << " " << rgb.y << " " << rgb.z << "\n";
        mtl << "Ks 0.0 0.0 0.0\n";
        mtl << "d 1.0\n";
        mtl << "illum 1\n\n";
    }
    mtl.close();

    std::ofstream obj(objP, std::ios::out);
    if (!obj) {
        std::cerr << "export_obj: failed to open OBJ for writing: " << objP.string() << "\n";
        return false;
    }

    obj << "# Exported by Mesh::export_obj\n";
    obj << "mtllib " << mtlP.filename().string() << "\n\n";

    int vIndexBase = 0;

    for (const auto& kv : groups) {
        const ColorKey k = kv.first;
        const auto& list = kv.second;

        if (list.empty()) continue;

        const std::string materialName = mat_name(k);

        obj << "o " << materialName << "\n";
        obj << "usemtl " << materialName << "\n";

        for (const GeomUtil::Triangle* tp : list) {
            const GeomUtil::Triangle& t = *tp;
            obj << "v " << t.v0.x << " " << t.v0.y << " " << t.v0.z << "\n";
            obj << "v " << t.v1.x << " " << t.v1.y << " " << t.v1.z << "\n";
            obj << "v " << t.v2.x << " " << t.v2.y << " " << t.v2.z << "\n";
        }

        const int triCount = (int)list.size();
        for (int i = 0; i < triCount; ++i) {
            int a = vIndexBase + 3 * i + 1;
            int b = vIndexBase + 3 * i + 2;
            int c = vIndexBase + 3 * i + 3;
            obj << "f " << a << " " << b << " " << c << "\n";
        }

        vIndexBase += 3 * triCount;
        obj << "\n";
    }

    obj.close();
    return true;
}

bool Scene::intersect(const glm::vec3& origin,
    const glm::vec3& dir,
    Hit& outHit,
    GeomUtil::Triangle& tri) const
{
    const float tMin = 1e-4f;
    float bestT = std::numeric_limits<float>::infinity();
    int bestIdx = -1;
    float bestU = 0.0f, bestV = 0.0f;

    for (int i = 0; i < (int)m_triangles.size(); ++i) {
        const GeomUtil::Triangle& tri = m_triangles[i];

        float t, u, v;
        if (!GeomUtil::moller_trumbore(origin, dir, tri.v0, tri.v1, tri.v2, t, u, v)) {
            continue;
        }
        if (t < tMin) continue;

        if (t < bestT) {
            bestT = t;
            bestIdx = i;
            bestU = u;
            bestV = v;
        }
    }

    if (bestIdx < 0) return false;

    tri = m_triangles[bestIdx];

    const float u = bestU;
    const float v = bestV;
    const float w = 1.0f - u - v;

    outHit.t = bestT;
    outHit.triIndex = bestIdx;
    outHit.bary = glm::vec3(w, u, v);
    outHit.p = origin + bestT * dir;

    glm::vec3 n = w * tri.n0 + u * tri.n1 + v * tri.n2;
    float n2 = glm::dot(n, n);
    if (n2 <= 0.0f) {
        n = glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
    }
    outHit.n = n / std::sqrt(glm::dot(n, n));

    return true;
}

bool Scene::visible(const glm::vec3& origin,
    const glm::vec3& target,
    float eps) const
{
    glm::vec3 d = target - origin;
    float dist2 = glm::dot(d, d);
    if (dist2 <= 0.0f) {
        return true;
    }

    float dist = std::sqrt(dist2);
    glm::vec3 dir = d / dist;

    return visible_along_ray(origin, dir, dist, eps);
}

bool Scene::visible_along_ray(const glm::vec3& origin,
    const glm::vec3& dir,
    float t_target,
    float eps) const
{
    if (t_target <= 0.0f) return true;

    Hit h;
    GeomUtil::Triangle tri;
    if (!intersect(origin, dir, h, tri)) {
        return true;
    }

    return !(h.t < (t_target - eps));
}
