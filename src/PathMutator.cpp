#include "path_mutator.h"
#include "GeomUtil.h"
#include <cmath>
#include <algorithm>
#include <array>

PathMutator::PathMutator(const Scene& scene,
    glm::vec3 camera_center,
    glm::vec3 light_pos)
    : m_scene(scene), m_C(camera_center), m_L(light_pos) {
}

bool PathMutator::sample_path(
    Path& path,
    int maxBounces,
    const glm::vec3& initialDir,
    int retriesPerBounce) const
{
    path.vertices.clear();
    path.faces.clear();
    path.bary_points.clear();
    path.light_visible.clear();
    path.bounces = 0;

    auto push_vertex_with_face = [&](const glm::vec3& p,
        const GeomUtil::Triangle& face,
        const glm::vec3& bary)
    {
        path.vertices.push_back(p);
        path.faces.push_back(face);
        path.bary_points.push_back(bary);
    };

    auto push_vertex_no_face = [&](const glm::vec3& p) {
        push_vertex_with_face(p, GeomUtil::Triangle{}, glm::vec3(0.0f));
    };

    push_vertex_no_face(m_C);

    std::random_device rd_seed;
    std::mt19937 rng(rd_seed());
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);

    glm::vec3 ro = m_C;
    glm::vec3 rd = GeomUtil::safe_normalize(initialDir);

    if (glm::dot(rd, rd) == 0.0f) {
        path.vertices.clear();
        path.faces.clear();
        path.bary_points.clear();
        path.light_visible.clear();
        path.bounces = 0;
        return false;
    }

    Scene::Hit hit;
    GeomUtil::Triangle tri;

    if (!m_scene.intersect(ro, rd, hit, tri)) {
        path.vertices.clear();
        path.faces.clear();
        path.bary_points.clear();
        path.light_visible.clear();
        path.bounces = 0;
        return false;
    }

    push_vertex_with_face(hit.p, tri, hit.bary);
    path.bounces = 1;

    ro = hit.p + 1e-4f * hit.n;

    for (int bounce = 1; bounce < maxBounces; ++bounce) {
        bool foundNext = false;

        Scene::Hit nextHit;
        GeomUtil::Triangle nextTri;

        for (int attempt = 0; attempt < retriesPerBounce; ++attempt) {
            glm::vec3 candDir = GeomUtil::sample_hemisphere_uniform(hit.n, rng, u01);
            if (glm::dot(candDir, hit.n) <= 0.0f) continue;

            Scene::Hit tmpHit;
            GeomUtil::Triangle tmpTri;
            if (m_scene.intersect(ro, candDir, tmpHit, tmpTri)) {
                foundNext = true;
                nextHit = tmpHit;
                nextTri = tmpTri;
                break;
            }
        }

        if (!foundNext) {
            break;
        }

        hit = nextHit;
        tri = nextTri;

        push_vertex_with_face(hit.p, tri, hit.bary);
        path.bounces++;

        ro = hit.p + 1e-4f * hit.n;
    }

    push_vertex_no_face(m_L);

    const float visEps = 1e-4f;
    const int nVerts = (int)path.vertices.size();
    const int nInternal = std::max(0, nVerts - 2);

    path.light_visible.resize(nInternal, false);

    for (int i = 1; i <= nVerts - 2; ++i) {
        path.light_visible[i - 1] = m_scene.visible(path.vertices[i], m_L, visEps);
    }

    return true;
}



bool PathMutator::mutate_vertex_meshwalk(Path& path, int index, float radius) const {
    if (radius <= 0.0f) return false;
    if (index < 0 || index >= (int)path.vertices.size()) return false;
    if ((int)path.faces.size() != (int)path.vertices.size()) return false;
    if ((int)path.bary_points.size() != (int)path.vertices.size()) return false;

    const int N = (int)path.vertices.size();
    const int nInternal = std::max(0, N - 2);
    if ((int)path.light_visible.size() != nInternal) return false;

    const auto& tris = m_scene.triangles();
    if (tris.empty()) return false;

    GeomUtil::Triangle cur = path.faces[index];
    glm::vec3 p = path.vertices[index];

    glm::vec3 n = GeomUtil::face_normal_geom(cur);
    if (glm::dot(n, n) <= 0.0f) return false;

    std::random_device rd_seed;
    std::mt19937 rng(rd_seed());
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);

    glm::vec3 U, V;
    GeomUtil::make_orthonormal_basis(n, U, V);

    float rho = std::sqrt(u01(rng)) * radius;
    float phi = 6.283185307179586f * u01(rng);

    glm::vec3 step = (rho * std::cos(phi)) * U + (rho * std::sin(phi)) * V;
    float L = std::sqrt(glm::dot(step, step));
    if (L <= 1e-6f) return true;

    glm::vec3 d = GeomUtil::safe_normalize(step);

    int guard = 0;
    const int GUARD_MAX = 100000;

    while (L > 1e-6f && guard++ < GUARD_MAX) {
        glm::vec3 n0 = GeomUtil::face_normal_geom(cur);
        if (glm::dot(n0, n0) <= 0.0f) return false;

        d = GeomUtil::safe_normalize(GeomUtil::project_to_plane(d, n0));
        if (glm::dot(d, d) <= 0.0f) return false;

        glm::vec3 e1 = cur.v1 - cur.v0;
        float e1len2 = glm::dot(e1, e1);
        if (e1len2 <= 1e-6f * 1e-6f) return false;

        glm::vec3 u = GeomUtil::safe_normalize(e1);
        glm::vec3 v = glm::cross(n0, u);

        auto proj2 = [&](const glm::vec3& x) -> glm::vec2 {
            glm::vec3 r = x - cur.v0;
            return glm::vec2(glm::dot(r, u), glm::dot(r, v));
        };

        glm::vec2 p2 = proj2(p);
        glm::vec2 r2(glm::dot(d, u), glm::dot(d, v));
        if (r2.x * r2.x + r2.y * r2.y <= 1e-6f * 1e-6f) return false;

        float bestS = std::numeric_limits<float>::infinity();
        int bestEdge = -1;

        for (int edgeIdx = 0; edgeIdx < 3; ++edgeIdx) {
            glm::vec3 a3, b3;
            GeomUtil::edge_endpoints(cur, edgeIdx, a3, b3);

            glm::vec2 a2 = proj2(a3);
            glm::vec2 b2 = proj2(b3);
            glm::vec2 e2 = b2 - a2;

            float denom = GeomUtil::cross2(r2, e2);
            if (std::fabs(denom) < 1e-6f) continue;

            glm::vec2 ap = a2 - p2;

            float s = GeomUtil::cross2(ap, e2) / denom;
            float tt = GeomUtil::cross2(ap, r2) / denom;

            if (s > 1e-6f && s <= (L + 1e-6f) && tt >= -1e-6f && tt <= (1.0f + 1e-6f)) {
                if (s < bestS) { bestS = s; bestEdge = edgeIdx; }
            }
        }

        if (bestEdge < 0 || !std::isfinite(bestS)) {
            p = p + L * d;
            L = 0.0f;
            break;
        }

        p = p + bestS * d;
        L -= bestS;

        int nextId = cur.adj[bestEdge];
        if (nextId < 0 || nextId >= (int)tris.size()) return false;

        const GeomUtil::Triangle& nxt = tris[nextId];
        glm::vec3 n1 = GeomUtil::face_normal_geom(nxt);
        if (glm::dot(n1, n1) <= 0.0f) return false;

        glm::vec3 ea, eb;
        GeomUtil::edge_endpoints(cur, bestEdge, ea, eb);
        glm::vec3 axis = GeomUtil::safe_normalize(eb - ea);
        if (glm::dot(axis, axis) <= 0.0f) return false;

        float theta = GeomUtil::signed_dihedral(n0, n1, axis);
        d = GeomUtil::rodrigues(d, axis, theta);

        d = GeomUtil::safe_normalize(GeomUtil::project_to_plane(d, n1));
        if (glm::dot(d, d) <= 0.0f) return false;

        p = p + 1e-6f * d;

        cur = nxt;
    }

    if (guard >= GUARD_MAX) return false;

    {
        const glm::vec3 a = cur.v0;
        const glm::vec3 b = cur.v1;
        const glm::vec3 c = cur.v2;

        const glm::vec3 v0 = b - a;
        const glm::vec3 v1 = c - a;
        const glm::vec3 v2 = p - a;

        const float d00 = glm::dot(v0, v0);
        const float d01 = glm::dot(v0, v1);
        const float d11 = glm::dot(v1, v1);
        const float d20 = glm::dot(v2, v0);
        const float d21 = glm::dot(v2, v1);

        const float denom = d00 * d11 - d01 * d01;
        if (std::fabs(denom) <= 1e-12f) return false;

        const float invDen = 1.0f / denom;
        const float uu = (d11 * d20 - d01 * d21) * invDen;
        const float vv = (d00 * d21 - d01 * d20) * invDen;
        const float ww = 1.0f - uu - vv;

        path.bary_points[index] = glm::vec3(ww, uu, vv);
    }

    path.vertices[index] = p;
    path.faces[index] = cur;

    if (index >= 1 && index <= N - 2) {
        const float visEps = 1e-4f;
        path.light_visible[index - 1] = m_scene.visible(path.vertices[index], m_L, visEps);
    }

    return true;
}

bool PathMutator::mutate_vertex_project(Path& path, int index, float radius) const {
    if (radius <= 0.0f) return false;
    if (index < 0 || index >= (int)path.vertices.size()) return false;
    if ((int)path.faces.size() != (int)path.vertices.size()) return false;
    if ((int)path.bary_points.size() != (int)path.vertices.size()) return false;

    const int N = (int)path.vertices.size();
    const int nInternal = std::max(0, N - 2);
    if ((int)path.light_visible.size() != nInternal) return false;

    const auto& tris = m_scene.triangles();
    if (tris.empty()) return false;

    const GeomUtil::Triangle& cur = path.faces[index];
    glm::vec3 p = path.vertices[index];

    glm::vec3 n = GeomUtil::face_normal_geom(cur);
    if (glm::dot(n, n) <= 0.0f) return false;

    float h = std::max(1e-3f, 0.5f * radius);
    glm::vec3 apex = p + h * n;

    std::random_device rd_seed;
    std::mt19937 rng(rd_seed());
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);

    glm::vec3 U, V;
    GeomUtil::make_orthonormal_basis(n, U, V);

    float rho = std::sqrt(u01(rng)) * radius;
    float phi = 6.283185307179586f * u01(rng);

    glm::vec3 q =
        p
        + (rho * std::cos(phi)) * U
        + (rho * std::sin(phi)) * V;

    glm::vec3 d = GeomUtil::safe_normalize(q - apex);
    if (glm::dot(d, d) <= 0.0f) return false;

    if (glm::dot(d, n) >= -1e-6f) return false;

    Scene::Hit hit;
    GeomUtil::Triangle hitTri;

    const float epsPush = 1e-4f;
    glm::vec3 ro = apex + epsPush * d;

    if (!m_scene.intersect(ro, d, hit, hitTri)) {
        return false;
    }

    glm::vec3 newP = hit.p;

    const float visEps = 1e-4f;

    if (index - 1 >= 0) {
        const glm::vec3& pPrev = path.vertices[index - 1];
        if (!m_scene.visible(pPrev, newP, visEps)) {
            return false;
        }
    }

    if (index + 1 < (int)path.vertices.size()) {
        const glm::vec3& pNext = path.vertices[index + 1];
        if (!m_scene.visible(newP, pNext, visEps)) {
            return false;
        }
    }

    path.vertices[index] = newP;
    path.faces[index] = hitTri;
    path.bary_points[index] = hit.bary;

    if (index >= 1 && index <= N - 2) {
        path.light_visible[index - 1] = m_scene.visible(path.vertices[index], m_L, visEps);
    }

    return true;
}

bool PathMutator::mutate_vertex_retrace(Path& path, int index) const {
    if (index < 0 || index >= (int)path.vertices.size()) return false;
    if ((int)path.faces.size() != (int)path.vertices.size()) return false;
    if ((int)path.bary_points.size() != (int)path.vertices.size()) return false;

    const int N = (int)path.vertices.size();
    const int nInternal = std::max(0, N - 2);
    if ((int)path.light_visible.size() != nInternal) return false;

    if (index == 0) return false;
    if (index == (int)path.vertices.size() - 1) return false;

    const auto& tris = m_scene.triangles();
    if (tris.empty()) return false;

    const GeomUtil::Triangle& curTri = path.faces[index];
    const glm::vec3 bary = path.bary_points[index];

    if (glm::dot(bary, bary) <= 0.0f) return false;

    const float w = bary.x;
    const float u = bary.y;
    const float v = bary.z;

    glm::vec3 p =
        w * curTri.v0 +
        u * curTri.v1 +
        v * curTri.v2;

    glm::vec3 n =
        w * curTri.n0 +
        u * curTri.n1 +
        v * curTri.n2;

    float n2 = glm::dot(n, n);
    if (n2 <= 0.0f) {
        n = glm::cross(curTri.v1 - curTri.v0, curTri.v2 - curTri.v0);
        n2 = glm::dot(n, n);
        if (n2 <= 0.0f) return false;
    }
    n = n / std::sqrt(n2);

    std::random_device rd_seed;
    std::mt19937 rng(rd_seed());
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);

    glm::vec3 d = GeomUtil::sample_hemisphere_uniform(n, rng, u01);
    if (glm::dot(d, n) <= 0.0f) return false;

    const float epsPush = 1e-4f;
    glm::vec3 ro = p + epsPush * n;

    Scene::Hit hit;
    GeomUtil::Triangle hitTri;
    if (!m_scene.intersect(ro, d, hit, hitTri)) return false;

    glm::vec3 newP = hit.p;

    const float visEps = 1e-4f;

    if (index - 1 >= 0) {
        const glm::vec3& pPrev = path.vertices[index - 1];
        if (!m_scene.visible(pPrev, newP, visEps)) return false;
    }

    if (index + 1 < (int)path.vertices.size()) {
        const glm::vec3& pNext = path.vertices[index + 1];
        if (!m_scene.visible(newP, pNext, visEps)) return false;
    }

    path.vertices[index] = newP;
    path.faces[index] = hitTri;
    path.bary_points[index] = hit.bary;

    path.light_visible[index - 1] = m_scene.visible(path.vertices[index], m_L, visEps);

    return true;
}
