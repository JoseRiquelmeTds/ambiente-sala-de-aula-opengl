#include "objects/projector.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

constexpr float kPlateRadius = 0.06f;
constexpr float kPlateThickness = 0.012f;
constexpr float kRodRadius = 0.012f;
constexpr float kRodLength = 0.20f;

// Ponta da haste: pivô da inclinação. Abaixo dele, tudo no espaço da cabeça.
constexpr glm::vec3 kTiltPivot(0.0f, -(kPlateThickness + kRodLength), 0.0f);

constexpr glm::vec3 kBracketSize(0.10f, 0.012f, 0.10f);
constexpr glm::vec3 kBodySize(0.30f, 0.10f, 0.25f);

constexpr float kLensRadius = 0.035f;
constexpr float kLensLength = 0.03f;
// Base da lente: centro da face da frente do corpo, na meia altura dele.
constexpr glm::vec3 kLensBase(0.0f, -(kBracketSize.y + kBodySize.y * 0.5f), kBodySize.z * 0.5f);

// Plástico do gabinete repete a textura a cada 25 cm; a lente estica a imagem.
constexpr float kMaterialTextureSize = 0.25f;

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

}  // namespace

Projector::Projector()
    : ceilingPlate_(primitives::cylinder(kPlateRadius, kPlateThickness, 32, kMaterialTextureSize)),
      rod_(primitives::cylinder(kRodRadius, kRodLength, 16, kMaterialTextureSize)),
      bracket_(primitives::box(kBracketSize, kMaterialTextureSize)),
      body_(primitives::box(kBodySize, kMaterialTextureSize)),
      lensMesh_(primitives::cylinder(kLensRadius, kLensLength, 32)) {}

SceneNode& Projector::addTo(SceneNode& parent, const glm::mat4& placement) {
    if (root_ != nullptr) {
        return *root_;
    }
    root_ = &parent.addChild(nullptr, placement);

    // As primitivas crescem de y = 0 para cima: para pendurar, desce a altura da própria peça.
    root_->addChild(&ceilingPlate_, translated({0.0f, -kPlateThickness, 0.0f}));
    root_->addChild(&rod_, translated(kTiltPivot));

    head_ = &root_->addChild(nullptr, headTransform());
    head_->addChild(&bracket_, translated({0.0f, -kBracketSize.y, 0.0f}));
    head_->addChild(&body_, translated({0.0f, -(kBracketSize.y + kBodySize.y), 0.0f}));

    // Rx(+90°) leva o eixo do cilindro (+Y) para +Z: a lente sai da frente do corpo.
    glm::mat4 lensTransform = translated(kLensBase);
    lensTransform = glm::rotate(lensTransform, glm::radians(90.0f), {1.0f, 0.0f, 0.0f});
    lens_ = &head_->addChild(&lensMesh_, lensTransform);

    return *root_;
}

void Projector::aimAt(const glm::vec3& target) {
    // Alvo relativo ao pivô, no plano YZ. O eixo da lente é a reta y = ly (abaixo do pivô)
    // no espaço da cabeça. Girado por Rx(θ), ele passa pelo alvo t quando a inversa da rotação
    // leva t para essa reta: ty·cos θ + tz·sen θ = ly, ou seja, |t|·cos(θ - atan2(tz, ty)) = ly.
    const float ty = target.y - kTiltPivot.y;
    const float tz = target.z - kTiltPivot.z;
    const float distance = std::sqrt(ty * ty + tz * tz);
    const float ly = kLensBase.y;
    if (!std::isfinite(distance) || distance <= std::abs(ly)) {
        return;
    }
    tiltRadians_ = std::atan2(tz, ty) - std::acos(ly / distance);
    if (head_ != nullptr) {
        head_->transform = headTransform();
    }
}

float Projector::tiltDegrees() const {
    return glm::degrees(tiltRadians_);
}

glm::vec3 Projector::lensCenter() const {
    const glm::vec3 front = kLensBase + glm::vec3(0.0f, 0.0f, kLensLength);
    return glm::vec3(headTransform() * glm::vec4(front, 1.0f));
}

glm::vec3 Projector::lensDirection() const {
    return glm::normalize(glm::vec3(headTransform() * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
}

glm::mat4 Projector::headTransform() const {
    return glm::rotate(translated(kTiltPivot), tiltRadians_, {1.0f, 0.0f, 0.0f});
}

}  // namespace objects
