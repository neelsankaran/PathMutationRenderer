#include "GeomUtil.h"

glm::vec3 GeomUtil::safe_normalize(const glm::vec3& v) {
    float len2 = glm::dot(v, v);
    if (len2 <= 0.0f) return glm::vec3(0.0f, 0.0f, 0.0f);
    return v / std::sqrt(len2);
}

void GeomUtil::make_orthonormal_basis(
    const glm::vec3& w,
    glm::vec3& u,
    glm::vec3& v
) {
    glm::vec3 a = (std::fabs(w.x) < 0.9f)
        ? glm::vec3(1, 0, 0)
        : glm::vec3(0, 1, 0);

    u = safe_normalize(glm::cross(a, w));
    v = glm::cross(w, u);
}

bool GeomUtil::moller_trumbore(
    const glm::vec3& ro,
    const glm::vec3& rd,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    float& t,
    float& u,
    float& v,
    float eps
) {
    const glm::vec3 e1 = v1 - v0;
    const glm::vec3 e2 = v2 - v0;

    const glm::vec3 pvec = glm::cross(rd, e2);
    const float det = glm::dot(e1, pvec);

    if (std::fabs(det) < eps) return false;

    const float invDet = 1.0f / det;

    const glm::vec3 tvec = ro - v0;

    u = glm::dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f) return false;

    const glm::vec3 qvec = glm::cross(tvec, e1);

    v = glm::dot(rd, qvec) * invDet;
    if (v < 0.0f || (u + v) > 1.0f) return false;

    t = glm::dot(e2, qvec) * invDet;
    if (t <= 0.0f) return false;

    return true;
}

glm::vec3 GeomUtil::sample_hemisphere_uniform(
    const glm::vec3& n,
    std::mt19937& rng,
    std::uniform_real_distribution<float>& u01
) {
    float z = u01(rng);
    float phi = 6.283185307179586f * u01(rng);
    float r = std::sqrt(std::max(0.0f, 1.0f - z * z));

    glm::vec3 local{
        r * std::cos(phi),
        r * std::sin(phi),
        z
    };

    glm::vec3 w = safe_normalize(n);
    if (glm::dot(w, w) == 0.0f) w = glm::vec3(0, 1, 0);

    glm::vec3 u, v;
    make_orthonormal_basis(w, u, v);

    return safe_normalize(local.x * u + local.y * v + local.z * w);
}

glm::vec3 GeomUtil::face_normal_geom(const Triangle& t) {
    return safe_normalize(glm::cross(t.v1 - t.v0, t.v2 - t.v0));
}

glm::vec3 GeomUtil::project_to_plane(const glm::vec3& d, const glm::vec3& n) {
    return d - n * glm::dot(d, n);
}

float GeomUtil::cross2(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
}

glm::vec3 GeomUtil::rodrigues(
    const glm::vec3& x,
    const glm::vec3& k_unit,
    float theta
) {
    float c = std::cos(theta);
    float s = std::sin(theta);
    return x * c
        + glm::cross(k_unit, x) * s
        + k_unit * (glm::dot(k_unit, x)) * (1.0f - c);
}

float GeomUtil::signed_dihedral(
    const glm::vec3& n0,
    const glm::vec3& n1,
    const glm::vec3& e_unit
) {
    float c = std::clamp(glm::dot(n0, n1), -1.0f, 1.0f);
    float theta = std::acos(c);

    float sgn = glm::dot(e_unit, glm::cross(n0, n1));
    if (sgn < 0.0f) theta = -theta;

    return theta;
}

void GeomUtil::edge_endpoints(
    const Triangle& t,
    int edgeIdx,
    glm::vec3& a,
    glm::vec3& b
) {
    if (edgeIdx == 0) { a = t.v0; b = t.v1; return; }
    if (edgeIdx == 1) { a = t.v1; b = t.v2; return; }
    a = t.v2; b = t.v0;
}
