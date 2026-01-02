#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <limits>
#include "GeomUtil.h"

class Scene {
public:

    bool load(const std::string& filename);
    const std::vector<GeomUtil::Triangle>& triangles() const { return m_triangles; }

    const glm::vec3& bounds_min() const { return m_boundsMin; }
    const glm::vec3& bounds_max() const { return m_boundsMax; }

    struct Hit {
        float t = 0.0f;
        glm::vec3 p{ 0.0f };
        glm::vec3 n{ 0.0f };
        glm::vec3 bary{ 0.0f };
        int triIndex = -1;
    };

    bool visible(const glm::vec3& origin,
        const glm::vec3& target,
        float eps = 1e-4f) const;

    bool visible_along_ray(const glm::vec3& origin,
        const glm::vec3& dir,
        float t_target,
        float eps = 1e-4f) const;

    bool intersect(const glm::vec3& origin,
        const glm::vec3& dir,
        Hit& outHit,
        GeomUtil::Triangle& tri) const;

    void draw_ray(const glm::vec3& origin,
        const glm::vec3& dir,
        float length,
        const glm::vec3& color,
        float radius = 0.0f,
        int sides = 0);

    bool export_obj(const std::string& obj_path) const;

private:
    void reset_bounds() {
        const float inf = std::numeric_limits<float>::infinity();
        m_boundsMin = glm::vec3(inf, inf, inf);
        m_boundsMax = glm::vec3(-inf, -inf, -inf);
    }

    void expand_bounds(const glm::vec3& p) {
        m_boundsMin = glm::min(m_boundsMin, p);
        m_boundsMax = glm::max(m_boundsMax, p);
    }

private:
    std::vector<GeomUtil::Triangle> m_triangles;
    std::vector<GeomUtil::Triangle> m_debugTriangles;
    std::vector<glm::vec3> m_positions;

    glm::vec3 m_boundsMin;
    glm::vec3 m_boundsMax;
};
