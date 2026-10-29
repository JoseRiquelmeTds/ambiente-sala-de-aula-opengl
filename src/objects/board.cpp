#include "objects/board.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <initializer_list>

#include "gfx/primitives.hpp"

namespace objects {

namespace {

constexpr float kWidth = 3.00f;
constexpr float kHeight = 1.20f;
constexpr float kBorder = 0.025f;
constexpr float kInnerWidth = kWidth - 2.0f * kBorder;
constexpr float kInnerHeight = kHeight - 2.0f * kBorder;
constexpr float kBodyDepth = 0.016f;
constexpr float kSurfaceDepth = 0.002f;
// Moldura até z = 0,022; acabamentos até 0,024. A lona existente começa em z = 0,038:
// os 14 mm de folga deixam a tela passar à frente sem alterar tela nem projetor.
constexpr float kFrameDepth = 0.022f;
constexpr float kCornerSize = 0.035f;
constexpr float kCornerDepth = 0.002f;

constexpr float kTrayWidth = 2.60f;
constexpr float kTrayDepth = 0.10f;
constexpr float kTrayBackZ = 0.012f;
constexpr float kTrayThickness = 0.004f;
constexpr float kTrayLipHeight = 0.014f;
constexpr float kTrayTop = -kHeight * 0.5f;

constexpr glm::vec3 kEraserSize(0.10f, 0.024f, 0.05f);
constexpr float kFeltHeight = 0.006f;  // apagador completo: 0,10 x 0,03 x 0,05 m (X, Y, Z)
constexpr float kMarkerRadius = 0.007f;
constexpr float kMarkerBodyLength = 0.102f;
constexpr float kMarkerCapRadius = 0.008f;
constexpr float kMarkerCapLength = 0.024f;
constexpr float kMarkerEndLength = 0.004f;  // comprimento total: 0,130 m
constexpr int kMarkerSegments = 16;
constexpr float kMaterialTextureSize = 0.25f;
constexpr float kAccessoryTextureSize = 0.10f;

glm::mat4 translated(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

// Agrupa caixas do mesmo material em dimensões reais, como nos demais móveis.
// Só translada: preserva normais, UVs e winding das primitivas.
void appendBox(MeshData& target, const glm::vec3& size, const glm::vec3& base) {
    const MeshData part = primitives::box(size, kMaterialTextureSize);
    const GLuint offset = static_cast<GLuint>(target.vertices.size());
    for (const Vertex& vertex : part.vertices) {
        target.vertices.push_back({vertex.position + base, vertex.normal, vertex.uv});
    }
    for (GLuint index : part.indices) {
        target.indices.push_back(offset + index);
    }
}

MeshData makeFrame() {
    MeshData data;
    for (float side : {-1.0f, 1.0f}) {
        const float y = side > 0.0f ? kHeight * 0.5f - kBorder : -kHeight * 0.5f;
        appendBox(data, {kWidth, kBorder, kFrameDepth}, {0.0f, y, kFrameDepth * 0.5f});
        // As laterais terminam entre as barras horizontais, sem faces frontais sobrepostas.
        const float x = side * (kWidth - kBorder) * 0.5f;
        appendBox(data, {kBorder, kInnerHeight, kFrameDepth},
                  {x, -kInnerHeight * 0.5f, kFrameDepth * 0.5f});
    }
    return data;
}

MeshData makeTray() {
    MeshData data;
    const float centerZ = kTrayBackZ + kTrayDepth * 0.5f;
    // O fundo encosta sob a moldura. A borda frontal e as tampas laterais retêm os materiais.
    appendBox(data, {kTrayWidth, kTrayThickness, kTrayDepth},
              {0.0f, kTrayTop - kTrayThickness, centerZ});
    appendBox(data, {kTrayWidth, kTrayLipHeight, kTrayThickness},
              {0.0f, kTrayTop, kTrayBackZ + kTrayDepth - kTrayThickness * 0.5f});
    for (float side : {-1.0f, 1.0f}) {
        appendBox(data, {kTrayThickness, kTrayLipHeight, kTrayDepth - kTrayThickness},
                  {side * (kTrayWidth - kTrayThickness) * 0.5f, kTrayTop,
                   centerZ - kTrayThickness * 0.5f});
    }
    return data;
}

}  // namespace

Board::Board()
    // UV 0..1 na face +Z, u para +X e v para +Y, para futura imagem inteira sem espelhamento.
    : surface_(primitives::box({kInnerWidth, kInnerHeight, kSurfaceDepth})),
      body_(primitives::box({kInnerWidth, kInnerHeight, kBodyDepth}, kMaterialTextureSize)),
      frame_(makeFrame()),
      corner_(primitives::box({kCornerSize, kCornerSize, kCornerDepth}, kAccessoryTextureSize)),
      tray_(makeTray()),
      eraser_(primitives::box(kEraserSize, kAccessoryTextureSize)),
      felt_(primitives::box({kEraserSize.x, kFeltHeight, kEraserSize.z}, kAccessoryTextureSize)),
      marker_(primitives::cylinder(kMarkerRadius, kMarkerBodyLength, kMarkerSegments, kAccessoryTextureSize)),
      markerCap_(primitives::cylinder(kMarkerCapRadius, kMarkerCapLength,
                                      kMarkerSegments, kAccessoryTextureSize)),
      markerEnd_(primitives::cylinder(kMarkerCapRadius, kMarkerEndLength,
                                      kMarkerSegments, kAccessoryTextureSize)) {}

SceneNode& Board::addTo(SceneNode& parent, const glm::mat4& placement) const {
    SceneNode& root = parent.addChild(nullptr, placement);
    root.addChild(&body_, translated({0.0f, -kInnerHeight * 0.5f, kBodyDepth * 0.5f}));
    root.addChild(&surface_, translated({0.0f, -kInnerHeight * 0.5f, kBodyDepth + kSurfaceDepth * 0.5f}));
    root.addChild(&frame_);
    for (float sideX : {-1.0f, 1.0f}) {
        for (float sideY : {-1.0f, 1.0f}) {
            const float y = sideY > 0.0f ? kHeight * 0.5f - kCornerSize : -kHeight * 0.5f;
            root.addChild(&corner_, translated({sideX * (kWidth - kCornerSize) * 0.5f, y,
                                               kFrameDepth + kCornerDepth * 0.5f}));
        }
    }
    root.addChild(&tray_);

    SceneNode& eraser = root.addChild(&eraser_, translated({-0.65f, kTrayTop + kFeltHeight, 0.065f}));
    eraser.addChild(&felt_, translated({0.0f, -kFeltHeight, 0.0f}));

    // Cilindros deitados em X (T · R); tampa e extremidade apoiam no fundo, sem atravessar a bandeja.
    // Nós separados permitem cores diferentes nas tampas na CG-72, reutilizando os mesmos meshes.
    for (const glm::vec3& position : {glm::vec3(0.30f, kTrayTop + kMarkerCapRadius, 0.043f),
                                     glm::vec3(0.55f, kTrayTop + kMarkerCapRadius, 0.080f)}) {
        const glm::mat4 markerPlacement = glm::rotate(translated(position), glm::radians(-90.0f),
                                                     glm::vec3(0.0f, 0.0f, 1.0f));
        SceneNode& marker = root.addChild(&marker_, markerPlacement);
        marker.addChild(&markerCap_, translated({0.0f, kMarkerBodyLength, 0.0f}));
        marker.addChild(&markerEnd_, translated({0.0f, -kMarkerEndLength, 0.0f}));
    }
    return root;
}

}  // namespace objects
