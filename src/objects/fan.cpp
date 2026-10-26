#include "objects/fan.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

#include "gfx/primitives.hpp"

namespace objects {
namespace {

constexpr glm::vec3 kRotorCenter(0.0f, 0.16f, 0.30f);
constexpr float kGuardRadius = 0.275f;
constexpr float kWireRadius = 0.0018f;
constexpr int kRingSegments = 80;
constexpr int kWireSegments = 6;

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

// Cilindros existentes têm base em Y = 0; esta transformação leva o eixo Y ao segmento.
glm::mat4 alongSegment(const glm::vec3& start, const glm::vec3& end) {
    const glm::vec3 direction = glm::normalize(end - start);
    const glm::vec3 helper = std::abs(direction.z) < 0.9f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
    const glm::vec3 x = glm::normalize(glm::cross(direction, helper));
    glm::mat4 transform(1.0f);
    transform[0] = glm::vec4(x, 0.0f);
    transform[1] = glm::vec4(direction, 0.0f);
    transform[2] = glm::vec4(glm::cross(x, direction), 0.0f);
    transform[3] = glm::vec4(start, 1.0f);
    return transform;
}

// Agrega fios numa única malha por grade, sem criar um SceneNode por haste/aro.
void appendMesh(MeshData& target, const MeshData& source, const glm::mat4& transform) {
    const GLuint offset = static_cast<GLuint>(target.vertices.size());
    for (const Vertex& vertex : source.vertices) {
        target.vertices.push_back({glm::vec3(transform * glm::vec4(vertex.position, 1.0f)),
                                   glm::normalize(glm::mat3(transform) * vertex.normal), vertex.uv});
    }
    for (GLuint index : source.indices) {
        target.indices.push_back(offset + index);
    }
}

void addWire(MeshData& data, const glm::vec3& start, const glm::vec3& end, float radius) {
    appendMesh(data, primitives::cylinder(radius, glm::length(end - start), kWireSegments, 0.1f),
               alongSegment(start, end));
}

// Toro específico dos aros da grade, centrado em XY. Costuras duplicadas preservam a UV.
void addRing(MeshData& data, float radius, float z, float wireRadius) {
    const GLuint offset = static_cast<GLuint>(data.vertices.size());
    for (int i = 0; i <= kRingSegments; ++i) {
        const float u = static_cast<float>(i) / kRingSegments;
        const float angle = glm::two_pi<float>() * u;
        const glm::vec3 radial(std::cos(angle), std::sin(angle), 0.0f);
        for (int j = 0; j <= kWireSegments; ++j) {
            const float v = static_cast<float>(j) / kWireSegments;
            const float tubeAngle = glm::two_pi<float>() * v;
            const glm::vec3 normal = radial * std::cos(tubeAngle) + glm::vec3(0, 0, std::sin(tubeAngle));
            data.vertices.push_back({radial * radius + glm::vec3(0, 0, z) + wireRadius * normal,
                                     normal, {u, v}});
        }
    }
    for (int i = 0; i < kRingSegments; ++i) {
        for (int j = 0; j < kWireSegments; ++j) {
            const GLuint a = offset + static_cast<GLuint>(i * (kWireSegments + 1) + j);
            const GLuint b = a + kWireSegments + 1;
            data.indices.insert(data.indices.end(), {a, b, b + 1, a, b + 1, a + 1});
        }
    }
}

float guardDepth(float radius, bool front) {
    // Domos afastados do envelope das pás (|z| < 0,023 m). Borda comum fora do rotor.
    const float dome = 1.0f - (radius * radius) / (kGuardRadius * kGuardRadius);
    return front ? 0.025f + 0.080f * dome : -0.025f - 0.060f * dome;
}

MeshData makeGuard(bool front) {
    MeshData data;
    const float innerRadius = front ? 0.035f : 0.080f;  // abertura traseira abraça o motor
    for (int ring = 0; ring <= 5; ++ring) {
        const float radius = innerRadius + (kGuardRadius - innerRadius) * static_cast<float>(ring) / 5.0f;
        addRing(data, radius, guardDepth(radius, front), ring == 5 ? 0.003f : kWireRadius);
    }
    for (int spoke = 0; spoke < 24; ++spoke) {
        const float angle = glm::two_pi<float>() * static_cast<float>(spoke) / 24.0f;
        const glm::vec3 radial(std::cos(angle), std::sin(angle), 0.0f);
        for (int segment = 0; segment < 5; ++segment) {
            const float r0 = innerRadius + (kGuardRadius - innerRadius) * static_cast<float>(segment) / 5.0f;
            const float r1 = innerRadius + (kGuardRadius - innerRadius) * static_cast<float>(segment + 1) / 5.0f;
            addWire(data, radial * r0 + glm::vec3(0, 0, guardDepth(r0, front)),
                    radial * r1 + glm::vec3(0, 0, guardDepth(r1, front)), kWireRadius);
        }
        // Liga as duas bordas só na malha traseira; tudo fica além do raio das hélices.
        if (!front && spoke % 3 == 0) {
            addWire(data, radial * kGuardRadius + glm::vec3(0, 0, -0.025f),
                    radial * kGuardRadius + glm::vec3(0, 0, 0.025f), 0.0025f);
        }
    }
    return data;
}

// Pá varrida, alargada no meio e torcida: duas superfícies e borda fechada de 3 mm.
glm::vec3 bladePoint(float t, float across, float side) {
    const float width = 0.010f + 0.090f * std::pow(std::max(0.0f, std::sin(glm::pi<float>() * t)), 0.85f);
    const float x = across * width;
    return {0.032f * t * t + x, 0.035f + 0.212f * t,
            x * (0.38f - 0.18f * t) + 0.004f * std::sin(glm::pi<float>() * t) + side * 0.0015f};
}

void addBladeQuad(MeshData& data, const glm::vec3& a, const glm::vec3& b,
                  const glm::vec3& c, const glm::vec3& d, const glm::vec2& uv0, const glm::vec2& uv1) {
    const GLuint offset = static_cast<GLuint>(data.vertices.size());
    const glm::vec3 normal = glm::normalize(glm::cross(b - a, d - a));
    data.vertices.insert(data.vertices.end(), {{a, normal, uv0}, {b, normal, {uv1.x, uv0.y}},
                                             {c, normal, uv1}, {d, normal, {uv0.x, uv1.y}}});
    data.indices.insert(data.indices.end(), {offset, offset + 1, offset + 2, offset, offset + 2, offset + 3});
}

MeshData makeBlade() {
    MeshData data;
    constexpr int kSections = 24;
    for (int i = 0; i < kSections; ++i) {
        const float t0 = static_cast<float>(i) / kSections;
        const float t1 = static_cast<float>(i + 1) / kSections;
        const glm::vec3 a = bladePoint(t0, -0.5f, 1), b = bladePoint(t0, 0.5f, 1);
        const glm::vec3 c = bladePoint(t1, 0.5f, 1), d = bladePoint(t1, -0.5f, 1);
        const glm::vec3 e = bladePoint(t0, -0.5f, -1), f = bladePoint(t0, 0.5f, -1);
        const glm::vec3 g = bladePoint(t1, 0.5f, -1), h = bladePoint(t1, -0.5f, -1);
        addBladeQuad(data, a, b, c, d, {0, t0}, {1, t1});
        addBladeQuad(data, f, e, h, g, {0, t0}, {1, t1});
        addBladeQuad(data, e, a, d, h, {0, t0}, {1, t1});
        addBladeQuad(data, b, f, g, c, {0, t0}, {1, t1});
        if (i == 0) {
            addBladeQuad(data, e, f, b, a, {0, 0}, {1, 1});
        }
        if (i == kSections - 1) {
            addBladeQuad(data, d, c, g, h, {0, 0}, {1, 1});
        }
    }
    return data;
}

MeshData makeSupport() {
    MeshData data;
    addWire(data, {0, -0.015f, 0.015f}, {0, -0.015f, 0.14f}, 0.018f);
    addWire(data, {0, -0.015f, 0.14f}, {0, 0.16f, 0.14f}, 0.018f);
    return data;
}

}  // namespace

Fan::Fan()
    : mount_(primitives::cylinder(0.070f, 0.018f, 40, 0.1f)),
      support_(makeSupport()),
      motor_(primitives::cylinder(0.065f, 0.180f, 40, 0.1f)),
      shaft_(primitives::cylinder(0.012f, 0.045f, 20, 0.1f)),
      blade_(makeBlade()),
      hub_(primitives::cylinder(0.046f, 0.032f, 40)),
      frontGuard_(makeGuard(true)),
      rearGuard_(makeGuard(false)) {}

SceneNode& Fan::addTo(SceneNode& parent, const glm::mat4& placement) {
    if (root_ != nullptr) {
        return *root_;
    }
    root_ = &parent.addChild(nullptr, placement);
    SceneNode& fixed = root_->addChild(nullptr);
    fixed.addChild(&mount_, alongSegment({0, 0, 0}, {0, 0, 0.018f}));
    fixed.addChild(&support_);
    fixed.addChild(&motor_, alongSegment({0, 0.16f, 0.075f}, {0, 0.16f, 0.255f}));
    fixed.addChild(&shaft_, alongSegment({0, 0.16f, 0.255f}, kRotorCenter));
    fixed.addChild(&frontGuard_, translated(kRotorCenter));
    fixed.addChild(&rearGuard_, translated(kRotorCenter));

    rotor_ = &root_->addChild(nullptr);
    for (int blade = 0; blade < 3; ++blade) {
        rotor_->addChild(&blade_, glm::rotate(glm::mat4(1.0f), glm::radians(120.0f * blade), {0, 0, 1}));
    }
    rotor_->addChild(&hub_, alongSegment({0, 0, -0.012f}, {0, 0, 0.020f}));
    setRotorAngle(rotorAngle_);
    return *root_;
}

void Fan::setRotorAngle(float degrees) {
    if (!std::isfinite(degrees)) {
        return;
    }
    rotorAngle_ = std::remainder(degrees, 360.0f);
    if (rotor_ != nullptr) {
        rotor_->transform = glm::rotate(translated(kRotorCenter), glm::radians(rotorAngle_), {0, 0, 1});
    }
}

}  // namespace objects
