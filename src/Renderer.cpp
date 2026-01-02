#include "Renderer.h"

#include <algorithm>
#include <fstream>
#include <cmath>
#include <iostream>

static constexpr float PI = 3.14159265358979323846f;

Renderer::Renderer(const Scene& scene,
    const Camera& cam,
    const glm::vec3& lightPos,
    const RenderParams& params)
    : m_scene(scene)
    , m_cam(cam)
    , m_lightPos(lightPos)
    , m_params(params)
    , m_mutator(scene, cam.center, lightPos)
{
    glm::vec3 diag = m_scene.bounds_max() - m_scene.bounds_min();
    m_sceneDiag = std::sqrt(glm::dot(diag, diag));
    if (!(m_sceneDiag > 0.0f)) m_sceneDiag = 1.0f;

    m_cam.forward = GeomUtil::safe_normalize(m_cam.forward);
    if (glm::dot(m_cam.forward, m_cam.forward) <= 0.0f) {
        m_cam.forward = glm::vec3(0, 0, 1);
    }
}

float Renderer::clamp01(float x) {
    return std::max(0.0f, std::min(1.0f, x));
}

glm::vec3 Renderer::generate_primary_dir(int px, int py) const {
    glm::vec3 f = GeomUtil::safe_normalize(m_cam.forward);

    glm::vec3 up = m_cam.world_up;
    if (glm::dot(up, up) <= 0.0f) up = glm::vec3(0, 1, 0);

    glm::vec3 r = glm::cross(f, up);
    r = GeomUtil::safe_normalize(r);

    if (glm::dot(r, r) <= 0.0f) {
        up = glm::vec3(0, 0, 1);
        r = GeomUtil::safe_normalize(glm::cross(f, up));
        if (glm::dot(r, r) <= 0.0f) r = glm::vec3(1, 0, 0);
    }

    glm::vec3 u = glm::cross(r, f);

    float cx = (float)m_cam.width * 0.5f;
    float cy = (float)m_cam.height * 0.5f;

    float x = ((float)px + 0.5f - cx) * m_cam.pixel_size;
    float y = (cy - ((float)py + 0.5f)) * m_cam.pixel_size;

    glm::vec3 dir = f * m_cam.focal_length + r * x + u * y;
    dir = GeomUtil::safe_normalize(dir);
    if (glm::dot(dir, dir) <= 0.0f) dir = f;

    return dir;
}

glm::vec3 Renderer::shading_normal_at(const PathMutator::Path& path, int i) const {
    if (i < 0 || i >= (int)path.vertices.size()) return glm::vec3(0.0f);
    if (i >= (int)path.faces.size()) return glm::vec3(0.0f);
    if (i >= (int)path.bary_points.size()) return glm::vec3(0.0f);

    const GeomUtil::Triangle& tri = path.faces[i];
    const glm::vec3 bary = path.bary_points[i];

    glm::vec3 n = bary.x * tri.n0 + bary.y * tri.n1 + bary.z * tri.n2;
    float n2 = glm::dot(n, n);

    if (n2 <= 0.0f) {
        n = GeomUtil::face_normal_geom(tri);
        n2 = glm::dot(n, n);
        if (n2 <= 0.0f) return glm::vec3(0.0f);
    }

    return n / std::sqrt(n2);
}

glm::vec3 Renderer::compute_radiance_for_path(const PathMutator::Path& path) const {
    const int N = (int)path.vertices.size();
    if (N < 3) return glm::vec3(0.0f);

    const int nInternal = std::max(0, N - 2);
    const bool hasCachedLightVis = ((int)path.light_visible.size() == nInternal);

    const glm::vec3 albedo = m_params.albedo;
    const glm::vec3 f_lam = albedo / PI;
    const float pdf_hemi_uniform = 1.0f / (2.0f * PI);

    glm::vec3 L(0.0f);
    glm::vec3 beta(1.0f);

    for (int i = 1; i <= N - 2; ++i) {
        const glm::vec3& xi = path.vertices[i];

        glm::vec3 ni = shading_normal_at(path, i);
        float ni2 = glm::dot(ni, ni);
        if (ni2 <= 0.0f) {
            return L;
        }
        ni /= std::sqrt(ni2);

        {
            glm::vec3 toL = m_lightPos - xi;
            float dist2 = glm::dot(toL, toL);
            if (dist2 > 1e-12f) {
                float dist = std::sqrt(dist2);
                glm::vec3 wi = toL / dist;

                float cosTheta = std::max(0.0f, glm::dot(ni, wi));
                if (cosTheta > 0.0f) {
                    bool vis = false;

                    if (hasCachedLightVis) {
                        vis = path.light_visible[i - 1];
                    }
                    else {
                        
                        glm::vec3 xo = xi + m_params.visibilityEps * ni;
                        vis = m_scene.visible(xo, m_lightPos, m_params.visibilityEps);
                    }

                    if (vis) {
                        glm::vec3 Li = m_params.lightIntensity / dist2;
                        L += beta * (f_lam * (Li * cosTheta));
                    }
                }
            }
        }

        if (i < N - 2) {
            const glm::vec3& xnext = path.vertices[i + 1];
            glm::vec3 wo = GeomUtil::safe_normalize(xnext - xi);
            if (glm::dot(wo, wo) <= 0.0f) break;

            float cosOut = std::max(0.0f, glm::dot(ni, wo));
            if (cosOut <= 0.0f) break;

            beta *= (f_lam * (cosOut / pdf_hemi_uniform));
        }
    }

    return L;
}

std::vector<glm::vec3> Renderer::render_scene(uint32_t seed, const int mutator_type) const {
    const int W = m_cam.width;
    const int H = m_cam.height;

    std::vector<glm::vec3> img((size_t)W * (size_t)H, glm::vec3(0.0f));

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);

    const float baseRadius = std::max(1e-6f, m_params.mutateRadiusFrac * m_sceneDiag);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            glm::vec3 rd0 = generate_primary_dir(x, y);

            PathMutator::Path cur;

            bool okSeed = m_mutator.sample_path(
                cur,
                m_params.maxBounces,
                rd0,
                m_params.retriesPerBounce
            );

            if (!okSeed) {
                img[(size_t)y * (size_t)W + (size_t)x] = glm::vec3(0.0f);
                continue;
            }

            glm::vec3 accum(0.0f);
            int accepted = 0;

            accum += compute_radiance_for_path(cur);
            accepted++;

            const int K = std::max(0, m_params.Kmutations);

            for (int k = 0; k < K; ++k) {
                PathMutator::Path proposal = cur;

                const int N = (int)proposal.vertices.size();
                if (N <= 2) break;

                const int lo = 2;
                const int hi = N - 2;
                if (hi < lo) break;

                int idx = lo + (int)std::floor(u01(rng) * (float)(hi - lo + 1));
                idx = std::max(lo, std::min(hi, idx));

                bool ok = false;

                if (mutator_type == 0) {
                    ok = m_mutator.mutate_vertex_retrace(proposal, idx);
                }
                else if (mutator_type == 1) {
                    ok = m_mutator.mutate_vertex_meshwalk(proposal, idx, baseRadius);
                }
                else if (mutator_type == 2) {
                    ok = m_mutator.mutate_vertex_project(proposal, idx, baseRadius);
                }
                else if (mutator_type == 3) {
                    PathMutator::Path fresh;
                    ok = m_mutator.sample_path(
                        fresh,
                        m_params.maxBounces,
                        rd0,
                        m_params.retriesPerBounce
                    );
                    if (ok) proposal = std::move(fresh);
                }
                else {
                    ok = false;
                }

                if (!ok) {
                    continue;
                }

                cur = std::move(proposal);
                accum += compute_radiance_for_path(cur);
                accepted++;
            }

            if (accepted > 0) accum /= (float)accepted;
            img[(size_t)y * (size_t)W + (size_t)x] = accum;
        }
    }

    return img;
}

bool Renderer::write_ppm(const std::string& path,
    const std::vector<glm::vec3>& img,
    int W, int H,
    float gamma)
{
    if ((int)img.size() != W * H) return false;

    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    out << "P6\n" << W << " " << H << "\n255\n";

    auto to_byte = [&](float c) -> unsigned char {
        c = std::max(0.0f, c);
        if (gamma > 0.0f) c = std::pow(c, 1.0f / gamma);
        c = std::min(1.0f, c);
        return (unsigned char)std::lround(c * 255.0f);
    };

    for (int i = 0; i < W * H; ++i) {
        unsigned char r = to_byte(img[i].x);
        unsigned char g = to_byte(img[i].y);
        unsigned char b = to_byte(img[i].z);
        out.write((char*)&r, 1);
        out.write((char*)&g, 1);
        out.write((char*)&b, 1);
    }

    return true;
}
