#pragma once

#include "scene.h"
#include "path_mutator.h"
#include "GeomUtil.h"

#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <string>

class Renderer {
public:
    struct Camera {
        glm::vec3 center{ 0.0f };
        glm::vec3 forward{ 0.0f, 0.0f, 1.0f };
        float focal_length = 1.0f;
        float pixel_size = 1.0f;

        int width = 640;
        int height = 480;

        glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };
    };

    struct RenderParams {
        int   maxBounces = 8;
        int   retriesPerBounce = 12;
        int   Kmutations = 64;
        float visibilityEps = 1e-4f;
        float mutateRadiusFrac = 0.05;

        glm::vec3 albedo{ 0.7f, 0.7f, 0.7f };

        glm::vec3 lightIntensity{ 20.0f, 20.0f, 20.0f };
    };

public:
    Renderer(const Scene& scene,
        const Camera& cam,
        const glm::vec3& lightPos,
        const RenderParams& params = RenderParams{});

    
    glm::vec3 compute_radiance_for_path(const PathMutator::Path& path) const;

    std::vector<glm::vec3> render_scene(uint32_t seed = 1337u, const int mutator_type=0) const;

    static bool write_ppm(const std::string& path,
        const std::vector<glm::vec3>& img,
        int W, int H,
        float gamma = 2.2f);

private:
    glm::vec3 generate_primary_dir(int px, int py) const;

    glm::vec3 shading_normal_at(const PathMutator::Path& path, int i) const;

    static float clamp01(float x);

private:
    const Scene& m_scene;
    Camera         m_cam;
    glm::vec3      m_lightPos;
    RenderParams   m_params;

    PathMutator    m_mutator;

    float          m_sceneDiag = 1.0f;
};
