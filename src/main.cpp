#include "scene.h"
#include "renderer.h"

#include <iostream>
#include <string>
#include <cmath>
#include <vector>

static std::string mutator_name(int t) {
    switch (t) {
    case 0: return "retrace";
    case 1: return "meshwalk";
    case 2: return "project";
    case 3: return "resample";
    default: return "unknown";
    }
}

int main() {
    Scene scene;
    const std::string scenePath = "C:/Users/neels/source/repos/PathMutation/Scenes/cornell-box.obj";

    if (!scene.load(scenePath)) {
        std::cout << "Failed to load scene: " << scenePath << "\n";
        return 1;
    }

    const glm::vec3 bmin = scene.bounds_min();
    const glm::vec3 bmax = scene.bounds_max();
    const glm::vec3 diag = bmax - bmin;

    const float sceneDiag = std::sqrt(glm::dot(diag, diag));

    const float cx = 0.5f * (bmin.x + bmax.x);
    const float cy = 0.5f * (bmin.y + bmax.y);

    const float cz = bmax.z + 0.5f * sceneDiag;

    Renderer::Camera cam;
    cam.center = glm::vec3(cx, cy, cz);
    cam.forward = glm::vec3(0.0f, 0.0f, -1.0f);
    cam.world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    cam.width = 640;
    cam.height = 480;

    cam.pixel_size = sceneDiag / float(cam.width);
    cam.focal_length = 1.0f * sceneDiag; // tweak (0.5..2.0)*sceneDiag for FOV

    const glm::vec3 light_pos =
        bmin + glm::vec3(0.5f) * (bmax - bmin) + glm::vec3(0.0f, 0.35f * diag.y, 0.0f);

    Renderer::RenderParams params;
    params.maxBounces = 8;
    params.retriesPerBounce = 12;
    params.Kmutations = 64;
    params.visibilityEps = 1e-4f;
    params.albedo = glm::vec3(0.7f, 0.7f, 0.7f);
    params.lightIntensity = glm::vec3(20.0f, 20.0f, 20.0f);

    Renderer renderer(scene, cam, light_pos, params);

    std::cout << "Rendering " << cam.width << "x" << cam.height
        << " | maxBounces=" << params.maxBounces
        << " | Kmutations=" << params.Kmutations << "\n";

    const uint32_t seedBase = 1337u;

    for (int mutType = 0; mutType <= 3; ++mutType) {
        const uint32_t seed = seedBase + 100u * (uint32_t)mutType;

        std::cout << "  -> mutator_type=" << mutType
            << " (" << mutator_name(mutType) << ")"
            << " seed=" << seed << "\n";

        std::vector<glm::vec3> img = renderer.render_scene(seed, mutType);

        const std::string outPath =
            "C:/Users/neels/source/repos/PathMutation/Scenes/out_" + mutator_name(mutType) + ".ppm";

        if (!Renderer::write_ppm(outPath, img, cam.width, cam.height, 2.2f)) {
            std::cout << "Failed to write image: " << outPath << "\n";
            return 1;
        }

        std::cout << "Wrote: " << outPath << "\n";
    }

    return 0;
}
