#include "objects/trash_can.hpp"

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>

namespace objects {

namespace {

constexpr float kBottomRadius = 0.13f;
constexpr float kTopRadius = 0.15f;
constexpr float kHeight = 0.40f;
constexpr float kWallThickness = 0.006f;
constexpr float kFloorThickness = 0.008f;
constexpr int kSegments = 40;

// Plástico contínuo: a textura repete a cada 25 cm.
constexpr float kTextureSize = 0.25f;

// Raio da face de fora na altura y (o tronco abre linearmente do fundo à boca).
float outerRadiusAt(float y) {
    return kBottomRadius + (kTopRadius - kBottomRadius) * y / kHeight;
}

// Lateral de tronco de cone entre (r0, y0) e (r1, y1). A volta começa em +Z e gira para +X,
// como em primitives::cylinder, e a última coluna repete a primeira com u no fim da volta.
void addFrustumWall(MeshData& data, float r0, float r1, float y0, float y1, bool inward) {
    const float step = glm::two_pi<float>() / static_cast<float>(kSegments);
    // Arco na meia altura: desenrolado, o tronco vira um setor de anel e a textura abre um pouco em cima.
    // Arredondado para um número inteiro de repetições, senão a textura não fecha na costura.
    const float arc = glm::two_pi<float>() * (r0 + r1) * 0.5f;
    const float uMax = std::max(1.0f, std::round(arc / kTextureSize));
    const GLuint base = static_cast<GLuint>(data.vertices.size());

    for (int i = 0; i <= kSegments; ++i) {
        const float angle = step * static_cast<float>(i);
        const glm::vec3 radial(std::sin(angle), 0.0f, std::cos(angle));
        // A geratriz sobe (r1 - r0) para fora a cada (y1 - y0) de altura; a perpendicular a ela no
        // plano radial é (y1 - y0, -(r1 - r0)). Com a boca mais larga, a normal aponta um pouco para baixo.
        glm::vec3 normal = glm::normalize(radial * (y1 - y0) + glm::vec3(0.0f, -(r1 - r0), 0.0f));
        if (inward) {
            normal = -normal;
        }
        const float u = uMax * static_cast<float>(i) / static_cast<float>(kSegments);
        data.vertices.push_back({radial * r0 + glm::vec3(0.0f, y0, 0.0f), normal, {u, y0 / kTextureSize}});
        data.vertices.push_back({radial * r1 + glm::vec3(0.0f, y1, 0.0f), normal, {u, y1 / kTextureSize}});
    }

    for (GLuint i = 0; i < static_cast<GLuint>(kSegments); ++i) {
        const GLuint bottom = base + 2 * i;
        const GLuint top = bottom + 1;
        const GLuint nextBottom = bottom + 2;
        const GLuint nextTop = bottom + 3;
        if (inward) {
            // Vista de dentro, a volta anda para a esquerda: a ordem inverte para continuar anti-horária.
            data.indices.insert(data.indices.end(), {bottom, nextTop, nextBottom, nextTop, bottom, top});
        } else {
            data.indices.insert(data.indices.end(), {bottom, nextBottom, nextTop, nextTop, top, bottom});
        }
    }
}

// UV planar vista de cima ou de baixo, em metros / kTextureSize, como as tampas das primitivas.
glm::vec2 planarUv(const glm::vec3& position, float normalY) {
    return {position.x / kTextureSize, -position.z * normalY / kTextureSize};
}

// Anel horizontal entre innerRadius e outerRadius: a espessura visível da borda.
void addRing(MeshData& data, float innerRadius, float outerRadius, float y, float normalY) {
    const float step = glm::two_pi<float>() / static_cast<float>(kSegments);
    const glm::vec3 normal(0.0f, normalY, 0.0f);
    const GLuint base = static_cast<GLuint>(data.vertices.size());

    for (int i = 0; i <= kSegments; ++i) {
        const float angle = step * static_cast<float>(i);
        const glm::vec3 radial(std::sin(angle), 0.0f, std::cos(angle));
        const glm::vec3 inner = radial * innerRadius + glm::vec3(0.0f, y, 0.0f);
        const glm::vec3 outer = radial * outerRadius + glm::vec3(0.0f, y, 0.0f);
        data.vertices.push_back({inner, normal, planarUv(inner, normalY)});
        data.vertices.push_back({outer, normal, planarUv(outer, normalY)});
    }

    for (GLuint i = 0; i < static_cast<GLuint>(kSegments); ++i) {
        const GLuint inner = base + 2 * i;
        const GLuint outer = inner + 1;
        const GLuint nextInner = inner + 2;
        const GLuint nextOuter = inner + 3;
        // Visto de cima, (dentro, fora, próximo fora) é anti-horário; visto de baixo, inverte.
        if (normalY > 0.0f) {
            data.indices.insert(data.indices.end(), {inner, outer, nextOuter, nextOuter, nextInner, inner});
        } else {
            data.indices.insert(data.indices.end(), {inner, nextOuter, outer, nextOuter, inner, nextInner});
        }
    }
}

// Disco horizontal em leque a partir do centro.
void addDisc(MeshData& data, float radius, float y, float normalY) {
    const float step = glm::two_pi<float>() / static_cast<float>(kSegments);
    const glm::vec3 normal(0.0f, normalY, 0.0f);
    const GLuint center = static_cast<GLuint>(data.vertices.size());

    const glm::vec3 centerPosition(0.0f, y, 0.0f);
    data.vertices.push_back({centerPosition, normal, planarUv(centerPosition, normalY)});
    for (int i = 0; i <= kSegments; ++i) {
        const float angle = step * static_cast<float>(i);
        const glm::vec3 position(std::sin(angle) * radius, y, std::cos(angle) * radius);
        data.vertices.push_back({position, normal, planarUv(position, normalY)});
    }

    for (GLuint i = 0; i < static_cast<GLuint>(kSegments); ++i) {
        const GLuint current = center + 1 + i;
        const GLuint next = current + 1;
        if (normalY > 0.0f) {
            data.indices.insert(data.indices.end(), {center, current, next});
        } else {
            data.indices.insert(data.indices.end(), {center, next, current});
        }
    }
}

MeshData makeBody() {
    const float innerTop = kTopRadius - kWallThickness;
    const float innerFloor = outerRadiusAt(kFloorThickness) - kWallThickness;

    MeshData data;
    addFrustumWall(data, kBottomRadius, kTopRadius, 0.0f, kHeight, false);
    addFrustumWall(data, innerFloor, innerTop, kFloorThickness, kHeight, true);
    addRing(data, innerTop, kTopRadius, kHeight, 1.0f);
    addDisc(data, kBottomRadius, 0.0f, -1.0f);
    addDisc(data, innerFloor, kFloorThickness, 1.0f);
    return data;
}

}  // namespace

TrashCan::TrashCan() : body_(makeBody()) {}

SceneNode& TrashCan::addTo(SceneNode& parent, const glm::mat4& placement) const {
    return parent.addChild(&body_, placement);
}

}  // namespace objects
