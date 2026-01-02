//// main.cpp
//#include "scene.h"
//#include "path_mutator.h"
//
//#include <iostream>
//#include <random>
//#include <cmath>
//#include <string>
//
//static glm::vec3 random_in_box(std::mt19937& rng, const glm::vec3& a, const glm::vec3& b) {
//    std::uniform_real_distribution<float> ux(a.x, b.x);
//    std::uniform_real_distribution<float> uy(a.y, b.y);
//    std::uniform_real_distribution<float> uz(a.z, b.z);
//    return glm::vec3(ux(rng), uy(rng), uz(rng));
//}
//
//static glm::vec3 safe_normalize(const glm::vec3& v) {
//    float len2 = glm::dot(v, v);
//    if (len2 <= 0.0f) return glm::vec3(1, 0, 0);
//    return v / std::sqrt(len2);
//}
//
//int main() {
//    Scene scene;
//    if (!scene.load("C:/Users/neels/source/repos/PathMutation/Scenes/cornell-box.obj")) {
//        std::cout << "Failed to load scene\n";
//        return 1;
//    }
//
//    std::cout << "Loaded triangles: " << scene.triangles().size() << "\n";
//
//    glm::vec3 bmin = scene.bounds_min();
//    glm::vec3 bmax = scene.bounds_max();
//    std::cout << "Min bounds: (" << bmin.x << ", " << bmin.y << ", " << bmin.z << ")\n";
//    std::cout << "Max bounds: (" << bmax.x << ", " << bmax.y << ", " << bmax.z << ")\n";
//
//    glm::vec3 diag = bmax - bmin;
//    float sceneDiag = std::sqrt(glm::dot(diag, diag));
//
//    float cx = 0.5f * (bmin.x + bmax.x);
//    float cy = 0.5f * (bmin.y + bmax.y);
//    float cz = 10.0f;
//
//    glm::vec3 camera_center(cx, cy, cz);
//
//    //glm::vec3 camera_center = bmin + glm::vec3(0.5f) * (bmax - bmin) + glm::vec3(0.0f, 0.0f, -0.5f * diag.z);
//    glm::vec3 light_pos = bmin + glm::vec3(0.5f) * (bmax - bmin) + glm::vec3(0.0f, 0.35f * diag.y, 0.0f);
//
//    std::cout << "Camera: (" << camera_center.x << ", " << camera_center.y << ", " << camera_center.z << ")\n";
//    std::cout << "Light:  (" << light_pos.x << ", " << light_pos.y << ", " << light_pos.z << ")\n";
//
//    PathMutator pm(scene, camera_center, light_pos);
//
//    std::random_device rd_seed;
//    std::mt19937 rng(rd_seed());
//
//    const int numPaths = 10;
//    const int maxBounces = 8;
//    const int retriesPerBounce = 12;
//    const glm::vec3 green(0.0f, 1.0f, 0.0f);
//
//    for (int k = 0; k < numPaths; ++k) {
//        glm::vec3 target = random_in_box(rng, bmin, bmax);
//        glm::vec3 initDir = safe_normalize(target - camera_center);
//
//        PathMutator::Path path;
//        bool ok = pm.sample_path(path, maxBounces, initDir, retriesPerBounce);
//
//        std::cout << "Path " << k << ": vertices=" << path.vertices.size()
//            << " bounces=" << path.bounces
//            << " valid=" << (ok ? "true" : "false") << "\n";
//
//        for (size_t i = 0; i + 1 < path.vertices.size(); ++i) {
//            glm::vec3 p0 = path.vertices[i];
//            glm::vec3 p1 = path.vertices[i + 1];
//            glm::vec3 seg = p1 - p0;
//            float len = std::sqrt(glm::dot(seg, seg));
//            if (len <= 1e-6f) continue;
//
//            glm::vec3 d = seg / len;
//
//            scene.draw_ray(p0, d, len, green);
//        }
//    }
//
//    const std::string outObj = "C:/Users/neels/source/repos/PathMutation/Scenes/cornell_with_paths.obj";
//    if (!scene.export_obj(outObj)) {
//        std::cout << "Failed to export obj\n";
//        return 1;
//    }
//
//    std::cout << "Exported: " << outObj << "\n";
//    return 0;
//}
//#include "scene.h"
//#include "path_mutator.h"
//#include "GeomUtil.h"
//
//#include <iostream>
//#include <random>
//#include <cmath>
//
//static glm::vec3 random_in_box(std::mt19937& rng, const glm::vec3& a, const glm::vec3& b) {
//    std::uniform_real_distribution<float> ux(a.x, b.x);
//    std::uniform_real_distribution<float> uy(a.y, b.y);
//    std::uniform_real_distribution<float> uz(a.z, b.z);
//    return glm::vec3(ux(rng), uy(rng), uz(rng));
//}
//
//static void draw_path_segments(Scene& scene,
//    const std::vector<glm::vec3>& verts,
//    const glm::vec3& color) {
//    for (size_t i = 0; i + 1 < verts.size(); ++i) {
//        glm::vec3 p0 = verts[i];
//        glm::vec3 p1 = verts[i + 1];
//        glm::vec3 seg = p1 - p0;
//        float len = std::sqrt(glm::dot(seg, seg));
//        if (len <= 1e-6f) continue;
//        glm::vec3 d = seg / len;
//        scene.draw_ray(p0, d, len, color);
//    }
//}
//
//int main() {
//    Scene scene;
//    if (!scene.load("C:/Users/neels/source/repos/PathMutation/Scenes/cornell-box.obj")) {
//        std::cout << "Failed to load scene\n";
//        return 1;
//    }
//
//    glm::vec3 bmin = scene.bounds_min();
//    glm::vec3 bmax = scene.bounds_max();
//    glm::vec3 diag = bmax - bmin;
//
//    float cx = 0.5f * (bmin.x + bmax.x);
//    float cy = 0.5f * (bmin.y + bmax.y);
//    float cz = 5.0f;
//    glm::vec3 camera_center(cx, cy, cz);
//
//    glm::vec3 light_pos = bmin + glm::vec3(0.5f) * (bmax - bmin) + glm::vec3(0.0f, 0.35f * diag.y, 0.0f);
//
//    PathMutator pm(scene, camera_center, light_pos);
//
//    std::random_device rd_seed;
//    std::mt19937 rng(rd_seed());
//
//    glm::vec3 target = random_in_box(rng, bmin, bmax);
//    glm::vec3 initDir = GeomUtil::safe_normalize(target - camera_center);
//
//    PathMutator::Path path;
//    bool ok = pm.sample_path(path, 8, initDir, 12);
//
//    if (path.vertices.size() < 3) {
//        std::cout << "Path too short to mutate\n";
//        return 1;
//    }
//
//    PathMutator::Path mutated = path;
//
//    int idx = 1;
//    if ((int)mutated.vertices.size() > 3) idx = (int)mutated.vertices.size() / 2;
//    float radius = 0.25f * std::sqrt(glm::dot(diag, diag));
//
//    bool mok = pm.mutate_vertex_retrace(mutated, idx);
//
//    std::cout << "original valid=" << (ok ? "true" : "false") << " vertices=" << path.vertices.size() << "\n";
//    std::cout << "mutated  ok=" << (mok ? "true" : "false") << " mutate_index=" << idx << "\n";
//
//    const glm::vec3 green(0.0f, 1.0f, 0.0f);
//    const glm::vec3 red(1.0f, 0.0f, 0.0f);
//
//    draw_path_segments(scene, path.vertices, green);
//    draw_path_segments(scene, mutated.vertices, red);
//
//    const std::string outObj = "C:/Users/neels/source/repos/PathMutation/Scenes/cornell_original_and_mutated.obj";
//    if (!scene.export_obj(outObj)) {
//        std::cout << "Failed to export obj\n";
//        return 1;
//    }
//
//    std::cout << "Exported: " << outObj << "\n";
//    return 0;
//}
//#include "scene.h"
//#include "path_mutator.h"
//#include "GeomUtil.h"
//#include "Renderer.h"
//
//#include <iostream>
//#include <random>
//#include <cmath>

//static glm::vec3 random_in_box(std::mt19937& rng, const glm::vec3& a, const glm::vec3& b) {
//    std::uniform_real_distribution<float> ux(a.x, b.x);
//    std::uniform_real_distribution<float> uy(a.y, b.y);
//    std::uniform_real_distribution<float> uz(a.z, b.z);
//    return glm::vec3(ux(rng), uy(rng), uz(rng));
//}
//
//static void draw_path_segments(
//    Scene& scene,
//    const std::vector<glm::vec3>& verts,
//    const glm::vec3& color,
//    float rayRadius = -1.0f,
//    int raySides = 12
//) {
//    for (size_t i = 0; i + 1 < verts.size(); ++i) {
//        glm::vec3 p0 = verts[i];
//        glm::vec3 p1 = verts[i + 1];
//        glm::vec3 seg = p1 - p0;
//
//        float len = std::sqrt(glm::dot(seg, seg));
//        if (len <= 1e-6f) continue;
//
//        glm::vec3 d = seg / len;
//        scene.draw_ray(p0, d, len, color, rayRadius, raySides);
//    }
//}
//
//int main() {
//    Scene scene;
//    if (!scene.load("C:/Users/neels/source/repos/PathMutation/Scenes/cornell-box.obj")) {
//        std::cout << "Failed to load scene\n";
//        return 1;
//    }
//
//    glm::vec3 bmin = scene.bounds_min();
//    glm::vec3 bmax = scene.bounds_max();
//    glm::vec3 diag = bmax - bmin;
//
//    float cx = 0.5f * (bmin.x + bmax.x);
//    float cy = 0.5f * (bmin.y + bmax.y);
//    float cz = 10.0f;
//    glm::vec3 camera_center(cx, cy, cz);
//
//    glm::vec3 light_pos =
//        bmin + glm::vec3(0.5f) * (bmax - bmin) + glm::vec3(0.0f, 0.35f * diag.y, 0.0f);
//
//    PathMutator pm(scene, camera_center, light_pos);
//
//    std::random_device rd_seed;
//    std::mt19937 rng(rd_seed());
//
//    glm::vec3 target = random_in_box(rng, bmin, bmax);
//    glm::vec3 initDir = GeomUtil::safe_normalize(target - camera_center);
//
//    PathMutator::Path path;
//    bool ok = pm.sample_path(path, 8, initDir, 12);
//
//    if (path.vertices.size() < 3) {
//        std::cout << "Path too short to mutate\n";
//        return 1;
//    }
//
//    int idx = 1;
//    if ((int)path.vertices.size() > 3) idx = (int)path.vertices.size() / 2;
//
//    float sceneDiag = std::sqrt(glm::dot(diag, diag));
//    float radius = 0.25f * sceneDiag;
//
//    const int TRIES = 64;
//    int successCount = 0;
//    int failCount = 0;
//
//    PathMutator::Path mutated = path;
//    bool mok = false;
//
//    for (int attempt = 0; attempt < TRIES; ++attempt) {
//        PathMutator::Path cand = path;
//
//        bool thisOk = pm.mutate_vertex_retrace(cand, idx);
//
//        if (thisOk) {
//            successCount++;
//            mok = true;
//            mutated = std::move(cand);
//        }
//        else {
//            failCount++;
//        }
//    }
//
//    std::cout << "original valid=" << (ok ? "true" : "false")
//        << " vertices=" << path.vertices.size() << "\n";
//    std::cout << "mutation tries=" << TRIES
//        << " success=" << successCount
//        << " fail=" << failCount
//        << " mutate_index=" << idx << "\n";
//
//    const glm::vec3 green(0.0f, 1.0f, 0.0f);
//    const glm::vec3 red(1.0f, 0.0f, 0.0f);
//
//    float rayRadius = -1.0f;
//    int raySides = 12;
//
//    draw_path_segments(scene, path.vertices, green, rayRadius, raySides);
//    if (mok) {
//        draw_path_segments(scene, mutated.vertices, red, rayRadius, raySides);
//    }
//    else {
//        std::cout << "No successful mutation found; exporting only original path.\n";
//    }
//
//    const std::string outObj =
//        "C:/Users/neels/source/repos/PathMutation/Scenes/cornell_original_and_mutated.obj";
//
//    if (!scene.export_obj(outObj)) {
//        std::cout << "Failed to export obj\n";
//        return 1;
//    }
//
//    std::cout << "Exported: " << outObj << "\n";
//    return 0;
//}
