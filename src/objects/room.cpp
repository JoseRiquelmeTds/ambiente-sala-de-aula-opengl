#include "objects/room.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

constexpr float kWallThickness = 0.10f;
constexpr float kFloorThickness = 0.12f;
constexpr float kCeilingThickness = 0.10f;
constexpr float kSkirtingHeight = 0.08f;
constexpr float kSkirtingDepth = 0.018f;
// Embutir 2 mm esconde as costas do rodapé dentro da parede, sem faces expostas coplanares.
constexpr float kSkirtingInset = 0.002f;
constexpr float kSkirtingOffset = kSkirtingDepth / 2.0f - kSkirtingInset;

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

}  // namespace

Room::Room()
    : floor_(primitives::box({kWidth, kFloorThickness, kDepth}, 1.0f)),
      endWall_(primitives::box({kWidth + 2.0f * kWallThickness, kHeight, kWallThickness}, 1.0f)),
      sideWall_(primitives::box({kWallThickness, kHeight, kDepth}, 1.0f)),
      ceiling_(primitives::box({kWidth, kCeilingThickness, kDepth}, 1.0f)),
      endSkirting_(primitives::box({kWidth, kSkirtingHeight, kSkirtingDepth}, 0.5f)),
      sideSkirting_(primitives::box({kSkirtingDepth, kSkirtingHeight,
                                   kDepth - 2.0f * (kSkirtingDepth - kSkirtingInset)}, 0.5f)) {}

SceneNode& Room::addTo(SceneNode& parent, const glm::mat4& placement) const {
    SceneNode& root = parent.addChild(nullptr, placement);
    // box cresce para +Y: o piso termina em 0 e o teto começa em 3, sem invadir a sala.
    root.addChild(&floor_, translated({0.0f, -kFloorThickness, 0.0f}));
    root.addChild(&endWall_, translated({0.0f, 0.0f, -(kDepth + kWallThickness) / 2.0f}));
    root.addChild(&endWall_, translated({0.0f, 0.0f, (kDepth + kWallThickness) / 2.0f}));
    root.addChild(&sideWall_, translated({-(kWidth + kWallThickness) / 2.0f, 0.0f, 0.0f}));
    root.addChild(&sideWall_, translated({(kWidth + kWallThickness) / 2.0f, 0.0f, 0.0f}));
    root.addChild(&ceiling_, translated({0.0f, kHeight, 0.0f}));

    // As caixas fechadas já têm normais e winding voltados para o interior nas faces úteis.
    // Rodapés laterais terminam nas faces dos frontais: juntas de topo, sem sobreposição visível.
    root.addChild(&endSkirting_, translated({0.0f, 0.0f, -kDepth / 2.0f + kSkirtingOffset}));
    root.addChild(&endSkirting_, translated({0.0f, 0.0f, kDepth / 2.0f - kSkirtingOffset}));
    root.addChild(&sideSkirting_, translated({-kWidth / 2.0f + kSkirtingOffset, 0.0f, 0.0f}));
    root.addChild(&sideSkirting_, translated({kWidth / 2.0f - kSkirtingOffset, 0.0f, 0.0f}));
    return root;
}

}  // namespace objects
