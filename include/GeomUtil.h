#pragma once

#include <cstdint>
#include <random>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

class GeomUtil {
public:
    struct Triangle {
        glm::vec3 v0, v1, v2;
        glm::vec3 n0, n1, n2;

        uint32_t i0 = 0, i1 = 0, i2 = 0;

        int     adj[3] = { -1, -1, -1 };
        uint8_t adjEdge[3] = { 255, 255, 255 };

        glm::vec3 color = glm::vec3(0.7f, 0.7f, 0.7f);
    };

    static glm::vec3 safe_normalize(const glm::vec3& v);

    static void make_orthonormal_basis(
        const glm::vec3& w,
        glm::vec3& u,
        glm::vec3& v
    );

    static bool moller_trumbore(
        const glm::vec3& ro,
        const glm::vec3& rd,
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        float& t,
        float& u,
        float& v,
        float eps = 1e-8f
    );

    static glm::vec3 sample_hemisphere_uniform(
        const glm::vec3& n,
        std::mt19937& rng,
        std::uniform_real_distribution<float>& u01
    );

    static glm::vec3 face_normal_geom(const Triangle& t);
    static glm::vec3 project_to_plane(const glm::vec3& d, const glm::vec3& n);

    static float cross2(const glm::vec2& a, const glm::vec2& b);

    static glm::vec3 rodrigues(
        const glm::vec3& x,
        const glm::vec3& k_unit,
        float theta
    );

    static float signed_dihedral(
        const glm::vec3& n0,
        const glm::vec3& n1,
        const glm::vec3& e_unit
    );

    static void edge_endpoints(
        const Triangle& t,
        int edgeIdx,
        glm::vec3& a,
        glm::vec3& b
    );
};
