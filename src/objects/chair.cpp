#include "objects/chair.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "gfx/primitives.hpp"
#include "objects/furniture_style.hpp"

namespace objects {

namespace {

constexpr float kTextureSize = 0.40f;  // UV em metros, inclusive nos tubos e nas bordas das placas
constexpr float kSeatSize = 0.40f;
constexpr float kSeatTop = 0.45f;
constexpr float kSeatBottom = kSeatTop - furniture::kBoardThickness;
constexpr float kFrameHeight = kSeatBottom - furniture::kTubeRadius;
constexpr float kLegOffset = 0.168f;
constexpr float kRailLength = 2.0f * kLegOffset;
// Os montantes ficam atrás da borda do assento, sem atravessar a placa.
constexpr float kBackBaseZ = -0.218f;
constexpr float kSeatSideLength = kLegOffset - kBackBaseZ;
constexpr float kBackTilt = -8.0f;  // Y inclina para -Z ao girar em X
constexpr float kBackSupportLength = 0.420f;
constexpr float kBackBoardBase = 0.210f;  // alturas relativas ao quadro inclinado do encosto
constexpr float kBackBoardHeight = 0.220f;
constexpr float kBackBoardZ = furniture::kTubeRadius + furniture::kBoardThickness * 0.5f;
constexpr float kBraceHeight = 0.180f;
constexpr float kFootHeight = 0.024f;

// Cilindros têm pivô na base e eixo +Y. Só translação/rotação: o comprimento já está no mesh.
glm::mat4 tubePlacement(const glm::vec3& base, float degrees, const glm::vec3& axis) {
    return glm::rotate(glm::translate(glm::mat4(1.0f), base), glm::radians(degrees), axis);
}

}  // namespace

Chair::Chair()
    : seat_(primitives::box({kSeatSize, furniture::kBoardThickness, kSeatSize}, kTextureSize)),
      backrest_(primitives::box({0.38f, kBackBoardHeight, furniture::kBoardThickness}, kTextureSize)),
      leg_(primitives::cylinder(furniture::kTubeRadius, kFrameHeight, furniture::kTubeSegments, kTextureSize)),
      rail_(primitives::cylinder(furniture::kTubeRadius, kRailLength, furniture::kTubeSegments, kTextureSize)),
      seatSide_(primitives::cylinder(furniture::kTubeRadius, kSeatSideLength,
                                     furniture::kTubeSegments, kTextureSize)),
      backSupport_(primitives::cylinder(furniture::kTubeRadius, kBackSupportLength,
                                        furniture::kTubeSegments, kTextureSize)),
      foot_(primitives::cylinder(0.013f, kFootHeight, furniture::kTubeSegments, kTextureSize)),
      fastener_(primitives::cylinder(0.003f, 0.0015f, furniture::kTubeSegments, kTextureSize)) {}

SceneNode& Chair::addTo(SceneNode& parent, const glm::mat4& placement) const {
    SceneNode& root = parent.addChild(nullptr, placement);
    root.addChild(&seat_, glm::translate(glm::mat4(1.0f), {0.0f, kSeatBottom, 0.0f}));

    for (float x : {-kLegOffset, kLegOffset}) {
        for (float z : {-kLegOffset, kLegOffset}) {
            root.addChild(&leg_, glm::translate(glm::mat4(1.0f), {x, 0.0f, z}));
            // Luvas discretas de 2 mm ao redor do tubo; a base plana toca y = 0.
            root.addChild(&foot_, glm::translate(glm::mat4(1.0f), {x, 0.0f, z}));
        }
        // Laterais do quadro tocam a face inferior do assento e se prolongam até os montantes.
        root.addChild(&seatSide_, tubePlacement({x, kFrameHeight, kBackBaseZ}, 90.0f, {1, 0, 0}));
        root.addChild(&rail_, tubePlacement({x, kBraceHeight, -kLegOffset}, 90.0f, {1, 0, 0}));
    }
    for (float z : {-kLegOffset, kLegOffset}) {
        root.addChild(&rail_, tubePlacement({-kLegOffset, kFrameHeight, z}, -90.0f, {0, 0, 1}));
    }
    // Travessas laterais e traseira deixam livre a passagem dos pés pela frente.
    root.addChild(&rail_, tubePlacement({-kLegOffset, kBraceHeight, -kLegOffset}, -90.0f, {0, 0, 1}));

    // Um único referencial inclinado mantém placa, montantes e fixações alinhados.
    SceneNode& back = root.addChild(nullptr,
        tubePlacement({0.0f, kFrameHeight, kBackBaseZ}, kBackTilt, {1, 0, 0}));
    for (float x : {-kLegOffset, kLegOffset}) {
        back.addChild(&backSupport_, glm::translate(glm::mat4(1.0f), {x, 0.0f, 0.0f}));
        for (float y : {0.255f, 0.385f}) {
            const float frontZ = kBackBoardZ + furniture::kBoardThickness * 0.5f;
            back.addChild(&fastener_, tubePlacement({x, y, frontZ}, 90.0f, {1, 0, 0}));
        }
    }
    back.addChild(&rail_, tubePlacement({-kLegOffset, 0.320f, 0.0f}, -90.0f, {0, 0, 1}));
    // Face traseira tangente aos tubos; topo a aproximadamente 0,85 m do chão.
    back.addChild(&backrest_, glm::translate(glm::mat4(1.0f), {0.0f, kBackBoardBase, kBackBoardZ}));
    return root;
}

}  // namespace objects
