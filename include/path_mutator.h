#pragma once

#include "scene.h"
#include <glm/glm.hpp>
#include <vector>
#include <random>

class PathMutator {
public:
    struct Path {
        std::vector<glm::vec3> vertices;
        std::vector<GeomUtil::Triangle> faces;
        std::vector<glm::vec3> bary_points;
        std::vector<bool> light_visible;
        int bounces = 0;
    };

    PathMutator(const Scene& scene,
        glm::vec3 camera_center,
        glm::vec3 light_pos);

    bool sample_path(Path& path,
        int maxBounces,
        const glm::vec3& initialDir,
        int retriesPerBounce = 8) const;

    bool mutate_vertex_meshwalk(
        Path& path,
        int index,
        float radius) const;

    bool mutate_vertex_project(
        Path& path, 
        int index, 
        float radius) const;

    bool mutate_vertex_retrace(Path& path,
        int index) const;

private:
    const Scene& m_scene;
    glm::vec3    m_C;
    glm::vec3    m_L;
};
